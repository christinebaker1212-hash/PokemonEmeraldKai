// Moemon Kai: the printed Magic cards (Scryfall's scans of each card's old-frame printing, the illustration's place,
// the artist: graphics/mtg/printed.bin from tools/mtg/build_printed.py), built in with the assembler's .incbin like
// src/ygo/ygo_art_data.c. json_data_rules.mk makes this file depend on it.
/* (no headers: only the data) */

#ifdef _WIN32
#define MTG_ART_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define MTG_ART_SECTION ".section .rodata\n"
#endif

__asm__(
    MTG_ART_SECTION
    ".balign 16\n"
    ".global gMtgPrintedArt\n"
    "gMtgPrintedArt:\n"
    ".incbin \"graphics/mtg/printed.bin\"\n"
    ".global gMtgPrintedArtEnd\n"
    "gMtgPrintedArtEnd:\n"
    ".byte 0\n"
    ".text\n");
