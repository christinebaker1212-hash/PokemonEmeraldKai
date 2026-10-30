/*
 * Moemon Fight: the part of SDL_mixer the engine uses, played through the game's audio.
 *
 * The mixer runs at 13379 Hz, the rate of Emerald's own samples, and the engine resamples every
 * sound and song to it (its WAV reader and BGM thread do that for SDL_mixer too). Mixing keeps
 * them at 8 bits like the GBA's DirectSound: sound effects are folded to mono, music stays
 * stereo. MfMix_Render then point-samples the result up to the game's output rate, as the
 * game's m4a mixer plays its own samples, and adds it to the game's sound. mf_rename.h gives the
 * Mix_ / SDL_ names here an MF12_ prefix.
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "mf_compat.h"

#define MIX_BLOCK 256       /* frames mixed at a time */
#define MIX_MAX_CHANNELS 64

typedef struct MixChannel {
    Mix_Chunk *chunk;
    Uint32 pos;             /* bytes */
    int loops;              /* -1: forever */
    int volume;             /* 0..MIX_MAX_VOLUME */
    int left, right;        /* panning, 0..255 */
    int playing;
} MixChannel;

static pthread_mutex_t sLock;
static int sLockReady;
static int sOpen;
static int sChunkSize = 1024;
static MixChannel sChannels[MIX_MAX_CHANNELS];
static int sChannelCount = 8;
static void (*sMusicHook)(void *udata, Uint8 *stream, int len);
static void *sMusicArg;
static void (*sMusicFinished)(void);
static int sMusicVolume = MIX_MAX_VOLUME;
/* Stage music under the voices: the fighters' voice level and the song's, both RMS 0..1 (0: not known) */
static volatile double sVoiceLevel, sMusicLevel;
/* The music hook hands over one whole block of the engine's music ring per call, the chunk size the mixer was
 * opened with (as SDL_mixer asks it): taken a block at a time, played out MIX_BLOCK frames at a time */
static Sint16 *sMusicBuf;
static int sMusicFrames, sMusicPos;

/* MfMix_Render's state: the current 13379 Hz frame and the block it comes from */
static Sint16 sBlock[MIX_BLOCK * 2];
static int sBlockPos = MIX_BLOCK;
static int sCur[2];
static unsigned int sPhase;

static void Mix_Lock(void)
{
    if (!sLockReady) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&sLock, &attr);
        pthread_mutexattr_destroy(&attr);
        sLockReady = 1;
    }
    pthread_mutex_lock(&sLock);
}

static void Mix_Unlock(void)
{
    pthread_mutex_unlock(&sLock);
}

/* Keeps the top 8 bits of a sample, as the GBA plays it. */
static int Gba8(int s)
{
    return (int)(Sint16)(s & ~0xFF);
}

/* Mixes the next MIX_BLOCK frames into sBlock (under the lock). */
static void Mix_Block(void)
{
    int acc[MIX_BLOCK * 2];
    Sint16 music[MIX_BLOCK * 2];
    int i, c;

    memset(acc, 0, sizeof(acc));
    if (sMusicHook != NULL) {
        int have = 0;
        while (have < MIX_BLOCK) {
            int n;
            if (sMusicPos >= sMusicFrames) {
                Sint16 *buf = realloc(sMusicBuf, (size_t)sChunkSize * 2 * sizeof(Sint16));
                if (buf == NULL) {
                    break;
                }
                sMusicBuf = buf;
                memset(sMusicBuf, 0, (size_t)sChunkSize * 2 * sizeof(Sint16));
                sMusicHook(sMusicArg, (Uint8 *)sMusicBuf, sChunkSize * 2 * (int)sizeof(Sint16));
                sMusicFrames = sChunkSize;
                sMusicPos = 0;
            }
            n = sMusicFrames - sMusicPos;
            if (n > MIX_BLOCK - have) {
                n = MIX_BLOCK - have;
            }
            memcpy(music + have * 2, sMusicBuf + sMusicPos * 2, (size_t)n * 2 * sizeof(Sint16));
            have += n;
            sMusicPos += n;
        }
        {
            /* the music's volume, times the gain that keeps a stage's song under the voices (out of 256) */
            int vol = (int)(sMusicVolume * 2 * MfMix_MusicGain() + 0.5);
            for (i = 0; i < have * 2; i++) {
                acc[i] += Gba8(music[i]) * vol / (MIX_MAX_VOLUME * 2);
            }
        }
    }
    for (c = 0; c < sChannelCount; c++) {
        MixChannel *ch = &sChannels[c];
        int vol;
        if (!ch->playing || ch->chunk == NULL || ch->chunk->abuf == NULL || ch->chunk->alen < 4) {
            continue;
        }
        vol = ch->volume * ch->chunk->volume / MIX_MAX_VOLUME;
        for (i = 0; i < MIX_BLOCK; i++) {
            const Sint16 *f;
            int mono;
            if (ch->pos + 4 > ch->chunk->alen) {
                if (ch->loops != 0) {
                    if (ch->loops > 0) {
                        ch->loops--;
                    }
                    ch->pos = 0;
                } else {
                    ch->playing = 0;
                    break;
                }
            }
            f = (const Sint16 *)(ch->chunk->abuf + ch->pos);
            mono = Gba8((f[0] + f[1]) / 2) * vol / MIX_MAX_VOLUME;
            acc[i * 2] += mono * ch->left / 255;
            acc[i * 2 + 1] += mono * ch->right / 255;
            ch->pos += 4;
        }
    }
    for (i = 0; i < MIX_BLOCK * 2; i++) {
        int v = acc[i] * 5 / 8;     /* about the game's own loudness, with room before clipping */
        sBlock[i] = (Sint16)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
    }
    sBlockPos = 0;
}

