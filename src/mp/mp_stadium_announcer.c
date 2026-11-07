// POKéMON STADIUM's announcer (docs/STADIUM.md "The announcer"): Stadium's own 823 voice clips, from the player's
// ROM (stadium/announcer.s1: tools/stadium/mort/make_voice_pack.py, or the ROM itself dropped in stadium/), or the
// Japanese ones (POKEMON STADIUM 2's 831, stadium/announcer_jp.s1), decoded by SubDrag's MORT decoder (mort/) on a
// worker thread and mixed over the game's sound (music_player.c). OPTIONS > BATTLE > ANNOUNCER: OFF / ENGLISH /
// JAPANESE.
//
// The battle's own messages are the cues (battle_message.c BufferStringBattle -> MP_Announcer_OnBattleString):
// a POKéMON sent out, a move used, super / not very effective, critical hits, misses, failures, statuses, faints,
// switches, the last POKéMON, the win. Kai's battles run much faster than Stadium's, so every line is weighed
// against the time it has before the battle moves on:
// - each clip's length is known from its header (no decoding needed);
// - the time a cue has is learned from this battle itself: for each kind of cue, how many frames went by before
//   the next message (a running average, kept apart for when A is being pressed to hurry the text along), so the
//   text speed, the BATTLE SPEED, the animations and the player's mashing are all in it; before anything is learned,
//   a guess from the text speed and the animation speed;
// - a line plays only if it ends within that time (a little over is allowed, never when a faint or the battle's end
//   is coming next): otherwise a shorter line for the same thing, or nothing;
// - a line is never cut off: while one plays, a new cue is dropped, or, for a faint or the win, kept for a moment
//   and said if the line ends soon enough for it to still fit;
// - lines just said aren't said again right away.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "battle_message.h"
#include "battle_anim.h"
#include "main.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "constants/battle_string_ids.h"
#include "constants/global.h"
#include "constants/moves.h"
#include "constants/species.h"

int Mort_Decode(const unsigned char *buf, int size, unsigned off, unsigned len, short **out, unsigned *count);   // mort/mort_api.cpp
const char *Updater_ExeFile(const char *name);
bool8 MP_BattlePlayer_Choosing(void);   // battle_controller_player.c
extern bool8 gMpStadiumBattle;

#define CLIPS 823
#define RATE 16000
#define OUT_RATE 42048
#define MAX_WANT 1024

// ---------------------------------------------------------------------------------------------------------------
// The clips Stadium has for each kind of cue (clip numbers: docs/STADIUM.md "The announcer")

enum {
    CUE_SENDOUT, CUE_MOVE, CUE_SUPER, CUE_NOTVERY, CUE_CRIT, CUE_MISS, CUE_FAIL, CUE_STATUS, CUE_FAINT, CUE_RETURN,
    CUE_STAT, CUE_STATE, CUE_OTHER, CUE_COUNT
};
enum { PRIO_AMBIENT, PRIO_MOVE, PRIO_SENDOUT, PRIO_REACT, PRIO_FAINT, PRIO_RESULT, PRIO_INTRO };

static const u16 sFirstMove[] = {181, 182};
static const u16 sTurnPrompt[] = {183, 184, 185, 186, 188};
static const u16 sIdle[] = {749, 750, 751};
static const u16 sHot[] = {756, 762, 780, 790, 814, 820};
static const u16 sSwitchPlayer[] = {245, 246, 247, 248, 251};
static const u16 sSwitchFoe[] = {253};
static const u16 sSuper[] = {261, 262, 267};
static const u16 sNotVery[] = {270, 317, 319};
static const u16 sMiss[] = {289, 293, 299, 337, 339};
static const u16 sFail[] = {290, 291, 335, 336};
static const u16 sCrit[] = {311, 312};
static const u16 sFaint[] = {349, 351, 357, 358};
static const u16 sDoubleFaint[] = {360, 821};
static const u16 sLastOne[] = {166, 167, 783};
static const u16 sBothLast[] = {784};
static const u16 sWon[] = {365};
static const u16 sSleep[] = {285}, sFreeze[] = {286}, sPara[] = {287}, sConfuse[] = {300}, sPoison[] = {301}, sBurn[] = {346};
// stats down (ATTACK, DEFENSE, SPEED, SP. ATK, SP. DEF, ACCURACY, EVASION: STAT_ATK..) and up, by stat
static const u16 sStatDown[8] = {0, 327, 328, 332, 329, 329, 330, 331};
static const u16 sStatUp[8] = {0, 527, 528, 529, 530, 533, 531, 532};
// the moments between hits (transcribed from the clips: docs/STADIUM.md "The announcer")
static const u16 sWokeUp[] = {520, 521}, sThawed[] = {522}, sFastAsleep[] = {546, 170}, sIsFrozen[] = {547, 169};
static const u16 sCantMove[] = {548}, sHurtItself[] = {552}, sNoMoreConfused[] = {555}, sIsConfused[] = {553, 554};
static const u16 sHealed[] = {523}, sSubMade[] = {558}, sSubGone[] = {538, 539}, sSubHit[] = {283, 341};
static const u16 sTransform[] = {557}, sFly[] = {559}, sDig[] = {560}, sCharge[] = {564}, sPoisonHurt[] = {171};
static const u16 sBurnHurt[] = {172}, sNothing[] = {572}, sHaze[] = {581};
// the turn: how the two stand
static const u16 sBothFit[] = {752}, sBothSpent[] = {753}, sHpGap[] = {754};
static const u16 sCount32[] = {768}, sCount31[] = {769}, sCount22[] = {770}, sCount23[] = {772}, sCount12[] = {774}, sCount11[] = {773, 761};
// a POKeMON coming out hurt, or after one fell
static const u16 sReadyToDrop[] = {173}, sHurtAlready[] = {178}, sAvenge[] = {179};
#define LIST(a) a, ARRAY_COUNT(a)
#define CLIP_SENDOUT(dex) (368 + (dex))   // "Oh! It's ...!" (dex 1-151)
#define CLIP_MOVE(move) (583 + (move))    // the move's name (moves 1-165)

// ---------------------------------------------------------------------------------------------------------------
// The pack and the decoded clips

struct Clip
{
    u32 off, len;      // the MORT stream in the archive
    u16 frames;        // how long it plays, in game frames (60 a second)
    SDL_atomic_t state;   // 0 not decoded, 1 asked for, 2 ready, 3 didn't decode
    s16 *pcm;
    u32 samples;
    float gain;        // to bring its peak to the same loudness as the others
};

