// Moemon Kai: the Triple Triad card pictures, built in with the assembler's .incbin like src/ygo/ygo_art_data.c.
//   graphics/tt/cards.bin  Final Fantasy XIV's card faces at two sizes, their icons, the back, the plate and the
//                          star and type icons (tools/tt/build_art.py)
#include "global.h"

#ifdef _WIN32
#define TT_ART_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define TT_ART_SECTION ".section .rodata\n"
#endif

__asm__(
    TT_ART_SECTION
    ".balign 16\n"
    ".global gTtCardArt\n"
    "gTtCardArt:\n"
    ".incbin \"graphics/tt/cards.bin\"\n"
    ".global gTtCardArtEnd\n"
    "gTtCardArtEnd:\n"
    ".byte 0\n"
    ".text\n");
