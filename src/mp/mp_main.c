// Moemon Co-op: client core (config, connection, player roster, local state)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mp_net.h"
#include "global.h"
#include "mp.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "script.h"
#include "palette.h"
#include "field_player_avatar.h"
#include "event_object_movement.h"
#include "constants/characters.h"
#include "string_util.h"

#define CONFIG_FILE "moemon_coop.ini"
#define RECONNECT_FRAMES (60 * 8)
#define STATE_HEARTBEAT_FRAMES 30

struct MpConfig gMpConfig;
struct MpPlayer gMpPlayers[MP_MAX_PLAYERS];
s8 gMpLocalId = -1;

static bool8 sInitDone;
static bool8 sWantConnect;
static bool8 sHelloSent;
static u32 sReconnectTimer;
static u32 sHeartbeat;
static struct MpState sLastSentState;
static u16 sStateSeq;
static u8 sEmoteCounter;
static u8 sEmoteKind;
static char sStatus[96] = "OFFLINE";
static u8 sMsgBuf[MP_MAX_MSG + 8];

// ---------------------------------------------------------------------------
// Text helpers (ASCII <-> game charset)

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen)
{
    int i, n = 0;
    for (i = 0; src[i] && n < maxLen; i++)
    {
        char c = src[i];
        u8 g;
        if (c >= 'A' && c <= 'Z') g = CHAR_A + (c - 'A');
        else if (c >= 'a' && c <= 'z') g = CHAR_a + (c - 'a');
        else if (c >= '0' && c <= '9') g = CHAR_0 + (c - '0');
        else if (c == ' ') g = CHAR_SPACE;
        else if (c == '!') g = CHAR_EXCL_MARK;
        else if (c == '?') g = CHAR_QUESTION_MARK;
        else if (c == '.') g = CHAR_PERIOD;
        else if (c == '-') g = CHAR_HYPHEN;
        else continue;
        dst[n++] = g;
    }
    dst[n] = EOS;
}

void MP_GameToAscii(char *dst, const u8 *src, int maxLen)
{
    int i;
    for (i = 0; i < maxLen && src[i] != EOS; i++)
    {
        u8 g = src[i];
        if (g >= CHAR_A && g <= CHAR_Z) dst[i] = 'A' + (g - CHAR_A);
        else if (g >= CHAR_a && g <= CHAR_z) dst[i] = 'a' + (g - CHAR_a);
        else if (g >= CHAR_0 && g <= CHAR_9) dst[i] = '0' + (g - CHAR_0);
        else dst[i] = ' ';
    }
    dst[i] = 0;
}

// ---------------------------------------------------------------------------
// Config file

static char *Trim(char *s)
{
    char *e;
    while (*s && isspace((unsigned char)*s))
        s++;
    e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1]))
        *--e = 0;
    return s;
}

static int ParseChoice(const char *v, const char *(*nameFn)(u8), int count)
{
    int i;
    char up[64];
    for (i = 0; v[i] && i < 63; i++)
        up[i] = (v[i] == '_') ? ' ' : toupper((unsigned char)v[i]);
    up[i] = 0;
    for (i = 0; i < count; i++)
        if (strcmp(up, nameFn(i)) == 0)
            return i;
    return atoi(v) < count ? atoi(v) : 0;
}

static void WriteDefaultConfig(void)
{
    FILE *f = fopen(CONFIG_FILE, "w");
    if (!f)
        return;
    fprintf(f,
        "; Moemon Emerald Kai Co-op settings\n"
        "; enabled = 1 to play online\n"
        "enabled = 1\n"
        "; host = 1 makes this game run the server too (other players connect to your IP)\n"
        "host = 0\n"
        "; address of the server (the host's IP or domain name)\n"
        "server = 127.0.0.1\n"
        "port = %d\n"
        "; name shown above your head (empty = your trainer name)\n"
        "name =\n"
        "; sprite = default, brendan, may, rs_brendan, rs_may or custom\n"
        "sprite = default\n"
        "; custom_sprite = path to a 144x32 or 288x32 PNG (used when sprite = custom)\n"
        "custom_sprite =\n"
        "; skin = default, porcelain, light, tan, brown, dark, deep\n"
        "skin = default\n"
        "; outfit / accent = default, red, blue, green, yellow, purple, pink, orange, black, white, teal\n"
        "outfit = default\n"
        "accent = default\n"
        "; how many tiles a player in Trainer Mode can see\n"
        "trainer_range = 4\n",
        MP_DEFAULT_PORT);
    fclose(f);
}

