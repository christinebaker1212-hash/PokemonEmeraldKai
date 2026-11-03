// Moemon Co-op: the phone app's sounds, recorded from the game's own sound engine (docs/PHONE_LINK.md "The app's
// sounds"). Part of MOEMON_EXPORT_APP (mp_export.c): every sound effect the app uses, every POKéMON's cry, the
// fanfares and the card games' duel songs (each scene's own: custom .kaisong music too), played by the game exactly
// as it plays them and written to DIR/sounds.bin:
//   "MSND" u32 version 1, u32 count, then count entries:
//     u8 nameLength, name (ASCII: se_select, cry_25, mus_obtain_item, tt:mus_rg_vs_trainer...), u8 kind
//     (0 sound effect, 1 cry, 2 fanfare, 3 song), u8 channels (1 mono, 2 stereo), u16 id (the song's or species'
//     number), u32 rate (Hz), u32 frames, u32 loop (the frame the song loops back to; 0xFFFFFFFF: no loop), u32 loopEnd
//     (where it loops back from; the frames past it are the song going on, about 50 ms, for a crossfade into the
//     loop's start so the seam doesn't click), u32 offset (from the file's start), u32 size. Sounds with the same data
//     share it.
//   then the sounds' data: IMA ADPCM, per channel an s16 start value, a u8 step index and a zero byte, then a nibble
//   a sample (low nibble first; stereo: left, right, left, right...).
// The game's mixer runs at 42048 Hz; sounds are filtered and taken down to a rate that keeps what they hold (cries
// 10512 Hz, their own rate; the rest 21024 Hz). Leading silence is cut (the mixer's buffers), and the tail ends when
// the sound has died away.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "m4a.h"
#include "sound.h"
#include "music_player.h"
#include "mp_kaisong.h"
#include "mp_kaisong_scene.h"
#include "constants/songs.h"
#include "constants/sound.h"
#include "constants/species.h"

void (*gMpAudioCapture)(const float *buffer, s32 bytes);   // sdl2.c Platform_QueueAudio
extern void (*gMpGotoWatch)(struct MP2KTrack *track);        // music_player.c MP2K_event_goto
const char *MP_Kaisong_SongFileName(u16 song);

enum { KIND_SE, KIND_CRY, KIND_FANFARE, KIND_SONG };

// The sound effects the app plays: the card games' tables (their boards' own), the PC, the POKéNAV, the BAG...
static const u16 sEffects[] = {
    SE_SELECT, SE_FAILURE, SE_SUCCESS, SE_WIN_OPEN, SE_USE_ITEM, SE_PIN, SE_BALL, SE_BALL_OPEN, SE_EXP, SE_EXP_MAX,
    SE_PC_ON, SE_PC_OFF, SE_PC_LOGIN, SE_POKENAV_ON, SE_POKENAV_OFF, SE_POKENAV_CALL, SE_DEX_SCROLL, SE_DEX_PAGE,
    SE_DING_DONG, SE_CLICK, SE_SWITCH, SE_BALL_TRADE, SE_EGG_HATCH, SE_SHOP, SE_RG_SHOP, SE_RG_BAG_CURSOR,
    SE_RG_BAG_POCKET, SE_RG_HELP_OPEN, SE_RG_HELP_CLOSE, SE_RG_BALL_CLICK, SE_RG_CARD_FLIP, SE_RG_CARD_FLIPPING,
    SE_RG_CARD_OPEN, SE_SHINY, SE_FAINT, SE_BANG, SE_EFFECTIVE, SE_SUPER_EFFECTIVE, SE_NOT_EFFECTIVE, SE_FIELD_POISON,
    SE_BALL_BOUNCE_1, SE_BALL_TRAY_BALL, SE_BALL_TRAY_ENTER, SE_BALL_TRAY_EXIT,
    SE_M_ACID_ARMOR, SE_M_BARRIER, SE_M_BIND, SE_M_BLIZZARD, SE_M_BONEMERANG, SE_M_BUBBLE_BEAM, SE_M_COMET_PUNCH,
    SE_M_CONFUSE_RAY, SE_M_COSMIC_POWER, SE_M_CUT, SE_M_DETECT, SE_M_DOUBLE_TEAM, SE_M_DRAGON_RAGE, SE_M_EXPLOSION,
    SE_M_FIRE_PUNCH, SE_M_FLAMETHROWER, SE_M_FLAME_WHEEL, SE_M_GIGA_DRAIN, SE_M_GUST, SE_M_HARDEN, SE_M_HAZE,
    SE_M_HEAL_BELL, SE_M_HORN_ATTACK, SE_M_HYDRO_PUMP, SE_M_HYPER_BEAM, SE_M_ICY_WIND, SE_M_LEER, SE_M_LOCK_ON,
    SE_M_MEGA_KICK, SE_M_MEGA_KICK2, SE_M_MINIMIZE, SE_M_MORNING_SUN, SE_M_NIGHTMARE, SE_M_PAY_DAY, SE_M_PETAL_DANCE,
    SE_M_POISON_POWDER, SE_M_PSYBEAM, SE_M_PSYBEAM2, SE_M_RAZOR_WIND, SE_M_REFLECT, SE_M_ROCK_THROW, SE_M_SING,
    SE_M_SKETCH, SE_M_STAT_DECREASE, SE_M_STAT_INCREASE, SE_M_STRENGTH, SE_M_STRING_SHOT, SE_M_SUPERSONIC,
    SE_M_SWORDS_DANCE, SE_M_TAKE_DOWN, SE_M_TELEPORT, SE_M_THUNDERBOLT, SE_M_THUNDER_WAVE, SE_M_TOXIC, SE_M_UPROAR,
    SE_M_VITAL_THROW, SE_M_WATERFALL, SE_M_WHIRLPOOL,
};

