// Moemon Rhythm: the DANCE machine's credit, screen by screen (docs/RHYTHM_DECISIONS.md #4, #5, #13, #14):
// CAUTION, SELECT STYLE, SELECT DIFFICULTY, SELECT MUSIC (the wheel, the song's preview), OPTIONS, the stage's
// title, the song (READY, HERE WE GO, CLEARED / FAILED), RESULTS, then the next stage or GAME OVER. Four stages a
// credit; a failed stage ends it after its results.
//
// Two players on one machine online: each game plays its own field on its own clock and tells the other what it
// stepped (RN_STATE ~10 times a second), so the friend's field is drawn from that. 1P (the picker) moves the wheel;
// the screens wait for both where both choose (difficulty, options, results).
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#endif
#include <SDL2/SDL.h>
#include "global.h"
#include "rhythm.h"
#include "platform/crt.h"
#include "rh.h"
#include "rh_songs.h"

bool8 Platform_IsTestMode(void);
void MPNet_Log(const char *fmt, ...);
// the songs' download (src/platform/updater.c)
enum { SONGS_IDLE, SONGS_CHECKING, SONGS_AVAILABLE, SONGS_UPTODATE, SONGS_NONE, SONGS_DOWNLOADING, SONGS_INSTALLING,
       SONGS_DONE, SONGS_FAILED };
void Updater_SongsCheck(void);
void Updater_SongsDownload(void);
void Updater_SongsCancel(void);
int Updater_SongsState(long *done, long *total, const char **error);

enum
{
    ST_OFF,
    ST_CAUTION,
    ST_STYLE,
    ST_DIFFICULTY,
    ST_MUSIC,
    ST_OPTIONS,
    ST_STAGE,
    ST_PLAY,
    ST_SONG_END,
    ST_RESULTS,
    ST_GAMEOVER,
    ST_NOSONGS,
    ST_CLOSING,     // the credit is over: the screen turns off like a CRT, then the map is back
};

// The machine's screen turns on and off like a CRT (the VS cabinet's effect: src/platform/crt_effect.c)
#define CRT_ON_FRAMES  24
#define CRT_OFF_FRAMES 24
static uint16_t sLastFrame[RH_W * RH_H], sCrtFrame[RH_W * RH_H];

// relay messages of a session (after mp_rhythm.c's own type byte)
enum
{
    RN_HELLO = 1,   // u8 side, u16 count, u32 song keys... (what we have)
    RN_DIFF,        // u8 diff, u8 confirmed, RhOptions (10 bytes)
    RN_WHEEL,       // u32 song key (the picker's wheel)
    RN_PICK,        // u32 song key
    RN_READY,       // u8 diff, RhOptions
    RN_GO,          // u32 song key, u8 stage
    RN_STATE,       // u16 seq, i32 score, u16 combo, u16 maxCombo, u8 life (0-200), u8 failed|finished<<1,
                    // u8 held, u8 n, (u16 note, u8 kind, u8 judge) * n: their events n back from seq
    RN_RESULT,      // the counts (results screen)
    RN_NEXT,        // pressed on, from the results
    RN_QUIT,        // left the machine
};

#define LEAD_IN 2.0             // seconds of the song's clock before the music
#define MAX_PRESSES 64
#define NET_STATE_EVERY 6       // frames
#define PEER_SILENCE 10.0       // seconds: the friend is gone

static const char *const sDiffNames[RH_DIFFS] = {"BEGINNER", "LIGHT", "STANDARD", "HEAVY", "CHALLENGE"};
static const char *const sCardNames[RH_DIFFS] = {"card_beginner", "card_light", "card_standard", "card_heavy", "card_oni"};
static const uint32_t sDiffColors[RH_DIFFS] = {
    RH_RGB(120, 220, 255), RH_RGB(255, 220, 60), RH_RGB(255, 80, 80), RH_RGB(80, 230, 80), RH_RGB(200, 120, 255),
};

enum { SORT_GROUP, SORT_ABC, SORT_BPM, SORT_COUNT };
static const char *const sSortNames[SORT_COUNT] = {"SERIES", "ABC", "BPM"};

struct WheelItem
{
    int16_t group;              // a section header (song -1)
    int16_t song;
};

static struct
{
    int state;
    float t;                    // seconds in this state
    double last;                // the PC's clock at the last frame
    struct RhythmBegin b;
    int versus;                 // a friend is on the other pad
    int peerGone;
    double peerHeard;
    int stage;                  // 0-3
    int stagesCleared;
    struct RhField field[2];    // by pad side
    int diffWant[2];            // the difficulty each player chose
    int diffConfirmed[2];
    int cursor;                 // style / difficulty / options screens
    // the wheel
    struct WheelItem *wheel;
    int wheelCount, wheelPos, sort, openGroup;
    float wheelScroll;
    int previewFor;
    float previewWait;
    struct RhImage banner, card[7], background;
    int bannerFor, cardFor[7], backgroundFor;
    int song;                   // the song being played
    uint32_t songKey;
    int pickedAt;               // the song picked (picker), waiting on options
    int ready[2];
    // the song
    struct RhChart chart[2];
    double musicFrom;
    int musicStarted;
    double songT, songBeat, lastT;
    double endT;
    float commentWait;
    float startHeld;
    int ended;
    // input
    struct { uint8_t col, down; double t; } presses[MAX_PRESSES];
    int pressCount;
    uint8_t heldNow[RH_COLS];
    uint16_t keysWas;
    int autoplay;
    // online
    uint32_t *peerSongs;
    int peerSongCount, peerHello;
    uint16_t netSeq;
    int netTimer;
    int peerNext;
    int localNext;
    uint32_t netEventsNote[256];
    uint8_t netEventsKind[256], netEventsJudge[256];
    // what's said about the records
    int newRecord;
    int stagesPlayed, allFullCombo, allAaa;
    int haveSongs;              // ST_NOSONGS: the songs are there (offering new ones) or not
    int crtOn, crtOff;          // frames of the CRT turning on / off so far
    struct RhythmStage pendingStage;    // counted when the results are left (the friend's final score is in by then)
    int stagePending;
    int lastCleared;
} R;

static struct RhOptions sOptions;      // kept for the next credit

// ---------------------------------------------------------------------------
// Little helpers

static void Go(int state)
{
    R.state = state;
    R.t = 0;
    R.cursor = 0;
}

static int Mine(void)
{
    return R.b.side & 1;
}

static int Other(void)
{
    return Mine() ^ 1;
}

static struct RhEntry *Song(int i)
{
    return RhSongs_Get(i);
}

// A song's key between two games and for the songs cleared: its title, subtitle, artist and folder (remixes share
// titles: "AFRONOVA" and "AFRONOVA" "(FROM NONSTOP MEGAMIX)")
static uint32_t SongKey(const struct RhEntry *e)
{
    const char *folder = strrchr(e->song.dir, '/');
    uint32_t h = Rh_Hash(e->song.title, (uint32_t)strlen(e->song.title), 0);
    h = Rh_Hash(e->song.subtitle, (uint32_t)strlen(e->song.subtitle), h);
    h = Rh_Hash(e->song.artist, (uint32_t)strlen(e->song.artist), h);
    folder = folder ? folder + 1 : e->song.dir;
    h = Rh_Hash(folder, (uint32_t)strlen(folder), h);
    return h ? h : 1;
}

// The title as shown: with its subtitle (the mix)
static const char *FullTitle(const struct RhEntry *e)
{
    static char buf[2][RH_TEXT * 2 + 2];
    static int which;
    which ^= 1;
    if (e->song.subtitle[0])
        snprintf(buf[which], sizeof(buf[which]), "%s %s", e->song.title, e->song.subtitle);
    else
        snprintf(buf[which], sizeof(buf[which]), "%s", e->song.title);
    return buf[which];
}

static int SongByKey(uint32_t key)
{
    int i;
    for (i = 0; i < RhSongs_Count(); i++)
        if (SongKey(Song(i)) == key)
            return i;
    return -1;
}

static int PeerHas(uint32_t key)
{
    int i;
    if (!R.versus || R.peerGone || !R.peerHello)
        return 1;
    for (i = 0; i < R.peerSongCount; i++)
        if (R.peerSongs[i] == key)
            return 1;
    return 0;
}

// The chart a player gets for a song: the difficulty they want, else the nearest the song has
static int DiffFor(const struct RhEntry *e, int want)
{
    int d;
    if (!e)
        return want;
    for (d = 0; d < RH_DIFFS; d++)
    {
        if (want - d >= 0 && e->song.charts[want - d].present)
            return want - d;
        if (want + d < RH_DIFFS && e->song.charts[want + d].present)
            return want + d;
    }
    return want;
}

static void Send(const uint8_t *data, uint16_t len)
{
    if (R.versus && !R.peerGone)
        MP_Rhythm_Send(R.b.peer, data, len);
}

static void Put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t Get32(const uint8_t *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
}

static void PutOptions(uint8_t *p, const struct RhOptions *o)
{
    p[0] = o->speed;
    p[1] = o->boost;
    p[2] = o->appearance;
    p[3] = o->turn;
    p[4] = o->dark;
    p[5] = o->reverse;
    p[6] = o->little;
    p[7] = o->noFreeze;
    p[8] = o->arrows;
    p[9] = o->marvelous;
}

static void GetOptions(const uint8_t *p, struct RhOptions *o)
{
    o->speed = p[0] % RH_SPEEDS;
    o->boost = p[1] & 1;
    o->appearance = p[2] & 3;
    o->turn = p[3] % 5;
    o->dark = p[4] & 1;
    o->reverse = p[5] & 1;
    o->little = p[6] & 1;
    o->noFreeze = p[7] & 1;
    o->arrows = p[8] & 3;
    o->marvelous = p[9] & 1;
}

static void SendDiff(int confirmed)
{
    uint8_t p[13];
    p[0] = RN_DIFF;
    p[1] = (uint8_t)R.diffWant[Mine()];
    p[2] = (uint8_t)confirmed;
    PutOptions(p + 3, &R.field[Mine()].opt);
    Send(p, sizeof(p));
}

// ---------------------------------------------------------------------------
// Input: menus from the game's buttons, steps from SDL's events (timed)

#define K_A      0x0001
#define K_B      0x0002
#define K_SELECT 0x0004
#define K_START  0x0008
#define K_RIGHT  0x0010
#define K_LEFT   0x0020
#define K_UP     0x0040
#define K_DOWN   0x0080
#define K_R      0x0100
#define K_L      0x0200

static uint16_t sKeys, sNew;

static int KeyCol(int scancode)
{
    switch (scancode)
    {
    case SDL_SCANCODE_LEFT: case SDL_SCANCODE_A: case SDL_SCANCODE_J:
        return 0;
    case SDL_SCANCODE_DOWN: case SDL_SCANCODE_S: case SDL_SCANCODE_K:
        return 1;
    case SDL_SCANCODE_UP: case SDL_SCANCODE_W: case SDL_SCANCODE_I:
        return 2;
    case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_D: case SDL_SCANCODE_L:
        return 3;
    }
    return -1;
}