static void LoadConfig(void)
{
    FILE *f;
    char line[512];
    const char *path = getenv("MOEMON_COOP_CONFIG") ? getenv("MOEMON_COOP_CONFIG") : CONFIG_FILE;

    memset(&gMpConfig, 0, sizeof(gMpConfig));
    gMpConfig.enabled = TRUE;
    gMpConfig.port = MP_DEFAULT_PORT;
    gMpConfig.trainerRange = 4;
    strcpy(gMpConfig.server, "127.0.0.1");

    f = fopen(path, "r");
    if (!f)
    {
        if (!getenv("MOEMON_COOP_CONFIG"))
            WriteDefaultConfig();
        return;
    }
    while (fgets(line, sizeof(line), f))
    {
        char *eq, *key, *val;
        if (line[0] == ';' || line[0] == '#')
            continue;
        eq = strchr(line, '=');
        if (!eq)
            continue;
        *eq = 0;
        key = Trim(line);
        val = Trim(eq + 1);
        if (strcmp(key, "enabled") == 0) gMpConfig.enabled = atoi(val) != 0;
        else if (strcmp(key, "host") == 0) gMpConfig.host = atoi(val) != 0;
        else if (strcmp(key, "server") == 0) snprintf(gMpConfig.server, sizeof(gMpConfig.server), "%s", val);
        else if (strcmp(key, "port") == 0) gMpConfig.port = atoi(val) > 0 ? atoi(val) : MP_DEFAULT_PORT;
        else if (strcmp(key, "name") == 0) snprintf(gMpConfig.name, sizeof(gMpConfig.name), "%s", val);
        else if (strcmp(key, "sprite") == 0) gMpConfig.app.base = ParseChoice(val, MP_Avatar_BaseName, MP_BASE_COUNT);
        else if (strcmp(key, "custom_sprite") == 0) snprintf(gMpConfig.customSprite, sizeof(gMpConfig.customSprite), "%s", val);
        else if (strcmp(key, "skin") == 0) gMpConfig.app.skin = ParseChoice(val, MP_Avatar_SkinName, MP_SKIN_COUNT);
        else if (strcmp(key, "outfit") == 0) gMpConfig.app.outfit = ParseChoice(val, MP_Avatar_ColorName, MP_COLOR_COUNT);
        else if (strcmp(key, "accent") == 0) gMpConfig.app.accent = ParseChoice(val, MP_Avatar_ColorName, MP_COLOR_COUNT);
        else if (strcmp(key, "trainer_range") == 0)
        {
            int r = atoi(val);
            gMpConfig.trainerRange = (r < 1) ? 1 : (r > 8 ? 8 : r);
        }
    }
    fclose(f);
}

static void LowerName(char *dst, const char *src)
{
    int i;
    for (i = 0; src[i]; i++)
        dst[i] = (src[i] == ' ') ? '_' : tolower((unsigned char)src[i]);
    dst[i] = 0;
}

void MP_SaveConfig(void)
{
    FILE *f;
    char sprite[32], skin[32], outfit[32], accent[32];
    const char *path = getenv("MOEMON_COOP_CONFIG") ? getenv("MOEMON_COOP_CONFIG") : CONFIG_FILE;

    LowerName(sprite, MP_Avatar_BaseName(gMpConfig.app.base));
    LowerName(skin, MP_Avatar_SkinName(gMpConfig.app.skin));
    LowerName(outfit, MP_Avatar_ColorName(gMpConfig.app.outfit));
    LowerName(accent, MP_Avatar_ColorName(gMpConfig.app.accent));
    f = fopen(path, "w");
    if (!f)
        return;
    fprintf(f,
        "; Moemon Emerald Kai Co-op settings\n"
        "enabled = %d\nhost = %d\nserver = %s\nport = %d\nname = %s\n"
        "; sprite = default, brendan, may, rs_brendan, rs_may or custom\n"
        "sprite = %s\ncustom_sprite = %s\n"
        "; skin = default, porcelain, light, tan, brown, dark, deep\n"
        "skin = %s\n"
        "; outfit / accent = default, red, blue, green, yellow, purple, pink, orange, black, white, teal\n"
        "outfit = %s\naccent = %s\ntrainer_range = %d\n",
        gMpConfig.enabled, gMpConfig.host, gMpConfig.server, gMpConfig.port, gMpConfig.name,
        sprite, gMpConfig.customSprite, skin, outfit, accent, gMpConfig.trainerRange);
    fclose(f);
}

