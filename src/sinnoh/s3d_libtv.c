// Moemon Kai: CANALAVE LIBRARY's TV news of LAKE VALOR, as Platinum shows it (src/library_tv/library_tv.c): the TV's
// frame over the news picture, its scan lines blended 4/16 over the picture and rolling up a line every 4 frames,
// faded in from black over 6 frames, held LIBRARY_TV_DURATION (240), faded out over 6 (the script holds the field for
// those 252: tools/sinnoh/gen_scripts.py StartLibraryTV). Its layers come from the user's Platinum ROM, which the decomp
// doesn't hold: sinnoh/platinum_libtv.bin (tools/sinnoh/build_library_tv.py; never committed). Without it the screen
// holds dark as before. The DS's 256 x 192 picture is cropped to the screen's middle, never scaled. Drawn over the
// finished frame (sdl2.c S3D_LibraryTvComposite).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"

const char *Updater_ExeFile(const char *name);

#define LTV_W 256
#define LTV_FADE 6
#define LTV_HOLD 240
#define CROP_X 8
#define CROP_Y 16

static struct
{
    bool8 tried, ok, on;
    u16 t;
    u16 *layer[3];   // the frame, the scan lines, the picture: 256 x 256, BGR555 with bit 15 drawn
    u16 *data;
} sTv;

static void Load(void)
{
    const char *path = Updater_ExeFile("sinnoh/platinum_libtv.bin");
    FILE *f;
    long size;
    sTv.tried = TRUE;
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size != 8 + 3 * LTV_W * LTV_W * 2 || (sTv.data = malloc(size)) == NULL || fread(sTv.data, 1, size, f) != (size_t)size
     || memcmp(sTv.data, "SLTV", 4) != 0)
    {
        fclose(f);
        free(sTv.data);
        sTv.data = NULL;
        MPNet_Log("sinnoh: %s isn't the library TV", path);
        return;
    }
    fclose(f);
    sTv.layer[0] = sTv.data + 4;
    sTv.layer[1] = sTv.layer[0] + LTV_W * LTV_W;
    sTv.layer[2] = sTv.layer[1] + LTV_W * LTV_W;
    sTv.ok = TRUE;
}

// SINNOH_MISC 73 (StartLibraryTV): the broadcast begins
void Sinnoh_LibraryTvStart(void)
{
    if (!sTv.tried)
        Load();
    sTv.on = sTv.ok, sTv.t = 0;
    MPNet_Log("sinnoh: the library TV%s", sTv.ok ? "" : " (no picture: sinnoh/platinum_libtv.bin isn't there)");
}

bool8 Sinnoh_LibraryTvOn(void)
{
    return sTv.on;
}

static u16 Mix(u16 a, u16 b, u32 wa, u32 wb)
{
    u32 ch, out = 0;
    for (ch = 0; ch < 15; ch += 5)
        out |= min(31u, (((a >> ch) & 31) * wa + ((b >> ch) & 31) * wb) / 16) << ch;
    return out;
}

void S3D_LibraryTvComposite(u16 *frame)
{
    s32 x, y, scroll;
    u32 level;
    if (!sTv.on)
        return;
    sTv.t++;
    if (sTv.t >= LTV_FADE + LTV_HOLD + LTV_FADE)
    {
        sTv.on = FALSE;
        return;
    }
    // (brightness: up from black, held, back down)
    level = sTv.t < LTV_FADE ? 16 * sTv.t / LTV_FADE
          : sTv.t >= LTV_FADE + LTV_HOLD ? 16 * (LTV_FADE + LTV_HOLD + LTV_FADE - sTv.t) / LTV_FADE : 16;
    scroll = (sTv.t * 4) >> 4;   // (LibraryTV_UpdateScanLines: 4 a frame, the offset its sixteenths)
    for (y = 0; y < DISPLAY_HEIGHT; y++)
    {
        s32 sy = y + CROP_Y;
        for (x = 0; x < DISPLAY_WIDTH; x++)
        {
            s32 sx = x + CROP_X;
            u16 c = 0, p = sTv.layer[2][sy * LTV_W + sx], s = sTv.layer[1][((sy + scroll) & 255) * LTV_W + sx],
                f = sTv.layer[0][sy * LTV_W + sx];
            if (p & 0x8000)
                c = p & 0x7FFF;
            if (s & 0x8000)   // (G2_SetBlendAlpha BG1 -> BG2 | BG3, 4, 12)
                c = Mix(s, c, 4, 12);
            if (f & 0x8000)
                c = f & 0x7FFF;
            frame[y * DISPLAY_WIDTH + x] = 0x8000 | Mix(c, 0, level, 0);
        }
    }
}