static struct
{
    bool8 tried, ok, jp;   // jp: the Japanese voices (POKEMON STADIUM 2's)
    u8 *data;
    u32 size;
    struct Clip clip[CLIPS];
    SDL_mutex *lock;
    SDL_cond *wake;
    SDL_Thread *thread;
    u16 want[MAX_WANT];   // the worker's queue
    u16 wantHead, wantCount;
    SDL_atomic_t decoding;   // the worker is decoding a clip
    u8 lang;                 // the voices asked for when the pack was loaded (0 English, 1 Japanese)
} sPack;

static u32 BE32(const u8 *p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
static u16 BE16(const u8 *p) { return (u16)(p[0] << 8 | p[1]); }

// The Japanese game (POKEMON STADIUM 2, the one the USA's POKeMON STADIUM was made from) has 831 clips: the same
// lines in the same order, with 8 more of its own among the remarks. sJpShift says how far each USA clip number
// moves in it (from that clip on), so the Japanese clips are laid out under the USA numbers and everything else
// here stays the same (clips were matched by transcribing both: docs/STADIUM.md "The Japanese announcer").
#define CLIPS_JP 831
static const struct { u16 from, add; } sJpShift[] = {
    {181, 2}, {217, 4}, {221, 6}, {365, 8},
};

static u16 JpClip(u16 us)
{
    u32 i;
    u16 add = 0;
    for (i = 0; i < ARRAY_COUNT(sJpShift); i++)
        if (us >= sJpShift[i].from)
            add = sJpShift[i].add;
    return us + add;
}

// The speech archive's clips (an S1 of three S1s of MORT streams). FALSE: not the archive.
static bool8 ReadArchive(void)
{
    static struct Clip sAll[CLIPS_JP];
    u32 c, i, n = 0;
    if (sPack.size < 64 || memcmp(sPack.data, "S1", 2) != 0 || BE16(sPack.data + 2) != 3)
        return FALSE;
    for (c = 0; c < 3; c++)
    {
        u32 child = BE32(sPack.data + 4 + c * 8), count;
        if (child + 4 > sPack.size || memcmp(sPack.data + child, "S1", 2) != 0)
            return FALSE;
        count = BE16(sPack.data + child + 2);
        for (i = 0; i < count; i++, n++)
        {
            struct Clip *k;
            if (n >= CLIPS_JP)
                return FALSE;
            k = &sAll[n];
            memset(k, 0, sizeof(*k));
            k->off = child + BE32(sPack.data + child + 4 + i * 8);
            k->len = BE32(sPack.data + child + 8 + i * 8);
            if (k->off + 8 > sPack.size || memcmp(sPack.data + k->off, "MORT", 4) != 0)
                return FALSE;
            // the header's frame count (160 samples each) at its own rate
            k->frames = (u16)((u32)BE16(sPack.data + k->off + 4) * 160 * 60 / (BE16(sPack.data + k->off + 6) ? BE16(sPack.data + k->off + 6) : RATE));
        }
    }
    if (n != CLIPS && n != CLIPS_JP)
        return FALSE;
    sPack.jp = n == CLIPS_JP;
    for (i = 0; i < CLIPS; i++)
        sPack.clip[i] = sAll[sPack.jp ? JpClip(i) : i];
    return TRUE;
}

static u8 *ReadFile(const char *path, u32 *size)
{
    FILE *f = fopen(path, "rb");
    long n;
    u8 *data;
    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = n > 0 ? malloc(n) : NULL;
    if (data == NULL || fread(data, 1, n, f) != (size_t)n)
    {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size = (u32)n;
    return data;
}

// The player's own ROM dropped in stadium/ (any byte order): the archive is copied out of it. Pokemon Stadium (USA)
// keeps it at 0x197C1E0, POKEMON STADIUM 2 (Japan) at 0x1975C00.
static bool8 FromRom(const char *path)
{
    static const u32 sAt[] = {0x197C1E0, 0x1975C00};
    u32 size, i, end = 0, c, at = 0;
    u8 *rom = ReadFile(path, &size);
    if (rom == NULL || size < sAt[0] + 64)
    {
        free(rom);
        return FALSE;
    }
    if (rom[0] == 0x37 && rom[1] == 0x80)   // .v64: bytes swapped in pairs
        for (i = 0; i + 1 < size; i += 2) { u8 t = rom[i]; rom[i] = rom[i + 1]; rom[i + 1] = t; }
    else if (rom[0] == 0x40 && rom[1] == 0x12)   // .n64: little-endian words
        for (i = 0; i + 3 < size; i += 4) { u8 a = rom[i], b = rom[i + 1]; rom[i] = rom[i + 3]; rom[i + 1] = rom[i + 2]; rom[i + 2] = b; rom[i + 3] = a; }
    for (i = 0; i < ARRAY_COUNT(sAt) && at == 0; i++)
        if (memcmp(rom + sAt[i], "S1", 2) == 0 && BE16(rom + sAt[i] + 2) == 3)
            at = sAt[i];
    if (at == 0)
    {
        free(rom);
        return FALSE;
    }
    for (c = 0; c < 3; c++)
    {
        u32 e = BE32(rom + at + 4 + c * 8) + BE32(rom + at + 8 + c * 8);
        if (e > end)
            end = e;
    }
    if (at + end > size)
    {
        free(rom);
        return FALSE;
    }
    sPack.data = malloc(end);
    if (sPack.data != NULL)
        memcpy(sPack.data, rom + at, end);
    sPack.size = end;
    free(rom);
    return sPack.data != NULL;
}

static int Worker(void *unused)
{
    (void)unused;
    for (;;)
    {
        u16 id;
        struct Clip *k;
        short *pcm = NULL;
        unsigned count = 0;
        SDL_LockMutex(sPack.lock);
        while (sPack.wantCount == 0)
            SDL_CondWait(sPack.wake, sPack.lock);
        id = sPack.want[sPack.wantHead];
        sPack.wantHead = (sPack.wantHead + 1) % MAX_WANT;
        sPack.wantCount--;
        SDL_AtomicSet(&sPack.decoding, 1);
        SDL_UnlockMutex(sPack.lock);
        k = &sPack.clip[id];
        if (Mort_Decode(sPack.data, (int)sPack.size, k->off, k->len, &pcm, &count) && count > 0)
        {
            unsigned i;
            int peak = 1;
            for (i = 0; i < count; i++)
                if (abs(pcm[i]) > peak)
                    peak = abs(pcm[i]);
            k->pcm = pcm;
            k->samples = count;
            k->gain = 0.62f * 32768.0f / (float)peak;   // every line as loud as the others
            if (k->gain > 4.0f)
                k->gain = 4.0f;
            if (getenv("MOEMON_ANNOUNCER_DUMP") != NULL)
            {
                // tests: each decoded clip as a WAV (MOEMON_ANNOUNCER_DUMP=folder), to listen to or transcribe
                char path[700];
                FILE *f;
                snprintf(path, sizeof(path), "%s/%03d.wav", getenv("MOEMON_ANNOUNCER_DUMP"), id);
                f = fopen(path, "wb");
                if (f != NULL)
                {
                    u32 bytes = count * 2, v;
                    u16 h;
                    fwrite("RIFF", 1, 4, f); v = 36 + bytes; fwrite(&v, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f);
                    v = 16; fwrite(&v, 4, 1, f); h = 1; fwrite(&h, 2, 1, f); fwrite(&h, 2, 1, f);
                    v = RATE; fwrite(&v, 4, 1, f); v = RATE * 2; fwrite(&v, 4, 1, f); h = 2; fwrite(&h, 2, 1, f);
                    h = 16; fwrite(&h, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&bytes, 4, 1, f);
                    fwrite(pcm, 2, count, f);
                    fclose(f);
                }
            }
            SDL_AtomicSet(&k->state, 2);
        }
        else
        {
            free(pcm);
            SDL_AtomicSet(&k->state, 3);
        }
        SDL_AtomicSet(&sPack.decoding, 0);
    }
    return 0;
}

static void Want(u16 id);

static s16 VoiceClip(void);

// The pack loaded before for the other language, let go (only while no line plays; the worker is told to stop first)
static void Unload(void)
{
    u32 i;
    if (sPack.lock != NULL)
    {
        SDL_LockMutex(sPack.lock);
        sPack.wantCount = 0;
        SDL_UnlockMutex(sPack.lock);
        while (SDL_AtomicGet(&sPack.decoding))
            SDL_Delay(1);
    }
    for (i = 0; i < CLIPS; i++)
    {
        free(sPack.clip[i].pcm);
        memset(&sPack.clip[i], 0, sizeof(sPack.clip[i]));
    }
    free(sPack.data);
    sPack.data = NULL;
    sPack.size = 0;
    sPack.ok = sPack.jp = FALSE;
}

// The voices in one language: its pack (stadium/announcer.s1 or announcer_jp.s1), else that game's ROM in stadium/
static void ReadVoices(u8 lang)
{
    static const char *const sRoms[2][4] = {
        {"stadium/Pokemon Stadium (USA).z64", "stadium/Pokemon Stadium (USA).v64", "stadium/Pokemon Stadium (USA).n64",
         "stadium/stadium.z64"},
        {"stadium/Pokemon Stadium 2 (Japan).z64", "stadium/Pokemon Stadium 2 (Japan).v64",
         "stadium/Pokemon Stadium 2 (Japan).n64", "stadium/stadium_jp.z64"}};
    char rom[600];
    const char *path = getenv(lang ? "MOEMON_STADIUM_VOICE_JP" : "MOEMON_STADIUM_VOICE");
    u32 i;
    if (path == NULL)
        path = Updater_ExeFile(lang ? "stadium/announcer_jp.s1" : "stadium/announcer.s1");
    sPack.data = ReadFile(path, &sPack.size);
    for (i = 0; i < ARRAY_COUNT(sRoms[lang]) && sPack.data == NULL; i++)
    {
        snprintf(rom, sizeof(rom), "%s", Updater_ExeFile(sRoms[lang][i]));
        if (FromRom(rom))
            MPNet_Log("announcer: the voices from %s", sRoms[lang][i]);
    }
    if (sPack.data != NULL && !ReadArchive())
    {
        free(sPack.data);
        sPack.data = NULL;
    }
}

// OPTIONS > BATTLE > ANNOUNCER: ENGLISH or JAPANESE; the other language's voices when only those are there
static bool8 LoadPack(void)
{
    u8 lang = gMpConfig.announcerJp ? 1 : 0;
    if (sPack.tried && sPack.lang == lang)
        return sPack.ok;
    if (sPack.tried)
    {
        if (VoiceClip() >= 0)
            return sPack.ok;   // (the language changes once the line playing ends)
        Unload();
    }
    sPack.tried = TRUE;
    sPack.lang = lang;
    ReadVoices(lang);
    if (sPack.data == NULL)
    {
        ReadVoices(!lang);
        if (sPack.data != NULL)
            MPNet_Log("announcer: no %s voices, the %s ones instead", lang ? "Japanese" : "English", lang ? "English" : "Japanese");
    }
    if (sPack.data == NULL)
    {
        MPNet_Log("announcer: no voices (stadium/announcer.s1 or announcer_jp.s1, or the ROM in stadium/)");
        return FALSE;
    }
    if (sPack.thread != NULL)
    {
        sPack.ok = TRUE;
        MPNet_Log("announcer: %d %s clips ready to decode (%u bytes)", CLIPS, sPack.jp ? "Japanese" : "English", (unsigned)sPack.size);
        return TRUE;
    }
    sPack.lock = SDL_CreateMutex();
    sPack.wake = SDL_CreateCond();
    sPack.thread = SDL_CreateThread(Worker, "moemon_announcer", NULL);
    sPack.ok = sPack.lock != NULL && sPack.wake != NULL && sPack.thread != NULL;
    MPNet_Log("announcer: %d %s clips ready to decode (%u bytes)%s", CLIPS, sPack.jp ? "Japanese" : "English", (unsigned)sPack.size, sPack.ok ? "" : ", no worker");
    if (sPack.ok && getenv("MOEMON_ANNOUNCER_DUMPALL") != NULL)
    {
        u16 c;
        for (c = 0; c < CLIPS; c++)
            Want(c);   // (tests: every clip decoded and dumped, MOEMON_ANNOUNCER_DUMP)
    }
    return sPack.ok;
}

// Asks the worker for a clip (it's decoded by the time it's wanted, as a rule)
static void Want(u16 id)
{
    if (!sPack.ok || id >= CLIPS || SDL_AtomicGet(&sPack.clip[id].state) != 0)
        return;
    SDL_LockMutex(sPack.lock);
    if (sPack.wantCount < MAX_WANT)
    {
        sPack.want[(sPack.wantHead + sPack.wantCount) % MAX_WANT] = id;
        sPack.wantCount++;
        SDL_AtomicSet(&sPack.clip[id].state, 1);
        SDL_CondSignal(sPack.wake);
    }
    SDL_UnlockMutex(sPack.lock);
}

static void WantList(const u16 *list, u8 n)
{
    u8 i;
    for (i = 0; i < n; i++)
        Want(list[i]);
}

// ---------------------------------------------------------------------------------------------------------------
// Playing (one voice; mixed over the game's sound, which is lowered a little while it speaks)

static struct
{
    s16 clip;          // -1: silent
    u32 pos;           // in the clip's samples, 16.16
    float duck;        // the game's sound: 1 normal .. 0.7 under the voice
} sVoice = {-1, 0, 1.0f};

static s16 VoiceClip(void)
{
    return sVoice.clip;
}

void MP_StadiumAnnouncer_Mix(float *buffer, s32 count)
{
    struct Clip *k;
    s32 n;
    const u32 step = (u32)(((u64)RATE << 16) / OUT_RATE);
    float target = sVoice.clip >= 0 ? 0.72f : 1.0f;
    for (n = 0; n + 1 < count; n += 2)
    {
        sVoice.duck += (target - sVoice.duck) * 0.002f;
        buffer[n] *= sVoice.duck;
        buffer[n + 1] *= sVoice.duck;
    }
    if (sVoice.clip < 0)
        return;
    k = &sPack.clip[sVoice.clip];
    for (n = 0; n + 1 < count; n += 2)
    {
        u32 i = sVoice.pos >> 16;
        float a, b, v, f;
        if (i + 1 >= k->samples)
        {
            sVoice.clip = -1;
            break;
        }
        f = (sVoice.pos & 0xFFFF) / 65536.0f;
        a = k->pcm[i], b = k->pcm[i + 1];
        v = (a + (b - a) * f) * k->gain / 32768.0f;
        buffer[n] += v;
        buffer[n + 1] += v;
        sVoice.pos += step;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// The scheduler

static struct
{
    bool8 battle;             // a STADIUM battle is on
    u32 frame;                // frames since the announcer woke (MP_StadiumAnnouncer_Update)
    u32 busyUntil;            // the frame the line playing ends
    u32 lastMsg;              // the frame of the last battle message
    u8 lastCue;               // its kind (CUE_*), CUE_COUNT: none yet
    bool8 lastMashed;         // A was being pressed while it was up
    u16 gap[CUE_COUNT][2];    // frames from a cue to the next message: [kind][0 calm / 1 mashing], 0 = not learned
    u32 aPress[6];            // the frames A was last pressed (mashing: 2 within a second)
    u16 recent[10];
    u8 recentAt;
    // a faint or the win kept for a moment while a line finishes
    const u16 *heldList;
    u8 heldCount, heldPrio, heldCue;
    u32 heldEvent, heldUntil;
    s16 screenClip;           // a screen's line waiting to be decoded (-1: none)
    u32 screenUntil;
    // the turn: the player choosing
    u32 chooseSince;
    bool8 prompted, idleSaid, firstPrompt;
    u32 lastFaint;
    u8 lastFaintSide;
    bool8 countDue;           // a POKeMON fell since the last turn line: the count of who's left
    u16 turnsChosen;
    bool8 winDue;             // the last foe fell: "There goes the battle!" waits for "...was defeated!" and the win song
    bool8 winNow;             // the defeat message is up: said as soon as no line is playing
} A = {.screenClip = -1};

static void ScreenUpdate(void);

static bool8 Enabled(void)
{
    return !gMpConfig.noAnnouncer && sPack.ok;
}

static bool8 Mashing(void)
{
    u8 i, n = 0;
    for (i = 0; i < ARRAY_COUNT(A.aPress); i++)
        if (A.aPress[i] && A.frame - A.aPress[i] < 60)
            n++;
    return n >= 2;
}

// A guess before this battle has taught anything: what a cue usually has at this text speed and animation speed
static u16 Guess(u8 cue, bool8 mashing)
{
    static const u16 sBase[CUE_COUNT] = {
        [CUE_SENDOUT] = 100, [CUE_MOVE] = 115, [CUE_SUPER] = 55, [CUE_NOTVERY] = 55, [CUE_CRIT] = 50, [CUE_MISS] = 60,
        [CUE_FAIL] = 60, [CUE_STATUS] = 65, [CUE_FAINT] = 80, [CUE_RETURN] = 75, [CUE_STAT] = 60, [CUE_STATE] = 60,
        [CUE_OTHER] = 50,
    };
    u32 g = sBase[cue];
    u8 speed = gSaveBlock2Ptr != NULL ? gSaveBlock2Ptr->optionsTextSpeed : OPTIONS_TEXT_SPEED_FAST;
    g = g * (speed == OPTIONS_TEXT_SPEED_SLOW ? 160 : speed == OPTIONS_TEXT_SPEED_MID ? 125 : 100) / 100;
    if ((cue == CUE_MOVE || cue == CUE_SENDOUT) && gMpConfig.animSpeed)
        g = g * 100 / (100 + gMpConfig.animSpeed);
    if (mashing)
        g = g * 60 / 100;
    return (u16)g;
}

// The frames a cue of this kind has before the battle moves on (learned, or the guess)
static u16 Window(u8 cue)
{
    bool8 m = Mashing();
    if (A.gap[cue][m])
        return A.gap[cue][m];
    if (m && A.gap[cue][0])
        return (u16)(A.gap[cue][0] * 60 / 100);
    if (!m && A.gap[cue][1])
        return (u16)(A.gap[cue][1] * 100 / 60);
    return Guess(cue, m);
}

static void Learn(u32 now)
{
    u32 gap;
    u16 *g;
    if (A.lastCue >= CUE_COUNT || A.lastMsg == 0)
        return;
    gap = now - A.lastMsg;
    if (gap < 4 || gap > 900)   // (a menu, the player thinking: not the battle's pace)
        return;
    g = &A.gap[A.lastCue][A.lastMashed];
    *g = *g ? (u16)((*g * 3 + gap) / 4) : (u16)gap;
}

static bool8 Recent(u16 clip)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(A.recent); i++)
        if (A.recent[i] == clip + 1)
            return TRUE;
    return FALSE;
}

static void Start(u16 clip, const char *why)
{
    sVoice.clip = (s16)clip;
    sVoice.pos = 0;
    A.busyUntil = A.frame + sPack.clip[clip].frames;
    A.recent[A.recentAt++ % ARRAY_COUNT(A.recent)] = clip + 1;
    MPNet_Log("announcer: clip %d (%s, %d frames)", clip, why, sPack.clip[clip].frames);
}

static bool8 Busy(void)
{
    return sVoice.clip >= 0 || A.frame < A.busyUntil;
}

// Picks a line from `list` that fits in `window` frames (`over`: may run that many percent past it) and isn't one
// just said; -1: none fits
static s16 Pick(const u16 *list, u8 n, u32 window, u8 over)
{
    u16 fits[16];
    u8 i, count = 0;
    s16 shortest = -1;
    for (i = 0; i < n; i++)
    {
        u16 c = list[i];
        if (c >= CLIPS || SDL_AtomicGet(&sPack.clip[c].state) != 2)
        {
            Want(c);   // (not decoded yet: next time)
            continue;
        }
        if (sPack.clip[c].frames * 100 <= window * (100 + over) && count < ARRAY_COUNT(fits))
        {
            if (!Recent(c) || n == 1)
                fits[count++] = c;
            else if (shortest < 0)
                shortest = (s16)c;   // (only ones said lately fit: one of them rather than nothing)
        }
    }
    if (count)
        return (s16)fits[Random() % count];
    return shortest;
}

// A cue's line: said now if one fits; for a faint or the win, kept a moment while the playing line ends
static void Cue(u8 cue, u8 prio, const u16 *list, u8 n, bool8 strict, const char *why)
{
    u32 window = cue < CUE_COUNT ? Window(cue) : 600;
    s16 clip;
    if (!Enabled() || n == 0)
        return;
    if (prio >= PRIO_RESULT)
        window = window > 330 ? window : 330;   // (the battle is over: the result screen follows)
    if (Busy())
    {
        u32 left = A.busyUntil > A.frame ? A.busyUntil - A.frame : 0;
        if (prio >= PRIO_FAINT && left < window / 2)
        {
            A.heldList = list, A.heldCount = n, A.heldPrio = prio, A.heldCue = cue;
            A.heldEvent = A.frame, A.heldUntil = A.frame + window / 2;
            MPNet_Log("announcer: %s held (%u frames of a line left)", why, (unsigned)left);
        }
        else
            MPNet_Log("announcer: %s dropped (a line is playing)", why);
        return;
    }
    // a faint or the battle's end coming next: never run past the window then
    clip = Pick(list, n, window, strict ? 0 : 15);
    if (clip < 0 && prio >= PRIO_FAINT)
        clip = Pick(list, n, window, 40);   // (what matters most may run a little long)
    if (clip < 0)
    {
        MPNet_Log("announcer: %s skipped (nothing fits %u frames)", why, (unsigned)window);
        return;
    }
    Start((u16)clip, why);
}

static void CueOne(u8 cue, u8 prio, u16 clip, bool8 strict, const char *why)
{
    static u16 sOne[8];
    static u8 sAt;
    u16 *slot = &sOne[sAt++ % ARRAY_COUNT(sOne)];
    *slot = clip;
    Cue(cue, prio, slot, 1, strict, why);
}

// ---------------------------------------------------------------------------------------------------------------
// The battle

static struct Pokemon *Party(u8 side)
{
    return side == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty;
}

static u8 Left(u8 side)
{
    struct Pokemon *p = Party(side);
    u8 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 s = GetMonData(&p[i], MON_DATA_SPECIES);
        if (s != SPECIES_NONE && s != SPECIES_EGG && GetMonData(&p[i], MON_DATA_HP) > 0)
            n++;
    }
    return n;
}

static u16 Dex(u16 species)
{
    u16 d = SpeciesToNationalPokedexNum(species);
    return d >= 1 && d <= 151 ? d : 0;
}

// Every clip this battle may want, asked of the worker at its start (the two teams' send-outs and moves first)
static void WantBattle(void)
{
    u8 side, i, m;
    for (side = 0; side < 2; side++)
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *p = &Party(side)[i];
            u16 d = Dex(GetMonData(p, MON_DATA_SPECIES));
            if (d)
                Want(CLIP_SENDOUT(d));
            for (m = 0; m < MAX_MON_MOVES; m++)
            {
                u16 move = GetMonData(p, MON_DATA_MOVE1 + m);
                if (move >= 1 && move <= 165)
                    Want(CLIP_MOVE(move));
            }
        }
    WantList(LIST(sSuper)); WantList(LIST(sCrit)); WantList(LIST(sFaint)); WantList(LIST(sMiss));
    WantList(LIST(sNotVery)); WantList(LIST(sFail)); WantList(LIST(sWon)); WantList(LIST(sLastOne));
    WantList(LIST(sBothLast)); WantList(LIST(sDoubleFaint)); WantList(LIST(sSwitchPlayer)); WantList(LIST(sSwitchFoe));
    WantList(LIST(sSleep)); WantList(LIST(sFreeze)); WantList(LIST(sPara)); WantList(LIST(sConfuse));
    WantList(LIST(sPoison)); WantList(LIST(sBurn)); WantList(LIST(sFirstMove)); WantList(LIST(sTurnPrompt));
    WantList(LIST(sIdle)); WantList(LIST(sHot));
    {
        u8 i;
        for (i = 1; i < 8; i++)
            Want(sStatDown[i]), Want(sStatUp[i]);
    }
    WantList(LIST(sWokeUp)); WantList(LIST(sThawed)); WantList(LIST(sFastAsleep)); WantList(LIST(sIsFrozen));
    WantList(LIST(sCantMove)); WantList(LIST(sHurtItself)); WantList(LIST(sNoMoreConfused)); WantList(LIST(sIsConfused));
    WantList(LIST(sHealed)); WantList(LIST(sSubMade)); WantList(LIST(sSubGone)); WantList(LIST(sSubHit));
    WantList(LIST(sTransform)); WantList(LIST(sFly)); WantList(LIST(sDig)); WantList(LIST(sCharge));
    WantList(LIST(sPoisonHurt)); WantList(LIST(sBurnHurt)); WantList(LIST(sNothing)); WantList(LIST(sHaze));
    WantList(LIST(sBothFit)); WantList(LIST(sBothSpent)); WantList(LIST(sHpGap)); WantList(LIST(sCount32));
    WantList(LIST(sCount31)); WantList(LIST(sCount22)); WantList(LIST(sCount23)); WantList(LIST(sCount12));
    WantList(LIST(sCount11)); WantList(LIST(sReadyToDrop)); WantList(LIST(sHurtAlready)); WantList(LIST(sAvenge));
}