static int PadCol(int button)
{
    switch (button)
    {
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: case SDL_CONTROLLER_BUTTON_X:
        return 0;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: case SDL_CONTROLLER_BUTTON_A:
        return 1;
    case SDL_CONTROLLER_BUTTON_DPAD_UP: case SDL_CONTROLLER_BUTTON_Y:
        return 2;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: case SDL_CONTROLLER_BUTTON_B:
        return 3;
    }
    return -1;
}

static double SongTimeNow(void);

static void Press(int col, int down, double t)
{
    if (R.pressCount < MAX_PRESSES)
    {
        R.presses[R.pressCount].col = (uint8_t)col;
        R.presses[R.pressCount].down = (uint8_t)down;
        R.presses[R.pressCount].t = t;
        R.pressCount++;
    }
}

// The left stick as arrows (held past half way), per controller
static uint8_t sStickWas[8];

void Rhythm_OnEvent(const void *ev)
{
    const SDL_Event *e = ev;
    int col = -1, down = 0;
    double ago;
    if (R.state != ST_PLAY || R.autoplay)
        return;
    switch (e->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        if (e->key.repeat)
            return;
        col = KeyCol(e->key.keysym.scancode);
        down = e->type == SDL_KEYDOWN;
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        col = PadCol(e->cbutton.button);
        down = e->type == SDL_CONTROLLERBUTTONDOWN;
        break;
    case SDL_CONTROLLERAXISMOTION:
        if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
        {
            int k = e->caxis.which & 7, bit, i;
            uint8_t now = sStickWas[k];
            if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
                now = (uint8_t)((now & ~9) | (e->caxis.value < -16000 ? 1 : 0) | (e->caxis.value > 16000 ? 8 : 0));
            else
                now = (uint8_t)((now & ~6) | (e->caxis.value > 16000 ? 2 : 0) | (e->caxis.value < -16000 ? 4 : 0));
            ago = (SDL_GetTicks() - e->caxis.timestamp) / 1000.0;
            for (i = 0; i < RH_COLS; i++)
            {
                bit = 1 << i;
                if ((now ^ sStickWas[k]) & bit)
                    Press(i, (now & bit) != 0, SongTimeNow() - (ago > 0 && ago < 0.25 ? ago : 0));
            }
            sStickWas[k] = now;
        }
        return;
    default:
        return;
    }
    if (col < 0)
        return;
    ago = (SDL_GetTicks() - e->common.timestamp) / 1000.0;
    Press(col, down, SongTimeNow() - (ago > 0 && ago < 0.25 ? ago : 0));
}

// The arrows held now (keys, pad buttons, the stick)
static void PollHeld(uint16_t gbaKeys)
{
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    int i, j;
    memset(R.heldNow, 0, sizeof(R.heldNow));
    if (Platform_IsTestMode())
    {
        R.heldNow[0] = (gbaKeys & K_LEFT) != 0;
        R.heldNow[1] = (gbaKeys & K_DOWN) != 0;
        R.heldNow[2] = (gbaKeys & K_UP) != 0;
        R.heldNow[3] = (gbaKeys & K_RIGHT) != 0;
        return;
    }
    for (i = 0; i < SDL_NUM_SCANCODES; i++)
        if (k[i] && KeyCol(i) >= 0)
            R.heldNow[KeyCol(i)] = 1;
    for (i = 0; i < SDL_NumJoysticks(); i++)
    {
        SDL_GameController *pad;
        if (!SDL_IsGameController(i))
            continue;
        pad = SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(i));
        if (!pad)
            continue;
        for (j = 0; j < SDL_CONTROLLER_BUTTON_MAX; j++)
            if (PadCol(j) >= 0 && SDL_GameControllerGetButton(pad, j))
                R.heldNow[PadCol(j)] = 1;
    }
    for (i = 0; i < 8; i++)
        for (j = 0; j < RH_COLS; j++)
            if (sStickWas[i] & (1 << j))
                R.heldNow[j] = 1;
}

// ---------------------------------------------------------------------------
// Pictures for the wheel and the song

static void FreeImages(void)
{
    int i;
    RhImage_Free(&R.banner);
    RhImage_Free(&R.background);
    for (i = 0; i < 7; i++)
    {
        RhImage_Free(&R.card[i]);
        R.cardFor[i] = -1;
    }
    R.bannerFor = R.backgroundFor = -1;
}

static void BannerFor(int song)
{
    struct RhEntry *e = Song(song);
    if (R.bannerFor == song)
        return;
    RhImage_Free(&R.banner);
    R.bannerFor = song;
    if (e)
        RhImage_Load(&R.banner, e->banner, 112, 35, 0);
}

static void BackgroundFor(int song)
{
    struct RhEntry *e = Song(song);
    if (R.backgroundFor == song)
        return;
    RhImage_Free(&R.background);
    R.backgroundFor = song;
    if (e && !RhImage_Load(&R.background, e->background, RH_W, RH_H, 1))
        RhImage_Load(&R.background, e->banner, RH_W, RH_H, 1);
}

// ---------------------------------------------------------------------------
// The wheel

static int CompareTitle(const void *a, const void *b)
{
    const struct RhEntry *x = Song(*(const int *)a), *y = Song(*(const int *)b);
    const char *tx = x->song.titleTranslit[0] ? x->song.titleTranslit : x->song.title;
    const char *ty = y->song.titleTranslit[0] ? y->song.titleTranslit : y->song.title;
#ifdef _WIN32
    return _stricmp(tx, ty);
#else
    return strcasecmp(tx, ty);
#endif
}

static int CompareBpm(const void *a, const void *b)
{
    const struct RhEntry *x = Song(*(const int *)a), *y = Song(*(const int *)b);
    if (x->song.displayBpmHi != y->song.displayBpmHi)
        return x->song.displayBpmHi < y->song.displayBpmHi ? -1 : 1;
    return CompareTitle(a, b);
}

// ABC / BPM sorts have their own sections (the letter, the BPM in 50s)
static int SectionOf(int song)
{
    const struct RhEntry *e = Song(song);
    const char *t = e->song.titleTranslit[0] ? e->song.titleTranslit : e->song.title;
    if (R.sort == SORT_ABC)
    {
        int c = toupper((unsigned char)t[0]);
        return c >= 'A' && c <= 'Z' ? c - 'A' + 1 : 0;
    }
    if (R.sort == SORT_BPM)
        return (int)(e->song.displayBpmHi / 50);
    return e->group;
}

static void SectionName(int section, char *out, int size)
{
    int i;
    if (R.sort == SORT_ABC)
    {
        if (section == 0)
            snprintf(out, size, "0-9 / OTHER");
        else
            snprintf(out, size, "%c", 'A' + section - 1);
        return;
    }
    if (R.sort == SORT_BPM)
    {
        snprintf(out, size, "BPM %d-%d", section * 50, section * 50 + 49);
        return;
    }
    for (i = 0; i < RhSongs_Count(); i++)
        if (Song(i)->group == section)
        {
            snprintf(out, size, "%s", Song(i)->song.group);
            return;
        }
    snprintf(out, size, "?");
}

static void BuildWheel(int keepSong)
{
    int n = RhSongs_Count(), i, *order, last = -999;
    free(R.wheel);
    R.wheel = calloc(n * 2 + 2, sizeof(*R.wheel));
    R.wheelCount = 0;
    order = malloc((n + 1) * sizeof(int));
    for (i = 0; i < n; i++)
        order[i] = i;
    if (R.sort == SORT_ABC)
        qsort(order, n, sizeof(int), CompareTitle);
    else if (R.sort == SORT_BPM)
        qsort(order, n, sizeof(int), CompareBpm);
    if (keepSong >= 0)
        R.openGroup = SectionOf(keepSong);
    for (i = 0; i < n; i++)
    {
        int s = SectionOf(order[i]);
        if (s != last)
        {
            R.wheel[R.wheelCount].group = (int16_t)s;
            R.wheel[R.wheelCount].song = -1;
            R.wheelCount++;
            last = s;
        }
        if (s == R.openGroup)
        {
            R.wheel[R.wheelCount].group = (int16_t)s;
            R.wheel[R.wheelCount].song = (int16_t)order[i];
            R.wheelCount++;
        }
    }
    free(order);
    R.wheelPos = 0;
    for (i = 0; i < R.wheelCount; i++)
        if ((keepSong >= 0 && R.wheel[i].song == keepSong) || (keepSong < 0 && R.wheel[i].song < 0 && R.wheel[i].group == R.openGroup))
        {
            R.wheelPos = i;
            break;
        }
    R.wheelScroll = (float)R.wheelPos;
}

static int WheelSong(void)
{
    return R.wheelCount ? R.wheel[R.wheelPos].song : -1;
}

// ---------------------------------------------------------------------------
// The song's clock

static double SongTimeNow(void)
{
    struct RhEntry *e = Song(R.song);
    double m;
    if (!e)
        return 0;
    if (R.musicStarted)
        m = RhAudio_MusicTime();
    else
        m = R.musicFrom;
    // TIMING (OPTIONS): the steps judged later than the music's clock says
    return m + e->song.offset - R.field[Mine()].opt.timing * 0.005;
}

// ---------------------------------------------------------------------------
// Begin / end

// the options are kept with the save (mp_rhythm.c "<save>.dance")
static void LoadOptions(void)
{
    uint8_t bytes[16];
    memset(bytes, 0, sizeof(bytes));
    MP_Rhythm_LoadOptions(bytes, sizeof(bytes));
    GetOptions(bytes, &sOptions);
    sOptions.timing = (int8_t)bytes[10];
}

int Rhythm_IsActive(void)
{
    return R.state != ST_OFF;
}

int Rhythm_LastStagesCleared(void)
{
    return R.lastCleared;
}

int Rhythm_Begin(const struct RhythmBegin *begin)
{
    int i;
    LoadOptions();
    for (i = 0; i < 2; i++)
    {
        RhPlay_End(&R.field[i].play);
        RhChart_Free(&R.chart[i]);
    }
    free(R.wheel);
    free(R.peerSongs);
    FreeImages();
    memset(&R, 0, sizeof(R));
    FreeImages();
    R.b = *begin;
    R.versus = begin->peer >= 0;
    R.peerHeard = RhAudio_Now();
    R.diffWant[0] = R.diffWant[1] = RH_DIFFICULT;
    R.field[0].opt = R.field[1].opt = sOptions;
    R.autoplay = getenv("MOEMON_DANCE_AUTOPLAY") != NULL;
    R.last = RhAudio_Now();
    R.song = -1;
    R.allFullCombo = R.allAaa = 1;
    // are there new songs (or any)? asked in the background while the CAUTION screen shows
    if (!Platform_IsTestMode() || getenv("MOEMON_UPDATE_URL"))
        Updater_SongsCheck();
    if (RhSongs_Scan() == 0)
    {
        R.haveSongs = 0;
        Go(ST_NOSONGS);
        return 1;
    }
    R.haveSongs = 1;
    R.openGroup = -1;
    for (i = 0; i < RhSongs_Count(); i++)
        if (strstr(Song(i)->song.group, "EXTREME"))
        {
            R.openGroup = Song(i)->group;
            break;
        }
    if (R.openGroup < 0)
        R.openGroup = Song(0)->group;
    BuildWheel(-1);
    if (R.versus)
    {
        // what we have, so the picker greys out what we don't
        int n = RhSongs_Count();
        uint8_t *p = malloc(4 + n * 4);
        p[0] = RN_HELLO;
        p[1] = Mine();
        p[2] = (uint8_t)n;
        p[3] = (uint8_t)(n >> 8);
        for (i = 0; i < n; i++)
            Put32(p + 4 + i * 4, SongKey(Song(i)));
        Send(p, (uint16_t)(4 + n * 4));
        free(p);
    }
    Go(ST_CAUTION);
    RhAudio_StopAll();
    RhAudio_Sfx("snd_coin");
    RhAudio_Announce("caution");
    MPNet_Log("dance: a credit, pad %d, %s", Mine(), R.versus ? "with a friend" : "alone");
    return 1;
}

