// Moemon Kai: SINNOH's people as Platinum draws them (sinnoh/platinum_gfx.pak, tools/sinnoh/build_gfx.py): overworld
// sprites (event_object_movement.c builds their graphics info) and trainers' battle pictures (drawn whole, 80x80, by
// the big-sprite renderer over the stand-in's place). Without the file everything keeps the game's own stand-ins.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "palette.h"
#include "sprite.h"
#include "s3d.h"
#include "mp_bigsprite.h"
#include "constants/sinnoh_gfx.h"
#include "constants/trainers.h"
#include "constants/battle.h"
#include "battle.h"
#include "data.h"
#include "mp.h"
#include "battle_anim.h"
#include "battle_setup.h"

const char *Updater_ExeFile(const char *name);

struct SinnohSprite { u8 kind; const u16 *pal[2]; const u8 *frames[2]; };   // ([1]: MIXED's smaller copy)
struct SinnohPic { const u16 *pal; struct MpBigSprite big; };

static struct
{
    bool8 tried, ok;
    u8 *data;
    struct SinnohSprite sprites[SINNOH_SPRITE_COUNT];
    struct SinnohPic pics[SINNOH_PIC_COUNT];
    const u16 *town[2];    // the TOWN MAP's pages (tools/sinnoh/gen_townmap.py picture), RGB555
    u16 townW, townH;
    const u8 *poketch;     // the POKéTCH's art (tools/sinnoh/build_poketch.py: "PKCH")
    const u8 *poketchEnd;
} sGfx;

static void Load(void)
{
    const char *path = getenv("MOEMON_S3D_GFX");
    FILE *f;
    long size;
    u8 *p, *end;
    u32 i, n;
    sGfx.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("sinnoh/platinum_gfx.pak");
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    sGfx.data = malloc(size);
    if (sGfx.data == NULL || fread(sGfx.data, 1, size, f) != (size_t)size || size < 10
     || memcmp(sGfx.data, "S3DG", 4) != 0 || sGfx.data[4] != 2)
    {
        fclose(f);
        free(sGfx.data);
        sGfx.data = NULL;
        printf("[s3d] %s isn't a SINNOH graphics pack\n", path);
        return;
    }
    fclose(f);
    p = sGfx.data + 8;
    end = sGfx.data + size;
    n = p[0] | p[1] << 8;
    p += 2;
    if (n != SINNOH_SPRITE_COUNT)
        goto bad;   // (a pack from another commit: names and numbers wouldn't match)
    for (i = 0; i < n; i++)
    {
        u8 kind;
        u32 frames;
        if (p + 26 + 32 > end)
            goto bad;
        kind = p[24];
        frames = kind == 0 ? 9 * 512 : kind == 3 ? 10 * 512 : kind == 1 ? 512 : kind == 4 ? 2048 : 128;
        if (p + 26 + 2 * (32 + frames) > end)
            goto bad;
        sGfx.sprites[i].kind = kind;
        sGfx.sprites[i].pal[0] = (const u16 *)(p + 26);
        sGfx.sprites[i].frames[0] = p + 26 + 32;
        sGfx.sprites[i].pal[1] = (const u16 *)(p + 26 + 32 + frames);
        sGfx.sprites[i].frames[1] = p + 26 + 32 + frames + 32;
        p += 26 + 2 * (32 + frames);
    }
    if (p + 2 > end)
        goto bad;
    n = p[0] | p[1] << 8;
    p += 2;
    if (n != SINNOH_PIC_COUNT)
        goto bad;
    for (i = 0; i < n; i++)
    {
        if (p + 24 + 32 + 8 + 80 * 40 > end)
            goto bad;
        sGfx.pics[i].pal = (const u16 *)(p + 24);
        sGfx.pics[i].big.frame = p + 24 + 32;
        sGfx.pics[i].big.bpp = 4;
        sGfx.pics[i].big.colorCount = 16;
        p += 24 + 32 + 8 + 80 * 40;
    }
    // then the TOWN MAP: "TOWN", width, height, pages (u16 each), the pages' RGB555 pixels (a pack without it: none)
    if (p + 10 <= end && memcmp(p, "TOWN", 4) == 0)
    {
        u32 w = p[4] | p[5] << 8, h = p[6] | p[7] << 8, pages = p[8] | p[9] << 8;
        if (pages >= 2 && p + 10 + pages * w * h * 2 <= end)
        {
            sGfx.townW = w;
            sGfx.townH = h;
            sGfx.town[0] = (const u16 *)(p + 10);
            sGfx.town[1] = (const u16 *)(p + 10 + w * h * 2);
            p += 10 + pages * w * h * 2;
        }
    }
    if (p + 4 <= end && memcmp(p, "PKCH", 4) == 0)
    {
        sGfx.poketch = p + 4;
        sGfx.poketchEnd = end;
    }
    sGfx.ok = TRUE;
    return;
bad:
    printf("[s3d] %s: another graphics pack (rebuild it: tools/sinnoh/make_pack.sh)\n", path);
}

// Platinum's sprite INDEX (SMALL: MIXED's smaller copy)
bool8 S3D_Gfx_Sprite(u16 index, bool8 small, u8 *kind, const u16 **pal, const u8 **frames)
{
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || index >= SINNOH_SPRITE_COUNT)
        return FALSE;
    *kind = sGfx.sprites[index].kind;
    *pal = sGfx.sprites[index].pal[small != 0];
    *frames = sGfx.sprites[index].frames[small != 0];
    return TRUE;
}

