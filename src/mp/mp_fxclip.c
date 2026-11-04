// Moemon Co-op: the ball effects recorded for the phone app's POKé BALL page (MOEMON_EXPORT_FXCLIPS=DIR, run by
// android/build_assets.py after MOEMON_EXPORT_APP). The game's own ball effect editor (mp_ballfx_editor.c) plays its
// preview loop once per piece: every SEND-OUT with the rest on DEFAULT, then every PARTICLES, then every RETURN, a
// Moemon seen from the front (as others see it). Each frame's battle field (the editor's left 112 x 112) goes to
// DIR/<kind>_<index>.clip: "MFXC", u16 width, height, then per frame u32 bytes and the frame XORed with the one
// before as runs (u16 unchanged pixels, u16 changed, the changed pixels' XOR); the pixels are the frame's
// (ABGR1555). build_assets.py packs them into the app's fxclips.bin. When every piece is done, the game quits.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "main.h"
#include "pokemon.h"
#include "constants/species.h"

void MP_BallFxEditor_Open(struct BoxPokemon *target, MainCallback exitCb);
void MPNet_Log(const char *fmt, ...);

#define CLIP_W 112
#define CLIP_H 112

enum { KIND_FORM, KIND_PART, KIND_RECALL, KIND_COUNT };

static struct
{
    bool8 on, recording, warm;
    u8 kind;
    s16 index;
    u16 frames;
    FILE *f;
    char dir[512];
    u16 prev[CLIP_W * CLIP_H];
} *sClip;

static struct BoxPokemon sMon;

bool8 MP_FxClip_Active(void)
{
    return sClip != NULL && sClip->on;
}

static void CloseClip(void)
{
    if (sClip->f != NULL)
    {
        fclose(sClip->f);
        sClip->f = NULL;
        MPNet_Log("fxclip: %u %d: %u frames", sClip->kind, sClip->index, sClip->frames);
    }
    sClip->recording = FALSE;
}

static u8 KindCount(u8 kind)
{
    return kind == KIND_FORM ? MPFX_FORM_COUNT : kind == KIND_PART ? MPFX_PART_COUNT : MPFX_RECALL_COUNT;
}

// The editor's preview starts again (mp_ballfx_editor.c StartPreview): the last clip is done, the next piece is set
// up in `fx` (everything else DEFAULT); FALSE when there are no more (the game quits)
bool8 MP_FxClip_Next(struct MpBallFx *fx)
{
    char path[600];
    static const char sMagic[4] = {'M', 'F', 'X', 'C'};
    u16 size[2] = {CLIP_W, CLIP_H};
    if (!MP_FxClip_Active())
        return FALSE;
    CloseClip();
    if (!sClip->warm)   // (the first loop plays out the editor's fade-in, unrecorded)
    {
        sClip->warm = TRUE;
        memset(fx, 0, sizeof(*fx));
        return TRUE;
    }
    if (++sClip->index >= KindCount(sClip->kind))
    {
        sClip->index = 0;
        if (++sClip->kind >= KIND_COUNT)
        {
            MPNet_Log("fxclip: every piece recorded");
            printf("[export] the ball effects are in %s\n", sClip->dir);
            fflush(stdout);
            exit(0);
        }
    }
    memset(fx, 0, sizeof(*fx));
    fx->color = MP_BALL_RED;
    if (sClip->kind == KIND_FORM)
        fx->form = sClip->index;
    else if (sClip->kind == KIND_PART)
        fx->part = sClip->index;
    else
        fx->recall = sClip->index;
    snprintf(path, sizeof(path), "%s/%u_%d.clip", sClip->dir, sClip->kind, sClip->index);
    sClip->f = fopen(path, "wb");
    if (sClip->f == NULL)
        return FALSE;
    fwrite(sMagic, 1, 4, sClip->f);
    fwrite(size, 2, 2, sClip->f);
    memset(sClip->prev, 0, sizeof(sClip->prev));
    sClip->frames = 0;
    sClip->recording = TRUE;
    return TRUE;
}

// sdl2.c, every finished frame (effects composited): the field, XORed with the frame before, as runs
void MP_FxClip_Frame(const u16 *frame)
{
    static u16 sOut[CLIP_W * CLIP_H * 2 + 8];
    u32 n = 0, i = 0, total = CLIP_W * CLIP_H, bytes;
    if (!MP_FxClip_Active() || !sClip->recording || sClip->f == NULL)
        return;
    while (i < total)
    {
        u32 same = 0, start, diff = 0;
        while (i + same < total && same < 0xFFFF
               && frame[((i + same) / CLIP_W) * DISPLAY_WIDTH + (i + same) % CLIP_W] == sClip->prev[i + same])
            same++;
        i += same;
        start = i;
        while (i + diff < total && diff < 0xFFFF
               && frame[((i + diff) / CLIP_W) * DISPLAY_WIDTH + (i + diff) % CLIP_W] != sClip->prev[i + diff])
            diff++;
        sOut[n++] = same;
        sOut[n++] = diff;
        for (i = start; i < start + diff; i++)
        {
            u16 px = frame[(i / CLIP_W) * DISPLAY_WIDTH + i % CLIP_W];
            sOut[n++] = px ^ sClip->prev[i];
            sClip->prev[i] = px;
        }
    }
    bytes = n * 2;
    fwrite(&bytes, 4, 1, sClip->f);
    fwrite(sOut, 2, n, sClip->f);
    sClip->frames++;
}

static void CB2_Quit(void)
{
    exit(0);
}

// mp_main.c MP_FrameUpdate, at the first frame when MOEMON_EXPORT_FXCLIPS is set: a Moemon, and the editor
void MP_FxClip_IfAsked(void)
{
    static bool8 sDone;
    const char *dir = getenv("MOEMON_EXPORT_FXCLIPS");
    char mk[600];
    if (sDone || dir == NULL || dir[0] == '\0')
        return;
    sDone = TRUE;
    sClip = calloc(1, sizeof(*sClip));
    if (sClip == NULL)
        return;
    snprintf(sClip->dir, sizeof(sClip->dir), "%s", dir);
#ifdef _WIN32
    snprintf(mk, sizeof(mk), "mkdir \"%s\" 2>nul", dir);
#else
    snprintf(mk, sizeof(mk), "mkdir -p '%s'", dir);
#endif
    if (system(mk) != 0)
        (void)0;
    sClip->on = TRUE;
    sClip->kind = KIND_FORM;
    sClip->index = -1;
    // (at the first frame there's no save yet: an empty one, on the first map, kept until the game quits)
    if (gSaveBlock1Ptr == NULL)
    {
        static struct SaveBlock1 sNoSave;
        gSaveBlock1Ptr = &sNoSave;
    }
    // PIKACHU, the same one every time (a fixed personality, not shiny)
    CreateBoxMon(&sMon, SPECIES_PIKACHU, 50, 31, TRUE, 0x00000088, OT_ID_PRESET, 0x0000A55A);
    MP_BallFxEditor_Open(&sMon, CB2_Quit);
}
