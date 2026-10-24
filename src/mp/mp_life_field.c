// Moemon Co-op: MOéMON GARDEN, the follower's own life on the map (docs/POKEMON_GARDEN.md 3, 7, 8).
//
// While its Trainer stands still (or strolls along), the follower lives a little: it looks around,
// wanders a few steps, forages in the grass when it's hungry, naps when it's sleepy, soaks by the water,
// basks in the sun, plays and invites its Trainer to join, stays close or takes a quiet moment apart.
// What it does comes from its needs, feelings and temperament and from the place itself (tiles, weather,
// time of day: works in any region). It never blocks anyone (mp_follower.c AutoCanStand) and hurries
// back when its Trainer moves on. Needs only build up as preferences: a cue stays small, never distress.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_life.h"
#include "mp_life_emote.h"
#include "mp_life_field.h"
#include "mp_life_social.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "constants/abilities.h"
#include "constants/event_bg.h"
#include "constants/flags.h"
#include "constants/map_types.h"
#include "constants/species.h"
#include "constants/weather.h"

#define IDLE_BEFORE_ACTIVITY 90   // frames its Trainer stands still before it does something of its own

struct Brain
{
    u8 act;             // LIFE_ACT_*
    u8 phase;           // 0 going there, 1 doing it
    u16 timer;          // frames in this phase
    u16 duration;       // frames to do it for
    u8 tneed;           // TNEED_* for LIFE_ACT_TYPE
    u8 emote;           // the emote while doing it
    u8 pose;
    s16 tx, ty;         // target tile
    s16 fx, fy;         // the tile it faces while doing it (or 0x7FFF)
    bool8 inviting;
    u16 idle;           // frames its Trainer has stood still
    s16 ownerX, ownerY;
    u8 ownerDir;
    u16 cooldown;       // frames before it starts something again
    u16 cueTimer;       // frames to the next need cue / feeling
    u16 envTimer;       // frames to the next look at the surroundings
    u16 reactTimer;     // a reaction (an answer to an interaction) holds the pose this long
    u16 lastMap;        // map it last saw (group << 8 | num)
    u16 arriveTimer;    // frames since arriving on this map
    u16 hintedItem;     // the hidden item flag it last sniffed at (so it doesn't keep doing it)
    u16 frightCooldown;
    u32 personality;    // the follower this is about
    u8 mingleId;        // the friend whose follower (or who) it went to meet
    bool8 mingleGreet;  // ... the friend themself
};
static struct Brain sB;

// ---------------------------------------------------------------------------
// The follower and its surroundings

static struct BoxPokemon *FollowerBox(void)
{
    u8 i;
    u32 p = gMpConfig.followerPersonality;
    if (p == 0)
        return NULL;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct BoxPokemon *box = &gPlayerParty[i].box;
        if (GetBoxMonData(box, MON_DATA_SPECIES) != SPECIES_NONE && !GetBoxMonData(box, MON_DATA_IS_EGG)
         && GetBoxMonData(box, MON_DATA_PERSONALITY) == p && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            return box;
    }
    return NULL;
}

struct MpLifeRec *MP_LifeField_FollowerRec(void)
{
    return MP_Life_Get(FollowerBox(), TRUE);
}

static bool8 IsNight(void)
{
    RtcCalcLocalTime();
    return gLocalTime.hours < 6 || gLocalTime.hours >= 19;
}

static bool8 Outdoors(void)
{
    u8 t = gMapHeader.mapType;
    return t == MAP_TYPE_TOWN || t == MAP_TYPE_CITY || t == MAP_TYPE_ROUTE || t == MAP_TYPE_OCEAN_ROUTE;
}

static bool8 Underground(void)
{
    return gMapHeader.mapType == MAP_TYPE_UNDERGROUND;
}

static bool8 Indoors(void)
{
    return gMapHeader.mapType == MAP_TYPE_INDOOR || gMapHeader.mapType == MAP_TYPE_SECRET_BASE;
}

static bool8 Sunny(void)
{
    u8 w = GetCurrentWeather();
    return Outdoors() && !IsNight() && (w == WEATHER_NONE || w == WEATHER_SUNNY_CLOUDS || w == WEATHER_SUNNY || w == WEATHER_DROUGHT);
}

static bool8 Rainy(void)
{
    u8 w = GetCurrentWeather();
    return Outdoors() && (w == WEATHER_RAIN || w == WEATHER_RAIN_THUNDERSTORM || w == WEATHER_DOWNPOUR);
}

static bool8 Stormy(void)
{
    u8 w = GetCurrentWeather();
    return Outdoors() && (w == WEATHER_RAIN_THUNDERSTORM || w == WEATHER_DOWNPOUR);
}

static u8 Behavior(s16 x, s16 y)
{
    return MapGridGetMetatileBehaviorAt(x, y);
}

static bool8 IsWater(s16 x, s16 y)
{
    return MetatileBehavior_IsSurfableWaterOrUnderwater(Behavior(x, y)) && !MetatileBehavior_IsWaterfall(Behavior(x, y));
}

static bool8 IsGrass(s16 x, s16 y)
{
    u8 b = Behavior(x, y);
    return MetatileBehavior_IsTallGrass(b) || MetatileBehavior_IsLongGrass(b) || MetatileBehavior_IsAshGrass(b);
}

static bool8 IsSand(s16 x, s16 y)
{
    return MetatileBehavior_IsSandOrDeepSand(Behavior(x, y)) && !MetatileBehavior_IsDeepSand(Behavior(x, y));
}

static bool8 IsPuddle(s16 x, s16 y)
{
    u8 b = Behavior(x, y);
    return MetatileBehavior_IsPuddle(b) || MetatileBehavior_IsShallowFlowingWater(b);
}

