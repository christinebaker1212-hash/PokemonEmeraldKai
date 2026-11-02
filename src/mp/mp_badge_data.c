// Moemon Co-op: the BADGE CASE's badge pictures (graphics/moemon/badges/badges.bin from tools/moemon_badges/
// build_badges.py), built in with the assembler's .incbin like the PMD sheets (mp_pmd_data.c)
#include "global.h"

#ifdef _WIN32
#define BADGE_SECTION ".section .rdata,\"dr\"\n"
#else
#define BADGE_SECTION ".section .rodata\n"
#endif

__asm__(
    BADGE_SECTION
    ".balign 16\n"
    ".global gMpBadgeData\n"
    "gMpBadgeData:\n"
    ".incbin \"graphics/moemon/badges/badges.bin\"\n"
    ".global gMpBadgeDataEnd\n"
    "gMpBadgeDataEnd:\n"
    ".byte 0\n"
    ".text\n");
