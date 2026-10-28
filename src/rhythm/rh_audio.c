// Moemon Rhythm: the DANCE machine's sound, mixed into the game's own (42048 Hz stereo, sdl2.c
// Platform_QueueAudio) in place of the GBA's while the machine is on (docs/RHYTHM_DECISIONS.md #7, #8).
// - Music: a song's file (Ogg Vorbis or MP3, read into memory) decoded as it plays and resampled (cubic) to the
//   game's rate. A position before the music's start is silence, so a song's lead-in runs on the same clock.
// - The clock: the music's position as heard now. Each block mixed is heard after what's queued ahead of it and the
//   sound card's own buffer; that delay is measured every block and smoothed against the PC's clock, so the arrows
//   move smoothly between blocks. Headless tests have no sound card: the clock is the samples mixed.
// - Sounds and the announcer: decoded whole when first played (they're short), from rhythm.bin.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "rh.h"
#include "../../fight/third_party/audio/dr_mp3.h"

// stb_vorbis (compiled in fight/compat/mf_bgm.c)
typedef struct stb_vorbis stb_vorbis;
typedef struct
{
    unsigned int sample_rate;
    int channels;
    unsigned int setup_memory_required, setup_temp_memory_required, temp_memory_required;
    int max_frame_size;
} stb_vorbis_info;
stb_vorbis *stb_vorbis_open_memory(const unsigned char *data, int len, int *error, const void *alloc);
stb_vorbis_info stb_vorbis_get_info(stb_vorbis *f);
int stb_vorbis_get_samples_short_interleaved(stb_vorbis *f, int channels, short *buffer, int num_shorts);
int stb_vorbis_seek(stb_vorbis *f, unsigned int sample_number);
unsigned int stb_vorbis_stream_length_in_samples(stb_vorbis *f);
void stb_vorbis_close(stb_vorbis *f);
int stb_vorbis_decode_memory(const unsigned char *mem, int len, int *channels, int *sample_rate, short **output);

bool8 Platform_IsTestMode(void);

#define OUT_RATE 42048.0
#define DEVICE_FRAMES 1024      // sdl2.c's want.samples
#define DECODE_CHUNK 2048

struct Music
{
    int on;
    uint8_t *file;
    stb_vorbis *ogg;
    drmp3 mp3;
    int isMp3;
    int channels;
    double rate;
    double length;              // seconds
    // where we are in the music, in its own frames (fractional; negative: silence before it)
    double pos;
    // decoded frames ahead: buf[0..count) starts at frame bufStart
    short buf[(DECODE_CHUNK + 8) * 2];
    int64_t bufStart;
    int count;
    int64_t decodedTo;          // the decoder's next frame
    int ended;
    // the preview loop
    int loop;
    double from, until;         // seconds
    double fade, fadeStep;      // volume 0-1 and its change per output frame
    int fadingOut;
};

static struct Music sMusic;

// the clock: heard = PC seconds + sOffset (music seconds)
static double sOffset, sMixedPos;
static int sClockValid;

#define SFX_SLOTS 8
struct Sfx
{
    char name[64];
    short *pcm;                 // stereo
    int frames, rate;
};
static struct Sfx sSfxCache[96];
static int sSfxCached;
static struct
{
    const struct Sfx *sfx;
    double pos;
    int announcer;
} sPlaying[SFX_SLOTS];

