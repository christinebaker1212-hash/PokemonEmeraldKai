// Moemon Co-op: friends, for the phone (docs/COMPANION_APP.md "Pillar 4: friends", docs/PHONE_LINK.md "Phase 4").
// - Friends are the players this save has been in a room with: each save has a friend key (32 random bytes) it gives
//   everyone it plays with (MPR_FRIEND: save ID, friend key, name); "<save>.pals" keeps them (64) and this save's own
//   keys. A friend is a save ID and its key, never a name: the server has no accounts.
// - Presence: while the game is on a co-op server, every minute it puts its record in the server's mailbox under a key
//   only its friends can work out (PhoneProto_Derive(friend key, "presence")), sealed with the friend key, guarded by
//   a secret only this save has: who, when, where, doing what, and (sharing EVERYTHING) the room to join and the party.
//   ONLINE ONLY shares just that it's playing; HIDDEN takes the record away.
// - The phone's pipes: on a server, the game hosts a pipe for each paired phone (its ID from the phone's own key), so
//   the phone can link through the server when it's away from home: the same sealed link, frames in chunks.
// - An invite: the phone's JOIN asks the game to join a friend's room ("join" order).
#include <stdio.h>
#include "mp_version.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "main.h"
#include "pokemon.h"
#include "constants/characters.h"
#include "phone_proto.h"
#include "mp_net.h"
#include "mp_phone_net.h"
#include "mp_phone_int.h"
#include "tcg/integration.h"

extern char gPlatformSavePath[512];
bool8 MP_Lobby_ServerAddress(char *out, int len);
const char *MP_Lobby_RoomName(void);
const u8 *MP_Lobby_RoomKey(void);
bool8 MP_Lobby_InRoom(void);
void MP_Lobby_Connect(const char *address, bool8 isLocal);
void MP_Lobby_JoinRoomWithKey(const char *room, const u8 *key);
u32 MP_Phone_Count(void);
const u8 *MP_Phone_KeyOf(u32 i);   // mp_phone.c

#define PALS_MAGIC 0x534C4150   // "PALS"
#define PALS_MAX   64
#define MPR_FRIEND 19

struct PalV1   // (1.74.0's file)
{
    u8 id[PHONE_SAVEID_SIZE];
    u8 key[PHONE_KEY_SIZE];
    u8 name[PLAYER_NAME_LENGTH + 1];
    u8 pad[3];
    u32 seen;
};

struct PalsV1
{
    u32 magic, version;
    u8 key[PHONE_KEY_SIZE], secret[PHONE_KEY_SIZE];
    u8 share, pad[3];
    u32 count;
    struct PalV1 pals[PALS_MAX];
};

struct Pal
{
    u8 id[PHONE_SAVEID_SIZE];
    u8 key[PHONE_KEY_SIZE];
    u8 name[PLAYER_NAME_LENGTH + 1];
    u8 pad[3];
    u32 seen;
    u8 pub[PHONE_KEY_SIZE];       // its X25519 public key (all 0: not met since 1.74.0: can't trade by mail yet)
};

static struct
{
    u32 magic, version;
    u8 key[PHONE_KEY_SIZE];       // this save's friend key (its friends have it)
    u8 secret[PHONE_KEY_SIZE];    // this save's own (its records' owner lock): never shared
    u8 share;                     // 0 EVERYTHING, 1 ONLINE ONLY, 2 HIDDEN
    u8 pad[3];
    u32 count;
    struct Pal pals[PALS_MAX];
    u8 xsecret[PHONE_KEY_SIZE];   // this save's X25519 secret (pair keys with each friend: mail trades): never shared
    u8 xpublic[PHONE_KEY_SIZE];
} sPals;
static bool8 sPalsLoaded;
static bool8 sPlaying;   // (on the field since this save loaded)
static u32 sTimer;

static void PalsPath(char *out, size_t size)
{
    snprintf(out, size, "%s.pals", gPlatformSavePath);
}

static void PalsWrite(void)
{
    char path[600];
    FILE *f;
    if (MOEMON_DEV_BUILD)   // (a dev build never writes a save or its side files)
        return;
    PalsPath(path, sizeof(path));
    f = fopen(path, "wb");
    if (f != NULL)
    {
        fwrite(&sPals, sizeof(sPals), 1, f);
        fclose(f);
    }
}

