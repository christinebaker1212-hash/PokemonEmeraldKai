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

// ---------------------------------------------------------------------------
// The mailbox (MPMSG_C_MAIL) and pipes (MPMSG_C_PIPE): outside rooms, for friends and phones (docs/PHONE_LINK.md
// "Phase 4"). The server never reads what's in them: friends seal their records with keys only they share.

#define MAIL_KEYS   4096
#define MAIL_FILE   "moemon_mail.dat"
#define MAIL_MAGIC  "MOEMAIL1\n"
struct MailRec
{
    uint8_t key[16];
    uint8_t owner[16];        // SHA-256 of the owner's secret (all 0: no owner yet)
    uint32_t expires;
    uint8_t count;
    uint16_t len[MAIL_ENTRIES];
    uint8_t *data[MAIL_ENTRIES];
};
static struct MailRec *sMail[MAIL_KEYS];
static int sMailDirty;
static time_t sMailSaved;

// Mail trades' escrow (ESC_*): two deposits per trade, kept until each side has taken what it's owed
#define ESC_RECS   2048
#define ESC_FILE   "moemon_escrow.dat"
#define ESC_MAGIC  "MOEESC1\n"
#define ESC_KEEP_DAYS 365   // (a deposit nobody came back for stays this long: a POKéMON is never dropped soon)
struct EscRec
{
    uint8_t key[16];
    uint8_t lock[2][16];      // SHA-256 of each side's secret (all 0: not set yet)
    uint8_t state;            // ESC_HAVE0 | ESC_HAVE1 | ESC_TAKEN0 | ESC_TAKEN1 | ESC_CLOSED
    uint32_t expires;
    uint16_t len[2];
    uint8_t *data[2];
};
static struct EscRec *sEsc[ESC_RECS];
static int sEscDirty;

#define PIPE_HOSTS 512
#define PIPE_CHANS 512
static struct { uint8_t id[16]; int conn; } sPipeHost[PIPE_HOSTS];   // conn -1: free
static struct { uint16_t ch; int host, phone; } sPipeChan[PIPE_CHANS];   // ch 0: free
static uint16_t sNextChan = 1;
static int sPipeInit;
static void PipeDropConn(int c);
static int UsesPipes(int c);

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

// --------------------------------------------------------------------------------------------------- the mailbox

static void MailFree(int i)
{
    int k;
    if (!sMail[i])
        return;
    for (k = 0; k < sMail[i]->count; k++)
        free(sMail[i]->data[k]);
    free(sMail[i]);
    sMail[i] = NULL;
    sMailDirty = 1;
}

static int MailFind(const uint8_t *key, int create)
{
    int i, freeSlot = -1;
    time_t now = time(NULL);
    for (i = 0; i < MAIL_KEYS; i++)
    {
        if (sMail[i] && (time_t)sMail[i]->expires < now)
            MailFree(i);
        if (sMail[i] && memcmp(sMail[i]->key, key, 16) == 0)
            return i;
        if (!sMail[i] && freeSlot < 0)
            freeSlot = i;
    }
    if (!create || freeSlot < 0)
        return -1;
    sMail[freeSlot] = (struct MailRec *)calloc(1, sizeof(struct MailRec));
    if (!sMail[freeSlot])
        return -1;
    memcpy(sMail[freeSlot]->key, key, 16);
    return freeSlot;
}

static void MailSave(void)
{
    FILE *f = fopen(MAIL_FILE ".tmp", "wb");
    int i, k;
    if (!f)
        return;
    fputs(MAIL_MAGIC, f);
    for (i = 0; i < MAIL_KEYS; i++)
    {
        struct MailRec *m = sMail[i];
        if (!m)
            continue;
        fwrite(m->key, 1, 16, f);
        fwrite(m->owner, 1, 16, f);
        fwrite(&m->expires, 4, 1, f);
        fputc(m->count, f);
        for (k = 0; k < m->count; k++)
        {
            fwrite(&m->len[k], 2, 1, f);
            fwrite(m->data[k], 1, m->len[k], f);
        }
    }
    fclose(f);
    remove(MAIL_FILE);
    rename(MAIL_FILE ".tmp", MAIL_FILE);
    sMailDirty = 0;
}

