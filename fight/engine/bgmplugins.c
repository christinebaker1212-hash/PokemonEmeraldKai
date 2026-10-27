/*
 * The built-in BGM plugins (original module 0x48AB60-0x48C3DF), whose
 * function tables are g_bgmMpg123Subsystem and g_bgmSdlmixSubsystem:
 *
 * - "mpg123": MP3 decoding with libmpg123-0.dll, loaded at init with
 *   LoadLibrary / GetProcAddress (mpg123 itself is not linked into
 *   mugen.exe). Data is fed to mpg123 from the plugin's file callbacks
 *   (BgmPluginIo) in readbuffersize blocks; decoded data goes to the stream
 *   loader (BGM_MODE_DECODED), with loop points in output frames (or in
 *   source samples converted to output frames with "ntom").
 * - "sdlmix": tracker and MIDI files played by SDL_mixer's music player
 *   (BGM_MODE_PLUGIN); one instance at a time.
 *
 * Plugin settings are read with BgmSettings_Get directly (not through the
 * accessor given to init).
 */
#include <SDL.h>
#include <SDL_mixer.h>

#include "mugen.h"

/* mpg123 constants used here (mpg123.h). */
#define MPG123_RVA              5
#define MPG123_NEW_FORMAT       (-11)
#define MPG123_NEED_MORE        (-10)
#define MPG123_DONE             (-12)
#define MPG123_ENC_UNSIGNED_8   0x01
#define MPG123_ENC_SIGNED_16    0xD0
#define MPG123_ENC_FLOAT_32     0x200
#define MPG123_MONO             1
#define MPG123_STEREO           2

/* ------------------------------------------------------------------ mpg123 */

/* 0x0048AB60: resolves the libmpg123 functions. Returns 0 when all were found. */
int Mpg123_LoadSymbols(void)
{
    HMODULE lib = g_mpg123Lib;

    g_mpg123_init = (Mpg123InitFn)GetProcAddress(lib, "mpg123_init");
    g_mpg123_format = (Mpg123FormatFn)GetProcAddress(g_mpg123Lib, "mpg123_format");
    g_mpg123_param = (Mpg123ParamFn)GetProcAddress(g_mpg123Lib, "mpg123_param");
    g_mpg123_new = (Mpg123NewFn)GetProcAddress(g_mpg123Lib, "mpg123_new");
    g_mpg123_exit = (Mpg123ExitFn)GetProcAddress(g_mpg123Lib, "mpg123_exit");
    g_mpg123_delete = (Mpg123HandleFn)GetProcAddress(g_mpg123Lib, "mpg123_delete");
    g_mpg123_getformat = (Mpg123GetFormatFn)GetProcAddress(g_mpg123Lib, "mpg123_getformat");
    g_mpg123_read = (Mpg123ReadFn)GetProcAddress(g_mpg123Lib, "mpg123_read");
    g_mpg123_open_feed = (Mpg123HandleIntFn)GetProcAddress(g_mpg123Lib, "mpg123_open_feed");
    g_mpg123_close = (Mpg123HandleIntFn)GetProcAddress(g_mpg123Lib, "mpg123_close");
    g_mpg123_feed = (Mpg123FeedFn)GetProcAddress(g_mpg123Lib, "mpg123_feed");
    g_mpg123_feedseek = (Mpg123FeedSeekFn)GetProcAddress(g_mpg123Lib, "mpg123_feedseek");
    g_mpg123_plain_strerror = (Mpg123PlainStrErrorFn)GetProcAddress(g_mpg123Lib, "mpg123_plain_strerror");
    g_mpg123_format_none = (Mpg123HandleIntFn)GetProcAddress(g_mpg123Lib, "mpg123_format_none");
    g_mpg123_getstate = (void *)GetProcAddress(g_mpg123Lib, "mpg123_getstate");
    g_mpg123_current_decoder = (void *)GetProcAddress(g_mpg123Lib, "mpg123_current_decoder");
    g_mpg123_getvolume = (void *)GetProcAddress(g_mpg123Lib, "mpg123_getvolume");
    g_mpg123_volume = (Mpg123VolumeFn)GetProcAddress(g_mpg123Lib, "mpg123_volume");
    g_mpg123_info = (Mpg123InfoFn)GetProcAddress(g_mpg123Lib, "mpg123_info");
    g_mpg123_getparam = (void *)GetProcAddress(g_mpg123Lib, "mpg123_getparam");
    g_mpg123_strerror = (Mpg123StrErrorFn)GetProcAddress(g_mpg123Lib, "mpg123_strerror");
    (void)lib;
    if (g_mpg123_init != NULL && g_mpg123_format != NULL && g_mpg123_param != NULL && g_mpg123_new != NULL &&
        g_mpg123_exit != NULL && g_mpg123_delete != NULL && g_mpg123_getformat != NULL && g_mpg123_read != NULL &&
        g_mpg123_open_feed != NULL && g_mpg123_close != NULL && g_mpg123_feedseek != NULL && g_mpg123_feed != NULL &&
        g_mpg123_plain_strerror != NULL && g_mpg123_format_none != NULL && g_mpg123_getstate != NULL &&
        g_mpg123_current_decoder != NULL && g_mpg123_getvolume != NULL && g_mpg123_volume != NULL &&
        g_mpg123_info != NULL && g_mpg123_getparam != NULL && g_mpg123_strerror != NULL) {
        return 0;
    }
    return 1;
}

