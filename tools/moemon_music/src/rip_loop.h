// Moemon Music Studio: a rip's loop, found in the original sound (rip_loop.c).
#ifndef RIP_LOOP_H
#define RIP_LOOP_H
#include "rip.h"

// The loop of a rip's recording: the game frame the song starts repeating from and the period in game
// frames (to a fraction: matched to the sample). candidates: periods (frames) to try besides the
// clearest repeats of the recording (e.g. from the notes). Returns 0 when the song doesn't clearly loop.
int rip_loop_find(const Rip *r, const int *candidates, int ncand, int *outStart, double *outPeriod, int debug);

#endif