void MP_StadiumAnnouncer_BattleStart(void)
{
    u8 c;
    if (gMpConfig.noAnnouncer || !LoadPack())
        return;
    for (c = 0; c < CUE_COUNT; c++)
        A.gap[c][0] = A.gap[c][1] = 0;   // (each battle teaches its own pace: the moves and animations differ)
    A.battle = TRUE;
    A.lastCue = CUE_COUNT;
    A.lastMsg = 0;
    A.heldList = NULL;
    A.heldCount = 0;
    A.chooseSince = 0;
    A.prompted = A.idleSaid = FALSE;
    A.firstPrompt = TRUE;
    A.turnsChosen = 0;
    A.lastFaint = 0;
    A.winDue = A.winNow = FALSE;
    WantBattle();
    MPNet_Log("announcer: battle on (text speed %d, anim speed %d)", gSaveBlock2Ptr->optionsTextSpeed, gMpConfig.animSpeed);
}

void MP_StadiumAnnouncer_BattleEnd(void)
{
    A.battle = FALSE;   // (a line playing ends on its own: never cut off)
    A.heldList = NULL;
    A.heldCount = 0;
}

// The stat a "rose" / "fell" message names (its text buffer: B_BUFF_STAT)
static u8 MsgStat(void)
{
    if (gBattleTextBuff1[0] == B_BUFF_PLACEHOLDER_BEGIN && gBattleTextBuff1[1] == B_BUFF_STAT && gBattleTextBuff1[2] < 8)
        return gBattleTextBuff1[2];
    return gBattleScripting.statChanger & 0xF;
}

