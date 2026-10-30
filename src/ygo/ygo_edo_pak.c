// Moemon Kai: EDOPro's card database, card scripts, strings and Forbidden & Limited lists for the Yu-Gi-Oh! duels,
// built in with the assembler's .incbin: graphics/ygo/edo.pak (tools/ygo/edo/build_pack.py; read by ygo_edo_data.c).
// json_data_rules.mk makes this file depend on it.
/* (no headers: only the data) */

#ifdef _WIN32
#define YGO_EDO_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define YGO_EDO_SECTION ".section .rodata\n"
#endif

__asm__(
    YGO_EDO_SECTION
    ".balign 16\n"
    ".global gYgoEdoPak\n"
    "gYgoEdoPak:\n"
    ".incbin \"graphics/ygo/edo.pak\"\n"
    ".global gYgoEdoPakEnd\n"
    "gYgoEdoPakEnd:\n"
    ".byte 0\n"
    ".text\n");