static const u16 sFanfares[] = {
    MUS_OBTAIN_ITEM, MUS_LEVEL_UP, MUS_EVOLVED, MUS_REGISTER_MATCH_CALL, MUS_OBTAIN_B_POINTS, MUS_SLOTS_WIN,
    MUS_TOO_BAD, MUS_HEAL,
};

// The card games' duel songs, each as its scene plays it (a source file of the scene: mp_kaisong.c SceneOfFile):
// the game's duels against card players and against friends (tt_ui.c / tcg_ui.c / ygo_ui.c), and the win
static const struct
{
    const char *scene, *file;
    u16 song;
} sSongs[] = {
    {"tt", "src/tt/tt_board.c", MUS_RG_VS_TRAINER},
    {"tt", "src/tt/tt_board.c", MUS_VICTORY_TRAINER},
    {"tcg", "src/tcg/tcg_board.c", MUS_VS_TRAINER},
    {"tcg", "src/tcg/tcg_board.c", MUS_RG_UNION_ROOM},
    {"tcg", "src/tcg/tcg_board.c", MUS_VICTORY_TRAINER},
    {"ygo", "src/ygo/ygo_board.c", MUS_VS_TRAINER},
    {"ygo", "src/ygo/ygo_board.c", MUS_RG_UNION_ROOM},
    {"ygo", "src/ygo/ygo_board.c", MUS_VICTORY_TRAINER},
};

#define MIXER_RATE 42048
#define PI_F 3.14159265f
#define MAX_FRAMES (60 * 60 * 5)   // (5 minutes: a song that never loops back)

// ------------------------------------------------------------------------------------------------- recording
static struct
{
    float *buf;            // stereo float pairs at 42048 Hz
    u32 len, cap;          // in pairs
    u32 frame;             // game frames recorded
    u32 frameStart[MAX_FRAMES + 1];   // where each frame's audio starts
    u32 loopFrame, endFrame;
    u8 *loopTo;
    uintptr_t trackPos[MAX_FRAMES + 1];   // the first track's place in its song after each frame (patterns: where they return)
    struct MP2KTrack *watched;
} R;

static void Capture(const float *buffer, s32 bytes)
{
    u32 pairs = (u32)bytes / (2 * sizeof(float));
    if (R.len + pairs > R.cap)
    {
        R.cap = (R.len + pairs) * 2 + MIXER_RATE;
        R.buf = realloc(R.buf, R.cap * 2 * sizeof(float));
    }
    memcpy(R.buf + R.len * 2, buffer, pairs * 2 * sizeof(float));
    R.len += pairs;
}

static void GotoWatch(struct MP2KTrack *track)
{
    if (track == R.watched && R.endFrame == 0)
    {
        R.endFrame = R.frame;
        R.loopTo = track->cmdPtr;
    }
}

static void Frame(void)
{
    if (R.frame < MAX_FRAMES)
        R.frameStart[R.frame] = R.len;
    R.frame++;
    m4aSoundMain();
    m4aSoundVSync();
    if (R.watched != NULL && R.frame <= MAX_FRAMES)
        R.trackPos[R.frame] = (uintptr_t)(R.watched->patternLevel ? R.watched->patternStack[0] : R.watched->cmdPtr);
}