void PhoneFriends_Load(void)
{
    char path[600];
    FILE *f;
    memset(&sPals, 0, sizeof(sPals));
    sPlaying = FALSE;
    PalsPath(path, sizeof(path));
    f = fopen(path, "rb");
    if (f != NULL)
    {
        static struct PalsV1 sOld;
        long size;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (size == (long)sizeof(sPals))
        {
            if (fread(&sPals, sizeof(sPals), 1, f) != 1 || sPals.magic != PALS_MAGIC || sPals.count > PALS_MAX)
                memset(&sPals, 0, sizeof(sPals));
        }
        else if (size == (long)sizeof(sOld) && fread(&sOld, sizeof(sOld), 1, f) == 1 && sOld.magic == PALS_MAGIC
              && sOld.count <= PALS_MAX)
        {
            // 1.74.0's: the same friends and keys, an X25519 pair of keys added (below)
            u32 i;
            sPals.magic = PALS_MAGIC;
            memcpy(sPals.key, sOld.key, sizeof(sPals.key));
            memcpy(sPals.secret, sOld.secret, sizeof(sPals.secret));
            sPals.share = sOld.share;
            sPals.count = sOld.count;
            for (i = 0; i < sOld.count; i++)
            {
                memcpy(sPals.pals[i].id, sOld.pals[i].id, sizeof(sPals.pals[i].id));
                memcpy(sPals.pals[i].key, sOld.pals[i].key, sizeof(sPals.pals[i].key));
                memcpy(sPals.pals[i].name, sOld.pals[i].name, sizeof(sPals.pals[i].name));
                sPals.pals[i].seen = sOld.pals[i].seen;
            }
        }
        fclose(f);
    }
    if (sPals.magic != PALS_MAGIC)
    {
        sPals.magic = PALS_MAGIC;
        PhoneProto_Random(sPals.key, sizeof(sPals.key));
        PhoneProto_Random(sPals.secret, sizeof(sPals.secret));
    }
    if (sPals.version != 2)
    {
        sPals.version = 2;
        PhoneProto_Random(sPals.xsecret, sizeof(sPals.xsecret));
        PhoneProto_PublicKey(sPals.xsecret, sPals.xpublic);
        PalsWrite();
    }
    sPalsLoaded = TRUE;
}

// ---------------------------------------------------------------------------------------- friends in a room