/* 0x0048ADD0: version of both built-in plugins (1.0.0.0). */
unsigned int Bgm_BuiltinVersion(void)
{
    return 0x01000000;
}

/* 0x0048ADE0: the mpg123 plugin's default extension list. */
const char *Mpg123_Extensions(void)
{
    return "mp3";
}

/*
 * 0x0048ADF0: restricts mh's output to the mixer format (any channel count
 * for 0); returns 3 for formats mpg123 cannot output, else mpg123's result.
 */
int Mpg123_SetupFormat(void *mh)
{
    int bad = 0;
    int channels;
    int encoding;

    g_mpg123_format_none(mh);
    switch (g_mpg123Format.channels) {
    case 0:
        channels = MPG123_MONO | MPG123_STEREO;
        break;
    case 1:
        channels = MPG123_MONO;
        break;
    case 2:
        channels = MPG123_STEREO;
        break;
    default:
        bad = 3;
        channels = 3;
        break;
    }
    if (g_mpg123Format.format == AUDIO_FMT_U8) {
        encoding = MPG123_ENC_UNSIGNED_8;
    } else if (g_mpg123Format.format == AUDIO_FMT_S16) {
        encoding = MPG123_ENC_SIGNED_16;
    } else {
        return 3;
    }
    if (bad != 0) {
        return bad;
    }
    return g_mpg123_format(mh, (long)(int)g_mpg123Format.rate, channels, encoding);
}

/* 0x0048AE80: clears inst and gives it an mpg123 handle (decoder and RVA settings). */
int Mpg123_NewHandle(Mpg123Instance *inst)
{
    void *mh;

    memset(inst, 0, offsetof(Mpg123Instance, readbuf) + 4);
    mh = g_mpg123_new(g_mpg123Decoder[0] != '\0' ? g_mpg123Decoder : NULL, &inst->last_error);
    inst->mh = mh;
    if (mh == NULL) {
        return 2;
    }
    return g_mpg123_param(mh, MPG123_RVA, g_mpg123Rva, 0.0);
}

/* 0x0048AEE0: a new instance with its read buffer, NULL before init succeeded. */
void *Mpg123_Create(void)
{
    Mpg123Instance *inst;

    if (!g_mpg123Initialized) {
        return NULL;
    }
    inst = calloc(1, g_mpg123ReadBufferSize + offsetof(Mpg123Instance, readbuf) + 4);
    if (inst == NULL) {
        g_outOfMemory = 1;
        free(inst);
        return NULL;
    }
    if (Mpg123_NewHandle(inst) != 0) {
        free(inst);
        return NULL;
    }
    return inst;
}

/* 0x0048AF30: closes and frees inst. Quirk: NULL is dereferenced. */
void Mpg123_Destroy(void *instance)
{
    Mpg123Instance *inst = instance;

    if (inst != NULL && inst->mh != NULL) {
        g_mpg123_close(inst->mh);
        inst->io.handle = NULL;
        inst->playing = 0;
        inst->pos = 0;
    }
    g_mpg123_delete(inst->mh);
    free(inst);
}

/*
 * 0x0048AF80: decodes up to size bytes into buf, feeding mpg123 from the
 * file while it needs more. The bytes decoded go to *got (if not NULL).
 * Returns mpg123's status: MPG123_DONE when the file ended, 5 (logged) when
 * a short read is not the end of the file.
 */
int Mpg123_FeedRead(Mpg123Instance *inst, void *buf, unsigned int size, unsigned int *got)
{
    int eof = 0;
    size_t n;
    unsigned int total;
    int r;

    r = g_mpg123_read(inst->mh, buf, size, &n);
    total = (unsigned int)n;
    while (r == MPG123_NEED_MORE) {
        unsigned int in = (unsigned int)inst->io.read(inst->readbuf, 1, g_mpg123ReadBufferSize, inst->io.handle);
        if (in < g_mpg123ReadBufferSize) {
            eof = inst->io.eof(inst->io.handle);
            if (eof == 0) {
                MatchLog_Printf(3, "I/O error in mpg123 subsystem\n");
                return 5;
            }
        }
        r = g_mpg123_feed(inst->mh, inst->readbuf, in);
        if (r != 0) {
            return r;
        }
        r = g_mpg123_read(inst->mh, (unsigned char *)buf + total, size - total, &n);
        total += (unsigned int)n;
        if (eof != 0) {
            break;
        }
    }
    if (got != NULL) {
        *got = total;
    }
    if (eof != 0 && r == MPG123_NEED_MORE) {
        r = MPG123_DONE;
    }
    return r;
}