static float Peak(u32 from, u32 to)
{
    float p = 0;
    u32 i;
    for (i = from * 2; i < to * 2 && i < R.len * 2; i++)
        if (fabsf(R.buf[i]) > p)
            p = fabsf(R.buf[i]);
    return p;
}

#define QUIET 0.0015f

// Everything stopped and the reverb gone: the next sound starts from silence
static void Silence(void)
{
    int n;
    m4aMPlayAllStop();
    StopCry();
    for (n = 0; n < 240; n++)
    {
        u32 at = R.len;
        Frame();
        if (n > 4 && Peak(at, R.len) < 0.00002f)
            break;
    }
    R.len = 0;
    R.frame = 0;
    R.endFrame = 0;
    R.loopTo = NULL;
    R.watched = NULL;
}

static struct MusicPlayerInfo *sSongPlayer;   // the player the song plays on (fanfares: one of their own)

static struct MusicPlayerInfo *PlayerOf(u16 song)
{
    return gMPlayTable[gSongTable[MP_Kaisong_TableIndex(song)].ms].info;
}

static bool8 SongPlaying(void)
{
    return (sSongPlayer->status & MUSICPLAYER_STATUS_TRACK) != 0 && !(sSongPlayer->status & MUSICPLAYER_STATUS_PAUSE);
}

// Frames until the sound's player has stopped (or `most` frames), then until it has died away (no more than 3 s)
static void PlayOut(bool8 (*playing)(void), u32 most)
{
    u32 quiet = 0, tail = 0;
    while (R.frame < MAX_FRAMES)
    {
        u32 at = R.len;
        Frame();
        if ((R.frame < 4 || playing()) && R.frame < most)
            continue;
        if (R.frame == most)
        {
            m4aMPlayAllStop();
            StopCry();
        }
        quiet = Peak(at, R.len) < QUIET ? quiet + 1 : 0;
        if (quiet >= 6 || ++tail > 180)
            break;
    }
}

// --------------------------------------------------------------------------------------- the pack's sounds
struct Sound
{
    char name[48];
    u8 kind, channels;
    u16 id;
    u32 rate, frames, loop, loopEnd, offset;
    u8 *data;
    u32 size;
    int alias;             // the same sound as this earlier one (-1: its own)
    const void *header;    // (songs: the music the game picked)
};

static struct Sound *sSounds;
static u32 sCount, sSoundCap;

// IMA ADPCM
static const s16 sSteps[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107,
    118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
    1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894,
    6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
    32767};
static const s8 sIndexMove[8] = {-1, -1, -1, -1, 2, 4, 6, 8};

struct Adpcm
{
    int pred, index;
};

static u8 AdpcmNibble(struct Adpcm *a, int sample)
{
    int step = sSteps[a->index], diff = sample - a->pred, code = 0, delta = step >> 3;
    if (diff < 0)
    {
        code = 8;
        diff = -diff;
    }
    if (diff >= step)
    {
        code |= 4;
        diff -= step;
        delta += step;
    }
    if (diff >= step >> 1)
    {
        code |= 2;
        diff -= step >> 1;
        delta += step >> 1;
    }
    if (diff >= step >> 2)
    {
        code |= 1;
        delta += step >> 2;
    }
    a->pred += code & 8 ? -delta : delta;
    if (a->pred > 32767)
        a->pred = 32767;
    if (a->pred < -32768)
        a->pred = -32768;
    a->index += sIndexMove[code & 7];
    if (a->index < 0)
        a->index = 0;
    if (a->index > 88)
        a->index = 88;
    return (u8)code;
}

// A start index that fits the first change, so the sound's attack isn't softened while the coder catches up
static int StartIndex(int first, int second)
{
    int d = abs(second - first), i = 0;
    while (i < 88 && sSteps[i] < d)
        i++;
    return i;
}