static void Status(u16 id)
{
    u8 stat;
    switch (id)
    {
    case STRINGID_ATTACKERSSTATROSE: case STRINGID_DEFENDERSSTATROSE:
        stat = MsgStat();
        if (stat >= 1 && stat < 8)
            CueOne(CUE_STAT, PRIO_REACT, sStatUp[stat], FALSE, "a stat up");
        return;
    case STRINGID_ATTACKERSSTATFELL: case STRINGID_DEFENDERSSTATFELL:
        stat = MsgStat();
        if (stat >= 1 && stat < 8)
            CueOne(CUE_STAT, PRIO_REACT, sStatDown[stat], FALSE, "a stat down");
        return;
    case STRINGID_PKMNWOKEUP: Cue(CUE_STATE, PRIO_REACT, LIST(sWokeUp), FALSE, "woke up"); return;
    case STRINGID_PKMNWASDEFROSTED: Cue(CUE_STATE, PRIO_REACT, LIST(sThawed), FALSE, "thawed"); return;
    case STRINGID_PKMNFASTASLEEP: Cue(CUE_STATE, PRIO_REACT, LIST(sFastAsleep), FALSE, "fast asleep"); return;
    case STRINGID_PKMNISFROZEN: Cue(CUE_STATE, PRIO_REACT, LIST(sIsFrozen), FALSE, "frozen solid"); return;
    case STRINGID_PKMNISPARALYZED: Cue(CUE_STATE, PRIO_REACT, LIST(sCantMove), FALSE, "can't move"); return;
    case STRINGID_ITHURTCONFUSION: Cue(CUE_STATE, PRIO_REACT, LIST(sHurtItself), FALSE, "hurt itself"); return;
    case STRINGID_PKMNHEALEDCONFUSION: Cue(CUE_STATE, PRIO_REACT, LIST(sNoMoreConfused), FALSE, "snapped out of it"); return;
    case STRINGID_PKMNISCONFUSED: Cue(CUE_STATE, PRIO_REACT, LIST(sIsConfused), FALSE, "confused"); return;
    case STRINGID_PKMNREGAINEDHEALTH: Cue(CUE_STATE, PRIO_REACT, LIST(sHealed), FALSE, "healed"); return;
    case STRINGID_PKMNMADESUBSTITUTE: Cue(CUE_STATE, PRIO_REACT, LIST(sSubMade), FALSE, "a substitute"); return;
    case STRINGID_PKMNSUBSTITUTEFADED: Cue(CUE_STATE, PRIO_REACT, LIST(sSubGone), FALSE, "the substitute gone"); return;
    case STRINGID_SUBSTITUTEDAMAGED: Cue(CUE_STATE, PRIO_REACT, LIST(sSubHit), FALSE, "the substitute hit"); return;
    case STRINGID_PKMNTRANSFORMEDINTO: Cue(CUE_STATE, PRIO_REACT, LIST(sTransform), FALSE, "transformed"); return;
    case STRINGID_PKMNFLEWHIGH: Cue(CUE_STATE, PRIO_REACT, LIST(sFly), FALSE, "flew up"); return;
    case STRINGID_PKMNDUGHOLE: Cue(CUE_STATE, PRIO_REACT, LIST(sDig), FALSE, "dug down"); return;
    case STRINGID_PKMNTOOKSUNLIGHT: Cue(CUE_STATE, PRIO_REACT, LIST(sCharge), FALSE, "charging"); return;
    case STRINGID_PKMNHURTBYPOISON: Cue(CUE_STATE, PRIO_REACT, LIST(sPoisonHurt), FALSE, "hurt by poison"); return;
    case STRINGID_PKMNHURTBYBURN: Cue(CUE_STATE, PRIO_REACT, LIST(sBurnHurt), FALSE, "hurt by its burn"); return;
    case STRINGID_BUTNOTHINGHAPPENED: Cue(CUE_STATE, PRIO_REACT, LIST(sNothing), FALSE, "nothing happened"); return;
    case STRINGID_STATCHANGESGONE: Cue(CUE_STATE, PRIO_REACT, LIST(sHaze), FALSE, "stats back to normal"); return;
    case STRINGID_PKMNFELLASLEEP: Cue(CUE_STATUS, PRIO_REACT, LIST(sSleep), FALSE, "asleep"); break;
    case STRINGID_PKMNWASFROZEN: case STRINGID_PKMNFROZENBY: Cue(CUE_STATUS, PRIO_REACT, LIST(sFreeze), FALSE, "frozen"); break;
    case STRINGID_PKMNWASPARALYZED: case STRINGID_PKMNWASPARALYZEDBY: Cue(CUE_STATUS, PRIO_REACT, LIST(sPara), FALSE, "paralyzed"); break;
    case STRINGID_PKMNWASCONFUSED: Cue(CUE_STATUS, PRIO_REACT, LIST(sConfuse), FALSE, "confused"); break;
    case STRINGID_PKMNWASPOISONED: case STRINGID_PKMNPOISONEDBY: case STRINGID_PKMNBADLYPOISONED:
        Cue(CUE_STATUS, PRIO_REACT, LIST(sPoison), FALSE, "poisoned"); break;
    case STRINGID_PKMNWASBURNED: case STRINGID_PKMNBURNEDBY: Cue(CUE_STATUS, PRIO_REACT, LIST(sBurn), FALSE, "burned"); break;
    }
}

