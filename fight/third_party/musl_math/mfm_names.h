/*
 * Moemon Fight: the fight engine's math (musl 1.2.5's, MIT, see COPYRIGHT) under names of its own, so every
 * player's build computes the same results whatever the C library or CPU (the engine calls these through
 * fight/compat/mf_port.h: sin is mfm_sin there).
 */
#ifndef MFM_NAMES_H
#define MFM_NAMES_H
#define sin mfm_sin
#define cos mfm_cos
#define tan mfm_tan
#define log mfm_log
#define exp mfm_exp
#define pow mfm_pow
#define atan mfm_atan
#define asin mfm_asin
#define acos mfm_acos
#define __sin mfm__sin
#define __cos mfm__cos
#define __tan mfm__tan
#define __rem_pio2 mfm__rem_pio2
#define __rem_pio2_large mfm__rem_pio2_large
#define __math_divzero mfm__math_divzero
#define __math_invalid mfm__math_invalid
#define __math_oflow mfm__math_oflow
#define __math_uflow mfm__math_uflow
#define __math_xflow mfm__math_xflow
#define __exp_data mfm__exp_data
#define __log_data mfm__log_data
#define __pow_log_data mfm__pow_log_data
#endif
