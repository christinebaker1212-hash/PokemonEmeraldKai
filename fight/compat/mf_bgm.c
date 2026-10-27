/*
 * Moemon Fight: the "moemon" BGM plugin, which plays every song of the fight: MP3 (dr_mp3), Ogg
 * Vorbis (stb_vorbis), FLAC (dr_flac) and WAV (dr_wav), with MUGEN's loop points (in samples of
 * the file). The engine's own mpg123 plugin needs a DLL and the sdlmix one refuses these
 * formats; the host makes this the only plugin (MfHost_ConfigLoaded). The decoders hand out
 * 16-bit frames at the file's rate; the BGM thread resamples them to the mixer's 13379 Hz.
 */
#define MF_SYSTEM_HEAP  /* Moemon Fight: the music thread's memory, outside the fight's heap (mf_mem.h) */
#include "mugen.h"

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include "dr_mp3.h"
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#include "dr_flac.h"
#define DR_WAV_IMPLEMENTATION
#define DR_WAV_NO_STDIO
#include "dr_wav.h"
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis.c"

enum {
    MFBGM_NONE,
    MFBGM_MP3,
    MFBGM_OGG,
    MFBGM_FLAC,
    MFBGM_WAV
};

typedef struct MfBgm {
    int kind;                   /* MFBGM_* */
    unsigned char *data;        /* the whole file */
    size_t size;
    drmp3 mp3;
    stb_vorbis *ogg;
    drflac *flac;
    drwav wav;
    int channels;               /* of the output: 1 or 2 */
    int rate;
    int loop;
    int loop_start;             /* frames */
    int loop_end;               /* frames; 0x7FFFFFFF: the end of the file */
    int pos;                    /* frames decoded */
    int playing;
    double volume;
    BgmEndFn end_callback;
    void *end_arg;
} MfBgm;

static unsigned int __cdecl MfBgm_Version(void)
{
    return 0x01000000u;
}

static const char *__cdecl MfBgm_Extensions(void)
{
    return "mp3,ogg,flac,wav";
}

static int __cdecl MfBgm_Init(const BgmPluginConfig *config, const BgmHostCallbacks *host, BgmSettingFn get_setting)
{
    return 0;
}

static void __cdecl MfBgm_Deinit(void)
{
}

static void *__cdecl MfBgm_Create(void)
{
    MfBgm *b = calloc(1, sizeof(MfBgm));

    if (b != NULL) {
        b->volume = 100.0;
    }
    return b;
}

static int __cdecl MfBgm_Close(void *instance)
{
    MfBgm *b = instance;

    if (b == NULL) {
        return 0;
    }
    switch (b->kind) {
    case MFBGM_MP3:
        drmp3_uninit(&b->mp3);
        break;
    case MFBGM_OGG:
        stb_vorbis_close(b->ogg);
        b->ogg = NULL;
        break;
    case MFBGM_FLAC:
        drflac_close(b->flac);
        b->flac = NULL;
        break;
    case MFBGM_WAV:
        drwav_uninit(&b->wav);
        break;
    }
    b->kind = MFBGM_NONE;
    free(b->data);
    b->data = NULL;
    b->size = 0;
    b->playing = 0;
    b->pos = 0;
    return 0;
}

static void __cdecl MfBgm_Destroy(void *instance)
{
    MfBgm_Close(instance);
    free(instance);
}

/* Reads the rest of the file into b->data. */
static int MfBgm_Slurp(MfBgm *b, const BgmPluginIo *io)
{
    size_t cap = 1 << 20;
    unsigned char *buf = malloc(cap);

    b->size = 0;
    if (buf == NULL) {
        return 0;
    }
    for (;;) {
        size_t n;
        if (b->size == cap) {
            unsigned char *grown = realloc(buf, cap * 2);
            if (grown == NULL) {
                free(buf);
                return 0;
            }
            buf = grown;
            cap *= 2;
        }
        n = io->read(buf + b->size, 1, cap - b->size, io->handle);
        if (n == 0) {
            break;
        }
        b->size += n;
    }
    b->data = buf;
    return b->size > 0;
}

static int MfBgm_Seek(MfBgm *b, int frame)
{
    int ok = 0;

    switch (b->kind) {
    case MFBGM_MP3:
        ok = drmp3_seek_to_pcm_frame(&b->mp3, (drmp3_uint64)frame);
        break;
    case MFBGM_OGG:
        ok = stb_vorbis_seek(b->ogg, (unsigned int)frame);
        break;
    case MFBGM_FLAC:
        ok = drflac_seek_to_pcm_frame(b->flac, (drflac_uint64)frame);
        break;
    case MFBGM_WAV:
        ok = drwav_seek_to_pcm_frame(&b->wav, (drwav_uint64)frame);
        break;
    }
    b->pos = frame;
    return ok;
}