void MfMix_Render(short *stereo, int frames, int rate)
{
    int i;

    if (!sOpen || rate <= 0) {
        return;
    }
    Mix_Lock();
    for (i = 0; i < frames; i++) {
        int l = stereo[i * 2] + sCur[0];
        int r = stereo[i * 2 + 1] + sCur[1];
        stereo[i * 2] = (short)(l > 32767 ? 32767 : l < -32768 ? -32768 : l);
        stereo[i * 2 + 1] = (short)(r > 32767 ? 32767 : r < -32768 ? -32768 : r);
        sPhase += MF_MIX_RATE;
        while (sPhase >= (unsigned int)rate) {
            sPhase -= (unsigned int)rate;
            if (sBlockPos >= MIX_BLOCK) {
                Mix_Block();
            }
            sCur[0] = sBlock[sBlockPos * 2];
            sCur[1] = sBlock[sBlockPos * 2 + 1];
            sBlockPos++;
        }
    }
    Mix_Unlock();
}

/*
 * Music under the voices. A sound's level is its RMS over the 50 ms windows where it sounds (above -45 dBFS),
 * so pauses don't count; sounds shorter than 0.15 s (hits, steps) give 0 and leave voices to the longer ones.
 */
double MfMix_ClipLevel(const short *stereo, int frames)
{
    const int win = MF_MIX_RATE / 20;
    double sum = 0.0;
    int active = 0, i, j;

    if (stereo == NULL || frames < MF_MIX_RATE * 15 / 100) {
        return 0.0;
    }
    for (i = 0; i + win <= frames; i += win) {
        double ms = 0.0;
        for (j = i; j < i + win; j++) {
            double v = (stereo[j * 2] + stereo[j * 2 + 1]) * (0.5 / 32768.0);
            ms += v * v;
        }
        ms /= win;
        if (ms > 3.1623e-5) {       /* -45 dBFS */
            sum += ms;
            active++;
        }
    }
    return active > 0 ? sqrt(sum / active) : 0.0;
}

/* A song's level from the mean squares of its 0.4 s blocks: blocks above -50 dBFS, then those within 10 dB of
 * their power mean (the gates of EBU R 128, without its weighting filter). ms is reordered. */
double MfMix_GatedLevel(double *ms, int count)
{
    double sum = 0.0, mean;
    int n = 0, i;

    for (i = 0; i < count; i++) {
        if (ms[i] > 1e-5) {
            ms[n++] = ms[i];
            sum += ms[i];
        }
    }
    if (n == 0) {
        return 0.0;
    }
    mean = sum / n;
    sum = 0.0;
    count = 0;
    for (i = 0; i < n; i++) {
        if (ms[i] > mean * 0.1) {
            sum += ms[i];
            count++;
        }
    }
    return count > 0 ? sqrt(sum / count) : 0.0;
}

void MfMix_SetVoiceLevel(double level)
{
    sVoiceLevel = level > 0.0 ? level : 0.0;
}