static void MailLoad(void)
{
    FILE *f = fopen(MAIL_FILE, "rb");
    char magic[16] = {0};
    int i = 0;
    if (!f)
        return;
    if (fread(magic, 1, strlen(MAIL_MAGIC), f) == strlen(MAIL_MAGIC) && memcmp(magic, MAIL_MAGIC, strlen(MAIL_MAGIC)) == 0)
    {
        while (i < MAIL_KEYS)
        {
            struct MailRec *m = (struct MailRec *)calloc(1, sizeof(*m));
            int k, ok = 1, c;
            if (!m || fread(m->key, 1, 16, f) != 16 || fread(m->owner, 1, 16, f) != 16 || fread(&m->expires, 4, 1, f) != 1
             || (c = fgetc(f)) == EOF || c > MAIL_ENTRIES)
            {
                free(m);
                break;
            }
            m->count = (uint8_t)c;
            for (k = 0; k < m->count && ok; k++)
            {
                ok = fread(&m->len[k], 2, 1, f) == 1 && m->len[k] <= MAIL_ENTRY_MAX
                  && (m->data[k] = (uint8_t *)malloc(m->len[k] ? m->len[k] : 1)) != NULL
                  && fread(m->data[k], 1, m->len[k], f) == m->len[k];
            }
            if (!ok)
            {
                for (c = 0; c < k; c++)
                    free(m->data[c]);
                free(m);
                break;
            }
            sMail[i++] = m;
        }
    }
    fclose(f);
}

static void EscSave(void);
static void EscHandle(int c, const uint8_t *data, uint16_t len);

static void MailSaveSoon(time_t now)
{
    if (sMailDirty && now - sMailSaved >= 5)
    {
        sMailSaved = now;
        MailSave();
    }
    if (sEscDirty)
        EscSave();   // (at once: a deposit is a POKéMON)
}

static void MailAnswer(int c, uint8_t op, const uint8_t *key, int i)
{
    int n = 0, k;
    sOut[n++] = op;
    memcpy(sOut + n, key, 16);
    n += 16;
    if (op == MAIL_GET)
    {
        struct MailRec *m = i >= 0 ? sMail[i] : NULL;
        int count = 0, at = n++;
        for (k = 0; m && k < m->count; k++)
        {
            if (n + 2 + m->len[k] > MP_MAX_MSG - 8)
                break;   // (what fits: newest entries are last, the oldest go first in ADD)
            sOut[n++] = m->len[k] & 0xFF;
            sOut[n++] = m->len[k] >> 8;
            memcpy(sOut + n, m->data[k], m->len[k]);
            n += m->len[k];
            count++;
        }
        sOut[at] = (uint8_t)count;
    }
    SendTo(c, MPMSG_S_MAIL, sOut, (uint16_t)n);
}

// Does SECRET open the record's owner lock (or has it none yet)?
static int MailOwner(struct MailRec *m, const uint8_t *secret)
{
    static const uint8_t zero[16];
    uint8_t h[32];
    MPNet_Sha256(secret, 16, h);
    if (memcmp(m->owner, zero, 16) == 0)
    {
        memcpy(m->owner, h, 16);
        return 1;
    }
    return memcmp(m->owner, h, 16) == 0;
}

