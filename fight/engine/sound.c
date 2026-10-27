/*
 * Sound effects (original module 0x46E430-0x46FB42): SDL_mixer set-up, the
 * table of playing sounds (g_sndSlots) and the loading of .snd/.wav sound
 * banks.
 *
 * Sounds are decoded either by SDL_mixer (Mix_LoadWAV_RW converts to the
 * mixer format) or, with [Sound] SFXResampleMethod = libresample, by the WAV
 * reader and resampler of the module at 0x4886E0, whose output becomes a
 * Mix_QuickLoad_RAW chunk owning a buffer from our heap.
 *
 * A Sound_Play id identifies the owner of a sound (e.g. a player channel);
 * playing again with the same id reuses its slot and so stops the previous
 * sound. Channels of the slots are SDL_mixer channels (Mix_AllocateChannels).
 */
#include <SDL.h>
#include <SDL_mixer.h>
#include <libresample.h>

/* Elecbyte's SDL_mixer build also reports the chunk size (SDL_mixer.def). */
extern DECLSPEC int SDLCALL Mix_QuerySpecEx(int *frequency, Uint16 *format, int *channels, int *chunksize);

#include "mugen.h"

#define SOUND_VOLUME_SCALE 1.28     /* percent to SDL_mixer volume (MIX_MAX_VOLUME = 128) */
#define SOUND_PAN_CENTER   0x7F

/* RIFF chunk ids, as read little endian. */
#define RIFF_ID_RIFF 0x46464952u    /* "RIFF" */
#define RIFF_ID_WAVE 0x45564157u    /* "WAVE" */
#define RIFF_ID_FMT  0x20746D66u    /* "fmt " */
#define RIFF_ID_DATA 0x61746164u    /* "data" */

/* Element index of an KeyedArray lookup, as computed inline by the original. */
static SndEntry *snd_entry(KeyedArray *map, int index)
{
    return (SndEntry *)((char *)map->data + map->elem_size * index);
}

/* Inlined WavReader close: readers only exist for PCM data. */
static void wav_reader_close(WavReader *r)
{
    if (r->info.format_tag == WAV_TAG_PCM) {
        free(r);
    }
}

/*
 * 0x0046E430: opens SDL_mixer. bits only has to be 8 or 16: 16-bit samples
 * are always requested. Returns 0 on failure.
 *
 * Quirks kept: the requested format, rate, channels and chunk size are
 * stored in the globals before the "already open" check; an unexpected
 * mixer format returns 0 with the mixer left open; when the slot table
 * cannot be allocated g_sndActive/g_sndOpen are already set.
 */
