// Moemon Co-op: lobby + relay server.
//
// One server holds many rooms ("servers" from the player's point of view).
// A room has a name, an optional password and up to MP_MAX_PLAYERS players.
// The server knows very little about the game: inside a room it assigns
// player ids, caches each player's HELLO/STATE/big data so late joiners see
// everyone, and forwards peer-to-peer RELAY messages (challenges, battles).
//
// The same code runs as the standalone online lobby (tools/mp_server) and
// inside a game that hosts on the local network (with UDP discovery on).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "mp_sock.h"
#include "mp_net.h"

#define MAX_CONNS        512
#define MAX_ROOMS        256
#define LIST_MAX_ROOMS   40
#define IDLE_TIMEOUT_S   60      // no message at all for this long = dead connection
#define LOBBY_TIMEOUT_S  600     // connected but never joined a room
#define BAD_PASSWORD_LIMIT 5

struct Blob
{
    uint8_t *data;
    uint32_t total;
    uint32_t have;    // contiguous bytes received
};

struct SrvConn
{
    struct MpConn conn;
    int room;                 // -1 = in the lobby
    int slot;
    int helloDone;
    uint8_t hello[MP_HELLO_SIZE];
    uint8_t state[MP_WIRESTATE_SIZE];
    int hasState;
    struct Blob blobs[MP_BLOB_KINDS];
    time_t lastMsg;
    time_t connectedAt;
    int badPasswords;
    time_t slowUntil;
    char addr[64];
};

struct Room
{
    int used;
    char name[MP_ROOM_NAME_MAX + 1];
    uint8_t key[MP_ROOM_KEY_SIZE];
    int locked;
    int hidden;
    uint16_t version;
    int members[MP_MAX_PLAYERS];   // conn index or -1
};

static mp_sock sListen = MP_BADSOCK;
static mp_sock sDiscovery = MP_BADSOCK;
static mp_sock sWho = MP_BADSOCK;    // UDP on the lobby's port: tells a player the address it comes from (MPUdp)
static int sPort;
static struct SrvConn *sConns[MAX_CONNS];
static struct Room sRooms[MAX_ROOMS];
static uint8_t sOut[MP_MAX_MSG + 16];
static uint8_t sIn[MP_MAX_MSG + 16];

static void Drop(int c);

int MPServer_IsRunning(void)
{
    return sListen != MP_BADSOCK;
}

int MPServer_PlayerCount(void)
{
    int i, n = 0;
    for (i = 0; i < MAX_CONNS; i++)
        if (sConns[i] && sConns[i]->room >= 0)
            n++;
    return n;
}

int MPServer_RoomCount(void)
{
    int i, n = 0;
    for (i = 0; i < MAX_ROOMS; i++)
        n += sRooms[i].used;
    return n;
}

// ---------------------------------------------------------------------------
// Helpers

static int RoomPlayerCount(const struct Room *r)
{
    int i, n = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        n += (r->members[i] >= 0);
    return n;
}

static int NameEqualsNoCase(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
    {
        char x = (*a >= 'A' && *a <= 'Z') ? *a - 'A' + 'a' : *a;
        char y = (*b >= 'A' && *b <= 'Z') ? *b - 'A' + 'a' : *b;
        if (x != y)
            return 0;
    }
    return *a == *b;
}

static int FindRoom(const char *name)
{
    int i;
    for (i = 0; i < MAX_ROOMS; i++)
        if (sRooms[i].used && NameEqualsNoCase(sRooms[i].name, name))
            return i;
    return -1;
}

static int KeysEqual(const uint8_t *a, const uint8_t *b)
{
    int i, diff = 0;
    for (i = 0; i < MP_ROOM_KEY_SIZE; i++)
        diff |= a[i] ^ b[i];
    return diff == 0;
}

static void SendTo(int c, uint8_t type, const void *data, uint16_t len)
{
    if (sConns[c])
        MPConn_Queue(&sConns[c]->conn, type, data, len);
}

// u8 reason + u16 server version, so a client on another version can say who needs to update
static void SendRoomError(int c, uint8_t reason)
{
    uint8_t msg[3] = {reason, MP_PROTOCOL_VERSION & 0xFF, MP_PROTOCOL_VERSION >> 8};
    SendTo(c, MPMSG_S_ROOM_ERROR, msg, sizeof(msg));
}

