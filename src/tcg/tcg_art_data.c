// Moemon Kai: the card pictures, built in with the assembler's .incbin like mp_pmd_data.c.
//   graphics/tcg/original.bin  the Game Boy illustrations (tools/tcg/build_original_art.py)
//   graphics/tcg/printed.bin   the printed cards and where each one's illustration sits
//                              (tools/tcg/build_printed_art.py)
// json_data_rules.mk makes this file depend on both.
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
    ".balign 16\n"
    ".global gTcgPrintedArt\n"
    "gTcgPrintedArt:\n"
    ".incbin \"graphics/tcg/printed.bin\"\n"
    ".global gTcgPrintedArtEnd\n"
    "gTcgPrintedArtEnd:\n"
    ".byte 0\n"
    ".text\n");
