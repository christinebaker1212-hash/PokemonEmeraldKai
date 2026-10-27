/*
 * BGM stream loader (original module 0x49AA50-0x49C3BF): the sample format
 * converters of the resampler (Audio_GetConverter), and the BGM stream: the
 * current BGM opened by a plugin ("subsystem"), either played by the plugin
 * itself (BGM_MODE_PLUGIN) or decoded and resampled to the mixer format
 * (BGM_MODE_DECODED) into blocks of the ring buffer between the BGM thread
 * and the mixer callback (bgmthread.c).
 *
 * The converters' float constants are in .rdata after the loader's strings,
 * which puts both in one object file.
 *
 * Everything here except the converters runs on the BGM thread
 * (BgmThread_Main and the commands it runs), the converters on whatever
 * thread drives the resampler.
 */
#include "mugen.h"

/* ------------------------------------------------------------ converters */

/*
 * 0x0049AA50: a float sample (-1..1) as signed 16-bit: x * 32768 rounded to
 * float, clamped to -32768..32767, truncated (NaN gives 0x80000000).
 */
int Audio_FloatToS16(float x)
{
    float v = (float)((double)x * 32768.0);

    if (v > 32767.0) {
        v = 32767.0f;
    } else if (-32768.0f > v) {
        v = -32768.0f;
    }
    return (int)v;
}

/*
 * 0x0049AAA0: a float sample (-1..1) as unsigned 8-bit: (x + 1) * 128
 * rounded to float, clamped to 0..255, truncated.
 */
unsigned char Audio_FloatToU8(float x)
{
    float v = (float)(((double)x + 1.0) * 128.0);

    if (v > 255.0) {
        v = 255.0f;
    } else if (0.0f > v) {
        v = 0.0f;
    }
    return (unsigned char)(int)v;
}

/*
 * The converters: frames frames of src_channels interleaved samples to
 * dst_channels (1 or 2; mono to stereo duplicates, stereo to mono averages).
 * Other channel counts convert nothing, except equal counts between equal
 * formats (a copy).
 */

/* 0x0049ABB0: signed 16-bit to signed 16-bit. */
void Audio_ConvertS16ToS16(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    short *d = dst;
    const short *s = src;
    unsigned int i;

    if (dst_channels == src_channels) {
        memcpy(dst, src, frames * src_channels * 2);
    } else if (dst_channels == 1) {
        if (src_channels == 2) {
            for (i = 0; i < frames; i++) {
                d[i] = (short)((s[2 * i] + s[2 * i + 1]) / 2);
            }
        }
    } else if (dst_channels == 2 && src_channels == 1) {
        for (i = 0; i < frames; i++) {
            d[2 * i] = s[i];
            d[2 * i + 1] = s[i];
        }
    }
}

/* 0x0049AC60: float to float. */
void Audio_ConvertF32ToF32(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    float *d = dst;
    const float *s = src;
    unsigned int i;

    if (dst_channels == src_channels) {
        memcpy(dst, src, frames * src_channels * 4);
    } else if (dst_channels == 1) {
        if (src_channels == 2) {
            for (i = 0; i < frames; i++) {
                d[i] = (float)(((double)s[2 * i + 1] + s[2 * i]) * 0.5);
            }
        }
    } else if (dst_channels == 2 && src_channels == 1) {
        for (i = 0; i < frames; i++) {
            d[2 * i] = s[i];
            d[2 * i + 1] = s[i];
        }
    }
}

/* 0x0049AD00: unsigned 8-bit to unsigned 8-bit. */
void Audio_ConvertU8ToU8(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    unsigned int i;

    if (dst_channels == src_channels) {
        memcpy(dst, src, frames * src_channels);
    } else if (dst_channels == 1) {
        if (src_channels == 2) {
            for (i = 0; i < frames; i++) {
                d[i] = (unsigned char)((s[2 * i] + s[2 * i + 1]) / 2);
            }
        }
    } else if (dst_channels == 2 && src_channels == 1) {
        for (i = 0; i < frames; i++) {
            d[2 * i] = s[i];
            d[2 * i + 1] = s[i];
        }
    }
}

