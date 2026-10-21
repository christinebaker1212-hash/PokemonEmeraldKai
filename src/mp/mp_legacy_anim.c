// Moemon Co-op: the GENESIS / EPOCH BALL throws: Pokémon Red's and Crystal's own
// POKé BALL animations, art and sounds.
//
// tools/moemon_legacy/anim_bake.py ran each game's animation engine offline and
// wrote every frame of every outcome (mp_legacy_anim_data.h): the sprites in Game
// Boy screen coordinates, with the original tiles and palettes, and the events
// (sounds, the target hiding / showing, Crystal's picture-shrink steps). This file
// plays the timeline for the outcome the battle already decided (the caseId in
// BALLTHROWANIM), one step per real frame (ANIM SPEED and the battle turbo never
// change it), mapping the Game Boy's thrower / target onto the Kai battlers, and
// draws it over the frame (sdl2.c VDraw) like the other co-op layers. The sounds
// are the originals rendered by tools/moemon_legacy/sfx_render.py, mixed in by
// MP_LegacySfx_Mix (music_player.c m4aSoundVSync).
//
// Started from Special_BallThrow (data/battle_anim_scripts.s): AnimTask_MpIsLegacyBall
// sends the throw to AnimTask_MpLegacyBallThrow for these two balls.
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_main.h"
#include "main.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "m4a.h"
#include "constants/battle_anim.h"
#include "constants/items.h"
#include "constants/rgb.h"

struct LegacyAnimSprite { s16 x, y; u8 tile, flags; };    // flags: bit 0 X flip, 1 Y flip, 4-7 palette
struct LegacyAnimFrame { u16 first, count; };
struct LegacyAnimEvent { u16 frame; u8 type, arg, arg2; };
struct LegacyAnimTimeline { const struct LegacyAnimFrame *frames; const struct LegacyAnimSprite *sprites;
                            const struct LegacyAnimEvent *events; u16 length; };
#include "mp_legacy_anim_data.h"

enum { EV_SOUND = 1, EV_HIDE_TARGET, EV_SHOW_TARGET, EV_TARGET_SIZE };
enum { TL_RED_MISS, TL_RED_1, TL_RED_2, TL_RED_3, TL_RED_CAUGHT,
       TL_GSC_0, TL_GSC_1, TL_GSC_2, TL_GSC_3, TL_GSC_CAUGHT, TL_RED_POOF, TL_GSC_POOF };

// Red's sprites: the GENESIS BALL's own colors for the Game Boy's four shades
static const u16 sRedShades[4] = {RGB(31, 31, 31), RGB(31, 24, 24), RGB(31, 16, 16), RGB(4, 4, 4)};

static struct {
    bool8 active;      // a throw is playing
    bool8 kept;        // the caught ball stays on the screen after the throw
    u8 game;           // MP_SPECIAL_GENESIS / MP_SPECIAL_EPOCH
    u8 timeline;
    u16 frame;         // the frame on screen
    u16 nextEvent;
    u8 target, attacker;
    u8 targetSize;     // Crystal: 7 the Moemon as it is, 5 / 3 its shrunk picture, 0 gone
    u32 lastTick;
    s32 ax, ay, tx, ty;   // the thrower's / target's anchors on the Kai screen (picture bottom centre)
} sAnim;

static const struct LegacyAnimTimeline *Timeline(void)
{
    return &sLegacyTimelines[sAnim.timeline];
}

// ---------------------------------------------------------------------------
// The script's hooks

// Special_BallThrow: 1 for a GENESIS / EPOCH BALL that isn't blocked (they play their own throw)
void AnimTask_MpIsLegacyBall(u8 taskId)
{
    gBattleAnimArgs[ARG_RET_ID] = MP_LegacyBall_Kind(gLastUsedItem)
        && gBattleSpritesDataPtr->animationData->ballThrowCaseId != BALL_TRAINER_BLOCK;
    DestroyAnimVisualTask(taskId);
}