/*
 * 0x0048B070: the plugin's init: keeps the mixer format, loads
 * libmpg123-0.dll and reads the settings: decoder, rva (track / album),
 * volume (percent, > 0; 1.0 when unreadable), ntom (loop points in source
 * samples) and readbuffersize. Returns 6 when the library cannot be used, 2
 * when mpg123_init fails. Quirk: an unreadable readbuffersize takes the
 * address of config as its value.
 */
int Mpg123_Init(const BgmPluginConfig *config, const BgmHostCallbacks *host, BgmSettingFn get_setting)
{
    const BgmSettings *settings = config->settings;
    const char *s;
    int size;

    g_mpg123Format = config->format;
    g_mpg123ReadBufferSize = 0x4000;
    g_mpg123Lib = LoadLibraryA("libmpg123-0.dll");
    if (g_mpg123Lib == NULL || Mpg123_LoadSymbols() != 0) {
        return 6;
    }
    s = BgmSettings_Get(settings, "decoder");
    if (s != NULL) {
        strncpy(g_mpg123Decoder, s, 0x3F);
        g_mpg123Decoder[0x3F] = '\0';
    }
    g_mpg123Rva = 0;
    s = BgmSettings_Get(settings, "rva");
    if (s != NULL) {
        if (_stricmp(s, "track") == 0) {
            g_mpg123Rva = 1;
        } else if (_stricmp(s, "album") == 0) {
            g_mpg123Rva = 2;
        }
    }
    s = BgmSettings_Get(settings, "volume");
    if (s != NULL) {
        double v;
        if (sscanf(s, "%lf", &g_mpg123Volume) == 1) {
            v = g_mpg123Volume;
            if (v < 0.0 || v == 0.0) {
                v = 0.0;
            } else {
                v = v / 100.0;
            }
        } else {
            v = 1.0;
        }
        g_mpg123Volume = v;
    }
    s = BgmSettings_Get(settings, "ntom");
    g_mpg123Ntom = 0;
    if (s != NULL && sscanf(s, "%d", &g_mpg123Ntom) != 1) {
        g_mpg123Ntom = 0;
    }
    s = BgmSettings_Get(settings, "readbuffersize");
    if (s != NULL) {
        size = (int)config;
        sscanf(s, "%d", &size);
        if (size > 0) {
            g_mpg123ReadBufferSize = (unsigned int)size;
        }
    }
    if (g_mpg123_init() != 0) {
        return 2;
    }
    g_mpg123Initialized = 1;
    return 0;
}

/* 0x0048B230: the plugin's deinit (g_mpg123Initialized stays set). */
void Mpg123_Deinit(void)
{
    if (g_mpg123Initialized) {
        g_mpg123_exit();
    }
    if (g_mpg123Lib != NULL) {
        FreeLibrary(g_mpg123Lib);
    }
    g_mpg123Lib = NULL;
}

/*
 * 0x0048B260: sets the loop points of inst (end -1 or not after start: the
 * end of the file), converted from source samples with ntom.
 */
void Mpg123_SetLoop(Mpg123Instance *inst, int start, int end)
{
    inst->loop_start_raw = start;
    if (start <= -1) {
        start = 0;
    }
    if (end != -1 && end <= start) {
        end = -1;
    }
    inst->loop_end_raw = end;
    if (g_mpg123Ntom) {
        double ratio = inst->format.rate / (double)inst->src_rate;
        inst->loop_start = (int)floor(ratio * start + 0.5);
        if (end < 0) {
            inst->loop_end = (int)2147483647.0;
        } else {
            inst->loop_end = (int)floor(end * ratio + 0.5);
        }
        return;
    }
    inst->loop_start = start;
    inst->loop_end = end < 0 ? 0x7FFFFFFF : end;
}

/* 0x0048B320: the plugin's get_format: inst's output format. */
int Mpg123_GetFormatOf(void *instance, AudioFormat *format)
{
    const Mpg123Instance *inst = instance;

    *format = inst->format;
    return 0;
}

/* 0x0048B340: the output format of inst's decoder into out; 3 for encodings the BGM cannot play. */
int Mpg123_GetFormat(AudioFormat *out, const Mpg123Instance *inst)
{
    long rate;
    int channels, encoding;
    double r;
    int format;

    g_mpg123_getformat(inst->mh, &rate, &channels, &encoding);
    r = (double)rate;
    switch (encoding) {
    case MPG123_ENC_UNSIGNED_8:
        format = AUDIO_FMT_U8;
        break;
    case MPG123_ENC_SIGNED_16:
        format = AUDIO_FMT_S16;
        break;
    case MPG123_ENC_FLOAT_32:
        format = AUDIO_FMT_F32;
        break;
    default:
        return 3;
    }
    out->rate = r;
    out->format = format;
    out->channels = channels;
    return 0;
}

