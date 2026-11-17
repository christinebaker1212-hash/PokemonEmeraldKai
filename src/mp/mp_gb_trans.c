// Moemon Kai: OPTIONS > DISPLAY > GB OPTIONS > TRANSITIONS (RED / CRYSTAL): Pokémon Red's and Crystal's own battle
// transitions.
//
// tools/moemon_legacy/trans_bake.py ran both games' transition routines offline on the Game Boy's 20x18 tile map and
// wrote every frame (mp_gb_trans_data.h): which of the map's cells each cell shows (Red's Shrink and Split move rows
// and columns), the black tile, Crystal's Poke Ball square, the background palette (the flashes; the screen going
// black), Crystal's palette 7 over a trainer battle's field and its wave (SCX per line).
//
// KAI's own transition runs on underneath, unchanged: the battle starts when it ends, on the same frame as for a
// friend without these options (co-op never waits). The field is taken as the transition starts (its first frame)
// and drawn through the Game Boy's frames over the whole screen, a Game Boy cell 12 x 8.9 pixels of ours; the
// timeline is fitted to KAI's transition's time (measured for each: sKaiTransFrames), a shorter one holding black.
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "event_data.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "mp_uistyle.h"
#include "mp_region.h"
#include "data.h"
#include "constants/trainers.h"
#include "constants/rgb.h"
#include "constants/map_types.h"
#include "constants/battle.h"
#include <string.h>

struct GbTransChange { u16 cell, value; };
struct GbTransTimeline { u16 length; const u16 *start; const struct GbTransChange *changes; const u8 *bgp;
                         const u8 *flags; const s8 (*scx)[144]; };
#include "mp_gb_trans_data.h"

#define W 240
#define H 160
#define GBW 20
#define GBH 18
#define V_BLACK 0xFFFF
#define V_SQUARE 0xFFFE

// How long KAI's own transitions take (B_TRANSITION_*: frames from the start to IsBattleTransitionDone, measured with
// MOEMON_ANIMLEN and `mp transition N`)
static const u16 sKaiTransFrames[B_TRANSITION_COUNT] = {
    122, 89, 89, 158, 150, 123, 141, 86, 88, 151, 106, 137, 221, 221, 221, 221, 221, 219, 219, 159, 159, 159, 271,
    271, 301, 223, 128, 209, 128, 158, 168, 142, 181, 158, 162, 152, 146, 146, 170, 165, 178, 178,
};

static struct {
    bool8 active;
    bool8 snapped;
    u8 timeline;
    u32 start;
    u16 fit;          // KAI's transition's frames
    u16 shown;        // the timeline frame whose map is in sMap
    u16 map[GBW * GBH];
} sT;
static u16 sSnap[W * H];
u8 gMpGbTransForce = 0xFF;    // `mp gbtrans N`: the next transition's timeline (tests)

// GB OPTIONS > TRANSITIONS: RED / CRYSTAL showing, the Game Boy's (MP_GB_KAI_TRANS unset)
static bool8 GbTransitions(void)
{
    return (MP_UiStyle_Active() == UISTYLE_RED || MP_UiStyle_Active() == UISTYLE_CRYSTAL) && !MP_Gb_On(MP_GB_KAI_TRANS);
}

// The level of the lead (the first that can battle) and the foe's: a wild one's own, a trainer's first's
static u8 LeadLevel(void)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0 && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            return GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
    return 1;
}

static u8 FoeLevel(bool8 trainer)
{
    const struct Trainer *t;
    u8 lv = 0;
    if (!trainer)
        return GetMonData(&gEnemyParty[0], MON_DATA_LEVEL);
    // (a trainer's party isn't made yet: its first POKéMON's level from the trainer's data, every region's)
    t = MP_Region_TrainerData(gTrainerBattleOpponent_A);
    if (t != NULL && t->partySize != 0)
        switch (t->partyFlags)
        {
        case 0: lv = t->party.NoItemDefaultMoves[0].lvl; break;
        case F_TRAINER_PARTY_CUSTOM_MOVESET: lv = t->party.NoItemCustomMoves[0].lvl; break;
        case F_TRAINER_PARTY_HELD_ITEM: lv = t->party.ItemDefaultMoves[0].lvl; break;
        default: lv = t->party.ItemCustomMoves[0].lvl; break;
        }
    return lv ? lv : LeadLevel();
}

