/*
 * WAV reader and resampler (original module 0x4886E0-0x48944F).
 *
 * Wav_ReadInfo parses the RIFF/WAVE header of a stream (PCM only, 8 or 16
 * bits, mono or stereo) and WavReader reads its frames. The resampler
 * converts blocks of frames to a target AudioFormat (the mixer's): through
 * the sample converters of Audio_GetConverter (0x49AB10) and, when the rates
 * differ, libresample working on floats. Sessions are used through the
 * function table at 0x4F1890 (g_resamplerVtbl).
 *
 * Quirks kept from the original:
 * - Wav_ReadInfo reports a broken "fmt " chunk's channel count read as
 *   WAV_ERR_CHANNELS but a bad channel count as WAV_ERR_FORMAT, does not
 *   count the pad byte of unknown chunks in its position, and fails with
 *   WAV_ERR_CORRUPT when "data" comes before "fmt ";
 * - Resampler_Close frees the session twice when it has no state;
 * - the inlined frame size code of WavReader_Read, Resampler_Write and
 *   Resampler_Read falls back to an uninitialised stack slot for sample
 *   formats without a known size bit (Resampler_Read's slot holds its session
 *   argument, kept here; the others use the format value like Wav_FrameSize).
 *   Only the known formats reach them.
 */
#include <libresample.h>

#include "mugen.h"

#define RIFF_ID_RIFF 0x46464952u    /* "RIFF" */
#define RIFF_ID_WAVE 0x45564157u    /* "WAVE" */
#define RIFF_ID_FMT  0x20746D66u    /* "fmt " */
#define RIFF_ID_DATA 0x61746164u    /* "data" */

#define WAV_RESAMPLER_BEGIN Resampler_Begin

/* Bytes per sample of a sample format (the size bits), or `other` without one. */
static unsigned int Wav_SampleSize(int format, unsigned int other)
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
    return other;
}

/* 0x004886E0: bytes per frame of a sample format (the format value itself without a size bit). */
int Wav_FrameSize(int format, int channels)
{
    return (int)Wav_SampleSize(format, (unsigned int)format) * channels;
}

/*
 * 0x00488730: reads the "fmt " chunk of size bytes into info. Returns
 * WAV_OK or a WAV_ERR_* code.
 */
int Wav_ReadFmt(Stream *s, WavInfo *info, unsigned int size)
{
    int extension = size > 16;
    unsigned short v16;
    unsigned int v32;

    if (Stream_ReadU16(s, &v16) == -1) {
        return WAV_ERR_CORRUPT;
    }
    info->format_tag = v16;
    if (v16 != WAV_TAG_PCM) {
        return WAV_ERR_FORMAT;
    }
    if (Stream_ReadU16(s, &v16) == -1) {
        return WAV_ERR_CHANNELS;
    }
    if ((unsigned short)(v16 - 1) > 1) {
        return WAV_ERR_FORMAT;
    }
    info->format.channels = v16;
    if (Stream_ReadU32(s, &v32) != 0) {
        return WAV_ERR_CORRUPT;
    }
    info->format.rate = (double)v32;
    if (Stream_ReadU32(s, &v32) != 0 || Stream_ReadU16(s, &v16) == -1 || Stream_ReadU16(s, &v16) == -1) {
        return WAV_ERR_CORRUPT;
    }
    if (v16 == 8) {
        info->format.format = AUDIO_FMT_U8;
    } else if (v16 == 16) {
        info->format.format = AUDIO_FMT_S16;
    } else {
        return WAV_ERR_SAMPLE_SIZE;
    }
    if (extension) {
        if (Stream_ReadU16(s, &v16) == -1) {
            return WAV_ERR_CORRUPT;
        }
        if (size > 18 && Stream_Seek(s, (long)(size - 18), SEEK_CUR) != 0) {
            return WAV_ERR_CORRUPT;
        }
    }
    return WAV_OK;
}

/*
 * 0x00488890: reads the RIFF/WAVE header of s: the "fmt " chunk and the
 * position and size of the "data" chunk. info is only written on success.
 * Returns WAV_OK or a WAV_ERR_* code.
 */
