// Moemon Music Studio: VGM (Sega Genesis / Mega Drive) rips. (being written)
#include <stdio.h>
extern "C" {
#include "rip.h"
}
extern "C" int rip_open_vgm(RipEmu *e, Rip *r, char *err, int errlen) {
    (void)e; (void)r;
    snprintf(err, errlen, "VGM isn't supported yet");
    return 0;
}