// Red: BattleTransitions by its three bits (a trainer; the foe 3 or more levels above the lead; a dungeon map).
// Crystal: StartTrainerBattle_DetermineWhichAnimation (a cave; stronger), its Poke Ball over a trainer's
static u8 ChooseTimeline(void)
{
    bool8 trainer = (gBattleTypeFlags & BATTLE_TYPE_TRAINER) != 0;
    bool8 stronger = FoeLevel(trainer) >= LeadLevel() + 3;
    bool8 cave = gMapHeader.mapType == MAP_TYPE_UNDERGROUND;
    if (MP_UiStyle_Active() == UISTYLE_RED)
    {
        static const u8 sRed[8] = {GB_TRANS_RED_DOUBLE_CIRCLE, GB_TRANS_RED_SPIRAL_IN, GB_TRANS_RED_CIRCLE,
                                   GB_TRANS_RED_SPIRAL_OUT, GB_TRANS_RED_HSTRIPES, GB_TRANS_RED_SHRINK,
                                   GB_TRANS_RED_VSTRIPES, GB_TRANS_RED_SPLIT};
        return sRed[trainer | (stronger << 1) | (cave << 2)];
    }
    return (cave ? (stronger ? GB_TRANS_GSC_CAVE_STRONGER : GB_TRANS_GSC_CAVE)
                 : (stronger ? GB_TRANS_GSC_NO_CAVE_STRONGER : GB_TRANS_GSC_NO_CAVE))
         + (trainer ? GB_TRANS_GSC_CAVE_TRAINER - GB_TRANS_GSC_CAVE : 0);
}

// battle_transition.c LaunchBattleTransitionTask: a battle (or `mp transition`) starts one
void MP_GbTrans_Begin(u8 transitionId)
{
    u32 i;
    sT.active = FALSE;
    if (!GbTransitions() || gMain.inBattle)
        return;
    sT.timeline = gMpGbTransForce < GB_TRANS_COUNT ? gMpGbTransForce : ChooseTimeline();
    sT.fit = transitionId < B_TRANSITION_COUNT ? sKaiTransFrames[transitionId] : 150;
    sT.start = gMain.vblankCounter1;
    sT.snapped = FALSE;
    sT.shown = 0xFFFF;
    for (i = 0; i < GBW * GBH; i++)
        sT.map[i] = i;
    sT.active = TRUE;
    MPNet_Log("gb transition: timeline %d (%d frames) fitted to KAI's %d (%d frames)", sT.timeline,
              sGbTransTimelines[sT.timeline].length, transitionId, sT.fit);
}

// IsBattleTransitionDone: KAI's ended, the battle starts
void MP_GbTrans_End(void)
{
    sT.active = FALSE;
}

bool8 MP_GbTrans_Active(void)
{
    return sT.active;
}

// The map as it is at frame F (the changes since the frame shown applied; from the start when going back)
static void SeekMap(u16 f)
{
    const struct GbTransTimeline *tl = &sGbTransTimelines[sT.timeline];
    u16 from, k;
    u32 i;
    if (sT.shown != 0xFFFF && f == sT.shown)
        return;
    if (sT.shown == 0xFFFF || f < sT.shown)
    {
        for (i = 0; i < GBW * GBH; i++)
            sT.map[i] = i;
        from = 0;
    }
    else
        from = sT.shown + 1;
    for (k = from; k <= f; k++)
        for (i = tl->start[k]; i < tl->start[k + 1]; i++)
            sT.map[tl->changes[i].cell] = tl->changes[i].value;
    sT.shown = f;
}

// A colour's Game Boy shade (0 white .. 3 black) by its brightness
static u8 ShadeOf(u16 c)
{
    s32 l = ((c & 31) * 77 + ((c >> 5) & 31) * 151 + ((c >> 10) & 31) * 28) >> 8;
    return l >= 24 ? 0 : l >= 16 ? 1 : l >= 8 ? 2 : 3;
}