// Sends "u8 fromSlot + data" to everyone in the room who said hello, except `except`
static void BroadcastWithId(int roomId, int except, uint8_t type, uint8_t fromSlot, const void *data, uint16_t len)
{
    struct Room *r = &sRooms[roomId];
    int i;
    sOut[0] = fromSlot;
    memcpy(sOut + 1, data, len);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        int m = r->members[i];
        if (m >= 0 && m != except && sConns[m] && sConns[m]->helloDone)
            SendTo(m, type, sOut, (uint16_t)(len + 1));
    }
}

static int GetStr(const uint8_t *data, uint16_t len, int *pos, char *out, int max)
{
    int n;
    if (*pos >= len)
        return 0;
    n = data[(*pos)++];
    if (n > max || *pos + n > len)
        return 0;
    memcpy(out, data + *pos, n);
    out[n] = 0;
    *pos += n;
    return 1;
}

static int PutStr(uint8_t *out, const char *s)
{
    int n = (int)strlen(s);
    out[0] = (uint8_t)n;
    memcpy(out + 1, s, n);
    return n + 1;
}

static void FreeBlobs(struct SrvConn *sc)
{
    int k;
    for (k = 0; k < MP_BLOB_KINDS; k++)
    {
        free(sc->blobs[k].data);
        memset(&sc->blobs[k], 0, sizeof(sc->blobs[k]));
    }
}

// Sends a finished blob to one connection in chunks
static void SendBlobTo(int to, uint8_t fromSlot, uint8_t kind, const struct Blob *b)
{
    uint32_t off;
    for (off = 0; off < b->total; off += MP_BLOB_CHUNK)
    {
        uint32_t n = b->total - off;
        if (n > MP_BLOB_CHUNK)
            n = MP_BLOB_CHUNK;
        sOut[0] = fromSlot;
        sOut[1] = kind;
        sOut[2] = b->total; sOut[3] = b->total >> 8; sOut[4] = b->total >> 16; sOut[5] = b->total >> 24;
        sOut[6] = off; sOut[7] = off >> 8; sOut[8] = off >> 16; sOut[9] = off >> 24;
        memcpy(sOut + 10, b->data + off, n);
        SendTo(to, MPMSG_S_BLOB, sOut, (uint16_t)(n + 10));
    }
}

// Room storage replies: back to the asker, or to everyone in its room
struct StoreCtx
{
    int c;
    int room;
};

static void StoreSend(void *ctx, int toAll, const uint8_t *data, uint16_t len)
{
    struct StoreCtx *sx = (struct StoreCtx *)ctx;
    int i;
    if (!toAll)
    {
        SendTo(sx->c, MPMSG_S_STORE, data, len);
        return;
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        int m = sRooms[sx->room].members[i];
        if (m >= 0 && sConns[m] && sConns[m]->helloDone && !(toAll == 2 && m == sx->c))
            SendTo(m, MPMSG_S_STORE, data, len);
    }
}

// ---------------------------------------------------------------------------
// Rooms

static void LeaveRoom(int c)
{
    struct SrvConn *sc = sConns[c];
    struct Room *r;
    int wasJoined;
    uint8_t slot;

    if (!sc || sc->room < 0)
        return;
    r = &sRooms[sc->room];
    slot = (uint8_t)sc->slot;
    wasJoined = sc->helloDone;
    r->members[sc->slot] = -1;
    if (wasJoined)
    {
        // a Pokemon still in their hand in the shared PC goes back into its boxes (told to whoever's left)
        struct StoreCtx sx = {c, sc->room};
        MPStore_PlayerLeft(r->key, slot, StoreSend, &sx);
    }
    if (wasJoined)
    {
        int i;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (r->members[i] >= 0 && sConns[r->members[i]] && sConns[r->members[i]]->helloDone)
                SendTo(r->members[i], MPMSG_S_PLAYER_LEAVE, &slot, 1);
    }
    MPNet_Log("%s left room \"%s\" (%d/%d)", sc->addr, r->name, RoomPlayerCount(r), MP_MAX_PLAYERS);
    if (RoomPlayerCount(r) == 0)
    {
        MPNet_Log("room \"%s\" closed", r->name);
        memset(r, 0, sizeof(*r));
    }
    sc->room = -1;
    sc->slot = -1;
    sc->helloDone = 0;
    sc->hasState = 0;
    FreeBlobs(sc);
}