// The recording [from, to) in pairs, filtered and taken down by `down`, as samples (s16) per channel
static s16 *Downsample(u32 from, u32 to, int down, int channels, u32 *outFrames)
{
    enum { TAPS = 31 };
    float h[TAPS], sum = 0;
    u32 n = (to - from) / down, i;
    s16 *out = malloc((size_t)(n ? n : 1) * channels * sizeof(s16));
    int k, c;
    // a windowed sinc at 90% of the new rate's half
    for (k = 0; k < TAPS; k++)
    {
        float t = k - (TAPS - 1) / 2.0f, x = 0.9f * t / down;
        float w = 0.42f - 0.5f * cosf(2 * PI_F * k / (TAPS - 1)) + 0.08f * cosf(4 * PI_F * k / (TAPS - 1));
        h[k] = (t == 0 ? 1.0f : sinf(PI_F * x) / (PI_F * x)) * w;
        sum += h[k];
    }
    for (k = 0; k < TAPS; k++)
        h[k] /= sum;
    for (i = 0; i < n; i++)
    {
        long center = (long)from + (long)i * down;
        for (c = 0; c < channels; c++)
        {
            float acc = 0;
            for (k = 0; k < TAPS; k++)
            {
                long at = center + k - (TAPS - 1) / 2;
                float v;
                if (at < (long)from || at >= (long)to)
                    continue;
                v = channels == 2 ? R.buf[at * 2 + c] : (R.buf[at * 2] + R.buf[at * 2 + 1]) * 0.5f;
                acc += v * h[k];
            }
            acc *= 32767.0f;
            out[i * channels + c] = (s16)(acc > 32767 ? 32767 : acc < -32768 ? -32768 : lrintf(acc));
        }
    }
    *outFrames = n;
    return out;
}

static void Add(const char *name, u8 kind, u16 id, u32 from, u32 to, u32 loopAt, u32 loopEnd, int down, int channels)
{
    struct Sound *s;
    s16 *pcm;
    u32 frames, i, nibbles;
    struct Adpcm a[2];
    u8 *p;
    int c;
    if (to <= from + 16)
        return;
    if (sCount == sSoundCap)
    {
        sSoundCap = sSoundCap ? sSoundCap * 2 : 256;
        sSounds = realloc(sSounds, sSoundCap * sizeof(*sSounds));
    }
    s = &sSounds[sCount++];
    memset(s, 0, sizeof(*s));
    snprintf(s->name, sizeof(s->name), "%s", name);
    s->kind = kind;
    s->id = id;
    s->alias = -1;
    s->channels = (u8)channels;
    s->rate = MIXER_RATE / down;
    pcm = Downsample(from, to, down, channels, &frames);
    s->frames = frames;
    s->loop = loopAt == 0xFFFFFFFF ? 0xFFFFFFFF : (loopAt - from) / down;
    s->loopEnd = loopAt == 0xFFFFFFFF ? frames : (loopEnd - from) / down;
    nibbles = frames * channels;
    s->size = 4 * channels + (nibbles + 1) / 2;
    s->data = calloc(1, s->size);
    p = s->data;
    for (c = 0; c < channels; c++)
    {
        a[c].pred = pcm[c];
        a[c].index = frames > 1 ? StartIndex(pcm[c], pcm[channels + c]) : 0;
        p[c * 4] = (u8)(a[c].pred & 0xFF);
        p[c * 4 + 1] = (u8)((a[c].pred >> 8) & 0xFF);
        p[c * 4 + 2] = (u8)a[c].index;
    }
    p += 4 * channels;
    for (i = 0; i < nibbles; i++)
    {
        u8 code = AdpcmNibble(&a[i % channels], pcm[i]);
        p[i / 2] |= (i & 1) ? (u8)(code << 4) : code;
    }
    free(pcm);
}

// Where the sound starts: past the mixer's silent frames
static u32 FirstSound(void)
{
    u32 i;
    for (i = 0; i < R.len; i++)
        if (fabsf(R.buf[i * 2]) > QUIET * 0.2f || fabsf(R.buf[i * 2 + 1]) > QUIET * 0.2f)
            return i;
    return R.len;
}

// The end, past which it stays quiet
static u32 LastSound(void)
{
    u32 i = R.len;
    while (i > 0 && fabsf(R.buf[i * 2 - 2]) < QUIET * 0.2f && fabsf(R.buf[i * 2 - 1]) < QUIET * 0.2f)
        i--;
    return i;
}

static void FileName(char *out, size_t size, const char *prefix, u16 song)
{
    const char *n = MP_Kaisong_SongFileName(song);
    if (n)
        snprintf(out, size, "%s%s", prefix, n);
    else
        snprintf(out, size, "%ssong_%u", prefix, song);
}

static bool8 SePlaying(void)
{
    return IsSEPlaying();
}