/* 0x0048B3C0: reads the decoder's output format, frame size and source rate into inst. */
int Mpg123_UpdateFormat(Mpg123Instance *inst)
{
    Mpg123FrameInfo fi;
    int r = Mpg123_GetFormat(&inst->format, inst);
    int bytes;

    if (r != 0) {
        return r;
    }
    if (inst->format.format & 0x04) {
        bytes = 1;
    } else if (inst->format.format & 0x08) {
        bytes = 2;
    } else if (inst->format.format & 0x10) {
        bytes = 4;
    } else if (inst->format.format & 0x20) {
        bytes = 8;
    } else {
        bytes = 0;      /* the original reads an uninitialised local (unreachable) */
    }
    inst->frame_bytes = inst->format.channels * bytes;
    r = g_mpg123_info(inst->mh, &fi);
    if (r != 0) {
        inst->last_error = r;
        return 2;
    }
    inst->src_rate = fi.rate;
    return 0;
}

/*
 * 0x0048B440: the plugin's open: opens the file in inst's decoder, reads
 * up to the first frame and reports the format (BGM_MODE_DECODED); loop
 * settings come from info. Returns 0, 4 without a decoder, 2 on mpg123
 * errors, or the read status.
 */
int Mpg123_Open(void *instance, const BgmPluginIo *io, BgmInfo *info, BgmOpenResult *out)
{
    Mpg123Instance *inst = instance;
    int r;

    if (inst == NULL || inst->mh == NULL) {
        return 4;
    }
    if (inst->io.handle != NULL) {
        Mpg123_Close(inst);
    }
    inst->io = *io;
    if (g_mpg123_open_feed(inst->mh) != 0) {
        inst->io.handle = NULL;
        return 2;
    }
    if (g_mpg123Ntom) {
        Mpg123_SetupFormat(inst->mh);
    }
    r = Mpg123_FeedRead(inst, NULL, 0, NULL);
    if (r == 0 || r == MPG123_NEW_FORMAT) {
        r = Mpg123_UpdateFormat(inst);
        if (r == 0) {
            out->format = inst->format;
            out->mode = BGM_MODE_DECODED;
            inst->loop = info->loop;
            Mpg123_SetLoop(inst, info->loop_start, info->loop_end);
            inst->pos = 0;
            r = 0;
        }
        return r;
    }
    if (r < 0) {
        inst->last_error = r;
        return 2;
    }
    return r;
}

/* 0x0048B540: the plugin's close. */
int Mpg123_Close(void *instance)
{
    Mpg123Instance *inst = instance;

    if (inst != NULL && inst->mh != NULL) {
        g_mpg123_close(inst->mh);
        inst->io.handle = NULL;
        inst->playing = 0;
        inst->pos = 0;
    }
    return 0;
}

/* 0x0048B580: the plugin's start (4 without an open file). */
int Mpg123_Start(void *instance)
{
    Mpg123Instance *inst = instance;

    if (inst != NULL && inst->mh != NULL && inst->io.handle != NULL) {
        inst->playing = 1;
        return 0;
    }
    return 4;
}

/* 0x0048B5B0: the plugin's stop: back to the start. */
int Mpg123_Stop(void *instance)
{
    Mpg123Instance *inst = instance;

    if (inst == NULL) {
        return 4;
    }
    if (inst->playing) {
        inst->playing = 0;
        inst->pos = 0;
    }
    return 0;
}

/* 0x0048B5E0: seeks inst's decoder and file to frame. Returns mpg123's result. */
int Mpg123_FeedSeek(Mpg123Instance *inst, int frame)
{
    long offset;
    int r = g_mpg123_feedseek(inst->mh, frame, SEEK_SET, &offset);

    inst->pos = frame;
    inst->io.seek(inst->io.handle, offset, SEEK_SET);
    return r;
}

/* 0x0048B610: seeks to pos (source samples with ntom). */
int Mpg123_Seek(void *instance, int pos)
{
    Mpg123Instance *inst = instance;
    int frame;
    int r;

    if (inst == NULL) {
        return 4;
    }
    if (inst->mh == NULL || inst->io.handle == NULL) {
        return 4;
    }
    if (g_mpg123Ntom) {
        frame = (int)floor(inst->format.rate / (double)inst->src_rate * pos + 0.5);
    } else {
        frame = pos;
    }
    r = Mpg123_FeedSeek(inst, frame);
    if (r < 0) {
        inst->last_error = r;
        return 2;
    }
    return 0;
}

/* 0x0048B5D0: seeks to the start. */
int Mpg123_Rewind(void *instance)
{
    return Mpg123_Seek(instance, 0);
}

/* 0x0048B680: playing? */
int Mpg123_IsPlaying(void *instance)
{
    const Mpg123Instance *inst = instance;

    return inst != NULL ? inst->playing : 0;
}

/* 0x0048B690: the volume (percent). */
double Mpg123_GetVolume(void *instance)
{
    const Mpg123Instance *inst = instance;

    return inst != NULL ? inst->volume : 0.0;
}

/* 0x0048B6A0: the plugin's set_volume (percent, clamped to 0..100), scaled by the volume setting. */
int Mpg123_SetVolume(void *instance, double volume)
{
    Mpg123Instance *inst = instance;

    if (inst == NULL) {
        return 4;
    }
    if (volume > 100.0) {
        volume = 100.0;
    } else if (0.0 > volume) {
        volume = 0.0;
    }
    inst->volume = volume;
    g_mpg123_volume(inst->mh, volume / 100.0 * g_mpg123Volume);
    return 0;
}

