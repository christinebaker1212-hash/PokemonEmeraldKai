// Moemon Kai: the foes close ranks (the user, 2026-11-17). In a Double or Triple Battle, when a foe leaves the field
// for good (fainted with nobody to take its place, caught, an empty co-op wild slot: gAbsentBattlerFlags), the ones
// left slide to the places that many POKéMON would have had from the start - two to the Double Battle's two spots, one
// to the single battle's - keeping their order left to right, and their HP boxes close the gap upward (a box only ever
// rises). Both move together, smoothly. Only the foes' side; looks only (Kai's own battle rules: Black's Triple Battle
// rules are never considered).
//
// The offsets reach everything that places a foe: GetBattlerSpriteCoord (battle_anim_mons.c: every move animation,
// POKé BALL, shadow and weather), the battler sprites themselves (moved here), KAI's Double Battle healthboxes (moved
// here, their parts following), the Triple cards (mp_tricards.c CardPlace) and every UI STYLE's boxes
// (mp_uistyle.c BattleHuds). The move waits for a settled field: every foe on it showing its box and no move
// animation playing (a sprite being sent out or animated is never pushed), and pauses while one plays.
#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "main.h"
#include "sprite.h"
#include "constants/battle.h"
#include "util.h"
#include "mp.h"

#define SHIFT_FRAMES 20

void MP_BattlerSlotCoord(u8 row, u8 position, s16 *x, s16 *y);   // battle_anim_mons.c (sBattlerCoords)

static s16 sCurX[MAX_BATTLERS_COUNT], sCurY[MAX_BATTLERS_COUNT];     // a foe's sprite offset now, in pixels
static s16 sFromX[MAX_BATTLERS_COUNT], sFromY[MAX_BATTLERS_COUNT], sToX[MAX_BATTLERS_COUNT], sToY[MAX_BATTLERS_COUNT];
static s32 sRow[MAX_BATTLERS_COUNT] = {-1, -1, -1, -1, -1, -1}, sRowFrom[MAX_BATTLERS_COUNT], sRowTo[MAX_BATTLERS_COUNT];   // box row x 256
static u8 sT, sKey = 0xFF;     // (frames into the move; the foes on the field the targets were made for)
static bool8 sMoving;

// the foes' positions as the screen shows them, left to right, and as their boxes stack, top to bottom
static const u8 sScreen3[3] = {B_POSITION_OPPONENT_RIGHT, B_POSITION_OPPONENT_CENTER, B_POSITION_OPPONENT_LEFT};
static const u8 sScreen2[2] = {B_POSITION_OPPONENT_RIGHT, B_POSITION_OPPONENT_LEFT};
static const u8 sRows3[3] = {B_POSITION_OPPONENT_RIGHT, B_POSITION_OPPONENT_CENTER, B_POSITION_OPPONENT_LEFT};
static const u8 sRows2[2] = {B_POSITION_OPPONENT_LEFT, B_POSITION_OPPONENT_RIGHT};
// KAI's Double Battle foe boxes, row by row (battle_interface.c InitBattlerHealthboxCoords)
static const s16 sBox2[2][2] = {{44, 19}, {32, 44}};

static bool8 On(void)
{
    return gMain.inBattle && IsDoubleBattle() && gBattlersCount > 2;
}

static u8 Slots(void)
{
    return IS_TRIPLE_BATTLE() ? 3 : 2;
}

static const u8 *ScreenOrder(void)
{
    return IS_TRIPLE_BATTLE() ? sScreen3 : sScreen2;
}

static const u8 *RowOrder(void)
{
    return IS_TRIPLE_BATTLE() ? sRows3 : sRows2;
}

static s32 OwnRow(u8 battler)
{
    u8 pos = GetBattlerPosition(battler), i;
    for (i = 0; i < Slots(); i++)
        if (RowOrder()[i] == pos)
            return i * 256;
    return 0;
}

static bool8 IsFoe(u8 b)
{
    return b < gBattlersCount && GetBattlerSide(b) == B_SIDE_OPPONENT;
}

// the foes on the field: a bit per battler
static u8 Present(void)
{
    u8 b, mask = 0;
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
        if (IsFoe(b) && !(gAbsentBattlerFlags & gBitTable[b]))
            mask |= 1 << b;
    return mask;
}

