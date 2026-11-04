// Moemon Kai: GameCube GX textures (ORRE's maps keep their textures in the disc's own encoding: tools/orre/build_pack.py; src/orre/o3d.c)
#ifndef GUARD_GX_TEX_H
#define GUARD_GX_TEX_H

#include <stdint.h>

// The bytes a GX texture of FORMAT takes at w x h (0: a format this doesn't know)
uint32_t GxTex_Size(uint8_t format, uint16_t w, uint16_t h);
// Decodes it into OUT (w * h pixels, r | g << 8 | b << 16 | a << 24, as the 3D engine's textures); TLUT: the palette
// (TLUT_FORMAT 0 IA8, 1 RGB565, 2 RGB5A3; ENTRIES of them) for C4 / C8 / C14X2. FALSE: a format it doesn't know
int GxTex_Decode(uint8_t format, uint16_t w, uint16_t h, const uint8_t *data, uint32_t size, uint8_t tlutFormat,
                 const uint8_t *tlut, uint16_t entries, uint32_t *out);

#endif
