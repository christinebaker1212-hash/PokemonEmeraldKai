// Moemon Kai: UNOVA's people as Black draws them (docs/UNOVA_HANDOFF.md): unova/black_gfx.pak beside the game
// (tools/unova/build_gfx.py, from the user's Black ROM when the release is built; nothing of it is in this repository),
// a sprite a Black object code: the game's nine walking frames or one still frame, Black's palette, and MIXED's smaller
// copy. event_object_movement.c builds the graphics info (UnovaGraphicsInfo); without the pack, stand-ins.
// Its own code, never SINNOH's (s3d_gfx.c) or ORRE's.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_setup.h"
#include "data.h"
#include "mp_bigsprite.h"
#include "palette.h"
#include "sprite.h"
#include "constants/battle.h"
#include "u3d.h"

const char *Updater_ExeFile(const char *name);

static struct
{
    bool8 tried, ok;
    u8 *data;
    u32 size;
    u16 count;
    u32 *at;   // each code's record (0: none)
    // Black's trainers' battle pictures: which a class shows, and each picture (its palette and big-sprite frame)
    const u8 *picOfClass;
    u16 classes, pics;
    const u16 **picPal;
    struct MpBigSprite *picBig;
    const u8 *backs;   // Black's back pictures: u16 palette[16], four 64x64 frames of tiles each
    u16 backCount;
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
     || (sGfx.data[4] | sGfx.data[5] << 8) < 1 || (sGfx.data[4] | sGfx.data[5] << 8) > 2)
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
    // (the battle pictures: "TRPC", the classes' pictures, the pictures)
    if (sGfx.ok && o + 8 <= sGfx.size && memcmp(sGfx.data + o, "TRPC", 4) == 0)
    {
        u8 *p = sGfx.data + o + 4, *end = sGfx.data + sGfx.size;
        sGfx.classes = p[0] | p[1] << 8;
        sGfx.picOfClass = p + 2;
        p += 2 + sGfx.classes + (sGfx.classes & 1);
        sGfx.pics = p + 2 <= end ? (p[0] | p[1] << 8) : 0;
        p += 2;
        sGfx.picPal = calloc(sGfx.pics ? sGfx.pics : 1, sizeof(*sGfx.picPal));
        sGfx.picBig = calloc(sGfx.pics ? sGfx.pics : 1, sizeof(*sGfx.picBig));
        for (i = 0; i < sGfx.pics && p + 40 <= end; i++)
        {
            u16 w = p[36] | p[37] << 8, h = p[38] | p[39] << 8;
            sGfx.picPal[i] = (const u16 *)p;
            sGfx.picBig[i].frame = p + 32;
            sGfx.picBig[i].bpp = 4;
            sGfx.picBig[i].colorCount = 16;
            p += 40 + (u32)((w + 1) / 2) * h;
        }
        if (p > end)
            sGfx.pics = 0;
        else if (p + 6 <= end && memcmp(p, "BACK", 4) == 0)
        {
            sGfx.backCount = p[4] | p[5] << 8;
            sGfx.backs = p + 6;
            if (sGfx.backs + (u32)sGfx.backCount * (32 + 4 * 2048) > end)
                sGfx.backCount = 0;
        }
    }
    printf("[u3d] UNOVA's people: %u object codes, %u battle pictures\n", sGfx.count, sGfx.pics);
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

// MIXED's copy of object code CODE is AveonTrainer's (a Gen 5 person drawn in FRLG's style: before a Gen 3 equivalent)
bool8 U3D_Gfx_SmallIsArt(u16 code)
{
    if (!sGfx.tried)
        Load();
    return sGfx.ok && code < sGfx.count && sGfx.at[code] != 0 && sGfx.data[sGfx.at[code] + 1] == 1;
}

// battle_controller_opponent.c / _link_opponent.c: an UNOVA trainer's own picture, Black's (its class's), over the
// stand-in the battle loaded (TRAINER_UNOVA_A / _B's), drawn whole by the big-sprite renderer
void U3D_Gfx_BattleTrainerPic(struct Sprite *sprite, u8 battler, u16 standIn)
{
    u16 trainer;
    u8 pic;
    bool8 second = GetBattlerPosition(battler) == B_POSITION_OPPONENT_RIGHT && (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS);
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return;
    trainer = second ? gTrainerBattleOpponent_B : gTrainerBattleOpponent_A;
    if (!U3D_IsTrainer(trainer) || gTrainers[trainer].trainerPic != standIn)
        return;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || sGfx.pics == 0 || U3D_TrainerClass(trainer) >= sGfx.classes)
        return;
    pic = sGfx.picOfClass[U3D_TrainerClass(trainer)];
    if (pic >= sGfx.pics)
        return;
    LoadPalette(sGfx.picPal[pic], OBJ_PLTT_ID(sprite->oam.paletteNum), PLTT_SIZE_4BPP);
    sprite->y -= (8 - gTrainerFrontPicCoords[standIn].size) * 4;   // (the stand-in's own height undone)
    sprite->mpBig = &sGfx.picBig[pic];
    printf("[test] unova battle picture: trainer %d picture %d\n", trainer, pic);
}

// Black's back picture INDEX (U3D_BACK_*): four 64x64 frames as GBA tiles and their 16 colours; FALSE without them
bool8 U3D_Gfx_Back(u8 index, const u8 **tiles, const u16 **pal)
{
    const u8 *b;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || index >= sGfx.backCount)
        return FALSE;
    b = sGfx.backs + (u32)index * (32 + 4 * 2048);
    *pal = (const u16 *)b;
    *tiles = b + 32;
    return TRUE;
}