/* 0x0048B700: the loop flag. */
int Mpg123_GetLoopFlag(void *instance)
{
    const Mpg123Instance *inst = instance;

    return inst != NULL ? inst->loop : 0;
}

/* 0x0048B710: sets the loop flag. */
int Mpg123_SetLoopFlag(void *instance, int loop)
{
    Mpg123Instance *inst = instance;

    if (inst == NULL) {
        return 4;
    }
    inst->loop = loop;
    return 0;
}

/* 0x0048B730: sets the loop points. */
int Mpg123_SetLoopPoints(void *instance, int start, int end)
{
    Mpg123Instance *inst = instance;

    if (inst == NULL) {
        return 4;
    }
    Mpg123_SetLoop(inst, start, end);
    return 0;
}

/* 0x0048B760: the loop points as given. */
int Mpg123_GetLoopPoints(void *instance, int *start, int *end)
{
    const Mpg123Instance *inst = instance;

    if (inst != NULL) {
        if (start != NULL) {
            *start = inst->loop_start_raw;
        }
        if (end != NULL) {
            *end = inst->loop_end_raw;
        }
    }
    return 0;
}

/* 0x0048B790: fills size bytes at dst with silence in format. */
void Mpg123_Silence(int format, void *dst, unsigned int size)
{
    if (format == AUDIO_FMT_U8) {
        memset(dst, 0x80, size);
    } else if (format == AUDIO_FMT_S16 || format == AUDIO_FMT_F32) {
        memset(dst, 0, size);
    }
}

/*
 * 0x0048B7C0: the plugin's read: decodes up to size bytes. Without looping
 * the end of the file is reported (BGM_READ_EVENT_END) and playing stops;
 * with looping, playing jumps back to the loop start at the loop end (or
 * the end of the file, which then becomes the loop end). Decoding errors
 * are logged and the rest of the buffer filled with silence. A format
 * change is reported (BGM_READ_EVENT_FORMAT). Quirk: event may only be
 * NULL while playing.
 */
int Mpg123_Read(void *instance, void *buf, unsigned int size, unsigned int *got, int *event)
{
    Mpg123Instance *inst = instance;
    unsigned int total = 0;
    unsigned int n = 0;
    int err = 0;
    int ended = 0;

    if (inst == NULL) {
        return 4;
    }
    if (inst->mh == NULL) {
        err = 4;
    } else if (!inst->playing) {
        *event = BGM_READ_EVENT_END;
        total = 0;
    } else {
        if (event != NULL) {
            *event = BGM_READ_EVENT_BLOCK;
        }
        if (inst->loop == 0) {
            int r = Mpg123_FeedRead(inst, buf, size, &n);
            total = n;
            if (r == MPG123_DONE) {
                if (event != NULL) {
                    *event = BGM_READ_EVENT_END;
                }
                ended = 1;
                inst->playing = 0;
            } else if (r == MPG123_NEW_FORMAT) {
                if (event != NULL) {
                    *event = BGM_READ_EVENT_FORMAT;
                }
                Mpg123_UpdateFormat(inst);
            } else if (r != 0) {
                if (r < 0) {
                    err = 2;
                    inst->last_error = r;
                } else {
                    err = r;
                }
            }
        } else {
            int frames_left = (int)(size / (unsigned int)inst->frame_bytes);
            do {
                int count = frames_left;
                int r, frames, pos;
                const char *msg;

                if (inst->loop_end - inst->pos < frames_left) {
                    count = inst->loop_end - inst->pos;
                }
                r = Mpg123_FeedRead(inst, (unsigned char *)buf + total, inst->frame_bytes * count, &n);
                total += n;
                frames = (int)(n / (unsigned int)inst->frame_bytes);
                inst->pos += frames;
                frames_left -= frames;
                pos = inst->pos;
                if (r == MPG123_DONE) {
                    if (pos < inst->loop_end) {
                        int start_after = inst->loop_start < pos;
                        inst->loop_end = pos;
                        if (!start_after) {
                            inst->loop_start = 0;
                        }
                    }
                } else if (pos < inst->loop_end) {
                    if (r == MPG123_NEW_FORMAT) {
                        if (event != NULL) {
                            *event = BGM_READ_EVENT_FORMAT;
                        }
                        Mpg123_UpdateFormat(inst);
                        break;
                    }
                    if (r == 0) {
                        continue;
                    }
                    if (r > 0) {
                        err = r;
                        msg = ((const char *const *)g_mpg123Errors)[r];
                    } else {
                        inst->last_error = r;
                        err = 2;
                        msg = g_mpg123_strerror(inst->mh);
                    }
                    MatchLog_Printf(3, "mpg123 error %s: filling silence...\n", msg);
                    Mpg123_Silence(inst->format.format, (unsigned char *)buf + total, size - total);
                    break;
                }
                /* back to the loop start */
                {
                    long offset = (long)inst;
                    int start = inst->loop_start;
                    int res = g_mpg123_feedseek(inst->mh, start, SEEK_SET, &offset);
                    inst->pos = start;
                    inst->io.seek(inst->io.handle, offset, SEEK_SET);
                    if (res < 0) {
                        inst->last_error = res;
                        err = 2;
                        MatchLog_Printf(3, "mpg123 error %s: filling silence...\n", g_mpg123_strerror(inst->mh));
                        Mpg123_Silence(inst->format.format, (unsigned char *)buf + total, size - total);
                        break;
                    }
                }
            } while (frames_left > 0);
        }
    }
    *got = total;
    if (ended && inst->end_callback != NULL) {
        inst->end_callback(inst->end_arg);
    }
    return err;
}