// PUT: key, secret, u32 ttl, u16 len, data; ADD: key, u32 ttl, u16 len, data; GET: key; DEL: key, secret;
// TAKE: key, secret, u8 index
static void MailHandle(int c, const uint8_t *data, uint16_t len)
{
    uint8_t op;
    const uint8_t *key;
    int i, k;
    if (len < 17)
        return;
    op = data[0];
    key = data + 1;
    if (op >= ESC_PUT && op <= ESC_LOOK)
    {
        EscHandle(c, data, len);
        return;
    }
    switch (op)
    {
    case MAIL_PUT:
    case MAIL_ADD:
    {
        int at = op == MAIL_PUT ? 33 : 17;
        uint32_t ttl;
        uint16_t n;
        uint8_t *copy;
        if (len < at + 6)
            return;
        ttl = data[at] | (data[at + 1] << 8) | (data[at + 2] << 16) | ((uint32_t)data[at + 3] << 24);
        n = data[at + 4] | (data[at + 5] << 8);
        if (n > MAIL_ENTRY_MAX || len < at + 6 + n)
            return;
        i = MailFind(key, 1);
        if (i < 0)
            return;
        if (op == MAIL_PUT && !MailOwner(sMail[i], data + 17))
        {
            MailAnswer(c, MAIL_DENIED, key, i);
            return;
        }
        if (op == MAIL_PUT)
        {
            for (k = 0; k < sMail[i]->count; k++)
                free(sMail[i]->data[k]);
            sMail[i]->count = 0;
        }
        else if (sMail[i]->count == MAIL_ENTRIES)
        {
            free(sMail[i]->data[0]);
            memmove(&sMail[i]->data[0], &sMail[i]->data[1], (MAIL_ENTRIES - 1) * sizeof(sMail[i]->data[0]));
            memmove(&sMail[i]->len[0], &sMail[i]->len[1], (MAIL_ENTRIES - 1) * sizeof(sMail[i]->len[0]));
            sMail[i]->count--;
        }
        copy = (uint8_t *)malloc(n ? n : 1);
        if (!copy)
            return;
        memcpy(copy, data + at + 6, n);
        sMail[i]->data[sMail[i]->count] = copy;
        sMail[i]->len[sMail[i]->count++] = n;
        ttl = ttl > 30 * 86400 ? 30 * 86400 : ttl;
        if ((uint32_t)(time(NULL) + ttl) > sMail[i]->expires || op == MAIL_PUT)
            sMail[i]->expires = (uint32_t)(time(NULL) + ttl);
        sMailDirty = 1;
        MailAnswer(c, op, key, i);
        break;
    }
    case MAIL_GET:
        MailAnswer(c, MAIL_GET, key, MailFind(key, 0));
        break;
    case MAIL_DEL:
    case MAIL_TAKE:
        if (len < 33)
            return;
        i = MailFind(key, 0);
        if (i < 0)
        {
            MailAnswer(c, op, key, -1);
            return;
        }
        if (!MailOwner(sMail[i], data + 17))
        {
            MailAnswer(c, MAIL_DENIED, key, i);
            return;
        }
        if (op == MAIL_DEL)
            MailFree(i);
        else if (len >= 34 && data[33] < sMail[i]->count)
        {
            k = data[33];
            free(sMail[i]->data[k]);
            memmove(&sMail[i]->data[k], &sMail[i]->data[k + 1], (sMail[i]->count - k - 1) * sizeof(sMail[i]->data[0]));
            memmove(&sMail[i]->len[k], &sMail[i]->len[k + 1], (sMail[i]->count - k - 1) * sizeof(sMail[i]->len[0]));
            sMail[i]->count--;
            sMailDirty = 1;
        }
        MailAnswer(c, op, key, i);
        break;
    }
}


// ---------------------------------------------------------------------------------------------------- the escrow

static void EscFree(int i)
{
    if (!sEsc[i])
        return;
    free(sEsc[i]->data[0]);
    free(sEsc[i]->data[1]);
    free(sEsc[i]);
    sEsc[i] = NULL;
    sEscDirty = 1;
}

static int EscFind(const uint8_t *key, int create)
{
    int i, freeSlot = -1;
    time_t now = time(NULL);
    for (i = 0; i < ESC_RECS; i++)
    {
        if (sEsc[i] && (time_t)sEsc[i]->expires < now)
            EscFree(i);
        if (sEsc[i] && memcmp(sEsc[i]->key, key, 16) == 0)
            return i;
        if (!sEsc[i] && freeSlot < 0)
            freeSlot = i;
    }
    if (!create || freeSlot < 0)
        return -1;
    sEsc[freeSlot] = (struct EscRec *)calloc(1, sizeof(struct EscRec));
    if (!sEsc[freeSlot])
        return -1;
    memcpy(sEsc[freeSlot]->key, key, 16);
    sEsc[freeSlot]->expires = (uint32_t)(now + ESC_KEEP_DAYS * 86400);
    sEscDirty = 1;
    return freeSlot;
}