static void EndCredit(void)
{
    int i;
    uint8_t q = RN_QUIT;
    Send(&q, 1);
    R.lastCleared = R.stagesCleared;
    RhAudio_StopAll();
    for (i = 0; i < 2; i++)
    {
        RhPlay_End(&R.field[i].play);
        RhChart_Free(&R.chart[i]);
    }
    sOptions = R.field[Mine()].opt;
    {
        uint8_t bytes[16];
        memset(bytes, 0, sizeof(bytes));
        PutOptions(bytes, &sOptions);
        bytes[10] = (uint8_t)sOptions.timing;
        MP_Rhythm_SaveOptions(bytes, sizeof(bytes));
    }
    if (R.stagesPlayed > 0)
        MP_Rhythm_CreditDone(R.stagesPlayed, R.allFullCombo, R.allAaa);
    FreeImages();
    free(R.wheel);
    R.wheel = NULL;
    RhSongs_Free();
    // the screen turns off (Rhythm_Step), then the map side hears the credit is over
    R.state = ST_CLOSING;
    R.crtOff = 0;
    MPNet_Log("dance: the credit is over, the screen turns off");
    MP_Rhythm_Dancing(0);
}

void Rhythm_PeerGone(uint8_t id)
{
    if (R.state == ST_OFF || R.state == ST_CLOSING || !R.versus || id != (uint8_t)R.b.peer || R.peerGone)
        return;
    R.peerGone = 1;
    R.b.picker = 1;
    R.field[Other()].used = 0;
    MPNet_Log("dance: the friend on the other pad is gone");
}

// ---------------------------------------------------------------------------
// Online

static void ApplyPeerEvents(struct RhField *f, const uint8_t *d, uint16_t len)
{
    uint16_t seq;
    int n, i;
    if (len < 15)
        return;
    seq = (uint16_t)(d[1] | d[2] << 8);
    f->netScore = (int32_t)Get32(d + 3);
    f->netCombo = d[7] | d[8] << 8;
    f->netMaxCombo = d[9] | d[10] << 8;
    f->netLife = d[11] / 200.0f;
    f->netFailed = d[12] & 1;
    for (i = 0; i < RH_COLS; i++)
        f->held[i] = (d[13] >> i) & 1;
    n = d[14];
    if (len < 15 + n * 4)
        return;
    // event k of the n is number seq - n + 1 + k
    for (i = 0; i < n; i++)
    {
        uint16_t number = (uint16_t)(seq - n + 1 + i);
        const uint8_t *e = d + 15 + i * 4;
        int note = e[0] | e[1] << 8, kind = e[2], judge = e[3];
        if ((int16_t)(number - f->netSeq) <= 0)
            continue;
        f->netSeq = number;
        if (!f->play.notes || note >= f->play.count)
            continue;
        if (kind == RH_EV_TAP)
        {
            f->play.judge[note] = (uint8_t)judge;
            if (f->play.notes[note].type == RH_NOTE_HOLD && judge > RH_J_MISS)
                f->play.hold[note] = RH_HOLD_ACTIVE;
        }
        else if (kind == RH_EV_HOLD)
            f->play.hold[note] = (uint8_t)judge;
        // the same pictures as our own steps
        if (f->play.eventCount < RH_EVENTS)
        {
            struct RhEvent *ev = &f->play.events[f->play.eventCount++];
            memset(ev, 0, sizeof(*ev));
            ev->kind = (uint8_t)kind;
            ev->judge = (uint8_t)judge;
            ev->col = f->play.notes[note].col;
            ev->note = note;
            ev->offset = 1;
        }
        if (kind == RH_EV_TAP && judge != RH_J_NONE)
        {
            // the row's judgment: when the row is all judged
            int a = note, b = note, k, worst = RH_J_MARVELOUS, done = 1;
            while (a > 0 && f->play.notes[a - 1].row == f->play.notes[note].row)
                a--;
            while (b + 1 < f->play.count && f->play.notes[b + 1].row == f->play.notes[note].row)
                b++;
            for (k = a; k <= b; k++)
            {
                if (f->play.notes[k].type == RH_NOTE_MINE)
                    continue;
                if (f->play.judge[k] == RH_J_NONE)
                    done = 0;
                else if (f->play.judge[k] < worst)
                    worst = f->play.judge[k];
            }
            if (done && f->play.eventCount < RH_EVENTS)
            {
                struct RhEvent *ev = &f->play.events[f->play.eventCount++];
                memset(ev, 0, sizeof(*ev));
                ev->kind = RH_EV_ROW;
                ev->judge = (uint8_t)worst;
            }
        }
    }
    // what's passed is done
    while (f->play.first < f->play.count && f->play.notes[f->play.first].time < R.songT - 1.0
           && (f->play.notes[f->play.first].type != RH_NOTE_HOLD || f->play.notes[f->play.first].endTime < R.songT - 1.0))
        f->play.first++;
}

// Our events since the last message, kept in a ring so a lost message is made up by the next
static void NoteEvents(const struct RhPlay *p)
{
    int i;
    for (i = 0; i < p->eventCount; i++)
    {
        const struct RhEvent *e = &p->events[i];
        if (e->kind != RH_EV_TAP && e->kind != RH_EV_HOLD)
            continue;
        R.netSeq++;
        R.netEventsNote[R.netSeq & 255] = (uint32_t)e->note;
        R.netEventsKind[R.netSeq & 255] = e->kind;
        R.netEventsJudge[R.netSeq & 255] = e->judge;
    }
}

static void SendState(void)
{
    const struct RhField *f = &R.field[Mine()];
    uint8_t p[15 + 40 * 4];
    int n = R.netSeq < 40 ? R.netSeq : 40, i;
    p[0] = RN_STATE;
    p[1] = (uint8_t)R.netSeq;
    p[2] = (uint8_t)(R.netSeq >> 8);
    Put32(p + 3, (uint32_t)f->play.score);
    p[7] = (uint8_t)f->play.combo;
    p[8] = (uint8_t)(f->play.combo >> 8);
    p[9] = (uint8_t)f->play.maxCombo;
    p[10] = (uint8_t)(f->play.maxCombo >> 8);
    p[11] = (uint8_t)(f->play.life * 200);
    p[12] = (uint8_t)(f->play.failed | f->play.finished << 1);
    p[13] = (uint8_t)(R.heldNow[0] | R.heldNow[1] << 1 | R.heldNow[2] << 2 | R.heldNow[3] << 3);
    p[14] = (uint8_t)n;
    for (i = 0; i < n; i++)
    {
        uint16_t number = (uint16_t)(R.netSeq - n + 1 + i);
        uint32_t note = R.netEventsNote[number & 255];
        p[15 + i * 4] = (uint8_t)note;
        p[16 + i * 4] = (uint8_t)(note >> 8);
        p[17 + i * 4] = R.netEventsKind[number & 255];
        p[18 + i * 4] = R.netEventsJudge[number & 255];
    }
    Send(p, (uint16_t)(15 + n * 4));
}

static void StartStage(int song);

void Rhythm_OnNet(uint8_t from, const uint8_t *d, uint16_t len)
{
    struct RhField *other;
    if (R.state == ST_OFF || R.state == ST_CLOSING || !R.versus || R.peerGone || (int8_t)from != R.b.peer || len < 1)
        return;
    R.peerHeard = RhAudio_Now();
    other = &R.field[Other()];
    switch (d[0])
    {
    case RN_HELLO:
        if (len >= 4)
        {
            int n = d[2] | d[3] << 8, i;
            if (len < 4 + n * 4)
                n = (len - 4) / 4;
            free(R.peerSongs);
            R.peerSongs = malloc((n + 1) * sizeof(uint32_t));
            for (i = 0; i < n; i++)
                R.peerSongs[i] = Get32(d + 4 + i * 4);
            R.peerSongCount = n;
            R.peerHello = 1;
        }
        break;
    case RN_DIFF:
        if (len >= 13)
        {
            R.diffWant[Other()] = d[1] % RH_DIFFS;
            R.diffConfirmed[Other()] = d[2];
            GetOptions(d + 3, &other->opt);
        }
        break;
    case RN_WHEEL:
        if (len >= 5 && !R.b.picker && R.state == ST_MUSIC)
        {
            int s = SongByKey(Get32(d + 1));
            if (s >= 0 && s != WheelSong())
                BuildWheel(s);
        }
        break;
    case RN_PICK:
        if (len >= 5 && !R.b.picker)
        {
            int s = SongByKey(Get32(d + 1));
            if (s >= 0)
            {
                if (WheelSong() != s)
                    BuildWheel(s);
                R.pickedAt = 1;
                R.ready[Mine()] = 0;
                RhAudio_Sfx("snd_start");
                Go(ST_OPTIONS);
            }
        }
        break;
    case RN_READY:
        if (len >= 12)
        {
            R.diffWant[Other()] = d[1] % RH_DIFFS;
            GetOptions(d + 2, &other->opt);
            R.ready[Other()] = 1;
        }
        break;
    case RN_GO:
        if (len >= 6 && !R.b.picker)
        {
            int s = SongByKey(Get32(d + 1));
            R.stage = d[5] < RHYTHM_STAGES ? d[5] : R.stage;
            if (s >= 0)
                StartStage(s);
        }
        break;
    case RN_STATE:
        if (R.state == ST_PLAY || R.state == ST_SONG_END || R.state == ST_STAGE)
            ApplyPeerEvents(other, d, len);
        break;
    case RN_NEXT:
        R.peerNext = 1;
        break;
    case RN_QUIT:
        MPNet_Log("dance: the friend left the machine");
        Rhythm_PeerGone(from);
        break;
    }
}

// ---------------------------------------------------------------------------
// Screens