/* 0x0048BA10: sets the function called when the stream ends by itself. */
void Mpg123_SetEndCallback(void *instance, BgmEndFn fn, void *arg)
{
    Mpg123Instance *inst = instance;

    if (inst != NULL) {
        inst->end_callback = fn;
        inst->end_arg = arg;
    }
}

/*
 * 0x0048BA30: the plugin's strerror: g_mpg123Errors, mpg123's message of
 * the last error for 2. Quirk: 7 reads the word after the table.
 */
const char *Mpg123_StrError(void *instance, int err)
{
    const Mpg123Instance *inst = instance;

    if (err < 0 || err > 7) {
        return NULL;
    }
    if (err == 2 && inst != NULL) {
        return g_mpg123_plain_strerror(inst->last_error);
    }
    return ((const char *const *)g_mpg123Errors)[err];
}

/* ------------------------------------------------------------------ sdlmix */

/* 0x0048BA70: the sdlmix plugin's default extension list. */
const char *Sdlmix_Extensions(void)
{
    return "669,amf,apun,dsm,far,gdm,it,imf,mod,med,mtm,okt,s3m,stm,stx,ult,uni,xm,mid,midi";
}

/* Is ext (lower-case) the first entry of list it is found in? (strstr's first match only.) */
static int Sdlmix_InList(const char *list, const char *ext)
{
    const char *p = strstr(list, ext);

    if (p == NULL || (p != list && p[-1] != ',')) {
        return 0;
    }
    p += strlen(ext);
    return *p == '\0' || *p == ',';
}

/*
 * 0x0048BA80: the kind of music an extension is (SDLMIX_TYPE_*), from
 * fixed lists; 0 when unknown. The extension is lower-cased into a
 * 0x200-byte buffer without a length check.
 */
int Sdlmix_Classify(const char *ext)
{
    char buf[0x200];
    size_t len = strlen(ext);
    size_t i;

    for (i = 0; ext[i] != '\0'; i++) {
        buf[i] = (char)tolower(ext[i]);
    }
    buf[len] = '\0';
    if (Sdlmix_InList("mp3", buf)) {
        return SDLMIX_TYPE_MP3;
    }
    if (Sdlmix_InList("669,amf,apun,dsm,far,gdm,it,imf,mod,med,mtm,okt,s3m,stm,stx,ult,uni,xm", buf)) {
        return SDLMIX_TYPE_MOD;
    }
    if (Sdlmix_InList("mid,midi", buf)) {
        return SDLMIX_TYPE_MIDI;
    }
    if (Sdlmix_InList("wav", buf)) {
        return SDLMIX_TYPE_WAV;
    }
    if (Sdlmix_InList("ogg,vorbis", buf)) {
        return SDLMIX_TYPE_OGG;
    }
    return 0;
}

/*
 * One volume setting (percent): 100 when absent or unreadable, negative
 * values give 0. Quirk kept: sscanf's EOF result (empty setting) counts as
 * read, and the value is whatever the scratch variable held.
 */
static double Sdlmix_ReadVolume(const BgmSettings *settings, const char *key, double *scratch)
{
    const char *s = BgmSettings_Get(settings, key);
    double v;

    if (s == NULL) {
        return 100.0;
    }
    v = sscanf(s, "%lf", scratch) != 0 ? *scratch : 100.0;
    if (0.0 > v) {
        v = 0.0;
    }
    return v;
}

/* 0x0048BC60: the sdlmix plugin's init: keeps the host callbacks and the mixer format, reads the volumes. */
int Sdlmix_Init(const BgmPluginConfig *config, const BgmHostCallbacks *host, BgmSettingFn get_setting)
{
    const BgmSettings *settings = config->settings;
    double scratch = 0.0;

    g_sdlmixHost = *host;
    g_sdlmixVolumes[SDLMIX_TYPE_MOD] = Sdlmix_ReadVolume(settings, "modvolume", &scratch);
    g_sdlmixVolumes[SDLMIX_TYPE_WAV] = Sdlmix_ReadVolume(settings, "wavvolume", &scratch);
    g_sdlmixVolumes[SDLMIX_TYPE_MP3] = Sdlmix_ReadVolume(settings, "mp3volume", &scratch);
    g_sdlmixVolumes[SDLMIX_TYPE_MIDI] = Sdlmix_ReadVolume(settings, "midivolume", &scratch);
    g_sdlmixVolumes[SDLMIX_TYPE_OGG] = Sdlmix_ReadVolume(settings, "oggvolume", &scratch);
    g_sdlmixFormat = config->format;
    return 0;
}

