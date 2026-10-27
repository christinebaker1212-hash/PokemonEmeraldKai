/*
 * Bob Jenkins' lookup3.c (May 2006, public domain): hashlittle, linked into
 * mugen.exe between the sprite codecs and gldetect.c. LTCG specialised it
 * for its only caller (CountedStr_Hash): the initial value 0x1337B47E is
 * folded in, which is what this file keeps.
 *
 * As in the original (built without VALGRIND), keys aligned on 4 bytes are
 * read a whole word at a time, up to 3 bytes past their end.
 */
#include "mugen.h"

#define LOOKUP3_INITVAL 0x1337B47Eu

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define mix(a, b, c)                    \
    {                                   \
        a -= c; a ^= rot(c, 4);  c += b; \
        b -= a; b ^= rot(a, 6);  a += c; \
        c -= b; c ^= rot(b, 8);  b += a; \
        a -= c; a ^= rot(c, 16); c += b; \
        b -= a; b ^= rot(a, 19); a += c; \
        c -= b; c ^= rot(b, 4);  b += a; \
    }

#define final(a, b, c)                  \
    {                                   \
        c ^= b; c -= rot(b, 14);        \
        a ^= c; a -= rot(c, 11);        \
        b ^= a; b -= rot(a, 25);        \
        c ^= b; c -= rot(b, 16);        \
        a ^= c; a -= rot(c, 4);         \
        b ^= a; b -= rot(a, 14);        \
        c ^= b; c -= rot(b, 24);        \
    }

/* 0x0048D9B0: hashlittle(key, length, 0x1337B47E). */
unsigned int Hash_Lookup3(const void *key, size_t length)
{
    uint32_t a, b, c;

    a = b = c = 0xDEADBEEFu + (uint32_t)length + LOOKUP3_INITVAL;
    if (((uintptr_t)key & 3) == 0) {
        const uint32_t *k = key;

        while (length > 12) {
            a += k[0];
            b += k[1];
            c += k[2];
            mix(a, b, c);
            length -= 12;
            k += 3;
        }
        switch (length) {
        case 12: c += k[2]; b += k[1]; a += k[0]; break;
        case 11: c += k[2] & 0xFFFFFF; b += k[1]; a += k[0]; break;
        case 10: c += k[2] & 0xFFFF; b += k[1]; a += k[0]; break;
        case 9: c += k[2] & 0xFF; b += k[1]; a += k[0]; break;
        case 8: b += k[1]; a += k[0]; break;
        case 7: b += k[1] & 0xFFFFFF; a += k[0]; break;
        case 6: b += k[1] & 0xFFFF; a += k[0]; break;
        case 5: b += k[1] & 0xFF; a += k[0]; break;
        case 4: a += k[0]; break;
        case 3: a += k[0] & 0xFFFFFF; break;
        case 2: a += k[0] & 0xFFFF; break;
        case 1: a += k[0] & 0xFF; break;
        case 0: return c;
        }
    } else if (((uintptr_t)key & 1) == 0) {
        const uint16_t *k = key;
        const uint8_t *k8;

        while (length > 12) {
            a += k[0] + ((uint32_t)k[1] << 16);
            b += k[2] + ((uint32_t)k[3] << 16);
            c += k[4] + ((uint32_t)k[5] << 16);
            mix(a, b, c);
            length -= 12;
            k += 6;
        }
        k8 = (const uint8_t *)k;
        switch (length) {
        case 12:
            c += k[4] + ((uint32_t)k[5] << 16);
            b += k[2] + ((uint32_t)k[3] << 16);
            a += k[0] + ((uint32_t)k[1] << 16);
            break;
        case 11:
            c += (uint32_t)k8[10] << 16;
            /* fall through */
        case 10:
            c += k[4];
            b += k[2] + ((uint32_t)k[3] << 16);
            a += k[0] + ((uint32_t)k[1] << 16);
            break;
        case 9:
            c += k8[8];
            /* fall through */
        case 8:
            b += k[2] + ((uint32_t)k[3] << 16);
            a += k[0] + ((uint32_t)k[1] << 16);
            break;
        case 7:
            b += (uint32_t)k8[6] << 16;
            /* fall through */
        case 6:
            b += k[2];
            a += k[0] + ((uint32_t)k[1] << 16);
            break;
        case 5:
            b += k8[4];
            /* fall through */
        case 4:
            a += k[0] + ((uint32_t)k[1] << 16);
            break;
        case 3:
            a += (uint32_t)k8[2] << 16;
            /* fall through */
        case 2:
            a += k[0];
            break;
        case 1:
            a += k8[0];
            break;
        case 0:
            return c;
        }
    } else {
        const uint8_t *k = key;

        while (length > 12) {
            a += k[0] + ((uint32_t)k[1] << 8) + ((uint32_t)k[2] << 16) + ((uint32_t)k[3] << 24);
            b += k[4] + ((uint32_t)k[5] << 8) + ((uint32_t)k[6] << 16) + ((uint32_t)k[7] << 24);
            c += k[8] + ((uint32_t)k[9] << 8) + ((uint32_t)k[10] << 16) + ((uint32_t)k[11] << 24);
            mix(a, b, c);
            length -= 12;
            k += 12;
        }
        switch (length) {
        case 12: c += (uint32_t)k[11] << 24; /* fall through */
        case 11: c += (uint32_t)k[10] << 16; /* fall through */
        case 10: c += (uint32_t)k[9] << 8;   /* fall through */
        case 9: c += k[8];                   /* fall through */
        case 8: b += (uint32_t)k[7] << 24;   /* fall through */
        case 7: b += (uint32_t)k[6] << 16;   /* fall through */
        case 6: b += (uint32_t)k[5] << 8;    /* fall through */
        case 5: b += k[4];                   /* fall through */
        case 4: a += (uint32_t)k[3] << 24;   /* fall through */
        case 3: a += (uint32_t)k[2] << 16;   /* fall through */
        case 2: a += (uint32_t)k[1] << 8;    /* fall through */
        case 1: a += k[0]; break;
        case 0: return c;
        }
    }
    final(a, b, c);
    return c;
}
