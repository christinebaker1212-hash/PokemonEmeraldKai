// Moemon Co-op: the PMD Collab sheets (graphics/moemon/pmd/pmd.bin, 13 MB), built in with the assembler's .incbin:
// INCBIN would spell every byte out as C text. json_data_rules.mk makes this file depend on pmd.bin.
#include "global.h"

#ifdef _WIN32
#define PMD_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define PMD_SECTION ".section .rodata\n"
#endif

__asm__(
    PMD_SECTION
    ".balign 16\n"
    ".global gMpPmdData\n"
    "gMpPmdData:\n"
    ".incbin \"graphics/moemon/pmd/pmd.bin\"\n"
    ".global gMpPmdDataEnd\n"
    "gMpPmdDataEnd:\n"
    ".byte 0\n"
    ".text\n");