/* 0x0048BE70: returns 0 (the sdlmix deinit, also used elsewhere as an empty callback). */
int Stub_ReturnZero(void)
{
    return 0;
}

/* 0x0048BE80: the sdlmix instance (only one: NULL while it exists). */
void *Sdlmix_Create(void)
{
    SdlmixInstance *inst;

    if (g_sdlmixInstance != NULL) {
        return NULL;
    }
    inst = calloc(1, sizeof(SdlmixInstance));
    if (inst == NULL) {
        g_outOfMemory = 1;
    }
    g_sdlmixInstance = inst;
    return inst;
}

/* Frees inst's music and unhooks the end callback. */
static void Sdlmix_FreeMusic(SdlmixInstance *inst)
{
    if (inst->music != NULL) {
        Mix_FreeMusic(inst->music);
    }
    inst->ended = 0;
    inst->finished = 0;
    Mix_HookMusicFinished(NULL);
    inst->music = NULL;
}

/* 0x0048BEB0: frees the instance. */
void Sdlmix_Destroy(void *instance)
{
    SdlmixInstance *inst = instance;

    if (inst != NULL) {
        Sdlmix_FreeMusic(inst);
        free(inst);
    }
    g_sdlmixInstance = NULL;
}

/*
 * 0x0048BEF0: the plugin's open: loads MIDI and tracker files (the other
 * kinds are refused: 4) through SDL_mixer; the plugin plays them itself
 * (out stays zero: BGM_MODE_PLUGIN).
 */
int Sdlmix_Open(void *instance, const BgmPluginIo *io, BgmInfo *info, BgmOpenResult *out)
{
    SdlmixInstance *inst = instance;
    SDL_RWops *rw;

    memset(out, 0, sizeof(*out));
    inst->ended = 0;
    if (inst->music != NULL) {
        Sdlmix_FreeMusic(inst);
    }
    inst->type = Sdlmix_Classify(g_sdlmixHost.get_extension(info->path));
    if (inst->type == 0 || inst->type == SDLMIX_TYPE_WAV || inst->type == SDLMIX_TYPE_MP3 ||
        inst->type == SDLMIX_TYPE_OGG) {
        return 4;
    }
    rw = Stream_ToRWops((Stream *)io->handle);
    if (rw == NULL) {
        return 4;
    }
    inst->music = Mix_LoadMUS_RW(rw);
    if (inst->music == NULL) {
        return 4;
    }
    if (inst->type - 1u <= 1u) {
        rw->close(rw);
    }
    inst->loop = info->loop;
    inst->loop_start = info->loop_start;
    inst->loop_end = info->loop_end;
    Sdlmix_SetVolume(inst, inst->volume);
    return 0;
}

/* 0x0048BFE0: the plugin's close. */
int Sdlmix_Close(void *instance)
{
    Sdlmix_FreeMusic(instance);
    return 0;
}

/* 0x0048C020: SDL_mixer's music finished hook: the end of the stream. */
void Sdlmix_OnFinished(void)
{
    SdlmixInstance *inst = g_sdlmixInstance;

    inst->ended = 1;
    if (inst->end_callback != NULL) {
        inst->end_callback(inst->end_arg);
    }
}

/* 0x0048C040: the plugin's start: plays once or looping. */
int Sdlmix_Start(void *instance)
{
    SdlmixInstance *inst = instance;

    if (inst->music == NULL) {
        return 3;
    }
    inst->finished = 0;
    Sdlmix_SetVolume(inst, inst->volume);
    Mix_PlayMusic(inst->music, inst->loop == 0 ? 1 : -1);
    Mix_HookMusicFinished(Sdlmix_OnFinished);
    inst->ended = 0;
    return 0;
}

/* 0x0048C0A0: the plugin's pause. */
int Sdlmix_Pause(void *instance)
{
    Mix_PauseMusic();
    return 0;
}

/* 0x0048C0B0: the plugin's resume (a finished stream starts again). */
int Sdlmix_Resume(void *instance)
{
    SdlmixInstance *inst = instance;

    if (inst->finished) {
        Sdlmix_Start(inst);
        return 0;
    }
    Mix_ResumeMusic();
    return 0;
}

/* 0x0048C0D0: the plugin's stop. */
int Sdlmix_Stop(void *instance)
{
    SdlmixInstance *inst = instance;

    Mix_HookMusicFinished(NULL);
    Mix_HaltMusic();
    inst->finished = 0;
    inst->ended = 0;
    return 0;
}

/* 0x0048C100: back to the start. */
int Sdlmix_Rewind(void *instance)
{
    Mix_RewindMusic();
    return 0;
}