// ---------------------------------------------------------------------------
// Serialization

static void PutU16(u8 *p, u16 v) { p[0] = v & 0xFF; p[1] = v >> 8; }
static void PutU32(u8 *p, u32 v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }
static u16 GetU16(const u8 *p) { return p[0] | (p[1] << 8); }
static u32 GetU32(const u8 *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24); }

static void PackState(const struct MpState *s, u8 *p)
{
    p[0] = s->mapGroup; p[1] = s->mapNum;
    PutU16(p + 2, (u16)s->x); PutU16(p + 4, (u16)s->y);
    p[6] = s->facing; p[7] = s->elevation; p[8] = s->avatarMode; p[9] = s->speed;
    p[10] = s->flags; p[11] = s->topLevel; p[12] = s->partyCount; p[13] = s->emote;
    PutU16(p + 14, s->seq);
}

static void UnpackState(const u8 *p, struct MpState *s)
{
    s->mapGroup = p[0]; s->mapNum = p[1];
    s->x = (s16)GetU16(p + 2); s->y = (s16)GetU16(p + 4);
    s->facing = p[6]; s->elevation = p[7]; s->avatarMode = p[8]; s->speed = p[9];
    s->flags = p[10]; s->topLevel = p[11]; s->partyCount = p[12]; s->emote = p[13];
    s->seq = GetU16(p + 14);
}

static void PackAppearance(const struct MpAppearance *a, u8 *p)
{
    p[0] = a->base; p[1] = a->skin; p[2] = a->outfit; p[3] = a->accent;
    p[4] = a->gender; p[5] = a->hasCustom; p[6] = 0; p[7] = 0;
}

static void UnpackAppearance(const u8 *p, struct MpAppearance *a)
{
    a->base = p[0] < MP_BASE_COUNT ? p[0] : 0;
    a->skin = p[1] < MP_SKIN_COUNT ? p[1] : 0;
    a->outfit = p[2] < MP_COLOR_COUNT ? p[2] : 0;
    a->accent = p[3] < MP_COLOR_COUNT ? p[3] : 0;
    a->gender = p[4] ? FEMALE : MALE;
    a->hasCustom = p[5];
}

static void GetLocalName(u8 *dst)
{
    if (gMpConfig.name[0])
        MP_AsciiToGame(dst, gMpConfig.name, PLAYER_NAME_LENGTH);
    else
        StringCopyN(dst, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    dst[PLAYER_NAME_LENGTH] = EOS;
}

static void BuildHello(u8 *p)
{
    struct MpAppearance app = gMpConfig.app;
    u8 name[MP_NAME_LEN + 1];
    int i;

    memset(name, EOS, sizeof(name));
    GetLocalName(name);
    app.gender = gSaveBlock2Ptr->playerGender;
    app.hasCustom = MP_Avatar_LocalCustomBlob() != NULL;
    PutU16(p, MP_PROTOCOL_VERSION);
    for (i = 0; i < MP_NAME_LEN; i++)
        p[2 + i] = name[i];
    PutU32(p + 2 + MP_NAME_LEN, GetU32(gSaveBlock2Ptr->playerTrainerId));
    PackAppearance(&app, p + 2 + MP_NAME_LEN + 4);
}

// ---------------------------------------------------------------------------
// Local state

bool8 MP_IsInOverworld(void)
{
    return gMain.callback2 == CB2_Overworld;
}

bool8 MP_IsLocalFree(void)
{
    if (!MP_IsInOverworld() || gPaletteFade.active)
        return FALSE;
    if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled())
        return FALSE;
    if (gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return FALSE;
    if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_FORCED_MOVE)
        return FALSE;
    return TRUE;
}

u8 MP_GetPartyTopLevel(void)
{
    u8 i, best = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (species != SPECIES_NONE && species != SPECIES_EGG)
        {
            u8 lvl = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
            if (lvl > best)
                best = lvl;
        }
    }
    return best;
}