static void EscSave(void)
{
    FILE *f = fopen(ESC_FILE ".tmp", "wb");
    int i, k;
    if (!f)
        return;
    fputs(ESC_MAGIC, f);
    for (i = 0; i < ESC_RECS; i++)
    {
        struct EscRec *e = sEsc[i];
        if (!e)
            continue;
        fwrite(e->key, 1, 16, f);
        fwrite(e->lock, 1, 32, f);
        fputc(e->state, f);
        fwrite(&e->expires, 4, 1, f);
        for (k = 0; k < 2; k++)
        {
            fwrite(&e->len[k], 2, 1, f);
            if (e->len[k])
                fwrite(e->data[k], 1, e->len[k], f);
        }
    }
    fflush(f);
    fclose(f);
    remove(ESC_FILE);
    rename(ESC_FILE ".tmp", ESC_FILE);
    sEscDirty = 0;
}

static void EscLoad(void)
{
    FILE *f = fopen(ESC_FILE, "rb");
    char magic[16] = {0};
    int i = 0, k, c;
    if (!f)
        return;
    if (fread(magic, 1, strlen(ESC_MAGIC), f) == strlen(ESC_MAGIC) && memcmp(magic, ESC_MAGIC, strlen(ESC_MAGIC)) == 0)
    {
        while (i < ESC_RECS)
        {
            struct EscRec *e = (struct EscRec *)calloc(1, sizeof(*e));
            int ok = e && fread(e->key, 1, 16, f) == 16 && fread(e->lock, 1, 32, f) == 32 && (c = fgetc(f)) != EOF
                  && fread(&e->expires, 4, 1, f) == 1;
            if (ok)
                e->state = (uint8_t)c;
            for (k = 0; k < 2 && ok; k++)
            {
                ok = fread(&e->len[k], 2, 1, f) == 1 && e->len[k] <= ESC_DATA_MAX;
                if (ok && e->len[k])
                    ok = (e->data[k] = (uint8_t *)malloc(e->len[k])) != NULL && fread(e->data[k], 1, e->len[k], f) == e->len[k];
            }
            if (!ok)
            {
                if (e)
                {
                    free(e->data[0]);
                    free(e->data[1]);
                }
                free(e);
                break;
            }
            sEsc[i++] = e;
        }
    }
    fclose(f);
}

// Does SECRET open SIDE's lock (setting it when there's none yet)?
static int EscLock(struct EscRec *e, int side, const uint8_t *secret)
{
    static const uint8_t zero[16];
    uint8_t h[32];
    MPNet_Sha256(secret, 16, h);
    if (memcmp(e->lock[side], zero, 16) == 0)
    {
        memcpy(e->lock[side], h, 16);
        sEscDirty = 1;
        return 1;
    }
    return memcmp(e->lock[side], h, 16) == 0;
}

static void EscAnswer(int c, uint8_t op, const uint8_t *key, uint8_t state, const uint8_t *data, uint16_t len)
{
    int n = 0;
    if (sEscDirty)
        EscSave();   // (on disk before anyone hears of it: a deposit is a POKéMON)
    sOut[n++] = op;
    memcpy(sOut + n, key, 16);
    n += 16;
    sOut[n++] = state;
    sOut[n++] = len & 0xFF;
    sOut[n++] = len >> 8;
    if (len)
        memcpy(sOut + n, data, len);
    n += len;
    SendTo(c, MPMSG_S_MAIL, sOut, (uint16_t)n);
}

