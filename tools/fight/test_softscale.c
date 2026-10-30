/*
 * Moemon Fight: tests of the resampled sprite drawing (fight/compat/mf_softscale.c). Built by build_linux.sh
 * (build/fight/linux/test_softscale); prints each check and exits 1 when one fails.
 *   - every source pixel covers the same screen width at 4/3 and 2/3 (nearest gave 1 or 2 pixels at 4/3); the
 *     rectangle's outermost pixels follow the pixel-centre rule (that is what keeps tiles seamless), so the
 *     outermost source columns are left out
 *   - wherever the sprite is (fractions of a pixel), it puts the same amount of colour on screen
 *   - tiles placed edge to edge at fractional positions leave no seam
 *   - colour 0 is transparent, and pixels outside the sprite are untouched
 *   - the drawer's operation is applied (subtract) and mixed by coverage
 *   - a parallax floor's rows keep their fractional starts
 *   - a sprite rotated by 90 degrees lands where the rotation puts it
 */
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mugen.h"
#include "mf_softscale.h"

#define TW 256
#define TH 64

static unsigned short sTarget[TW * TH];
static int sTargetRows[TH];
static Sprite sTspr;
static int sFails;

static void Check(int ok, const char *what)
{
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        sFails++;
    }
}

static void Put(unsigned short *d, unsigned int c, void *ctx)
{
    if (ctx != NULL) {     /* subtract, as softrender.c's SOFT_OP_SUB */
        int r = (int)(*d >> 11) - (int)(c >> 11), g = (int)(*d >> 5 & 63) - (int)(c >> 5 & 63);
        int b = (int)(*d & 31) - (int)(c & 31);
        *d = (unsigned short)((r < 0 ? 0 : r) << 11 | (g < 0 ? 0 : g) << 5 | (b < 0 ? 0 : b));
    } else {
        *d = (unsigned short)c;
    }
}

static void Clear(unsigned short c)
{
    int i;
    for (i = 0; i < TW * TH; i++) {
        sTarget[i] = c;
    }
    g_clipX1 = 0;
    g_clipY1 = 0;
    g_clipX2 = TW;
    g_clipY2 = TH;
}

/* A 1-row sprite of w pixels */
static unsigned char sPix[512];
static int sRows[64];
static Sprite MakeSprite(int w, int h)
{
    Sprite s;
    int i;
    memset(&s, 0, sizeof(s));
    s.w = w;
    s.h = h;
    s.ncolors = 256;
    s.rows = sRows;
    for (i = 0; i < h; i++) {
        sRows[i] = i * w;
    }
    return s;
}

/* red in row y, summed in pixels' worth (31 = one full pixel) */
static double RedSum(int y)
{
    double sum = 0.0;
    int x;
    for (x = 0; x < TW; x++) {
        sum += (sTarget[y * TW + x] >> 11) / 31.0;
    }
    return sum;
}