void MP_TriggerLocalEmote(u8 kind)
{
    sEmoteKind = kind;
    sEmoteCounter = (sEmoteCounter + 1) & 0xF;
}

static struct MpState sLastLocalState;

void MP_GetLocalState(struct MpState *s)
{
    *s = sLastLocalState;
}

static void CollectLocalState(struct MpState *s)
{
    *s = sLastLocalState;
    s->flags &= ~(MPF_FREE | MPF_IN_OVERWORLD | MPF_IN_BATTLE | MPF_TRAINER_MODE | MPF_ENGAGED);

    if (MP_IsInOverworld() && gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT
     && gObjectEvents[gPlayerAvatar.objectEventId].active)
    {
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        u8 flags = gPlayerAvatar.flags;

        s->mapGroup = gSaveBlock1Ptr->location.mapGroup;
        s->mapNum = gSaveBlock1Ptr->location.mapNum;
        s->x = player->currentCoords.x;
        s->y = player->currentCoords.y;
        s->facing = player->facingDirection;
        s->elevation = player->currentElevation;
        if (flags & PLAYER_AVATAR_FLAG_MACH_BIKE)
        {
            s->avatarMode = MP_AVATAR_MACH_BIKE;
            s->speed = gPlayerAvatar.bikeSpeed >= 2 ? MP_SPEED_FASTER : MP_SPEED_FAST;
        }
        else if (flags & PLAYER_AVATAR_FLAG_ACRO_BIKE)
        {
            s->avatarMode = MP_AVATAR_ACRO_BIKE;
            s->speed = MP_SPEED_FAST;
        }
        else if (flags & PLAYER_AVATAR_FLAG_SURFING)
        {
            s->avatarMode = MP_AVATAR_SURF;
            s->speed = MP_SPEED_FAST;
        }
        else if (flags & PLAYER_AVATAR_FLAG_UNDERWATER)
        {
            s->avatarMode = MP_AVATAR_UNDERWATER;
            s->speed = MP_SPEED_WALK;
        }
        else
        {
            s->avatarMode = MP_AVATAR_FOOT;
            s->speed = (flags & PLAYER_AVATAR_FLAG_DASH) ? MP_SPEED_RUN : MP_SPEED_WALK;
        }
        s->flags |= MPF_IN_OVERWORLD;
        if (MP_IsLocalFree())
            s->flags |= MPF_FREE;
    }
    if (MP_Battle_IsActive())
        s->flags |= MPF_IN_BATTLE;
    if (MP_Trainer_IsActive())
        s->flags |= MPF_TRAINER_MODE;
    if (MP_Trainer_IsEngaged() || MP_Battle_IsActive())
        s->flags |= MPF_ENGAGED;
    s->topLevel = MP_GetPartyTopLevel();
    s->partyCount = gPlayerPartyCount;
    s->emote = (sEmoteCounter << 4) | sEmoteKind;
    sLastLocalState = *s;
}

static bool8 StateDiffers(const struct MpState *a, const struct MpState *b)
{
    return a->mapGroup != b->mapGroup || a->mapNum != b->mapNum || a->x != b->x || a->y != b->y
        || a->facing != b->facing || a->elevation != b->elevation || a->avatarMode != b->avatarMode
        || a->speed != b->speed || a->flags != b->flags || a->topLevel != b->topLevel
        || a->emote != b->emote;
}

static void SendLocalState(bool8 force)
{
    struct MpState s;
    u8 buf[MP_WIRESTATE_SIZE];

    CollectLocalState(&s);
    if (!force && !StateDiffers(&s, &sLastSentState) && ++sHeartbeat < STATE_HEARTBEAT_FRAMES)
        return;
    sHeartbeat = 0;
    s.seq = ++sStateSeq;
    PackState(&s, buf);
    MPNet_ClientSend(MPMSG_C_STATE, buf, sizeof(buf));
    sLastSentState = s;
}

void MP_SendAppearance(void)
{
    u8 hello[MP_HELLO_SIZE];
    if (MPNet_ClientStatus() != MPNET_CONNECTED || gMpLocalId < 0)
        return;
    if (MP_Avatar_LocalCustomBlob())
        MPNet_ClientSend(MPMSG_C_CUSTOM_GFX, MP_Avatar_LocalCustomBlob(), MP_CUSTOM_BLOB_SIZE);
    BuildHello(hello);
    MPNet_ClientSend(MPMSG_C_INFO, hello, sizeof(hello));
}

