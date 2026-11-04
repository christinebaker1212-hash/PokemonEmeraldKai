// Moemon Kai: card words in the letters the game's font has (tcg_gfx.c GameChar: ASCII and é): Forge's "\n" as a
// line break, mana symbols {R} {2} {T} as (R) (2) (T), dashes and quotes plain, Æ as AE, accents dropped.
#include <string.h>
#include "mtg/mtg.h"

void Mtg_PlainText(char *out, unsigned cap, const char *in)
{
    unsigned n = 0;
    const unsigned char *p = (const unsigned char *)in;
    if (!cap)
        return;
#define PUT(c) ((void)(n + 1 < cap ? (out[n++] = (char)(c)) : 0))
    while (*p) {
        unsigned char c = *p;
        if (c == '\\' && p[1] == 'n') {
            PUT('\n');
            p += 2;
        } else if (c == '{') {
            const unsigned char *e = (const unsigned char *)strchr((const char *)p, '}');
            if (!e) {
                p++;
                continue;
            }
            PUT('(');
            for (p++; p < e; p++)
                if (*p != '/')
                    PUT(*p);
            PUT(')');
            p = e + 1;
        } else if (c < 0x80) {
            PUT(c);
            p++;
        } else if (c == 0xC3 && p[1]) {
            unsigned char d = p[1];
            static const char sUpper[] = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYTs";   // 0xC3 0x80-0x9F
            static const char sLower[] = "aaaaaaaceeeeiiiidnooooo/ouuuuyty";   // 0xC3 0xA0-0xBF
            if (d == 0xA9)
                PUT(0xC3), PUT(0xA9);   // é: the font has it
            else if (d == 0x86)
                PUT('A'), PUT('E');
            else if (d == 0xA6)
                PUT('a'), PUT('e');
            else if (d >= 0x80 && d <= 0x9F)
                PUT(sUpper[d - 0x80]);
            else if (d >= 0xA0 && d <= 0xBF)
                PUT(sLower[d - 0xA0]);
            p += 2;
        } else if (c == 0xE2 && p[1] == 0x80 && p[2]) {   // general punctuation
            unsigned char e = p[2];
            if (e == 0x98 || e == 0x99)
                PUT('\'');
            else if (e == 0x9C || e == 0x9D)
                PUT('"');
            else if (e == 0xA2)
                PUT('-');          // •
            else if (e == 0xA6)
                PUT('.'), PUT('.'), PUT('.');
            else
                PUT('-');          // – — and the rest
            p += 3;
        } else if (c == 0xE2 && p[1] == 0x88 && p[2] == 0x92) {   // − (minus)
            PUT('-');
            p += 3;
        } else {
            // any other letter: skipped whole (its continuation bytes too)
            p++;
            while ((*p & 0xC0) == 0x80)
                p++;
        }
    }
#undef PUT
    out[n] = 0;
}
