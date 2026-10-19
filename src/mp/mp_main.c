// Moemon Co-op: client core (config, lobby session, player roster, local state)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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
#define STATE_HEARTBEAT_FRAMES 30
#define PING_FRAMES (60 * 5)
#define CONNECT_TIMEOUT_FRAMES (60 * 6)
#define RECONNECT_FRAMES (60 * 3)

_Static_assert(MP_BLOB_KINDS == ARRAY_COUNT(((struct MpPlayer *)0)->blobBuf), "update MpPlayer blob arrays");

struct MpConfig gMpConfig;
struct MpPlayer gMpPlayers[MP_MAX_PLAYERS];
s8 gMpLocalId = -1;

static bool8 sInitDone;
static bool8 sHelloSent;
static u32 sHeartbeat;
static u32 sPingTimer;
static struct MpState sLastSentState;
static u16 sStateSeq;
static u8 sEmoteCounter;
static u8 sEmoteKind;
static char sStatus[96] = "OFFLINE";
static u8 sMsgBuf[MP_MAX_MSG + 8];

// ---------------------------------------------------------------------------
// Lobby session

enum {
    REQ_NONE,
    REQ_CREATE,
    REQ_JOIN,
};

static struct
{
    u8 state;                 // MP_LOBBY_*
    u8 error;                 // MP_LOBBY_ERR_*
    char host[128];
    int port;
    bool8 isLocal;            // our own LAN server
    u32 timer;
    // room we are in (or asking for)
    u8 request;
    char room[MP_ROOM_NAME_MAX + 1];
    u8 key[MP_ROOM_KEY_SIZE];
    bool8 locked;
    bool8 hidden;
    bool8 wantRoom;           // stay in this room: rejoin after a dropped connection
    bool8 reconnecting;
    // room list
    struct MpLanRoom list[MP_LIST_MAX];
    u8 listCount;
    bool8 listReady;
} sLobby;

// ---------------------------------------------------------------------------
// Text helpers (ASCII <-> game charset)

static u8 AsciiCharToGame(char c)
{
    if (c >= 'A' && c <= 'Z') return CHAR_A + (c - 'A');
    if (c >= 'a' && c <= 'z') return CHAR_a + (c - 'a');
    if (c >= '0' && c <= '9') return CHAR_0 + (c - '0');
    switch (c)
    {
    case ' ': return CHAR_SPACE;
    case '!': return CHAR_EXCL_MARK;
    case '?': return CHAR_QUESTION_MARK;
    case '.': return CHAR_PERIOD;
    case '-': return CHAR_HYPHEN;
    case '/': return CHAR_SLASH;
    case ',': return CHAR_COMMA;
    case ':': return CHAR_COLON;
    case '\'': return CHAR_SGL_QUOTE_RIGHT;
    case '(': return CHAR_LEFT_PAREN;
    case ')': return CHAR_RIGHT_PAREN;
    case '&': return CHAR_AMPERSAND;
    case '+': return CHAR_PLUS;
    case '=': return CHAR_EQUALS;
    case '%': return CHAR_PERCENT;
    case '"': return CHAR_DBL_QUOTE_LEFT;
    case '\n': return CHAR_NEWLINE;
    case '\f': return CHAR_PROMPT_CLEAR;   // next page (wait for A)
    case '_': return CHAR_SPACE;
    case '<': return CHAR_LEFT_ARROW;
    case '>': return CHAR_RIGHT_ARROW;
    case '$': return CHAR_CURRENCY;
    case '~': return CHAR_e_ACUTE;      // as in POK~MON
    }
    return 0xFF;
}

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen)
{
    int i, n = 0;
    for (i = 0; src[i] && n < maxLen; i++)
    {
        u8 g = AsciiCharToGame(src[i]);
        if (g != 0xFF)
            dst[n++] = g;
    }
    dst[n] = EOS;
}

static char GameCharToAscii(u8 g)
{
    if (g >= CHAR_A && g <= CHAR_Z) return 'A' + (g - CHAR_A);
    if (g >= CHAR_a && g <= CHAR_z) return 'a' + (g - CHAR_a);
    if (g >= CHAR_0 && g <= CHAR_9) return '0' + (g - CHAR_0);
    switch (g)
    {
    case CHAR_EXCL_MARK: return '!';
    case CHAR_QUESTION_MARK: return '?';
    case CHAR_PERIOD: return '.';
    case CHAR_HYPHEN: return '-';
    case CHAR_SLASH: return '/';
    case CHAR_COMMA: return ',';
    case CHAR_COLON: return ':';
    case CHAR_SGL_QUOTE_RIGHT: case CHAR_SGL_QUOTE_LEFT: return '\'';
    case CHAR_LEFT_PAREN: return '(';
    case CHAR_RIGHT_PAREN: return ')';
    case CHAR_AMPERSAND: return '&';
    case CHAR_PLUS: return '+';
    case CHAR_e_ACUTE: return '~';      // back to e-acute in MP_AsciiToGame
    case CHAR_EQUALS: return '=';
    case CHAR_PERCENT: return '%';
    case CHAR_DBL_QUOTE_LEFT: case CHAR_DBL_QUOTE_RIGHT: return '"';
    }
    return ' ';
}