static void SendMine(void)
{
    u8 msg[1 + PHONE_SAVEID_SIZE + PHONE_KEY_SIZE + PLAYER_NAME_LENGTH + 1 + PHONE_KEY_SIZE];
    msg[0] = MPR_FRIEND;
    memcpy(msg + 1, MP_Phone_SaveId(), PHONE_SAVEID_SIZE);
    memcpy(msg + 1 + PHONE_SAVEID_SIZE, sPals.key, PHONE_KEY_SIZE);
    memcpy(msg + 1 + PHONE_SAVEID_SIZE + PHONE_KEY_SIZE, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    msg[1 + PHONE_SAVEID_SIZE + PHONE_KEY_SIZE + PLAYER_NAME_LENGTH] = EOS;
    memcpy(msg + 1 + PHONE_SAVEID_SIZE + PHONE_KEY_SIZE + PLAYER_NAME_LENGTH + 1, sPals.xpublic, PHONE_KEY_SIZE);
    MP_SendRelay(0xFF, msg, sizeof(msg));
}

// MPR_FRIEND from a player in the room: their save's ID, key and name (kept, the oldest pushed out past 64)
void PhoneFriends_OnRelay(u8 from, const u8 *data, u16 len)
{
    const u8 *id = data + 1, *key = data + 1 + PHONE_SAVEID_SIZE, *name = key + PHONE_KEY_SIZE;
    u32 i, oldest = 0;
    (void)from;
    if (!sPalsLoaded || len < 1 + PHONE_SAVEID_SIZE + PHONE_KEY_SIZE + 1 || memcmp(id, MP_Phone_SaveId(), PHONE_SAVEID_SIZE) == 0)
        return;
    for (i = 0; i < sPals.count; i++)
        if (memcmp(sPals.pals[i].id, id, PHONE_SAVEID_SIZE) == 0)
            break;
    if (i == sPals.count)
    {
        if (sPals.count < PALS_MAX)
            sPals.count++;
        else
            for (i = 1, oldest = 0; i < PALS_MAX; i++)
                if (sPals.pals[i].seen < sPals.pals[oldest].seen)
                    oldest = i;
        i = sPals.count <= PALS_MAX && i == sPals.count ? sPals.count - 1 : oldest;
        MPNet_Log("friends: met a new friend");
    }
    memcpy(sPals.pals[i].id, id, PHONE_SAVEID_SIZE);
    memcpy(sPals.pals[i].key, key, PHONE_KEY_SIZE);
    memset(sPals.pals[i].name, EOS, sizeof(sPals.pals[i].name));
    memcpy(sPals.pals[i].name, name, len - 1 - PHONE_SAVEID_SIZE - PHONE_KEY_SIZE > PLAYER_NAME_LENGTH
                                         ? PLAYER_NAME_LENGTH : len - 1 - PHONE_SAVEID_SIZE - PHONE_KEY_SIZE);
    sPals.pals[i].name[PLAYER_NAME_LENGTH] = EOS;
    if (len >= 1 + PHONE_SAVEID_SIZE + PHONE_KEY_SIZE + PLAYER_NAME_LENGTH + 1 + PHONE_KEY_SIZE)   // (1.74.1 on)
        memcpy(sPals.pals[i].pub, name + PLAYER_NAME_LENGTH + 1, PHONE_KEY_SIZE);
    sPals.pals[i].seen = (u32)time(NULL);
    PalsWrite();
}


// ------------------------------------------------------------------------------------- keys for mail trades

static struct Pal *FindPal(const u8 id[PHONE_SAVEID_SIZE])
{
    u32 i;
    for (i = 0; i < sPals.count; i++)
        if (memcmp(sPals.pals[i].id, id, PHONE_SAVEID_SIZE) == 0)
            return &sPals.pals[i];
    return NULL;
}

// The key only this save and that friend share (FALSE: not a friend, or not met since 1.74.1)
bool8 PhoneFriends_PairKey(const u8 id[PHONE_SAVEID_SIZE], u8 out[PHONE_KEY_SIZE])
{
    static const u8 zero[PHONE_KEY_SIZE];
    struct Pal *p = FindPal(id);
    if (!sPalsLoaded || p == NULL || memcmp(p->pub, zero, PHONE_KEY_SIZE) == 0)
        return FALSE;
    PhoneProto_PairKey(sPals.xsecret, p->pub, out);
    return TRUE;
}

const u8 *PhoneFriends_Name(const u8 id[PHONE_SAVEID_SIZE])
{
    struct Pal *p = FindPal(id);
    return p ? p->name : NULL;
}

// This save's inbox (offers from friends: anyone with its friend key may add; only it takes) and the key its trades'
// side secrets come from (the phone has both)
void PhoneFriends_Inbox(u8 key[16], u8 owner[16])
{
    PhoneProto_Derive(sPals.key, "inbox", key);
    PhoneProto_Derive(sPals.secret, "inbox", owner);
}

void PhoneFriends_TradeKey(u8 out[PHONE_KEY_SIZE])
{
    PhoneProto_Derive32(sPals.secret, "trade", out);
}

bool8 PhoneFriends_Loaded(void)
{
    return sPalsLoaded;
}

// ------------------------------------------------------------------------------------------------- presence

void PhoneFriends_Mail(u8 op, const u8 key[16], const u8 *secret16, u32 ttl, const u8 *data, u16 n)
{
    static u8 msg[MP_MAX_MSG];
    u32 at = 0;
    msg[at++] = op;
    memcpy(msg + at, key, 16);
    at += 16;
    if (secret16 != NULL)
    {
        memcpy(msg + at, secret16, 16);
        at += 16;
    }
    if (op == MAIL_PUT || op == MAIL_ADD)
    {
        msg[at++] = ttl & 0xFF, msg[at++] = ttl >> 8, msg[at++] = ttl >> 16, msg[at++] = ttl >> 24;
        msg[at++] = n & 0xFF, msg[at++] = n >> 8;
        memcpy(msg + at, data, n);
        at += n;
    }
    MPNet_ClientSend(MPMSG_C_MAIL, msg, (u16)at);
}

static const char *Doing(void)
{
    if (Tcg_IsActive())
        return "cards";
    if (gMain.inBattle)
        return gMpCoopBattle ? "coop" : "battle";
    if (MP_IsLocalFree())
        return "field";
    return "busy";
}

static void Publish(void)
{
    static char json[1400];
    static u8 sealed[1400 + PHONE_SEAL_EXTRA];
    u8 key[16], owner[16];
    char nameHex[2 * (PLAYER_NAME_LENGTH + 1) + 1], srv[96], roomKey[2 * MP_ROOM_KEY_SIZE + 1];
    size_t n = 0, k;
    u32 i;
    PhoneProto_Derive(sPals.key, "presence", key);
    PhoneProto_Derive(sPals.secret, "owner", owner);
    if (sPals.share == 2)
    {
        PhoneFriends_Mail(MAIL_DEL, key, owner, 0, NULL, 0);
        return;
    }
    for (k = 0; k < PLAYER_NAME_LENGTH && gSaveBlock2Ptr->playerName[k] != EOS; k++)
        snprintf(nameHex + 2 * k, 3, "%02X", gSaveBlock2Ptr->playerName[k]);
    nameHex[2 * k] = '\0';
    n += snprintf(json + n, sizeof(json) - n, "{\"n\":\"%s\",\"t\":%u", nameHex, (unsigned)time(NULL));
    if (sPals.share == 0)
    {
        if (!MP_Lobby_ServerAddress(srv, sizeof(srv)))
            srv[0] = '\0';   // (the online lobby)
        PhoneProto_ToHex(MP_Lobby_RoomKey(), MP_ROOM_KEY_SIZE, roomKey);
        n += snprintf(json + n, sizeof(json) - n, ",\"sec\":%u,\"reg\":%u,\"do\":\"%s\",\"srv\":\"%s\"",
                      gMapHeader.regionMapSectionId, MP_Region_Current(), Doing(), srv);
        if (MP_Lobby_InRoom())
        {
            const char *room = MP_Lobby_RoomName();
            n += snprintf(json + n, sizeof(json) - n, ",\"room\":\"");
            for (k = 0; room[k] && n < sizeof(json) - 4; k++)
                json[n++] = room[k] == '"' || room[k] == '\\' ? '_' : room[k];
            n += snprintf(json + n, sizeof(json) - n, "\",\"rkey\":\"%s\"", roomKey);
        }
        if (MP_InviteCode()[0])   // (a game hosted on that PC: the code friends type in CO-OP > JOIN)
            n += snprintf(json + n, sizeof(json) - n, ",\"code\":\"%s\"", MP_InviteCode());
        n += snprintf(json + n, sizeof(json) - n, ",\"party\":[");
        for (i = 0, k = 0; i < PARTY_SIZE && n < sizeof(json) - 32; i++)
        {
            u32 sp = GetMonData(&OwnParty()[i], MON_DATA_SPECIES);
            if (sp == SPECIES_NONE)
                continue;
            n += snprintf(json + n, sizeof(json) - n, "%s[%u,%u,%u]", k++ ? "," : "", (unsigned)sp,
                          (unsigned)GetMonData(&OwnParty()[i], MON_DATA_LEVEL), (unsigned)GetMonData(&OwnParty()[i], MON_DATA_IS_EGG));
        }
        n += snprintf(json + n, sizeof(json) - n, "]");
    }
    n += snprintf(json + n, sizeof(json) - n, "}");
    if (n >= sizeof(json))
        return;
    PhoneProto_Seal(sPals.key, 0, json, n, sealed);
    PhoneFriends_Mail(MAIL_PUT, key, owner, 600, sealed, (u16)(n + PHONE_SEAL_EXTRA));
}

// ------------------------------------------------------------------------------------------------- the pipes

#define PIPE_CHUNK 7900
static struct { u16 ch; u8 *buf; size_t len, cap; } sIn[PHONE_NET_CONNS];

static void PipeOut(u16 ch, const u8 *body, size_t len)
{
    static u8 msg[PIPE_CHUNK + 8];
    size_t at = 0;
    do
    {
        size_t n = len - at > PIPE_CHUNK ? PIPE_CHUNK : len - at;
        msg[0] = PIPE_DATA;
        msg[1] = ch & 0xFF;
        msg[2] = ch >> 8;
        msg[3] = at + n < len;   // (more follows)
        memcpy(msg + 4, body + at, n);
        MPNet_ClientSend(MPMSG_C_PIPE, msg, (u16)(4 + n));
        at += n;
    } while (at < len);
}

static void PipeClose(u16 ch)
{
    u8 msg[3] = {PIPE_CLOSE, ch & 0xFF, ch >> 8};
    if (MPNet_ClientStatus() == MPNET_CONNECTED)
        MPNet_ClientSend(MPMSG_C_PIPE, msg, sizeof(msg));
}

static void HostPipes(void)
{
    u32 i;
    for (i = 0; i < MP_Phone_Count(); i++)
    {
        u8 msg[17];
        msg[0] = PIPE_HOST;
        PhoneProto_Derive(MP_Phone_KeyOf(i), "pipe", msg + 1);
        MPNet_ClientSend(MPMSG_C_PIPE, msg, sizeof(msg));
    }
}

// MPMSG_S_PIPE / S_MAIL from the server
void PhoneFriends_OnServer(u8 type, const u8 *data, u16 len)
{
    u16 ch;
    u32 i;
    if (type == MPMSG_S_MAIL)
    {
        PhoneTrade_OnServer(data, len);
        return;
    }
    if (type != MPMSG_S_PIPE || len < 3)
        return;
    ch = data[1] | (data[2] << 8);
    switch (data[0])
    {
    case PIPE_JOINED:
        if (PhoneNet_PipeOpen(ch) < 0)
            PipeClose(ch);
        else
            MPNet_Log("phone: a phone linked through the server (channel %u)", ch);
        break;
    case PIPE_DATA:
    {
        // (a frame in chunks: gathered until the last)
        int slot = -1, freeSlot = -1;
        if (len < 4)
            return;
        for (i = 0; i < PHONE_NET_CONNS; i++)
        {
            if (sIn[i].ch == ch)
                slot = i;
            if (sIn[i].ch == 0 && freeSlot < 0)
                freeSlot = i;
        }
        if (slot < 0)
        {
            if (freeSlot < 0)
                return;
            slot = freeSlot;
            sIn[slot].ch = ch;
            sIn[slot].len = 0;
        }
        if (sIn[slot].len + len - 4 > PHONE_FRAME_MAX)
        {
            sIn[slot].ch = 0;
            return;
        }
        if (sIn[slot].len + len - 4 > sIn[slot].cap)
        {
            size_t cap = (sIn[slot].len + len - 4) * 2;
            u8 *p = realloc(sIn[slot].buf, cap);
            if (p == NULL)
                return;
            sIn[slot].buf = p;
            sIn[slot].cap = cap;
        }
        memcpy(sIn[slot].buf + sIn[slot].len, data + 4, len - 4);
        sIn[slot].len += len - 4;
        if (!data[3])
        {
            PhoneNet_PipeIn(ch, sIn[slot].buf, sIn[slot].len);
            sIn[slot].ch = 0;
            sIn[slot].len = 0;
        }
        break;
    }
    case PIPE_CLOSED:
        PhoneNet_PipeClosed(ch);
        break;
    }
}

// ------------------------------------------------------------------------------------------------ every frame

void PhoneFriends_Update(void)
{
    static int sWasConnected;
    static u32 sRoomTimer;
    int connected = MPNet_ClientStatus() == MPNET_CONNECTED;
    // (a save playing: on the field once since it loaded; the title screen has no name, place or party yet)
    if (MP_IsInOverworld())
    {
        if (!sPlaying)
            sTimer = 60 * 60 - 120;   // (the first record soon after)
        sPlaying = TRUE;
    }
    gPhonePipeOut = PipeOut;
    gPhonePipeClose = PipeClose;
    if (!sPalsLoaded)
        return;
    if (connected && !sWasConnected)
    {
        sTimer = 60 * 60 - 120;   // (presence soon after connecting)
        HostPipes();
    }
    sWasConnected = connected;
    if (!connected)
        return;
    if (++sTimer >= 60 * 60)
    {
        sTimer = 0;
        HostPipes();   // (phones paired since; the server keeps one entry each)
        if (sPlaying)
            Publish();
    }
    if (MP_Lobby_InRoom() && gMpLocalId >= 0 && ++sRoomTimer >= 60 * 10)
    {
        sRoomTimer = 0;
        SendMine();   // (every 10 seconds in a room: whoever joined since meets us)
    }
}

// --------------------------------------------------------------------------------------------- the snapshot

void PhoneFriends_Snapshot(struct Buf *b)
{
    char srv[96], hex[2 * PHONE_KEY_SIZE + 1];
    u32 i;
    if (!sPalsLoaded)
        return;
    if (!MP_Lobby_ServerAddress(srv, sizeof(srv)))
        snprintf(srv, sizeof(srv), "%s", gMpConfig.lastRoomAddr[0] && strcmp(gMpConfig.lastRoomAddr, "home") != 0
                                             ? gMpConfig.lastRoomAddr : MP_OnlineLobbyAddress());
    BufAdd(b, ",\"friends\":{\"share\":%u,\"srv\":\"%s\",\"online\":%u,\"list\":[", sPals.share, srv,
           MPNet_ClientStatus() == MPNET_CONNECTED);
    for (i = 0; i < sPals.count; i++)
    {
        PhoneProto_ToHex(sPals.pals[i].id, PHONE_SAVEID_SIZE, hex);
        BufAdd(b, "%s{\"id\":\"%s\"", i ? "," : "", hex);
        PhoneProto_ToHex(sPals.pals[i].key, PHONE_KEY_SIZE, hex);
        BufAdd(b, ",\"key\":\"%s\",\"n\":", hex);
        BufText(b, sPals.pals[i].name, PLAYER_NAME_LENGTH);
        BufAdd(b, ",\"seen\":%u", (unsigned)sPals.pals[i].seen);
        {
            u8 pk[PHONE_KEY_SIZE];
            if (PhoneFriends_PairKey(sPals.pals[i].id, pk))
            {
                PhoneProto_ToHex(pk, PHONE_KEY_SIZE, hex);
                BufAdd(b, ",\"pk\":\"%s\"", hex);   // (mail trades: the phone seals offers with it)
            }
        }
        BufAdd(b, "}");
    }
    {
        u8 tk[PHONE_KEY_SIZE], inbox[16], owner[16];
        PhoneProto_ToHex(MP_Phone_SaveId(), PHONE_SAVEID_SIZE, hex);
        BufAdd(b, "],\"me\":\"%s\"", hex);
        PhoneFriends_TradeKey(tk);
        PhoneProto_ToHex(tk, PHONE_KEY_SIZE, hex);
        BufAdd(b, ",\"tk\":\"%s\"", hex);
        PhoneFriends_Inbox(inbox, owner);
        PhoneProto_ToHex(inbox, 16, hex);
        BufAdd(b, ",\"inbox\":\"%s\"", hex);
        PhoneProto_ToHex(owner, 16, hex);
        BufAdd(b, ",\"inown\":\"%s\"", hex);
    }
    BufAdd(b, "}");
}

// ------------------------------------------------------------------------------------------------------ orders

// "share": how much friends see (0 EVERYTHING, 1 ONLINE ONLY, 2 HIDDEN); "join": a friend's room
const char *PhoneFriends_Apply(const char *op, const char *json)
{
    if (strcmp(op, "share") == 0)
    {
        long long level = 0;
        if (!JsonNum(json, "level", &level) || level < 0 || level > 2)
            return "bad";
        sPals.share = (u8)level;
        PalsWrite();
        if (MPNet_ClientStatus() == MPNET_CONNECTED)
            Publish();
        return NULL;
    }
    if (strcmp(op, "join") == 0)
    {
        char srv[96], room[40], keyHex[2 * MP_ROOM_KEY_SIZE + 4];
        u8 key[MP_ROOM_KEY_SIZE];
        if (!JsonStr(json, "room", room, sizeof(room)) || !JsonStr(json, "rkey", keyHex, sizeof(keyHex))
         || !PhoneProto_FromHex(keyHex, key, MP_ROOM_KEY_SIZE))
            return "bad";
        if (!JsonStr(json, "srv", srv, sizeof(srv)) || srv[0] == '\0')
            snprintf(srv, sizeof(srv), "%s", MP_OnlineLobbyAddress());
        if (MP_Lobby_InRoom())
            return "inroom";
        MP_Lobby_Connect(srv, FALSE);
        MP_Lobby_JoinRoomWithKey(room, key);
        MP_Events_QueueToast("Joining your friend's room...");
        return NULL;
    }
    if (strncmp(op, "tt", 2) == 0 || strncmp(op, "tcg", 3) == 0 || strncmp(op, "ygo", 3) == 0)
        return PhoneCards_Apply(op, json);   // (the card games: mp_phone_cards.c)
    return PhoneTrade_Apply(op, json);
}