static u8 BattlerAt(u8 position)
{
    u8 b;
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
        if (GetBattlerPosition(b) == position)
            return b;
    return MAX_BATTLERS_COUNT;
}

// where each foe goes when MASK is on the field
static void Targets(u8 mask)
{
    const u8 *screen = ScreenOrder(), *rows = RowOrder();
    u8 i, k = 0, n = Slots(), b, idx;
    s16 tx, ty, hx, hy;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
        sToX[b] = sCurX[b], sToY[b] = sCurY[b], sRowTo[b] = sRow[b];
    for (i = 0; i < n; i++)
        if ((b = BattlerAt(screen[i])) < MAX_BATTLERS_COUNT && (mask & (1 << b)))
            k++;
    if (k == 0)
        return;
    // the sprites: the k on the field take the spots of a battle of k, in their order
    for (i = 0, idx = 0; i < n; i++)
    {
        if ((b = BattlerAt(screen[i])) >= MAX_BATTLERS_COUNT || !(mask & (1 << b)))
            continue;
        MP_BattlerSlotCoord(IS_TRIPLE_BATTLE() ? 2 : 1, screen[i], &hx, &hy);   // (its own spot)
        if (k == 3)
            MP_BattlerSlotCoord(2, sScreen3[idx], &tx, &ty);
        else if (k == 2)
            MP_BattlerSlotCoord(1, sScreen2[idx], &tx, &ty);
        else
            MP_BattlerSlotCoord(0, B_POSITION_OPPONENT_LEFT, &tx, &ty);
        sToX[b] = tx - hx, sToY[b] = ty - hy;
        idx++;
    }
    // the boxes: the k on the field stack from the top, in their order
    for (i = 0, k = 0; i < n; i++)
        if ((b = BattlerAt(rows[i])) < MAX_BATTLERS_COUNT && (mask & (1 << b)))
            sRowTo[b] = (k++) * 256;
}

void MP_FoeShift_Reset(void)
{
    u8 b;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
        sCurX[b] = sCurY[b] = 0, sRow[b] = -1;
    sMoving = FALSE;
    sKey = 0xFF;
}

static void BoxAt(s32 row, s16 *x, s16 *y)
{
    s32 r0 = row >> 8, f = row & 255;
    if (r0 < 0)
        r0 = 0, f = 0;
    if (r0 >= 1)
        r0 = 1, f = 0;
    *x = sBox2[r0][0] + (r0 + 1 < 2 ? ((sBox2[r0 + 1][0] - sBox2[r0][0]) * f) >> 8 : 0);
    *y = sBox2[r0][1] + (r0 + 1 < 2 ? ((sBox2[r0 + 1][1] - sBox2[r0][1]) * f) >> 8 : 0);
}

// (a foe on the field with its box showing: its sprite ids are its own now)
static bool8 Settled(u8 b)
{
    struct Sprite *box;
    if (gHealthboxSpriteIds[b] >= MAX_SPRITES || gBattlerSpriteIds[b] >= MAX_SPRITES)
        return FALSE;
    box = &gSprites[gHealthboxSpriteIds[b]];
    return box->inUse && !box->invisible && gSprites[gBattlerSpriteIds[b]].inUse;
}

