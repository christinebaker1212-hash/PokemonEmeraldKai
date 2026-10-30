// Moemon Kai: the battle GIMMICKS' settings, pictures and menu (include/battle_gimmick.h; the battle side is in
// src/battle_gen59.c). Everything here runs on every screen of a link battle: the gimmick animation
// (B_ANIM_MP_GIMMICK) swaps a MEGA's picture in and tints TERA and DYNAMAX, and remembers it by party slot, so the
// picture comes back the same when the Pokémon is drawn again (a switch, the bag, a SUBSTITUTE).
#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_gimmick.h"
#include "contest.h"
#include "data.h"
#include "decompress.h"
#include "mp.h"
#include "mp_region.h"
#include "palette.h"
#include "pokemon.h"
#include "sprite.h"
#include "string_util.h"
#include "util.h"
#include "constants/abilities.h"
#include "constants/battle_anim.h"
#include "constants/species.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/rgb.h"

#include "data/mp_megas.h"

u8 MP_Mega_Count(void)
{
    return ARRAY_COUNT(sMegaForms);
}

const struct MegaForm *MP_Mega_Get(u8 form)
{
    return &sMegaForms[form < ARRAY_COUNT(sMegaForms) ? form : 0];
}

// ---------------------------------------------------------------------------------------------- settings

static const u8 sGimmickGen[GIMMICK_COUNT] = {0, 6, 7, 8, 9};   // AUTO turns each on from its generation

bool8 MP_Gimmick_On(u8 gimmick)
{
    if (gimmick == GIMMICK_NONE || gimmick >= GIMMICK_COUNT)
        return FALSE;
    switch (MP_Gimmick_Setting(gimmick))
    {
    case GIMMICK_SET_ON:  return TRUE;
    case GIMMICK_SET_OFF: return FALSE;
    }
    return MP_Generation() >= sGimmickGen[gimmick];
}

bool8 MP_Gimmick_ItemAllowed(u16 item)
{
    if (item >= ITEM_VENUSAURITE && item <= ITEM_DIANCITE)
        return MP_Gimmick_On(GIMMICK_MEGA);
    if (item >= ITEM_NORMALIUM_Z && item <= ITEM_FAIRIUM_Z)
        return MP_Gimmick_On(GIMMICK_Z);
    return TRUE;
}

// ---------------------------------------------------------------------------------------------- pictures

static EWRAM_DATA u8 sPicMega[2][PARTY_SIZE] = {0};   // MEGA form + 1, by side and party slot
static EWRAM_DATA u8 sPicTera[2][PARTY_SIZE] = {0};   // TERA type + 1
static EWRAM_DATA u8 sPicDyna[MAX_BATTLERS_COUNT] = {0};   // party slot + 1 of a DYNAMAX Pokémon

void MP_Gimmick_ResetPictures(void)
{
    memset(sPicMega, 0, sizeof(sPicMega));
    memset(sPicTera, 0, sizeof(sPicTera));
    memset(sPicDyna, 0, sizeof(sPicDyna));
}

// the TERA tints: each type's crystal color
static const u16 sTeraColor[NUMBER_OF_MON_TYPES] =
{
    [TYPE_NORMAL] = RGB(28, 28, 26), [TYPE_FIGHTING] = RGB(26, 10, 6), [TYPE_FLYING] = RGB(18, 22, 31),
    [TYPE_POISON] = RGB(22, 8, 26), [TYPE_GROUND] = RGB(26, 20, 10), [TYPE_ROCK] = RGB(22, 18, 10),
    [TYPE_BUG] = RGB(18, 24, 6), [TYPE_GHOST] = RGB(14, 10, 22), [TYPE_STEEL] = RGB(22, 24, 26),
    [TYPE_MYSTERY] = RGB(24, 24, 24), [TYPE_FIRE] = RGB(31, 14, 6), [TYPE_WATER] = RGB(8, 16, 31),
    [TYPE_GRASS] = RGB(10, 26, 10), [TYPE_ELECTRIC] = RGB(31, 26, 6), [TYPE_PSYCHIC] = RGB(31, 12, 20),
    [TYPE_ICE] = RGB(16, 28, 30), [TYPE_DRAGON] = RGB(14, 10, 31), [TYPE_DARK] = RGB(10, 8, 10),
    [TYPE_FAIRY] = RGB(31, 18, 26),
};