static u8 CueOf(u16 id)
{
    switch (id)
    {
    case STRINGID_INTROSENDOUT: case STRINGID_SWITCHINMON: return CUE_SENDOUT;
    case STRINGID_USEDMOVE: return CUE_MOVE;
    case STRINGID_SUPEREFFECTIVE: return CUE_SUPER;
    case STRINGID_NOTVERYEFFECTIVE: return CUE_NOTVERY;
    case STRINGID_CRITICALHIT: return CUE_CRIT;
    case STRINGID_ATTACKMISSED: case STRINGID_AVOIDEDDAMAGE: return CUE_MISS;
    case STRINGID_BUTITFAILED: case STRINGID_ITDOESNTAFFECT: return CUE_FAIL;
    case STRINGID_PKMNFELLASLEEP: case STRINGID_PKMNWASFROZEN: case STRINGID_PKMNFROZENBY: case STRINGID_PKMNWASPARALYZED:
    case STRINGID_PKMNWASPARALYZEDBY: case STRINGID_PKMNWASCONFUSED: case STRINGID_PKMNWASPOISONED:
    case STRINGID_PKMNPOISONEDBY: case STRINGID_PKMNBADLYPOISONED: case STRINGID_PKMNWASBURNED: case STRINGID_PKMNBURNEDBY:
        return CUE_STATUS;
    case STRINGID_ATTACKERFAINTED: case STRINGID_TARGETFAINTED: return CUE_FAINT;
    case STRINGID_RETURNMON: return CUE_RETURN;
    case STRINGID_ATTACKERSSTATROSE: case STRINGID_DEFENDERSSTATROSE: case STRINGID_ATTACKERSSTATFELL:
    case STRINGID_DEFENDERSSTATFELL: return CUE_STAT;
    case STRINGID_PKMNWOKEUP: case STRINGID_PKMNWASDEFROSTED: case STRINGID_PKMNFASTASLEEP: case STRINGID_PKMNISFROZEN:
    case STRINGID_PKMNISPARALYZED: case STRINGID_ITHURTCONFUSION: case STRINGID_PKMNHEALEDCONFUSION:
    case STRINGID_PKMNISCONFUSED: case STRINGID_PKMNREGAINEDHEALTH: case STRINGID_PKMNMADESUBSTITUTE:
    case STRINGID_PKMNSUBSTITUTEFADED: case STRINGID_SUBSTITUTEDAMAGED: case STRINGID_PKMNTRANSFORMEDINTO:
    case STRINGID_PKMNFLEWHIGH: case STRINGID_PKMNDUGHOLE: case STRINGID_PKMNTOOKSUNLIGHT: case STRINGID_PKMNHURTBYPOISON:
    case STRINGID_PKMNHURTBYBURN: case STRINGID_BUTNOTHINGHAPPENED: case STRINGID_STATCHANGESGONE:
        return CUE_STATE;
    }
    return CUE_OTHER;
}

