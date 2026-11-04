// Moemon Kai: the Magic card pack, Forge's scripts for the pool (graphics/mtg/forge.pak, tools/mtg/build_pack.py),
// built in with the assembler's .incbin and read once by mtg_integration.c Mtg_EnsureDb. json_data_rules.mk makes
// this file depend on it.
/* (no headers: only the data) */

#ifdef _WIN32
#define MTG_PAK_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define MTG_PAK_SECTION ".section .rodata\n"
#endif

__asm__(
    MTG_PAK_SECTION
    ".balign 16\n"
    ".global gMtgForgePak\n"
    "gMtgForgePak:\n"
    ".incbin \"graphics/mtg/forge.pak\"\n"
    ".global gMtgForgePakEnd\n"
    "gMtgForgePakEnd:\n"
    ".byte 0\n"
    ".text\n");