static void EnterRoom(int c, int roomId, int created)
{
    struct SrvConn *sc = sConns[c];
    struct Room *r = &sRooms[roomId];
    uint8_t out[64];
    int i, n;

    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (r->members[i] < 0)
            break;
    r->members[i] = c;
    sc->room = roomId;
    sc->slot = i;
    sc->helloDone = 0;
    sc->hasState = 0;
    n = PutStr(out, r->name);
    out[n++] = (uint8_t)RoomPlayerCount(r);
    out[n++] = (uint8_t)created;
    SendTo(c, MPMSG_S_ROOM_OK, out, (uint16_t)n);
    MPNet_Log("%s %s room \"%s\" (%d/%d)", sc->addr, created ? "created" : "joined", r->name, RoomPlayerCount(r), MP_MAX_PLAYERS);
}

static int BuildRoomList(uint8_t *out, int max, int includeHidden)
{
    int i, count = 0, pos = 1;
    for (i = 0; i < MAX_ROOMS && count < LIST_MAX_ROOMS; i++)
    {
        struct Room *r = &sRooms[i];
        if (!r->used || (r->hidden && !includeHidden))
            continue;
        if (pos + MP_ROOM_NAME_MAX + 6 > max)
            break;
        pos += PutStr(out + pos, r->name);
        out[pos++] = (uint8_t)RoomPlayerCount(r);
        out[pos++] = MP_MAX_PLAYERS;
        out[pos++] = r->locked ? MP_ROOM_FLAG_LOCKED : 0;
        out[pos++] = r->version & 0xFF;
        out[pos++] = r->version >> 8;
        count++;
    }
    out[0] = (uint8_t)count;
    return pos;
}

static void HandleCreate(int c, const uint8_t *data, uint16_t len)
{
    struct SrvConn *sc = sConns[c];
    char name[MP_ROOM_NAME_MAX + 1];
    int pos = 2, i;
    uint16_t version;
    struct Room *r;

    if (len < 3)
        return;
    version = data[0] | (data[1] << 8);
    if (!GetStr(data, len, &pos, name, MP_ROOM_NAME_MAX) || pos + 2 + MP_ROOM_KEY_SIZE > len)
    {
        SendRoomError(c, MP_ROOM_ERR_BAD_NAME);
        return;
    }
    if (version != MP_PROTOCOL_VERSION)
    {
        SendRoomError(c, MP_ROOM_ERR_VERSION);
        return;
    }
    if (!MPNet_IsValidRoomName(name))
    {
        SendRoomError(c, MP_ROOM_ERR_BAD_NAME);
        return;
    }
    if (FindRoom(name) >= 0)
    {
        SendRoomError(c, MP_ROOM_ERR_NAME_TAKEN);
        return;
    }
    for (i = 0; i < MAX_ROOMS; i++)
        if (!sRooms[i].used)
            break;
    if (i == MAX_ROOMS)
    {
        SendRoomError(c, MP_ROOM_ERR_SERVER_FULL);
        return;
    }
    LeaveRoom(c);
    r = &sRooms[i];
    memset(r, 0, sizeof(*r));
    r->used = 1;
    snprintf(r->name, sizeof(r->name), "%s", name);
    r->locked = data[pos++] != 0;
    memcpy(r->key, data + pos, MP_ROOM_KEY_SIZE);
    pos += MP_ROOM_KEY_SIZE;
    r->hidden = data[pos] != 0;
    r->version = version;
    memset(r->members, -1, sizeof(r->members));
    EnterRoom(c, i, 1);
    (void)sc;
}