/* Decodes up to `frames` frames of b->channels 16-bit samples; returns how many. */
static int MfBgm_Decode(MfBgm *b, short *out, int frames)
{
    switch (b->kind) {
    case MFBGM_MP3:
        return (int)drmp3_read_pcm_frames_s16(&b->mp3, (drmp3_uint64)frames, out);
    case MFBGM_OGG:
        return stb_vorbis_get_samples_short_interleaved(b->ogg, b->channels, out, frames * b->channels);
    case MFBGM_FLAC:
        return (int)drflac_read_pcm_frames_s16(b->flac, (drflac_uint64)frames, out);
    case MFBGM_WAV:
        return (int)drwav_read_pcm_frames_s16(&b->wav, (drwav_uint64)frames, out);
    }
    return 0;
}

static int __cdecl MfBgm_Open(void *instance, const BgmPluginIo *io, BgmInfo *info, BgmOpenResult *out)
{
    MfBgm *b = instance;
    const char *ext;
    int start, end;

    if (b == NULL) {
        return 4;
    }
    MfBgm_Close(b);
    if (!MfBgm_Slurp(b, io)) {
        return 2;
    }
    ext = strrchr(info->path, '.');
    ext = ext != NULL ? ext + 1 : "";
    if (_stricmp(ext, "ogg") == 0) {
        int err = 0;
        stb_vorbis_info vi;
        b->ogg = stb_vorbis_open_memory(b->data, (int)b->size, &err, NULL);
        if (b->ogg == NULL) {
            return 2;
        }
        vi = stb_vorbis_get_info(b->ogg);
        b->kind = MFBGM_OGG;
        b->rate = (int)vi.sample_rate;
        b->channels = vi.channels >= 2 ? 2 : 1;
    } else if (_stricmp(ext, "flac") == 0) {
        b->flac = drflac_open_memory(b->data, b->size, NULL);
        if (b->flac == NULL || b->flac->channels > 2) {
            return 2;
        }
        b->kind = MFBGM_FLAC;
        b->rate = (int)b->flac->sampleRate;
        b->channels = b->flac->channels;
    } else if (_stricmp(ext, "wav") == 0) {
        if (!drwav_init_memory(&b->wav, b->data, b->size, NULL) || b->wav.channels > 2) {
            return 2;
        }
        b->kind = MFBGM_WAV;
        b->rate = (int)b->wav.sampleRate;
        b->channels = b->wav.channels;
    } else {
        if (!drmp3_init_memory(&b->mp3, b->data, b->size, NULL)) {
            return 2;
        }
        b->kind = MFBGM_MP3;
        b->rate = (int)b->mp3.sampleRate;
        b->channels = (int)b->mp3.channels;
    }
    Log_Printf("BGM %s: %d Hz, %d channel(s), loop %d (%d-%d)\n", info->path, b->rate, b->channels, info->loop,
               info->loop_start, info->loop_end);
    out->mode = BGM_MODE_DECODED;
    out->format.rate = (double)b->rate;
    out->format.format = AUDIO_FMT_S16;
    out->format.channels = b->channels;
    b->loop = info->loop;
    start = info->loop_start <= -1 ? 0 : info->loop_start;
    end = info->loop_end;
    if (end != -1 && end <= start) {
        end = -1;
    }
    b->loop_start = start;
    b->loop_end = end < 0 ? 0x7FFFFFFF : end;
    b->pos = 0;
    return 0;
}

static int __cdecl MfBgm_Start(void *instance)
{
    MfBgm *b = instance;

    if (b == NULL || b->kind == MFBGM_NONE) {
        return 4;
    }
    b->playing = 1;
    return 0;
}

static int __cdecl MfBgm_Stop(void *instance)
{
    MfBgm *b = instance;

    if (b == NULL || b->kind == MFBGM_NONE) {
        return 4;
    }
    b->playing = 0;
    MfBgm_Seek(b, 0);
    return 0;
}

static int __cdecl MfBgm_Rewind(void *instance)
{
    MfBgm *b = instance;

    if (b == NULL || b->kind == MFBGM_NONE) {
        return 4;
    }
    MfBgm_Seek(b, 0);
    return 0;
}

static int __cdecl MfBgm_SeekTo(void *instance, int pos)
{
    MfBgm *b = instance;

    if (b == NULL || b->kind == MFBGM_NONE) {
        return 4;
    }
    MfBgm_Seek(b, pos);
    return 0;
}

static int __cdecl MfBgm_IsPlaying(void *instance)
{
    MfBgm *b = instance;

    return b != NULL && b->playing;
}

static double __cdecl MfBgm_GetVolume(void *instance)
{
    MfBgm *b = instance;

    return b != NULL ? b->volume : 0.0;
}

static int __cdecl MfBgm_SetVolume(void *instance, double volume)
{
    MfBgm *b = instance;

    if (b != NULL) {
        b->volume = volume;
    }
    return 0;
}

static int __cdecl MfBgm_GetLoopFlag(void *instance)
{
    MfBgm *b = instance;

    return b != NULL ? b->loop : 0;
}

static int __cdecl MfBgm_SetLoopFlag(void *instance, int loop)
{
    MfBgm *b = instance;

    if (b != NULL) {
        b->loop = loop;
    }
    return 0;
}