static void DrawMenuBack(const char *title)
{
    const struct RhPic *bg = RhPic_Get("menu_bg"), *footer = RhPic_Get("footer");
    RhGfx_Pic(bg, 0, 0, 0, 0, 256);
    RhGfx_Fill(0, 0, RH_W, 15, RH_RGB(20, 40, 10), 200);
    RhGfx_Text(RH_FONT_NORMAL, 6, 1, title, RH_RGB(230, 255, 140), RH_LEFT);
    {
        char st[24];
        if (R.stage == RHYTHM_STAGES - 1)
            snprintf(st, sizeof(st), "FINAL STAGE");
        else
            snprintf(st, sizeof(st), "STAGE %d", R.stage + 1);
        RhGfx_Text(RH_FONT_SMALL, RH_W - 6, 3, st, RH_RGB(255, 255, 255), RH_RIGHT);
    }
    if (footer)
        RhGfx_Pic(footer, 0, 0, RH_H - footer->h, 0, 256);
}

static void Hint(const char *text)
{
    RhGfx_Fill(0, RH_H - 12, RH_W, 12, RH_RGB(0, 0, 0), 150);
    RhGfx_Text(RH_FONT_SMALL, RH_W / 2, RH_H - 11, text, RH_RGB(255, 255, 255), RH_CENTER);
}

static void StateCaution(void)
{
    const struct RhPic *c = RhPic_Get("caution");
    RhGfx_Clear(0);
    if (c)
        RhGfx_Pic(c, 0, (RH_W - c->w) / 2, (RH_H - c->h) / 2, 0, R.t < 0.3f ? (int)(R.t / 0.3f * 256) : 256);
    if (R.t > 3.5f || (R.t > 0.5f && (sNew & (K_A | K_START))))
    {
        long done, total;
        const char *err;
        // new songs out: offered when dancing alone (a friend waiting on the other pad isn't kept waiting)
        if (!R.versus && Updater_SongsState(&done, &total, &err) == SONGS_AVAILABLE)
        {
            Go(ST_NOSONGS);
            return;
        }
        RhAudio_Sfx("snd_start");
        RhAudio_PlayMenuMusic();
        RhAudio_Announce("select style intro");
        Go(ST_STYLE);
    }
}

static void StateStyle(void)
{
    const struct RhPic *single = RhPic_Get("style_single"), *versus = RhPic_Get("style_versus");
    int vs = R.versus && !R.peerGone;
    DrawMenuBack("SELECT STYLE");
    if (single)
        RhGfx_Pic(single, 0, 18, 40, 0, vs ? 110 : 256);
    if (versus)
        RhGfx_Pic(versus, 0, RH_W - 18 - versus->w, 40, 0, vs ? 256 : 110);
    RhGfx_Fill(vs ? RH_W - 20 - (versus ? versus->w : 0) : 16, 38, (single ? single->w : 90) + 4, 2,
               RH_RGB(255, 255, 255), (int)(160 + 90 * sin(R.t * 8)));
    if (vs)
    {
        char who[48];
        snprintf(who, sizeof(who), "With %s on the %s pad!", R.b.peerName, Mine() ? "left" : "right");
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 100, who, RH_RGB(255, 255, 255), RH_CENTER);
    }
    else
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 100, "One player!", RH_RGB(255, 255, 255), RH_CENTER);
    Hint("A: OK    B: leave the machine");
    if (R.t > 0.3f && (sNew & (K_A | K_START)))
    {
        RhAudio_Sfx("snd_start");
        RhAudio_Announce(vs ? "select style comment versus" : "select style comment single");
        Go(ST_DIFFICULTY);
        R.cursor = R.diffWant[Mine()];
    }
    else if (R.t > 0.3f && (sNew & K_B))
    {
        RhAudio_Sfx("snd_back");
        EndCredit();
    }
}

static void StateDifficulty(void)
{
    int me = Mine(), i;
    static const char *const comments[RH_DIFFS] = {"ScreenSelectMaster comment beginner", "ScreenSelectMaster comment light",
        "ScreenSelectMaster comment standard", "ScreenSelectMaster comment heavy", "select difficulty challenge"};
    DrawMenuBack("SELECT DIFFICULTY");
    // the cards in a row, ours big in the middle
    for (i = 0; i < RH_DIFFS; i++)
    {
        const struct RhPic *card = RhPic_Get(sCardNames[i]);
        int d = i - R.cursor;
        int cx = RH_W / 2 + d * 46, scale = d == 0 ? 100 : 58;
        if (!card || d < -3 || d > 3)
            continue;
        if (d != 0)
            RhGfx_PicScaled(card, 0, cx, 82, card->w * scale / 100, card->h * scale / 100, 170);
    }
    {
        const struct RhPic *card = RhPic_Get(sCardNames[R.cursor]);
        if (card)
            RhGfx_PicScaled(card, 0, RH_W / 2, 82, card->w, card->h, 256);
    }
    if (R.versus && !R.peerGone)
    {
        char st[48];
        snprintf(st, sizeof(st), "%s: %s%s", R.b.peerName, sDiffNames[R.diffWant[Other()]],
                 R.diffConfirmed[Other()] ? " (ready)" : "");
        RhGfx_Text(RH_FONT_SMALL, RH_W / 2, 132, st, RH_RGB(255, 255, 255), RH_CENTER);
    }
    if (R.diffConfirmed[me])
    {
        Hint("Waiting for your friend...");
        if (!R.versus || R.peerGone || R.diffConfirmed[Other()])
        {
            RhAudio_Announce("select music intro");
            Go(ST_MUSIC);
        }
        return;
    }
    Hint("LEFT / RIGHT: difficulty   A: OK   B: back");
    if (sNew & (K_LEFT | K_UP) && R.cursor > 0)
    {
        R.cursor--;
        RhAudio_Sfx("snd_diffchange");
    }
    if (sNew & (K_RIGHT | K_DOWN) && R.cursor < RH_DIFFS - 1)
    {
        R.cursor++;
        RhAudio_Sfx("snd_diffchange");
    }
    R.diffWant[me] = R.cursor;
    if (sNew & (K_LEFT | K_RIGHT | K_UP | K_DOWN))
        SendDiff(0);
    if (R.t > 0.3f && (sNew & (K_A | K_START)))
    {
        RhAudio_Sfx("snd_start");
        RhAudio_Announce(comments[R.cursor]);
        R.diffConfirmed[me] = 1;
        SendDiff(1);
    }
    else if (sNew & K_B)
    {
        RhAudio_Sfx("snd_back");
        Go(ST_STYLE);
    }
}

static void DrawFeet(int x, int y, int meter, uint32_t color)
{
    int i;
    for (i = 0; i < 10; i++)
        RhGfx_Fill(x + i * 5, y, 4, 4, i < meter ? color : RH_RGB(60, 60, 60), 256);
    if (meter > 10)
    {
        char n[8];
        snprintf(n, sizeof(n), "%d", meter);
        RhGfx_Text(RH_FONT_SMALL, x + 52, y - 4, n, color, RH_LEFT);
    }
}

static void DrawWheelItem(int i, int y, int selected)
{
    const struct WheelItem *w = &R.wheel[i];
    int x = selected ? 118 : 124;
    if (w->song < 0)
    {
        char name[RH_TEXT];
        SectionName(w->group, name, sizeof(name));
        RhGfx_Fill(x, y, RH_W - x - 2, 14, w->group == R.openGroup ? RH_RGB(60, 120, 20) : RH_RGB(30, 70, 10), 230);
        RhGfx_Fill(x, y, 3, 14, RH_RGB(200, 255, 80), 256);
        RhGfx_TextFit(RH_FONT_SMALL, x + 6, y + 2, name, RH_RGB(230, 255, 160), RH_LEFT, RH_W - x - 10);
    }
    else
    {
        struct RhEntry *e = Song(w->song);
        int has = PeerHas(SongKey(e)), slot = (i - R.wheelPos + 3) % 7;
        if (slot < 0)
            slot += 7;
        RhGfx_Fill(x, y, RH_W - x - 2, 14, selected ? RH_RGB(250, 250, 235) : RH_RGB(220, 225, 210), has ? 240 : 120);
        // the title card when the song has one, else its title
        if (R.cardFor[slot] != w->song)
        {
            RhImage_Free(&R.card[slot]);
            R.cardFor[slot] = w->song;
            if (e->card[0])
                RhImage_Load(&R.card[slot], e->card, 96, 16, 0);
        }
        if (R.card[slot].rgba)
            RhGfx_Image(&R.card[slot], x + 2, y - 1, has ? 256 : 120);
        else
            RhGfx_TextFit(RH_FONT_SMALL, x + 4, y + 2, FullTitle(e), RH_RGB(20, 20, 30), RH_LEFT, RH_W - x - 10);
    }
}

static void Preview(int song)
{
    struct RhEntry *e = Song(song);
    if (!e)
    {
        if (R.previewFor != -1)
            RhAudio_PlayMenuMusic();
        R.previewFor = -1;
        return;
    }
    RhAudio_PlayMusic(e->music, e->song.sampleStart, e->song.sampleLength, 1, 0.5);
    R.previewFor = song;
}