int Sound_Init(int buffer_log2, int channels, int bits, int mix_channels, int sample_rate, int resample_method,
               int resample_quality)
{
    char driver[51] = "(none)";
    int saved_error = g_libraryError;
    int freq, out_channels, chunk_size, out_bits;
    Uint16 format;
    int i;

    g_sndFormat.rate = (double)sample_rate;
    if (bits != 8 && bits != 16) {
        return 0;
    }
    g_sndFormat.format = AUDIO_FMT_S16;
    if (buffer_log2 > 15) {
        buffer_log2 = 15;
    } else if (buffer_log2 < 8) {
        buffer_log2 = 8;
    }
    g_sndChunkSize = 1 << buffer_log2;
    if (channels < 1) {
        channels = 1;
    } else if (channels > 2) {
        channels = 2;
    }
    g_sndFormat.channels = channels;
    if (g_sndOpen != 0) {
        return 0;
    }
    SDL_AudioDriverName(driver, 50);
    MatchLog_Printf(7, "Using SDL audio driver: %s\n", driver);
    MatchLog_Printf(7, "Requesting mixer parameters (%g, %d, %d, %d)...\n", g_sndFormat.rate, bits,
                    g_sndFormat.channels, g_sndChunkSize);
    if (Mix_OpenAudio((int)g_sndFormat.rate, AUDIO_S16LSB, g_sndFormat.channels, g_sndChunkSize) < 0) {
        g_libraryError = LIBERR_SOUND;
        MatchLog_Printf(3, "Couldn't open audio: %s\n", SDL_GetError());
        return 0;
    }
    Mix_QuerySpecEx(&freq, &format, &out_channels, &chunk_size);
    g_sndFormat.rate = (double)freq;
    g_sndFormat.channels = out_channels;
    g_sndChunkSize = chunk_size;
    if (format == AUDIO_U8) {
        out_bits = 8;
        g_sndFormat.format = AUDIO_FMT_U8;
    } else if (format == AUDIO_S16LSB) {
        out_bits = 16;
        g_sndFormat.format = AUDIO_FMT_S16;
    } else {
        return 0;
    }
    MatchLog_Printf(7, "Opened mixer with parameters (%g, %d, %d, %d)...\n", g_sndFormat.rate, out_bits,
                    out_channels, chunk_size);
    g_sndMixChannels = Mix_AllocateChannels(mix_channels);
    g_sndUnused346C = 0x7F;
    g_sndUnusedVolume = 0x7F;
    g_sndActive = 1;
    g_sndOpen = 1;
    g_sndSlotCount = SOUND_SLOT_COUNT;
    g_sndSlots = calloc(SOUND_SLOT_COUNT, sizeof(SoundSlot));
    if (g_sndSlots == NULL) {
        g_outOfMemory = 1;
        g_libraryError = LIBERR_SOUND;
        return 0;
    }
    for (i = 0; i < SOUND_SLOT_COUNT; i++) {
        g_sndSlots[i].channel = -1;
    }
    g_sndResampleMethod = resample_method;
    if (resample_method == SND_RESAMPLE_LIBRESAMPLE) {
        resample_init(resample_quality == 0, resample_quality == 1);
        /* The original calls it through the resampler table (0x4F1890 + 4). */
        g_sndResampler = Resampler_Create(&g_sndFormat, g_sndChunkSize, &resample_quality);
    }
    g_libraryError = saved_error;
    return 1;
}

/*
 * 0x0046E730: closes the mixer. As in the original, g_sndResampler is freed
 * but left pointing to the freed block, and g_sndMixChannels keeps its value.
 */
void Sound_Deinit(void)
{
    if (g_sndOpen == 0) {
        return;
    }
    Mix_CloseAudio();
    if (g_sndResampler != NULL) {
        Resampler *r = g_sndResampler;
        if (r->quality != NULL) {
            free(r->quality);
        }
        free(r);
    }
    if (g_sndResampleMethod == SND_RESAMPLE_LIBRESAMPLE) {
        resample_deinit();
    }
    memset(&g_sndFormat, 0, sizeof(g_sndFormat));
    g_sndOpen = 0;
    g_sndSlotCount = 0;
    if (g_sndSlots != NULL) {
        free(g_sndSlots);
        g_sndSlots = NULL;
    }
    g_sndActive = 0;
}

/*
 * 0x0046E7C0: an empty sound bank with room for capacity sounds.
 *
 * Quirk: when the map cannot be created, Snd_Free dereferences the NULL map.
 */
