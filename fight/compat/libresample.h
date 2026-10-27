/*
 * Minimal header for the libresample.dll shipped with MUGEN 1.1 (exports in
 * third_party/sdk/def/libresample.def).
 *
 * Elecbyte's build is libresample 0.1.3 with multi-channel (interleaved)
 * buffers and a few extra exports. The signatures below were recovered from
 * the call sites in mugen.exe; all functions are __cdecl. Exports that the
 * game never calls (resample_dup, resample_get_filter_width,
 * resample_get_inbuffer_size, resample_get_outbuffer_size) are left out.
 */
#ifndef LIBRESAMPLE_H
#define LIBRESAMPLE_H

#if !defined(_WIN32) && !defined(__cdecl)
#define __cdecl
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LIBRESAMPLE_API   /* Moemon Fight: compat/mf_resample.c */

/*
 * Builds the shared filter tables before the first resample_open:
 * Sound_Init passes (quality == 0, quality == 1) for the configured
 * [Sound] SFXResampleQuality. Tables that already exist are kept. Returns
 * nonzero on success (the game ignores it).
 */
LIBRESAMPLE_API int __cdecl resample_init(int low_quality, int high_quality);

/*
 * Frees the filter tables. The pointers are not cleared, so a later
 * resample_init keeps the freed tables (the game initialises only once).
 */
LIBRESAMPLE_API void __cdecl resample_deinit(void);

/* A resampler for channels interleaved channels and factors in [min_factor, max_factor]. */
LIBRESAMPLE_API void *__cdecl resample_open(int quality, int channels, double min_factor, double max_factor);

/*
 * Resamples up to in_len input frames by factor (output rate / input rate);
 * last is nonzero for the final block. Stores the input frames consumed in
 * *in_used and returns the output frames written (at most out_len).
 */
LIBRESAMPLE_API int __cdecl resample_process(void *handle, double factor, float *in, int in_len, int last,
                                             int *in_used, float *out, int out_len);

/* Forgets buffered samples so the handle can start a new stream. */
LIBRESAMPLE_API void __cdecl resample_reset(void *handle);

LIBRESAMPLE_API void __cdecl resample_close(void *handle);

#ifdef __cplusplus
}
#endif

#endif /* LIBRESAMPLE_H */
