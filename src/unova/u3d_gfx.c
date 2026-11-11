// Moemon Kai: UNOVA's people as Black draws them (docs/UNOVA_HANDOFF.md): unova/black_gfx.pak beside the game
// (tools/unova/build_gfx.py, from the user's Black ROM when the release is built; nothing of it is in this repository),
// a sprite a Black object code: the game's nine walking frames or one still frame, Black's palette, and MIXED's smaller
// copy. event_object_movement.c builds the graphics info (UnovaGraphicsInfo); without the pack, stand-ins.
// Its own code, never SINNOH's (s3d_gfx.c) or ORRE's.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "u3d.h"

const char *Updater_ExeFile(const char *name);

static struct
{
    bool8 tried, ok;
    u8 *data;
    u32 size;
    u16 count;
    u32 *at;   // each code's record (0: none)
} sGfx;

static u32 FrameBytes(u8 kind) { return kind == 4 ? 2048 : 512; }
static u32 FrameCount(u8 kind) { return kind == 0 ? 9 : 1; }

static void Load(void)
{
    const char *path = getenv("MOEMON_U3D_GFX");
    FILE *f;
    long size;
    u32 i, o;
    sGfx.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("unova/black_gfx.pak");
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    sGfx.data = malloc(size > 0 ? size : 1);
    if (sGfx.data == NULL || fread(sGfx.data, 1, size, f) != (size_t)size || size < 10 || memcmp(sGfx.data, "U3DG", 4) != 0
     || (sGfx.data[4] | sGfx.data[5] << 8) != 1)
    {
        fclose(f);
        free(sGfx.data);
        sGfx.data = NULL;
        printf("[u3d] %s isn't UNOVA's people\n", path);
        return;
    }
    fclose(f);
    sGfx.size = size;
    sGfx.count = sGfx.data[8] | sGfx.data[9] << 8;
    sGfx.at = calloc(sGfx.count ? sGfx.count : 1, sizeof(u32));
    for (i = 0, o = 10; i < sGfx.count && o + 2 <= sGfx.size; i++)
    {
        u8 kind = sGfx.data[o];
        if (kind == 0xFF)
        {
            o += 2;
            continue;
        }
        sGfx.at[i] = o;
        o += 2 + 32 + FrameCount(kind) * FrameBytes(kind);   // (Black's size)
        o += 32 + FrameCount(kind) * FrameBytes(kind);       // (MIXED's)
    }
    sGfx.ok = o <= sGfx.size;
    printf("[u3d] UNOVA's people: %u object codes\n", sGfx.count);
}

// Black's object code CODE's sprite (SMALL: MIXED's smaller copy); FALSE without the pack or for codes it doesn't draw
bool8 U3D_Gfx_Sprite(u16 code, bool8 small, u8 *kind, const u16 **pal, const u8 **frames)
{
    u32 o;
    u8 k;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || code >= sGfx.count || (o = sGfx.at[code]) == 0)
        return FALSE;
    k = sGfx.data[o];
    o += 2;
    if (small)
        o += 32 + FrameCount(k) * FrameBytes(k);
    *kind = k;
    *pal = (const u16 *)(sGfx.data + o);
    *frames = sGfx.data + o + 32;
    return TRUE;
}
