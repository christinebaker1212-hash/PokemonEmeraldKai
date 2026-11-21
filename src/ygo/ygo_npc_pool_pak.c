// Moemon Kai: the generic duelists' Yu-Gi-Oh! decks from YGOPRODeck, rated (graphics/ygo/npc_pool.bin from
// tools/ygo/write_npc_pool.py; read by ygo_npc_pool.c), built in with the assembler's .incbin.
// json_data_rules.mk makes this file depend on it.
/* (no headers: only the data) */

#ifdef _WIN32
#define YGO_POOL_SECTION ".section .rdata,\"dr\"\n"   // (PE/COFF: read-only data)
#else
#define YGO_POOL_SECTION ".section .rodata\n"
#endif

__asm__(
    YGO_POOL_SECTION
    ".balign 16\n"
    ".global gYgoNpcPoolPak\n"
    "gYgoNpcPoolPak:\n"
    ".incbin \"graphics/ygo/npc_pool.bin\"\n"
    ".global gYgoNpcPoolPakEnd\n"
    "gYgoNpcPoolPakEnd:\n"
    ".byte 0\n"
    ".text\n");
