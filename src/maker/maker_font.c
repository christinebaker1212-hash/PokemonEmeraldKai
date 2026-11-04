// Moemon Maker: the editor's font, Liberation Sans (SIL Open Font License 1.1, maker/fonts/OFL.txt), built in with the
// assembler's .incbin (as mp_pmd_data.c does) so the editor needs no files beside the game.
#include "global.h"

#ifdef _WIN32
#define MAKER_FONT_SECTION ".section .rdata,\"dr\"\n"
#else
#define MAKER_FONT_SECTION ".section .rodata\n"
#endif

__asm__(
    MAKER_FONT_SECTION
    ".balign 16\n"
    ".global gMakerFontRegular\n"
    "gMakerFontRegular:\n"
    ".incbin \"maker/fonts/LiberationSans-Regular.ttf\"\n"
    ".global gMakerFontRegularEnd\n"
    "gMakerFontRegularEnd:\n"
    ".balign 16\n"
    ".global gMakerFontBold\n"
    "gMakerFontBold:\n"
    ".incbin \"maker/fonts/LiberationSans-Bold.ttf\"\n"
    ".global gMakerFontBoldEnd\n"
    "gMakerFontBoldEnd:\n"
    ".byte 0\n"
    ".text\n");