static int __cdecl MfBgm_GetLoopPoints(void *instance, int *start, int *end)
{
    MfBgm *b = instance;

    if (b == NULL) {
        return 4;
    }
    *start = b->loop_start;
    *end = b->loop_end == 0x7FFFFFFF ? -1 : b->loop_end;
    return 0;
}

static int __cdecl MfBgm_SetLoopPoints(void *instance, int start, int end)
{
    MfBgm *b = instance;

    if (b == NULL) {
        return 4;
    }
    if (start < 0) {
        start = 0;
    }
    if (end != -1 && end <= start) {
        end = -1;
    }
    b->loop_start = start;
    b->loop_end = end < 0 ? 0x7FFFFFFF : end;
    return 0;
}

static void __cdecl MfBgm_SetEndCallback(void *instance, BgmEndFn fn, void *arg)
{
    MfBgm *b = instance;

    if (b != NULL) {
        b->end_callback = fn;
        b->end_arg = arg;
    }
}

static const char *__cdecl MfBgm_StrError(void *instance, int err)
{
    switch (err) {
    case 0:
        return "no error";
    case 2:
        return "can't decode the file";
    case 4:
        return "no file open";
    default:
        return "error";
    }
}

/* Fills buf with up to size bytes of frames; loops back to the loop start at the loop end (or the end
 * of the file, which then becomes the loop end), else reports the end. */
static int __cdecl MfBgm_Read(void *instance, void *buf, unsigned int size, unsigned int *got, int *event)
{
    MfBgm *b = instance;
    int frame_bytes, frames_left;
    unsigned int total = 0;
    int ended = 0;

    if (b == NULL) {
        return 4;
    }
    *got = 0;
    if (b->kind == MFBGM_NONE) {
        return 4;
    }
    if (!b->playing) {
        if (event != NULL) {
            *event = BGM_READ_EVENT_END;
        }
        return 0;
    }
    if (event != NULL) {
        *event = BGM_READ_EVENT_BLOCK;
    }
    frame_bytes = b->channels * 2;
    frames_left = (int)(size / (unsigned int)frame_bytes);
    while (frames_left > 0) {
        int want = frames_left;
        int n;
        if (b->loop && b->loop_end - b->pos < want) {
            want = b->loop_end - b->pos;
        }
        n = want > 0 ? MfBgm_Decode(b, (short *)((unsigned char *)buf + total), want) : 0;
        if (n > 0) {
            total += (unsigned int)(n * frame_bytes);
            frames_left -= n;
            b->pos += n;
        }
        if (n < want || b->pos >= b->loop_end) {
            /* the end of the file or of the loop */
            if (!b->loop) {
                ended = 1;
                b->playing = 0;
                if (event != NULL) {
                    *event = BGM_READ_EVENT_END;
                }
                break;
            }
            if (n < want && b->pos < b->loop_end) {
                b->loop_end = b->pos;
                if (b->loop_start >= b->pos) {
                    b->loop_start = 0;
                }
            }
            if (b->loop_end <= b->loop_start || !MfBgm_Seek(b, b->loop_start)) {
                ended = 1;
                b->playing = 0;
                break;
            }
        }
    }
    *got = total;
    if (ended && b->end_callback != NULL) {
        b->end_callback(b->end_arg);
    }
    return 0;
}

static int __cdecl MfBgm_GetFormat(void *instance, AudioFormat *format)
{
    MfBgm *b = instance;

    if (b == NULL) {
        return 4;
    }
    format->rate = (double)b->rate;
    format->format = AUDIO_FMT_S16;
    format->channels = b->channels;
    return 0;
}

static int __cdecl MfBgm_Nothing(void *instance)
{
    return 0;
}

static int __cdecl MfBgm_SetMute(void *instance, int muted)
{
    return 0;
}

const BgmSubsystem g_mfBgmSubsystem = {
    MfBgm_Version,
    MfBgm_Extensions,
    MfBgm_Init,
    MfBgm_Deinit,
    MfBgm_Create,
    MfBgm_Destroy,
    (void *)MfBgm_IsPlaying,
    MfBgm_Start,
    MfBgm_Stop,
    (void *)MfBgm_Rewind,
    (void *)MfBgm_SeekTo,
    MfBgm_Open,
    MfBgm_Close,
    (void *)MfBgm_GetVolume,
    MfBgm_SetVolume,
    (void *)MfBgm_GetLoopFlag,
    (void *)MfBgm_SetLoopFlag,
    (void *)MfBgm_GetLoopPoints,
    (void *)MfBgm_SetLoopPoints,
    (void *)MfBgm_SetEndCallback,
    MfBgm_StrError,
    MfBgm_Read,
    MfBgm_GetFormat,
    MfBgm_Nothing,
    (void *)MfBgm_Nothing,
    MfBgm_Nothing,
    MfBgm_Nothing,
    MfBgm_SetMute,
    (void *)MfBgm_Nothing,
    NULL,
};