static void SetTargetSize(u8 size)
{
    struct Sprite *mon = &gSprites[gBattlerSpriteIds[sAnim.target]];
    sAnim.targetSize = size;
    if (gBattlerSpriteIds[sAnim.target] < MAX_SPRITES)
        mon->invisible = size != 7;   // the shrunk steps are drawn over the frame
}

static void RunEvent(const struct LegacyAnimEvent *e)
{
    switch (e->type)
    {
    case EV_SOUND:
        MP_LegacySfx_Play(e->arg, e->arg2);
        break;
    case EV_HIDE_TARGET:
        SetTargetSize(0);
        break;
    case EV_SHOW_TARGET:
        SetTargetSize(7);
        break;
    case EV_TARGET_SIZE:
        SetTargetSize(e->arg);
        break;
    }
}

#define tState data[0]

static void Task_LegacyBallThrow(u8 taskId)
{
    const struct LegacyAnimTimeline *tl = Timeline();
    // one step a real frame: the battle turbo may run tasks twice in one frame
    if (sAnim.lastTick == gMain.vblankCounter1)
        return;
    sAnim.lastTick = gMain.vblankCounter1;

    if (gTasks[taskId].tState == 0)
    {
        while (tl->events[sAnim.nextEvent].frame == sAnim.frame)
            RunEvent(&tl->events[sAnim.nextEvent++]);
        if (sAnim.frame + 1 < tl->length)
        {
            sAnim.frame++;
            return;
        }
        gTasks[taskId].tState = 1;   // the last frame stays up while its sounds finish
    }
    // WaitForSoundToFinish / WaitSFX
    if (MP_LegacySfx_Playing())
        return;
    sAnim.active = FALSE;
    if (gBattleSpritesDataPtr->animationData->ballThrowCaseId == BALL_3_SHAKES_SUCCESS)
    {
        u8 monSprite = gBattlerSpriteIds[sAnim.target];
        sAnim.kept = TRUE;           // the ball stays where it landed
        SetTargetSize(0);
        // the Moemon is in the ball: its sprite goes, as Emerald's own capture does
        if (monSprite < MAX_SPRITES && gSprites[monSprite].inUse)
        {
            FreeOamMatrix(gSprites[monSprite].oam.matrixNum);
            DestroySprite(&gSprites[monSprite]);
        }
        // the battle music stops here, as with the other balls; the catch jingle plays with the message
        if (!(gMpCoopWild && MP_Coop_OtherWildOut(sAnim.target)))
            m4aMPlayAllStop();
    }
    else
    {
        SetTargetSize(7);
    }
    gDoingBattleAnim = FALSE;
    DestroyAnimVisualTask(taskId);
}

#undef tState

static void Anchor(u8 battler, s32 *x, s32 *y)
{
    *x = GetBattlerSpriteCoord(battler, BATTLER_COORD_X);
    *y = GetBattlerSpriteCoord(battler, BATTLER_COORD_Y) + 32;
}

