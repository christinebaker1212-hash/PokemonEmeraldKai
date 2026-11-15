// Moemon Kai: the BATTLE SUBWAY's map (Black's PROC_CallModeScreen 2, ov224 SUBWAY_MAP_GRAPHIC: the user's approved
// mock-up, 2026-11-15). Black's own 256 x 192 picture (a/2/0/6, the land with the lines over it, built into the scripts
// pack: tools/unova/build_scripts.py subway_map) shown pixel for pixel in the 240 x 160 screen, never stretched: it
// opens centred, the D-pad scrolls to its edges, B (or A) closes it and Black's script goes on.
#include <SDL2/SDL.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "gba/io_reg.h"
#include "mp.h"
#include "u3d.h"
#include "u3d_screen.h"
#include "../tcg/tcg_gfx.h"

#define SW 240
#define SH 160

const u16 *U3D_Vm_SubwayMap(u16 *w, u16 *h);   // (u3d_vm.c)

static struct
{
    SDL_Texture *tex;
    SDL_Renderer *owner;
    u16 w, h;
    s16 x, y;   // (the window's top left on the picture)
} M;

static void MapOpen(void)
{
    M.x = (M.w - SW) / 2;
    M.y = (M.h - SH) / 2;
}

static void MapDraw(void)
{
    SDL_Renderer *r = G_R();
    SDL_Rect src, dst = {0, 0, SW, SH};
    u16 w, h;
    const u16 *px;
    if (M.tex && M.owner != r)
    {
        SDL_DestroyTexture(M.tex);
        M.tex = NULL;
    }
    if (!M.tex && (px = U3D_Vm_SubwayMap(&w, &h)) != NULL)
    {
        static uint32_t argb[256 * 192];
        u32 i, n = (u32)w * h;
        if (n > ARRAY_COUNT(argb))
            return;
        for (i = 0; i < n; i++)
        {
            u16 c = px[i];
            u32 rr = (c & 31) * 255 / 31, gg = ((c >> 5) & 31) * 255 / 31, bb = ((c >> 10) & 31) * 255 / 31;
            argb[i] = 0xFF000000u | rr << 16 | gg << 8 | bb;
        }
        M.tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
        if (M.tex)
            SDL_UpdateTexture(M.tex, NULL, argb, w * 4);
        M.owner = r;
    }
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderFillRect(r, &dst);
    if (!M.tex)
        return;
    src.x = M.x, src.y = M.y, src.w = SW, src.h = SH;
    SDL_RenderCopy(r, M.tex, &src, &dst);
}

static void MapStep(u16 pressed, u16 keys)
{
    if (pressed & (B_BUTTON | A_BUTTON))
    {
        PlaySE(SE_SELECT);
        Unova_Screen_Close(NULL);
        return;
    }
    if ((keys & DPAD_LEFT) && M.x > 0)
        M.x--;
    if ((keys & DPAD_RIGHT) && M.x < M.w - SW)
        M.x++;
    if ((keys & DPAD_UP) && M.y > 0)
        M.y--;
    if ((keys & DPAD_DOWN) && M.y < M.h - SH)
        M.y++;
}

static const struct UnovaScreen sMap = {"SUBWAY MAP", MapOpen, MapDraw, MapStep};

// u3d_vm.c PROC_CallModeScreen 2: FALSE without the picture (an older pack), and the script goes on at once
bool8 Unova_SubwayMap_Open(void)
{
    if (U3D_Vm_SubwayMap(&M.w, &M.h) == NULL || M.w < SW || M.h < SH)
        return FALSE;
    PlaySE(SE_SELECT);
    Unova_Screen_Open(&sMap);
    return TRUE;
}

// Tests: `mp u3dsubwaymap`: the map opened over the field
void Unova_SubwayMap_Debug(void)
{
    if (!Unova_SubwayMap_Open())
        MPNet_Log("unova: no subway map in the pack");
}