static void StateMusic(void)
{
    int me = Mine(), song = WheelSong(), i;
    struct RhEntry *e = Song(song);
    int picker = R.b.picker || !R.versus || R.peerGone;
    DrawMenuBack("SELECT MUSIC");
    // the wheel: 7 rows, the chosen one in the middle
    R.wheelScroll += (R.wheelPos - R.wheelScroll) * 0.35f;
    for (i = -3; i <= 3; i++)
    {
        int k = R.wheelPos + i, y;
        if (R.wheelCount == 0)
            break;
        k = ((k % R.wheelCount) + R.wheelCount) % R.wheelCount;
        y = 66 + i * 16 + (int)((R.wheelPos - R.wheelScroll) * 16);
        DrawWheelItem(k, y, i == 0);
    }
    RhGfx_Fill(116, 64, 2, 18, RH_RGB(255, 255, 255), (int)(160 + 90 * sin(R.t * 8)));
    // the song: banner, title, artist, BPM, the charts' feet and the records
    if (e)
    {
        char bpm[32];
        int d;
        BannerFor(song);
        if (R.banner.rgba)
            RhGfx_Image(&R.banner, 2 + (112 - R.banner.w) / 2, 18, 256);
        RhGfx_TextFit(RH_FONT_SMALL, 3, 54, FullTitle(e), RH_RGB(255, 255, 255), RH_LEFT, 112);
        RhGfx_TextFit(RH_FONT_SMALL, 3, 63, e->song.artist, RH_RGB(200, 230, 255), RH_LEFT, 112);
        if (e->song.displayBpmRandom)
            snprintf(bpm, sizeof(bpm), "BPM ???");
        else if ((int)e->song.displayBpmLo != (int)e->song.displayBpmHi)
            snprintf(bpm, sizeof(bpm), "BPM %d-%d", (int)e->song.displayBpmLo, (int)e->song.displayBpmHi);
        else
            snprintf(bpm, sizeof(bpm), "BPM %d", (int)e->song.displayBpmHi);
        RhGfx_Text(RH_FONT_SMALL, 3, 72, bpm, RH_RGB(255, 230, 120), RH_LEFT);
        for (d = 0; d < RH_DIFFS; d++)
        {
            int y = 83 + d * 9, mineHere = DiffFor(e, R.diffWant[me]) == d;
            int theirs = R.versus && !R.peerGone && DiffFor(e, R.diffWant[Other()]) == d;
            if (!e->song.charts[d].present)
                continue;
            if (mineHere || theirs)
                RhGfx_Fill(2, y - 1, 112, 9, mineHere ? RH_RGB(255, 255, 255) : RH_RGB(120, 160, 255), 60);
            RhGfx_Text(RH_FONT_SMALL, 4, y - 1, sDiffNames[d], sDiffColors[d], RH_LEFT);
            DrawFeet(58, y + 1, e->song.charts[d].meter, sDiffColors[d]);
        }
        // the server's best for our chart
        {
            char names[3][16], line[48];
            uint32_t scores[3];
            uint8_t grades[3];
            int dd = DiffFor(e, R.diffWant[me]);
            uint32_t key = RhSongs_ChartKey(e, dd);
            int n = MP_Rhythm_Records(key, names, scores, grades, 1);
            if (R.t < 0.05f || R.previewFor != song)
                MP_Rhythm_AskRecords(key);
            if (n > 0)
                snprintf(line, sizeof(line), "RECORD %s %u", names[0], scores[0]);
            else
                snprintf(line, sizeof(line), "RECORD ---");
            RhGfx_TextFit(RH_FONT_SMALL, 3, 130, line, RH_RGB(255, 255, 255), RH_LEFT, 112);
        }
        if (!PeerHas(SongKey(e)))
        {
            char st[48];
            snprintf(st, sizeof(st), "%s doesn't have it", R.b.peerName);
            RhGfx_TextFit(RH_FONT_SMALL, 3, 139, st, RH_RGB(255, 120, 120), RH_LEFT, 112);
        }
    }
    {
        char sort[24];
        snprintf(sort, sizeof(sort), "SORT: %s", sSortNames[R.sort]);
        RhGfx_Text(RH_FONT_SMALL, RH_W - 4, 16, sort, RH_RGB(230, 255, 160), RH_RIGHT);
    }
    Hint(picker ? "LEFT/RIGHT: song  UP/DOWN: level  A: OK  R: sort" : "UP/DOWN: your level  (1P picks the song)");
    // the preview starts when the wheel rests a moment
    if (song != R.previewFor)
    {
        R.previewWait += 1 / 60.0f;
        if (R.previewWait > 0.4f)
        {
            R.previewWait = 0;
            Preview(song);
        }
    }
    else
        R.previewWait = 0;
    if (sNew & K_UP && R.diffWant[me] > 0)
    {
        R.diffWant[me] = e ? DiffFor(e, R.diffWant[me]) : R.diffWant[me];
        if (R.diffWant[me] > 0)
            R.diffWant[me]--;
        if (e)
            while (R.diffWant[me] > 0 && !e->song.charts[R.diffWant[me]].present)
                R.diffWant[me]--;
        RhAudio_Sfx("snd_harder");
        SendDiff(1);
    }
    if (sNew & K_DOWN && R.diffWant[me] < RH_DIFFS - 1)
    {
        R.diffWant[me] = e ? DiffFor(e, R.diffWant[me]) : R.diffWant[me];
        if (R.diffWant[me] < RH_DIFFS - 1)
            R.diffWant[me]++;
        if (e)
            while (R.diffWant[me] < RH_DIFFS - 1 && !e->song.charts[R.diffWant[me]].present)
                R.diffWant[me]++;
        RhAudio_Sfx("snd_harder");
        SendDiff(1);
    }
    if (!picker)
        return;
    if (sNew & (K_LEFT | K_RIGHT) && R.wheelCount)
    {
        R.wheelPos = (R.wheelPos + (sNew & K_RIGHT ? 1 : R.wheelCount - 1)) % R.wheelCount;
        if (R.wheelPos == 0 && (sNew & K_RIGHT))
            R.wheelScroll -= R.wheelCount;
        if (R.wheelPos == R.wheelCount - 1 && (sNew & K_LEFT))
            R.wheelScroll += R.wheelCount;
        RhAudio_Sfx("snd_wheel");
        if (WheelSong() >= 0)
        {
            uint8_t p[5];
            p[0] = RN_WHEEL;
            Put32(p + 1, SongKey(Song(WheelSong())));
            Send(p, 5);
        }
    }
    if (sNew & (K_R | K_SELECT))
    {
        R.sort = (R.sort + 1) % SORT_COUNT;
        BuildWheel(song);
        RhAudio_Sfx("snd_sort");
    }
    if (sNew & (K_A | K_START) && R.t > 0.3f && R.wheelCount)
    {
        if (song < 0)
        {
            // a section: open it (or close it)
            int group = R.wheel[R.wheelPos].group;
            R.openGroup = R.openGroup == group ? -999 : group;
            BuildWheel(-1);
            for (i = 0; i < R.wheelCount; i++)
                if (R.wheel[i].song < 0 && R.wheel[i].group == group)
                    R.wheelPos = i;
            R.wheelScroll = (float)R.wheelPos;
            RhAudio_Sfx("snd_expand");
        }
        else if (!PeerHas(SongKey(e)))
            RhAudio_Sfx("snd_back");
        else
        {
            uint8_t p[5];
            p[0] = RN_PICK;
            Put32(p + 1, SongKey(e));
            Send(p, 5);
            RhAudio_Sfx("snd_start");
            R.pickedAt = 1;
            R.ready[0] = R.ready[1] = 0;
            Go(ST_OPTIONS);
        }
    }
    else if (sNew & K_B && R.stage == 0)
    {
        RhAudio_Sfx("snd_back");
        R.diffConfirmed[me] = 0;
        SendDiff(0);
        Go(ST_DIFFICULTY);
        R.cursor = R.diffWant[me];
        RhAudio_PlayMenuMusic();
        R.previewFor = -1;
    }
}

// OPTIONS (the arcade's, docs/RHYTHM_DECISIONS.md #5): START held or A on the song opens it; START / A leaves
enum { OPT_SPEED, OPT_BOOST, OPT_APPEAR, OPT_TURN, OPT_DARK, OPT_REVERSE, OPT_LITTLE, OPT_FREEZE, OPT_ARROWS,
       OPT_JUDGE, OPT_TIMING, OPT_DONE, OPT_COUNT };

static const char *OptionValue(const struct RhOptions *o, int row, char *buf, int size)
{
    static const char *const appear[] = {"VISIBLE", "HIDDEN", "SUDDEN", "STEALTH"};
    static const char *const turn[] = {"OFF", "MIRROR", "LEFT", "RIGHT", "SHUFFLE"};
    static const char *const arrows[] = {"VIVID", "NOTE", "FLAT", "SOLO"};
    static const char *const speeds[RH_SPEEDS] = {"x1", "x1.5", "x2", "x3", "x5", "x8", "x0.5"};
    switch (row)
    {
    case OPT_SPEED: return speeds[o->speed % RH_SPEEDS];
    case OPT_BOOST: return o->boost ? "ON" : "OFF";
    case OPT_APPEAR: return appear[o->appearance & 3];
    case OPT_TURN: return turn[o->turn % 5];
    case OPT_DARK: return o->dark ? "ON" : "OFF";
    case OPT_REVERSE: return o->reverse ? "ON" : "OFF";
    case OPT_LITTLE: return o->little ? "ON" : "OFF";
    case OPT_FREEZE: return o->noFreeze ? "OFF" : "ON";
    case OPT_ARROWS: return arrows[o->arrows & 3];
    case OPT_JUDGE: return o->marvelous ? "MARVELOUS" : "ARCADE";
    case OPT_TIMING:
        snprintf(buf, size, o->timing ? "%+d ms" : "0 ms", o->timing * 5);
        return buf;
    }
    (void)buf;
    (void)size;
    return "";
}

static void ChangeOption(struct RhOptions *o, int row, int dir)
{
    switch (row)
    {
    case OPT_SPEED:
    {
        // x0.5 sits before x1
        static const int order[RH_SPEEDS] = {6, 0, 1, 2, 3, 4, 5};
        int k;
        for (k = 0; k < RH_SPEEDS && order[k] != o->speed; k++)
            ;
        k = (k + dir + RH_SPEEDS) % RH_SPEEDS;
        o->speed = (uint8_t)order[k];
        break;
    }
    case OPT_BOOST: o->boost ^= 1; break;
    case OPT_APPEAR: o->appearance = (uint8_t)((o->appearance + dir + 4) % 4); break;
    case OPT_TURN: o->turn = (uint8_t)((o->turn + dir + 5) % 5); break;
    case OPT_DARK: o->dark ^= 1; break;
    case OPT_REVERSE: o->reverse ^= 1; break;
    case OPT_LITTLE: o->little ^= 1; break;
    case OPT_FREEZE: o->noFreeze ^= 1; break;
    case OPT_ARROWS: o->arrows = (uint8_t)((o->arrows + dir + 4) % 4); break;
    case OPT_JUDGE: o->marvelous ^= 1; break;
    case OPT_TIMING:
        if ((dir > 0 && o->timing < 60) || (dir < 0 && o->timing > -60))
            o->timing = (int8_t)(o->timing + dir);
        break;
    }
}

static void SendReady(void)
{
    uint8_t p[12];
    p[0] = RN_READY;
    p[1] = (uint8_t)R.diffWant[Mine()];
    PutOptions(p + 2, &R.field[Mine()].opt);
    Send(p, sizeof(p));
}

static void StateOptions(void)
{
    static const char *const names[OPT_COUNT] = {"SPEED", "BOOST", "APPEARANCE", "TURN", "DARK", "REVERSE", "LITTLE",
                                                 "FREEZE ARROW", "ARROW", "JUDGMENT", "TIMING", "READY!"};
    struct RhOptions *o = &R.field[Mine()].opt;
    int i, picker = R.b.picker || !R.versus || R.peerGone;
    struct RhEntry *e = Song(WheelSong());
    // the first moment: "OPTIONS? A" (or it starts on its own)
    DrawMenuBack("OPTIONS");
    if (e)
        RhGfx_TextFit(RH_FONT_NORMAL, RH_W / 2, 18, FullTitle(e), RH_RGB(255, 255, 255), RH_CENTER, 230);
    for (i = 0; i < OPT_COUNT; i++)
    {
        char buf[16];
        int y = 31 + i * 10;
        int sel = R.cursor == i;
        if (sel)
            RhGfx_Fill(20, y - 1, 200, 10, RH_RGB(255, 255, 255), 70);
        RhGfx_Text(RH_FONT_SMALL, 26, y - 1, names[i], sel ? RH_RGB(255, 255, 120) : RH_RGB(230, 255, 200), RH_LEFT);
        if (i != OPT_DONE)
            RhGfx_Text(RH_FONT_SMALL, 214, y - 1, OptionValue(o, i, buf, sizeof(buf)), RH_RGB(255, 255, 255), RH_RIGHT);
    }
    if (R.ready[Mine()])
    {
        Hint("Waiting for your friend...");
    }
    else
    {
        Hint("UP/DOWN: row  LEFT/RIGHT: change  A: ready");
        if (sNew & K_UP)
            R.cursor = (R.cursor + OPT_COUNT - 1) % OPT_COUNT;
        if (sNew & K_DOWN)
            R.cursor = (R.cursor + 1) % OPT_COUNT;
        if (sNew & (K_LEFT | K_RIGHT) && R.cursor != OPT_DONE)
        {
            ChangeOption(o, R.cursor, sNew & K_RIGHT ? 1 : -1);
            RhAudio_Sfx("snd_change");
        }
        if (sNew & (K_UP | K_DOWN))
            RhAudio_Sfx("snd_row");
        if (R.t > 0.25f && (sNew & K_START || (sNew & K_A && R.cursor == OPT_DONE)))
        {
            RhAudio_Sfx("snd_start");
            R.ready[Mine()] = 1;
            SendReady();
        }
        if (sNew & K_A && R.cursor != OPT_DONE && R.cursor != OPT_TIMING)
        {
            ChangeOption(o, R.cursor, 1);
            RhAudio_Sfx("snd_change");
        }
        if (sNew & K_B && picker && !R.versus)
        {
            RhAudio_Sfx("snd_back");
            Go(ST_MUSIC);
            return;
        }
    }
    // the picker starts the stage once both are ready (or the friend is gone)
    if (picker && R.ready[Mine()] && (!R.versus || R.peerGone || R.ready[Other()] || R.t > 30))
    {
        uint8_t p[6];
        p[0] = RN_GO;
        Put32(p + 1, SongKey(e));
        p[5] = (uint8_t)R.stage;
        Send(p, 6);
        StartStage(WheelSong());
    }
}

