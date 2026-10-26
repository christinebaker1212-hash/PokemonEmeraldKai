#include "tcg/integration.h"
#include "mp_version.h"
// Moemon Co-op: client core (config, lobby session, player roster, local state)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "mp_net.h"
#include "mp_join.h"
#include "global.h"
#include "mp.h"
#include "mp_pmd.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "script.h"
#include "palette.h"
#include "field_player_avatar.h"
#include "event_object_movement.h"
#include "constants/characters.h"
#include "string_util.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "platform/gamepad.h"
#include "mp_life_battle.h"
#include "mp_life_social.h"
#include "mp_garden.h"
#include "mp_farm.h"

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
    bool8 creator;            // we made the room we're in (we send the welcome note)
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
static void SetError(u8 err);
static u16 sOtherVersion;   // the version a server or room told us about when it didn't match ours

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

static void SendWelcome(u8 id);
static bool8 sRoomPadding, sRoomRulesHeard;   // MPR_RULES from the room's host
static s8 sRoomHostId = -1;          // ... and who sent them (the host's player id)
static u8 sRoomScale[MP_SCALE_COUNT - 1];   // ... LEVEL SCALING's settings
static void LevelRulesFromEnv(void);
static bool8 sRoomLevelScaling;
// The game clock: online it's the host's PC clock (MPR_CLOCK), offline our own
static bool8 sHostClockHeard;
static s64 sHostClockOffset;      // the host's local time minus our time(NULL), in seconds
static u16 sClockResendTimer;
static u8 sRoomAnimSpeed;
static bool8 OwnPadding(void);
static u8 OwnAnimSpeed(void);
static bool8 OwnLevelScaling(void);
static void ShowWelcome(u8 from, const u8 *text, u16 len);

