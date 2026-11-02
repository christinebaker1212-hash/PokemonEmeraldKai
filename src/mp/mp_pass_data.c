// Moemon Co-op: the TRAINER CARD's designs (graphics/moemon/cards/passes.bin from tools/moemon_cards/build_passes.py:
// Pokemon Battle Revolution's Battle Pass designs), built in with the assembler's .incbin like the PMD sheets (mp_pmd_data.c)
#include "global.h"

#ifdef _WIN32
#define PASS_SECTION ".section .rdata,\"dr\"\n"
#else
#define PASS_SECTION ".section .rodata\n"
#endif

__asm__(
    PASS_SECTION
    ".balign 16\n"
    ".global gMpPassData\n"
    "gMpPassData:\n"
    ".incbin \"graphics/moemon/cards/passes.bin\"\n"
    ".global gMpPassDataEnd\n"
    "gMpPassDataEnd:\n"
    ".byte 0\n"
    ".text\n");