static int LoadField(int side, int remote)
{
    struct RhField *f = &R.field[side];
    struct RhEntry *e = Song(R.song);
    struct RhNoteOptions no;
    int diff = DiffFor(e, R.diffWant[side]);
    RhPlay_End(&f->play);
    RhChart_Free(&R.chart[side]);
    RhField_Begin(f, side, remote);
    f->diff = diff;
    f->meter = e->song.charts[diff].meter;
    if (!RhChart_Load(&e->song, diff, &R.chart[side]))
        return 0;
    memset(&no, 0, sizeof(no));
    no.turn = f->opt.turn;
    no.little = f->opt.little;
    no.noFreeze = f->opt.noFreeze;
    // (shuffle: the same for both games, from the song and stage)
    no.shuffleSeed = SongKey(e) ^ (uint32_t)(R.stage * 7919 + side * 104729);
    RhPlay_Begin(&f->play, &e->song, &R.chart[side], f->meter, &no, f->opt.marvelous);
    return 1;
}

static void StartStage(int song)
{
    struct RhEntry *e = Song(song);
    char folder[24];
    if (!e)
        return;
    R.song = song;
    R.songKey = SongKey(e);
    if (!LoadField(Mine(), 0))
    {
        Go(ST_MUSIC);
        return;
    }
    if (R.versus && !R.peerGone)
        LoadField(Other(), 1);
    else
        R.field[Other()].used = 0;
    BackgroundFor(song);
    BannerFor(song);
    RhAudio_StopMusic(0.4);
    if (R.stage == RHYTHM_STAGES - 1)
        snprintf(folder, sizeof(folder), "stage final");
    else
        snprintf(folder, sizeof(folder), "stage %d", R.stage + 1);
    RhAudio_Announce(folder);
    RhAudio_Sfx("snd_swoosh");
    R.netSeq = 0;
    R.field[Other()].netSeq = 0;
    R.pressCount = 0;
    R.musicStarted = 0;
    R.ended = 0;
    R.peerNext = R.localNext = 0;
    R.newRecord = 0;
    Go(ST_STAGE);
}

static void StateStage(void)
{
    static const char *const stagePics[RHYTHM_STAGES] = {"stage_1", "stage_2", "stage_3", "stage_final"};
    const struct RhPic *pic = RhPic_Get(R.stage == RHYTHM_STAGES - 1 ? stagePics[3] : stagePics[R.stage]);
    struct RhEntry *e = Song(R.song);
    int slide = R.t < 0.3f ? (int)((0.3f - R.t) / 0.3f * RH_W) : 0;
    RhGfx_Clear(0);
    if (R.background.rgba)
        RhGfx_Image(&R.background, 0, 0, 90);
    RhGfx_Fill(0, 42, RH_W, 50, RH_RGB(0, 0, 0), 200);
    if (pic)
        RhGfx_Pic(pic, 0, (RH_W - pic->w) / 2 + slide, 48, 0, 256);
    if (e)
    {
        if (R.banner.rgba)
            RhGfx_Image(&R.banner, (RH_W - R.banner.w) / 2 - slide, 98, 256);
        RhGfx_TextFit(RH_FONT_NORMAL, RH_W / 2, 74, FullTitle(e), RH_RGB(255, 255, 255), RH_CENTER, 230);
    }
    if (R.t > 2.2f)
    {
        // the song's clock starts before the music: READY, HERE WE GO, then the steps
        double first = 1e9;
        int i;
        for (i = 0; i < 2; i++)
            if (R.field[i].used && R.field[i].play.count && R.field[i].play.notes[0].time < first)
                first = R.field[i].play.notes[0].time;
        // music time = song time - offset; READY and HERE WE GO need about 3 s before the first step
        R.musicFrom = (first - 3.0) - e->song.offset;
        if (R.musicFrom > -LEAD_IN)
            R.musicFrom = -LEAD_IN;
        R.lastT = SongTimeNow();
        R.startHeld = 0;
        R.commentWait = 8;
        R.netTimer = 0;
        Go(ST_PLAY);
        RhAudio_Announce("gameplay ready");
        if (!RhAudio_PlayMusic(e->music, R.musicFrom, 0, 0, 0))
            MPNet_Log("dance: %s won't play", e->music);
        R.musicStarted = 1;
    }
}

static void DrawSongBack(void)
{
    RhGfx_Clear(0);
    if (R.background.rgba)
        RhGfx_Image(&R.background, (RH_W - R.background.w) / 2, (RH_H - R.background.h) / 2, 150);
}

static void DrawPlayFrames(void)
{
    const struct RhPic *life = RhPic_Get("life_frame"), *score = RhPic_Get("score_frame");
    static const char *const small[RHYTHM_STAGES] = {"stage_small_1st", "stage_small_2nd", "stage_small_3rd",
                                                     "stage_small_final"};
    int i;
    for (i = 0; i < 2; i++)
        if (R.field[i].used)
            RhField_DrawLife(&R.field[i], R.songBeat);
    if (life)
        RhGfx_Pic(life, 0, 0, 0, 0, 256);
    {
        const struct RhPic *st = RhPic_Get(small[R.stage < RHYTHM_STAGES ? R.stage : 3]);
        if (st)
            RhGfx_Pic(st, 0, (RH_W - st->w) / 2, 12, 0, 256);
    }
    if (score)
        RhGfx_PicPart(score, 0, 0, RH_H - 16, 0, 16, 0, 256);
    for (i = 0; i < 2; i++)
        if (R.field[i].used)
        {
            struct RhEntry *e = Song(R.song);
            char d[24];
            RhField_DrawScore(&R.field[i]);
            static const char *const shortNames[RH_DIFFS] = {"BEG", "LGT", "STD", "HVY", "CHA"};
            // (both fields: the short names, so the two fit side by side between the scores)
            snprintf(d, sizeof(d), "%s %d", R.field[i ^ 1].used ? shortNames[R.field[i].diff] : sDiffNames[R.field[i].diff],
                     e ? e->song.charts[R.field[i].diff].meter : 0);
            RhGfx_Text(RH_FONT_SMALL, i == 0 ? 92 : RH_W - 92, RH_H - 16, d, sDiffColors[R.field[i].diff],
                       i == 0 ? RH_LEFT : RH_RIGHT);
        }
}

static void Autoplay(struct RhField *f, double t)
{
    // tests (MOEMON_DANCE_AUTOPLAY=1): every step on time, freezes held to their ends
    struct RhPlay *p = &f->play;
    int i;
    for (i = p->first; i < p->count && p->notes[i].time <= t; i++)
        if (p->judge[i] == RH_J_NONE && p->notes[i].type != RH_NOTE_MINE)
            RhPlay_Press(p, p->notes[i].col, p->notes[i].time);
    for (i = 0; i < RH_COLS; i++)
    {
        int k, keep = 0;
        for (k = p->first; k < p->count && p->notes[k].time <= t; k++)
            if (p->notes[k].col == i && p->notes[k].type == RH_NOTE_HOLD && p->notes[k].endTime > t)
                keep = 1;
        R.heldNow[i] = (uint8_t)keep;
        p->held[i] = (uint8_t)keep;
    }
}

static void FinishSong(void);

