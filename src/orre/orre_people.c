// ORRE's people (include/orre_people.h): Colosseum's character models as LOOK > CREATE recipes (field sprites) and
// Pokemon Showdown trainer sprites (battle pictures, 80x80 as drawn, like SINNOH's Platinum pictures but ORRE's own
// line). The table and the pictures: tools/orre/people.py (src/data/orre_people.h, graphics/orre/people_pics.bin:
// "ORPP", u16 count, then each: name[24], u8 bpp (4 / 8), u8 colors, u16 colors[] (0 transparent), the big sprite
// frame (s16 dx, s16 dy, u16 80, u16 80, then the pixels)).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "sprite.h"
#include "palette.h"
#include "task.h"
#include "event_data.h"
#include "data.h"
#include "battle.h"
#include "battle_anim.h"
#include "mp_bigsprite.h"
#include "orre.h"
#include "orre_people.h"
#include "constants/battle.h"

bool8 O3D_IsOrreGroup(u8 group);

struct OrrePerson
{
    const char *model;
    const char *recipe;   // MP_CREATED_BYTES as hex (MP_Created_FromHex)
    u8 pic;
};

#include "../data/orre_people.h"

static const u8 sPicData[] = INCBIN_U8("graphics/orre/people_pics.bin");

struct OrrePic
{
    struct MpBigSprite big;
    const u16 *pal;
    u8 colors;
    u16 extraOwn[240];
};

static struct OrrePic sPics[ORRE_PEOPLE_PIC_COUNT];
static bool8 sPicsRead, sPicsOk;
static u16 sFoe[2] = {ORRE_PERSON_NONE, ORRE_PERSON_NONE};

static void ReadPics(void)
{
    const u8 *p = sPicData, *end = sPicData + sizeof(sPicData);
    u32 i, n;
    sPicsRead = TRUE;
    if (sizeof(sPicData) < 6 || memcmp(p, "ORPP", 4) != 0 || (n = p[4] | p[5] << 8) != ORRE_PEOPLE_PIC_COUNT)
    {
        printf("[orre] people_pics.bin: another table (rerun tools/orre/people.py build)\n");
        return;
    }
    p += 6;
    for (i = 0; i < n; i++)
    {
        u8 bpp, colors;
        u32 bytes;
        if (p + 26 > end)
            return;
        bpp = p[24];
        colors = p[25];
        p += 26;
        bytes = bpp == 4 ? 40 * 80 : 80 * 80;
        if (p + colors * 2 + 8 + bytes > end)
            return;
        sPics[i].pal = (const u16 *)p;
        sPics[i].colors = colors;
        p += colors * 2;
        sPics[i].big.frame = p;
        sPics[i].big.bpp = bpp;
        sPics[i].big.colorCount = colors;
        if (colors > 16)
        {
            memcpy(sPics[i].extraOwn, sPics[i].pal + 16, (colors - 16) * 2);
            memcpy(sPics[i].big.extraShown, sPics[i].extraOwn, (colors - 16) * 2);
        }
        p += 8 + bytes;
    }
    sPicsOk = TRUE;
}

u16 Orre_People_Count(void)
{
    return ORRE_PEOPLE_COUNT;
}

u16 Orre_People_Find(const char *model)
{
    u16 i;
    for (i = 0; i < ORRE_PEOPLE_COUNT; i++)
        if (strcmp(sOrrePeople[i].model, model) == 0)
            return i;
    return ORRE_PERSON_NONE;
}

const char *Orre_People_Model(u16 person)
{
    return person < ORRE_PEOPLE_COUNT ? sOrrePeople[person].model : "";
}

bool8 Orre_People_Recipe(u16 person, struct MpCreated *out)
{
    return person < ORRE_PEOPLE_COUNT && MP_Created_FromHex(sOrrePeople[person].recipe, out);
}

const char *Orre_People_PicName(u16 person)
{
    return person < ORRE_PEOPLE_COUNT ? sOrrePicName[sOrrePeople[person].pic] : "";
}

const char *Orre_People_PicArtist(u16 person)
{
    return person < ORRE_PEOPLE_COUNT ? sOrrePicArtist[sOrrePeople[person].pic] : "";
}

// ---------------------------------------------------------------------------
// Field sprites: each person's 9 walking frames, built from their recipe the first time they're shown

static u8 (*sField[ORRE_PEOPLE_COUNT])[32 * 32 / 2];
static u16 sFieldPal[ORRE_PEOPLE_COUNT][16];
static u8 sFieldFrames[ORRE_PEOPLE_COUNT][9][32 * 32 / 2];