void MP_FoeShift_Update(void)
{
    u8 b, mask;
    if (!On())
        return;
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
        if (IsFoe(b) && sRow[b] < 0)
            sRow[b] = OwnRow(b);
    mask = Present();
    if (sKey == 0xFF)
    {
        // (the battle's first look: everyone where the battle put them)
        u8 all = 0;
        for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
            if (IsFoe(b))
                all |= 1 << b;
        sKey = all;
    }
    if (!sMoving && mask != sKey)
    {
        if (gAnimScriptActive)
            return;
        for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
            if ((mask & (1 << b)) && !Settled(b))
                return;   // (a foe still coming out, or down and waiting for its replacement)
        Targets(mask);
        for (b = 0; b < MAX_BATTLERS_COUNT; b++)
            sFromX[b] = sCurX[b], sFromY[b] = sCurY[b], sRowFrom[b] = sRow[b];
        sKey = mask;
        sT = 0;
        sMoving = TRUE;
        for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
            if (IsFoe(b))
            {
                struct Sprite *mon = gBattlerSpriteIds[b] < MAX_SPRITES ? &gSprites[gBattlerSpriteIds[b]] : NULL;
                MPNet_Log("foe shift: b%d pos %d %s x %d inv %d -> dx %d dy %d row %d -> %d", b, GetBattlerPosition(b),
                          (mask & (1 << b)) ? "on" : "gone", mon ? mon->x : -1, mon ? mon->invisible : -1,
                          sToX[b], sToY[b], (int)sRow[b], (int)sRowTo[b]);
            }
        MPNet_Log("foe shift: %d on the field (mask %x)", (mask & 1) + ((mask >> 1) & 1) + ((mask >> 2) & 1)
                  + ((mask >> 3) & 1) + ((mask >> 4) & 1) + ((mask >> 5) & 1), mask);
    }
    if (!sMoving || gAnimScriptActive)
        return;
    sT++;
    {
        // (eased: slow, quick, slow)
        s32 t = (sT << 8) / SHIFT_FRAMES, e = (t * t * (768 - 2 * t)) >> 16;
        for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
        {
            s16 nx, ny, ox, oy, bx, by;
            s32 nr;
            if (!IsFoe(b))
                continue;
            nx = sFromX[b] + (((sToX[b] - sFromX[b]) * e) >> 8);
            ny = sFromY[b] + (((sToY[b] - sFromY[b]) * e) >> 8);
            nr = sRowFrom[b] + (((sRowTo[b] - sRowFrom[b]) * e) >> 8);
            if (sT >= SHIFT_FRAMES)
                nx = sToX[b], ny = sToY[b], nr = sRowTo[b];
            if ((mask & (1 << b)) && Settled(b))
            {
                struct Sprite *mon = &gSprites[gBattlerSpriteIds[b]];
                mon->x += nx - sCurX[b];
                mon->y += ny - sCurY[b];
                if (!IS_TRIPLE_BATTLE())
                {
                    // KAI's box (its other half, bar and caught ball follow it)
                    struct Sprite *box = &gSprites[gHealthboxSpriteIds[b]];
                    BoxAt(sRow[b], &ox, &oy);
                    BoxAt(nr, &bx, &by);
                    box->x += bx - ox;
                    box->y += by - oy;
                }
            }
            sCurX[b] = nx, sCurY[b] = ny, sRow[b] = nr;
        }
    }
    if (sT >= SHIFT_FRAMES)
        sMoving = FALSE;
}

s16 MP_FoeShift_Dx(u8 battler)
{
    return On() && battler < MAX_BATTLERS_COUNT && IsFoe(battler) ? sCurX[battler] : 0;
}

s16 MP_FoeShift_Dy(u8 battler)
{
    return On() && battler < MAX_BATTLERS_COUNT && IsFoe(battler) ? sCurY[battler] : 0;
}

// a foe's box row now, x 256 (0 the top; between two while it rises)
s32 MP_FoeShift_Row(u8 battler)
{
    if (!On() || battler >= MAX_BATTLERS_COUNT || !IsFoe(battler))
        return 0;
    return sRow[battler] < 0 ? OwnRow(battler) : sRow[battler];
}

// KAI's Double Battle foe box where its row is now (InitBattlerHealthboxCoords, a box made after a move)
bool8 MP_FoeShift_BoxPos(u8 battler, s16 *x, s16 *y)
{
    if (!On() || IS_TRIPLE_BATTLE() || battler >= MAX_BATTLERS_COUNT || !IsFoe(battler))
        return FALSE;
    BoxAt(MP_FoeShift_Row(battler), x, y);
    return TRUE;
}

// the position whose box a foe's box stands in now (its own while it moves): a style's box for that spot
u8 MP_FoeShift_ShownPos(u8 battler)
{
    s32 row;
    if (!On() || battler >= MAX_BATTLERS_COUNT || !IsFoe(battler) || sMoving)
        return GetBattlerPosition(battler);
    row = MP_FoeShift_Row(battler);
    if ((row & 255) || (row >> 8) >= Slots())
        return GetBattlerPosition(battler);
    return RowOrder()[row >> 8];
}