// Special_MpLegacyBallThrow
void AnimTask_MpLegacyBallThrow(u8 taskId)
{
    u8 caseId = gBattleSpritesDataPtr->animationData->ballThrowCaseId;
    sAnim.game = MP_LegacyBall_Kind(gLastUsedItem);
    if (sAnim.game == MP_SPECIAL_GENESIS)
        sAnim.timeline = caseId == BALL_MP_MISS ? TL_RED_MISS
                       : caseId == BALL_3_SHAKES_SUCCESS ? TL_RED_CAUGHT
                       : (caseId >= BALL_1_SHAKE && caseId <= BALL_3_SHAKES_FAIL) ? TL_RED_1 + caseId - BALL_1_SHAKE
                       : TL_RED_MISS;
    else
        sAnim.timeline = caseId == BALL_3_SHAKES_SUCCESS ? TL_GSC_CAUGHT
                       : caseId <= BALL_3_SHAKES_FAIL ? TL_GSC_0 + caseId : TL_GSC_0;
    sAnim.active = TRUE;
    sAnim.kept = FALSE;
    sAnim.frame = 0;
    sAnim.nextEvent = 0;
    sAnim.lastTick = gMain.vblankCounter1 - 1;
    sAnim.target = gBattleAnimTarget;
    sAnim.attacker = gBattleAnimAttacker;
    sAnim.targetSize = 7;
    Anchor(sAnim.attacker, &sAnim.ax, &sAnim.ay);
    Anchor(sAnim.target, &sAnim.tx, &sAnim.ty);
    gBattleSpritesDataPtr->animationData->wildMonInvisible = gSprites[gBattlerSpriteIds[gBattleAnimTarget]].invisible;
    MPNet_Log("legacy anim: %s timeline %d (case %d), b%d -> b%d", sAnim.game == MP_SPECIAL_GENESIS ? "GENESIS" : "EPOCH",
              sAnim.timeline, caseId, sAnim.attacker, sAnim.target);
    gTasks[taskId].func = Task_LegacyBallThrow;
    gTasks[taskId].data[0] = 0;
    Task_LegacyBallThrow(taskId);
}

// The caught ball goes (a co-op battle carries on without that Moemon, or the battle ends)
void MP_LegacyAnim_ClearKept(u8 battler)
{
    if (sAnim.kept && (battler == sAnim.target || battler >= MAX_BATTLERS_COUNT))
        sAnim.kept = FALSE;
}

bool8 MP_LegacyAnim_Active(void)
{
    return sAnim.active;
}

// ---------------------------------------------------------------------------
// Drawing

static inline u16 Mix555(u16 a, u16 b, u8 k)   // k/16 of b over a
{
    u16 r = ((a & 31) * (16 - k) + (b & 31) * k) >> 4;
    u16 g = (((a >> 5) & 31) * (16 - k) + ((b >> 5) & 31) * k) >> 4;
    u16 bl = (((a >> 10) & 31) * (16 - k) + ((b >> 10) & 31) * k) >> 4;
    return r | (g << 5) | (bl << 10);
}

static u16 Faded(u16 c)
{
    if (gPaletteFade.active || gPaletteFade.y)
    {
        u8 y = gPaletteFade.y > 16 ? 16 : gPaletteFade.y;
        if (y)
            c = Mix555(c, gPaletteFade.blendColor, y);
    }
    return c | 0x8000;
}

// Game Boy screen -> Kai screen: the Game Boy's player picture (bottom centre 38,96) and
// enemy picture (124,56) onto the thrower's and the target's
static void MapPoint(s32 gx, s32 gy, s32 *x, s32 *y)
{
    *x = sAnim.ax + (gx - 38) * (sAnim.tx - sAnim.ax) / (124 - 38);
    *y = sAnim.ay + (gy - 96) * (sAnim.ty - sAnim.ay) / (56 - 96);
}

// The battle's text box (from y 112) stays in front, as the GB games' did
#define CLIP_BOTTOM 112

static inline void Plot(u16 *frame, s32 x, s32 y, u16 c)
{
    if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < CLIP_BOTTOM)
        frame[y * DISPLAY_WIDTH + x] = c;
}

static void DrawTile(u16 *frame, const u8 *tile, s32 x0, s32 y0, u8 flags, const u16 *colors)
{
    s32 x, y;
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
        {
            s32 sx = (flags & 1) ? 7 - x : x, sy = (flags & 2) ? 7 - y : y;
            u8 v = tile[sy * 8 + sx];
            if (v)
                Plot(frame, x0 + x, y0 + y, Faded(colors[v]));
        }
}

#define MAX_FRAME_SPRITES 40   // the timelines have up to 20