static void StatePlay(uint16_t gbaKeys, float dt)
{
    struct RhEntry *e = Song(R.song);
    struct RhField *me = &R.field[Mine()];
    int i;
    double t = SongTimeNow();
    if (t < R.lastT)
        t = R.lastT;   // (never backwards)
    R.songT = t;
    R.songBeat = RhSong_BeatAt(&e->song, t);
    // steps: from SDL's events (timed), or the test harness's buttons at the frame
    PollHeld(gbaKeys);
    if (R.autoplay)
        Autoplay(me, t);
    else if (Platform_IsTestMode())
    {
        static const uint16_t bits[RH_COLS] = {K_LEFT, K_DOWN, K_UP, K_RIGHT};
        for (i = 0; i < RH_COLS; i++)
        {
            if ((gbaKeys & bits[i]) && !(R.keysWas & bits[i]))
                RhPlay_Press(&me->play, i, t);
            if (!(gbaKeys & bits[i]) && (R.keysWas & bits[i]))
                RhPlay_Release(&me->play, i, t);
        }
    }
    else
    {
        for (i = 0; i < R.pressCount; i++)
        {
            double pt = R.presses[i].t;
            if (pt > t)
                pt = t;
            if (R.presses[i].down)
                RhPlay_Press(&me->play, R.presses[i].col, pt);
            else
                RhPlay_Release(&me->play, R.presses[i].col, pt);
        }
        // the held state wins (a release lost between frames)
        for (i = 0; i < RH_COLS; i++)
            if (!R.heldNow[i] && me->play.held[i])
                RhPlay_Release(&me->play, i, t);
    }
    R.keysWas = gbaKeys;
    R.pressCount = 0;
    for (i = 0; i < RH_COLS; i++)
        me->held[i] = R.heldNow[i];
    RhPlay_Update(&me->play, t, t - R.lastT);
    R.lastT = t;
    MP_Rhythm_Dancing((uint8_t)(R.heldNow[0] | R.heldNow[1] << 1 | R.heldNow[2] << 2 | R.heldNow[3] << 3));
    // online: tell the friend; their field from what they said
    if (R.versus && !R.peerGone)
    {
        NoteEvents(&me->play);
        if (++R.netTimer >= NET_STATE_EVERY)
        {
            R.netTimer = 0;
            SendState();
        }
        if (RhAudio_Now() - R.peerHeard > PEER_SILENCE)
        {
            MPNet_Log("dance: nothing from the friend for %.1f s", RhAudio_Now() - R.peerHeard);
            Rhythm_PeerGone((uint8_t)R.b.peer);
        }
    }
    for (i = 0; i < 2; i++)
        if (R.field[i].used)
            RhField_Events(&R.field[i], dt, i == Mine());
    // the announcer between steps now and then
    R.commentWait -= dt;
    if (R.commentWait <= 0 && !RhAudio_Announcing() && t > 0 && !me->play.failed)
    {
        R.commentWait = 9 + (rand() % 6);
        if (me->play.life >= 0.999f)
            RhAudio_Announce("gameplay comment hot");
        else if (me->play.life < 0.25f)
            RhAudio_Announce("gameplay comment danger");
        else if (me->play.combo > 30)
            RhAudio_Announce("gameplay comment good");
    }
    // START held for a second and a half gives up
    if (gbaKeys & K_START)
    {
        R.startHeld += dt;
        if (R.startHeld > 1.5f && !me->play.failed)
        {
            me->play.failed = 1;
            me->play.life = 0;
            me->play.failTime = t;
        }
    }
    else
        R.startHeld = 0;
    // drawing
    DrawSongBack();
    if (R.versus && !R.peerGone)
    {
        RhField_Draw(&R.field[0], &e->song, t, R.songBeat, 60);
        RhField_Draw(&R.field[1], &e->song, t, R.songBeat, RH_W - 60);
    }
    else
        RhField_Draw(me, &e->song, t, R.songBeat, Mine() ? RH_W - 60 : 60);
    DrawPlayFrames();
    {
        // READY, then HERE WE GO, before the first step
        double first = me->play.count ? me->play.notes[0].time : 0;
        double until = first - t;
        const struct RhPic *ready = RhPic_Get("ready"), *go = RhPic_Get("herewego");
        if (R.t < 1.6f && ready)
            RhGfx_Pic(ready, 0, (RH_W - ready->w) / 2, 70, 0, R.t > 1.3f ? (int)((1.6f - R.t) / 0.3f * 256) : 256);
        else if (R.t < 2.8f && until > 0.2 && go)
        {
            static int sSaid;
            if (R.t < 1.7f)
                sSaid = 0;
            if (!sSaid)
            {
                sSaid = 1;
                RhAudio_Announce(R.stage == RHYTHM_STAGES - 1 ? "gameplay here we go final" : "gameplay here we go normal");
            }
            RhGfx_Pic(go, 0, (RH_W - go->w) / 2, 70, 0, R.t > 2.5f ? (int)((2.8f - R.t) / 0.3f * 256) : 256);
        }
    }
    // the end: everyone failed, or the steps are done and the music (or 3 s) is over
    {
        int alive = 0, done = 1;
        for (i = 0; i < 2; i++)
        {
            struct RhField *f = &R.field[i];
            if (!f->used)
                continue;
            if (!RhField_Failed(f))
                alive = 1;
            if (!f->remote && !f->play.finished && !f->play.failed)
                done = 0;
            if (f->remote && !(f->netFailed) && f->play.notes && t < f->play.notes[f->play.count - 1].endTime + 1)
                done = 0;
        }
        if (!alive)
        {
            FinishSong();
            return;
        }
        if (done)
        {
            double lastStep = 0;
            for (i = 0; i < me->play.count; i++)
                if (me->play.notes[i].endTime > lastStep)
                    lastStep = me->play.notes[i].endTime;
            if (!RhAudio_MusicPlaying() || t > lastStep + 3.0)
                FinishSong();
        }
    }
}

static void FinishSong(void)
{
    struct RhField *me = &R.field[Mine()];
    RhAudio_StopMusic(me->play.failed ? 0.3 : 1.5);
    RhAudio_Announce(me->play.failed ? "gameplay failed" : "gameplay cleared");
    RhAudio_Sfx("snd_swoosh");
    if (R.versus && !R.peerGone)
        SendState();
    MP_Rhythm_Dancing(0);
    Go(ST_SONG_END);
}

static void StateSongEnd(void)
{
    struct RhField *me = &R.field[Mine()];
    struct RhEntry *e = Song(R.song);
    const struct RhPic *pic = RhPic_Get(me->play.failed ? "failed" : "cleared");
    DrawSongBack();
    if (R.versus && !R.peerGone)
    {
        RhField_Draw(&R.field[0], &e->song, R.songT, R.songBeat, 60);
        RhField_Draw(&R.field[1], &e->song, R.songT, R.songBeat, RH_W - 60);
    }
    else
        RhField_Draw(me, &e->song, R.songT, R.songBeat, Mine() ? RH_W - 60 : 60);
    DrawPlayFrames();
    RhGfx_Fill(0, 60, RH_W, 40, RH_RGB(0, 0, 0), R.t < 0.3f ? (int)(R.t / 0.3f * 160) : 160);
    if (pic)
    {
        int w = R.t < 0.2f ? (int)(pic->w * (1.6f - R.t * 3)) : pic->w;
        int h = R.t < 0.2f ? (int)(pic->h * (1.6f - R.t * 3)) : pic->h;
        RhGfx_PicScaled(pic, 0, RH_W / 2, 80, w, h, 256);
    }
    if (R.t > 2.6f)
    {
        // the records and the counters, once
        const struct RhPlay *p = &me->play;
        int judges[RH_J_COUNT + 2], k;
        int grade = RhPlay_Grade(p), fc = RhPlay_FullCombo(p);
        for (k = 0; k < RH_J_COUNT; k++)
            judges[k] = p->rowCount[k];
        judges[RH_J_COUNT] = p->ok;
        judges[RH_J_COUNT + 1] = p->ng;
        if (!p->failed)
        {
            uint32_t key = RhSongs_ChartKey(e, me->diff);
            char names[1][16];
            uint32_t best[1];
            uint8_t g[1];
            int n = MP_Rhythm_Records(key, names, best, g, 1);
            R.newRecord = n == 0 || (uint32_t)p->score > best[0];
            MP_Rhythm_SubmitScore(key, FullTitle(e), me->diff, me->meter, (uint32_t)p->score, grade, p->maxCombo, fc,
                                  judges);
            R.stagesCleared++;
        }
        {
            struct RhythmStage *st = &R.pendingStage;
            memset(st, 0, sizeof(*st));
            st->title = e->song.title;
            st->chart = RhSongs_ChartKey(e, me->diff);
            st->song = SongKey(e);
            st->diff = (uint8_t)me->diff;
            st->meter = (uint8_t)me->meter;
            st->cleared = !p->failed;
            st->grade = (uint8_t)grade;
            st->fullCombo = (uint8_t)fc;
            st->speed = me->opt.speed;
            st->appearance = me->opt.appearance;
            st->marvelous = me->opt.marvelous;
            st->maxCombo = (uint16_t)p->maxCombo;
            st->score = (uint32_t)p->score;
            st->record = (uint8_t)(R.newRecord && !p->failed);
            R.stagePending = 1;
            R.stagesPlayed++;
            if (fc == 0)
                R.allFullCombo = 0;
            if (grade > RH_GRADE_AAA)
                R.allAaa = 0;
        }
        RhAudio_PlayMenuMusic();
        Go(ST_RESULTS);
        {
            static const char *const calls[] = {"evaluation aaa", "evaluation aaa", "evaluation aa", "evaluation a",
                                                "evaluation b", "evaluation c", "evaluation d", "evaluation e"};
            RhAudio_Announce(R.newRecord && !p->failed ? "evaluation new record" : calls[grade]);
        }
    }
}

static void DrawResults(const struct RhField *f, int x)
{
    static const char *const names[] = {"MARVELOUS", "PERFECT", "GREAT", "GOOD", "BOO", "MISS", "O.K.", "N.G."};
    const struct RhPic *grades = RhPic_Get("grades"), *digits = RhPic_Get("small_digits");
    const struct RhPlay *p = &f->play;
    int counts[8], k, grade;
    if (f->remote)
    {
        // the friend's: from their results message, else what we saw
        grade = f->netFailed ? RH_GRADE_E : -1;
        RhGfx_Text(RH_FONT_NORMAL, x + 52, 24, R.b.peerName, RH_RGB(255, 255, 255), RH_CENTER);
        RhGfx_Text(RH_FONT_SMALL, x + 52, 44, f->netFailed ? "FAILED" : "CLEARED", RH_RGB(255, 230, 120), RH_CENTER);
        RhGfx_Text(RH_FONT_SMALL, x + 4, 60, "MAX COMBO", RH_RGB(230, 255, 160), RH_LEFT);
        RhGfx_Number(digits, x + 100, 60, (uint32_t)f->netMaxCombo, RH_RIGHT, 1, 256, 0);
        RhGfx_Text(RH_FONT_SMALL, x + 4, 72, "SCORE", RH_RGB(230, 255, 160), RH_LEFT);
        RhGfx_Number(RhPic_Get("score_digits"), x + 100, 72, (uint32_t)f->netScore, RH_RIGHT, 9, 256, 0);
        (void)grade;
        return;
    }
    grade = RhPlay_Grade(p);
    counts[0] = p->rowCount[RH_J_MARVELOUS];
    counts[1] = p->rowCount[RH_J_PERFECT];
    counts[2] = p->rowCount[RH_J_GREAT];
    counts[3] = p->rowCount[RH_J_GOOD];
    counts[4] = p->rowCount[RH_J_BOO];
    counts[5] = p->rowCount[RH_J_MISS];
    counts[6] = p->ok;
    counts[7] = p->ng;
    if (!f->opt.marvelous)
    {
        counts[1] += counts[0];
        counts[0] = -1;
    }
    if (grades)
        RhGfx_PicScaled(grades, grade, x + 26, 38, grades->w, grades->h, 256);
    {
        int fc = RhPlay_FullCombo(p);
        static const char *const fcNames[] = {"", "FULL COMBO", "PERFECT FULL COMBO", "MARVELOUS FULL COMBO"};
        if (fc == 3 && !f->opt.marvelous)
            fc = 2;
        if (fc)
            RhGfx_TextFit(RH_FONT_SMALL, x + 52, 54, fcNames[fc], RH_RGB(255, 230, 80), RH_CENTER, 104);
    }
    for (k = 0; k < 8; k++)
    {
        int y = 64 + (k - (counts[0] < 0 ? 1 : 0)) * 8;
        if (counts[k] < 0)
            continue;
        RhGfx_Text(RH_FONT_SMALL, x + 4, y - 1, names[k], RH_RGB(230, 255, 160), RH_LEFT);
        RhGfx_Number(digits, x + 100, y, (uint32_t)counts[k], RH_RIGHT, 1, 256, 0);
    }
    RhGfx_Text(RH_FONT_SMALL, x + 4, 119, "MAX COMBO", RH_RGB(230, 255, 160), RH_LEFT);
    RhGfx_Number(digits, x + 100, 120, (uint32_t)p->maxCombo, RH_RIGHT, 1, 256, 0);
    RhGfx_Number(RhPic_Get("eval_digits"), x + 100, 128, (uint32_t)p->score, RH_RIGHT, 9, 256, 0);
    if (p->offsetN > 0)
    {
        char st[40];
        snprintf(st, sizeof(st), "AVG %+.1f ms  FAST %d  SLOW %d", p->offsetSum / p->offsetN, p->fastCount, p->slowCount);
        RhGfx_TextFit(RH_FONT_SMALL, x + 52, 138, st, RH_RGB(200, 200, 255), RH_CENTER, 104);
    }
}