void MfMix_SetMusicLevel(double level)
{
    sMusicLevel = level > 0.0 ? level : 0.0;
}

double MfMix_MusicLevel(void)
{
    return sMusicLevel;
}

static volatile int sMusicOpens;

void MfMix_MusicOpened(void)
{
    sMusicOpens++;
}

int MfMix_MusicOpens(void)
{
    return sMusicOpens;
}

/* At most 1: the song at its volume stays MF_BGM_UNDER_VOICES_DB under the voices. */
double MfMix_MusicGain(void)
{
    double voice = sVoiceLevel, music = sMusicLevel * sMusicVolume / MIX_MAX_VOLUME;
    double cap;

    if (voice <= 0.0 || music <= 0.0) {
        return 1.0;
    }
    cap = voice * pow(10.0, -MF_BGM_UNDER_VOICES_DB / 20.0) / music;
    return cap < 1.0 ? cap : 1.0;
}

void MfMix_Reset(void)
{
    int c;

    Mix_Lock();
    for (c = 0; c < MIX_MAX_CHANNELS; c++) {
        sChannels[c].playing = 0;
    }
    sCur[0] = sCur[1] = 0;
    sBlockPos = MIX_BLOCK;
    Mix_Unlock();
}

/* ------------------------------------------------------------------ SDL_mixer */

int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
{
    int c;

    Mix_Lock();
    sChunkSize = chunksize > 0 ? chunksize : 1024;
    for (c = 0; c < MIX_MAX_CHANNELS; c++) {
        memset(&sChannels[c], 0, sizeof(sChannels[c]));
        sChannels[c].volume = MIX_MAX_VOLUME;
        sChannels[c].left = sChannels[c].right = 255;
    }
    sOpen = 1;
    Mix_Unlock();
    return 0;
}

/* Elecbyte's addition to their SDL_mixer: what Mix_OpenAudio opened. */
int Mix_QuerySpecEx(int *frequency, Uint16 *format, int *channels, int *chunksize)
{
    if (frequency != NULL) {
        *frequency = MF_MIX_RATE;
    }
    if (format != NULL) {
        *format = AUDIO_S16LSB;
    }
    if (channels != NULL) {
        *channels = 2;
    }
    if (chunksize != NULL) {
        *chunksize = sChunkSize;
    }
    return sOpen;
}

void Mix_CloseAudio(void)
{
    Mix_Lock();
    sOpen = 0;
    sMusicHook = NULL;
    Mix_Unlock();
}

int Mix_AllocateChannels(int numchans)
{
    if (numchans < 0) {
        return sChannelCount;
    }
    Mix_Lock();
    if (numchans > MIX_MAX_CHANNELS) {
        numchans = MIX_MAX_CHANNELS;
    }
    sChannelCount = numchans;
    Mix_Unlock();
    return sChannelCount;
}

Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc)
{
    /* The engine reads WAVs itself (its resample method is always set to the libresample one) */
    if (src != NULL && freesrc) {
        src->close(src);
    }
    return NULL;
}

Mix_Chunk *Mix_QuickLoad_RAW(Uint8 *mem, Uint32 len)
{
    Mix_Chunk *chunk = calloc(1, sizeof(Mix_Chunk));

    if (chunk == NULL) {
        return NULL;
    }
    chunk->allocated = 0;
    chunk->abuf = mem;
    chunk->alen = len;
    chunk->volume = MIX_MAX_VOLUME;
    return chunk;
}

void Mix_FreeChunk(Mix_Chunk *chunk)
{
    int c;

    if (chunk == NULL) {
        return;
    }
    Mix_Lock();
    for (c = 0; c < MIX_MAX_CHANNELS; c++) {
        if (sChannels[c].chunk == chunk) {
            sChannels[c].playing = 0;
            sChannels[c].chunk = NULL;
        }
    }
    Mix_Unlock();
    if (chunk->allocated) {
        free(chunk->abuf);
    }
    free(chunk);
}

/* Sounds started in the last frames, so a frame run again after going back (rollback) doesn't start them twice */
#define PLAYED_LOG 256
static struct {
    unsigned int frame;
    const Mix_Chunk *chunk;
    int asked, got;
} sPlayed[PLAYED_LOG];
static int sPlayedNext;

int MfHost_Replaying(void);
unsigned int MfHost_FrameRunning(void);