int Wav_ReadInfo(Stream *s, WavInfo *info)
{
    WavInfo wi;
    unsigned int size, pos, id, len;
    int have_fmt = 0, have_data = 0;
    int err;

    Stream_Seek(s, 0, SEEK_END);
    size = (unsigned int)Stream_Tell(s);
    if (size < 12) {
        return WAV_ERR_FORMAT;
    }
    Stream_Seek(s, 0, SEEK_SET);
    if (Stream_ReadU32(s, &id) != 0 || id != RIFF_ID_RIFF || Stream_ReadU32(s, &len) != 0 ||
        Stream_ReadU32(s, &id) != 0 || id != RIFF_ID_WAVE) {
        return WAV_ERR_FORMAT;
    }
    pos = 12;
    while (pos < size) {
        if (Stream_ReadU32(s, &id) != 0 || Stream_ReadU32(s, &len) != 0) {
            break;
        }
        pos = len + pos + 8;
        if (id == RIFF_ID_FMT) {
            err = Wav_ReadFmt(s, &wi, len);
            if (err != WAV_OK) {
                return err;
            }
            have_fmt = 1;
        } else if (id == RIFF_ID_DATA) {
            if (!have_fmt) {
                return WAV_ERR_CORRUPT;
            }
            wi.data_size = len;
            wi.frames = len / (unsigned int)Wav_FrameSize(wi.format.format, wi.format.channels);
            wi.data_offset = Stream_Tell(s);
            if (Stream_Seek(s, (long)len, SEEK_CUR) != 0) {
                return WAV_ERR_CORRUPT;
            }
            if ((len & 1) && pos < size) {
                pos++;
                if (Stream_Seek(s, 1, SEEK_CUR) != 0) {
                    return WAV_ERR_CORRUPT;
                }
            }
            have_data = 1;
        } else {
            if (len & 1) {
                len++;
            }
            Stream_Seek(s, (long)len, SEEK_CUR);
        }
    }
    if (!have_fmt || !have_data) {
        return WAV_ERR_CORRUPT;
    }
    *info = wi;
    return WAV_OK;
}

/*
 * 0x00488AF0: a reader of the PCM frames of s described by info, positioned
 * at the start of the data. *status (when not NULL) gets WAV_OK or the error.
 */
WavReader *WavReader_Open(const WavInfo *info, Stream *s, int *status)
{
    WavReader *r;

    if (info->format_tag != WAV_TAG_PCM) {
        if (status != NULL) {
            *status = WAV_ERR_UNKNOWN;
        }
        return NULL;
    }
    r = calloc(1, sizeof(WavReader));
    if (r == NULL) {
        g_outOfMemory = 1;
        if (status != NULL) {
            *status = WAV_ERR_NOMEM;
        }
        return NULL;
    }
    r->info = *info;
    r->stream = s;
    r->unused_2c = 0;
    if (Stream_Seek(s, info->data_offset, SEEK_SET) != 0) {
        free(r);
        if (status != NULL) {
            *status = WAV_ERR_IO;
        }
        return NULL;
    }
    r->frames_read = 0;
    if (status != NULL) {
        *status = WAV_OK;
    }
    return r;
}

/* 0x00488BB0: reads up to frames frames into buf; returns the number read. */
unsigned int WavReader_Read(WavReader *r, unsigned int frames, void *buf)
{
    unsigned int left = r->info.frames - r->frames_read;
    unsigned int n;

    if (frames > left) {
        frames = left;
    }
    if (r->info.format_tag != WAV_TAG_PCM) {
        return 0;
    }
    n = (unsigned int)Stream_Read(buf, r->info.format.channels *
                                           Wav_SampleSize(r->info.format.format, (unsigned int)r->info.format.format),
                                  frames, r->stream);
    r->frames_read += n;
    return n;
}

/* 0x00488C20: message of a WAV_* status. */
const char *Wav_StrError(int status)
{
    switch (status) {
    case WAV_OK:
        return "OK";
    case WAV_ERR_FORMAT:
        return "Unsupported sample format (only PCM supported)";
    case WAV_ERR_CORRUPT:
        return "Corrupted or unrecognized WAV file";
    case WAV_ERR_CHANNELS:
        return "Unsupported number of channels (only mono or stereo supported)";
    case WAV_ERR_SAMPLE_SIZE:
        return "Unsupported sample size (only 8-bit or 16-bit supported)";
    case WAV_ERR_NOMEM:
        return "Out of memory";
    case WAV_ERR_IO:
        return "I/O error";
    case WAV_ERR_RATE:
        return "Unsupported sample rate";
    case WAV_ERR_UNKNOWN:
        return "Unknown error";
    default:
        return "Unrecognized wave status code (library's fault)";
    }
}

/* 0x00488C90: Wav_FrameSize for formats other than 8-bit ones (no 1-byte size bit). */
int Audio_FrameSize(int format, int channels)
{
    unsigned int bytes;

    if (format & 0x08) {
        bytes = 2;
    } else if (format & 0x10) {
        bytes = 4;
    } else if (format & 0x20) {
        bytes = 8;
    } else {
        bytes = (unsigned int)format;
    }
    return (int)bytes * channels;
}

/* (double) of an unsigned int, as the original computes it (fild, + 2^32 when negative). */
static double Wav_UnsignedToDouble(unsigned int v)
{
    return (double)v;
}