double RhAudio_Now(void)
{
    return (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
}

// ---------------------------------------------------------------------------
// Music

static void CloseMusic(void)
{
    if (sMusic.ogg)
        stb_vorbis_close(sMusic.ogg);
    if (sMusic.isMp3)
        drmp3_uninit(&sMusic.mp3);
    free(sMusic.file);
    memset(&sMusic, 0, sizeof(sMusic));
}

static uint8_t *ReadFile(const char *path, long *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (*size <= 0 || !(data = malloc(*size)))
    {
        fclose(f);
        return NULL;
    }
    if (fread(data, 1, *size, f) != (size_t)*size)
    {
        fclose(f);
        free(data);
        return NULL;
    }
    fclose(f);
    return data;
}

static void SeekTo(int64_t frame)
{
    if (frame < 0)
        frame = 0;
    if (sMusic.ogg)
        stb_vorbis_seek(sMusic.ogg, (unsigned int)frame);
    else if (sMusic.isMp3)
        drmp3_seek_to_pcm_frame(&sMusic.mp3, (drmp3_uint64)frame);
    sMusic.decodedTo = frame;
    sMusic.bufStart = frame;
    sMusic.count = 0;
    sMusic.ended = 0;
}

// Decoded frames from `frame` on: makes sure buf holds frame .. frame + 3
static void Ensure(int64_t frame)
{
    if (frame >= sMusic.bufStart && frame + 4 <= sMusic.bufStart + sMusic.count)
        return;
    if (frame < sMusic.bufStart || frame > sMusic.decodedTo + DECODE_CHUNK)
        SeekTo(frame);
    // keep what's from `frame` on, decode more after it
    if (frame > sMusic.bufStart && sMusic.count > 0)
    {
        int keep = (int)(sMusic.bufStart + sMusic.count - frame);
        if (keep < 0)
            keep = 0;
        memmove(sMusic.buf, sMusic.buf + (sMusic.count - keep) * 2, keep * 2 * sizeof(short));
        sMusic.count = keep;
        sMusic.bufStart = frame;
    }
    while (sMusic.count < DECODE_CHUNK && !sMusic.ended)
    {
        short tmp[1024 * 2];
        int want = DECODE_CHUNK - sMusic.count, got;
        if (want > 1024)
            want = 1024;
        if (sMusic.ogg)
            got = stb_vorbis_get_samples_short_interleaved(sMusic.ogg, 2, tmp, want * 2);
        else
        {
            drmp3_int16 m[1024 * 2];
            got = (int)drmp3_read_pcm_frames_s16(&sMusic.mp3, (drmp3_uint64)want, m);
            if (sMusic.channels == 1)
            {
                int i;
                for (i = 0; i < got; i++)
                    tmp[i * 2] = tmp[i * 2 + 1] = m[i];
            }
            else
                memcpy(tmp, m, got * 2 * sizeof(short));
        }
        if (got <= 0)
        {
            sMusic.ended = 1;
            break;
        }
        memcpy(sMusic.buf + sMusic.count * 2, tmp, got * 2 * sizeof(short));
        sMusic.count += got;
        sMusic.decodedTo += got;
    }
}

static float At(int64_t frame, int ch)
{
    if (frame < sMusic.bufStart || frame >= sMusic.bufStart + sMusic.count)
        return 0;
    return sMusic.buf[(frame - sMusic.bufStart) * 2 + ch] / 32768.0f;
}

static int OpenMusic(uint8_t *file, long size, double from, double length, int loop, double fadeIn)
{
    CloseMusic();
    sMusic.file = file;
    if (!sMusic.file)
        return 0;
    sMusic.ogg = stb_vorbis_open_memory(sMusic.file, (int)size, NULL, NULL);
    if (sMusic.ogg)
    {
        stb_vorbis_info info = stb_vorbis_get_info(sMusic.ogg);
        sMusic.rate = info.sample_rate;
        sMusic.channels = info.channels;
        sMusic.length = stb_vorbis_stream_length_in_samples(sMusic.ogg) / (double)info.sample_rate;
    }
    else if (drmp3_init_memory(&sMusic.mp3, sMusic.file, (size_t)size, NULL))
    {
        sMusic.isMp3 = 1;
        sMusic.rate = sMusic.mp3.sampleRate;
        sMusic.channels = (int)sMusic.mp3.channels;
        sMusic.length = drmp3_get_pcm_frame_count(&sMusic.mp3) / (double)sMusic.mp3.sampleRate;
        if (sMusic.channels > 2)
        {
            CloseMusic();
            return 0;
        }
    }
    else
    {
        CloseMusic();
        return 0;
    }
    if (sMusic.rate <= 0)
    {
        CloseMusic();
        return 0;
    }
    sMusic.on = 1;
    sMusic.loop = loop;
    sMusic.from = from;
    sMusic.until = length > 0 ? from + length : 1e9;
    sMusic.pos = from * sMusic.rate;
    sMusic.fade = fadeIn > 0 ? 0 : 1;
    sMusic.fadeStep = fadeIn > 0 ? 1.0 / (fadeIn * OUT_RATE) : 0;
    SeekTo(sMusic.pos > 0 ? (int64_t)sMusic.pos : 0);
    sClockValid = 0;
    sMixedPos = from;
    return 1;
}

int RhAudio_PlayMusic(const char *path, double from, double length, int loop, double fadeIn)
{
    long size = 0;
    uint8_t *file = path ? ReadFile(path, &size) : NULL;
    return OpenMusic(file, size, from, length, loop, fadeIn);
}

void RhAudio_StopMusic(double fadeOut)
{
    if (!sMusic.on)
        return;
    if (fadeOut <= 0)
    {
        CloseMusic();
        return;
    }
    sMusic.fadingOut = 1;
    sMusic.loop = 0;
    sMusic.fadeStep = -1.0 / (fadeOut * OUT_RATE);
}

int RhAudio_MusicPlaying(void)
{
    return sMusic.on;
}

double RhAudio_MusicLength(void)
{
    return sMusic.on ? sMusic.length : 0;
}

double RhAudio_MusicTime(void)
{
    if (!sMusic.on)
        return 0;
    if (Platform_IsTestMode() || !sClockValid)
        return sMixedPos;
    return RhAudio_Now() + sOffset;
}

void RhAudio_PlayMenuMusic(void)
{
    uint32_t size;
    const uint8_t *data = RhSound_Get("snd_menu_music", &size);
    uint8_t *copy;
    if (!data || !(copy = malloc(size)))
        return;
    memcpy(copy, data, size);
    OpenMusic(copy, (long)size, 0, 0, 1, 0.3);
}

// The music into out (frames of stereo at the game's rate)
static void MixMusic(float *out, int frames)
{
    double step = sMusic.rate / OUT_RATE;
    int i;
    for (i = 0; i < frames && sMusic.on; i++)
    {
        double t = sMusic.pos / sMusic.rate;
        if (sMusic.loop && t >= sMusic.until && !sMusic.fadingOut)
        {
            // the preview starts over
            sMusic.pos = sMusic.from * sMusic.rate;
            sMusic.fade = 0;
            sMusic.fadeStep = 1.0 / (0.5 * OUT_RATE);
            t = sMusic.from;
        }
        if (sMusic.loop && !sMusic.fadingOut && sMusic.until < 1e8 && t >= sMusic.until - 1.0 && sMusic.fadeStep >= 0)
            sMusic.fadeStep = -1.0 / OUT_RATE;    // a second's fade before it starts over
        if (sMusic.pos >= 0)
        {
            int64_t f = (int64_t)sMusic.pos;
            double x = sMusic.pos - f;
            int ch;
            Ensure(f > 0 ? f - 1 : 0);
            if (sMusic.ended && f >= sMusic.bufStart + sMusic.count)
            {
                if (sMusic.loop)
                {
                    sMusic.pos = sMusic.from * sMusic.rate;
                    continue;
                }
                sMusic.on = 0;   // (CloseMusic below)
                break;
            }
            for (ch = 0; ch < 2; ch++)
            {
                // Catmull-Rom between frames f and f + 1
                float p0 = At(f - 1, ch), p1 = At(f, ch), p2 = At(f + 1, ch), p3 = At(f + 2, ch);
                float v = p1 + 0.5f * (float)x * (p2 - p0 + (float)x * (2 * p0 - 5 * p1 + 4 * p2 - p3
                          + (float)x * (3 * (p1 - p2) + p3 - p0)));
                out[i * 2 + ch] += v * (float)sMusic.fade;
            }
        }
        sMusic.pos += step;
        sMusic.fade += sMusic.fadeStep;
        if (sMusic.fade >= 1)
        {
            sMusic.fade = 1;
            if (sMusic.fadeStep > 0)
                sMusic.fadeStep = 0;
        }
        if (sMusic.fade <= 0)
        {
            sMusic.fade = 0;
            if (sMusic.fadingOut)
                sMusic.on = 0;
            else if (sMusic.fadeStep < 0 && !sMusic.loop)
                sMusic.fadeStep = 0;
        }
    }
    if (!sMusic.on)
        CloseMusic();
}

// ---------------------------------------------------------------------------
// Sounds

static const struct Sfx *SfxFor(const char *name)
{
    int i, ch, rate;
    short *pcm;
    uint32_t size;
    const uint8_t *data;
    struct Sfx *s;
    for (i = 0; i < sSfxCached; i++)
        if (!strcmp(sSfxCache[i].name, name))
            return &sSfxCache[i];
    data = RhSound_Get(name, &size);
    if (!data)
        return NULL;
    i = stb_vorbis_decode_memory(data, (int)size, &ch, &rate, &pcm);
    if (i <= 0 || ch < 1)
        return NULL;
    if (sSfxCached == (int)(sizeof(sSfxCache) / sizeof(sSfxCache[0])))
    {
        // forget the oldest (not one playing)
        int k;
        for (k = 0; k < SFX_SLOTS; k++)
            if (sPlaying[k].sfx == &sSfxCache[0])
                sPlaying[k].sfx = NULL;
        free(sSfxCache[0].pcm);
        memmove(sSfxCache, sSfxCache + 1, (sSfxCached - 1) * sizeof(sSfxCache[0]));
        sSfxCached--;
        for (k = 0; k < SFX_SLOTS; k++)
            if (sPlaying[k].sfx)
                sPlaying[k].sfx--;
    }
    s = &sSfxCache[sSfxCached++];
    snprintf(s->name, sizeof(s->name), "%s", name);
    s->frames = i;
    s->rate = rate;
    if (ch == 2)
        s->pcm = pcm;
    else
    {
        int k;
        s->pcm = malloc((size_t)i * 2 * sizeof(short));
        for (k = 0; k < i; k++)
            s->pcm[k * 2] = s->pcm[k * 2 + 1] = pcm[k * ch];
        free(pcm);
    }
    return s;
}

static void PlaySfx(const struct Sfx *s, int announcer)
{
    int i, slot = -1;
    if (!s)
        return;
    for (i = 0; i < SFX_SLOTS; i++)
    {
        if (announcer && sPlaying[i].sfx && sPlaying[i].announcer)
            sPlaying[i].sfx = NULL;   // one line at a time
        if (!sPlaying[i].sfx && slot < 0)
            slot = i;
    }
    if (slot < 0)
        slot = 0;
    sPlaying[slot].sfx = s;
    sPlaying[slot].pos = 0;
    sPlaying[slot].announcer = announcer;
}

void RhAudio_Sfx(const char *name)
{
    PlaySfx(SfxFor(name), 0);
}

int RhAudio_Announce(const char *folder)
{
    char prefix[80], name[96];
    int n;
    snprintf(prefix, sizeof(prefix), "ann:%s:", folder);
    n = RhSound_Count(prefix);
    if (n <= 0)
        return 0;
    snprintf(name, sizeof(name), "%s%d", prefix, rand() % n);
    PlaySfx(SfxFor(name), 1);
    return 1;
}

int RhAudio_Announcing(void)
{
    int i;
    for (i = 0; i < SFX_SLOTS; i++)
        if (sPlaying[i].sfx && sPlaying[i].announcer)
            return 1;
    return 0;
}

void RhAudio_StopAll(void)
{
    CloseMusic();
    memset(sPlaying, 0, sizeof(sPlaying));
}

void RhAudio_Mix(float *stereo, int frames)
{
    int i, k;
    double before = sMusic.on ? sMusic.pos / sMusic.rate : 0;
    memset(stereo, 0, (size_t)frames * 2 * sizeof(float));
    if (sMusic.on)
    {
        // the clock: this block's first frame is heard after what's queued and the sound card's buffer
        double now = RhAudio_Now();
        double delay = (SDL_GetQueuedAudioSize(1) / 8 + DEVICE_FRAMES) / OUT_RATE;
        double offset = before - (now + delay);
        sMixedPos = before;
        if (!sClockValid || fabs(offset - sOffset) > 0.03)
            sOffset = offset;
        else
            sOffset += (offset - sOffset) * 0.05;
        sClockValid = 1;
        MixMusic(stereo, frames);
    }
    for (k = 0; k < SFX_SLOTS; k++)
    {
        const struct Sfx *s = sPlaying[k].sfx;
        double step;
        if (!s)
            continue;
        step = s->rate / OUT_RATE;
        for (i = 0; i < frames; i++)
        {
            int f = (int)sPlaying[k].pos;
            double x = sPlaying[k].pos - f;
            if (f + 1 >= s->frames)
            {
                sPlaying[k].sfx = NULL;
                break;
            }
            stereo[i * 2] += (float)((s->pcm[f * 2] * (1 - x) + s->pcm[f * 2 + 2] * x) / 32768.0);
            stereo[i * 2 + 1] += (float)((s->pcm[f * 2 + 1] * (1 - x) + s->pcm[f * 2 + 3] * x) / 32768.0);
            sPlaying[k].pos += step;
        }
    }
    for (i = 0; i < frames * 2; i++)
    {
        // soft limit
        float v = stereo[i];
        if (v > 1)
            v = 1;
        if (v < -1)
            v = -1;
        stereo[i] = v;
    }
}