Snd *Snd_Create(int capacity)
{
    Snd *snd = calloc(1, sizeof(Snd));

    if (snd == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    snd->map = KeyedArray_Create(capacity, sizeof(SndEntry));
    if (snd->map == NULL) {
        Snd_Free(snd);
        return NULL;
    }
    return snd;
}

/*
 * 0x0046E810: adds chunk as sound group,number. The original's result test
 * is ((index != -1) != -1), so it always returns 1.
 */
int Snd_Add(Snd *snd, int group, int number, struct Mix_Chunk *chunk)
{
    SndEntry e;
    int added;

    e.unused_00 = 1;
    e.type = SND_ENTRY_CHUNK;
    e.unused_08 = 0;
    e.chunk = chunk;
    added = KeyedArray_Add(snd->map, &e, group, number) != -1;
    return added != -1;
}

/* 0x0046E870: frees a sound bank and its chunks (NULL is ignored). */
void Snd_Free(Snd *snd)
{
    KeyedArray *map;

    if (snd == NULL) {
        return;
    }
    map = snd->map;
    if (map->count != 0) {
        /* Inlined iteration over the used elements (0x466990 / 0x4669B0). */
        int i = map->first;
        SndEntry *e;

        map->iter = i;
        e = snd_entry(map, i);
        while (e != NULL) {
            if (e->type == SND_ENTRY_CHUNK && e->chunk != NULL) {
                Mix_Chunk *chunk = e->chunk;
                if (chunk->allocated == 0) {
                    free(chunk->abuf);
                }
                Mix_FreeChunk(chunk);
            }
            i = map->iter;
            if (i >= map->last) {
                break;
            }
            do {
                i++;
            } while (map->slots[i].used == 0);
            map->iter = i;
            e = snd_entry(map, i);
        }
    }
    if (map->data != NULL) {
        free(map->data);
    }
    if (map->slots != NULL) {
        free(map->slots);
    }
    free(map);
    free(snd);
}

/*
 * 0x0046E920: first slot that is not reserved and not playing, or -1.
 * g_sndActive is checked again on every iteration (the original reloads it
 * after each Mix_Playing call).
 */
int Sound_FindFreeSlot(void)
{
    int i;

    if (g_sndActive != 1) {
        return -1;
    }
    for (i = 0; i < g_sndSlotCount; i++) {
        if (g_sndActive != 1) {
            return i;
        }
        if (g_sndSlots[i].reserved == 0) {
            int channel = g_sndSlots[i].channel;
            if (channel == -1 || Mix_Playing(channel) == 0) {
                return i;
            }
        }
    }
    return -1;
}

/*
 * 0x0046E990: plays sound group,number of snd on slot at volume percent.
 * Returns the mixer channel, or -1.
 *
 * The volume only reaches Mix_Volume; the slot keeps the 50% set by
 * Sound_PrepareSlot (Sound_RestoreVolumes goes back to it).
 */
int Sound_PlayOnSlot(int slot, Snd *snd, int group, int number, double volume)
{
    int channel;
    KeyedArray *map;
    int index;

    if (g_sndActive != 1) {
        return -1;
    }
    channel = Sound_PrepareSlot(slot, snd, group, number);
    if (channel == -1) {
        return -1;
    }
    map = snd->map;
    index = KeyedArray_Find(map, group, number);
    if (index != -1) {
        SndEntry *e = snd_entry(map, index);
        if (e != NULL) {
            if (e->type == SND_ENTRY_CHUNK) {
                Mix_Volume(channel, g_sndMuted != 0 ? 0 : (int)(volume * SOUND_VOLUME_SCALE));
            }
            Mix_PlayChannelTimed(channel, e->chunk, g_sndSlots[slot].loop != 0 ? -1 : 0, -1);
        }
    }
    return channel;
}

/*
 * 0x0046EA50: gives slot the chunk of sound group,number and a mixer
 * channel (halting what it played), at 50% volume, centred and not looped.
 * Returns the channel, or -1.
 *
 * Quirk: the slot's chunk is replaced even when no free channel is found.
 */
int Sound_PrepareSlot(int slot, Snd *snd, int group, int number)
{
    KeyedArray *map;
    SndEntry *e;
    int index;
    int channel;

    if (g_sndActive != 1) {
        return -1;
    }
    Sound_ReleaseStopped();
    map = snd->map;
    index = KeyedArray_Find(map, group, number);
    if (index == -1) {
        return -1;
    }
    e = snd_entry(map, index);
    if (e == NULL) {
        return -1;
    }
    channel = g_sndSlots[slot].channel;
    g_sndSlots[slot].chunk = e->chunk;
    if (channel == -1) {
        channel = Sound_FindFreeChannel();
        if (channel == -1) {
            return -1;
        }
        g_sndSlots[slot].channel = channel;
    } else {
        Mix_HaltChannel(channel);
    }
    Sound_SetSlotVolume(slot, 50.0);
    Sound_SetSlotPan(slot, 0);
    /* Inlined Sound_SetSlotLoop(slot, 0). */
    if (g_sndActive == 1 && g_sndSlots[slot].channel != -1) {
        g_sndSlots[slot].loop = 0;
    }
    return channel;
}

/* 0x0046EB20: starts the sound prepared on slot. */
void Sound_PlaySlot(int slot)
{
    int channel;

    if (g_sndActive != 1) {
        return;
    }
    channel = g_sndSlots[slot].channel;
    if (channel == -1) {
        return;
    }
    Mix_Volume(channel, g_sndMuted != 0 ? 0 : g_sndSlots[slot].mix_volume);
    Mix_PlayChannelTimed(channel, g_sndSlots[slot].chunk, g_sndSlots[slot].loop != 0 ? -1 : 0, -1);
}

/* 0x0046EB80: sets the volume of slot (percent, clamped to 0..100; NaN is kept). */
void Sound_SetSlotVolume(int slot, double volume)
{
    SoundSlot *s;
    double v;
    int mix_volume;

    if (g_sndActive != 1) {
        return;
    }
    if (volume > 100.0) {
        v = 100.0;
    } else if (volume < 0.0) {
        v = 0.0;
    } else {
        v = volume;
    }
    s = &g_sndSlots[slot];
    s->volume = v;
    mix_volume = (int)(v * SOUND_VOLUME_SCALE);
    s->mix_volume = mix_volume;
    if (s->channel != -1) {
        Mix_Volume(s->channel, g_sndMuted != 0 ? 0 : mix_volume);
    }
}

/* 0x0046EBF0: re-applies the slot volumes (or silence while muted) to the playing channels. */
void Sound_RestoreVolumes(void)
{
    int i;

    for (i = 0; i < g_sndSlotCount; i++) {
        int channel = g_sndSlots[i].channel;
        if (channel != -1 && Mix_Playing(channel) != 0) {
            Mix_Volume(channel, g_sndMuted != 0 ? 0 : g_sndSlots[i].mix_volume);
        }
    }
}

/*
 * 0x0046EC50: pans slot; pan is -127 (left) .. 127 (right), mirrored by
 * g_stereoDirection. Quirk: a right level of 0xFE becomes 0xFF.
 */
void Sound_SetSlotPan(int slot, int pan)
{
    int right;
    int channel;

    if (g_sndActive != 1) {
        return;
    }
    right = g_stereoDirection * pan + SOUND_PAN_CENTER;
    if (right >= 0xFE) {
        right = 0xFF;
    } else if (right < 0) {
        right = 0;
    }
    channel = g_sndSlots[slot].channel;
    if (channel != -1) {
        Mix_SetPanning(channel, (Uint8)(0xFF - right), (Uint8)right);
    }
}

/* 0x0046ECA0 */
void Sound_SetSlotLoop(int slot, int loop)
{
    if (g_sndActive == 1 && g_sndSlots[slot].channel != -1) {
        g_sndSlots[slot].loop = loop;
    }
}

/* 0x0046ECC0: frees the slots whose sound has finished. */
void Sound_ReleaseStopped(void)
{
    int i;

    if (g_sndActive != 1) {
        return;
    }
    for (i = 0; i < g_sndSlotCount; i++) {
        int channel = g_sndSlots[i].channel;
        if (channel != -1 && g_sndSlots[i].reserved == 0 && Mix_Playing(channel) == 0) {
            g_sndSlots[i].channel = -1;
            g_sndSlots[i].owner = -1;
            g_sndSlots[i].chunk = NULL;
        }
    }
}

/*
 * 0x0046ED30: plays sound group,number of snd for owner id (-1: any free
 * slot) at volume percent. Returns the slot, or -1.
 */
int Sound_Play(int id, Snd *snd, int group, int number, double volume)
{
    int slot;

    if (g_sndActive != 1) {
        return -1;
    }
    slot = Sound_GetSlot(id);
    if (slot != -1) {
        if (Sound_PlayOnSlot(slot, snd, group, number, volume) == -1) {
            return -1;
        }
        g_sndSlots[slot].owner = id;
    }
    return slot;
}

/* 0x0046ED90: like Sound_Play, but only prepares the slot (see Sound_PlaySlot). */
int Sound_Prepare(int id, Snd *snd, int group, int number)
{
    int slot;

    if (g_sndActive != 1) {
        return -1;
    }
    slot = Sound_GetSlot(id);
    if (slot != -1) {
        if (Sound_PrepareSlot(slot, snd, group, number) == -1) {
            return -1;
        }
        g_sndSlots[slot].owner = id;
    }
    return slot;
}

/*
 * 0x0046EDE0: stops the sound of owner id, or every channel for -1.
 * Quirk: the slot's owner becomes 0, not -1.
 */
void Sound_Stop(int id)
{
    int slot;

    if (g_sndActive != 1) {
        return;
    }
    if (id == -1) {
        Mix_HaltChannel(-1);
        return;
    }
    slot = Sound_FindSlot(id);
    if (slot != -1) {
        SoundSlot *s = &g_sndSlots[slot];
        s->owner = 0;
        if (s->channel != -1) {
            Mix_HaltChannel(s->channel);
        }
    }
}

/* 0x0046EE30: 1 if owner id has a sound playing; -1 when sound is inactive. */
int Sound_IsPlaying(int id)
{
    int i;

    if (g_sndActive != 1) {
        return -1;
    }
    if (id == -1) {
        return 0;
    }
    for (i = 0; i < g_sndSlotCount; i++) {
        if (g_sndSlots[i].owner == id) {
            int channel = g_sndSlots[i].channel;
            if (channel == -1) {
                return 0;
            }
            return Mix_Playing(channel) != 0;
        }
    }
    return 0;
}

/* 0x0046EE90: slot of owner id, or -1. */
int Sound_FindSlot(int id)
{
    int i;

    if (g_sndActive != 1 || id == -1) {
        return -1;
    }
    for (i = 0; i < g_sndSlotCount; i++) {
        if (g_sndSlots[i].owner == id) {
            return i;
        }
    }
    return -1;
}

/* 0x0046EEC0: slot of owner id, else a free slot (Sound_FindFreeSlot). */
int Sound_GetSlot(int id)
{
    int i;

    if (g_sndActive == 1 && id != -1) {
        for (i = 0; i < g_sndSlotCount; i++) {
            if (g_sndSlots[i].owner == id) {
                return i;
            }
        }
    }
    return Sound_FindFreeSlot();
}

/* 0x0046EF00: first mixer channel that is not playing, or -1. */
int Sound_FindFreeChannel(void)
{
    int i;

    for (i = 0; i < g_sndMixChannels; i++) {
        if (Mix_Playing(i) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * 0x0046EF30: loads a .snd bank, or a .wav as sound 0,0. With [Misc]
 * BufferedRead the file is read into memory first.
 *
 * Quirks kept: other file types return NULL without closing the stream; a
 * .wav that fails to decode still yields an (empty) bank, and its status is
 * left uninitialised when Snd_DecodeWav does not set it.
 */
Snd *Snd_Load(const char *path)
{
    Stream *s;
    Snd *snd;
    int status;
    struct Mix_Chunk *chunk;

    if (g_sndBufferedRead == 0) {
        s = Stream_OpenFile(path, "rb");
        if (s == NULL) {
            return NULL;
        }
    } else {
        FILE *fp = fopen(path, "rb");
        long pos, size;
        void *buf;

        if (fp == NULL) {
            return NULL;
        }
        pos = ftell(fp);
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, pos, SEEK_SET);
        buf = malloc(size);
        /* fread returns 0 or 1 here, never -1: read errors go unnoticed. */
        if (buf == NULL || fread(buf, size, 1, fp) == (size_t)-1) {
            fclose(fp);
            return NULL;
        }
        fclose(fp);
        s = Stream_OpenMemory(buf, size);
        if (s == NULL) {
            free(buf);
            return NULL;
        }
        s->owns_buf = 1;
    }
    Stream_Seek(s, 0, SEEK_SET);
    switch (File_GetType(path)) {
    case FILETYPE_WAV:
        snd = Snd_Create(1);
        if (snd == NULL) {
            Stream_Close(s);
            return NULL;
        }
        chunk = Snd_DecodeWav(s, &status);
        if (chunk == NULL) {
            MatchLog_Printf(5, "Error loading wave file <%s>: %s\n", path, Wav_StrError(status));
        } else if (!Snd_Add(snd, 0, 0, chunk)) {
            Stream_Close(s);
            Snd_Free(snd);
            return NULL;
        }
        Stream_Close(s);
        return snd;
    case FILETYPE_SND:
        snd = Snd_Parse(s, &status);
        if (snd == NULL) {
            Stream_Close(s);
            return NULL;
        }
        if (status == SND_ERR_IGNORED) {
            MatchLog_Printf(5, "Ignored error(s) while loading SND file: %s\n", path);
        } else if (status != SND_OK) {
            MatchLog_Printf(5, "Invalid status code %d returned while loading SND file (engine's fault): %s\n",
                            status, path);
        }
        Stream_Close(s);
        return snd;
    default:
        return NULL;
    }
}

/* 0x0046F120: decodes the WAV data of s with the configured resample method (NULL for unknown methods). */
struct Mix_Chunk *Snd_DecodeWav(Stream *s, int *status)
{
    if (g_sndResampleMethod == SND_RESAMPLE_SDL) {
        return Snd_DecodeWavSDL(s, status);
    }
    if (g_sndResampleMethod == SND_RESAMPLE_LIBRESAMPLE) {
        return Snd_DecodeWavResampled(s, status);
    }
    return NULL;
}

/*
 * 0x0046F150: decodes the WAV data of s to the mixer format with the WAV
 * reader and g_sndResampler. Sets *status on the failures the WAV reader
 * reports (and to 0 for an empty sound), but not on success.
 *
 * The output buffer holds ceil(frames * ratio) + 2 * ceil(ratio) frames;
 * reaching that size logs a warning, which repeats forever if the resampler
 * still has output (as in the original).
 */
struct Mix_Chunk *Snd_DecodeWavResampled(Stream *s, int *status)
{
    long start = Stream_Tell(s);
    WavInfo info;
    int err;
    double ratio;
    WavReader *reader;
    unsigned int in_frame, out_frame;
    unsigned int estimate, capacity;
    unsigned int written = 0;
    unsigned int last = 0;          /* frames of the last read from the resampler or the WAV reader */
    unsigned char *out_buf, *in_buf;
    ResamplerSession *session;
    int state;
    unsigned int size;
    void *data;

    err = Wav_ReadInfo(s, &info);
    if (err != WAV_OK) {
        if (status != NULL) {
            *status = err;
        }
        return NULL;
    }
    if (info.frames == 0) {
        struct Mix_Chunk *chunk = Mix_QuickLoad_RAW(NULL, 0);
        if (status != NULL) {
            *status = WAV_OK;
        }
        return chunk;
    }
    ratio = g_sndFormat.rate / info.format.rate;
    reader = WavReader_Open(&info, s, &err);
    if (reader == NULL) {
        if (status != NULL) {
            *status = err;
        }
        return NULL;
    }
    in_frame = (unsigned int)Wav_FrameSize(info.format.format, info.format.channels);
    out_frame = (unsigned int)Wav_FrameSize(g_sndFormat.format, g_sndFormat.channels);
    estimate = (unsigned int)(__int64)ceil((double)info.frames * ratio);
    capacity = (unsigned int)(__int64)((double)estimate + (ceil(ratio) + ceil(ratio)));
    out_buf = calloc(capacity, out_frame);
    if (out_buf == NULL) {
        g_outOfMemory = 1;
        wav_reader_close(reader);
        return NULL;
    }
    /* Not checked for NULL in the original either. */
    session = g_sndResampler->begin(g_sndResampler, &info);
    in_buf = calloc(session->vtbl->in_block(session), in_frame);
    if (in_buf == NULL) {
        g_outOfMemory = 1;
        wav_reader_close(reader);
        free(out_buf);
        session->vtbl->close(session);
        return NULL;
    }
    state = session->vtbl->state(session);
    while (state != RESAMPLER_DONE && written <= capacity) {
        if (state == RESAMPLER_OUTPUT) {
            if (written == capacity) {
                MatchLog_Printf(3, "Warning: Resampled PCM data exceeds estimated length %ld (engine's fault); "
                                "truncating...\n", capacity);
            } else {
                last = session->vtbl->read(session, out_buf + out_frame * written, capacity - written);
                written += last;
            }
        } else if (state == RESAMPLER_INPUT) {
            unsigned int need = session->vtbl->input_needed(session);
            unsigned int got = 0;
            while (got < need) {
                last = WavReader_Read(reader, need - got, in_buf + got * in_frame);
                if (last == 0) {
                    break;
                }
                got += last;
            }
            /* With need == 0, "last" still holds the previous count. */
            session->vtbl->write(session, in_buf, got, last == 0);
        }
        state = session->vtbl->state(session);
    }
    session->vtbl->close(session);
    wav_reader_close(reader);
    free(in_buf);
    Stream_Seek(s, start, SEEK_SET);
    size = out_frame * written;
    data = realloc(out_buf, size);
    if (data == NULL) {
        return NULL;
    }
    return Mix_QuickLoad_RAW(data, size);
}

/*
 * 0x0046F520: decodes the WAV data of s with SDL_mixer, retrying with a
 * cleaned-up copy (Snd_RepairWav) when SDL rejects it. Only file and plain
 * memory streams are supported. *status is 0 even when SDL fails both times.
 *
 * Quirks kept: the first RWops leaks when the repaired copy is used, and
 * both leak when the copy cannot be made; the RWops of the copy is freed
 * after its stream is closed.
 */
struct Mix_Chunk *Snd_DecodeWavSDL(Stream *s, int *status)
{
    long start = Stream_Tell(s);
    SDL_RWops *rw;
    Mix_Chunk *chunk;

    if (s->type == STREAM_FILE) {
        rw = SDL_RWFromFP(s->fp, 0);
    } else if (s->type == STREAM_MEMORY) {
        int offset = (int)(s->pos - s->begin);
        long size = Stream_GetSize(s);
        rw = SDL_RWFromMem(s->pos, size - offset);
    } else {
        goto fail;
    }
    if (rw == NULL) {
        goto fail;
    }
    chunk = Mix_LoadWAV_RW(rw, 0);
    if (chunk == NULL) {
        Stream *fixed = Snd_RepairWav(s);
        if (fixed == NULL) {
            goto fail;
        }
        rw = Stream_ToSDLRWops(fixed);
        if (rw == NULL) {
            goto fail;
        }
        chunk = Mix_LoadWAV_RW(rw, 0);
        Stream_Close(fixed);
    }
    SDL_FreeRW(rw);
    Stream_Seek(s, start, SEEK_SET);
    if (status != NULL) {
        *status = WAV_OK;
    }
    return chunk;

fail:
    if (status != NULL) {
        *status = WAV_ERR_UNKNOWN;
    }
    return NULL;
}

/*
 * 0x0046F620: reads a .snd bank from s. *status gets SND_OK, SND_ERR_IGNORED
 * when some sounds could not be decoded (they are logged and skipped), or
 * SND_ERR_FAILED with a NULL result.
 */
Snd *Snd_Parse(Stream *s, int *status)
{
    SndFileHeader header;
    SndSubHeader sub;
    Snd *snd = NULL;
    int result = SND_OK;
    int wav_status = 0;
    long start = Stream_Tell(s);
    int offset;
    int i;

    if (Stream_Read(&header, sizeof(header), 1, s) != 1 || strcmp(header.signature, SND_SIGNATURE) != 0) {
        goto fail;
    }
    snd = Snd_Create(header.count);
    if (snd == NULL) {
        goto fail;
    }
    offset = header.first;
    for (i = 0; i < header.count; i++) {
        unsigned char *buf;
        Stream *ms;
        struct Mix_Chunk *chunk;

        Stream_Seek(s, offset + start, SEEK_SET);
        if (Stream_Read(&sub, sizeof(sub), 1, s) != 1) {
            goto fail;
        }
        buf = malloc(sub.size);
        if (buf == NULL) {
            goto fail;
        }
        /* Never -1: a short read goes unnoticed. */
        if (Stream_Read(buf, sub.size, 1, s) == (size_t)-1 || (ms = Stream_OpenMemory(buf, sub.size)) == NULL) {
            free(buf);
            goto fail;
        }
        ms->owns_buf = 1;
        chunk = Snd_DecodeWav(ms, &wav_status);
        if (chunk == NULL) {
            MatchLog_Printf(5, "ERROR loading wave file %d, %d: %s\n", sub.group, sub.number,
                            Wav_StrError(wav_status));
            result = SND_ERR_IGNORED;
        }
        Stream_Close(ms);
        if (chunk != NULL && !Snd_Add(snd, sub.group, sub.number, chunk)) {
            goto fail;
        }
        offset = sub.next;
    }
    Stream_Seek(s, start, SEEK_SET);
    if (status != NULL) {
        *status = result;
    }
    return snd;

fail:
    Snd_Free(snd);
    if (status != NULL) {
        *status = SND_ERR_FAILED;
    }
    return NULL;
}

/*
 * 0x0046F860: copies the WAV data of s into a new memory stream with only
 * the RIFF header, the "fmt " chunk and the "data" chunk, adding one frame of
 * silence when there is no sample data. NULL for non-PCM data or on errors.
 *
 * Quirks kept: the Wav_ReadInfo result is ignored, so after a failure the
 * (uninitialised) info decides; without a "fmt " chunk its offset and size
 * become the file size; the silent U8 frame is always two bytes, one past
 * the buffer for mono.
 */
Stream *Snd_RepairWav(Stream *s)
{
    unsigned char *data = NULL;
    int file_size = Stream_GetSize(s);
    int pad = 0;
    long start = Stream_Tell(s);
    WavInfo info;
    int pos;
    int fmt_off = file_size;
    unsigned int fmt_len = (unsigned int)file_size;
    unsigned int id, len;
    unsigned int total;
    unsigned int at;
    Stream *out;

    Wav_ReadInfo(s, &info);
    if (info.format_tag != WAV_TAG_PCM) {
        return NULL;
    }
    Stream_Seek(s, 12, SEEK_SET);
    for (pos = 12; pos < file_size;) {
        /* Every iteration reads both ids, even when the first read fails. */
        int r1 = Stream_ReadU32(s, &id);
        int r2 = Stream_ReadU32(s, &len);
        if (r1 != 0 || r2 != 0) {
            goto fail;
        }
        if (id == RIFF_ID_FMT) {
            fmt_off = pos + 8;
            fmt_len = len;
            break;
        }
        /* Inlined Stream_Seek(s, len, SEEK_CUR). */
        Stream_Seek(s, (long)len, SEEK_CUR);
        pos = (int)len + pos + 8;
        if (pos % 2 == 1) {
            Stream_Seek(s, 1, SEEK_CUR);
            pos++;
        }
    }
    total = info.data_size + fmt_len + 0x1C;
    if (info.frames == 0) {
        pad = 1;
        total += Wav_FrameSize(info.format.format, info.format.channels);
    }
    data = malloc(total);
    if (data == NULL) {
        return NULL;
    }
    out = Stream_OpenMemory(data, total);
    if (out == NULL) {
        goto fail;
    }
    out->owns_buf = 1;
    ((unsigned int *)data)[0] = RIFF_ID_RIFF;
    ((unsigned int *)data)[1] = total - 8;
    ((unsigned int *)data)[2] = RIFF_ID_WAVE;
    Stream_Seek(s, fmt_off - 8, SEEK_SET);
    Stream_Read(data + 12, 1, fmt_len + 8, s);
    Stream_Seek(s, info.data_offset, SEEK_SET);
    at = fmt_len + 0x14;
    *(unsigned int *)(data + at + 4) =
        (pad ? (unsigned int)Wav_FrameSize(info.format.format, info.format.channels) : 0) + info.data_size;
    *(unsigned int *)(data + at) = RIFF_ID_DATA;
    at += 8;
    Stream_Read(data + at, 1, info.data_size, s);
    at += info.data_size;
    if (pad) {
        if (info.format.format == AUDIO_FMT_U8) {
            data[at] = 0x80;
            data[at + 1] = 0x80;
        } else if (info.format.format == AUDIO_FMT_S16) {
            memset(data + at, 0, info.format.channels * 2);
        }
    }
    Stream_Seek(s, start, SEEK_SET);
    return out;

fail:
    Stream_Seek(s, start, SEEK_SET);
    if (data != NULL) {
        free(data);
    }
    return NULL;
}