static bool8 CryPlaying(void)
{
    return IsCryPlaying();
}

static void Effects(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sEffects); i++)
    {
        char name[48];
        Silence();
        PlaySE(sEffects[i]);
        PlayOut(SePlaying, 60 * 6);
        FileName(name, sizeof(name), "", sEffects[i]);
        Add(name, KIND_SE, sEffects[i], FirstSound(), LastSound(), 0xFFFFFFFF, 0, 2, 1);
    }
}

static void Cries(void)
{
    u16 species;
    for (species = 1; species < NUM_SPECIES; species++)
    {
        char name[16];
        int tries;
        // (now and then a cry player is still taken after the one before, and the cry doesn't start: once more,
        // after a longer quiet)
        for (tries = 0; tries < 4; tries++)
        {
            int n;
            Silence();
            for (n = 0; n < tries * 30; n++)
                Frame();
            R.len = 0;
            R.frame = 0;
            PlayCry_NormalNoDucking(species, 0, CRY_VOLUME, CRY_PRIORITY_NORMAL);
            PlayOut(CryPlaying, 60 * 6);
            if (FirstSound() + 16 < LastSound())
                break;
        }
        snprintf(name, sizeof(name), "cry_%u", species);
        Add(name, KIND_CRY, species, FirstSound(), LastSound(), 0xFFFFFFFF, 0, 4, 1);
    }
}

static void Fanfares(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sFanfares); i++)
    {
        char name[48];
        Silence();
        sSongPlayer = PlayerOf(sFanfares[i]);
        m4aSongNumStart(sFanfares[i]);
        PlayOut(SongPlaying, 60 * 20);
        FileName(name, sizeof(name), "", sFanfares[i]);
        Add(name, KIND_FANFARE, sFanfares[i], FirstSound(), LastSound(), 0xFFFFFFFF, 0, 2, 2);
    }
}

// A song once through its loop: [start, the GOTO back) with the loop's place; a song without one plays to its end
static void Songs(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSongs); i++)
    {
        char name[64], prefix[16];
        struct MP2KPlayerState *player;
        u32 start, end, loop = 0xFFFFFFFF, loopEnd = 0, f;
        Silence();
        sSongPlayer = PlayerOf(sSongs[i].song);
        player = (struct MP2KPlayerState *)sSongPlayer;
        MP_Kaisong_Request(sSongs[i].file);
        m4aSongNumStart(sSongs[i].song);
        MP_Kaisong_RequestEnd();
        snprintf(prefix, sizeof(prefix), "%s:", sSongs[i].scene);
        FileName(name, sizeof(name), prefix, sSongs[i].song);
        {
            // the same music as a scene before it (no file of this scene's own): that one again
            u32 j;
            for (j = 0; j < sCount; j++)
                if (sSounds[j].header == sSongPlayer->songHeader && sSounds[j].alias < 0)
                    break;
            if (j < sCount)
            {
                struct Sound copy = sSounds[j];
                if (sCount == sSoundCap)
                {
                    sSoundCap = sSoundCap ? sSoundCap * 2 : 256;
                    sSounds = realloc(sSounds, sSoundCap * sizeof(*sSounds));
                }
                snprintf(copy.name, sizeof(copy.name), "%s", name);
                copy.alias = (int)j;
                copy.data = NULL;
                sSounds[sCount++] = copy;
                continue;
            }
        }
        R.watched = player->tracks;
        R.trackPos[0] = (uintptr_t)R.watched->cmdPtr;
        gMpGotoWatch = GotoWatch;
        while (R.frame < MAX_FRAMES && R.endFrame == 0 && (R.frame < 4 || SongPlaying()))
            Frame();
        gMpGotoWatch = NULL;
        start = FirstSound();
        if (R.endFrame)
        {
            // it loops back to where the track was first past the GOTO's place
            for (f = 1; f <= R.endFrame && f <= MAX_FRAMES; f++)
                if (R.trackPos[f] > (uintptr_t)R.loopTo)
                    break;
            loopEnd = R.frameStart[R.endFrame - 1];
            loop = R.frameStart[f - 1];
            // the song going on past the loop's end, for the crossfade
            while (R.len < loopEnd + MIXER_RATE / 15 && R.frame < MAX_FRAMES)
                Frame();
            end = loopEnd + MIXER_RATE / 20;
            if (loop < start)
                start = loop;
            printf("[export] %s:%s loops at %.2f s of %.2f s\n", sSongs[i].scene, MP_Kaisong_SongFileName(sSongs[i].song),
                   (loop - start) / (double)MIXER_RATE, (loopEnd - start) / (double)MIXER_RATE);
        }
        else
        {
            PlayOut(SongPlaying, MAX_FRAMES);
            end = LastSound();
        }
        Add(name, R.endFrame ? KIND_SONG : KIND_FANFARE, sSongs[i].song, start, end, loop, loopEnd, 2, 2);
        sSounds[sCount - 1].header = sSongPlayer->songHeader;
    }
    R.watched = NULL;
}