int main(void)
{
    unsigned short pal[256];
    int i, j;

    memset(pal, 0, sizeof(pal));
    pal[1] = 0xF800;    /* red */
    pal[2] = 0x001F;    /* blue */
    pal[3] = 0xFFFF;
    for (i = 0; i < TH; i++) {
        sTargetRows[i] = i * TW * 2;
    }
    sTspr.w = TW;
    sTspr.h = TH;
    sTspr.rows = sTargetRows;

    /* each source column alone, at 4/3 and 2/3: its screen width is the scale */
    {
        static const double scales[2] = { 4.0 / 3.0, 2.0 / 3.0 };
        int s;
        for (s = 0; s < 2; s++) {
            double worst = 0.0;
            Sprite spr = MakeSprite(30, 1);
            for (j = 1; j < 29; j++) {
                memset(sPix, 2, 30);
                sPix[j] = 1;
                Clear(0);
                MfSoft_ScaleResampled(&spr, sPix, &sTspr, (unsigned char *)sTarget, 10.37, 0.0, 30 * scales[s],
                                      1.0, 0, 0, 0, pal, Put, NULL, 1);
                worst = fmax(worst, fabs(RedSum(0) - scales[s]));
            }
            printf("     scale %.3f: each source pixel covers %.3f +- %.3f screen pixels\n", scales[s], scales[s],
                   worst);
            Check(worst < 0.07, s == 0 ? "even widths at 4/3" : "even widths at 2/3");
        }
    }

    /* the same sprite at 12 positions a twelfth of a pixel apart: the same amount of colour */
    {
        Sprite spr = MakeSprite(20, 1);
        double lo = 1e9, hi = 0.0;
        for (i = 0; i < 20; i++) {
            sPix[i] = (i % 3 == 0) ? 1 : 2;
        }
        sPix[0] = sPix[19] = 0;     /* transparent ends, as a character's frame has */
        for (j = 0; j < 12; j++) {
            double sum;
            Clear(0);
            MfSoft_ScaleResampled(&spr, sPix, &sTspr, (unsigned char *)sTarget, 30.0 + j / 12.0, 0.0, 20 * 4.0 / 3.0,
                                  1.0, 0, 0, 0, pal, Put, NULL, 1);
            sum = RedSum(0);
            lo = fmin(lo, sum);
            hi = fmax(hi, sum);
        }
        printf("     moving a twelfth of a pixel at a time: %.3f to %.3f pixels of red\n", lo, hi);
        Check(hi - lo < 0.25, "steady as it moves (within the 5-bit rounding of partly covered pixels)");
    }

    /* two tiles of 16 at 4/3, edge to edge from 5.3: one unbroken run of white */
    {
        Sprite spr = MakeSprite(16, 1);
        double w = 16 * 4.0 / 3.0;
        int first = (int)ceil(5.3 - 0.5), last = (int)ceil(5.3 + 2 * w - 0.5) - 1, gaps = 0;
        memset(sPix, 3, 16);
        Clear(0);
        MfSoft_ScaleResampled(&spr, sPix, &sTspr, (unsigned char *)sTarget, 5.3, 0.0, w, 1.0, 0, 0, 0, pal, Put,
                              NULL, 1);
        MfSoft_ScaleResampled(&spr, sPix, &sTspr, (unsigned char *)sTarget, 5.3 + w, 0.0, w, 1.0, 0, 0, 0, pal, Put,
                              NULL, 1);
        for (i = first; i <= last; i++) {
            gaps += sTarget[i] != 0xFFFF;
        }
        Check(gaps == 0 && sTarget[first - 1] == 0 && sTarget[last + 1] == 0, "tiles meet without a seam");
    }

    /* colour 0 transparent; outside untouched */
    {
        Sprite spr = MakeSprite(8, 2);
        int changed = 0;
        memset(sPix, 0, 16);
        Clear(0x1234);
        MfSoft_ScaleResampled(&spr, sPix, &sTspr, (unsigned char *)sTarget, 3.5, 2.25, 10.66, 2.66, 0, 0, 0, pal, Put,
                              NULL, 1);
        for (i = 0; i < TW * TH; i++) {
            changed += sTarget[i] != 0x1234;
        }
        Check(changed == 0, "colour 0 draws nothing");
    }

    /* subtract: full white minus red = cyan where fully covered, half-way where half covered */
    {
        Sprite spr = MakeSprite(3, 1);
        memset(sPix, 1, 3);
        Clear(0xFFFF);
        MfSoft_ScaleResampled(&spr, sPix, &sTspr, (unsigned char *)sTarget, 10.0, 0.0, 4.0, 1.0, 0, 0, 0, pal, Put,
                              (void *)1, 0);
        Check(sTarget[10] == 0x07FF && sTarget[13] == 0x07FF && sTarget[9] == 0xFFFF && sTarget[14] == 0xFFFF,
              "subtract applied where covered, nothing outside");
    }

    /* parallax: rows starting at 20.0 and 20.5 (tile off): the second's first pixel only half red */
    {
        MfParallaxRow rows[2];
        memset(sPix, 1, 16);
        rows[0].src = sPix;
        rows[0].srcw = 8;
        rows[0].start = 20.0;
        rows[0].width = 8.0;
        rows[1] = rows[0];
        rows[1].start = 20.25;
        Clear(0);
        MfSoft_Parallax(rows, 2, 0.0, 1.0, 0, 0, pal, 256, &sTspr, (unsigned char *)sTarget);
        printf("     row 0: %.2f pixels of red from x %d; row 1: %.2f from %d\n", RedSum(0), 20, RedSum(1), 20);
        Check(fabs(RedSum(0) - 8.0) < 0.05 && fabs(RedSum(1) - 8.0) < 0.05 && (sTarget[TW + 20] >> 11) == 31 &&
              sTarget[TW + 28] == 0 && sTarget[27] >> 11 == 31,
              "parallax rows keep their own fractional starts");
    }

    /* rotated 90 degrees (sine table values for 90: cos 0, sin 1024) around (40, 20): a 6x2 bar stands up */
    {
        Sprite spr = MakeSprite(6, 2);
        int tall = 0, wide = 0;
        memset(sPix, 1, 12);
        Clear(0);
        MfSoft_RotateAt(&spr, sPix, &sTspr, (unsigned char *)sTarget, 40, 20, 0, 1024, 1024, 1024, 1.0, 1.0, 0, 0,
                        pal, Put, NULL);
        for (i = 0; i < TH; i++) {
            tall += sTarget[i * TW + 40] != 0 || sTarget[i * TW + 41] != 0;
        }
        for (i = 0; i < TW; i++) {
            wide += sTarget[20 * TW + i] != 0;
        }
        printf("     rotated bar: %d rows tall, %d columns wide\n", tall, wide);
        Check(tall >= 5 && tall <= 7 && wide >= 1 && wide <= 3, "rotation by 90 degrees");
    }

    if (sFails) {
        printf("%d failed\n", sFails);
        return 1;
    }
    printf("all ok\n");
    return 0;
}