static void HandleJoin(int c, const uint8_t *data, uint16_t len)
{
    struct SrvConn *sc = sConns[c];
    char name[MP_ROOM_NAME_MAX + 1];
    int pos = 2, roomId;
    uint16_t version;
    time_t now = time(NULL);

    if (len < 3)
        return;
    if (sc->slowUntil > now)
    {
        SendRoomError(c, MP_ROOM_ERR_SLOW_DOWN);
        return;
    }
    version = data[0] | (data[1] << 8);
    if (!GetStr(data, len, &pos, name, MP_ROOM_NAME_MAX) || pos + MP_ROOM_KEY_SIZE > len)
    {
        SendRoomError(c, MP_ROOM_ERR_NOT_FOUND);
        return;
    }
    roomId = FindRoom(name);
    if (roomId < 0)
    {
        SendRoomError(c, MP_ROOM_ERR_NOT_FOUND);
        return;
    }
    if (sc->room == roomId)
    {
        EnterRoom(c, roomId, 0); // already here; answer again
        return;
    }
    if (version != sRooms[roomId].version || version != MP_PROTOCOL_VERSION)
    {
        SendRoomError(c, MP_ROOM_ERR_VERSION);
        return;
    }
    // Rooms without a password accept any key
    if (sRooms[roomId].locked && !KeysEqual(sRooms[roomId].key, data + pos))
    {
        if (++sc->badPasswords >= BAD_PASSWORD_LIMIT)
        {
            sc->badPasswords = 0;
            sc->slowUntil = now + 10;
        }
        SendRoomError(c, MP_ROOM_ERR_BAD_PASSWORD);
        return;
    }
    if (RoomPlayerCount(&sRooms[roomId]) >= MP_MAX_PLAYERS)
    {
        SendRoomError(c, MP_ROOM_ERR_FULL);
        return;
    }
    LeaveRoom(c);
    sc->badPasswords = 0;
    EnterRoom(c, roomId, 0);
}

// ---------------------------------------------------------------------------
// Messages

static void HandleBlob(int c, const uint8_t *data, uint16_t len)
{
    struct SrvConn *sc = sConns[c];
    uint8_t kind;
    uint32_t total, off, n;
    struct Blob *b;

    if (!sc->helloDone || len < 9)
        return;
    kind = data[0];
    total = data[1] | (data[2] << 8) | (data[3] << 16) | ((uint32_t)data[4] << 24);
    off = data[5] | (data[6] << 8) | (data[7] << 16) | ((uint32_t)data[8] << 24);
    n = len - 9;
    if (kind >= MP_BLOB_KINDS || total == 0 || total > MP_BLOB_MAX_SIZE || off + n > total)
        return;
    b = &sc->blobs[kind];
    if (off == 0)
    {
        free(b->data);
        b->data = (uint8_t *)malloc(total);
        b->total = total;
        b->have = 0;
    }
    if (!b->data || b->total != total || off != b->have)
        return; // out of order; the client resends from 0 on its next change
    memcpy(b->data + off, data + 9, n);
    b->have += n;
    BroadcastWithId(sc->room, c, MPMSG_S_BLOB, (uint8_t)sc->slot, data, len);
}