static void DrawSprites(u16 *frame, u16 index)
{
    const struct LegacyAnimTimeline *tl = Timeline();
    const struct LegacyAnimFrame *f = &tl->frames[index];
    s32 i;
    u16 gscColors[2][4];
    u8 p, c;
    for (p = 0; p < 2; p++)
        for (c = 0; c < 4; c++)
            gscColors[p][c] = RGB(sGscAnimPalettes[p][c][0], sGscAnimPalettes[p][c][1], sGscAnimPalettes[p][c][2]);
    // Tiles that touch are one object (the ball is four of them): the object's centre moves
    // with the battlers and its tiles keep their places around it, so no gaps open between them
    u8 group[MAX_FRAME_SPRITES], n = f->count < MAX_FRAME_SPRITES ? f->count : MAX_FRAME_SPRITES, j, g;
    s32 offX[MAX_FRAME_SPRITES], offY[MAX_FRAME_SPRITES];
    for (i = 0; i < n; i++)
        group[i] = i;
    for (i = 0; i < n; i++)
        for (j = 0; j < i; j++)
        {
            const struct LegacyAnimSprite *a = &tl->sprites[f->first + i], *b = &tl->sprites[f->first + j];
            if (a->x - b->x <= 8 && b->x - a->x <= 8 && a->y - b->y <= 8 && b->y - a->y <= 8 && group[i] != group[j])
            {
                u8 from = group[i], to = group[j];
                for (g = 0; g < n; g++)
                    if (group[g] == from)
                        group[g] = to;
            }
        }
    for (g = 0; g < n; g++)
    {
        s32 minX = 0x7FFF, minY = 0x7FFF, maxX = -0x7FFF, maxY = -0x7FFF, cx, cy, x, y;
        for (i = 0; i < n; i++)
            if (group[i] == g)
            {
                const struct LegacyAnimSprite *s = &tl->sprites[f->first + i];
                minX = min(minX, s->x); maxX = max(maxX, s->x);
                minY = min(minY, s->y); maxY = max(maxY, s->y);
            }
        if (minX > maxX)
            continue;
        cx = (minX + maxX + 8) / 2;
        cy = (minY + maxY + 8) / 2;
        MapPoint(cx, cy, &x, &y);
        for (i = 0; i < n; i++)
            if (group[i] == g)
            {
                offX[i] = x - cx;
                offY[i] = y - cy;
            }
    }
    // the first OAM entry is on top: draw from the last
    for (i = n - 1; i >= 0; i--)
    {
        const struct LegacyAnimSprite *s = &tl->sprites[f->first + i];
        if (sAnim.game == MP_SPECIAL_GENESIS)
            DrawTile(frame, sRedAnimTiles[s->tile], s->x + offX[i], s->y + offY[i], s->flags, sRedShades);
        else
            DrawTile(frame, sGscAnimTiles[s->tile], s->x + offX[i], s->y + offY[i], s->flags, gscColors[(s->flags >> 4) & 1]);
    }
}

// The picture-shrink steps both games use (Crystal's BattleBGEffect_ReturnMon / EnterMon,
// Red's AnimateSendingOutMon / AnimateRetreatingPlayerMon): the Moemon drawn from a subset
// of its own tiles, 5x5 or 3x3 of them (Red also a single tile), centred on its bottom
// edge. Their 7x7 pictures keep tiles {0,1,3,5,6} / {0,3,6} / {3} of a row; a Kai picture
// is 8x8 tiles: the same places.
static const u8 sShrink5[5] = {0, 1, 4, 6, 7};
static const u8 sShrink3[3] = {0, 4, 7};
static const u8 sShrink1Row[1] = {7}, sShrink1Col[1] = {4};

