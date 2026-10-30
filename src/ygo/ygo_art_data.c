// Moemon Kai: the Yu-Gi-Oh! card pictures, built in with the assembler's .incbin like src/tcg/tcg_art_data.c.
//   graphics/ygo/printed.bin  the printed cards and where each one's illustration sits (tools/ygo/build_printed.py)
//   graphics/ygo/sdd_art.bin  Stairway to the Destined Duel's own pictures (tools/ygo/import_sdd.py)
// json_data_rules.mk makes this file depend on both.
#include "global.h"

#ifdef _WIN32
#define YGO_ART_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define YGO_ART_SECTION ".section .rodata\n"
#endif

__asm__(
    YGO_ART_SECTION
    ".balign 16\n"
    ".global gYgoPrintedArt\n"
    "gYgoPrintedArt:\n"
    ".incbin \"graphics/ygo/printed.bin\"\n"
    ".global gYgoPrintedArtEnd\n"
    "gYgoPrintedArtEnd:\n"
    ".byte 0\n"
    ".balign 16\n"
    ".global gYgoSddArt\n"
    "gYgoSddArt:\n"
    ".incbin \"graphics/ygo/sdd_art.bin\"\n"
    ".global gYgoSddArtEnd\n"
    "gYgoSddArtEnd:\n"
    ".byte 0\n"
    ".text\n");