/* Scale of the integer to float conversions. */
#define AUDIO_S16_SCALE (1.0 / 32768)
#define AUDIO_U8_SCALE  (1.0 / 128)

/* 0x0049ADB0: signed 16-bit to float. */
void Audio_ConvertS16ToF32(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    float *d = dst;
    const short *s = src;
    unsigned int i;

    if (src_channels == 1) {
        if (dst_channels == 2) {
            for (i = 0; i < frames; i++) {
                float v = (float)(s[i] * AUDIO_S16_SCALE);
                d[2 * i] = v;
                d[2 * i + 1] = v;
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                d[i] = (float)(s[i] * AUDIO_S16_SCALE);
            }
        }
    } else if (src_channels == 2) {
        if (dst_channels == 2) {
            for (i = 0; i < frames * 2; i++) {
                d[i] = (float)(s[i] * AUDIO_S16_SCALE);
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                float a = (float)(s[2 * i] * AUDIO_S16_SCALE);
                float b = (float)(s[2 * i + 1] * AUDIO_S16_SCALE);
                d[i] = (float)(((double)a + b) * 0.5);
            }
        }
    }
}

/* 0x0049B190: unsigned 8-bit to float. */
void Audio_ConvertU8ToF32(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    float *d = dst;
    const unsigned char *s = src;
    unsigned int i;

    if (src_channels == 1) {
        if (dst_channels == 2) {
            for (i = 0; i < frames; i++) {
                float v = (float)((s[i] - 128.0) * AUDIO_U8_SCALE);
                d[2 * i] = v;
                d[2 * i + 1] = v;
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                d[i] = (float)((s[i] - 128.0) * AUDIO_U8_SCALE);
            }
        }
    } else if (src_channels == 2) {
        if (dst_channels == 2) {
            for (i = 0; i < frames * 2; i++) {
                d[i] = (float)((s[i] - 128.0) * AUDIO_U8_SCALE);
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                float a = (float)((s[2 * i] - 128.0) * AUDIO_U8_SCALE);
                float b = (float)((s[2 * i + 1] - 128.0) * AUDIO_U8_SCALE);
                d[i] = (float)(((double)a + b) * 0.5);
            }
        }
    }
}

/* 0x0049B580: float to signed 16-bit. */
void Audio_ConvertF32ToS16(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    short *d = dst;
    const float *s = src;
    unsigned int i;

    if (src_channels == 1) {
        if (dst_channels == 2) {
            for (i = 0; i < frames; i++) {
                short v = (short)Audio_FloatToS16(s[i]);
                d[2 * i] = v;
                d[2 * i + 1] = v;
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                d[i] = (short)Audio_FloatToS16(s[i]);
            }
        }
    } else if (src_channels == 2) {
        if (dst_channels == 2) {
            for (i = 0; i < frames * 2; i++) {
                d[i] = (short)Audio_FloatToS16(s[i]);
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                float v = (float)(((double)s[2 * i + 1] + s[2 * i]) * 0.5);
                d[i] = (short)Audio_FloatToS16(v);
            }
        }
    }
}

/* 0x0049B6B0: unsigned 8-bit to signed 16-bit (16-bit arithmetic). */
void Audio_ConvertU8ToS16(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    short *d = dst;
    const unsigned char *s = src;
    unsigned int i;

    if (src_channels == 1) {
        if (dst_channels == 2) {
            for (i = 0; i < frames; i++) {
                short v = (short)((s[i] - 0x80) * 0x100);
                d[2 * i] = v;
                d[2 * i + 1] = v;
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                d[i] = (short)((s[i] - 0x80) * 0x100);
            }
        }
    } else if (src_channels == 2) {
        if (dst_channels == 2) {
            for (i = 0; i < frames * 2; i++) {
                d[i] = (short)((s[i] - 0x80) * 0x100);
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                unsigned char m = (unsigned char)((s[2 * i] + s[2 * i + 1]) / 2);
                d[i] = (short)((m - 0x80) * 0x100);
            }
        }
    }
}

