/* The CRT turning on (a white line widens, then opens into the picture) and off (the picture collapses into a line
 * that shrinks to a point): the VS cabinet's fights and the DANCE machine share it (src/platform/crt_effect.c). */
#ifndef GUARD_PLATFORM_CRT_H
#define GUARD_PLATFORM_CRT_H

#include <stdint.h>

enum { CRT_FMT_RGB565, CRT_FMT_BGR555 };   /* the fight's picture / the game's (bit 15: drawn) */
enum { CRT_PHASE_NONE, CRT_PHASE_ON, CRT_PHASE_OFF };

/* dst = src as the CRT shows it at progress p (0..1) of the phase; w x h pixels */
void Crt_Apply(const uint16_t *src, uint16_t *dst, int w, int h, int phase, double p, int fmt);

#endif