static void W32(FILE *f, u32 v)
{
    u8 b[4] = {(u8)v, (u8)(v >> 8), (u8)(v >> 16), (u8)(v >> 24)};
    fwrite(b, 1, 4, f);
}

static void W16(FILE *f, u16 v)
{
    u8 b[2] = {(u8)v, (u8)(v >> 8)};
    fwrite(b, 1, 2, f);
}

void MP_Export_Sounds(const char *dir)
{
    char path[512];
    FILE *f;
    u32 i, header = 12, offset;
    // (custom music needs a save to look at: at the first frame there's none yet, so one with no map: no place's own
    // music, only the scenes')
    static struct SaveBlock1 sNoSave;
    struct SaveBlock1 *save = gSaveBlock1Ptr;
    s16 group = 0, num = 0;
    if (save == NULL)
        gSaveBlock1Ptr = &sNoSave;
    group = gSaveBlock1Ptr->location.mapGroup;
    num = gSaveBlock1Ptr->location.mapNum;
    gSaveBlock1Ptr->location.mapGroup = 0x7F;
    gSaveBlock1Ptr->location.mapNum = 0x7F;
    gMpAudioCapture = Capture;
    if (!getenv("MOEMON_EXPORT_SOUND_ONLY") || strstr(getenv("MOEMON_EXPORT_SOUND_ONLY"), "se"))
        Effects();
    if (!getenv("MOEMON_EXPORT_SOUND_ONLY") || strstr(getenv("MOEMON_EXPORT_SOUND_ONLY"), "cry"))
        Cries();
    if (!getenv("MOEMON_EXPORT_SOUND_ONLY") || strstr(getenv("MOEMON_EXPORT_SOUND_ONLY"), "fanfare"))
        Fanfares();
    if (!getenv("MOEMON_EXPORT_SOUND_ONLY") || strstr(getenv("MOEMON_EXPORT_SOUND_ONLY"), "song"))
        Songs();
    m4aMPlayAllStop();
    gMpAudioCapture = NULL;
    gSaveBlock1Ptr->location.mapGroup = group;
    gSaveBlock1Ptr->location.mapNum = num;
    gSaveBlock1Ptr = save;

    for (i = 0; i < sCount; i++)
        header += 1 + strlen(sSounds[i].name) + 1 + 1 + 2 + 4 * 6;
    snprintf(path, sizeof(path), "%s/sounds.bin", dir);
    f = fopen(path, "wb");
    if (!f)
        return;
    fwrite("MSND", 1, 4, f);
    W32(f, 1);
    W32(f, sCount);
    offset = header;
    for (i = 0; i < sCount; i++)
    {
        struct Sound *s = &sSounds[i];
        u8 len = (u8)strlen(s->name);
        u32 at = s->alias >= 0 ? sSounds[s->alias].offset : offset;   // (the same music in two scenes: once)
        s->offset = at;
        fputc(len, f);
        fwrite(s->name, 1, len, f);
        fputc(s->kind, f);
        fputc(s->channels, f);
        W16(f, s->id);
        W32(f, s->rate);
        W32(f, s->frames);
        W32(f, s->loop);
        W32(f, s->loopEnd);
        W32(f, at);
        W32(f, s->size);
        if (s->alias < 0)
            offset += s->size;
    }
    {
        u32 written = header;
        for (i = 0; i < sCount; i++)
            if (sSounds[i].alias < 0)
            {
                fwrite(sSounds[i].data, 1, sSounds[i].size, f);
                written += sSounds[i].size;
            }
        for (i = 0; i < sCount; i++)
            free(sSounds[i].data);
    }
    fclose(f);
    printf("[export] %u sounds, %u KB\n", sCount, offset / 1024);
    free(sSounds);
    sSounds = NULL;
    sCount = sSoundCap = 0;
    free(R.buf);
    R.buf = NULL;
    R.len = R.cap = 0;
}