// The background palette on a colour: its shade taken to the one rBGP gives it (the flashes, the screen going black),
// the colour made that much lighter or darker
static u16 ApplyBgp(u16 c, u8 bgp)
{
    static const s32 sBright[4] = {256, 170, 85, 0};
    u8 from = ShadeOf(c), to = (bgp >> (from * 2)) & 3;
    s32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31, k;
    if (bgp == 0xFF)   // (every shade black: the darkest colours too)
        return RGB(0, 0, 0);
    if (bgp == 0x00)
        return RGB(31, 31, 31);
    if (to == from)
        return c;
    if (to == 3)
        return RGB(0, 0, 0);
    if (to > from)   // darker
    {
        k = sBright[to] * 256 / sBright[from];
        return RGB(r * k >> 8, g * k >> 8, b * k >> 8);
    }
    k = (sBright[to] - sBright[from]) * 256 / (256 - sBright[from]);   // lighter: toward white
    return RGB(r + ((31 - r) * k >> 8), g + ((31 - g) * k >> 8), b + ((31 - b) * k >> 8));
}

// sdl2.c VDraw, over the finished picture: the Game Boy's transition over KAI's
void MP_GbTrans_Composite(u16 *frame)
{
    const struct GbTransTimeline *tl;
    u32 elapsed;
    u16 f;
    u8 bgp, flags;
    s32 x, y;
    if (!sT.active)
        return;
    if (!sT.snapped)
    {
        memcpy(sSnap, frame, sizeof(sSnap));   // (KAI's first frame: the field as it was)
        sT.snapped = TRUE;
    }
    tl = &sGbTransTimelines[sT.timeline];
    elapsed = gMain.vblankCounter1 - sT.start;
    // fitted: a longer timeline spread over KAI's time, a shorter one holding its last frame (black)
    f = tl->length > sT.fit ? elapsed * tl->length / sT.fit : elapsed;
    if (f >= tl->length)
        f = tl->length - 1;
    SeekMap(f);
    bgp = tl->bgp[f];
    flags = tl->flags[f];
    for (y = 0; y < H; y++)
    {
        s32 cy = y * GBH / H, y0 = cy * H / GBH, y1 = (cy + 1) * H / GBH, shift = 0, sy;
        u16 *row = frame + y * W;
        if (flags & 2)
        {
            // Crystal's wave: the line's SCX, its Game Boy line's (half again as wide here); the waves are counted
            // from the start of the frames that have one
            u16 k, n = 0;
            for (k = 0; k < f; k++)
                n += (tl->flags[k] & 2) != 0;
            shift = tl->scx[n][y * 144 / H] * 3 / 2;
        }
        for (x = 0; x < W; x++)
        {
            s32 xs = ((x + shift) % W + W) % W;
            s32 cx = xs * GBW / W, x0 = cx * W / GBW;
            u16 v = sT.map[cy * GBW + cx], c;
            if (v == V_BLACK)
                c = (flags & 1) && bgp != 0xFF ? sGbTransPal7[(bgp >> 6) & 3] : RGB(0, 0, 0);   // (palette 7's darkest)
            else if (v == V_SQUARE)
            {
                // Crystal's square tile in its palette 7, scaled to the cell
                u8 idx = sGbTransSquare[(y - y0) * 8 / (y1 - y0)][(xs - x0) * 8 / (W / GBW)];
                c = bgp == 0xFF ? RGB(0, 0, 0) : sGbTransPal7[(bgp >> (idx * 2)) & 3];
                row[x] = c;
                continue;
            }
            else
            {
                s32 scx = v % GBW, scy = v / GBW;
                s32 sx = scx * W / GBW + (xs - x0);
                sy = scy * H / GBH + (y - y0) * ((scy + 1) * H / GBH - scy * H / GBH) / (y1 - y0);
                c = sSnap[sy * W + sx];
                if (flags & 1)   // Crystal's trainer battle: the whole field in palette 7 (all black at the end)
                {
                    row[x] = bgp == 0xFF ? RGB(0, 0, 0) : sGbTransPal7[(bgp >> (ShadeOf(c) * 2)) & 3];
                    continue;
                }
            }
            row[x] = ApplyBgp(c, bgp);
        }
    }
}