// The two-sided escrow (mp_net.h ESC_*). Each answer says the record's state, so a side that asks again (a game that
// stopped between sending and hearing back) learns where it stands: every op can be repeated safely.
static void EscHandle(int c, const uint8_t *data, uint16_t len)
{
    uint8_t op = data[0], side;
    const uint8_t *key = data + 1, *secret;
    struct EscRec *e;
    int i, both;
    if (op == ESC_LOOK)
    {
        i = EscFind(key, 0);
        EscAnswer(c, op, key, i < 0 ? 0 : sEsc[i]->state, NULL, 0);
        return;
    }
    if (len < 34 || data[17] > 1)
        return;
    side = data[17];
    secret = data + 18;
    i = EscFind(key, op == ESC_PUT || op == ESC_CANCEL);
    if (i < 0)
    {
        EscAnswer(c, op, key, 0, NULL, 0);
        return;
    }
    e = sEsc[i];
    if (!EscLock(e, side, secret))
    {
        EscAnswer(c, MAIL_DENIED, key, e->state, NULL, 0);
        return;
    }
    both = (e->state & (ESC_HAVE0 | ESC_HAVE1)) == (ESC_HAVE0 | ESC_HAVE1);
    switch (op)
    {
    case ESC_PUT:
    {
        uint16_t n;
        if (len < 36)
            return;
        n = data[34] | (data[35] << 8);
        if (n == 0 || n > ESC_DATA_MAX || len < 36 + n)
            return;
        if (!(e->state & ESC_CLOSED) && !(e->state & (ESC_HAVE0 << side)))
        {
            e->data[side] = (uint8_t *)malloc(n);
            if (!e->data[side])
                return;
            memcpy(e->data[side], data + 36, n);
            e->len[side] = n;
            e->state |= ESC_HAVE0 << side;
            sEscDirty = 1;
        }
        EscAnswer(c, op, key, e->state, NULL, 0);   // (closed: nothing kept, the depositor keeps its own)
        break;
    }
    case ESC_TAKE:
        if (both && !(e->state & ESC_CLOSED))
        {
            if (!(e->state & (ESC_TAKEN0 << side)))
            {
                e->state |= ESC_TAKEN0 << side;
                if ((e->state & (ESC_TAKEN0 | ESC_TAKEN1)) == (ESC_TAKEN0 | ESC_TAKEN1))
                    e->expires = (uint32_t)(time(NULL) + 30 * 86400);   // (both done: kept a while for late repeats)
                sEscDirty = 1;
            }
            EscAnswer(c, op, key, e->state, e->data[side ^ 1], e->len[side ^ 1]);
        }
        else
            EscAnswer(c, op, key, e->state, NULL, 0);
        break;
    case ESC_CANCEL:
        if (!both || (e->state & ESC_CLOSED))
        {
            if (!(e->state & ESC_CLOSED))
            {
                e->state |= ESC_CLOSED;
                if (!(e->state & (ESC_HAVE0 | ESC_HAVE1)))
                    e->expires = (uint32_t)(time(NULL) + 30 * 86400);   // (a tombstone: only that it's off)
                sEscDirty = 1;
            }
            EscAnswer(c, op, key, e->state, e->data[side], e->len[side]);
        }
        else
            EscAnswer(c, op, key, e->state, NULL, 0);   // (both are in: the trade stands)
        break;
    }
}

// ------------------------------------------------------------------------------------------------------ pipes

static void PipeInit(void)
{
    int i;
    if (sPipeInit)
        return;
    sPipeInit = 1;
    for (i = 0; i < PIPE_HOSTS; i++)
        sPipeHost[i].conn = -1;
}

static int UsesPipes(int c)
{
    int i;
    PipeInit();
    for (i = 0; i < PIPE_HOSTS; i++)
        if (sPipeHost[i].conn == c)
            return 1;
    for (i = 0; i < PIPE_CHANS; i++)
        if (sPipeChan[i].ch && (sPipeChan[i].host == c || sPipeChan[i].phone == c))
            return 1;
    return 0;
}

static void PipeSendOp(int c, uint8_t op, uint16_t ch, const uint8_t *extra, uint16_t n)
{
    sOut[0] = op;
    sOut[1] = ch & 0xFF;
    sOut[2] = ch >> 8;
    if (n)
        memcpy(sOut + 3, extra, n);
    SendTo(c, MPMSG_S_PIPE, sOut, (uint16_t)(3 + n));
}

static void PipeClose(int i, int except)
{
    if (sPipeChan[i].host != except && sPipeChan[i].host >= 0 && sConns[sPipeChan[i].host])
        PipeSendOp(sPipeChan[i].host, PIPE_CLOSED, sPipeChan[i].ch, NULL, 0);
    if (sPipeChan[i].phone != except && sPipeChan[i].phone >= 0 && sConns[sPipeChan[i].phone])
        PipeSendOp(sPipeChan[i].phone, PIPE_CLOSED, sPipeChan[i].ch, NULL, 0);
    sPipeChan[i].ch = 0;
}