void MP_SendRelay(u8 target, const void *data, u16 len)
{
    u8 *buf = sMsgBuf;
    if (MPNet_ClientStatus() != MPNET_CONNECTED || len + 1 > MP_MAX_MSG)
        return;
    buf[0] = target;
    memcpy(buf + 1, data, len);
    MPNet_ClientSend(MPMSG_C_RELAY, buf, len + 1);
}

// ---------------------------------------------------------------------------
// Roster

static void ClearPlayer(u8 id)
{
    free(gMpPlayers[id].customGfx);
    memset(&gMpPlayers[id], 0, sizeof(gMpPlayers[id]));
}

static void HandlePlayerLeft(u8 id)
{
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return;
    MP_Overworld_OnPlayerLeft(id);
    MP_Trainer_OnPlayerLeft(id);
    MP_Battle_OnPlayerLeft(id);
    ClearPlayer(id);
}

static void ResetRoster(void)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        HandlePlayerLeft(i);
    gMpLocalId = -1;
}

u8 MP_ConnectedCount(void)
{
    u8 i, n = (gMpLocalId >= 0) ? 1 : 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (gMpPlayers[i].present)
            n++;
    return n;
}

bool8 MP_IsConnected(void)
{
    return MPNet_ClientStatus() == MPNET_CONNECTED && gMpLocalId >= 0;
}

const char *MP_StatusText(void)
{
    return sStatus;
}

static void HandleMessage(u8 type, u8 *data, u16 len)
{
    u8 id;
    switch (type)
    {
    case MPMSG_S_WELCOME:
        if (len < 1)
            return;
        gMpLocalId = data[0];
        snprintf(sStatus, sizeof(sStatus), "ONLINE (PLAYER %d)", gMpLocalId + 1);
        MPNet_Log("joined server as player %d", gMpLocalId + 1);
        if (MP_Avatar_LocalCustomBlob())
            MPNet_ClientSend(MPMSG_C_CUSTOM_GFX, MP_Avatar_LocalCustomBlob(), MP_CUSTOM_BLOB_SIZE);
        SendLocalState(TRUE);
        break;
    case MPMSG_S_REJECT:
        snprintf(sStatus, sizeof(sStatus), (len && data[0] == MP_REJECT_FULL) ? "SERVER FULL" : "VERSION MISMATCH");
        MPNet_Log("server rejected us: %s", sStatus);
        MPNet_ClientClose();
        sReconnectTimer = RECONNECT_FRAMES * 4;
        break;
    case MPMSG_S_PLAYER_INFO:
    {
        struct MpPlayer *p;
        int i;
        char ascii[16];
        if (len < 1 + MP_HELLO_SIZE || data[0] >= MP_MAX_PLAYERS || data[0] == gMpLocalId)
            return;
        id = data[0];
        p = &gMpPlayers[id];
        if (!p->present)
            memset(p, 0, sizeof(*p));
        p->present = TRUE;
        p->hasInfo = TRUE;
        for (i = 0; i < MP_NAME_LEN; i++)
            p->name[i] = data[3 + i];
        p->name[MP_NAME_LEN] = EOS;
        p->otId = GetU32(data + 3 + MP_NAME_LEN);
        UnpackAppearance(data + 3 + MP_NAME_LEN + 4, &p->app);
        p->appearanceVersion++;
        MP_GameToAscii(ascii, p->name, MP_NAME_LEN);
        MPNet_Log("player %d is %s", id + 1, ascii);
        MP_Overworld_OnAppearanceChanged(id);
        break;
    }
    case MPMSG_S_PLAYER_LEAVE:
        if (len >= 1)
            HandlePlayerLeft(data[0]);
        break;
    case MPMSG_S_STATE:
    {
        struct MpState old;
        struct MpPlayer *p;
        if (len < 1 + MP_WIRESTATE_SIZE || data[0] >= MP_MAX_PLAYERS || data[0] == gMpLocalId)
            return;
        id = data[0];
        p = &gMpPlayers[id];
        if (!p->present)
            memset(p, 0, sizeof(*p));
        p->present = TRUE;
        old = p->state;
        if (!p->hasState)
            memset(&old, 0xFF, sizeof(old));
        UnpackState(data + 1, &p->state);
        if (old.avatarMode != p->state.avatarMode)
            p->appearanceVersion++;
        p->hasState = TRUE;
        MP_Overworld_OnRemoteState(id, &old);
        break;
    }
    case MPMSG_S_CUSTOM_GFX:
        if (len < 1 + MP_CUSTOM_BLOB_SIZE || data[0] >= MP_MAX_PLAYERS || data[0] == gMpLocalId)
            return;
        id = data[0];
        if (!gMpPlayers[id].customGfx)
            gMpPlayers[id].customGfx = malloc(MP_CUSTOM_BLOB_SIZE);
        memcpy(gMpPlayers[id].customGfx, data + 1, MP_CUSTOM_BLOB_SIZE);

        gMpPlayers[id].present = TRUE;
        gMpPlayers[id].appearanceVersion++;
        MP_Overworld_OnAppearanceChanged(id);
        break;
    case MPMSG_S_RELAY:
        if (len < 2 || data[0] >= MP_MAX_PLAYERS)
            return;
        if (data[1] >= MPR_LINK_PLAYER)
            MP_Battle_OnRelay(data[0], data + 1, len - 1);
        else
            MP_Trainer_OnRelay(data[0], data + 1, len - 1);
        break;
    }
}