static void Tint(u8 battler, u16 color, u8 amount)
{
    u16 offset = OBJ_PLTT_ID(battler);
    BlendPalette(offset, 16, amount, color);
    CpuCopy32(&gPlttBufferFaded[offset], &gPlttBufferUnfaded[offset], PLTT_SIZEOF(16));
}

static void LoadMegaPicture(u8 battler, u8 form, struct Pokemon *mon)
{
    const struct MegaForm *f = MP_Mega_Get(form);
    u8 position = GetBattlerPosition(battler), i;
    u8 *buf = gMonSpritesGfxPtr->sprites.ptr[position];
    u16 offset = OBJ_PLTT_ID(battler);
    LZDecompressWram(GetBattlerSide(battler) == B_SIDE_PLAYER ? f->back : f->front, buf);
    for (i = 1; i < MAX_MON_PIC_FRAMES; i++)
        memcpy(buf + MON_PIC_SIZE * i, buf, MON_PIC_SIZE);   // (one frame: every frame of the sprite's animations)
    LZDecompressWram(IsMonShiny(mon) ? f->shinyPalette : f->palette, gDecompressionBuffer);
    LoadPalette(gDecompressionBuffer, offset, PLTT_SIZE_4BPP);
    LoadPalette(gDecompressionBuffer, BG_PLTT_ID(8) + BG_PLTT_ID(battler), PLTT_SIZE_4BPP);
}

static struct Pokemon *BattlerMon(u8 battler)
{
    return &(GetBattlerSide(battler) == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty)[gBattlerPartyIndexes[battler] % PARTY_SIZE];
}

void MP_Gimmick_AfterMonGfx(struct Pokemon *mon, u8 battler)
{
    u8 side = GetBattlerSide(battler), slot;
    struct Pokemon *party = side == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty;
    if (mon < party || mon >= party + PARTY_SIZE || battler >= MAX_BATTLERS_COUNT)
        return;
    slot = mon - party;
    if (sPicDyna[battler] && sPicDyna[battler] - 1 != slot)
        sPicDyna[battler] = 0;   // (another Pokémon came in)
    if (gBattleSpritesDataPtr->battlerData[battler].transformSpecies != SPECIES_NONE)
        return;
    if (sPicMega[side][slot] && MP_Mega_Get(sPicMega[side][slot] - 1)->species == GetMonData(mon, MON_DATA_SPECIES))
        LoadMegaPicture(battler, sPicMega[side][slot] - 1, mon);
    if (sPicTera[side][slot])
        Tint(battler, sTeraColor[(sPicTera[side][slot] - 1) % NUMBER_OF_MON_TYPES], 5);
    if (sPicDyna[battler])
        Tint(battler, RGB(31, 4, 8), 8);
}

u8 MP_Gimmick_YDelta(u8 battler, u8 yDelta)
{
    u8 side, slot, m;
    if (IsContest() || battler >= MAX_BATTLERS_COUNT || gBattleSpritesDataPtr == NULL
     || gBattleSpritesDataPtr->battlerData[battler].transformSpecies != SPECIES_NONE)
        return yDelta;
    side = GetBattlerSide(battler);
    slot = gBattlerPartyIndexes[battler] % PARTY_SIZE;
    m = sPicMega[side][slot];
    if (!m || MP_Mega_Get(m - 1)->species != GetMonData(BattlerMon(battler), MON_DATA_SPECIES))
        return yDelta;
    return side == B_SIDE_PLAYER ? MP_Mega_Get(m - 1)->backY : MP_Mega_Get(m - 1)->frontY;
}

// B_ANIM_MP_GIMMICK: a white flash on the Pokémon; at its peak the picture / tint changes
#define tState   data[0]
#define tCount   data[1]
#define tBattler data[2]
#define tArg     data[3]