// A faint: whose, then the faint line, both down, the last one, or the win
static void Faint(u16 id)
{
    u8 battler = id == STRINGID_ATTACKERFAINTED ? gBattlerAttacker : gBattlerTarget;
    u8 side = GetBattlerSide(battler), other = side ^ 1;
    bool8 both = A.lastFaint && A.frame - A.lastFaint < 150 && A.lastFaintSide != side;
    A.lastFaint = A.frame;
    A.lastFaintSide = side;
    A.countDue = TRUE;
    if (Left(side) == 0 && side == B_SIDE_OPPONENT && Left(other) > 0)
    {
        // (the user, 2026-11-07: the win line with "STADIUM X was defeated!" and the victory song, not at the faint)
        A.winDue = TRUE;
        Want(sWon[0]);
        MPNet_Log("announcer: the win waits for the defeat message");
    }
    else if (both)
        Cue(CUE_FAINT, PRIO_FAINT, LIST(sDoubleFaint), TRUE, "both down");
    else if (Left(side) == 1 && Left(other) == 1)
        Cue(CUE_FAINT, PRIO_FAINT, LIST(sBothLast), TRUE, "both down to the last");
    else if (Left(side) == 1)
        Cue(CUE_FAINT, PRIO_FAINT, LIST(sLastOne), TRUE, "the last one");
    else
        Cue(CUE_FAINT, PRIO_FAINT, LIST(sFaint), TRUE, "faint");
}