static void StateResults(void)
{
    struct RhField *me = &R.field[Mine()];
    struct RhEntry *e = Song(R.song);
    DrawMenuBack("RESULTS");
    if (e)
        RhGfx_TextFit(RH_FONT_SMALL, RH_W / 2, 16, FullTitle(e), RH_RGB(255, 255, 255), RH_CENTER, 230);
    DrawResults(&R.field[0], 8);
    if (R.field[1].used)
        DrawResults(&R.field[1], RH_W - 112);
    if (R.newRecord && !me->play.failed && ((int)(R.t * 4) & 1))
        RhGfx_Text(RH_FONT_SMALL, (Mine() ? RH_W - 112 : 8) + 78, 30, "NEW RECORD!", RH_RGB(255, 80, 80), RH_CENTER);
    if (R.localNext)
        Hint("Waiting for your friend...");
    else
        Hint("A: next");
    if (!R.localNext && R.t > 1.0f && (sNew & (K_A | K_START) || R.t > 20))
    {
        uint8_t n = RN_NEXT;
        R.localNext = 1;
        Send(&n, 1);
        RhAudio_Sfx("snd_start");
    }
    if (R.localNext && (!R.versus || R.peerGone || R.peerNext || R.t > 30))
    {
        if (R.stagePending)
        {
            // VERSUS: a win is a higher score than the friend's (a tie is no win)
            R.pendingStage.versus = (int8_t)(R.versus && !R.peerGone && R.field[Other()].used
                                             ? ((int32_t)R.pendingStage.score > R.field[Other()].netScore ? 1 : 0) : -1);
            MP_Rhythm_StageDone(&R.pendingStage);
            R.stagePending = 0;
        }
        // the next stage, or the end of the credit
        if (me->play.failed || R.stage >= RHYTHM_STAGES - 1)
        {
            RhAudio_StopMusic(0.5);
            RhAudio_Announce("game over");
            Go(ST_GAMEOVER);
            return;
        }
        R.stage++;
        R.ready[0] = R.ready[1] = 0;
        R.previewFor = -2;
        R.peerNext = R.localNext = 0;
        if (!R.versus || R.peerGone)
            R.b.picker = 1;
        BuildWheel(R.song);
        RhAudio_Announce("select music intro");
        Go(ST_MUSIC);
    }
}

static void StateGameOver(void)
{
    const struct RhPic *pic = RhPic_Get("gameover");
    RhGfx_Clear(0);
    if (pic)
        RhGfx_Pic(pic, 0, (RH_W - pic->w) / 2, 56, 0, R.t < 0.4f ? (int)(R.t / 0.4f * 256) : 256);
    if (R.t > 1)
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 100, "THANK YOU FOR PLAYING!", RH_RGB(255, 255, 255), RH_CENTER);
    if (R.t > 3.5f || (R.t > 1.5f && (sNew & (K_A | K_START | K_B))))
        EndCredit();
}

// No songs here, or new ones out: download them (src/platform/updater.c), with the progress on the screen
static void StateNoSongs(void)
{
    long done, total;
    const char *err;
    int st = Updater_SongsState(&done, &total, &err);
    char line[96];
    DrawMenuBack(R.haveSongs ? "NEW SONGS" : "DANCE MACHINE");
    switch (st)
    {
    case SONGS_IDLE:
    case SONGS_CHECKING:
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 60, "Looking for the songs...", RH_RGB(255, 255, 255), RH_CENTER);
        Hint("B: leave");
        break;
    case SONGS_AVAILABLE:
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 44, R.haveSongs ? "New songs are out!" : "The songs aren't on this PC yet.",
                   RH_RGB(255, 255, 255), RH_CENTER);
        snprintf(line, sizeof(line), "Download them now? (%ld MB)", (total + 524288) / 1048576);
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 64, line, RH_RGB(230, 255, 160), RH_CENTER);
        RhGfx_Text(RH_FONT_SMALL, RH_W / 2, 86, "Only the first time: they stay on this PC.", RH_RGB(255, 255, 255), RH_CENTER);
        Hint(R.haveSongs ? "A: download   B: play with the songs I have" : "A: download   B: leave");
        if (R.t > 0.3f && (sNew & (K_A | K_START)))
        {
            RhAudio_Sfx("snd_start");
            Updater_SongsDownload();
        }
        break;
    case SONGS_DOWNLOADING:
    case SONGS_INSTALLING:
    {
        int w = total > 0 ? (int)((double)done / total * 200) : 0;
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 50, st == SONGS_DOWNLOADING ? "Downloading the songs..." : "Unpacking the songs...",
                   RH_RGB(255, 255, 255), RH_CENTER);
        RhGfx_Fill(20, 72, 200, 10, RH_RGB(20, 40, 10), 256);
        RhGfx_Fill(20, 72, w > 200 ? 200 : w, 10, RH_RGB(200, 255, 80), 256);
        if (st == SONGS_DOWNLOADING)
            snprintf(line, sizeof(line), "%ld / %ld MB", done / 1048576, total / 1048576);
        else
            snprintf(line, sizeof(line), "%ld / %ld files", done, total);
        RhGfx_Text(RH_FONT_SMALL, RH_W / 2, 88, line, RH_RGB(230, 255, 160), RH_CENTER);
        Hint(st == SONGS_DOWNLOADING ? "B: stop" : "");
        if (st == SONGS_DOWNLOADING && (sNew & K_B))
        {
            Updater_SongsCancel();
            RhAudio_Sfx("snd_back");
        }
        return;   // (B doesn't leave mid-download)
    }
    case SONGS_DONE:
        RhAudio_Sfx("snd_unlock");
        R.haveSongs = RhSongs_Scan() > 0;
        MPNet_Log("dance: %d songs after the download", RhSongs_Count());
        if (R.haveSongs)
        {
            R.openGroup = -1;
            BuildWheel(-1);
            RhAudio_PlayMenuMusic();
            RhAudio_Announce("select style intro");
            Go(ST_STYLE);
            return;
        }
        Updater_SongsCheck();
        break;
    default:
    {
        char path[300];
        RhGfx_Text(RH_FONT_NORMAL, RH_W / 2, 44, R.haveSongs ? "No new songs." : "No songs are installed.",
                   RH_RGB(255, 255, 255), RH_CENTER);
        if (st == SONGS_FAILED && err && err[0])
            RhGfx_TextFit(RH_FONT_SMALL, RH_W / 2, 62, err, RH_RGB(255, 140, 140), RH_CENTER, 230);
        snprintf(path, sizeof(path), "%s/Songs/", RhSongs_Root());
        RhGfx_Text(RH_FONT_SMALL, RH_W / 2, 80, "StepMania songs go in", RH_RGB(230, 255, 160), RH_CENTER);
        RhGfx_TextFit(RH_FONT_SMALL, RH_W / 2, 90, path, RH_RGB(255, 255, 255), RH_CENTER, 230);
        Hint(R.haveSongs ? "A / B: play with the songs I have" : "A / B: leave");
        if (R.t > 0.5f && (sNew & (K_A | K_START)))
            sNew |= K_B;
        break;
    }
    }
    if (R.t > 0.3f && (sNew & K_B))
    {
        RhAudio_Sfx("snd_back");
        if (R.haveSongs)
        {
            RhAudio_PlayMenuMusic();
            RhAudio_Announce("select style intro");
            Go(ST_STYLE);
        }
        else
            EndCredit();
    }
}

int Rhythm_Step(uint16_t *frame, uint16_t gbaKeys)
{
    double now = RhAudio_Now();
    float dt = Platform_IsTestMode() ? 1 / 60.0f : (float)(now - R.last);
    if (R.state == ST_OFF)
        return 0;
    if (R.state == ST_CLOSING)
    {
        // the last picture collapses into a line, the line into a point; then the map is the picture again
        Crt_Apply(sLastFrame, frame, RH_W, RH_H, CRT_PHASE_OFF, (double)++R.crtOff / CRT_OFF_FRAMES, CRT_FMT_BGR555);
        if (R.crtOff >= CRT_OFF_FRAMES)
        {
            R.state = ST_OFF;
            MP_Rhythm_Ended();
        }
        return 1;
    }
    if (dt > 0.1f)
        dt = 0.1f;
    if (dt < 0)
        dt = 0;
    R.last = now;
    R.t += dt;
    sNew = gbaKeys & ~sKeys;
    sKeys = gbaKeys;
    switch (R.state)
    {
    case ST_CAUTION: StateCaution(); break;
    case ST_STYLE: StateStyle(); break;
    case ST_DIFFICULTY: StateDifficulty(); break;
    case ST_MUSIC: StateMusic(); break;
    case ST_OPTIONS: StateOptions(); break;
    case ST_STAGE: StateStage(); break;
    case ST_PLAY: StatePlay(gbaKeys, dt); break;
    case ST_SONG_END: StateSongEnd(); break;
    case ST_RESULTS: StateResults(); break;
    case ST_GAMEOVER: StateGameOver(); break;
    case ST_NOSONGS: StateNoSongs(); break;
    }
    // a friend on the other pad gone quiet (not while playing: StatePlay looks)
    if (R.state != ST_OFF && R.state != ST_CLOSING && R.versus && !R.peerGone && R.state != ST_PLAY
        && RhAudio_Now() - R.peerHeard > 60)
    {
        MPNet_Log("dance: nothing from the friend for a minute");
        Rhythm_PeerGone((uint8_t)R.b.peer);
    }
    if (R.state == ST_OFF)
        return 0;
    if (R.versus && !R.peerGone && R.state != ST_PLAY)
    {
        // keep the friend hearing from us
        static int sPing;
        if (++sPing >= 60)
        {
            uint8_t p[13];
            sPing = 0;
            p[0] = RN_DIFF;
            p[1] = (uint8_t)R.diffWant[Mine()];
            p[2] = (uint8_t)R.diffConfirmed[Mine()];
            PutOptions(p + 3, &R.field[Mine()].opt);
            Send(p, sizeof(p));
        }
    }
    RhGfx_ToGba(frame);
    memcpy(sLastFrame, frame, sizeof(sLastFrame));
    if (R.crtOn < CRT_ON_FRAMES)
    {
        // the screen comes on: a white line widens, then the picture opens from it
        memcpy(sCrtFrame, frame, sizeof(sCrtFrame));
        Crt_Apply(sCrtFrame, frame, RH_W, RH_H, CRT_PHASE_ON, (double)++R.crtOn / CRT_ON_FRAMES, CRT_FMT_BGR555);
    }
    return 1;
}

void Rhythm_MixAudio(float *stereo, int frames)
{
    if (R.state == ST_OFF)
        return;
    RhAudio_Mix(stereo, frames);
}