int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
    unsigned int frame = MfHost_FrameRunning();
    int asked = channel;
    int c;

    if (chunk == NULL) {
        return -1;
    }
    if (MfHost_Replaying()) {
        for (c = 0; c < PLAYED_LOG; c++) {
            if (sPlayed[c].frame == frame && sPlayed[c].chunk == chunk && sPlayed[c].asked == asked) {
                return sPlayed[c].got;  /* already playing from the first run */
            }
        }
    }
    Mix_Lock();
    if (channel < 0) {
        for (c = 0; c < sChannelCount; c++) {
            if (!sChannels[c].playing) {
                break;
            }
        }
        channel = c < sChannelCount ? c : -1;
    }
    if (channel >= 0 && channel < sChannelCount) {
        sChannels[channel].chunk = chunk;
        sChannels[channel].pos = 0;
        sChannels[channel].loops = loops;
        sChannels[channel].playing = 1;
    } else {
        channel = -1;
    }
    Mix_Unlock();
    sPlayed[sPlayedNext].frame = frame;
    sPlayed[sPlayedNext].chunk = chunk;
    sPlayed[sPlayedNext].asked = asked;
    sPlayed[sPlayedNext].got = channel;
    sPlayedNext = (sPlayedNext + 1) % PLAYED_LOG;
    return channel;
}

int Mix_Volume(int channel, int volume)
{
    int c, prev = 0;

    Mix_Lock();
    if (channel < 0) {
        int sum = 0;
        for (c = 0; c < sChannelCount; c++) {
            sum += sChannels[c].volume;
            if (volume >= 0) {
                sChannels[c].volume = volume > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : volume;
            }
        }
        prev = sChannelCount > 0 ? sum / sChannelCount : 0;
    } else if (channel < sChannelCount) {
        prev = sChannels[channel].volume;
        if (volume >= 0) {
            sChannels[channel].volume = volume > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : volume;
        }
    }
    Mix_Unlock();
    return prev;
}

int Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
    if (channel < 0 || channel >= sChannelCount) {
        return 0;
    }
    Mix_Lock();
    sChannels[channel].left = left;
    sChannels[channel].right = right;
    Mix_Unlock();
    return 1;
}

int Mix_HaltChannel(int channel)
{
    int c;

    Mix_Lock();
    for (c = 0; c < sChannelCount; c++) {
        if (channel < 0 || c == channel) {
            sChannels[c].playing = 0;
        }
    }
    Mix_Unlock();
    return 0;
}

int Mix_Playing(int channel)
{
    int c, n = 0;

    Mix_Lock();
    if (channel < 0) {
        for (c = 0; c < sChannelCount; c++) {
            n += sChannels[c].playing != 0;
        }
    } else if (channel < sChannelCount) {
        n = sChannels[channel].playing != 0;
    }
    Mix_Unlock();
    return n;
}

void Mix_HookMusic(void (*mix_func)(void *udata, Uint8 *stream, int len), void *arg)
{
    Mix_Lock();
    sMusicHook = mix_func;
    sMusicArg = arg;
    sMusicFrames = sMusicPos = 0;   /* the next block is the new music's */
    Mix_Unlock();
}

void Mix_HookMusicFinished(void (*music_finished)(void))
{
    Mix_Lock();
    sMusicFinished = music_finished;
    Mix_Unlock();
}

int Mix_VolumeMusic(int volume)
{
    int prev = sMusicVolume;

    if (volume >= 0) {
        sMusicVolume = volume > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : volume;
    }
    return prev;
}

/* SDL_mixer's own music (the "sdlmix" plugin's tracker and MIDI files): not played */
Mix_Music *Mix_LoadMUS_RW(SDL_RWops *rw)
{
    return NULL;
}

void Mix_FreeMusic(Mix_Music *music)
{
}

int Mix_PlayMusic(Mix_Music *music, int loops)
{
    return -1;
}

int Mix_HaltMusic(void)
{
    return 0;
}

void Mix_PauseMusic(void)
{
}

void Mix_ResumeMusic(void)
{
}

void Mix_RewindMusic(void)
{
}

int Mix_PausedMusic(void)
{
    return 0;
}

int Mix_PlayingMusic(void)
{
    return 0;
}

int Mix_SetMusicPosition(double position)
{
    return -1;
}

/* SDL_LockAudio / SDL_UnlockAudio guard what the music hook reads */
void SDL_LockAudio(void)
{
    Mix_Lock();
}

void SDL_UnlockAudio(void)
{
    Mix_Unlock();
}