static void DrawPicSubset(u16 *frame, u8 spriteId, u8 size)
{
    struct Sprite *s;
    const u8 *tiles, *rows, *cols;
    u8 n, r, c, px, py;
    s32 left, top;
    if (spriteId >= MAX_SPRITES)
        return;
    s = &gSprites[spriteId];
    if (!s->inUse || s->oam.bpp != ST_OAM_4BPP || s->oam.shape != ST_OAM_SQUARE || s->oam.size != ST_OAM_SIZE_3)
        return;
    n = size == 5 ? 5 : size == 3 ? 3 : 1;
    rows = n == 5 ? sShrink5 : n == 3 ? sShrink3 : sShrink1Row;
    cols = n == 5 ? sShrink5 : n == 3 ? sShrink3 : sShrink1Col;
    tiles = (const u8 *)OBJ_VRAM0 + s->oam.tileNum * TILE_SIZE_4BPP;
    left = s->x + s->x2 + (s->coordOffsetEnabled ? gSpriteCoordOffsetX : 0) - 32 + (64 - n * 8) / 2;
    top = s->y + s->y2 + (s->coordOffsetEnabled ? gSpriteCoordOffsetY : 0) - 32 + (64 - n * 8);
    for (r = 0; r < n; r++)
        for (c = 0; c < n; c++)
        {
            const u8 *t = tiles + (rows[r] * 8 + cols[c]) * TILE_SIZE_4BPP;
            for (py = 0; py < 8; py++)
                for (px = 0; px < 8; px++)
                {
                    u8 v = t[py * 4 + px / 2];
                    v = (px & 1) ? (v >> 4) : (v & 0xF);
                    if (v)
                        Plot(frame, left + c * 8 + px, top + r * 8 + py,
                             Faded(gPlttBufferUnfaded[OBJ_PLTT_ID(s->oam.paletteNum) + v]));
                }
        }
}

static void DrawShrunkTarget(u16 *frame)
{
    if (gBattlerSpriteIds[sAnim.target] < MAX_SPRITES)
        DrawPicSubset(frame, gBattlerSpriteIds[sAnim.target], sAnim.targetSize);
}

// ---------------------------------------------------------------------------
// Send-outs and recalls of a Moemon caught in one of these balls (its own game's,
// unless its trainer picked an effect of their own: mp_ballfx.c)
//   Red  out:    POOF_ANIM at the Moemon, then 1 tile (3 frames), 3x3 (4), 5x5 (5), all of it
//   Red  back:   5x5 (4 frames), 3x3 (3), gone (AnimateRetreatingPlayerMon)
//   Crystal out: SFX_BALL_POOF + BALL_POOF, 5 frames later ENTER_MON (3x3, 5x5, 7x7 every
//                4 frames), done 33 frames after it (BattleAnim_SendOutMon)
//   Crystal back: SFX_BALL_POOF + RETURN_MON (5x5, 3x3, gone every 4 frames), 33 frames
//                (BattleAnim_ReturnMon)
struct LegacyStep { u8 at, size; };
static const struct LegacyStep sRedOut[] = {{0, 0}, {40, 1}, {43, 3}, {47, 5}, {52, 7}, {52, 0xFF}};
static const struct LegacyStep sRedBack[] = {{0, 5}, {4, 3}, {7, 0}, {7, 0xFF}};
static const struct LegacyStep sGscOut[] = {{0, 0}, {5, 3}, {9, 5}, {13, 7}, {41, 0xFF}};
static const struct LegacyStep sGscBack[] = {{0, 7}, {4, 5}, {8, 3}, {12, 0}, {36, 0xFF}};

static struct {
    bool8 active, recall;
    u8 game, sprite, size, poof;   // poof: the smoke timeline playing (0xFF none)
    u16 t, poofEvent;
    s32 x, y;                      // the Moemon's picture bottom centre
    bool8 mirror;                  // a back picture (Red mirrors its smoke onto the player's side)
    u32 lastTick;
    const struct LegacyStep *steps;
} sMove[MAX_BATTLERS_COUNT];

