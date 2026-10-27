/* Moemon Fight: musl's internal libm.h (libm_musl.h) for the bundled math: little endian, no FMA, no hidden
 * visibility (PE has none), and the functions under the names in mfm_names.h */
#ifndef MFM_LIBM_H
#define MFM_LIBM_H
#include <stdint.h>
#include <float.h>
#include <math.h>
#include "mfm_names.h"
#define hidden
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __BYTE_ORDER __LITTLE_ENDIAN
#undef __FP_FAST_FMA
#include "libm_musl.h"
#endif