// Every battle message in a STADIUM battle (battle_message.c BufferStringBattle)
void MP_Announcer_OnBattleString(u16 id)
{
    u8 cue;
    if (!A.battle || !gMpStadiumBattle)
        return;
    cue = CueOf(id);
    Learn(A.frame);
    A.lastMsg = A.frame;
    A.lastCue = cue;
    A.lastMashed = Mashing();
    A.chooseSince = 0;
    if (!Enabled())
        return;
    switch (id)
    {
    case STRINGID_INTROSENDOUT:
    case STRINGID_SWITCHINMON:
    {
        u8 battler = id == STRINGID_SWITCHINMON ? gBattleScripting.battler : gActiveBattler;
        u8 side = GetBattlerSide(battler);
        struct Pokemon *mon = &Party(side)[gBattlerPartyIndexes[battler]];
        u16 d = Dex(GetMonData(mon, MON_DATA_SPECIES));
        u16 hp = GetMonData(mon, MON_DATA_HP), maxHp = GetMonData(mon, MON_DATA_MAX_HP);
        bool8 replacing = A.lastFaint && A.frame - A.lastFaint < 600 && A.lastFaintSide == side;
        if (maxHp && hp * 4 <= maxHp)
            Cue(CUE_SENDOUT, PRIO_SENDOUT, LIST(sReadyToDrop), FALSE, "coming out nearly down");
        else if (maxHp && hp * 2 <= maxHp && (Random() & 1))
            Cue(CUE_SENDOUT, PRIO_SENDOUT, LIST(sHurtAlready), FALSE, "coming out hurt");
        else if (replacing && (Random() % 3) == 0)
            Cue(CUE_SENDOUT, PRIO_SENDOUT, LIST(sAvenge), FALSE, "after one fell");
        else if (d)
            CueOne(CUE_SENDOUT, PRIO_SENDOUT, CLIP_SENDOUT(d), FALSE, "send-out");
        break;
    }
    case STRINGID_USEDMOVE:
    {
        u16 move = gBattleMsgDataPtr->currentMove;
        if (move >= 1 && move <= 165)
            CueOne(CUE_MOVE, PRIO_MOVE, CLIP_MOVE(move), FALSE, "the move");
        break;
    }
    case STRINGID_SUPEREFFECTIVE:
        // the hit already took the HP: a faint next means no running past the window
        Cue(CUE_SUPER, PRIO_REACT, LIST(sSuper), gBattleMons[gBattlerTarget].hp == 0, "super effective");
        break;
    case STRINGID_NOTVERYEFFECTIVE:
        Cue(CUE_NOTVERY, PRIO_REACT, LIST(sNotVery), gBattleMons[gBattlerTarget].hp == 0, "not very effective");
        break;
    case STRINGID_CRITICALHIT:
        Cue(CUE_CRIT, PRIO_REACT, LIST(sCrit), gBattleMons[gBattlerTarget].hp == 0, "critical hit");
        break;
    case STRINGID_ATTACKMISSED:
    case STRINGID_AVOIDEDDAMAGE:
        Cue(CUE_MISS, PRIO_REACT, LIST(sMiss), FALSE, "miss");
        break;
    case STRINGID_BUTITFAILED:
    case STRINGID_ITDOESNTAFFECT:
        Cue(CUE_FAIL, PRIO_REACT, LIST(sFail), FALSE, "failed");
        break;
    case STRINGID_ATTACKERFAINTED:
    case STRINGID_TARGETFAINTED:
        Faint(id);
        break;
    case STRINGID_PLAYERDEFEATEDTRAINER1:
    case STRINGID_TWOENEMIESDEFEATED:
        if (A.winDue)
        {
            A.winDue = FALSE;
            A.winNow = TRUE;   // (MP_StadiumAnnouncer_Update: once any line playing ends; never one over another)
        }
        break;
    case STRINGID_RETURNMON:
        if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
            Cue(CUE_RETURN, PRIO_REACT, LIST(sSwitchPlayer), FALSE, "switch");
        else
            Cue(CUE_RETURN, PRIO_REACT, LIST(sSwitchFoe), FALSE, "the foe switches");
        break;
    default:
        Status(id);
        break;
    }
}