/* 0x0048C110: seeks: a pattern for tracker files, samples (at the mixer rate) for MP3 and Ogg. */
int Sdlmix_Seek(void *instance, int pos)
{
    const SdlmixInstance *inst = instance;

    switch (inst->type) {
    case SDLMIX_TYPE_MOD:
        Mix_SetMusicPosition((double)pos);
        break;
    case SDLMIX_TYPE_MP3:
        Mix_RewindMusic();
        Mix_SetMusicPosition((double)pos / g_sdlmixFormat.rate);
        break;
    case SDLMIX_TYPE_OGG:
        Mix_SetMusicPosition((double)pos / g_sdlmixFormat.rate);
        break;
    }
    return 0;
}

/* 0x0048C190: playing (or finished)? */
int Sdlmix_IsPlaying(void *instance)
{
    const SdlmixInstance *inst = instance;

    if (inst->music == NULL) {
        return 0;
    }
    if (inst->finished) {
        return 1;
    }
    return Mix_PlayingMusic() != 0;
}

/* 0x0048C1C0: paused (or finished)? */
int Sdlmix_IsPaused(void *instance)
{
    const SdlmixInstance *inst = instance;
    int r;

    if (inst->finished) {
        return 1;
    }
    r = Mix_PausedMusic();
    return r != 0 ? 1 : r;
}

/* 0x0048C1E0: the volume (percent). */
double Sdlmix_GetVolume(void *instance)
{
    const SdlmixInstance *inst = instance;

    return inst->volume;
}

/*
 * 0x0048C1F0: the plugin's set_volume: percent clamped to 0..100, scaled by
 * the volume setting of the music kind; 0 while muted. Nothing for unknown
 * kinds.
 */
int Sdlmix_SetVolume(void *instance, double volume)
{
    SdlmixInstance *inst = instance;
    int type = inst->type;

    if (type != 0) {
        int v;
        if (0.0 > volume) {
            volume = 0.0;
        } else if (!(volume < 100.0)) {
            if (volume == volume) {
                volume = 100.0;
            }
        }
        inst->volume = volume;
        v = (int)(volume / 100.0 * g_sdlmixVolumes[type] * 1.28);
        Mix_VolumeMusic(inst->muted ? 0 : v);
    }
    return 0;
}

/* 0x0048C260: the loop flag. */
int Sdlmix_GetLoopFlag(void *instance)
{
    const SdlmixInstance *inst = instance;

    return inst->loop;
}

/* 0x0048C270: sets the loop flag; 1 (refused) when it changes while music plays. */
int Sdlmix_SetLoopFlag(void *instance, int loop)
{
    SdlmixInstance *inst = instance;

    if (loop != inst->loop && Mix_PlayingMusic()) {
        return 1;
    }
    inst->loop = loop;
    return 0;
}

/* 0x0048C2A0: the loop points (not used by SDL_mixer). */
int Sdlmix_GetLoopPoints(void *instance, int *start, int *end)
{
    const SdlmixInstance *inst = instance;

    if (start != NULL) {
        *start = inst->loop_start;
    }
    if (end != NULL) {
        *end = inst->loop_end;
    }
    return 0;
}

/* 0x0048C2D0: sets the loop points. */
int Sdlmix_SetLoopPoints(void *instance, int start, int end)
{
    SdlmixInstance *inst = instance;

    inst->loop_start = start;
    inst->loop_end = end;
    return 0;
}

/* 0x0048C2F0: the plugin's read: no data, only the end of the stream (*got untouched). */
int Sdlmix_Read(void *instance, void *buf, unsigned int size, unsigned int *got, int *event)
{
    const SdlmixInstance *inst = instance;

    if (event != NULL) {
        *event = inst->ended;
    }
    return 0;
}

/* 0x0048C310: sets the function called when the music ends. */
void Sdlmix_SetEndCallback(void *instance, BgmEndFn fn, void *arg)
{
    SdlmixInstance *inst = instance;

    inst->end_callback = fn;
    inst->end_arg = arg;
}

/* 0x0048C330: the plugin's strerror. Quirk: 5 reads the word after the table. */
const char *Sdlmix_StrError(void *instance, int err)
{
    if (err < 0 || err > 5) {
        return NULL;
    }
    return ((const char *const *)g_sdlmixErrors)[err];
}

/* 0x0048C350: the plugin's set_mute. */
int Sdlmix_SetMute(void *instance, int muted)
{
    SdlmixInstance *inst = instance;

    if (muted) {
        if (inst->muted) {
            return 0;
        }
        inst->muted = 1;
    } else {
        if (!inst->muted) {
            return 0;
        }
        inst->muted = 0;
    }
    Sdlmix_SetVolume(inst, inst->volume);
    return 0;
}

/* 0x0048C3A0: muted? */
int Sdlmix_GetMute(void *instance)
{
    const SdlmixInstance *inst = instance;

    return inst->muted;
}

/* 0x0048C3B0: notes that loaded music stopped playing by itself (finished). */
int Sdlmix_Poll(void *instance)
{
    SdlmixInstance *inst = instance;

    if (inst->music != NULL && !inst->finished && !Mix_PlayingMusic()) {
        inst->finished = 1;
    }
    return 0;
}
