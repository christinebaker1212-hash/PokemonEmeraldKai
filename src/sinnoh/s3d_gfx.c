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

struct SinnohSprite { u8 kind; const u16 *pal; const u8 *frames; };
struct SinnohPic { const u16 *pal; struct MpBigSprite big; };

static struct
{
    bool8 tried, ok;
    u8 *data;
    struct SinnohSprite sprites[SINNOH_SPRITE_COUNT];
    struct SinnohPic pics[SINNOH_PIC_COUNT];
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
     || memcmp(sGfx.data, "S3DG", 4) != 0 || sGfx.data[4] != 1)
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
        if (p + 26 + 32 > end)
            goto bad;
        kind = p[24];
        sGfx.sprites[i].kind = kind;
        sGfx.sprites[i].pal = (const u16 *)(p + 26);
        sGfx.sprites[i].frames = p + 26 + 32;
        p += 26 + 32 + (kind == 0 ? 9 * 512 : kind == 3 ? 10 * 512 : kind == 1 ? 512 : 128);
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
    sGfx.ok = TRUE;
    return;
bad:
    printf("[s3d] %s: another graphics pack (rebuild it: tools/sinnoh/make_pack.sh)\n", path);
}

bool8 S3D_Gfx_Sprite(u16 index, u8 *kind, const u16 **pal, const u8 **frames)
{
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || index >= SINNOH_SPRITE_COUNT)
        return FALSE;
    *kind = sGfx.sprites[index].kind;
    *pal = sGfx.sprites[index].pal;
    *frames = sGfx.sprites[index].frames;
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
