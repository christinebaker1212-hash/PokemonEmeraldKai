/* Moemon Fight: resampled drawing of scaled and rotated sprites (mf_softscale.c). */
#ifndef MF_SOFTSCALE_H
#define MF_SOFTSCALE_H

/* The drawer's pixel operation, given the colour (RGB565) a fully covered pixel gets */
typedef void (*MfSoftPut)(unsigned short *d, unsigned int c565, void *ctx);

/* Draws spr (8-bit pixels src, palette pal) over [x0, x0 + sw) x [y0, y0 + sh) of the target, flipped,
 * colour 0 transparent unless opaque, inside g_clip*; copy: the operation is a plain copy (faster) */
void MfSoft_ScaleResampled(const Sprite *spr, const unsigned char *src, const Sprite *tspr, unsigned char *dst,
                           double x0, double y0, double sw, double sh, int flip_h, int flip_v, int opaque,
                           const unsigned short *pal, MfSoftPut put, void *ctx, int copy);

/* The same for a rotated sprite: m maps a screen point (x, y) to the sprite's pixels
 * (u = m[0] x + m[1] y + m[2], v = m[3] x + m[4] y + m[5]); ix0..ix1 x iy0..iy1 bounds it */
void MfSoft_RotateResampled(const Sprite *spr, const unsigned char *src, const Sprite *tspr, unsigned char *dst,
                            int ix0, int iy0, int ix1, int iy1, const double m[6], int opaque,
                            const unsigned short *pal, MfSoftPut put, void *ctx);

/* A rotated drawer's sprite (softrender.c Soft_Rotate's mapping, with its integer pivot, sine table values and
 * 1024ths scales) */
void MfSoft_RotateAt(const Sprite *spr, const unsigned char *src, const Sprite *tspr, unsigned char *dst, int ix,
                     int iy, int cosv, int sinv, int sw, int sh, double scale2_x, double scale2_y, int mirror,
                     int opaque, const unsigned short *pal, MfSoftPut put, void *ctx);

/* One source row of a parallax floor: its pixels (srcw of them) span [start, start + width) on screen */
typedef struct MfParallaxRow {
    const unsigned char *src;
    int srcw;
    double start, width;
} MfParallaxRow;

/* A parallax floor of nrows_src rows from screen row top down, scale_y screen rows per source row, each row
 * repeated across when tile_x (colour 0 transparent unless opaque), inside g_clip* */
void MfSoft_Parallax(const MfParallaxRow *rows, int nrows_src, double top, double scale_y, int tile_x, int opaque,
                     const unsigned short *pal, int ncolors, const Sprite *tspr, unsigned char *dst);

/* Nearest drawing as in MUGEN (MOEMON_FIGHT_NEAREST=1, for comparing) */
extern int g_mfNearest;
int MfSoft_Nearest(void);

#endif