static void PipeDropConn(int c)
{
    int i;
    PipeInit();
    for (i = 0; i < PIPE_HOSTS; i++)
        if (sPipeHost[i].conn == c)
            sPipeHost[i].conn = -1;
    for (i = 0; i < PIPE_CHANS; i++)
        if (sPipeChan[i].ch && (sPipeChan[i].host == c || sPipeChan[i].phone == c))
            PipeClose(i, c);
}

static void PipeHandle(int c, const uint8_t *data, uint16_t len)
{
    int i, free1 = -1;
    PipeInit();
    if (len < 1)
        return;
    switch (data[0])
    {
    case PIPE_HOST:
        if (len < 17)
            return;
        for (i = 0; i < PIPE_HOSTS; i++)
        {
            if (sPipeHost[i].conn >= 0 && memcmp(sPipeHost[i].id, data + 1, 16) == 0)
            {
                sPipeHost[i].conn = c;   // (the game again, on a new connection)
                return;
            }
            if (sPipeHost[i].conn < 0 && free1 < 0)
                free1 = i;
        }
        if (free1 >= 0)
        {
            memcpy(sPipeHost[free1].id, data + 1, 16);
            sPipeHost[free1].conn = c;
        }
        break;
    case PIPE_UNHOST:
        if (len < 17)
            return;
        for (i = 0; i < PIPE_HOSTS; i++)
            if (sPipeHost[i].conn == c && memcmp(sPipeHost[i].id, data + 1, 16) == 0)
                sPipeHost[i].conn = -1;
        break;
    case PIPE_JOIN:
    {
        int host = -1;
        if (len < 17)
            return;
        for (i = 0; i < PIPE_HOSTS; i++)
            if (sPipeHost[i].conn >= 0 && sConns[sPipeHost[i].conn] && memcmp(sPipeHost[i].id, data + 1, 16) == 0)
                host = sPipeHost[i].conn;
        for (i = 0; i < PIPE_CHANS && free1 < 0; i++)
            if (!sPipeChan[i].ch)
                free1 = i;
        if (host < 0 || free1 < 0)
        {
            sOut[0] = PIPE_NOHOST;
            memcpy(sOut + 1, data + 1, 16);
            SendTo(c, MPMSG_S_PIPE, sOut, 17);
            return;
        }
        if (++sNextChan == 0)
            sNextChan = 1;
        sPipeChan[free1].ch = sNextChan;
        sPipeChan[free1].host = host;
        sPipeChan[free1].phone = c;
        PipeSendOp(host, PIPE_JOINED, sNextChan, data + 1, 16);
        PipeSendOp(c, PIPE_OPENED, sNextChan, NULL, 0);
        break;
    }
    case PIPE_DATA:
    case PIPE_CLOSE:
    {
        uint16_t ch;
        if (len < 3)
            return;
        ch = data[1] | (data[2] << 8);
        for (i = 0; i < PIPE_CHANS; i++)
            if (sPipeChan[i].ch == ch && (sPipeChan[i].host == c || sPipeChan[i].phone == c))
            {
                if (data[0] == PIPE_CLOSE)
                    PipeClose(i, c);
                else
                    PipeSendOp(sPipeChan[i].host == c ? sPipeChan[i].phone : sPipeChan[i].host, PIPE_DATA, ch, data + 3,
                               (uint16_t)(len - 3));
                break;
            }
        break;
    }
    }
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
    case MPMSG_C_MAIL:
        MailHandle(c, data, len);
        break;
    case MPMSG_C_PIPE:
        PipeHandle(c, data, len);
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
    PipeDropConn(c);
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
    MailLoad();
    EscLoad();
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
         || (sc->room < 0 && now - sc->connectedAt > LOBBY_TIMEOUT_S && !UsesPipes(i)))
        {
            MPNet_Log("%s timed out", sc->addr);
            Drop(i);
        }
    }

    for (i = 0; i < MAX_CONNS; i++)
        if (sConns[i] && MPConn_Flush(&sConns[i]->conn) < 0)
            Drop(i);
    MailSaveSoon(now);
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