#include "../data/sinnoh_trainer_pics.h"

// A SINNOH trainer's own battle picture on SPRITE (made with the stand-in's 64x64 picture STANDIN and its palette, at
// the stand-in's height): Platinum's 80x80 picture drawn whole in its place, feet where the game's pictures have
// theirs, its colors in the sprite's palette slot. Nothing changes for any other trainer or without the pack.
void S3D_Gfx_TrainerPic(struct Sprite *sprite, u16 trainerId, u16 standIn)
{
    u8 pic;
    if (trainerId < FIRST_SINNOH_TRAINER || trainerId >= FIRST_SINNOH_TRAINER + SINNOH_TRAINERS_COUNT
     || gTrainers[trainerId].trainerPic != standIn)
        return;
    pic = sSinnohTrainerPics[trainerId - FIRST_SINNOH_TRAINER];
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || pic >= SINNOH_PIC_COUNT)
        return;
    LoadPalette(sGfx.pics[pic].pal, OBJ_PLTT_ID(sprite->oam.paletteNum), PLTT_SIZE_4BPP);
    sprite->y -= (8 - gTrainerFrontPicCoords[standIn].size) * 4;   // (the stand-in's own height undone)
    sprite->mpBig = &sGfx.pics[pic].big;
    printf("[test] sinnoh battle picture: trainer %d picture %d\n", trainerId, pic);
}

// A SINNOH trainer's Platinum picture for other screens (the card games' challenge screen): 80x80, rows of 40 bytes
// (4 bits a pixel, the low nibble first), and its 16 colors; FALSE: none (another trainer, or no pack)
bool8 S3D_Gfx_TrainerPicPixels(u16 trainerId, const u8 **pixels, const u16 **pal)
{
    u8 pic;
    if (trainerId < FIRST_SINNOH_TRAINER || trainerId >= FIRST_SINNOH_TRAINER + SINNOH_TRAINERS_COUNT)
        return FALSE;
    pic = sSinnohTrainerPics[trainerId - FIRST_SINNOH_TRAINER];
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || pic >= SINNOH_PIC_COUNT)
        return FALSE;
    *pixels = sGfx.pics[pic].big.frame + 8;
    *pal = sGfx.pics[pic].pal;
    return TRUE;
}

// The battle's opponent trainer picture for BATTLER (battle controllers): the trainer it belongs to
void S3D_Gfx_BattleTrainerPic(struct Sprite *sprite, u8 battler, u16 standIn)
{
    bool8 second = GetBattlerPosition(battler) == B_POSITION_OPPONENT_RIGHT
                && ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) || (gMpCoopBattle && MP_Coop_TwoTrainers()));
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER) || ((gBattleTypeFlags & BATTLE_TYPE_LINK) && !gMpCoopBattle))
        return;
    S3D_Gfx_TrainerPic(sprite, second ? gTrainerBattleOpponent_B : gTrainerBattleOpponent_A, standIn);
}

// The TOWN MAP's page PAGE (0 NORTH, 1 SOUTH): W x H RGB555 pixels; FALSE: none (no pack, or one without it)
bool8 S3D_Gfx_TownMap(u8 page, const u16 **pixels, u16 *w, u16 *h)
{
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || page > 1 || sGfx.town[page] == NULL)
        return FALSE;
    *pixels = sGfx.town[page];
    *w = sGfx.townW;
    *h = sGfx.townH;
    return TRUE;
}

// The POKéTCH's app screen NAME: 192x160 shade bytes (0 lightest .. 3 darkest); NULL: none
const u8 *S3D_Gfx_PoketchScreen(const char *name)
{
    const u8 *p;
    u32 i, n;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || (p = sGfx.poketch) == NULL)
        return NULL;
    n = p[0] | p[1] << 8;
    p += 2;
    for (i = 0; i < n && p + 16 + 192 * 160 <= sGfx.poketchEnd; i++, p += 16 + 192 * 160)
        if (strncmp((const char *)p, name, 16) == 0)
            return p + 16;
    return NULL;
}

// Cell CELL of the POKéTCH's sprite sheet SHEET: its top-left from the sprite's point, size and shade bytes (255 clear)
bool8 S3D_Gfx_PoketchCell(const char *sheet, u16 cell, s16 *x, s16 *y, u16 *w, u16 *h, const u8 **px)
{
    const u8 *p;
    u32 i, n, k, cells;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || (p = sGfx.poketch) == NULL)
        return FALSE;
    n = p[0] | p[1] << 8;
    p += 2 + n * (16 + 192 * 160);
    if (p + 2 > sGfx.poketchEnd)
        return FALSE;
    n = p[0] | p[1] << 8;
    p += 2;
    for (i = 0; i < n; i++)
    {
        bool8 want = strncmp((const char *)p, sheet, 16) == 0;
        cells = p[16] | p[17] << 8;
        p += 18;
        for (k = 0; k < cells; k++)
        {
            u16 cw = p[4] | p[5] << 8, ch = p[6] | p[7] << 8;
            if (p + 8 + cw * ch > sGfx.poketchEnd)
                return FALSE;
            if (want && k == cell)
            {
                *x = (s16)(p[0] | p[1] << 8);
                *y = (s16)(p[2] | p[3] << 8);
                *w = cw;
                *h = ch;
                *px = p + 8;
                return TRUE;
            }
            p += 8 + cw * ch;
        }
        if (want)
            return FALSE;
    }
    return FALSE;
}
