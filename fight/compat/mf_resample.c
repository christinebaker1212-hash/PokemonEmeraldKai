/*
 * Moemon Fight: the libresample calls of the engine (Elecbyte's multichannel build), as linear
 * interpolation. Everything ends at 13379 Hz in 8 bits (mf_mixer.c), where a finer filter
 * would not be heard.
 */
#include <stdlib.h>
#include <string.h>

#include "libresample.h"

#define RS_MAX_CHANNELS 8

typedef struct Resampler {
    int channels;
    double pos;                     /* position of the next output frame, from the previous input frame */
    float prev[RS_MAX_CHANNELS];    /* the last input frame of the previous call */
    int have_prev;
} Resampler;

int __cdecl resample_init(int low_quality, int high_quality)
{
    return 0;
}

void __cdecl resample_deinit(void)
{
}

void *__cdecl resample_open(int quality, int channels, double min_factor, double max_factor)
{
    Resampler *r;

    if (channels < 1 || channels > RS_MAX_CHANNELS) {
        return NULL;
    }
    r = calloc(1, sizeof(Resampler));
    if (r != NULL) {
        r->channels = channels;
    }
    return r;
}

void __cdecl resample_reset(void *handle)
{
    Resampler *r = handle;

    if (r != NULL) {
        r->pos = 0.0;
        r->have_prev = 0;
        memset(r->prev, 0, sizeof(r->prev));
    }
}

void __cdecl resample_close(void *handle)
{
    free(handle);
}

/*
 * factor = output rate / input rate. in and out hold interleaved frames of r->channels floats;
 * in_len and out_len count frames. Returns the frames written; *in_used the frames consumed.
 * Output frame k sits at input position pos + k / factor, between frame i (prev for -1) and i + 1.
 */
int __cdecl resample_process(void *handle, double factor, float *in, int in_len, int last, int *in_used,
                             float *out, int out_len)
{
    Resampler *r = handle;
    double step;
    int n = 0, c;

    *in_used = 0;
    if (r == NULL || factor <= 0.0 || in_len <= 0) {
        return 0;
    }
    step = 1.0 / factor;
    if (!r->have_prev) {
        for (c = 0; c < r->channels; c++) {
            r->prev[c] = in[c];
        }
        r->have_prev = 1;
    }
    /* r->pos is measured from prev (index -1) */
    while (n < out_len) {
        int i = (int)r->pos;            /* 0: between prev and in[0] */
        double t = r->pos - i;
        const float *a, *b;
        if (i >= in_len) {
            break;
        }
        a = i == 0 ? r->prev : in + (size_t)(i - 1) * r->channels;
        b = in + (size_t)i * r->channels;
        for (c = 0; c < r->channels; c++) {
            out[(size_t)n * r->channels + c] = (float)(a[c] + (b[c] - a[c]) * t);
        }
        n++;
        r->pos += step;
    }
    {
        int used = (int)r->pos;
        if (used > in_len) {
            used = in_len;
        }
        if (n < out_len || used == in_len) {
            used = in_len;
        }
        if (used > 0) {
            for (c = 0; c < r->channels; c++) {
                r->prev[c] = in[(size_t)(used - 1) * r->channels + c];
            }
        }
        r->pos -= used;
        if (r->pos < 0.0) {
            r->pos = 0.0;
        }
        *in_used = used;
    }
    return n;
}