static void HandleMessage(int c, uint8_t type, uint8_t *data, uint16_t len)
{
    struct SrvConn *sc = sConns[c];
    struct Room *r = (sc->room >= 0) ? &sRooms[sc->room] : NULL;
    int i;

    sc->lastMsg = time(NULL);
    switch (type)
    {
    case MPMSG_C_ROOM_LIST:
    {
        int n = BuildRoomList(sOut, sizeof(sOut), 0);
        SendTo(c, MPMSG_S_ROOM_LIST, sOut, (uint16_t)n);
        break;
    }
    case MPMSG_C_ROOM_CREATE:
        HandleCreate(c, data, len);
        break;
    case MPMSG_C_ROOM_JOIN:
        HandleJoin(c, data, len);
        break;
    case MPMSG_C_ROOM_LEAVE:
        LeaveRoom(c);
        break;
    case MPMSG_C_HELLO:
    case MPMSG_C_INFO:
        if (len < MP_HELLO_SIZE)
            return;
        if (!r)
        {
            uint8_t reason = MP_REJECT_NO_ROOM;
            SendTo(c, MPMSG_S_REJECT, &reason, 1);
            return;
        }
        if (type == MPMSG_C_HELLO)
        {
            uint16_t ver = data[0] | (data[1] << 8);
            uint8_t welcome[2];
            if (ver != MP_PROTOCOL_VERSION)
            {
                uint8_t msg[3] = {MP_REJECT_VERSION, MP_PROTOCOL_VERSION & 0xFF, MP_PROTOCOL_VERSION >> 8};
                SendTo(c, MPMSG_S_REJECT, msg, sizeof(msg));
                return;
            }
            welcome[0] = (uint8_t)sc->slot;
            welcome[1] = MP_MAX_PLAYERS;
            SendTo(c, MPMSG_S_WELCOME, welcome, 2);
            // Tell the newcomer about everyone already here
            for (i = 0; i < MP_MAX_PLAYERS; i++)
            {
                int m = r->members[i];
                int k;
                struct SrvConn *o;
                if (m < 0 || m == c || !sConns[m] || !sConns[m]->helloDone)
                    continue;
                o = sConns[m];
                sOut[0] = (uint8_t)i;
                memcpy(sOut + 1, o->hello, MP_HELLO_SIZE);
                SendTo(c, MPMSG_S_PLAYER_INFO, sOut, MP_HELLO_SIZE + 1);
                for (k = 0; k < MP_BLOB_KINDS; k++)
                    if (o->blobs[k].data && o->blobs[k].have == o->blobs[k].total)
                        SendBlobTo(c, (uint8_t)i, (uint8_t)k, &o->blobs[k]);
                if (o->hasState)
                {
                    sOut[0] = (uint8_t)i;
                    memcpy(sOut + 1, o->state, MP_WIRESTATE_SIZE);
                    SendTo(c, MPMSG_S_STATE, sOut, MP_WIRESTATE_SIZE + 1);
                }
            }
            sc->helloDone = 1;
        }
        if (!sc->helloDone)
            return;
        memcpy(sc->hello, data, MP_HELLO_SIZE);
        BroadcastWithId(sc->room, c, MPMSG_S_PLAYER_INFO, (uint8_t)sc->slot, sc->hello, MP_HELLO_SIZE);
        break;
    case MPMSG_C_STATE:
        if (!r || !sc->helloDone || len < MP_WIRESTATE_SIZE)
            return;
        memcpy(sc->state, data, MP_WIRESTATE_SIZE);
        sc->hasState = 1;
        BroadcastWithId(sc->room, c, MPMSG_S_STATE, (uint8_t)sc->slot, data, MP_WIRESTATE_SIZE);
        break;
    case MPMSG_C_BLOB:
        if (r)
            HandleBlob(c, data, len);
        break;
    case MPMSG_C_RELAY:
    {
        uint8_t target;
        if (!r || !sc->helloDone || len < 2)
            return;
        target = data[0];
        sOut[0] = (uint8_t)sc->slot;
        memcpy(sOut + 1, data + 1, len - 1);
        if (target == 0xFF)
        {
            for (i = 0; i < MP_MAX_PLAYERS; i++)
            {
                int m = r->members[i];
                if (m >= 0 && m != c && sConns[m] && sConns[m]->helloDone)
                    SendTo(m, MPMSG_S_RELAY, sOut, len);
            }
        }
        else if (target < MP_MAX_PLAYERS)
        {
            int m = r->members[target];
            if (m >= 0 && sConns[m] && sConns[m]->helloDone)
                SendTo(m, MPMSG_S_RELAY, sOut, len);
        }
        break;
    }
    case MPMSG_C_PING:
        SendTo(c, MPMSG_S_PONG, data, len);
        break;
    case MPMSG_C_STORE:
        if (r && sc->helloDone)
        {
            struct StoreCtx sx = {c, sc->room};
            if (len >= 1 && data[0] >= MP_ST_RH_SUBMIT && data[0] <= MP_ST_SIM_ROSTER)
                MPScores_Handle(data, len, StoreSend, &sx);   // DANCE records, BATTLE SIMULATOR trainers: the whole server
            else
                MPStore_Handle(r->key, sc->slot, data, len, StoreSend, &sx);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Connections

static void Drop(int c)
{
    struct SrvConn *sc = sConns[c];
    if (!sc)
        return;
    LeaveRoom(c);
    MPConn_Reset(&sc->conn);
    free(sc->conn.send);
    FreeBlobs(sc);
    free(sc);
    sConns[c] = NULL;
}

static void AcceptNew(void)
{
    for (;;)
    {
        struct sockaddr_storage addr;
        mp_socklen addrLen = sizeof(addr);
        mp_sock s = accept(sListen, (struct sockaddr *)&addr, &addrLen);
        struct SrvConn *sc;
        int i;

        if (s == MP_BADSOCK)
            break;
        for (i = 0; i < MAX_CONNS; i++)
            if (!sConns[i])
                break;
        if (i == MAX_CONNS)
        {
            mp_closesock(s);
            continue;
        }
        MPSock_SetNonBlocking(s);
        sc = (struct SrvConn *)calloc(1, sizeof(*sc));
        if (!sc)
        {
            mp_closesock(s);
            continue;
        }
        sc->conn.sock = s;
        sc->room = -1;
        sc->slot = -1;
        sc->lastMsg = sc->connectedAt = time(NULL);
        if (addr.ss_family == AF_INET)
            inet_ntop(AF_INET, &((struct sockaddr_in *)&addr)->sin_addr, sc->addr, sizeof(sc->addr));
        else if (addr.ss_family == AF_INET6)
            inet_ntop(AF_INET6, &((struct sockaddr_in6 *)&addr)->sin6_addr, sc->addr, sizeof(sc->addr));
        else
            snprintf(sc->addr, sizeof(sc->addr), "?");
        sConns[i] = sc;
    }
}

static void AnswerDiscovery(void)
{
    uint8_t buf[64];
    int guard;
    if (sDiscovery == MP_BADSOCK)
        return;
    for (guard = 0; guard < 32; guard++)
    {
        struct sockaddr_storage from;
        mp_socklen fromLen = sizeof(from);
        int n = (int)recvfrom(sDiscovery, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromLen);
        int len;
        if (n <= 0)
            break;
        if (n < 7 || memcmp(buf, "MOEMON?", 7) != 0)
            continue;
        memcpy(sOut, "MOEMON!", 7);
        sOut[7] = MP_PROTOCOL_VERSION & 0xFF;
        sOut[8] = MP_PROTOCOL_VERSION >> 8;
        sOut[9] = sPort & 0xFF;
        sOut[10] = sPort >> 8;
        len = 11 + BuildRoomList(sOut + 11, 1200, 0);
        sendto(sDiscovery, (const char *)sOut, len, 0, (struct sockaddr *)&from, fromLen);
    }
}

// "MFWHO" + 4 bytes -> "MFYOU" + the same 4 bytes + the IPv4 address and port the datagram came from: a player's
// address as the internet sees it, for a direct connection to a friend (the VS cabinet, src/mp/mp_fight.c)
static void AnswerWho(void)
{
    uint8_t buf[32], out[17];
    int guard;
    if (sWho == MP_BADSOCK)
        return;
    for (guard = 0; guard < 64; guard++)
    {
        struct sockaddr_in from;
        mp_socklen fromLen = sizeof(from);
        int n = (int)recvfrom(sWho, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromLen);
        if (n <= 0)
            break;
        if (n < 9 || memcmp(buf, "MFWHO", 5) != 0 || from.sin_family != AF_INET)
            continue;
        memcpy(out, "MFYOU", 5);
        memcpy(out + 5, buf + 5, 4);
        memcpy(out + 9, &from.sin_addr.s_addr, 4);   // network order
        memcpy(out + 13, &from.sin_port, 2);
        sendto(sWho, (const char *)out, 15, 0, (struct sockaddr *)&from, fromLen);
    }
}

static mp_sock OpenListen(int port)
{
    struct sockaddr_in6 addr6;
    struct sockaddr_in addr4;
    int one = 1;
    mp_sock s = socket(AF_INET6, SOCK_STREAM, 0);

    // Prefer a dual stack IPv6 socket, fall back to IPv4
    if (s != MP_BADSOCK)
    {
        int zero = 0;
#ifndef _WIN32
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
#endif
        setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&zero, sizeof(zero));
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = htons((uint16_t)port);
        if (bind(s, (struct sockaddr *)&addr6, sizeof(addr6)) != 0)
        {
            mp_closesock(s);
            s = MP_BADSOCK;
        }
    }
    if (s == MP_BADSOCK)
    {
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == MP_BADSOCK)
            return MP_BADSOCK;
#ifndef _WIN32
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
#endif
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = htonl(INADDR_ANY);
        addr4.sin_port = htons((uint16_t)port);
        if (bind(s, (struct sockaddr *)&addr4, sizeof(addr4)) != 0)
        {
            mp_closesock(s);
            return MP_BADSOCK;
        }
    }
    if (listen(s, 16) != 0)
    {
        mp_closesock(s);
        return MP_BADSOCK;
    }
    MPSock_SetNonBlocking(s);
    return s;
}

int MPServer_Start(int port, int lanDiscovery)
{
    int i;

    MPNet_Init();
    if (sListen != MP_BADSOCK)
        return 0;
    for (i = 0; i < MAX_ROOMS; i++)
        memset(&sRooms[i], 0, sizeof(sRooms[i]));
    sListen = OpenListen(port);
    if (sListen == MP_BADSOCK)
    {
        MPNet_Log("port %d is already in use", port);
        return -1;
    }
    sPort = port;
    {
        struct sockaddr_in addr;
        sWho = socket(AF_INET, SOCK_DGRAM, 0);
        if (sWho != MP_BADSOCK)
        {
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons((uint16_t)port);
            if (bind(sWho, (struct sockaddr *)&addr, sizeof(addr)) != 0)
            {
                mp_closesock(sWho);
                sWho = MP_BADSOCK;
            }
            else
                MPSock_SetNonBlocking(sWho);
        }
    }
    if (lanDiscovery)
    {
        struct sockaddr_in addr;
        int one = 1;
        sDiscovery = socket(AF_INET, SOCK_DGRAM, 0);
        if (sDiscovery != MP_BADSOCK)
        {
            setsockopt(sDiscovery, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(MP_DISCOVERY_PORT);
            if (bind(sDiscovery, (struct sockaddr *)&addr, sizeof(addr)) != 0)
            {
                MPNet_Log("local network discovery unavailable (UDP port %d busy)", MP_DISCOVERY_PORT);
                mp_closesock(sDiscovery);
                sDiscovery = MP_BADSOCK;
            }
            else
                MPSock_SetNonBlocking(sDiscovery);
        }
    }
    MPNet_Log("lobby server listening on port %d", port);
    return 0;
}

int MPServer_Port(void)
{
    return MPServer_IsRunning() ? sPort : 0;
}

void MPServer_Stop(void)
{
    int i;
    MPPortMap_Close(); // close the router port opened for this server
    for (i = 0; i < MAX_CONNS; i++)
        Drop(i);
    if (sListen != MP_BADSOCK)
        mp_closesock(sListen);
    if (sDiscovery != MP_BADSOCK)
        mp_closesock(sDiscovery);
    if (sWho != MP_BADSOCK)
        mp_closesock(sWho);
    sListen = MP_BADSOCK;
    sDiscovery = MP_BADSOCK;
    sWho = MP_BADSOCK;
}

void MPServer_Poll(void)
{
    int i;
    uint8_t type;
    uint16_t len;
    time_t now;

    if (sListen == MP_BADSOCK)
        return;
    AcceptNew();
    AnswerDiscovery();
    AnswerWho();
    now = time(NULL);

    for (i = 0; i < MAX_CONNS; i++)
    {
        struct SrvConn *sc = sConns[i];
        int rc, guard;
        if (!sc)
            continue;
        if (MPConn_Fill(&sc->conn) < 0)
        {
            while (sConns[i] && MPConn_Pop(&sConns[i]->conn, &type, sIn, &len) == 1)
                HandleMessage(i, type, sIn, len);
            Drop(i);
            continue;
        }
        rc = 0;
        for (guard = 0; guard < 256 && sConns[i]; guard++)
        {
            rc = MPConn_Pop(&sConns[i]->conn, &type, sIn, &len);
            if (rc != 1)
                break;
            HandleMessage(i, type, sIn, len);
        }
        if (!sConns[i])
            continue;
        if (rc < 0)
        {
            Drop(i);
            continue;
        }
        if (now - sc->lastMsg > IDLE_TIMEOUT_S
         || (sc->room < 0 && now - sc->connectedAt > LOBBY_TIMEOUT_S))
        {
            MPNet_Log("%s timed out", sc->addr);
            Drop(i);
        }
    }

    for (i = 0; i < MAX_CONNS; i++)
        if (sConns[i] && MPConn_Flush(&sConns[i]->conn) < 0)
            Drop(i);
}

void MPServer_Wait(int ms)
{
    fd_set rset, wset;
    struct timeval tv;
    mp_sock maxSock = sListen;
    int i;

    if (sListen == MP_BADSOCK)
        return;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(sListen, &rset);
    if (sDiscovery != MP_BADSOCK)
    {
        FD_SET(sDiscovery, &rset);
        if (sDiscovery > maxSock)
            maxSock = sDiscovery;
    }
    if (sWho != MP_BADSOCK)
    {
        FD_SET(sWho, &rset);
        if (sWho > maxSock)
            maxSock = sWho;
    }
    for (i = 0; i < MAX_CONNS; i++)
    {
        mp_sock s;
        if (!sConns[i])
            continue;
        s = sConns[i]->conn.sock;
#ifndef _WIN32
        if (s >= FD_SETSIZE)
            continue;
#endif
        FD_SET(s, &rset);
        if (sConns[i]->conn.sendLen > 0)
            FD_SET(s, &wset);
        if (s > maxSock)
            maxSock = s;
    }
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    select((int)maxSock + 1, &rset, &wset, NULL, &tv);
}