// Every frame: A presses, a held line said if it still fits, the player thinking at the menu
void MP_StadiumAnnouncer_Update(void)
{
    A.frame++;
    if (!sPack.ok)
        return;
    if (sVoice.clip < 0 && A.busyUntil > A.frame + 2)
        A.busyUntil = A.frame;   // (the mixer finished it a little early: free)
    ScreenUpdate();
    // (the win line keeps its place even when the battle's screens close before the playing line ends)
    if (A.winNow && !Busy())
    {
        if (SDL_AtomicGet(&sPack.clip[sWon[0]].state) == 2)
        {
            A.winNow = FALSE;
            Start(sWon[0], "the win");
        }
        else
            Want(sWon[0]);
    }
    if (!A.battle)
        return;
    if (gMain.newKeys & A_BUTTON)
    {
        memmove(&A.aPress[1], &A.aPress[0], sizeof(A.aPress) - sizeof(A.aPress[0]));
        A.aPress[0] = A.frame;
    }
    if (A.heldCount && !Busy())
    {
        // the time the held cue had, less what went by waiting
        u32 window = Window(A.heldCue), gone = A.frame - A.heldEvent;
        s16 clip;
        if (A.heldPrio >= PRIO_RESULT && window < 330)
            window = 330;
        clip = A.frame <= A.heldUntil && window > gone ? Pick(A.heldList, A.heldCount, window - gone, 10) : -1;
        if (clip >= 0)
            Start((u16)clip, "held");
        else
            MPNet_Log("announcer: the held line didn't fit any more");
        A.heldCount = 0;
        A.heldList = NULL;
    }
    if (!Enabled() || !MP_BattlePlayer_Choosing())
    {
        A.chooseSince = 0;
        return;
    }
    // the player choosing: once they've thought for a while (a line said then may run on into the turn, so only
    // after they've clearly stopped to think), "What will the trainer do now?"; much longer, "What's the matter?"
    if (A.chooseSince == 0)
    {
        A.chooseSince = A.frame;
        A.prompted = A.idleSaid = FALSE;
        A.turnsChosen++;
    }
    if (!A.prompted && A.frame - A.chooseSince >= 150 && !Busy())
    {
        u8 b0 = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), b1 = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        bool8 hot = A.turnsChosen > 3 && gBattleMons[b0].hp * 2 <= gBattleMons[b0].maxHP
                 && gBattleMons[b1].hp * 2 <= gBattleMons[b1].maxHP;
        u32 h0 = gBattleMons[b0].maxHP ? gBattleMons[b0].hp * 100 / gBattleMons[b0].maxHP : 0;
        u32 h1 = gBattleMons[b1].maxHP ? gBattleMons[b1].hp * 100 / gBattleMons[b1].maxHP : 0;
        u8 mine = Left(B_SIDE_PLAYER), theirs = Left(B_SIDE_OPPONENT);
        A.prompted = TRUE;
        if (A.firstPrompt)
            Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sFirstMove), FALSE, "the first move");
        else if (A.countDue && mine == 3 && theirs == 2) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sCount32), FALSE, "the count");
        else if (A.countDue && mine == 3 && theirs == 1) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sCount31), FALSE, "the count");
        else if (A.countDue && mine == 2 && theirs == 2) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sCount22), FALSE, "the count");
        else if (A.countDue && mine == 2 && theirs == 3) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sCount23), FALSE, "the count");
        else if (A.countDue && mine == 1 && theirs == 2) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sCount12), FALSE, "the count");
        else if (A.countDue && mine == 1 && theirs == 1) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sCount11), FALSE, "the count");
        else if (A.turnsChosen > 2 && h0 <= 25 && h1 <= 25) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sBothSpent), FALSE, "both worn out");
        else if (A.turnsChosen > 1 && (h0 > h1 + 60 || h1 > h0 + 60)) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sHpGap), FALSE, "far apart");
        else if (A.turnsChosen > 2 && h0 >= 90 && h1 >= 90 && (Random() & 1)) Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sBothFit), FALSE, "both fit");
        else if (hot && (Random() & 1))
            Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sHot), FALSE, "a heated battle");
        else
            Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sTurnPrompt), FALSE, "the turn");
        A.firstPrompt = FALSE;
        A.countDue = FALSE;
    }
    if (!A.idleSaid && A.frame - A.chooseSince >= 900 && !Busy())
    {
        A.idleSaid = TRUE;
        Cue(CUE_COUNT, PRIO_AMBIENT, LIST(sIdle), FALSE, "the trainer thinking");
    }
}

// ---------------------------------------------------------------------------------------------------------------
// The STADIUM's screens (mp_stadium_ui.c): the castle's introductions, a stage cleared, the champion

void MP_StadiumAnnouncer_Screen(u16 clip, const char *why)
{
    if (gMpConfig.noAnnouncer || !LoadPack() || clip >= CLIPS)
        return;
    Want(clip);
    if (Busy())
    {
        MPNet_Log("announcer: %s dropped (a line is playing)", why);
        return;
    }
    // (a screen waits for the player: the line is said once it's decoded, a few frames from now)
    A.screenClip = (s16)clip;
    A.screenUntil = A.frame + 30;
}

static void ScreenUpdate(void)
{
    u8 state;
    if (A.screenClip < 0)
        return;
    state = (u8)SDL_AtomicGet(&sPack.clip[A.screenClip].state);
    if (state == 2 && !Busy())
    {
        Start((u16)A.screenClip, "a screen");
        A.screenClip = -1;
    }
    else if (A.frame > A.screenUntil || state == 3)
        A.screenClip = -1;
}

// Tests (mp announcer): what was learned
void MP_StadiumAnnouncer_Debug(void)
{
    u8 c;
    for (c = 0; c < CUE_COUNT; c++)
        MPNet_Log("announcer: cue %d window %d (learned %d / %d mashing)", c, Window(c), A.gap[c][0], A.gap[c][1]);
}