/* 0x0049B7F0: float to unsigned 8-bit. */
void Audio_ConvertF32ToU8(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    unsigned char *d = dst;
    const float *s = src;
    unsigned int i;

    if (src_channels == 1) {
        if (dst_channels == 2) {
            for (i = 0; i < frames; i++) {
                unsigned char v = Audio_FloatToU8(s[i]);
                d[2 * i] = v;
                d[2 * i + 1] = v;
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                d[i] = Audio_FloatToU8(s[i]);
            }
        }
    } else if (src_channels == 2) {
        if (dst_channels == 2) {
            for (i = 0; i < frames * 2; i++) {
                d[i] = Audio_FloatToU8(s[i]);
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                float v = (float)(((double)s[2 * i + 1] + s[2 * i]) * 0.5);
                d[i] = Audio_FloatToU8(v);
            }
        }
    }
}

/* 0x0049B910: signed 16-bit to unsigned 8-bit (the high byte, offset by 128). */
void Audio_ConvertS16ToU8(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels)
{
    unsigned char *d = dst;
    const short *s = src;
    const unsigned char *b = src;
    unsigned int i;

    if (src_channels == 1) {
        if (dst_channels == 2) {
            for (i = 0; i < frames; i++) {
                unsigned char v = (unsigned char)(b[2 * i + 1] - 0x80);
                d[2 * i] = v;
                d[2 * i + 1] = v;
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                d[i] = (unsigned char)(b[2 * i + 1] - 0x80);
            }
        }
    } else if (src_channels == 2) {
        if (dst_channels == 2) {
            for (i = 0; i < frames * 2; i++) {
                d[i] = (unsigned char)(b[2 * i + 1] - 0x80);
            }
        } else if (dst_channels == 1) {
            for (i = 0; i < frames; i++) {
                d[i] = (unsigned char)((((s[2 * i] + s[2 * i + 1]) / 2) >> 8) - 0x80);
            }
        }
    }
}

/* 0x0049AB10: the converter from src's sample format to dst's, NULL for unknown formats. */
AudioConvertFn Audio_GetConverter(const AudioFormat *src, const AudioFormat *dst)
{
    switch (src->format) {
    case AUDIO_FMT_U8:
        switch (dst->format) {
        case AUDIO_FMT_U8: return Audio_ConvertU8ToU8;
        case AUDIO_FMT_S16: return Audio_ConvertU8ToS16;
        case AUDIO_FMT_F32: return Audio_ConvertU8ToF32;
        }
        return NULL;
    case AUDIO_FMT_S16:
        switch (dst->format) {
        case AUDIO_FMT_U8: return Audio_ConvertS16ToU8;
        case AUDIO_FMT_S16: return Audio_ConvertS16ToS16;
        case AUDIO_FMT_F32: return Audio_ConvertS16ToF32;
        }
        return NULL;
    case AUDIO_FMT_F32:
        switch (dst->format) {
        case AUDIO_FMT_U8: return Audio_ConvertF32ToU8;
        case AUDIO_FMT_S16: return Audio_ConvertF32ToS16;
        case AUDIO_FMT_F32: return Audio_ConvertF32ToF32;
        }
        return NULL;
    }
    return NULL;
}

/* --------------------------------------------------------------- streams */

/* Bytes per sample of AudioFormat.format's size bits (8, 16, 32, 64), else fallback. */
static int Bgm_SampleBytes(int format, int fallback)
{
    if (format & 0x04) {
        return 1;
    }
    if (format & 0x08) {
        return 2;
    }
    if (format & 0x10) {
        return 4;
    }
    if (format & 0x20) {
        return 8;
    }
    return fallback;
}

/*
 * 0x0049BA00: a ring of BGM_RING_BLOCKS blocks of block bytes, all free
 * (space = size).
 */
