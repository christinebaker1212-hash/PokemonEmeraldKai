// tools/orre/test_gxtex.sh: decodes every texture of an orre3d.pak with src/orre/gx_tex.c and prints each one's
// FNV-1a hash of its RGBA, to compare with tools/orre/gxtex.py's
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gx_tex.h"

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb");
    long size;
    uint8_t *d;
    uint32_t nt, nm, i, base;
    if (!f) return 1;
    fseek(f, 0, SEEK_END); size = ftell(f); fseek(f, 0, SEEK_SET);
    d = malloc(size);
    if (fread(d, 1, size, f) != (size_t)size) return 1;
    memcpy(&nt, d + 8, 4); memcpy(&nm, d + 12, 4);
    base = 16 + nt * 16 + nm * 44;
    for (i = 0; i < nt; i++)
    {
        const uint8_t *t = d + 16 + i * 16;
        uint16_t w, h, n; uint32_t off, sz, k, hash = 2166136261u, *out;
        memcpy(&w, t + 2, 2); memcpy(&h, t + 4, 2); memcpy(&n, t + 6, 2); memcpy(&off, t + 8, 4); memcpy(&sz, t + 12, 4);
        out = malloc(4u * w * h);
        if (!GxTex_Decode(t[0], w, h, d + base + off, sz - n * 2, t[1], d + base + off + sz - n * 2, n, out))
            { printf("%u FAIL\n", i); continue; }
        for (k = 0; k < 4u * w * h; k++) { hash ^= ((uint8_t *)out)[k]; hash *= 16777619u; }
        printf("%u %08x\n", i, hash);
        free(out);
    }
    return 0;
}