/*
 * 0x00488CD0: starts converting frames of format in to format out, in output
 * blocks of chunk frames; quality is libresample's. Returns NULL when a rate
 * is not positive, a format has no converter, or memory runs out.
 */
ResamplerSession *Resampler_Open(const AudioFormat *out, const AudioFormat *in, int chunk, const int *quality)
{
    double ratio = out->rate / in->rate;
    int q = *quality;
    int fail = 0;
    int resampling;
    AudioFormat mid;
    AudioConvertFn convert_in, convert_out = NULL;
    ResamplerState *st;
    ResamplerSession *sess;
    unsigned int frame;

    if (!(0.0 < out->rate) || !(0.0 < in->rate)) {
        return NULL;
    }
    mid.channels = out->channels <= in->channels ? out->channels : in->channels;
    if (in->rate == out->rate) {
        convert_in = Audio_GetConverter(in, out);
        if (convert_in == NULL) {
            return NULL;
        }
    } else {
        mid.rate = in->rate;
        mid.format = AUDIO_FMT_F32;
        convert_in = Audio_GetConverter(in, &mid);
        convert_out = Audio_GetConverter(&mid, out);
        if (convert_in == NULL || convert_out == NULL) {
            return NULL;
        }
    }
    st = calloc(1, sizeof(ResamplerState));
    if (st == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    sess = calloc(1, sizeof(ResamplerSession));
    if (sess == NULL) {
        g_outOfMemory = 1;
        free(st);
        return NULL;
    }
    sess->vtbl = &g_resamplerVtbl;
    sess->state = st;
    resampling = in->rate != out->rate;
    st->resampling = resampling;
    st->convert_in = convert_in;
    st->convert_out = convert_out;
    st->chunk = (unsigned int)chunk;
    st->in_block = (unsigned int)Trunc64(ceil(Wav_UnsignedToDouble((unsigned int)chunk) / ratio));
    if (st->resampling) {
        double out_frames = ceil(Wav_UnsignedToDouble(st->in_block) * ratio);
        double margin = ceil(ratio);
        st->buf_frames = Trunc64(margin + margin + out_frames);
        st->rs = resample_open(q, mid.channels, ratio, ratio);
        st->in_float = Mem_Calloc(st->in_block, mid.channels * 4);
        st->out_float = Mem_Calloc(st->buf_frames, mid.channels * 4);
        if (st->in_float == NULL || st->out_float == NULL || st->rs == NULL) {
            fail = 1;
        }
        st->mid = mid;
    } else {
        st->rs = NULL;
        st->in_float = NULL;
        st->out_float = NULL;
    }
    st->out_capacity = st->resampling ? (unsigned int)st->buf_frames * 2 : (unsigned int)chunk * 2;
    frame = (out->format & 0x04) ? (unsigned int)out->channels : (unsigned int)Audio_FrameSize(out->format, out->channels);
    st->out_buf = Mem_Calloc(st->out_capacity, frame);
    if (st->out_buf == NULL || fail) {
        if (st->rs != NULL) {
            resample_close(st->rs);
        }
        free(st->out_buf);
        free(st->in_float);
        free(st->out_float);
        free(sess);
        free(st);
        return NULL;
    }
    st->out = *out;
    st->in = *in;
    return sess;
}

/* 0x00489020: ends a conversion. Quirk: a session without state is freed twice. */
void Resampler_Close(ResamplerSession *sess)
{
    ResamplerState *st;

    if (sess == NULL || sess->state == NULL) {
        free(sess);
        free(sess);
        return;
    }
    st = sess->state;
    if (st->rs != NULL) {
        resample_close(st->rs);
    }
    free(st->in_float);
    free(st->out_float);
    free(st->out_buf);
    free(st);
    free(sess);
}

/* 0x00489080: input frames to write next: a block while an output block is not ready, else 0. */
unsigned int Resampler_InputNeeded(ResamplerSession *sess)
{
    const ResamplerState *st = sess->state;

    return st->out_count < st->chunk ? st->in_block : 0;
}

/* 0x004890A0: RESAMPLER_OUTPUT when frames can be read, RESAMPLER_INPUT or RESAMPLER_DONE otherwise. */
int Resampler_State(ResamplerSession *sess)
{
    const ResamplerState *st = sess->state;

    if (st->out_count >= st->chunk) {
        return RESAMPLER_OUTPUT;
    }
    if (st->last != 0 && st->out_count > 0) {
        return RESAMPLER_OUTPUT;
    }
    return st->last != 0 ? RESAMPLER_DONE : RESAMPLER_INPUT;
}

/*
 * 0x004890D0: converts up to an input block of frames from in; last marks
 * the final block (dropped when more than a block is given). Returns the
 * frames consumed (0 after the final block, or for a short non-final block).
 */
unsigned int Resampler_Write(ResamplerSession *sess, const void *in, unsigned int frames, int last)
{
    ResamplerState *st = sess->state;
    unsigned int need = st->out_count < st->chunk ? st->in_block : 0;
    int channels = st->out.channels < st->in.channels ? st->out.channels : st->in.channels;
    unsigned int out_frame;
    unsigned int used = 0;

    if (st->last != 0) {
        return 0;
    }
    if (!last && frames < need) {
        return 0;
    }
    if (frames > need) {
        frames = need;
        last = 0;
    }
    out_frame = (unsigned int)st->out.channels * Wav_SampleSize(st->out.format, (unsigned int)st->out.format);
    if (st->resampling) {
        double ratio = st->out.rate / st->in.rate;
        int produced = 0;
        int consumed = 0;
        st->convert_in(st->in_float, in, frames, channels, st->in.channels);
        while (used < frames) {
            if ((unsigned int)produced >= st->out_capacity - st->out_count) {
                break;
            }
            produced += resample_process(st->rs, ratio, st->in_float + st->mid.channels * used, (int)(frames - used),
                                         last, &consumed, st->out_float + st->mid.channels * produced,
                                         st->buf_frames - produced);
            used += (unsigned int)consumed;
        }
        st->convert_out(st->out_buf + st->out_count * out_frame, st->out_float, (unsigned int)produced,
                        st->out.channels, channels);
        st->out_count += (unsigned int)produced;
        st->last = last;
        return used;
    }
    st->convert_in(st->out_buf + st->out_count * out_frame, in, frames, st->out.channels, st->in.channels);
    st->out_count += frames;
    st->last = last;
    return frames;
}

/*
 * 0x00489280: copies up to frames converted frames (at most a block) to out;
 * returns the number copied (0 while a block is not ready, except at the end).
 */
unsigned int Resampler_Read(ResamplerSession *sess, void *out, unsigned int frames)
{
    ResamplerState *st = sess->state;
    unsigned int n = st->out_count;
    unsigned int frame, bytes;

    if (n < st->chunk && (st->last == 0 || n == 0)) {
        return 0;
    }
    if (n > st->chunk) {
        n = st->chunk;
    }
    if (n > frames) {
        n = frames;
    }
    frame = (unsigned int)st->out.channels * Wav_SampleSize(st->out.format, (unsigned int)(size_t)sess);
    bytes = frame * n;
    memcpy(out, st->out_buf, bytes);
    st->out_count -= n;
    memmove(st->out_buf, st->out_buf + bytes, st->out_count * frame);
    return n;
}

/*
 * 0x00489330: a converter to out in blocks of chunk frames, with the given
 * libresample quality; its begin function opens sessions from a WavInfo.
 */
Resampler *Resampler_Create(const AudioFormat *out, int chunk, const int *quality)
{
    Resampler *r = calloc(1, sizeof(Resampler));
    int *q;

    if (r == NULL) {
        g_outOfMemory = 1;
    }
    q = calloc(1, sizeof(int));
    if (q == NULL) {
        g_outOfMemory = 1;
    }
    if (r == NULL || q == NULL) {
        free(r);
        free(q);
        return NULL;
    }
    r->out = *out;
    r->vtbl = &g_resamplerVtbl;
    *q = *quality;
    r->quality = q;
    r->chunk = chunk;
    r->begin = WAV_RESAMPLER_BEGIN;
    return r;
}

/* 0x004893D0: Resampler.begin: a session converting frames of the format of in to r's. */
ResamplerSession *Resampler_Begin(Resampler *r, const WavInfo *in)
{
    return Resampler_Open(&r->out, &in->format, r->chunk, r->quality);
}

/* 0x004893F0: the input format of a session. */
const AudioFormat *Resampler_InFormat(ResamplerSession *sess)
{
    return &sess->state->in;
}

/* 0x00489400: the output format of a session. */
const AudioFormat *Resampler_OutFormat(ResamplerSession *sess)
{
    return &sess->state->out;
}

/* 0x00489410: drops the frames waiting and the end mark, to convert a new stream. */
void Resampler_Reset(ResamplerSession *sess)
{
    ResamplerState *st = sess->state;

    st->out_count = 0;
    st->last = 0;
    if (st->resampling) {
        resample_reset(st->rs);
    }
}

/* 0x00489430: output block size in frames. */
unsigned int Resampler_OutBlock(ResamplerSession *sess)
{
    return sess->state->chunk;
}

/* 0x00489440: input block size in frames. */
unsigned int Resampler_InBlock(ResamplerSession *sess)
{
    return sess->state->in_block;
}