BgmRing *BgmRing_Create(int block)
{
    BgmRing *ring = calloc(1, sizeof(BgmRing));
    unsigned char *buf;

    if (ring == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    buf = calloc(BGM_RING_BLOCKS, block);
    if (buf == NULL) {
        g_outOfMemory = 1;
    }
    ring->buf = buf;
    if (buf == NULL) {
        free(ring);
        return NULL;
    }
    ring->block = block;
    ring->count = BGM_RING_BLOCKS;
    ring->size = block << 5;
    ring->filled = 0;
    ring->space = ring->size;
    ring->write = 0;
    ring->read = 0;
    return ring;
}

/*
 * 0x0049BA90: starts the loaded stream s at its volume: held (flags) streams
 * that the plugin plays are started paused. Returns the plugin's error (0 on
 * success, also when there is nothing to start).
 */
int Bgm_StreamStart(BgmStream *s, unsigned int flags)
{
    BgmInstanceFn start;
    int err;

    if (s == NULL || s->plugin == NULL || s->playing || s->info.path[0] == '\0') {
        return 0;
    }
    s->plugin->set_volume(s->instance, Bgm_ScaleVolume(s->volume));
    if (flags != 0 && s->mode == BGM_MODE_PLUGIN) {
        start = s->plugin->start_paused;
    } else {
        start = s->plugin->start;
    }
    err = start(s->instance);
    s->playing = err == 0;
    if (err != 0) {
        MatchLog_Printf(3, "%s\n", s->plugin->strerror(s->instance, err));
        return err;
    }
    s->flags = flags;
    Bgm_StreamSetMute(s, s->muted);
    return 0;
}

/* 0x0049BB60: mutes or unmutes s; only playing streams that the plugin plays are told. Returns 1. */
int Bgm_StreamSetMute(BgmStream *s, int muted)
{
    if (s != NULL) {
        s->muted = muted;
        if (s->plugin != NULL && s->mode == BGM_MODE_PLUGIN && s->playing) {
            s->plugin->set_mute(s->instance, muted);
        }
    }
    return 1;
}

/*
 * 0x0049BBA0: sets the volume (percent; -1: the BGM's own, negative: 0) of
 * the current stream, scaled by the BGM volume. Quirk: -1 reads the stream
 * before checking that there is one.
 */
void Bgm_StreamSetVolume(double volume)
{
    BgmStream *s = g_bgmStream;

    if (volume == -1.0) {
        volume = s->info.volume;
    }
    if (volume < 0.0) {
        volume = 0.0;
    }
    if (s != NULL && s->plugin != NULL) {
        s->plugin->set_volume(s->instance, volume * g_bgmVolume / 100.0);
    }
}

/*
 * 0x0049BC10: prepares s's resampler from the decoder format in to the
 * mixer format (the current one is reset when its input format matches,
 * else replaced) and its decode buffer for one input block. Returns 0 when
 * the resampler or the buffer cannot be created (the old buffer leaks).
 * fallback: bytes per sample of unknown sample formats (the original reads
 * whatever its caller left in ecx).
 */
int Bgm_StreamSetupResampler(const AudioFormat *in, BgmStream *s, int fallback)
{
    ResamplerSession *r = s->resampler;
    unsigned int frames;
    unsigned int size;

    if (r != NULL) {
        const AudioFormat *cur = r->vtbl->in_format(r);
        if (cur->rate == in->rate && in->format == cur->format && in->channels == cur->channels) {
            s->resampler->vtbl->reset(s->resampler);
            s->format_changed = 0;
            return 1;
        }
    }
    if (s->resampler != NULL) {
        s->resampler->vtbl->close(s->resampler);
    }
    r = g_bgmResampler->begin(g_bgmResampler, (const WavInfo *)in);
    s->resampler = r;
    if (r == NULL) {
        return 0;
    }
    s->format_changed = 0;
    frames = r->vtbl->in_block(r);
    size = in->channels * Bgm_SampleBytes(in->format, fallback) * frames;
    if (size > s->decode_capacity) {
        s->decode_capacity = size;
        s->decode_buf = realloc(s->decode_buf, size);
        if (s->decode_buf == NULL) {
            return 0;
        }
    }
    s->decode_size = size;
    return 1;
}

/*
 * 0x0049BD30: opens info's file in the current stream with the plugin of
 * its extension (the stream is closed first; the plugin instance is kept
 * when the plugin is the same). Bad loop points are reset in info (-1).
 * Returns 0 when the instance or the file cannot be opened, else 1 (also
 * when no plugin plays the file or the plugin cannot open it, logged).
 */
int Bgm_StreamOpen(const BgmInfo *info_in)
{
    BgmInfo *info = (BgmInfo *)info_in;
    BgmStream *s = g_bgmStream;
    const BgmSubsystem *plugin;
    BgmPluginIo io;
    BgmInfo copy;
    Stream *file;
    int err;

    if (s == NULL) {
        return 0;
    }
    Bgm_StreamClose(s);
    plugin = Bgm_FindSubsystem(info->path);
    if (plugin == NULL) {
        MatchLog_Printf(5, "playback system not found for %s!", info->path);
        return 1;
    }
    if (s->plugin != plugin) {
        if (s->plugin != NULL && s->instance != NULL) {
            s->plugin->destroy(s->instance);
        }
        s->plugin = plugin;
        s->instance = plugin->create();
        if (s->instance == NULL) {
            MatchLog_Printf(3, "error opening subsystem instance for %s!\n", info->path);
            return 0;
        }
    }
    file = Stream_OpenFile(info->path, "rb");
    if (file == NULL) {
        MatchLog_Printf(3, "Couldn't open BGM file %s!\n", info->path);
        return 0;
    }
    if (info->loop_end != -1 && info->loop_end <= info->loop_start) {
        info->loop_end = -1;
    }
    if (info->loop_start < -1) {
        info->loop_start = -1;
    }
    if (info->loop_end < -1) {
        info->loop_end = -1;
    }
    io.handle = file;
    copy = *info;
    io.eof = (BgmIoEofFn)Stream_Eof;
    io.read = (BgmIoReadFn)Stream_Read;
    io.seek = (BgmIoSeekFn)Stream_Seek;
    io.tell = (BgmIoTellFn)Stream_Tell;
    {
        /* Moemon Fight: through a real BgmOpenResult (on 64-bit, BgmStream's mode and format don't sit
         * where the original's cast puts them) */
        BgmOpenResult res;
        memset(&res, 0, sizeof(res));
        res.mode = s->mode;
        res.format = s->format;
        err = plugin->open(s->instance, &io, &copy, &res);
        s->mode = res.mode;
        s->format = res.format;
    }
    if (err != 0) {
        MatchLog_Printf(3, "Error loading file %s: %s\n", info->path, s->plugin->strerror(s->instance, err));
        Stream_Close(file);
        return 1;
    }
    s->info = *info;
    s->file = file;
    s->volume = info->volume;
    if (s->mode == BGM_MODE_PLUGIN) {
        plugin->set_mute(s->instance, s->muted);
    } else {
        Bgm_StreamSetupResampler(&s->format, s, (int)file);
    }
    return 1;
}

/*
 * 0x0049BF40: what loading info means for the current stream: 1 for no BGM,
 * 0 when it is the playing BGM (same path, case-insensitive, and loop
 * setting), else 2.
 */
int Bgm_StreamCompare(const BgmInfo *info)
{
    BgmStream *s = g_bgmStream;

    if (info == NULL || info->path[0] == '\0') {
        return 1;
    }
    if (s->playing && _stricmp(s->info.path, info->path) == 0 && s->info.loop == info->loop) {
        return 0;
    }
    return 2;
}

/* 0x0049BF90: stops the current stream (it stays loaded) and resets its resampler. */
void Bgm_StreamStop(void)
{
    BgmStream *s = g_bgmStream;

    if (s == NULL) {
        return;
    }
    if (s->plugin != NULL) {
        s->plugin->stop(s->instance);
    }
    s->playing = 0;
    s->flags = 0;
    if (s->resampler != NULL) {
        s->resampler->vtbl->reset(s->resampler);
    }
    s->format_changed = 0;
}

/*
 * 0x0049BFF0: closes the file of s (the plugin instance and the resampler
 * are kept) and forgets its BGM.
 */
void Bgm_StreamClose(BgmStream *s)
{
    if (s == NULL) {
        return;
    }
    if (s->plugin != NULL) {
        s->plugin->close(s->instance);
    }
    if (s->file != NULL) {
        Stream_Close(s->file);
        s->file = NULL;
    }
    memset(&s->info, 0, sizeof(BgmInfo));
    s->volume = 0.0;
    s->playing = 0;
    s->flags = 0;
    if (s->resampler != NULL) {
        s->resampler->vtbl->reset(s->resampler);
    }
    s->format_changed = 0;
}

/*
 * 0x0049C070: a new, empty stream decoding one mixer chunk per ring block.
 * Not stored: the caller sets g_bgmStream. (For an unknown mixer sample
 * format the original multiplies by whatever its caller left in ecx; 0
 * here.)
 */
BgmStream *Bgm_StreamCreate(void)
{
    BgmStream *s = calloc(1, sizeof(BgmStream));

    if (s == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    s->block_bytes = g_bgmFormat.channels * g_bgmChunk * Bgm_SampleBytes((unsigned char)g_bgmFormat.format, 0);
    return s;
}

/* 0x0049C0E0: frees the current stream: resampler and decode buffer, plugin instance, file. */
void Bgm_StreamFree(void)
{
    BgmStream *s = g_bgmStream;

    if (s->resampler != NULL) {
        s->resampler->vtbl->close(s->resampler);
        free(s->decode_buf);
    }
    if (s->instance != NULL) {
        s->plugin->destroy(s->instance);
    }
    if (s->file != NULL) {
        Stream_Close(s->file);
    }
    free(s);
}

/*
 * 0x0049C140: decodes, resamples and converts one ring block (block_bytes)
 * of the current decoded stream into dst: data read from the plugin is
 * given to the resampler block by block (events of BgmReadFn), the
 * resampled frames read back. A format change (BGM_READ_EVENT_FORMAT)
 * takes effect when the resampler is drained. Returns the bytes written;
 * *status: BGM read status 1 (more), 2 (the stream ended) or 0 (nothing
 * playing, or a plugin error: logged, the stream is closed). Quirk: status
 * may only be NULL when nothing plays.
 */
unsigned int Bgm_StreamRead(void *dst, int *status)
{
    BgmStream *s = g_bgmStream;
    unsigned int pos = 0;
    unsigned int out = 0;
    int ended = 0;
    unsigned int in_frame, out_frame;

    if (s->mode == BGM_MODE_PLUGIN || !s->playing) {
        if (status != NULL) {
            *status = 0;
        }
        return 0;
    }
    in_frame = Wav_FrameSize(s->format.format, s->format.channels);
    out_frame = Wav_FrameSize(g_bgmFormat.format, g_bgmFormat.channels);
    while (out < s->block_bytes && !ended) {
        int st = s->resampler->vtbl->state(s->resampler);
        while (st == RESAMPLER_INPUT) {
            unsigned int got;
            int event;
            int err = s->plugin->read(s->instance, s->decode_buf + pos, s->decode_size - pos, &got, &event);
            pos += got;
            if (err != 0) {
                MatchLog_Printf(3, "Subsystem returned error: %s\n", s->plugin->strerror(s->instance, err));
                *status = 0;
                Bgm_StreamClose(s);
                return 0;
            }
            switch (event) {
            case BGM_READ_EVENT_BLOCK:
                s->resampler->vtbl->write(s->resampler, s->decode_buf, pos / in_frame, 0);
                pos = 0;
                break;
            case BGM_READ_EVENT_END:
                s->resampler->vtbl->write(s->resampler, s->decode_buf, pos / in_frame, 1);
                pos = 0;
                break;
            case BGM_READ_EVENT_FORMAT:
                s->format_changed = 1;
                s->resampler->vtbl->write(s->resampler, s->decode_buf, pos / in_frame, 0);
                break;
            }
            st = s->resampler->vtbl->state(s->resampler);
        }
        if (st == RESAMPLER_OUTPUT) {
            unsigned int n = s->resampler->vtbl->read(s->resampler, (unsigned char *)dst + out,
                                                      (s->block_bytes - out) / out_frame);
            out += n * out_frame;
        } else if (st == RESAMPLER_DONE) {
            if (s->format_changed == 1) {
                AudioFormat format;
                s->plugin->get_format(s->instance, &format);
                Bgm_StreamSetupResampler(&format, s, (int)&format);
            } else {
                ended = 1;
            }
        }
    }
    *status = ended + 1;
    return out;
}