void MP_GameToAscii(char *dst, const u8 *src, int maxLen)
{
    int i;
    for (i = 0; i < maxLen && src[i] != EOS; i++)
        dst[i] = GameCharToAscii(src[i]);
    dst[i] = 0;
}

// ---------------------------------------------------------------------------
// Config file (appearance and preferences; nothing network related is needed)

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

static const char *ConfigPath(void)
{
    return getenv("MOEMON_COOP_CONFIG") ? getenv("MOEMON_COOP_CONFIG") : CONFIG_FILE;
}

static void LoadConfig(void)
{
    FILE *f;
    char line[512];

    memset(&gMpConfig, 0, sizeof(gMpConfig));
    gMpConfig.enabled = TRUE;
    gMpConfig.trainerRange = 4;
    memset(gMpConfig.phrases, 0xFF, sizeof(gMpConfig.phrases));

    f = fopen(ConfigPath(), "r");
    if (!f)
        return;
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
        if (strcmp(key, "lobby") == 0) snprintf(gMpConfig.lobby, sizeof(gMpConfig.lobby), "%s", val);
        else if (strcmp(key, "name") == 0) snprintf(gMpConfig.name, sizeof(gMpConfig.name), "%s", val);
        else if (strcmp(key, "last_room") == 0) snprintf(gMpConfig.lastRoom, sizeof(gMpConfig.lastRoom), "%s", val);
        else if (strcmp(key, "last_room_key") == 0 && strlen(val) == MP_ROOM_KEY_SIZE * 2) snprintf(gMpConfig.lastRoomKey, sizeof(gMpConfig.lastRoomKey), "%s", val);
        else if (strcmp(key, "last_host_name") == 0) snprintf(gMpConfig.lastHostName, sizeof(gMpConfig.lastHostName), "%s", val);
        else if (strcmp(key, "sprite") == 0)
        {
            char lower[64];
            int i;
            for (i = 0; val[i] && i < 63; i++)
                lower[i] = (val[i] == ' ') ? '_' : tolower((unsigned char)val[i]);
            lower[i] = 0;
            if (!MP_Avatar_ParseSpriteName(lower, &gMpConfig.app))
                gMpConfig.app.base = MP_BASE_DEFAULT;
        }
        else if (strcmp(key, "custom_sprite") == 0) snprintf(gMpConfig.customSprite, sizeof(gMpConfig.customSprite), "%s", val);
        else if (strcmp(key, "skin") == 0) gMpConfig.app.skin = ParseChoice(val, MP_Avatar_SkinName, MP_SKIN_COUNT);
        else if (strcmp(key, "outfit") == 0) gMpConfig.app.outfit = ParseChoice(val, MP_Avatar_ColorName, MP_COLOR_COUNT);
        else if (strcmp(key, "accent") == 0) gMpConfig.app.accent = ParseChoice(val, MP_Avatar_ColorName, MP_COLOR_COUNT);
        else if (strcmp(key, "hair") == 0) gMpConfig.app.hair = ParseChoice(val, MP_Avatar_HairName, MP_HAIR_COUNT);
        else if (strcmp(key, "trainer_pic") == 0) gMpConfig.app.frontSel = MP_Avatar_ParseFrontSel(val);
        else if (strcmp(key, "back_pic") == 0) gMpConfig.app.backSel = MP_Avatar_ParseBackSel(val);
        else if (strncmp(key, "custom", 6) == 0 && key[6] >= '1' && key[6] < '1' + MP_CREATED_SLOTS && key[7] == 0)
        {
            struct MpCreated r;
            if (MP_Created_FromHex(val, &r))
                gMpConfig.custom[key[6] - '1'] = r;
        }
        else if (strcmp(key, "trainer_range") == 0)
        {
            int r = atoi(val);
            gMpConfig.trainerRange = (r < 1) ? 1 : (r > 8 ? 8 : r);
        }
        else if (strcasecmp(key, "AllowSmallTweaks") == 0) gMpConfig.smallTweaks = atoi(val) == 1;
        else if (strcmp(key, "follower") == 0) gMpConfig.followerPersonality = (u32)strtoul(val, NULL, 16);
        else if (strncmp(key, "phrase", 6) == 0 && key[6] >= '1' && key[6] < '1' + MP_NUM_PHRASES && key[7] == 0)
        {
            char *p = val;
            int w;
            for (w = 0; w < MP_PHRASE_WORDS; w++)
                gMpConfig.phrases[key[6] - '1'][w] = (u16)strtoul(p, &p, 16);
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
    char sprite[48], skin[32], outfit[32], accent[32], hair[32];

    MP_Avatar_SpriteConfigName(&gMpConfig.app, sprite, sizeof(sprite));
    LowerName(skin, MP_Avatar_SkinName(gMpConfig.app.skin));
    LowerName(outfit, MP_Avatar_ColorName(gMpConfig.app.outfit));
    LowerName(accent, MP_Avatar_ColorName(gMpConfig.app.accent));
    LowerName(hair, MP_Avatar_HairName(gMpConfig.app.hair));
    f = fopen(ConfigPath(), "w");
    if (!f)
        return;
    fprintf(f,
        "; Moemon Emerald Kai Co-op settings. The game writes this file itself;\n"
        "; everything here can be changed in game (CO-OP menu).\n"
        "name = %s\n"
        "; sprite = default, brendan, may, rs_brendan, rs_may, custom_1..3, or any sprite name from the CO-OP menu (e.g. hiker, steven)\n"
        "sprite = %s\ncustom_sprite = %s\n"
        "; skin = default, porcelain, light, tan, brown, dark, deep\n"
        "skin = %s\n"
        "; outfit / accent = default, red, blue, green, yellow, purple, pink, orange, black, white, teal\n"
        "outfit = %s\naccent = %s\n"
        "; hair = default, black, brown, chestnut, blonde, ginger, red, silver, white, blue, pink, purple, green, teal, gold\n"
        "hair = %s\n"
        "; trainer card / battle pictures: match (same as the sprite) or a name from CO-OP > LOOK\n"
        "trainer_pic = %s\nback_pic = %s\n"
        "; how many tiles a player holding the BATTLE FLAG can see\n"
        "trainer_range = %d\n"
        "; the party Moemon that follows you (SELECT in the party menu); 0 = none\n"
        "follower = %08x\n"
        "; remembered for the next session\n"
        "last_room = %s\nlast_room_key = %s\nlast_host_name = %s\n"
        "; advanced: use a different online lobby server (address or address:port)\n"
        "lobby = %s\n"
        "; your 4 CO-OP phrases (made in CO-OP menu > EMOTES; word codes, ffff = empty)\n",
        gMpConfig.name, sprite, gMpConfig.customSprite, skin, outfit, accent, hair,
        MP_Avatar_FrontSelConfigName(gMpConfig.app.frontSel), MP_Avatar_BackSelConfigName(gMpConfig.app.backSel), gMpConfig.trainerRange, (unsigned)gMpConfig.followerPersonality,
        gMpConfig.lastRoom, gMpConfig.lastRoomKey, gMpConfig.lastHostName, gMpConfig.lobby);
    {
        int i;
        for (i = 0; i < MP_NUM_PHRASES; i++)
            fprintf(f, "phrase%d = %04x %04x %04x %04x\n", i + 1, gMpConfig.phrases[i][0], gMpConfig.phrases[i][1],
                gMpConfig.phrases[i][2], gMpConfig.phrases[i][3]);
    }
    {
        int i;
        char hex[2 * MP_CREATED_BYTES + 1];
        fprintf(f, "; trainers made in CO-OP > LOOK > CREATE (sprite = custom_1..3)\n");
        for (i = 0; i < MP_CREATED_SLOTS; i++)
        {
            MP_Created_ToHex(&gMpConfig.custom[i], hex);
            fprintf(f, "custom%d = %s\n", i + 1, hex);
        }
    }
    fprintf(f, "AllowSmallTweaks = %d\n", gMpConfig.smallTweaks ? 1 : 0);
    fclose(f);
}

const char *MP_OnlineLobbyAddress(void)
{
    if (getenv("MOEMON_LOBBY"))
        return getenv("MOEMON_LOBBY");
    if (gMpConfig.lobby[0])
        return gMpConfig.lobby;
    // lobby.txt next to the game: whoever runs the server can put its address
    // there once and share the folder, so nobody else has to set anything
    {
        static char sFileLobby[128];
        static bool8 sFileRead;
        if (!sFileRead)
        {
            FILE *f = fopen("lobby.txt", "r");
            sFileRead = TRUE;
            if (f)
            {
                char line[128];
                while (fgets(line, sizeof(line), f))
                {
                    char *t = Trim(line);
                    if (t[0] && t[0] != ';' && t[0] != '#')
                    {
                        snprintf(sFileLobby, sizeof(sFileLobby), "%s", t);
                        break;
                    }
                }
                fclose(f);
            }
        }
        if (sFileLobby[0])
            return sFileLobby;
    }
    return MP_DEFAULT_LOBBY;
}

// Invite code for a game hosted on this PC: from the router's public address,
// or (last resort) from a Radmin VPN / Hamachi address. "" when there is none.
static char sVpnName[24];

const char *MP_VpnName(void)
{
    return sVpnName;
}

const char *MP_InviteCode(void)
{
    static char sVpnCode[16];
    static int sVpnPort;
    if (!MPServer_IsRunning())
        return "";
    if (MPPortMap_State() == MP_PM_OPEN)
        return MPPortMap_InviteCode();
    if (MPPortMap_State() != MP_PM_FAILED)
        return "";
    if (sVpnPort != MPServer_Port())
    {
        char addr[32];
        unsigned a, b, c, d;
        sVpnCode[0] = 0;
        sVpnPort = MPServer_Port();
        if (MPNet_FindVpnAddress(addr, sizeof(addr), sVpnName, sizeof(sVpnName))
         && sscanf(addr, "%u.%u.%u.%u", &a, &b, &c, &d) == 4)
            MPNet_MakeInviteCode((a << 24) | (b << 16) | (c << 8) | d, sVpnPort, sVpnCode, sizeof(sVpnCode));
    }
    return sVpnCode;
}

bool8 MP_HasOnlineLobby(void)
{
    return MP_OnlineLobbyAddress()[0] != 0;
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
    PutU16(p + 16, s->followerDex); p[18] = s->followerFlags; p[19] = s->badges;
}

static void UnpackState(const u8 *p, struct MpState *s)
{
    s->mapGroup = p[0]; s->mapNum = p[1];
    s->x = (s16)GetU16(p + 2); s->y = (s16)GetU16(p + 4);
    s->facing = p[6]; s->elevation = p[7]; s->avatarMode = p[8]; s->speed = p[9];
    s->flags = p[10]; s->topLevel = p[11]; s->partyCount = p[12]; s->emote = p[13];
    s->seq = GetU16(p + 14);
    s->followerDex = GetU16(p + 16); s->followerFlags = p[18]; s->badges = p[19];
}

#define APPEARANCE_SIZE 32

static void PackAppearance(const struct MpAppearance *a, u8 *p)
{
    memset(p, 0, APPEARANCE_SIZE);
    p[0] = a->base; p[1] = a->skin; p[2] = a->outfit; p[3] = a->accent;
    p[4] = a->gender; p[5] = a->hasCustom;
    PutU16(p + 6, a->gfxId);
    PutU16(p + 8, a->frontSel);
    p[10] = a->backSel;
    p[11] = a->hair;
    p[12] = a->created;
    MP_Created_Pack(&a->recipe, p + 13);   // 13 bytes
}

static void UnpackAppearance(const u8 *p, struct MpAppearance *a)
{
    memset(a, 0, sizeof(*a));
    a->base = p[0] < MP_BASE_COUNT ? p[0] : 0;
    a->skin = p[1] < MP_SKIN_COUNT ? p[1] : 0;
    a->outfit = p[2] < MP_COLOR_COUNT ? p[2] : 0;
    a->accent = p[3] < MP_COLOR_COUNT ? p[3] : 0;
    a->gender = p[4] ? FEMALE : MALE;
    a->hasCustom = p[5];
    a->gfxId = GetU16(p + 6);
    a->frontSel = GetU16(p + 8) < MP_Avatar_FrontChoiceCount() ? GetU16(p + 8) : 0;
    a->backSel = p[10] < MP_Avatar_BackChoiceCount() ? p[10] : 0;
    a->hair = p[11] < MP_HAIR_COUNT ? p[11] : 0;
    a->created = p[12] <= MP_CREATED_SLOTS ? p[12] : 0;
    MP_Created_Unpack(p + 13, &a->recipe);
    if (a->base == MP_BASE_CREATED && !a->recipe.made)
        a->base = MP_BASE_DEFAULT;
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

bool8 MP_IsBattleTransitionRunning(void);   // battle_transition.c

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

// Right stick flick on a controller (0 up, 1 right, 2 down, 3 left)
void MP_Emote_FromStick(u8 dir)
{
    // up !, right heart, down ?, left music note
    static const u8 sKeys[] = {0, 2, 1, 3};
    MP_Emote_Hotkey(sKeys[dir & 3]);
}

static struct MpState sLastLocalState;

void MP_GetLocalState(struct MpState *s)
{
    *s = sLastLocalState;
}

static void CollectLocalState(struct MpState *s)
{
    *s = sLastLocalState;
    s->flags &= ~(MPF_FREE | MPF_IN_OVERWORLD | MPF_IN_BATTLE | MPF_TRAINER_MODE | MPF_ENGAGED | MPF_CHALLENGED);

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
    // From the moment the battle transition starts, so others see it right away
    if (MP_Battle_IsActive() || gMain.inBattle || MP_IsBattleTransitionRunning())
        s->flags |= MPF_IN_BATTLE;
    if (MP_Coop_IsChallenged())
        s->flags |= MPF_CHALLENGED;
    if (MP_Piggy_IsRiding())
        s->flags |= MPF_RIDING;
    if (MP_Trainer_IsActive())
        s->flags |= MPF_TRAINER_MODE;
    if (MP_Trainer_IsEngaged() || MP_Battle_IsActive())
        s->flags |= MPF_ENGAGED;
    s->topLevel = MP_GetPartyTopLevel();
    s->badges = MP_Badges();
    s->partyCount = gPlayerPartyCount;
    s->emote = (sEmoteCounter << 4) | sEmoteKind;
    {
        bool8 female;
        s->followerDex = MP_Follower_LocalDex(&female);
        s->followerFlags = female ? 1 : 0;
    }
    sLastLocalState = *s;
}

static bool8 StateDiffers(const struct MpState *a, const struct MpState *b)
{
    return a->mapGroup != b->mapGroup || a->mapNum != b->mapNum || a->x != b->x || a->y != b->y
        || a->facing != b->facing || a->elevation != b->elevation || a->avatarMode != b->avatarMode
        || a->speed != b->speed || a->flags != b->flags || a->topLevel != b->topLevel
        || a->emote != b->emote || a->followerDex != b->followerDex || a->followerFlags != b->followerFlags
        || a->badges != b->badges;
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

// Sends big data (custom sprites) in chunks
void MP_SendBlob(u8 kind, const void *data, u32 len)
{
    static u8 chunk[MP_BLOB_CHUNK + 16];
    u32 off;
    if (!MP_IsConnected() || len == 0 || len > MP_BLOB_MAX_SIZE)
        return;
    for (off = 0; off < len; off += MP_BLOB_CHUNK)
    {
        u32 n = len - off;
        if (n > MP_BLOB_CHUNK)
            n = MP_BLOB_CHUNK;
        chunk[0] = kind;
        PutU32(chunk + 1, len);
        PutU32(chunk + 5, off);
        memcpy(chunk + 9, (const u8 *)data + off, n);
        MPNet_ClientSend(MPMSG_C_BLOB, chunk, n + 9);
    }
}

static void SendLocalBlobs(void)
{
    if (MP_Avatar_LocalCustomBlob())
        MP_SendBlob(MP_BLOB_SPRITES, MP_Avatar_LocalCustomBlob(), MP_Avatar_LocalCustomBlobSize());
}

void MP_SendAppearance(void)
{
    u8 hello[MP_HELLO_SIZE];
    if (!MP_IsConnected())
        return;
    SendLocalBlobs();
    BuildHello(hello);
    MPNet_ClientSend(MPMSG_C_INFO, hello, sizeof(hello));
}

void MP_SendRelay(u8 target, const void *data, u16 len)
{
    static u8 buf[MP_MAX_MSG + 8];
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
    u8 k;
    free(gMpPlayers[id].customGfx);
    for (k = 0; k < MP_BLOB_KINDS; k++)
        free(gMpPlayers[id].blobBuf[k]);
    memset(&gMpPlayers[id], 0, sizeof(gMpPlayers[id]));
}

static void HandlePlayerLeft(u8 id)
{
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return;
    MP_Overworld_OnPlayerLeft(id);
    MP_Emote_OnPlayerLeft(id);
    MP_Piggy_OnPlayerLeft(id);
    MP_Linkx_OnPlayerLeft(id);
    MP_Minigame_OnPlayerLeft(id);
    MP_Group_OnPlayerLeft(id);
    MP_Trainer_OnPlayerLeft(id);
    MP_Battle_OnPlayerLeft(id);
    MP_Spectate_OnPlayerLeft(id);
    MP_Coop_OnPlayerLeft(id);
    ClearPlayer(id);
}

static void ResetRoster(void)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        HandlePlayerLeft(i);
    gMpLocalId = -1;
    sHelloSent = FALSE;
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

static void OnBlobComplete(u8 id, u8 kind)
{
    struct MpPlayer *p = &gMpPlayers[id];
    if (kind == MP_BLOB_SPRITES)
    {
        if (!MP_Avatar_IsValidBlob(p->blobBuf[kind], p->blobTotal[kind]))
            return;
        free(p->customGfx);
        p->customGfx = p->blobBuf[kind];
        p->customGfxSize = p->blobTotal[kind];
        p->blobBuf[kind] = NULL;
        p->appearanceVersion++;
        MP_Overworld_OnAppearanceChanged(id);
    }
}

static void HandleBlobChunk(const u8 *data, u16 len)
{
    u8 id, kind;
    u32 total, off, n;
    struct MpPlayer *p;

    if (len < 10)
        return;
    id = data[0];
    kind = data[1];
    total = GetU32(data + 2);
    off = GetU32(data + 6);
    n = len - 10;
    if (id >= MP_MAX_PLAYERS || id == gMpLocalId || kind >= MP_BLOB_KINDS
     || total == 0 || total > MP_BLOB_MAX_SIZE || off + n > total)
        return;
    p = &gMpPlayers[id];
    if (off == 0)
    {
        free(p->blobBuf[kind]);
        p->blobBuf[kind] = malloc(total);
        p->blobTotal[kind] = total;
        p->blobHave[kind] = 0;
    }
    if (!p->blobBuf[kind] || p->blobTotal[kind] != total || p->blobHave[kind] != off)
        return;
    memcpy(p->blobBuf[kind] + off, data + 10, n);
    p->blobHave[kind] += n;
    if (p->blobHave[kind] == total)
        OnBlobComplete(id, kind);
}

static void Lobby_OnMessage(u8 type, const u8 *data, u16 len);

static void HandleMessage(u8 type, u8 *data, u16 len)
{
    u8 id;
    switch (type)
    {
    case MPMSG_S_ROOM_LIST:
    case MPMSG_S_ROOM_OK:
    case MPMSG_S_ROOM_ERROR:
        Lobby_OnMessage(type, data, len);
        break;
    case MPMSG_S_WELCOME:
        if (len < 1)
            return;
        gMpLocalId = data[0];
        snprintf(sStatus, sizeof(sStatus), "ONLINE (PLAYER %d)", gMpLocalId + 1);
        MPNet_Log("joined \"%s\" as player %d", sLobby.room, gMpLocalId + 1);
        SendLocalBlobs();
        SendLocalState(TRUE);
        break;
    case MPMSG_S_REJECT:
        MPNet_Log("server rejected us (%d)", len ? data[0] : 0);
        if (len && data[0] == MP_REJECT_NO_ROOM)
            sHelloSent = FALSE; // we'll say hello again after rejoining
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
    case MPMSG_S_BLOB:
        HandleBlobChunk(data, len);
        break;
    case MPMSG_S_STORE:
        MP_Store_OnMessage(data, len);
        break;
    case MPMSG_S_RELAY:
        if (len < 2 || data[0] >= MP_MAX_PLAYERS)
            return;
        MP_DispatchRelay(data[0], data + 1, len - 1);
        break;
    }
}

// Routes peer messages to the module that owns the message id
void MP_DispatchRelay(u8 from, const u8 *data, u16 len)
{
    if (data[0] >= MPR_LINK_PLAYER && data[0] < MPR_SPECTATE_FIRST)
        MP_Battle_OnRelay(from, data, len);
    else if (data[0] >= MPR_SPECTATE_FIRST && data[0] < MPR_COOP_FIRST)
        MP_Spectate_OnRelay(from, data, len);
    else if (data[0] >= MPR_COOP_FIRST && data[0] < MPR_COOP_FIRST + 20)
        MP_Coop_OnRelay(from, data, len);
    else if (data[0] == MPR_SAY)
        MP_Emote_OnRelay(from, data, len);
    else if (data[0] >= MPR_DEX_FIRST && data[0] < MPR_DEX_FIRST + 4)
        MP_Dex_OnRelay(from, data, len);
    else if (data[0] >= MPR_TRADE_FIRST && data[0] < MPR_GIFT_FIRST)
        MP_Trade_OnRelay(from, data, len);
    else if (data[0] >= MPR_GIFT_FIRST && data[0] < MPR_EVENT_FIRST)
        MP_Gift_OnRelay(from, data, len);
    else if (data[0] >= MPR_TOURNEY_FIRST && data[0] < MPR_TOURNEY_FIRST + 20)
        MP_Tourney_OnRelay(from, data, len);
    else if (data[0] >= MPR_SOCIAL_FIRST + 5 && data[0] < MPR_SOCIAL_FIRST + 10)
        MP_Piggy_OnRelay(from, data, len);
    else if (data[0] >= MPR_SOCIAL_FIRST && data[0] < MPR_SOCIAL_FIRST + 5)
        MP_Social_OnRelay(from, data, len);
    else if (data[0] >= MPR_LINKX_FIRST && data[0] < MPR_LINKX_FIRST + 20)
        MP_Linkx_OnRelay(from, data, len);
    else if (data[0] >= MPR_MINIGAME_FIRST && data[0] < MPR_MINIGAME_FIRST + 14)
        MP_Minigame_OnRelay(from, data, len);
    else if (data[0] >= MPR_MINIGAME_FIRST + 14 && data[0] < MPR_MINIGAME_FIRST + 20)
        MP_Group_OnRelay(from, data, len);
    else if (data[0] < MPR_LINK_PLAYER)
        MP_Trainer_OnRelay(from, data, len);
}

// ---------------------------------------------------------------------------
// Lobby

u8 MP_Lobby_State(void) { return sLobby.state; }
u8 MP_Lobby_Error(void) { return sLobby.error; }
const char *MP_Lobby_RoomName(void) { return sLobby.room; }
bool8 MP_Lobby_IsLocalHost(void) { return sLobby.isLocal; }
bool8 MP_Lobby_InRoom(void) { return sLobby.state == MP_LOBBY_IN_ROOM; }

static void SetError(u8 err)
{
    sLobby.error = err;
    sLobby.state = MP_LOBBY_ERROR;
    sLobby.request = REQ_NONE;
}

static void OpenConnection(void)
{
    MPNet_ClientClose();
    ResetRoster();
    sLobby.timer = 0;
    if (MPNet_Connect(sLobby.host, sLobby.port) != 0)
    {
        MPNet_Log("%s", MPNet_LastError());
        SetError(MP_LOBBY_ERR_UNREACHABLE);
        return;
    }
    sLobby.state = MP_LOBBY_CONNECTING;
    snprintf(sStatus, sizeof(sStatus), "CONNECTING...");
}

void MP_Lobby_Connect(const char *address, bool8 isLocal)
{
    char host[128];
    int port = MP_DEFAULT_PORT;
    MPNet_ParseAddress(address, host, sizeof(host), &port);
    memset(&sLobby, 0, sizeof(sLobby));
    snprintf(sLobby.host, sizeof(sLobby.host), "%s", host);
    sLobby.port = port;
    sLobby.isLocal = isLocal;
    MPNet_Log("connecting to lobby %s:%d", host, port);
    OpenConnection();
}

void MP_Lobby_Disconnect(void)
{
    if (MPNet_ClientStatus() != MPNET_DISCONNECTED && sLobby.state == MP_LOBBY_IN_ROOM)
        MPNet_ClientSend(MPMSG_C_ROOM_LEAVE, NULL, 0);
    MPNet_ClientPoll(&(u8){0}, sMsgBuf, &(u16){0}); // flush the leave message
    MPNet_ClientClose();
    ResetRoster();
    memset(&sLobby, 0, sizeof(sLobby));
    snprintf(sStatus, sizeof(sStatus), "OFFLINE");
}

void MP_Lobby_RequestList(void)
{
    sLobby.listReady = FALSE;
    if (MPNet_ClientStatus() == MPNET_CONNECTED)
        MPNet_ClientSend(MPMSG_C_ROOM_LIST, NULL, 0);
}

bool8 MP_Lobby_ListReady(void)
{
    return sLobby.listReady;
}

int MP_Lobby_GetList(struct MpLanRoom *out, int max)
{
    int i;
    for (i = 0; i < sLobby.listCount && i < max; i++)
        out[i] = sLobby.list[i];
    return i;
}

static int PutStr(u8 *out, const char *s)
{
    int n = (int)strlen(s);
    if (n > MP_ROOM_NAME_MAX)
        n = MP_ROOM_NAME_MAX;
    out[0] = (u8)n;
    memcpy(out + 1, s, n);
    return n + 1;
}

static void SendRoomRequest(void)
{
    u8 buf[64];
    int n = 0;
    PutU16(buf, MP_PROTOCOL_VERSION);
    n = 2;
    n += PutStr(buf + n, sLobby.room);
    if (sLobby.request == REQ_CREATE)
    {
        buf[n++] = sLobby.locked;
        memcpy(buf + n, sLobby.key, MP_ROOM_KEY_SIZE);
        n += MP_ROOM_KEY_SIZE;
        buf[n++] = sLobby.hidden;
        MPNet_ClientSend(MPMSG_C_ROOM_CREATE, buf, n);
    }
    else
    {
        memcpy(buf + n, sLobby.key, MP_ROOM_KEY_SIZE);
        n += MP_ROOM_KEY_SIZE;
        MPNet_ClientSend(MPMSG_C_ROOM_JOIN, buf, n);
    }
    sLobby.timer = 0;
}

static void StartRequest(u8 req, const char *room, const char *password, bool8 hidden)
{
    snprintf(sLobby.room, sizeof(sLobby.room), "%s", room);
    MPNet_RoomKey(room, password ? password : "", sLobby.key);
    sLobby.locked = (password && password[0]) ? TRUE : FALSE;
    sLobby.hidden = hidden;
    sLobby.request = req;
    sLobby.error = MP_LOBBY_ERR_NONE;
    if (sLobby.state == MP_LOBBY_READY || sLobby.state == MP_LOBBY_ERROR)
    {
        if (MPNet_ClientStatus() != MPNET_CONNECTED)
        {
            OpenConnection(); // the request goes out once connected
            return;
        }
        sLobby.state = MP_LOBBY_WAITING;
        SendRoomRequest();
    }
}

void MP_Lobby_CreateRoom(const char *room, const char *password, bool8 hidden)
{
    StartRequest(REQ_CREATE, room, password, hidden);
}

void MP_Lobby_JoinRoom(const char *room, const char *password)
{
    StartRequest(REQ_JOIN, room, password, FALSE);
}

// Rejoin a server from last time with the remembered key (no password needed)
void MP_Lobby_JoinRoomWithKey(const char *room, const u8 *key)
{
    StartRequest(REQ_JOIN, room, "", FALSE);
    memcpy(sLobby.key, key, MP_ROOM_KEY_SIZE);
    if (sLobby.state == MP_LOBBY_WAITING)
        SendRoomRequest(); // the first request went out with the wrong key; resend
}

const u8 *MP_Lobby_RoomKey(void)
{
    return sLobby.key;
}

static int GetStr(const u8 *data, u16 len, int *pos, char *out, int max)
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

static void Lobby_OnMessage(u8 type, const u8 *data, u16 len)
{
    switch (type)
    {
    case MPMSG_S_ROOM_LIST:
    {
        int pos = 1, i, count;
        if (len < 1)
            return;
        count = data[0];
        sLobby.listCount = 0;
        for (i = 0; i < count && sLobby.listCount < MP_LIST_MAX; i++)
        {
            struct MpLanRoom *r = &sLobby.list[sLobby.listCount];
            memset(r, 0, sizeof(*r));
            if (!GetStr(data, len, &pos, r->name, MP_ROOM_NAME_MAX) || pos + 5 > len)
                break;
            r->players = data[pos];
            r->maxPlayers = data[pos + 1];
            r->flags = data[pos + 2];
            r->version = data[pos + 3] | (data[pos + 4] << 8);
            pos += 5;
            snprintf(r->host, sizeof(r->host), "%s", sLobby.host);
            r->port = sLobby.port;
            sLobby.listCount++;
        }
        sLobby.listReady = TRUE;
        break;
    }
    case MPMSG_S_ROOM_OK:
    {
        int pos = 0;
        char name[MP_ROOM_NAME_MAX + 1];
        if (!GetStr(data, len, &pos, name, MP_ROOM_NAME_MAX))
            return;
        snprintf(sLobby.room, sizeof(sLobby.room), "%s", name); // server's spelling
        sLobby.state = MP_LOBBY_IN_ROOM;
        sLobby.request = REQ_NONE;
        sLobby.wantRoom = TRUE;
        sLobby.reconnecting = FALSE;
        snprintf(sStatus, sizeof(sStatus), "IN %s", name);
        snprintf(gMpConfig.lastRoom, sizeof(gMpConfig.lastRoom), "%s", name);
        break;
    }
    case MPMSG_S_ROOM_ERROR:
    {
        u8 reason = len ? data[0] : 0;
        // Rejoining after a dropped connection, but the room closed meanwhile
        // (everyone dropped, or the server restarted): open it again.
        if (sLobby.reconnecting && sLobby.request == REQ_JOIN && reason == MP_ROOM_ERR_NOT_FOUND)
        {
            sLobby.request = REQ_CREATE;
            SendRoomRequest();
            return;
        }
        if (sLobby.reconnecting && sLobby.request == REQ_CREATE && reason == MP_ROOM_ERR_NAME_TAKEN)
        {
            sLobby.request = REQ_JOIN;
            SendRoomRequest();
            return;
        }
        switch (reason)
        {
        case MP_ROOM_ERR_NAME_TAKEN:   SetError(MP_LOBBY_ERR_NAME_TAKEN); break;
        case MP_ROOM_ERR_NOT_FOUND:    SetError(MP_LOBBY_ERR_NOT_FOUND); break;
        case MP_ROOM_ERR_BAD_PASSWORD: SetError(MP_LOBBY_ERR_BAD_PASSWORD); break;
        case MP_ROOM_ERR_FULL:         SetError(MP_LOBBY_ERR_FULL); break;
        case MP_ROOM_ERR_VERSION:      SetError(MP_LOBBY_ERR_VERSION); break;
        case MP_ROOM_ERR_BAD_NAME:     SetError(MP_LOBBY_ERR_BAD_NAME); break;
        case MP_ROOM_ERR_SLOW_DOWN:    SetError(MP_LOBBY_ERR_SLOW_DOWN); break;
        default:                       SetError(MP_LOBBY_ERR_SERVER_FULL); break;
        }
        sLobby.wantRoom = FALSE;
        sLobby.reconnecting = FALSE;
        break;
    }
    }
}

// Runs the connection state machine once per frame
static void Lobby_Update(void)
{
    int status = MPNet_ClientStatus();

    switch (sLobby.state)
    {
    case MP_LOBBY_CONNECTING:
        if (status == MPNET_CONNECTED)
        {
            sLobby.state = MP_LOBBY_READY;
            snprintf(sStatus, sizeof(sStatus), "CONNECTED");
            if (sLobby.request != REQ_NONE)
            {
                sLobby.state = MP_LOBBY_WAITING;
                SendRoomRequest();
            }
        }
        else if (status == MPNET_DISCONNECTED || ++sLobby.timer > CONNECT_TIMEOUT_FRAMES)
        {
            MPNet_ClientClose();
            if (sLobby.wantRoom)
            {
                sLobby.state = MP_LOBBY_RECONNECT_WAIT;
                sLobby.timer = 0;
            }
            else
                SetError(MP_LOBBY_ERR_UNREACHABLE);
        }
        break;
    case MP_LOBBY_WAITING:
        if (status == MPNET_DISCONNECTED && sLobby.wantRoom)
        {
            sLobby.state = MP_LOBBY_RECONNECT_WAIT;
            sLobby.timer = 0;
        }
        else if (status == MPNET_DISCONNECTED)
            SetError(MP_LOBBY_ERR_UNREACHABLE);
        else if (++sLobby.timer > CONNECT_TIMEOUT_FRAMES * 2)
        {
            MPNet_ClientClose();
            SetError(MP_LOBBY_ERR_UNREACHABLE);
        }
        break;
    case MP_LOBBY_READY:
        if (status == MPNET_DISCONNECTED)
            SetError(MP_LOBBY_ERR_UNREACHABLE);
        break;
    case MP_LOBBY_IN_ROOM:
        if (status == MPNET_DISCONNECTED)
        {
            MPNet_Log("connection to the lobby lost, reconnecting");
            ResetRoster();
            snprintf(sStatus, sizeof(sStatus), "RECONNECTING...");
            sLobby.state = MP_LOBBY_RECONNECT_WAIT;
            sLobby.timer = 0;
        }
        break;
    case MP_LOBBY_RECONNECT_WAIT:
        if (++sLobby.timer >= RECONNECT_FRAMES)
        {
            sLobby.reconnecting = TRUE;
            sLobby.request = REQ_JOIN;
            OpenConnection();
            if (sLobby.state == MP_LOBBY_ERROR) // could not even start: keep trying
            {
                sLobby.state = MP_LOBBY_RECONNECT_WAIT;
                sLobby.timer = 0;
            }
            sLobby.reconnecting = TRUE;
        }
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
    snprintf(sStatus, sizeof(sStatus), "OFFLINE");
}

void MP_FrameUpdate(void)
{
    u8 type;
    u16 len;
    int guard;

    if (!sInitDone)
        MP_Init();
    MP_Qol_Update();
    MP_Social_Update();
    MP_Together_Update();
    MP_Linkx_Update();
    MP_Minigame_Update();
    MP_Group_Update();

    MPServer_Poll();
    MP_AutoSession_Update();

    if (sLobby.state == MP_LOBBY_OFF)
        return;

    for (guard = 0; guard < 512 && MPNet_ClientPoll(&type, sMsgBuf, &len); guard++)
        HandleMessage(type, sMsgBuf, len);

    Lobby_Update();

    if (MPNet_ClientStatus() != MPNET_CONNECTED)
        return;

    if (++sPingTimer >= PING_FRAMES)
    {
        sPingTimer = 0;
        MPNet_ClientSend(MPMSG_C_PING, NULL, 0);
    }

    // Say hello once we're in a room and a save file is loaded (name, gender)
    if (sLobby.state == MP_LOBBY_IN_ROOM && !sHelloSent && MP_IsInOverworld())
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
    {
        MP_Coop_Update();
        MP_Dex_Update();
        MP_Trade_Update();
        MP_Events_Update();
        MP_Tourney_Update();
        SendLocalState(FALSE);
        MP_Spectate_FrameUpdate();
    }

    // flush now so input latency stays low
    MPNet_ClientPoll(&type, sMsgBuf, &len) ? HandleMessage(type, sMsgBuf, len) : (void)0;
}