static u8 WallsAround(s16 x, s16 y)
{
    u8 n = 0;
    n += MapGridGetCollisionAt(x + 1, y) != 0;
    n += MapGridGetCollisionAt(x - 1, y) != 0;
    n += MapGridGetCollisionAt(x, y + 1) != 0;
    n += MapGridGetCollisionAt(x, y - 1) != 0;
    return n;
}

// A tile within reach that passes the test (nearest first, a little random among equals)
typedef bool8 (*TileTest)(s16 x, s16 y, s16 *faceX, s16 *faceY);

static bool8 FindTile(TileTest test, u8 radius, s16 *outX, s16 *outY, s16 *faceX, s16 *faceY)
{
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 ox = owner->currentCoords.x, oy = owner->currentCoords.y, r, dx, dy;
    u8 start = Random() & 3;
    for (r = 1; r <= radius; r++)
    {
        u8 k;
        for (k = 0; k < 4; k++)
        {
            u8 side = (start + k) & 3;
            for (dx = -r; dx <= r; dx++)
            {
                s16 x, y;
                dy = r - abs(dx);
                switch (side)
                {
                case 0: x = ox + dx; y = oy + dy; break;
                case 1: x = ox + dx; y = oy - dy; break;
                case 2: x = ox - dx; y = oy + dy; break;
                default: x = ox - dx; y = oy - dy; break;
                }
                if (MP_Follower_CanStandAt(x, y) && test(x, y, faceX, faceY))
                {
                    *outX = x;
                    *outY = y;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

static bool8 Test_Any(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return TRUE; }
static bool8 Test_Grass(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return IsGrass(x, y); }
static bool8 Test_Sand(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return IsSand(x, y); }
static bool8 Test_Puddle(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return IsPuddle(x, y); }
static bool8 Test_Wall(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return WallsAround(x, y) >= 2; }
static bool8 Test_Open(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return WallsAround(x, y) == 0; }

static bool8 Test_ByWater(s16 x, s16 y, s16 *fx, s16 *fy)
{
    static const s8 d[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    u8 i;
    for (i = 0; i < 4; i++)
        if (IsWater(x + d[i][0], y + d[i][1]))
        {
            *fx = x + d[i][0];
            *fy = y + d[i][1];
            return TRUE;
        }
    return FALSE;
}

static bool8 IsPlayerObject(u8 obj)
{
    u8 i;
    if (obj == gPlayerAvatar.objectEventId)
        return TRUE;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (gMpPlayers[i].present && MP_GetRemoteObjectEventId(i) == obj)
            return TRUE;
    return FALSE;
}

// Beside someone or something to look at (an NPC, an item ball)
static bool8 Test_ByObject(s16 x, s16 y, s16 *fx, s16 *fy)
{
    static const s8 d[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    u8 i;
    for (i = 0; i < 4; i++)
    {
        u8 obj = GetObjectEventIdByXY(x + d[i][0], y + d[i][1]);
        if (obj != OBJECT_EVENTS_COUNT && !IsPlayerObject(obj))
        {
            *fx = x + d[i][0];
            *fy = y + d[i][1];
            return TRUE;
        }
    }
    return FALSE;
}

// Beside its Trainer
static bool8 Test_ByOwner(s16 x, s16 y, s16 *fx, s16 *fy)
{
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (abs(x - owner->currentCoords.x) + abs(y - owner->currentCoords.y) != 1)
        return FALSE;
    *fx = owner->currentCoords.x;
    *fy = owner->currentCoords.y;
    return TRUE;
}

// A few tiles away from its Trainer (a quiet moment)
static bool8 Test_Apart(s16 x, s16 y, s16 *fx, s16 *fy)
{
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 d = abs(x - owner->currentCoords.x) + abs(y - owner->currentCoords.y);
    if (d < 3)
        return FALSE;
    // faces away from its Trainer
    *fx = x + (x - owner->currentCoords.x > 0 ? 1 : x - owner->currentCoords.x < 0 ? -1 : 0);
    *fy = y + (y - owner->currentCoords.y > 0 ? 1 : y - owner->currentCoords.y < 0 ? -1 : 0);
    return TRUE;
}

// A hidden item near its Trainer it hasn't sniffed at yet (rare, gentle hints: docs 7)
static bool8 FindHiddenItem(s16 *x, s16 *y, u16 *flag)
{
    const struct MapEvents *ev = gMapHeader.events;
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 i;
    if (ev == NULL)
        return FALSE;
    for (i = 0; i < ev->bgEventCount; i++)
    {
        const struct BgEvent *bg = &ev->bgEvents[i];
        s16 hx, hy;
        u16 f;
        if (bg->kind != BG_EVENT_HIDDEN_ITEM)
            continue;
        f = bg->bgUnion.hiddenItem.hiddenItemId + FLAG_HIDDEN_ITEMS_START;
        if (FlagGet(f) || f == sB.hintedItem)
            continue;
        hx = bg->x + MAP_OFFSET;
        hy = bg->y + MAP_OFFSET;
        if (abs(hx - owner->currentCoords.x) + abs(hy - owner->currentCoords.y) > 4)
            continue;
        *x = hx;
        *y = hy;
        *flag = f;
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Choosing what to do

static u8 LeadTypeNeed(struct MpLifeRec *rec, u8 *amount)
{
    u8 a = MP_Life_TypeNeed(rec, FALSE), b = MP_Life_TypeNeed(rec, TRUE);
    u16 va = rec->need[NEED_TYPE_A] * (128 + rec->typeLean / 2) / 255;
    u16 vb = rec->need[NEED_TYPE_B] * (255 - rec->typeLean / 2) / 255;
    if (a == b || va >= vb)
    {
        *amount = rec->need[NEED_TYPE_A];
        return a;
    }
    *amount = rec->need[NEED_TYPE_B];
    return b;
}

static u8 NeedOfType(struct MpLifeRec *rec, u8 tneed)
{
    return MP_Life_TypeNeed(rec, FALSE) == tneed ? NEED_TYPE_A : NEED_TYPE_B;
}

static void Plan(u8 act, s16 tx, s16 ty, s16 fx, s16 fy, u16 duration, u8 pose, u8 emote)
{
    sB.act = act;
    sB.phase = 0;
    sB.timer = 0;
    sB.tx = tx;
    sB.ty = ty;
    sB.fx = fx;
    sB.fy = fy;
    sB.duration = duration;
    sB.pose = pose;
    sB.emote = emote;
    sB.inviting = FALSE;
    MP_Follower_AutoTarget(tx, ty, 1);
    MPNet_Log("life field: act %u to %d,%d", act, tx - MAP_OFFSET, ty - MAP_OFFSET);
}

// Its type need, if this place lets it: where to go, how to look doing it
static bool8 PlanTypeNeed(struct MpLifeRec *rec, u8 tneed)
{
    s16 x, y, fx = 0x7FFF, fy = 0;
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 ox = owner->currentCoords.x, oy = owner->currentCoords.y;
    bool8 ok = FALSE;
    u8 pose = LIFE_POSE_SIT, emote = LIFE_EMOTE_CALM;
    switch (tneed)
    {
    case TNEED_SOAK:    ok = FindTile(Test_ByWater, 4, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_WATER; break;
    case TNEED_BASK:
    case TNEED_SUN:     ok = Sunny() && FindTile(Test_Open, 3, &x, &y, &fx, &fy); emote = LIFE_EMOTE_SUN; break;
    case TNEED_DIG:     ok = FindTile(Test_Sand, 4, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_DOTS; break;
    case TNEED_FOLIAGE: ok = FindTile(Test_Grass, 4, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_LEAF; break;
    case TNEED_SHADE:   ok = (Indoors() || Underground() || IsNight()) ? FindTile(Test_Any, 2, &x, &y, &fx, &fy)
                                                                       : FindTile(Test_Wall, 3, &x, &y, &fx, &fy); break;
    case TNEED_SKY:     ok = Outdoors() && FindTile(Test_Open, 2, &x, &y, &fx, &fy); pose = LIFE_POSE_LOOKUP; emote = LIFE_EMOTE_NOTE; break;
    case TNEED_STORM:   ok = Rainy() && FindTile(Test_Open, 2, &x, &y, &fx, &fy); pose = LIFE_POSE_HOP; emote = LIFE_EMOTE_EXCLAIM; break;
    case TNEED_STILL:   ok = FindTile(Test_Wall, 3, &x, &y, &fx, &fy) || FindTile(Test_Any, 2, &x, &y, &fx, &fy); break;
    case TNEED_COOL:    ok = (Underground() || IsNight()) ? FindTile(Test_Any, 2, &x, &y, &fx, &fy)
                                                          : FindTile(Test_ByWater, 4, &x, &y, &fx, &fy); break;
    case TNEED_HEIGHT:  ok = Outdoors() && FindTile(Test_Open, 2, &x, &y, &fx, &fy); pose = LIFE_POSE_LOOKUP; emote = LIFE_EMOTE_SPARKLE; break;
    case TNEED_NIGHT:   ok = IsNight() && FindTile(Test_Any, 3, &x, &y, &fx, &fy); pose = LIFE_POSE_HOP; emote = LIFE_EMOTE_NOTE; break;
    case TNEED_SPAR:    ok = FindTile(Test_Open, 2, &x, &y, &fx, &fy) || FindTile(Test_Any, 2, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_SPARKLE; break;
    case TNEED_SHINE:   ok = FindTile(Test_ByObject, 4, &x, &y, &fx, &fy); emote = LIFE_EMOTE_SPARKLE; break;
    case TNEED_DAMP:    ok = FindTile(Test_Puddle, 4, &x, &y, &fx, &fy) || (Rainy() && FindTile(Test_Any, 2, &x, &y, &fx, &fy)); break;
    case TNEED_STONE:   ok = (Underground() || gMapHeader.mapType == MAP_TYPE_ROUTE) && FindTile(Test_Wall, 3, &x, &y, &fx, &fy); break;
    case TNEED_COZY:    ok = FindTile(Test_ByOwner, 1, &x, &y, &fx, &fy) || FindTile(Test_Wall, 2, &x, &y, &fx, &fy); emote = LIFE_EMOTE_CALM; break;
    }
    (void)ox; (void)oy;
    if (!ok)
        return FALSE;
    sB.tneed = tneed;
    Plan(LIFE_ACT_TYPE, x, y, fx, fy, 240 + (Random() % 180), pose, emote);
    return TRUE;
}

static u8 sForceAct = 0xFF;
static struct { s16 x, y, fx, fy; u8 emote, pose; bool8 valid; } sMingle;   // where a meeting would be (ChooseActivity)   // mp life act N (tests): this activity next, if it can

static void ChooseActivity(struct MpLifeRec *rec)
{
    u16 score[LIFE_ACT_COUNT] = {0};
    u16 total = 0, pick;
    u8 i, tneed, tamount;
    s16 x, y, fx, fy;
    u16 flag;
    bool8 haveInvestigate, haveForage, haveNap, haveCompany, haveQuiet, haveHint;
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];

    haveInvestigate = FindTile(Test_ByObject, 4, &x, &y, &fx, &fy) || FindTile(Test_Grass, 3, &x, &y, &fx, &fy)
                   || FindTile(Test_ByWater, 3, &x, &y, &fx, &fy);
    haveForage = !Indoors() && (FindTile(Test_Grass, 4, &x, &y, &fx, &fy) || (Outdoors() && FindTile(Test_Any, 2, &x, &y, &fx, &fy)));
    haveNap = TRUE;
    haveCompany = FindTile(Test_ByOwner, 1, &x, &y, &fx, &fy);
    haveQuiet = FindTile(Test_Apart, 4, &x, &y, &fx, &fy);
    haveHint = FindHiddenItem(&x, &y, &flag);
    {
        // friends pausing nearby: their followers (docs 7: when followers meet)
        u8 mFriend, mEmote, mPose;
        bool8 mGreet;
        s16 mx, my, mfx, mfy;
        sMingle.valid = FALSE;
        if (MP_LifeSocial_MingleTarget(rec, &mFriend, &mx, &my, &mfx, &mfy, &mEmote, &mPose, &mGreet))
        {
            score[LIFE_ACT_MINGLE] = 40 + rec->cur[LIFE_SOCIAL] / 4 + rec->need[NEED_COMPANY] / 4 + (mGreet ? 60 : 0);
            sB.mingleId = mFriend;
            sB.mingleGreet = mGreet;
            sMingle.x = mx; sMingle.y = my; sMingle.fx = mfx; sMingle.fy = mfy; sMingle.emote = mEmote; sMingle.pose = mPose; sMingle.valid = TRUE;
        }
    }

    score[LIFE_ACT_LOOK] = 20 + rec->cur[LIFE_CURIOUS] / 8;
    score[LIFE_ACT_WANDER] = 10 + rec->cur[LIFE_LIVELY] / 6 + rec->need[NEED_EXPLORE] / 6;
    if (haveInvestigate)
        score[LIFE_ACT_INVESTIGATE] = 15 + rec->cur[LIFE_CURIOUS] / 5;
    if (haveForage && rec->need[NEED_HUNGER] >= 140)
        score[LIFE_ACT_FORAGE] = rec->need[NEED_HUNGER] / 2;
    if (haveNap && rec->need[NEED_REST] >= 150)
        score[LIFE_ACT_NAP] = rec->need[NEED_REST] / 2;
    if (rec->need[NEED_PLAY] >= 130)
        score[LIFE_ACT_PLAY] = rec->need[NEED_PLAY] / 3 + rec->cur[LIFE_LIVELY] / 6;
    if (haveCompany && ((rec->cur[LIFE_SOCIAL] >= 128 && rec->need[NEED_COMPANY] >= 100) || rec->cur[LIFE_BOLD] < 90 || rec->cur[LIFE_WARM] > 190))
        score[LIFE_ACT_COMPANY] = rec->need[NEED_COMPANY] / 3 + (rec->cur[LIFE_BOLD] < 90 ? 30 : 0) + (rec->cur[LIFE_WARM] > 190 ? 20 : 0);
    if (haveQuiet && rec->cur[LIFE_SOCIAL] < 128 && rec->need[NEED_QUIET] >= 100)
        score[LIFE_ACT_QUIET] = rec->need[NEED_QUIET] / 3;
    tneed = LeadTypeNeed(rec, &tamount);
    if (tneed != TNEED_NONE && tamount >= 120)
        score[LIFE_ACT_TYPE] = tamount / 2;
    // the rare hint: a curious or PICKUP Pokémon sometimes notices something hidden
    if (haveHint && (rec->cur[LIFE_CURIOUS] >= 150 || GetAbilityBySpecies(rec->species, GetBoxMonData(FollowerBox(), MON_DATA_ABILITY_NUM)) == ABILITY_PICKUP) && (Random() % 8) == 0)
        score[LIFE_ACT_HINT] = 60;

    if (sForceAct < LIFE_ACT_COUNT)
    {
        u8 f = sForceAct;
        sForceAct = 0xFF;
        memset(score, 0, sizeof(score));
        score[f] = 1;
    }
    for (i = 0; i < LIFE_ACT_COUNT; i++)
        total += score[i];
    if (total == 0)
        return;
    pick = Random() % total;
    for (i = 0; i < LIFE_ACT_COUNT; i++)
    {
        if (pick < score[i])
            break;
        pick -= score[i];
    }
    switch (i)
    {
    case LIFE_ACT_LOOK:
    {
        // looks around where it stands
        s16 cx, cy;
        u8 d;
        MP_Follower_LocalPos(&cx, &cy, &d);
        Plan(LIFE_ACT_LOOK, cx, cy, 0x7FFF, 0, 100 + Random() % 120, LIFE_POSE_SPIN, rec->cur[LIFE_CURIOUS] > 160 ? LIFE_EMOTE_QUESTION : LIFE_EMOTE_NONE);
        break;
    }
    case LIFE_ACT_WANDER:
        if (FindTile(Test_Any, 2 + (rec->cur[LIFE_LIVELY] > 160), &x, &y, &fx, &fy))
            Plan(LIFE_ACT_WANDER, x, y, 0x7FFF, 0, 40 + Random() % 60, LIFE_POSE_NONE, LIFE_EMOTE_NONE);
        break;
    case LIFE_ACT_INVESTIGATE:
        if (FindTile(Test_ByObject, 4, &x, &y, &fx, &fy) || FindTile(Test_ByWater, 3, &x, &y, &fx, &fy)
         || FindTile(Test_Grass, 3, &x, &y, &fx, &fy))
            Plan(LIFE_ACT_INVESTIGATE, x, y, fx, fy, 120 + Random() % 60, LIFE_POSE_NONE, LIFE_EMOTE_QUESTION);
        break;
    case LIFE_ACT_FORAGE:
        if (FindTile(Test_Grass, 4, &x, &y, &fx, &fy) || FindTile(Test_Any, 2, &x, &y, &fx, &fy))
            Plan(LIFE_ACT_FORAGE, x, y, 0x7FFF, 0, 200 + Random() % 100, LIFE_POSE_SHAKE, LIFE_EMOTE_FOOD);
        break;
    case LIFE_ACT_NAP:
        // a cozy spot: by a wall, in the grass, or (warm ones) right by its Trainer
        if ((rec->cur[LIFE_WARM] > 150 && FindTile(Test_ByOwner, 1, &x, &y, &fx, &fy))
         || FindTile(Test_Wall, 3, &x, &y, &fx, &fy) || FindTile(Test_Grass, 3, &x, &y, &fx, &fy)
         || FindTile(Test_Any, 1, &x, &y, &fx, &fy))
            Plan(LIFE_ACT_NAP, x, y, 0x7FFF, 0, 60 * 30, LIFE_POSE_NAP, LIFE_EMOTE_ZZZ);
        break;
    case LIFE_ACT_PLAY:
        if (FindTile(Test_ByOwner, 2, &x, &y, &fx, &fy) || FindTile(Test_Any, 2, &x, &y, &fx, &fy))
        {
            Plan(LIFE_ACT_PLAY, x, y, owner->currentCoords.x, owner->currentCoords.y, 60 * 8, LIFE_POSE_HOP, LIFE_EMOTE_BALL);
        }
        break;
    case LIFE_ACT_COMPANY:
        if (FindTile(Test_ByOwner, 1, &x, &y, &fx, &fy))
            Plan(LIFE_ACT_COMPANY, x, y, fx, fy, 60 * 6, LIFE_POSE_SIT, rec->cur[LIFE_WARM] > 128 ? LIFE_EMOTE_HEART : LIFE_EMOTE_CALM);
        break;
    case LIFE_ACT_QUIET:
        if (FindTile(Test_Apart, 4, &x, &y, &fx, &fy))
            Plan(LIFE_ACT_QUIET, x, y, fx, fy, 60 * 8, LIFE_POSE_SIT, LIFE_EMOTE_CALM);
        break;
    case LIFE_ACT_TYPE:
        PlanTypeNeed(rec, tneed);
        break;
    case LIFE_ACT_MINGLE:
        if (sMingle.valid)
            Plan(LIFE_ACT_MINGLE, sMingle.x, sMingle.y, sMingle.fx, sMingle.fy, 60 * 5, sMingle.pose, sMingle.emote);
        break;
    case LIFE_ACT_HINT:
    {
        s16 hx, hy;
        if (FindHiddenItem(&hx, &hy, &flag))
        {
            sB.hintedItem = flag;
            // stands beside the spot, sniffing at it
            if (MP_Follower_CanStandAt(hx, hy + 1)) x = hx, y = hy + 1;
            else if (MP_Follower_CanStandAt(hx - 1, hy)) x = hx - 1, y = hy;
            else if (MP_Follower_CanStandAt(hx + 1, hy)) x = hx + 1, y = hy;
            else if (MP_Follower_CanStandAt(hx, hy - 1)) x = hx, y = hy - 1;
            else break;
            Plan(LIFE_ACT_HINT, x, y, hx, hy, 180, LIFE_POSE_SHAKE, LIFE_EMOTE_QUESTION);
            MPNet_Log("life field: hint at %d,%d", hx - MAP_OFFSET, hy - MAP_OFFSET);
        }
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Doing it

static void Satisfy(struct MpLifeRec *rec)
{
    // a little at a time while it's doing it (every 30 frames)
    switch (sB.act)
    {
    case LIFE_ACT_FORAGE:  MP_Life_Satisfy(rec, NEED_HUNGER, 18); break;
    case LIFE_ACT_NAP:     MP_Life_Satisfy(rec, NEED_REST, 10); MP_Life_Feel(rec, EMO_CALM, 2); break;
    case LIFE_ACT_PLAY:    MP_Life_Satisfy(rec, NEED_PLAY, 4); MP_Life_Feel(rec, EMO_JOY, 1); break;
    case LIFE_ACT_COMPANY: MP_Life_Satisfy(rec, NEED_COMPANY, 14); MP_Life_Feel(rec, EMO_AFFECTION, 1); break;
    case LIFE_ACT_MINGLE:  MP_Life_Satisfy(rec, NEED_COMPANY, 14); MP_Life_Feel(rec, EMO_JOY, 1); break;
    case LIFE_ACT_QUIET:   MP_Life_Satisfy(rec, NEED_QUIET, 14); MP_Life_Feel(rec, EMO_CALM, 2); break;
    case LIFE_ACT_TYPE:    MP_Life_Satisfy(rec, NeedOfType(rec, sB.tneed), 16); MP_Life_Feel(rec, EMO_JOY, 1); break;
    case LIFE_ACT_WANDER:
    case LIFE_ACT_INVESTIGATE:
    case LIFE_ACT_HINT:
    case LIFE_ACT_LOOK:    MP_Life_Satisfy(rec, NEED_EXPLORE, 6); break;
    }
}

static void Finish(struct MpLifeRec *rec)
{
    if (sB.act == LIFE_ACT_FORAGE)
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, LIFE_EMOTE_NOTE);   // found something good
    sB.act = LIFE_ACT_NONE;
    sB.inviting = FALSE;
    MP_Follower_AutoPose(LIFE_POSE_NONE);
    // something else in a moment, or back to its Trainer's side
    sB.cooldown = 30 + Random() % 90;
    if ((Random() % 3) == 0)
    {
        sB.act = LIFE_ACT_RETURN;
        MP_Follower_AutoReturn();
    }
}

static void RunActivity(struct MpLifeRec *rec)
{
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (sB.act == LIFE_ACT_RETURN)
    {
        if (MP_Follower_AutoFinishReturn())
        {
            sB.act = LIFE_ACT_NONE;
            sB.cooldown = 120 + Random() % 180;
        }
        return;
    }
    if (sB.act == LIFE_ACT_NONE)
        return;
    sB.timer++;
    if (sB.phase == 0)
    {
        if (MP_Follower_AutoArrived() || MP_Follower_AutoStuck() || sB.timer > 240)
        {
            sB.phase = 1;
            sB.timer = 0;
            if (sB.fx != 0x7FFF)
                MP_Follower_AutoFaceTile(sB.fx, sB.fy);
            MP_Follower_AutoPose(sB.pose);
            if (sB.emote != LIFE_EMOTE_NONE)
                MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, sB.emote);
            if (sB.act == LIFE_ACT_MINGLE)
                MP_LifeSocial_Met(sB.mingleId, sB.mingleGreet);
            if (sB.act == LIFE_ACT_PLAY)
            {
                sB.inviting = TRUE;
                MP_Follower_AutoFaceTile(owner->currentCoords.x, owner->currentCoords.y);
            }
        }
        return;
    }
    // doing it
    if ((sB.timer % 30) == 0)
        Satisfy(rec);
    // the emote comes back now and then (Zzz while napping, the ball while inviting)
    if (sB.emote != LIFE_EMOTE_NONE && (sB.timer % 150) == 0 && sB.timer)
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, sB.emote);
    if (sB.act == LIFE_ACT_PLAY && (sB.timer % 60) == 0)
        MP_Follower_AutoFaceTile(owner->currentCoords.x, owner->currentCoords.y);
    if (sB.timer >= sB.duration)
        Finish(rec);
}

// ---------------------------------------------------------------------------
// Feelings, cues and the place it's in

static u8 EmoteForEmotion(u8 e)
{
    switch (e)
    {
    case EMO_JOY:         return LIFE_EMOTE_NOTE;
    case EMO_FEAR:        return LIFE_EMOTE_SWEAT;
    case EMO_FRUSTRATION: return LIFE_EMOTE_ANGRY;
    case EMO_SADNESS:     return LIFE_EMOTE_TEAR;
    case EMO_SURPRISE:    return LIFE_EMOTE_EXCLAIM;
    case EMO_AFFECTION:   return LIFE_EMOTE_HEART;
    case EMO_PRIDE:       return LIFE_EMOTE_SPARKLE;
    }
    return LIFE_EMOTE_CALM;
}

static u8 CueForNeed(struct MpLifeRec *rec, u8 need)
{
    switch (need)
    {
    case NEED_HUNGER:  return LIFE_EMOTE_FOOD;
    case NEED_REST:    return rec->cur[LIFE_PROUD] >= 180 ? LIFE_EMOTE_NONE : LIFE_EMOTE_ZZZ;   // proud ones hide that they're tired
    case NEED_PLAY:    return LIFE_EMOTE_BALL;
    case NEED_EXPLORE: return LIFE_EMOTE_QUESTION;
    case NEED_COMPANY: return LIFE_EMOTE_HEART;
    case NEED_QUIET:   return LIFE_EMOTE_DOTS;
    }
    switch (MP_Life_TypeNeed(rec, need == NEED_TYPE_B))
    {
    case TNEED_SOAK: return LIFE_EMOTE_WATER;
    case TNEED_BASK: case TNEED_SUN: return LIFE_EMOTE_SUN;
    case TNEED_FOLIAGE: return LIFE_EMOTE_LEAF;
    }
    return LIFE_EMOTE_DOTS;
}

// Now and then: a strong feeling shows, or a pressing need gives a small cue (never more)
static void Cues(struct MpLifeRec *rec)
{
    u8 amount, e, need;
    if (sB.cueTimer)
    {
        sB.cueTimer--;
        return;
    }
    sB.cueTimer = 60 * 8 + Random() % (60 * 12);
    if (MP_LifeEmote_Current(LIFE_EMOTE_SLOT_LOCAL) != LIFE_EMOTE_NONE)
        return;
    e = MP_Life_TopEmotion(rec, &amount);
    if (e != EMO_CALM && amount >= 55)
    {
        // proud ones keep fear and sadness to themselves unless it's strong
        if ((e == EMO_FEAR || e == EMO_SADNESS) && rec->cur[LIFE_PROUD] >= 180 && amount < 80)
            return;
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, EmoteForEmotion(e));
        return;
    }
    need = MP_Life_PressingNeed(rec, &amount);
    if (amount >= 200)
    {
        u8 cue = CueForNeed(rec, need);
        // proud ones cue a third as often
        if (rec->cur[LIFE_PROUD] >= 180 && (Random() % 3) != 0)
            return;
        if (cue != LIFE_EMOTE_NONE)
            MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, cue);
        // a dramatic one flops down for a moment when it's tired and its Trainer stands still
        if (need == NEED_REST && rec->cur[LIFE_LIVELY] >= 170 && rec->cur[LIFE_PROUD] < 100 && sB.idle > 30
         && !MP_Follower_AutoActive() && MP_Follower_AutoStart())
        {
            s16 x, y;
            u8 d;
            MP_Follower_LocalPos(&x, &y, &d);
            Plan(LIFE_ACT_NAP, x, y, 0x7FFF, 0, 120, LIFE_POSE_NAP, LIFE_EMOTE_ZZZ);
        }
    }
}

static bool8 HasMemory(struct MpLifeRec *rec, u8 kind, u16 param)
{
    u8 i;
    for (i = 0; i < LIFE_MEMORIES; i++)
        if (rec->mem[i].kind == kind && rec->mem[i].param == param)
            return TRUE;
    return FALSE;
}

static bool8 OwnerBeside(void)
{
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x, y;
    u8 d;
    if (!MP_Follower_LocalPos(&x, &y, &d))
        return FALSE;
    return abs(x - owner->currentCoords.x) + abs(y - owner->currentCoords.y) <= 1;
}

// The place: weather, darkness, storms (every 2 seconds)
static void Surroundings(struct MpLifeRec *rec)
{
    u8 w = GetCurrentWeather();
    bool8 storm = Stormy();
    struct MpLifeTrait *afraid;
    if (sB.envTimer)
    {
        sB.envTimer--;
        return;
    }
    sB.envTimer = 120;
    if (sB.frightCooldown)
        sB.frightCooldown = sB.frightCooldown > 120 ? sB.frightCooldown - 120 : 0;

    // its place soothes its type need a little while it walks with its Trainer
    switch (MP_Life_TypeNeed(rec, FALSE))
    {
    case TNEED_SOAK:  if (Rainy()) MP_Life_Satisfy(rec, NEED_TYPE_A, 6); break;
    case TNEED_BASK:
    case TNEED_SUN:   if (Sunny()) MP_Life_Satisfy(rec, NEED_TYPE_A, 6); break;
    case TNEED_SHADE:
    case TNEED_NIGHT: if (IsNight() || Underground()) MP_Life_Satisfy(rec, NEED_TYPE_A, 6); break;
    case TNEED_STORM: if (storm) MP_Life_Satisfy(rec, NEED_TYPE_A, 10); break;
    case TNEED_SKY:   if (Outdoors()) MP_Life_Satisfy(rec, NEED_TYPE_A, 3); break;
    case TNEED_COOL:  if (Underground() || IsNight()) MP_Life_Satisfy(rec, NEED_TYPE_A, 4); break;
    }

    // Weather it has never seen: something to remember (snow, ash, sand, a downpour)
    if ((w == WEATHER_SNOW || w == WEATHER_VOLCANIC_ASH || w == WEATHER_SANDSTORM || w == WEATHER_DOWNPOUR
      || w == WEATHER_RAIN_THUNDERSTORM) && !HasMemory(rec, MEM_WEATHER, w))
        MP_Life_Remember(rec, MEM_WEATHER, 120 + rec->cur[LIFE_CURIOUS] / 4, TIER_PART, 0, w);

    // Thunderstorms: some love them, some fear them; a fear softens when it's weathered beside a friend
    if (storm)
    {
        afraid = MP_Life_FindTrait(rec, TRAIT_AFRAID, LIFE_TRIG_THUNDER);
        if (MP_Life_TypeNeed(rec, FALSE) == TNEED_STORM || MP_Life_TypeNeed(rec, TRUE) == TNEED_STORM || rec->cur[LIFE_BOLD] >= 190)
        {
            MP_Life_Feel(rec, EMO_JOY, 8);
        }
        else if (afraid || rec->cur[LIFE_BOLD] < 100)
        {
            MP_Life_Feel(rec, EMO_FEAR, afraid ? 14 : 8);
            if (afraid && OwnerBeside() && rec->emo[EMO_FEAR] < 70)
                MP_Life_SoftenTrait(rec, TRAIT_AFRAID, LIFE_TRIG_THUNDER, 3);   // weathering it together
            else if (!afraid && rec->emo[EMO_FEAR] >= 60 && sB.frightCooldown == 0)
            {
                MP_Life_Remember(rec, MEM_FRIGHT, 130, TIER_PART, 0, LIFE_TRIG_THUNDER);
                MP_Life_AddTrait(rec, TRAIT_AFRAID, LIFE_TRIG_THUNDER, 60);
                sB.frightCooldown = 60 * 60 * 5;
            }
        }
    }
    // Darkness (a cave that needs FLASH)
    if (Underground() && gSaveBlock1Ptr->flashLevel > 1 && rec->cur[LIFE_BOLD] < 100
     && MP_Life_TypeNeed(rec, FALSE) != TNEED_SHADE && MP_Life_TypeNeed(rec, FALSE) != TNEED_NIGHT)
    {
        afraid = MP_Life_FindTrait(rec, TRAIT_AFRAID, LIFE_TRIG_DARK);
        MP_Life_Feel(rec, EMO_FEAR, 6);
        if (afraid && OwnerBeside())
            MP_Life_SoftenTrait(rec, TRAIT_AFRAID, LIFE_TRIG_DARK, 2);
        else if (!afraid && rec->emo[EMO_FEAR] >= 60 && sB.frightCooldown == 0)
        {
            MP_Life_Remember(rec, MEM_FRIGHT, 120, TIER_PART, 0, LIFE_TRIG_DARK);
            MP_Life_AddTrait(rec, TRAIT_AFRAID, LIFE_TRIG_DARK, 50);
            sB.frightCooldown = 60 * 60 * 5;
        }
    }
}

// Arriving somewhere: a curious look around, or joy at a place it loves
static void Arrival(struct MpLifeRec *rec)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    if (map != sB.lastMap)
    {
        sB.lastMap = map;
        sB.arriveTimer = 0;
        return;
    }
    if (sB.arriveTimer < 0xFFFF)
        sB.arriveTimer++;
    if (sB.arriveTimer == 40)
    {
        struct MpLifePlace *p = MP_Life_Place(rec, MP_Life_CurrentPlace(), FALSE);
        if (p && ((p->flags & LIFE_PLACE_FAVORITE) || MP_Life_FindTrait(rec, TRAIT_LOVES_PLACE, p->map)))
        {
            MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, LIFE_EMOTE_HEART);
            MP_Life_Feel(rec, EMO_JOY, 20);
        }
        else if (p == NULL || p->level < 10)
        {
            if (rec->cur[LIFE_CURIOUS] >= 140)
                MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, LIFE_EMOTE_QUESTION);
            else if (rec->cur[LIFE_BOLD] < 80)
                MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, LIFE_EMOTE_DOTS);
        }
    }
}

// ---------------------------------------------------------------------------

u8 MP_LifeField_Activity(void)
{
    return sB.act;
}

bool8 MP_LifeField_Inviting(void)
{
    return sB.act == LIFE_ACT_PLAY && sB.phase == 1 && sB.inviting;
}

void MP_LifeField_OnHurryBack(s16 distance)
{
    // its Trainer moved on: whatever it was doing stops (a napper wakes with a start)
    if (sB.act == LIFE_ACT_NAP && sB.phase == 1)
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, LIFE_EMOTE_EXCLAIM);
    sB.act = LIFE_ACT_NONE;
    sB.inviting = FALSE;
    sB.cooldown = 60;
    sB.idle = 0;
}

void MP_LifeField_Reaction(u8 emote, u8 pose, u16 frames)
{
    if (emote != LIFE_EMOTE_NONE)
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_LOCAL, emote);
    if (pose != LIFE_POSE_NONE && (MP_Follower_AutoActive() || MP_Follower_AutoStart()))
    {
        s16 x, y;
        u8 d;
        MP_Follower_LocalPos(&x, &y, &d);
        sB.act = LIFE_ACT_NONE;
        MP_Follower_AutoTarget(x, y, 1);
        MP_Follower_AutoPose(pose);
        {
            struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
            MP_Follower_AutoFaceTile(owner->currentCoords.x, owner->currentCoords.y);
        }
        sB.reactTimer = frames;
    }
}

static void PlaceEmotes(void)
{
    u8 slot;
    for (slot = 0; slot < 1 + MP_MAX_PLAYERS; slot++)
    {
        s16 x, top;
        u8 sub;
        bool8 vis = MP_Follower_SlotScreenTop(slot, &x, &top, &sub);
        MP_LifeEmote_Place(slot, vis, x, top, sub);
    }
}

void MP_LifeField_Update(void)
{
    struct BoxPokemon *box;
    struct MpLifeRec *rec;
    struct ObjectEvent *owner;

    PlaceEmotes();
    box = FollowerBox();
    if (box == NULL || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
    {
        sB.act = LIFE_ACT_NONE;
        sB.personality = 0;
        return;
    }
    rec = MP_Life_Get(box, TRUE);
    if (rec == NULL)
        return;
    if (sB.personality != rec->personality)
    {
        memset(&sB, 0, sizeof(sB));
        sB.personality = rec->personality;
        sB.cueTimer = 60 * 6;
    }
    owner = &gObjectEvents[gPlayerAvatar.objectEventId];

    // Its Trainer standing still (nothing going on: no script, no menu, not moving)
    if (owner->currentCoords.x == sB.ownerX && owner->currentCoords.y == sB.ownerY
     && !(owner->heldMovementActive && !owner->heldMovementFinished) && MP_IsLocalFree())
    {
        if (sB.idle < 0xFFFF)
            sB.idle++;
    }
    else
    {
        sB.idle = 0;
    }
    sB.ownerX = owner->currentCoords.x;
    sB.ownerY = owner->currentCoords.y;
    sB.ownerDir = owner->facingDirection;

    Arrival(rec);
    Surroundings(rec);
    if (!MP_IsLocalFree())
        return;   // a script, a menu: it stays as it is
    Cues(rec);

    if (sB.reactTimer)
    {
        if (--sB.reactTimer == 0)
        {
            // the answer is over: it stays right there and follows its Trainer from there
            MP_Follower_AutoPose(LIFE_POSE_NONE);
            MP_Follower_AutoReturn();
            sB.act = LIFE_ACT_RETURN;
            sB.cooldown = 120;
        }
        return;
    }
    if (!MP_Follower_AutoActive() && sB.act != LIFE_ACT_NONE)
    {
        // it was put back beside its Trainer (a warp, a new map): whatever it was doing is over
        sB.act = LIFE_ACT_NONE;
        sB.inviting = FALSE;
    }
    if (MP_Follower_AutoActive())
    {
        RunActivity(rec);
        if (sB.act == LIFE_ACT_NONE)
        {
            if (sB.cooldown)
            {
                sB.cooldown--;
            }
            else if (sB.idle < IDLE_BEFORE_ACTIVITY)
            {
                sB.act = LIFE_ACT_RETURN;
                MP_Follower_AutoReturn();
            }
            else
            {
                ChooseActivity(rec);
                if (sB.act == LIFE_ACT_NONE)
                {
                    sB.act = LIFE_ACT_RETURN;
                    MP_Follower_AutoReturn();
                }
            }
        }
        return;
    }
    if (sB.cooldown)
    {
        sB.cooldown--;
        return;
    }
    // shy and warm ones are slower to wander off; lively, curious ones quicker
    if (sB.idle >= IDLE_BEFORE_ACTIVITY + (255 - rec->cur[LIFE_LIVELY]) / 4 && MP_Follower_AutoStart())
    {
        ChooseActivity(rec);
        if (sB.act == LIFE_ACT_NONE)
        {
            sB.act = LIFE_ACT_RETURN;
            MP_Follower_AutoReturn();
        }
    }
}

void MP_LifeField_DebugForce(u8 act)
{
    sForceAct = act;
}

void MP_LifeField_Debug(void)
{
    s16 x = 0, y = 0;
    u8 d = 0;
    bool8 pos = MP_Follower_LocalPos(&x, &y, &d);
    printf("[test] life field act %u phase %u idle %u cooldown %u auto %d free %d pos %d %d,%d box %d\n", sB.act, sB.phase, sB.idle,
           sB.cooldown, MP_Follower_AutoActive(), MP_IsLocalFree(), pos, x - MAP_OFFSET, y - MAP_OFFSET, FollowerBox() != NULL);
}