bool8 Orre_People_Field(u16 person, const u8 **frames, const u16 **pal)
{
    if (person >= ORRE_PEOPLE_COUNT)
        return FALSE;
    if (sField[person] == NULL)
    {
        static struct MpCreatedGfx sScratch;
        struct MpCreated r;
        if (!Orre_People_Recipe(person, &r))
            return FALSE;
        MP_Created_BuildField(&r, &sScratch);
        memcpy(sFieldFrames[person], sScratch.ow, sizeof(sFieldFrames[person]));
        memcpy(sFieldPal[person], sScratch.owPal, sizeof(sFieldPal[person]));
        sField[person] = sFieldFrames[person];
    }
    *frames = sField[person][0];
    *pal = sFieldPal[person];
    return TRUE;
}

// ---------------------------------------------------------------------------
// Battle pictures

void Orre_People_SetFoe(u8 which, u16 person)
{
    sFoe[which & 1] = person < ORRE_PEOPLE_COUNT ? person : ORRE_PERSON_NONE;
}

// ... unless a script has named one already (orre_battle.c: the trainer's own look)
void Orre_People_SetFoeDefault(u8 which, u16 person)
{
    if (sFoe[which & 1] == ORRE_PERSON_NONE)
        Orre_People_SetFoe(which, person);
}

void Orre_SetFoePerson(void)
{
    Orre_People_SetFoe(gSpecialVar_0x8005, gSpecialVar_0x8004);
}

void Orre_People_BattleEnd(void)
{
    sFoe[0] = sFoe[1] = ORRE_PERSON_NONE;
}

// 8 bpp pictures: colors 16+ fade with the sprite's palette slot (the slot holds colors 0-15), as long as the
// picture is on the sprite
#define tSprite data[0]
#define tPic    data[1]

static void Task_PicFade(u8 taskId)
{
    struct Sprite *sprite = &gSprites[gTasks[taskId].tSprite];
    struct OrrePic *pic = &sPics[gTasks[taskId].tPic];
    struct MpFadeFit fit;
    u16 slot, i;
    if (!sprite->inUse || sprite->mpBig != &pic->big)
    {
        DestroyTask(taskId);
        return;
    }
    slot = OBJ_PLTT_ID(sprite->oam.paletteNum);
    MP_FadeFit_FromColors(&fit, &gPlttBufferUnfaded[slot], &gPlttBufferFaded[slot]);
    for (i = 0; i + 16 < pic->colors; i++)
        pic->big.extraShown[i] = MP_FadeFit_Apply(&fit, pic->extraOwn[i]);
}

void Orre_People_DrawPic(struct Sprite *sprite, u16 person, u16 standIn)
{
    struct OrrePic *pic;
    u16 pal[16];
    u8 t;
    if (person >= ORRE_PEOPLE_COUNT)
        return;
    if (!sPicsRead)
        ReadPics();
    if (!sPicsOk)
        return;
    pic = &sPics[sOrrePeople[person].pic];
    memset(pal, 0, sizeof(pal));
    memcpy(pal, pic->pal, (pic->colors < 16 ? pic->colors : 16) * 2);
    LoadPalette(pal, OBJ_PLTT_ID(sprite->oam.paletteNum), PLTT_SIZE_4BPP);
    sprite->y -= (8 - gTrainerFrontPicCoords[standIn].size) * 4;   // (the stand-in's own height undone)
    sprite->mpBig = &pic->big;
    if (pic->colors > 16)
    {
        t = CreateTask(Task_PicFade, 3);
        gTasks[t].tSprite = sprite - gSprites;
        gTasks[t].tPic = sOrrePeople[person].pic;
        Task_PicFade(t);
    }
    printf("[test] orre battle picture: %s (%s, %d colors)\n", sOrrePeople[person].model,
           sOrrePicName[sOrrePeople[person].pic], pic->colors);
}

void Orre_People_BattleTrainerPic(struct Sprite *sprite, u8 battler, u16 standIn)
{
    bool8 second = GetBattlerPosition(battler) == B_POSITION_OPPONENT_RIGHT
                && ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) || (gMpCoopBattle && MP_Coop_TwoTrainers()));
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER) || ((gBattleTypeFlags & BATTLE_TYPE_LINK) && !gMpCoopBattle))
        return;
    if (!O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup))
        return;
    Orre_People_DrawPic(sprite, sFoe[second], standIn);
}

bool8 Orre_People_PicPixels(u16 person, const u8 **pixels, u8 *bpp, const u16 **pal, u8 *colors)
{
    struct OrrePic *pic;
    if (person >= ORRE_PEOPLE_COUNT)
        return FALSE;
    if (!sPicsRead)
        ReadPics();
    if (!sPicsOk)
        return FALSE;
    pic = &sPics[sOrrePeople[person].pic];
    *pixels = pic->big.frame + 8;
    *bpp = pic->big.bpp;
    *pal = pic->pal;
    *colors = pic->colors;
    return TRUE;
}
