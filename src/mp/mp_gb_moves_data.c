// Moemon Kai: GB OPTIONS > MOVES' recordings (graphics/moemon/legacy/gbmoves.bin: tools/moemon_legacy/move_pack.py),
// built in with the assembler's .incbin as the PMD sheets are (mp_pmd_data.c). json_data_rules.mk makes this file
// depend on the pack.
#include "global.h"

#ifdef _WIN32
#define GBMOVES_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define GBMOVES_SECTION ".section .rodata\n"
#endif

__asm__(
    GBMOVES_SECTION
    ".balign 16\n"
    ".global gMpGbMovesData\n"
    "gMpGbMovesData:\n"
    ".incbin \"graphics/moemon/legacy/gbmoves.bin\"\n"
    ".byte 0\n"
    ".text\n");