// Button icons in co-op text: "{A}", "{START}", "{LT}"... (mp_buttons.c draws the controller's)
static const struct { const char *name; u8 id; } sButtonTokens[] = {
    {"A", CHAR_A_BUTTON}, {"B", CHAR_B_BUTTON}, {"L", CHAR_L_BUTTON}, {"R", CHAR_R_BUTTON},
    {"START", CHAR_START_BUTTON}, {"SELECT", CHAR_SELECT_BUTTON},
    {"UP", CHAR_DPAD_UP}, {"DOWN", CHAR_DPAD_DOWN}, {"LEFT", CHAR_DPAD_LEFT}, {"RIGHT", CHAR_DPAD_RIGHT},
    {"UPDOWN", CHAR_DPAD_UPDOWN}, {"LEFTRIGHT", CHAR_DPAD_LEFTRIGHT}, {"DPAD", CHAR_DPAD_NONE},
    {"LT", CHAR_MP_LT}, {"RT", CHAR_MP_RT}, {"LS", CHAR_MP_LS}, {"RS", CHAR_MP_RS},
    {"WEST", CHAR_MP_WEST}, {"NORTH", CHAR_MP_NORTH},
};

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen)
{
    int i, n = 0;
    for (i = 0; src[i] && n < maxLen; i++)
    {
        // {COOP}: what opens the CO-OP menu (the C key, or the pad's north button)
        if (strncmp(src + i, "{COOP}", 6) == 0)
        {
            if (Gamepad_IconFamily() == GAMEPAD_ICONS_KEYBOARD)
            {
                static const char sKey[] = "the C key";
                int k;
                for (k = 0; sKey[k] && n < maxLen; k++)
                    dst[n++] = AsciiCharToGame(sKey[k]);
            }
            else if (n + 2 <= maxLen)
            {
                dst[n++] = CHAR_KEYPAD_ICON;
                dst[n++] = CHAR_MP_NORTH;
            }
            i += 5;
            continue;
        }
        if (src[i] == '{' && n + 2 <= maxLen)
        {
            const char *end = strchr(src + i, '}');
            int k, len = end ? (int)(end - src - i - 1) : -1;
            for (k = 0; len > 0 && k < (int)ARRAY_COUNT(sButtonTokens); k++)
            {
                if ((int)strlen(sButtonTokens[k].name) == len && strncmp(src + i + 1, sButtonTokens[k].name, len) == 0)
                {
                    dst[n++] = CHAR_KEYPAD_ICON;
                    dst[n++] = sButtonTokens[k].id;
                    i += len + 1;
                    break;
                }
            }
            if (len > 0 && k < (int)ARRAY_COUNT(sButtonTokens))
                continue;
        }
        // é / É written straight into the text (UTF-8: "MOéMON", "POKéMON")
        if ((u8)src[i] == 0xC3 && ((u8)src[i + 1] == 0xA9 || (u8)src[i + 1] == 0x89))
        {
            dst[n++] = (u8)src[i + 1] == 0xA9 ? CHAR_e_ACUTE : CHAR_E_ACUTE;
            i++;
            continue;
        }
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

u8 *MP_CopyPlayerName(u8 *dst, u8 id)
{
    u8 i;
    if (id < MP_MAX_PLAYERS)
        for (i = 0; i < MP_NAME_LEN && gMpPlayers[id].name[i] != EOS; i++)
            *dst++ = gMpPlayers[id].name[i];
    *dst = EOS;
    return dst;
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

// Before anything is loaded (sdl2.c main: the Windows log console): AllowSmallTweaks = 1?
bool8 MP_ConfigAllowsTweaks(void)
{
    char line[256];
    bool8 on = FALSE;
    FILE *f = fopen(ConfigPath(), "r");
    if (!f)
        return FALSE;
    while (fgets(line, sizeof(line), f))
    {
        char *p = line, *eq;
        while (*p == ' ' || *p == '\t')
            p++;
        if (strncasecmp(p, "AllowSmallTweaks", 16) != 0)
            continue;
        eq = p + 16;
        while (*eq == ' ' || *eq == '\t')
            eq++;
        if (*eq == '=')
            on = atoi(eq + 1) == 1;
    }
    fclose(f);
    return on;
}

static int sMainAutosaveMinutes;

static void LoadConfig(void)
{
    FILE *f;
    char line[512];

    memset(&gMpConfig, 0, sizeof(gMpConfig));
    gMpConfig.enabled = TRUE;
    gMpConfig.trainerRange = 4;
    gMpConfig.autosaveMinutes = 5;
    gMpConfig.trainerPadding = TRUE;
    gMpConfig.levelScaling = TRUE;
    gMpConfig.scale[MP_SCALE_WILD - 1] = 2;       // MEDIUM: halfway between the area's level and the team's
    gMpConfig.scale[MP_SCALE_TRAINERS - 1] = 2;   // EVEN
    gMpConfig.scale[MP_SCALE_BOSS - 1] = 0;       // AUTO
    gMpConfig.scale[MP_SCALE_CAPS - 1] = 1;
    gMpConfig.scale[MP_SCALE_PAST8 - 1] = 0;
    gMpConfig.animSpeed = 30;
    gMpConfig.spectateScale = 1;
    gMpConfig.bikeMusic = TRUE;
    gMpConfig.surfMusic = TRUE;
    memset(gMpConfig.phrases, 0xFF, sizeof(gMpConfig.phrases));
    gMpConfig.quickStock[0] = 0;   // Hello!
    gMpConfig.quickStock[1] = 4;   // Follow me!
    gMpConfig.quickStock[2] = 2;   // Thank you!
    gMpConfig.quickStock[3] = 3;   // Good game!

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
        else if (strcmp(key, "last_room_address") == 0) snprintf(gMpConfig.lastRoomAddr, sizeof(gMpConfig.lastRoomAddr), "%s", val);
        else if (strcmp(key, "last_join_code") == 0) snprintf(gMpConfig.lastJoinCode, sizeof(gMpConfig.lastJoinCode), "%s", val);
        else if (strcmp(key, "welcome") == 0) snprintf(gMpConfig.welcome, sizeof(gMpConfig.welcome), "%s", val);
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
        else if (strcmp(key, "bike") == 0) gMpConfig.app.bike = (strcmp(val, "bike") == 0);
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
        else if (strcmp(key, "autosave") == 0)
        {
            int m = atoi(val);
            gMpConfig.autosaveMinutes = m < 0 ? 0 : (m > 60 ? 60 : m);
        }
        else if (strcmp(key, "trainer_padding") == 0) gMpConfig.trainerPadding = atoi(val) != 0;
        else if (strcmp(key, "level_scaling") == 0) gMpConfig.levelScaling = atoi(val) != 0;
        else if (strcmp(key, "level_scaling_rules") == 0)
        {
            int v[MP_SCALE_COUNT - 1], k;
            if (sscanf(val, "%d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4]) == MP_SCALE_COUNT - 1)
                for (k = 0; k < MP_SCALE_COUNT - 1; k++)
                    if (v[k] >= 0 && v[k] < MP_LevelScaleRuleCount(k + 1))
                        gMpConfig.scale[k] = v[k];
        }
        else if (strcmp(key, "spectate_panel") == 0) gMpConfig.spectatePanel = atoi(val) != 0;
        else if (strcmp(key, "bike_music") == 0) gMpConfig.bikeMusic = atoi(val) != 0;
        else if (strcmp(key, "surf_music") == 0) gMpConfig.surfMusic = atoi(val) != 0;
        else if (strcmp(key, "moenav_calls") == 0) gMpConfig.moenavCalls = strcmp(val, "battles") == 0;
        else if (strcmp(key, "follower_sprites") == 0)
            gMpConfig.followerSprites = strcmp(val, "pmd") == 0 ? MP_FOLLOW_SPRITES_PMD
                                      : strcmp(val, "original") == 0 ? MP_FOLLOW_SPRITES_ORIGINAL : MP_FOLLOW_SPRITES_DEFAULT;
        else if (strcmp(key, "spectate_friend") == 0)
        {
            // spectate_friend = SIDE SIZE POPOUT SHOW NAME
            char side[16];
            int size = 1, pop = 0, show = 1, used = 0, i, n;
            if (sscanf(val, "%15s %d %d %d %n", side, &size, &pop, &show, &used) >= 4 && val[used])
            {
                for (n = 0; n < MP_SPEC_FRIENDS && gMpConfig.specFriends[n].name[0]; n++)
                    ;
                if (n < MP_SPEC_FRIENDS)
                {
                    struct MpSpecFriend *fr = &gMpConfig.specFriends[n];
                    static const char *const sides[] = {"below", "above", "left", "right"};
                    snprintf(fr->name, sizeof(fr->name), "%s", val + used);
                    fr->side = MP_SPEC_BELOW;
                    for (i = 0; i < 4; i++)
                        if (strcmp(side, sides[i]) == 0)
                            fr->side = i;
                    fr->size = size < 1 ? 1 : (size > 4 ? 4 : size);
                    fr->popout = pop != 0;
                    fr->show = show != 0;
                }
            }
        }
        else if (strcmp(key, "spectate_size") == 0)
        {
            int v = atoi(val);
            gMpConfig.spectateScale = v < 1 ? 1 : (v > 4 ? 4 : v);
        }
        else if (strcmp(key, "battle_anim_speed") == 0)
        {
            int v = atoi(val);
            gMpConfig.animSpeed = v >= 30 ? 30 : (v >= 25 ? 25 : 0);
        }
        else if (strcasecmp(key, "AllowSmallTweaks") == 0) gMpConfig.smallTweaks = atoi(val) == 1;
        else if (strcmp(key, "follower") == 0) gMpConfig.followerPersonality = (u32)strtoul(val, NULL, 16);
        else if (strcmp(key, "quick_stock") == 0)
        {
            char *p = val;
            int i;
            for (i = 0; i < MP_QUICK_STOCK_SLOTS; i++)
                gMpConfig.quickStock[i] = (u8)strtoul(p, &p, 10) % MP_Emote_QuickPhraseCount();
        }
        else if (strncmp(key, "phrase", 6) == 0 && atoi(key + 6) >= 1 && atoi(key + 6) <= MP_NUM_PHRASES)
        {
            char *p = val;
            int w, n = atoi(key + 6) - 1;
            for (w = 0; w < MP_PHRASE_WORDS; w++)
                gMpConfig.phrases[n][w] = (u16)strtoul(p, &p, 16);
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
        "; a character who rides a Pokemon or a vehicle on a bike: ride (as drawn) or bike (a bicycle)\n"
        "bike = %s\n"
        "; how many tiles a player holding the BATTLE FLAG can see\n"
        "trainer_range = %d\n"
        "; the party Moemon that follows you (SELECT in the party menu); 0 = none\n"
        "follower = %08x\n"
        "; save quietly on the map every this many minutes, at safe moments (0 = off)\n"
        "autosave = %d\n"
        "; remembered for the next session\n"
        "last_room = %s\nlast_room_key = %s\nlast_host_name = %s\n"
        "; where that room is (empty: the online lobby; home: hosted on this PC) and the code that got you in\n"
        "last_room_address = %s\nlast_join_code = %s\n"
        "; shown to friends who join a server you host (HOST A GAME > WELCOME NOTE); | starts line 2\n"
        "welcome = %s\n"
        "; servers you host: co-op trainers get one extra Moemon, so Double / Triple Battles work\n"
        "; against smaller teams (HOST A GAME > TEAM PADDING; 0 = off)\n"
        "trainer_padding = %d\n"
        "; OPTIONS > GAME > LEVEL SCALING: wild Pokemon and trainers keep up with your team (online, the host's counts)\n"
        "level_scaling = %d\n"
        "; its settings: wild (0 off, 1 low, 2 medium, 3 high), trainers (0 off, 1 below, 2 even, 3 above), boss margin\n"
        "; (0 auto, 1-9 = +0 to +8), badge caps (0 / 1), past 8 badges (0 capped until the League, 1 free, 2 always capped)\n"
        "level_scaling_rules = %d %d %d %d %d\n"
        "; servers you host (and playing alone): battle animations this much faster, 0, 25 or 30 (percent)\n"
        "battle_anim_speed = %d\n"
        "; CO-OP > SPECTATE: friends' screens in a panel under yours (0 / 1), each this many times (1-4)\n"
        "spectate_panel = %d\nspectate_size = %d\n"
        "; OPTIONS > GAME: the cycling song on a bike, the surfing / underwater songs (1) or the map's own (0)\n"
        "bike_music = %d\nsurf_music = %d\n"
        "; OPTIONS > CO-OP > MOENAV CALLS: all, or battles (only trainers asking for a battle; story calls always come)\n"
        "moenav_calls = %s\n"
        "; OPTIONS > GAME > FOLLOWERS: default (the original sprites on the map, PMD in the MOEMON GARDEN), pmd or original\n"
        "follower_sprites = %s\n"
        "; advanced: use a different online lobby server (address or address:port)\n"
        "lobby = %s\n"
        "; your 32 CO-OP phrases (made in CO-OP menu > EMOTES; word codes, ffff = empty)\n",
        gMpConfig.name, sprite, gMpConfig.customSprite, skin, outfit, accent, hair,
        MP_Avatar_FrontSelConfigName(gMpConfig.app.frontSel), MP_Avatar_BackSelConfigName(gMpConfig.app.backSel),
        gMpConfig.app.bike ? "bike" : "ride", gMpConfig.trainerRange, (unsigned)gMpConfig.followerPersonality, MOEMON_DEV_BUILD ? sMainAutosaveMinutes : gMpConfig.autosaveMinutes,
        gMpConfig.lastRoom, gMpConfig.lastRoomKey, gMpConfig.lastHostName, gMpConfig.lastRoomAddr, gMpConfig.lastJoinCode, gMpConfig.welcome, gMpConfig.trainerPadding, gMpConfig.levelScaling,
        gMpConfig.scale[0], gMpConfig.scale[1], gMpConfig.scale[2], gMpConfig.scale[3], gMpConfig.scale[4], gMpConfig.animSpeed,
        gMpConfig.spectatePanel, gMpConfig.spectateScale, gMpConfig.bikeMusic, gMpConfig.surfMusic,
        gMpConfig.moenavCalls ? "battles" : "all",
        gMpConfig.followerSprites == MP_FOLLOW_SPRITES_PMD ? "pmd" : gMpConfig.followerSprites == MP_FOLLOW_SPRITES_ORIGINAL ? "original" : "default",
        gMpConfig.lobby);
    {
        int i;
        for (i = 0; i < MP_NUM_PHRASES; i++)
            fprintf(f, "phrase%d = %04x %04x %04x %04x\n", i + 1, gMpConfig.phrases[i][0], gMpConfig.phrases[i][1],
                gMpConfig.phrases[i][2], gMpConfig.phrases[i][3]);
        fprintf(f, "; ready-made phrases R2 offers for empty phrase slots (0-%d)\nquick_stock = %d %d %d %d\n",
            MP_Emote_QuickPhraseCount() - 1, gMpConfig.quickStock[0], gMpConfig.quickStock[1],
            gMpConfig.quickStock[2], gMpConfig.quickStock[3]);
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
    {
        static const char *const sides[] = {"below", "above", "left", "right"};
        int i;
        fprintf(f, "; CO-OP > SPECTATE, each friend's screen: side (below, above, left, right), size (1-4), own window (0 / 1),\n"
                   "; shown (0 / 1), their name\n");
        for (i = 0; i < MP_SPEC_FRIENDS; i++)
            if (gMpConfig.specFriends[i].name[0])
                fprintf(f, "spectate_friend = %s %d %d %d %s\n", sides[gMpConfig.specFriends[i].side & 3],
                        gMpConfig.specFriends[i].size, gMpConfig.specFriends[i].popout, gMpConfig.specFriends[i].show,
                        gMpConfig.specFriends[i].name);
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
    PutU16(p + 20, s->followerFx & 0xFFFF);
    p[22] = s->light;
    p[23] = s->followerFx >> 16;
    PutU16(p + 24, (u16)s->followerHue);
    PutU32(p + 26, s->followerKey);
    p[30] = s->lifeAct; p[31] = s->lifePose; p[32] = (u8)s->lifeDx; p[33] = (u8)s->lifeDy;
    p[34] = s->lifeEmote; p[35] = s->lifeEmoteSeq;
    p[36] = (u8)s->lifePx; p[37] = (u8)s->lifePy; p[38] = s->lifeFace;
    p[39] = s->top3;
}

static void UnpackState(const u8 *p, struct MpState *s)
{
    s->mapGroup = p[0]; s->mapNum = p[1];
    s->x = (s16)GetU16(p + 2); s->y = (s16)GetU16(p + 4);
    s->facing = p[6]; s->elevation = p[7]; s->avatarMode = p[8]; s->speed = p[9];
    s->flags = p[10]; s->topLevel = p[11]; s->partyCount = p[12]; s->emote = p[13];
    s->seq = GetU16(p + 14);
    s->followerDex = GetU16(p + 16); s->followerFlags = p[18]; s->badges = p[19];
    s->followerFx = GetU16(p + 20) | ((u32)p[23] << 16);
    s->light = p[22];
    s->followerHue = (s16)GetU16(p + 24);
    s->followerKey = GetU32(p + 26);
    s->lifeAct = p[30]; s->lifePose = p[31]; s->lifeDx = (s8)p[32]; s->lifeDy = (s8)p[33];
    s->lifeEmote = p[34]; s->lifeEmoteSeq = p[35];
    s->lifePx = (s8)p[36]; s->lifePy = (s8)p[37]; s->lifeFace = p[38];
    s->top3 = p[39];
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
    MP_Created_Pack(&a->recipe, p + 13);   // MP_CREATED_BYTES (14): bytes 13-26, FIELD HAIR last
    p[27] = a->bike;
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
    a->bike = p[27] ? 1 : 0;
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

// Caves and dungeons: no paid rescue patch-ups (mp_rescue.c) and no shared PC (mp_sharedpc.c)
bool8 MP_InDungeon(void)
{
    static const u8 sDungeons[] = {
        MAPSEC_GRANITE_CAVE, MAPSEC_RUSTURF_TUNNEL, MAPSEC_ABANDONED_SHIP, MAPSEC_NEW_MAUVILLE, MAPSEC_METEOR_FALLS,
        MAPSEC_METEOR_FALLS2, MAPSEC_MT_PYRE, MAPSEC_AQUA_HIDEOUT_OLD, MAPSEC_SHOAL_CAVE, MAPSEC_SEAFLOOR_CAVERN,
        MAPSEC_UNDERWATER_SEAFLOOR_CAVERN, MAPSEC_VICTORY_ROAD, MAPSEC_CAVE_OF_ORIGIN, MAPSEC_FIERY_PATH, MAPSEC_FIERY_PATH2,
        MAPSEC_SEALED_CHAMBER, MAPSEC_UNDERWATER_SEALED_CHAMBER, MAPSEC_SCORCHED_SLAB, MAPSEC_ISLAND_CAVE,
        MAPSEC_DESERT_RUINS, MAPSEC_ANCIENT_TOMB, MAPSEC_SKY_PILLAR, MAPSEC_AQUA_HIDEOUT, MAPSEC_MAGMA_HIDEOUT,
        MAPSEC_MIRAGE_TOWER, MAPSEC_ARTISAN_CAVE, MAPSEC_MARINE_CAVE, MAPSEC_UNDERWATER_MARINE_CAVE, MAPSEC_TERRA_CAVE,
        MAPSEC_DESERT_UNDERPASS, MAPSEC_ALTERING_CAVE,
    };
    u32 i;
    if (gMapHeader.mapType == MAP_TYPE_UNDERGROUND)
        return TRUE;
    for (i = 0; i < ARRAY_COUNT(sDungeons); i++)
        if (gMapHeader.regionMapSectionId == sDungeons[i])
            return TRUE;
    return FALSE;
}

bool8 MP_IsInOverworld(void)
{
    return gMain.callback2 == CB2_Overworld;
}

bool8 MP_IsBattleTransitionRunning(void);   // battle_transition.c

bool8 MP_SmallTweaksAllowed(void)
{
    return gMpConfig.smallTweaks;
}

bool8 MP_IsLocalFree(void)
{
    if (Tcg_IsActive())
        return FALSE;
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
    s->flags &= (u8)~(MPF_FREE | MPF_IN_OVERWORLD | MPF_IN_BATTLE | MPF_TRAINER_MODE | MPF_ENGAGED | MPF_CHALLENGED | MPF_RIDING | MPF_IN_MENU);

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
        s->light = GetFlashLevel();   // friends draw our light in a dark cave (mp_world.c)
        if (MP_IsLocalFree())
            s->flags |= MPF_FREE;
    }
    // From the moment the battle transition starts, so others see it right away
    if (MP_Battle_IsActive() || gMain.inBattle || MP_IsBattleTransitionRunning())
        s->flags |= MPF_IN_BATTLE;
    // In a menu: the START menu, the CO-OP menu (and its windows), a shop or a script's choice menu on the field, or any
    // other screen (bag, party, PC boxes, POKéDEX...) that isn't a battle or a map load
    if (!(s->flags & MPF_IN_BATTLE) && !gMain.inBattle)
    {
        if (MP_IsInOverworld() ? (MP_IsStartMenuOpen() || MP_IsScriptMenuOpen() || MP_IsShopMenuOpen()
                                  || MP_Menu_IsCoopMenuOpen() || MpUi_IsOpen())
                               : !MP_IsFieldLoadCallback())
            s->flags |= MPF_IN_MENU;
    }
    if (MP_Coop_IsChallenged())
        s->flags |= MPF_CHALLENGED;
    if (MP_Piggy_IsRiding())
    {
        s->flags |= MPF_RIDING;
        s->speed = MP_Piggy_CarrierSpeed();
    }
    if (MP_Trainer_IsActive())
        s->flags |= MPF_TRAINER_MODE;
    if (MP_Trainer_IsEngaged() || MP_Battle_IsActive())
        s->flags |= MPF_ENGAGED;
    s->topLevel = MP_GetPartyTopLevel();
    s->top3 = MP_LevelScale_Top3();
    s->badges = MP_Badges();
    s->partyCount = gPlayerPartyCount;
    s->emote = (sEmoteCounter << 4) | sEmoteKind;
    {
        bool8 female;
        u8 sound;
        s->followerDex = MP_Follower_LocalDex(&female);
        s->followerFx = MP_Follower_LocalFx(&sound);
        s->followerHue = s->followerDex ? MP_Follower_LocalHue() : 0;
        s->followerFlags = (female ? 1 : 0) | ((sound & 7) << 1) | ((MP_Follower_LocalToggleSeq() & 3) << 4) | ((sound & 8) ? 0x40 : 0)
                         | (s->followerDex && MP_Follower_LocalShiny() ? 0x80 : 0);
    }
    MP_LifeSocial_CollectState(s);   // MOéMON GARDEN: the follower's own life
    sLastLocalState = *s;
}

static bool8 StateDiffers(const struct MpState *a, const struct MpState *b)
{
    return a->mapGroup != b->mapGroup || a->mapNum != b->mapNum || a->x != b->x || a->y != b->y
        || a->facing != b->facing || a->elevation != b->elevation || a->avatarMode != b->avatarMode
        || a->speed != b->speed || a->flags != b->flags || a->topLevel != b->topLevel
        || a->emote != b->emote || a->followerDex != b->followerDex || a->followerFlags != b->followerFlags
        || a->followerFx != b->followerFx || a->followerHue != b->followerHue
        || a->badges != b->badges || a->followerKey != b->followerKey || a->lifeAct != b->lifeAct
        || a->lifePose != b->lifePose || a->lifeDx != b->lifeDx || a->lifeDy != b->lifeDy || a->lifeEmoteSeq != b->lifeEmoteSeq
        || a->lifePx != b->lifePx || a->lifePy != b->lifePy || a->lifeFace != b->lifeFace || a->top3 != b->top3;
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
    gMpPlayers[id].name[0] = EOS;   // zeros are spaces in the game's text: a name needs its end marker
}

static void HandlePlayerLeft(u8 id)
{
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return;
    MPNet_Log("player %d left", id + 1);
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
    MP_Join_OnPlayerLeft(id);
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
        if (data[0] >= MP_MAX_PLAYERS)
            return;
        gMpLocalId = data[0];
        gMpPlayers[gMpLocalId].name[0] = EOS;   // our own entry is never filled in
        snprintf(sStatus, sizeof(sStatus), "ONLINE (PLAYER %d)", gMpLocalId + 1);
        MPNet_Log("joined \"%s\" as player %d", sLobby.room, gMpLocalId + 1);
        SendLocalBlobs();
        SendLocalState(TRUE);
        break;
    case MPMSG_S_REJECT:
        MPNet_Log("server rejected us (%d)", len ? data[0] : 0);
        if (len && data[0] == MP_REJECT_NO_ROOM)
            sHelloSent = FALSE; // we'll say hello again after rejoining
        if (len && data[0] == MP_REJECT_VERSION)
        {
            MP_Lobby_SetOtherVersion(len >= 3 ? data[1] | (data[2] << 8) : 0);
            SetError(MP_LOBBY_ERR_VERSION);
            sLobby.wantRoom = FALSE;
            sLobby.reconnecting = FALSE;
        }
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
        {
            memset(p, 0, sizeof(*p));
            p->name[0] = EOS;
        }
        p->present = TRUE;
        if (!p->hasInfo)
            SendWelcome(id);   // someone new: the host's welcome note
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
        {
            memset(p, 0, sizeof(*p));
            p->name[0] = EOS;
        }
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
    if (!len) return;
    if (data[0] == 255)
        Tcg_OnlineRelay(from, data, len);
    else if (data[0] == MPR_COOP_BATTLING)
        MP_Coop_OnRelay(from, data, len);
    else if (data[0] >= MPR_LINK_PLAYER && data[0] < MPR_SPECTATE_FIRST)
        MP_Battle_OnRelay(from, data, len);
    else if (data[0] >= MPR_SPECTATE_FIRST && data[0] < MPR_COOP_FIRST)
        MP_Spectate_OnRelay(from, data, len);
    else if (data[0] >= MPR_COOP_FIRST && data[0] < MPR_COOP_FIRST + 20)
        MP_Coop_OnRelay(from, data, len);
    else if (data[0] == MPR_SAY)
        MP_Emote_OnRelay(from, data, len);
    else if (data[0] == MPR_WELCOME)
        ShowWelcome(from, data + 1, len - 1);
    else if (data[0] == MPR_RULES)
    {
        if (len >= 2 && !sLobby.creator)
        {
            sRoomPadding = data[1] != 0;
            sRoomAnimSpeed = len >= 3 ? (data[2] > 30 ? 30 : data[2]) : 30;
            sRoomLevelScaling = len >= 4 ? data[3] != 0 : TRUE;
            {
                u8 k;
                for (k = 0; k < MP_SCALE_COUNT - 1; k++)
                    sRoomScale[k] = len >= 5 + k && data[4 + k] < MP_LevelScaleRuleCount(k + 1) ? data[4 + k] : gMpConfig.scale[k];
            }
            sRoomRulesHeard = TRUE;
            sRoomHostId = from;
            MPNet_Log("room rules from player %d: team padding %s, anim speed +%d%%, level scaling %s", from + 1,
                      sRoomPadding ? "on" : "off", sRoomAnimSpeed, sRoomLevelScaling ? "on" : "off");
        }
    }
    else if (data[0] == MPR_CLOCK)
    {
        if (len >= 9 && !sLobby.creator)
        {
            s64 hostLocal = 0;
            u8 k;
            for (k = 0; k < 8; k++)
                hostLocal |= (s64)data[1 + k] << (8 * k);
            if (!sHostClockHeard || llabs((long long)(hostLocal - (s64)time(NULL) - sHostClockOffset)) > 2)
                MPNet_Log("clock: following the host's clock (UTC%+lld min)", (long long)((hostLocal - (s64)time(NULL) + 30) / 60));
            sHostClockOffset = hostLocal - (s64)time(NULL);
            sHostClockHeard = TRUE;
        }
    }
    else if (data[0] >= MPR_LIFE_FIRST && data[0] < MPR_LIFE_FIRST + 3)
        MP_LifeSocial_OnRelay(from, data, len);
    else if (data[0] == MPR_PING)
        MP_Ping_OnRelay(from, data, len);
    else if (data[0] == MPR_PHOTO)
        MP_Photo_OnRelay(from, data, len);
    else if (data[0] == MPR_LEND)
        MP_Lend_OnRelay(from, data, len);
    else if (data[0] == MPR_RESCUE)
        MP_Rescue_OnRelay(from, data, len);
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
    else if (data[0] >= MPR_WORLD_FIRST && data[0] < MPR_WORLD_FIRST + 2)
        MP_World_OnRelay(from, data, len);
    else if (data[0] >= MPR_GATCHA_FIRST && data[0] < MPR_GATCHA_FIRST + 4)
        MP_Gatcha_OnRelay(from, data, len);
    else if (data[0] >= MPR_JOIN_FIRST && data[0] < MPR_JOIN_FIRST + 8)
        MP_Join_OnRelay(from, data, len);
    else if (data[0] >= MPR_GARDEN_FIRST && data[0] < MPR_GARDEN_FIRST + 2)
        MP_GardenNet_OnRelay(from, data, len);
    else if (data[0] >= MPR_FARM_FIRST && data[0] < MPR_FARM_FIRST + 3)
        MP_Farm_OnRelay(from, data, len);
    else if (data[0] < MPR_LINK_PLAYER)
        MP_Trainer_OnRelay(from, data, len);
}

// ---------------------------------------------------------------------------
// Lobby

u8 MP_Lobby_State(void) { return sLobby.state; }
u8 MP_Lobby_Error(void) { return sLobby.error; }
const char *MP_Lobby_RoomName(void) { return sLobby.room; }
// ---------------------------------------------------------------------------
// Welcome note: the player who made the room sends it to each friend who joins

// This PC's local wall clock, as seconds as if it were UTC (so gmtime() gives the local
// date and time). Worked out field by field: Windows has no tm_gmtoff.
s64 MP_LocalClockAsUtc(void)
{
    time_t now = time(NULL);
    struct tm lt = *localtime(&now), gt = *gmtime(&now);
    s64 days = lt.tm_yday - gt.tm_yday;
    if (lt.tm_year != gt.tm_year)
        days = lt.tm_year > gt.tm_year ? 1 : -1;   // across New Year
    return (s64)now + days * 86400 + (lt.tm_hour - gt.tm_hour) * 3600 + (lt.tm_min - gt.tm_min) * 60 + (lt.tm_sec - gt.tm_sec);
}

// The room's host tells everyone its clock (when they join, and now and then)
static void SendClock(u8 to)
{
    u8 msg[9], k;
    s64 local = MP_LocalClockAsUtc();
    msg[0] = MPR_CLOCK;
    for (k = 0; k < 8; k++)
        msg[1 + k] = (u8)(local >> (8 * k));
    MP_SendRelay(to, msg, sizeof(msg));
}

// Online (in a room we didn't make), the game clock is the host's
bool8 MP_HostClockOffset(s64 *offset)
{
    if (sLobby.creator || !sHostClockHeard)
        return FALSE;
    if (sLobby.state != MP_LOBBY_IN_ROOM && sLobby.state != MP_LOBBY_RECONNECT_WAIT)
    {
        sHostClockHeard = FALSE;   // offline: our own clock again
        return FALSE;
    }
    *offset = sHostClockOffset;
    return TRUE;
}

// How many seconds the game clock is ahead of this PC's own (the host's clock minus ours; 0 offline)
s64 MP_ClockShift(void)
{
    s64 offset;
    if (!MP_HostClockOffset(&offset))
        return 0;
    return offset - (MP_LocalClockAsUtc() - (s64)time(NULL));
}

static void SendWelcome(u8 id)
{
    u8 buf[2 + sizeof(gMpConfig.welcome)];
    u16 len = strlen(gMpConfig.welcome);
    if (sLobby.creator)
    {
        // the room's settings first (the host decides them for everyone)
        u8 rules[4 + MP_SCALE_COUNT - 1] = {MPR_RULES, OwnPadding(), OwnAnimSpeed(), OwnLevelScaling()};
        LevelRulesFromEnv();
        memcpy(rules + 4, gMpConfig.scale, MP_SCALE_COUNT - 1);
        MP_SendRelay(id, rules, sizeof(rules));
        SendClock(id);
    }
    if (!sLobby.creator || len == 0 || strspn(gMpConfig.welcome, " |") == len)
        return;
    buf[0] = MPR_WELCOME;
    memcpy(buf + 1, gMpConfig.welcome, len);
    MP_SendRelay(id, buf, 1 + len);
}

static void ShowWelcome(u8 from, const u8 *text, u16 len)
{
    char note[sizeof(gMpConfig.welcome)], name[16], msg[128], *bar;
    if (from >= MP_MAX_PLAYERS || len == 0)
        return;
    if (len >= sizeof(note))
        len = sizeof(note) - 1;
    memcpy(note, text, len);
    note[len] = 0;
    bar = strchr(note, '|');
    if (bar)
        *bar = '\n';
    MP_GameToAscii(name, gMpPlayers[from].name, MP_NAME_LEN);
    snprintf(msg, sizeof(msg), "A note from %s, your host:\f%s", name, note);
    MP_ShowNotice(msg);
}

bool8 MP_Lobby_IsLocalHost(void) { return sLobby.isLocal; }

bool8 MP_PmdWanted(bool8 garden)
{
    static bool8 sEnvRead;
    u8 mode;
    if (!sEnvRead)
    {
        sEnvRead = TRUE;
        if (getenv("MOEMON_FOLLOWER_SPRITES"))   // tests: the starting value (0 default, 1 pmd, 2 original)
            gMpConfig.followerSprites = atoi(getenv("MOEMON_FOLLOWER_SPRITES")) % MP_FOLLOW_SPRITES_COUNT;
    }
    mode = gMpConfig.followerSprites;
    return mode == MP_FOLLOW_SPRITES_PMD || (mode == MP_FOLLOW_SPRITES_DEFAULT && garden);
}
bool8 MP_Lobby_IsCreator(void) { return sLobby.creator && sLobby.state == MP_LOBBY_IN_ROOM; }

// The room's host (who made it), heard with its rules when we joined: while they're still here
s8 MP_RoomHostId(void)
{
    if (sLobby.creator || !sRoomRulesHeard || sRoomHostId < 0 || sRoomHostId >= MP_MAX_PLAYERS || sRoomHostId == gMpLocalId
     || (sLobby.state != MP_LOBBY_IN_ROOM && sLobby.state != MP_LOBBY_RECONNECT_WAIT) || !gMpPlayers[sRoomHostId].present)
        return -1;
    return sRoomHostId;
}

// TEAM PADDING: the host's choice, heard when we joined (our own when we made the room,
// before we hear it, or with nobody around)
static bool8 OwnPadding(void)
{
    if (getenv("MOEMON_TRAINER_PADDING"))   // tests
        return atoi(getenv("MOEMON_TRAINER_PADDING")) != 0;
    return gMpConfig.trainerPadding;
}

static u8 OwnAnimSpeed(void)
{
    if (getenv("MOEMON_ANIM_SPEED"))   // tests
        return atoi(getenv("MOEMON_ANIM_SPEED"));
    return gMpConfig.animSpeed;
}

u8 MP_BattleAnimSpeed(void)
{
    if (!sLobby.creator && sRoomRulesHeard)
        return sRoomAnimSpeed;
    return OwnAnimSpeed();
}

static bool8 OwnLevelScaling(void)
{
    if (getenv("MOEMON_LEVEL_SCALING"))   // tests
        return atoi(getenv("MOEMON_LEVEL_SCALING")) != 0;
    return gMpConfig.levelScaling;
}

bool8 MP_LevelScalingIsOurs(void)   // not a joiner who heard the host's rules
{
    return sLobby.creator || !sRoomRulesHeard;
}

u8 MP_LevelScaleRuleCount(u8 which)
{
    static const u8 sCounts[MP_SCALE_COUNT] = {2, 4, 4, 10, 2, 3};
    return which < MP_SCALE_COUNT ? sCounts[which] : 1;
}

// Tests: MOEMON_LEVEL_RULES="WILD TRAINERS BOSS CAPS PAST8" sets ours at the start
static void LevelRulesFromEnv(void)
{
    static bool8 sDone;
    int v[MP_SCALE_COUNT - 1], k;
    if (sDone)
        return;
    sDone = TRUE;
    if (getenv("MOEMON_LEVEL_RULES") && sscanf(getenv("MOEMON_LEVEL_RULES"), "%d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4]) == 5)
        for (k = 0; k < MP_SCALE_COUNT - 1; k++)
            if (v[k] >= 0 && v[k] < MP_LevelScaleRuleCount(k + 1))
                gMpConfig.scale[k] = v[k];
}

u8 MP_LevelScaleOwnRule(u8 which)
{
    LevelRulesFromEnv();
    if (which == MP_SCALE_ON)
        return OwnLevelScaling();
    return which < MP_SCALE_COUNT ? gMpConfig.scale[which - 1] : 0;
}

u8 MP_LevelScaleRule(u8 which)
{
    LevelRulesFromEnv();
    if (which == MP_SCALE_ON)
        return MP_LevelScaling();
    if (which >= MP_SCALE_COUNT)
        return 0;
    if (!sLobby.creator && sRoomRulesHeard)
        return sRoomScale[which - 1];
    return gMpConfig.scale[which - 1];
}

void MP_LevelScaleSetOwn(u8 which, u8 value)
{
    if (which >= MP_SCALE_COUNT || value >= MP_LevelScaleRuleCount(which))
        return;
    if (which == MP_SCALE_ON)
        gMpConfig.levelScaling = value;
    else
        gMpConfig.scale[which - 1] = value;
    MP_SaveConfig();
    MP_SendRoomRules();
}

bool8 MP_LevelScaling(void)
{
    if (!sLobby.creator && sRoomRulesHeard)
        return sRoomLevelScaling;
    return OwnLevelScaling();
}

// The host changed a room setting (OPTIONS): everyone hears it again
void MP_SendRoomRules(void)
{
    u8 rules[4 + MP_SCALE_COUNT - 1];
    u8 i;
    if (!sLobby.creator || sLobby.state != MP_LOBBY_IN_ROOM)
        return;
    rules[0] = MPR_RULES;
    rules[1] = OwnPadding();
    rules[2] = OwnAnimSpeed();
    rules[3] = OwnLevelScaling();
    LevelRulesFromEnv();
    memcpy(rules + 4, gMpConfig.scale, MP_SCALE_COUNT - 1);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present)
            MP_SendRelay(i, rules, sizeof(rules));
}

bool8 MP_TrainerPadding(void)
{
    if (!sLobby.creator && sRoomRulesHeard)
        return sRoomPadding;
    return OwnPadding();
}
bool8 MP_Lobby_InRoom(void) { return sLobby.state == MP_LOBBY_IN_ROOM; }

void MP_Lobby_SetOtherVersion(u16 version) { sOtherVersion = version; }
u16 MP_Lobby_OtherVersion(void) { return sOtherVersion; }

static void SetError(u8 err)
{
    if (err == MP_LOBBY_ERR_VERSION)
        MPNet_Log("version mismatch: ours %d, theirs %d", MP_PROTOCOL_VERSION, sOtherVersion);
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

// The server we're connected to, "host:port" (FALSE: the online lobby, or none)
bool8 MP_Lobby_ServerAddress(char *out, int len)
{
    char host[128];
    int port = MP_DEFAULT_PORT;
    if (sLobby.host[0] == 0)
        return FALSE;
    if (MP_OnlineLobbyAddress()[0])
    {
        MPNet_ParseAddress(MP_OnlineLobbyAddress(), host, sizeof(host), &port);
        if (strcmp(host, sLobby.host) == 0 && port == sLobby.port)
            return FALSE;
    }
    snprintf(out, len, "%s:%d", sLobby.host, sLobby.port);
    return TRUE;
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
    sLobby.creator = (req == REQ_CREATE);
    sRoomRulesHeard = FALSE;
    sRoomHostId = -1;
    sHostClockHeard = FALSE;
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
        case MP_ROOM_ERR_VERSION:
            MP_Lobby_SetOtherVersion(len >= 3 ? data[1] | (data[2] << 8) : 0);
            SetError(MP_LOBBY_ERR_VERSION);
            break;
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
    sMainAutosaveMinutes = gMpConfig.autosaveMinutes;
    if (MOEMON_DEV_BUILD)
        gMpConfig.autosaveMinutes = 0;
    MPNet_Init();
    MP_Avatar_Init();
    snprintf(sStatus, sizeof(sStatus), "OFFLINE");
}

// After the game's frame: send what it queued right away and let a server
// hosted in this game pass it on, so replies don't wait for the next frame
void MP_FrameEnd(void)
{
    if (sLobby.state == MP_LOBBY_OFF)
        return;
    MPNet_ClientFlush();
    MPServer_Poll();
}

void MP_FrameUpdate(void)
{
    u8 type;
    u16 len;
    int guard;

    Tcg_OnlineTick();
    MP_Achieve_Update();        // achievements are personal: offline too
    MP_Autosave_Update();       // quiet saves on the map
    MP_Events_UpdateToasts();   // ... and so are their notes
    MP_ItemDesc_Update();       // the next page of an item's description too long for its window (mp_itemdesc.c)
    MpUi_RefreshIcons();        // help lines follow keyboard / pad switches

    if (!sInitDone)
        MP_Init();
    MP_Qol_Update();
    MP_ReflectionPalettes_Sweep();   // reflection palettes no sprite shows any more
    MP_Pmd_FrameUpdate();            // PMD sheets nobody shows any more (mp_pmd.c)
    MP_Candy_Update();          // PROF. BIRCH brings the CANDY JAR after the first release
    MP_TmRefund_Update();       // the LOTTERY CORNER clerk brings back TMs used up before they became reusable
    MP_Life_FrameUpdate();      // MOéMON GARDEN: needs, feelings, places (mp_life.c)
    MP_LifeSocial_Update();     // ... and among friends (mp_life_social.c)
    MP_Garden_FrameUpdate();    // the MOéMON GARDEN (mp_garden.c, mp_garden_life.c)
    MP_SharedBox_Update();      // the shared PC's boxes: what changed goes to the server
    MP_Social_Update();
    MP_Together_Update();
    MP_Linkx_Update();
    MP_World_Update();
    MP_Minigame_Update();
    MP_Group_Update();

    MPServer_Poll();
    MP_AutoSession_Update();
    if (gMain.inBattle)
        MP_Emote_BattleUpdate();   // battle emote bubbles (solo battles too)
    MP_LifeBattle_Update();        // MOéMON GARDEN in battle: moments, bonds, Pokémon emotes, rare lines
    if (!(MPNet_ClientStatus() == MPNET_CONNECTED && gMpLocalId >= 0))
        MP_Events_Update();        // daily goals alone (online: below)

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
    // The host's clock again every few minutes (a PC clock can be changed or change for summer time)
    if (sLobby.creator && sLobby.state == MP_LOBBY_IN_ROOM && gMpLocalId >= 0 && ++sClockResendTimer >= 60 * 60 * 5)
    {
        sClockResendTimer = 0;
        SendClock(0xFF);
    }
    if (gMpLocalId >= 0)
    {
        MP_Coop_Update();
        MP_Join_Update();
        MP_Dex_Update();
        MP_Trade_Update();
        MP_Gatcha_VoteUpdate();
        MP_Events_Update();
        MP_Tourney_Update();
        SendLocalState(FALSE);
        MP_Spectate_FrameUpdate();
    }

    // flush now so input latency stays low
    MPNet_ClientPoll(&type, sMsgBuf, &len) ? HandleMessage(type, sMsgBuf, len) : (void)0;
}