// The ball this battler's Moemon came in, if one of these and its own look applies
static u8 NativeGame(u8 battler, bool8 recall)
{
    struct Pokemon *mon = GetBattlerSide(battler) == B_SIDE_PLAYER ? &gPlayerParty[gBattlerPartyIndexes[battler]]
                                                                   : &gEnemyParty[gBattlerPartyIndexes[battler]];
    u8 special = MP_BallStyle_Special(MP_BallStyle_OfMon(mon));
    struct MpBallFx fx;
    if (special != MP_SPECIAL_GENESIS && special != MP_SPECIAL_EPOCH)
        return 0;
    MP_BallFx_OfMon(mon, &fx);
    if (recall ? fx.recall != MPFX_RECALL_DEFAULT : (fx.form != MPFX_FORM_DEFAULT || fx.part != MPFX_PART_DEFAULT))
        return 0;   // its trainer chose an effect of their own
    return special;
}

bool8 MP_LegacyAnim_NativeSendOut(u8 battler)
{
    return gMain.inBattle && NativeGame(battler, FALSE) != 0;
}

bool8 MP_LegacyAnim_NativeRecall(u8 battler)
{
    return gMain.inBattle && NativeGame(battler, TRUE) != 0;
}

static void Task_LegacyMove(u8 taskId);

static bool8 StartMove(u8 battler, bool8 recall)
{
    u8 game = NativeGame(battler, recall);
    if (!gMain.inBattle || game == 0 || battler >= MAX_BATTLERS_COUNT || gBattlerSpriteIds[battler] >= MAX_SPRITES)
        return FALSE;
    sMove[battler].active = TRUE;
    sMove[battler].recall = recall;
    sMove[battler].game = game;
    sMove[battler].sprite = gBattlerSpriteIds[battler];
    sMove[battler].t = 0;
    sMove[battler].poof = 0xFF;
    sMove[battler].poofEvent = 0;
    sMove[battler].mirror = GetBattlerSide(battler) == B_SIDE_PLAYER;
    sMove[battler].x = GetBattlerSpriteCoord(battler, BATTLER_COORD_X);
    sMove[battler].y = GetBattlerSpriteCoord(battler, BATTLER_COORD_Y) + 32;
    sMove[battler].lastTick = gMain.vblankCounter1 - 1;
    if (game == MP_SPECIAL_GENESIS)
        sMove[battler].steps = recall ? sRedBack : sRedOut;
    else
        sMove[battler].steps = recall ? sGscBack : sGscOut;
    sMove[battler].size = recall ? 7 : 0;
    if (!recall)
        sMove[battler].poof = game == MP_SPECIAL_GENESIS ? TL_RED_POOF : TL_GSC_POOF;
    else if (game == MP_SPECIAL_EPOCH)
        MP_LegacySfx_Play(5, sMove[battler].mirror ? 1 : 2);   // LEGACY_SFX_GSC_BALL_POOF on the Moemon's side
    gSprites[sMove[battler].sprite].invisible = TRUE;
    MPNet_Log("legacy anim: %s %s for b%d", game == MP_SPECIAL_GENESIS ? "GENESIS" : "EPOCH", recall ? "recall" : "send-out", battler);
    {
        u8 taskId = CreateTask(Task_LegacyMove, 3);
        gTasks[taskId].data[0] = battler;
        Task_LegacyMove(taskId);
    }
    return TRUE;
}

bool8 MP_LegacyAnim_StartSendOut(u8 battler)
{
    return StartMove(battler, FALSE);
}

bool8 MP_LegacyAnim_StartRecall(u8 battler)
{
    return StartMove(battler, TRUE);
}

bool8 MP_LegacyAnim_MoveRunning(u8 battler)
{
    return battler < MAX_BATTLERS_COUNT && sMove[battler].active;
}