static void ApplyGimmickPicture(u8 battler, u16 arg)
{
    u8 kind = arg >> 8, value = arg & 0xFF, side = GetBattlerSide(battler), slot = gBattlerPartyIndexes[battler] % PARTY_SIZE;
    struct Pokemon *mon = BattlerMon(battler);
    u8 spriteId = gBattlerSpriteIds[battler];
    switch (kind)
    {
    case GIMMICK_ANIM_MEGA:
        if (value >= MP_Mega_Count())
            break;
        sPicMega[side][slot] = value + 1;
        LoadMegaPicture(battler, value, mon);
        if (spriteId < MAX_SPRITES)
        {
            DmaCopy32Defvars(3, gMonSpritesGfxPtr->sprites.ptr[GetBattlerPosition(battler)],
                             (void *)(OBJ_VRAM0 + gSprites[spriteId].oam.tileNum * 32), MON_PIC_SIZE);
            gSprites[spriteId].y = GetBattlerSpriteDefault_Y(battler);
        }
        break;
    case GIMMICK_ANIM_TERA:
        sPicTera[side][slot] = value + 1;
        Tint(battler, sTeraColor[value % NUMBER_OF_MON_TYPES], 5);
        break;
    case GIMMICK_ANIM_DYNAMAX:
        sPicDyna[battler] = slot + 1;
        Tint(battler, RGB(31, 4, 8), 8);
        break;
    case GIMMICK_ANIM_DYNAMAX_END:
        sPicDyna[battler] = 0;
        if (side == B_SIDE_PLAYER)
            BattleLoadPlayerMonSpriteGfx(mon, battler);   // (its own palette again, MEGA / TERA kept)
        else
            BattleLoadOpponentMonSpriteGfx(mon, battler);
        break;
    }
}

void AnimTask_MpGimmick(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    u16 offset;
    switch (task->tState)
    {
    case 0:
        task->tBattler = gBattleAnimAttacker;
        task->tArg = gBattleSpritesDataPtr->animationData->animArg;
        task->tCount = 0;
        task->tState++;
        // fallthrough
    case 1:
        offset = OBJ_PLTT_ID(task->tBattler);
        BlendPalette(offset, 16, task->tCount * 2, RGB_WHITE);
        if (++task->tCount > 8)
        {
            ApplyGimmickPicture(task->tBattler, task->tArg);
            task->tCount = 8;
            task->tState++;
        }
        break;
    case 2:
        offset = OBJ_PLTT_ID(task->tBattler);
        BlendPalette(offset, 16, task->tCount * 2, RGB_WHITE);
        if (task->tCount-- == 0)
        {
            BlendPalette(offset, 16, 0, RGB_WHITE);
            DestroyAnimVisualTask(taskId);
        }
        break;
    }
}

#undef tState
#undef tCount
#undef tBattler
#undef tArg

// ---------------------------------------------------------------------------------------------- menus and the AI

const u8 *MP_Gimmick_MenuLabel(u8 gimmick, u8 type)
{
    static const u8 sMega[] = _("MEGA EVOLVE!");
    static const u8 sDyna[] = _("DYNAMAX!");
    static const u8 sZ[] = _("Z-MOVE!");
    static u8 sTera[24];
    switch (gimmick)
    {
    case GIMMICK_MEGA:    return sMega;
    case GIMMICK_DYNAMAX: return sDyna;
    case GIMMICK_Z:       return sZ;
    case GIMMICK_TERA:
    {
        static const u8 sTeraPrefix[] = _("TERA ");
        StringCopy(StringCopy(sTera, sTeraPrefix), gTypeNames[type < NUMBER_OF_MON_TYPES ? type : TYPE_NORMAL]);
        return sTera;
    }
    }
    return NULL;
}

// a trainer's Pokémon uses its gimmick when it's the last one its trainer has (MEGA as soon as it can)
u8 MP_Gimmick_AiPick(u8 battler, u16 move)
{
    u8 avail = gBattleBufferA[battler][3], i, alive = 0;
    struct Pokemon *party = GetBattlerSide(battler) == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty;
    if (avail == 0 || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return GIMMICK_NONE;
    if (avail & GIMMICK_AVAIL_MEGA)
        return GIMMICK_MEGA;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&party[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&party[i], MON_DATA_IS_EGG)
         && GetMonData(&party[i], MON_DATA_HP) != 0)
            alive++;
    if (alive > 1)
        return GIMMICK_NONE;
    if ((avail >> 3) && gBattleMoves[move].power != 0 && gBattleMoves[move].type == GIMMICK_AVAIL_Z_TYPE(avail))
        return GIMMICK_Z;
    if (avail & GIMMICK_AVAIL_TERA)
        return GIMMICK_TERA;
    if (avail & GIMMICK_AVAIL_DYNAMAX)
        return GIMMICK_DYNAMAX;
    return GIMMICK_NONE;
}