// ---------------------------------------------------------------------------

void MP_Init(void)
{
    if (sInitDone)
        return;
    sInitDone = TRUE;
    LoadConfig();
    MPNet_Init();
    MP_Avatar_Init();
    if (gMpConfig.enabled && gMpConfig.host)
    {
        if (MPServer_Start(gMpConfig.port) != 0)
            MPNet_Log("could not start server: %s", MPNet_LastError());
    }
    sWantConnect = gMpConfig.enabled;
    snprintf(sStatus, sizeof(sStatus), gMpConfig.enabled ? "WAITING TO CONNECT" : "OFFLINE (DISABLED)");
}

static void TryConnect(void)
{
    const char *host = gMpConfig.server;
    MPNet_Log("connecting to %s:%d", host, gMpConfig.port);
    snprintf(sStatus, sizeof(sStatus), "CONNECTING...");
    if (MPNet_Connect(host, gMpConfig.port) != 0)
    {
        snprintf(sStatus, sizeof(sStatus), "CANNOT REACH SERVER");
        MPNet_Log("%s", MPNet_LastError());
        // Name lookups can be slow, so retry those much less often
        sReconnectTimer = strstr(MPNet_LastError(), "resolve") ? RECONNECT_FRAMES * 8 : RECONNECT_FRAMES;
    }
    sHelloSent = FALSE;
}

void MP_FrameUpdate(void)
{
    u8 type;
    u16 len;
    int guard;

    if (!sInitDone)
        MP_Init();

    MPServer_Poll();

    if (!sWantConnect)
        return;

    // Wait until a save is loaded so we know our name and gender
    if (MPNet_ClientStatus() == MPNET_DISCONNECTED)
    {
        if (gMpLocalId >= 0 || sHelloSent)
        {
            ResetRoster();
            sHelloSent = FALSE;
            snprintf(sStatus, sizeof(sStatus), "DISCONNECTED - RETRYING");
            if (sReconnectTimer == 0)
                sReconnectTimer = RECONNECT_FRAMES;
        }
        if (!MP_IsInOverworld())
            return;
        if (sReconnectTimer > 0)
        {
            sReconnectTimer--;
            return;
        }
        TryConnect();
        return;
    }

    for (guard = 0; guard < 512 && MPNet_ClientPoll(&type, sMsgBuf, &len); guard++)
        HandleMessage(type, sMsgBuf, len);

    if (MPNet_ClientStatus() == MPNET_CONNECTED && !sHelloSent)
    {
        u8 hello[MP_HELLO_SIZE];
        MP_Avatar_Init();
        BuildHello(hello);
        MPNet_ClientSend(MPMSG_C_HELLO, hello, sizeof(hello));
        sHelloSent = TRUE;
        snprintf(sStatus, sizeof(sStatus), "JOINING...");
    }

    MP_Battle_FrameUpdate();
    if (gMpLocalId >= 0)
        SendLocalState(FALSE);

    // flush now so input latency stays low
    MPNet_ClientPoll(&type, sMsgBuf, &len) ? HandleMessage(type, sMsgBuf, len) : (void)0;
}