static void Task_LegacyMove(u8 taskId)
{
    u8 b = gTasks[taskId].data[0], i;
    struct Sprite *mon = &gSprites[sMove[b].sprite];
    if (sMove[b].lastTick == gMain.vblankCounter1)
        return;
    sMove[b].lastTick = gMain.vblankCounter1;
    if (!gMain.inBattle || !sMove[b].active)
    {
        sMove[b].active = FALSE;
        DestroyTask(taskId);
        return;
    }
    // the smoke's sounds (Crystal's on the Moemon's side)
    if (sMove[b].poof != 0xFF)
    {
        const struct LegacyAnimTimeline *tl = &sLegacyTimelines[sMove[b].poof];
        u16 f = sMove[b].t;
        while (tl->events[sMove[b].poofEvent].frame == f)
        {
            const struct LegacyAnimEvent *e = &tl->events[sMove[b].poofEvent++];
            if (e->type == EV_SOUND)
                MP_LegacySfx_Play(e->arg, sMove[b].game == MP_SPECIAL_EPOCH ? (sMove[b].mirror ? 1 : 2) : 0);
        }
        if (f + 1 >= tl->length)
            sMove[b].poof = 0xFF;
    }
    for (i = 0; sMove[b].steps[i].size != 0xFF; i++)
        if (sMove[b].steps[i].at == sMove[b].t)
            sMove[b].size = sMove[b].steps[i].size;
    for (i = 0; sMove[b].steps[i].size != 0xFF; i++)
        ;
    if (sMove[b].t >= sMove[b].steps[i].at)
    {
        // done: the Moemon as it is (out), or gone (back)
        sMove[b].active = FALSE;
        mon->invisible = sMove[b].recall;
        DestroyTask(taskId);
        return;
    }
    mon->invisible = sMove[b].size != 7;   // the shrunk steps are drawn over the frame
    sMove[b].t++;
}

static void DrawMoves(u16 *frame)
{
    u8 b;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
    {
        if (!sMove[b].active)
            continue;
        if (sMove[b].size != 0 && sMove[b].size != 7)
            DrawPicSubset(frame, sMove[b].sprite, sMove[b].size);
        if (sMove[b].poof != 0xFF && sMove[b].t > 0)
        {
            const struct LegacyAnimTimeline *tl = &sLegacyTimelines[sMove[b].poof];
            const struct LegacyAnimFrame *f = &tl->frames[sMove[b].t - 1 < tl->length ? sMove[b].t - 1 : tl->length - 1];
            u16 colors[4];
            s32 i;
            u8 c;
            for (c = 0; c < 4; c++)
                colors[c] = sMove[b].game == MP_SPECIAL_GENESIS ? sRedShades[c]
                          : RGB(sGscAnimPalettes[0][c][0], sGscAnimPalettes[0][c][1], sGscAnimPalettes[0][c][2]);
            for (i = f->count - 1; i >= 0; i--)
            {
                const struct LegacyAnimSprite *s = &tl->sprites[f->first + i];
                // the enemy's picture bottom centre (124, 56) onto this Moemon's; Red mirrors it onto ours
                s32 dx = s->x - 124, dy = s->y - 56;
                u8 flags = s->flags;
                if (sMove[b].mirror && sMove[b].game == MP_SPECIAL_GENESIS)
                {
                    dx = -dx - 8;
                    flags ^= 1;
                }
                else if (sMove[b].mirror)
                {
                    dx -= 8;   // Crystal puts the player's smoke at (44, 96): 8 left, 8 up of where the foe's sits
                    dy -= 8;
                }
                DrawTile(frame, sMove[b].game == MP_SPECIAL_GENESIS ? sRedAnimTiles[s->tile] : sGscAnimTiles[s->tile],
                         sMove[b].x + dx, sMove[b].y + dy, flags, colors);
            }
        }
    }
}

void MP_LegacyAnim_Composite(u16 *frame)
{
    if (!gMain.inBattle)
    {
        u8 b;
        sAnim.active = sAnim.kept = FALSE;
        for (b = 0; b < MAX_BATTLERS_COUNT; b++)
            sMove[b].active = FALSE;
        return;
    }
    if (gMain.callback2 != BattleMainCB2)
        return;
    DrawMoves(frame);
    if (!sAnim.active && !sAnim.kept)
        return;
    if (sAnim.active && (sAnim.targetSize == 5 || sAnim.targetSize == 3))
        DrawShrunkTarget(frame);
    DrawSprites(frame, sAnim.active ? sAnim.frame : Timeline()->length - 1);
}
