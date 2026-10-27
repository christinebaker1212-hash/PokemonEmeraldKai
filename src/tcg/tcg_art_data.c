// Moemon Kai: the Game Boy card illustrations (graphics/tcg/original.bin, built by
// tools/tcg/build_original_art.py), built in with the assembler's .incbin like mp_pmd_data.c.
// json_data_rules.mk makes this file depend on original.bin.
#include "global.h"

#ifdef _WIN32
#define TCG_ART_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define TCG_ART_SECTION ".section .rodata\n"
#endif

__asm__(
    TCG_ART_SECTION
    ".balign 16\n"
    ".global gTcgOriginalArt\n"
    "gTcgOriginalArt:\n"
    ".incbin \"graphics/tcg/original.bin\"\n"
    ".global gTcgOriginalArtEnd\n"
    "gTcgOriginalArtEnd:\n"
    ".byte 0\n"
    ".text\n");
