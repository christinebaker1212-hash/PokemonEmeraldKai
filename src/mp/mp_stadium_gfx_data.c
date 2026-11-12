// Moemon Kai: POKéMON STADIUM's own pictures for its screens (graphics/stadium/stadium.bin from
// tools/stadium/extract_gfx.py and the user's ROM), built in with the assembler's .incbin like the badges
#include "global.h"

#ifdef _WIN32
#define STADIUM_SECTION ".section .rdata,\"dr\"\n"
#else
#define STADIUM_SECTION ".section .rodata\n"
#endif

__asm__(
    STADIUM_SECTION
    ".balign 16\n"
    ".global gMpStadiumGfx\n"
    "gMpStadiumGfx:\n"
    ".incbin \"graphics/stadium/stadium.bin\"\n"
    ".global gMpStadiumGfxEnd\n"
    "gMpStadiumGfxEnd:\n"
    ".byte 0\n"
    ".balign 16\n"
    ".global gMpStadiumThemes\n"   // STADIUM's themes' pieces (tools/stadium/restyle/build_themes.py)
    "gMpStadiumThemes:\n"
    ".incbin \"graphics/stadium/themes.bin\"\n"
    ".byte 0\n"
    ".text\n");
