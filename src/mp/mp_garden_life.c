// Moemon Co-op: the MOéMON GARDEN's life (docs/POKEMON_GARDEN.md 11.3-11.5).
//
// In the Garden the whole party lives freely (followers included: mp_follower.c hides them here). Each
// Pokémon is a Garden actor: a sprite of its own (the follower walking sprites), anchored to the player's
// sprite the way followers are, standing on a tile of its own that people can't walk through
// (field_player_avatar.c asks MP_Garden_ActorAt).
//   - Needs drive what they do: napping, foraging in the tall grass, soaking at the pond, basking in the
//     flowers, digging in the sand, perching in the grove, the storm stone for those who love storms.
//   - Social batteries: some come and sit with their Trainer, others take a quiet corner.
//   - Bonds form groups on their own: friends play (and play tag), rivals square up, close ones nap together.
//   - Decorations from the player's Secret Base collection are used their own way (mp_garden_decor.c).
//   - Friends' Pokémon come along when friends visit (their own games run them; synced positions).
// A in front of one: the same tap / hold-A wheel as with the follower (mp_life_interact.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "mp_life.h"
#include "mp_life_emote.h"
#include "mp_life_field.h"
#include "mp_life_social.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "constants/songs.h"
#include "constants/weather.h"
#include "mp_garden_map.h"

#define MAX_ACTORS     24
#define TAG_GARDEN_PAL 0x1160     // + actor index
#define ACTOR_MAGIC    0x4741     // "GA" in sprite data[6]
#define GX(x) ((x) + MAP_OFFSET)
#define GY(y) ((y) + MAP_OFFSET)

enum { FDIR_DOWN, FDIR_UP, FDIR_LEFT, FDIR_RIGHT };

// What a Garden Pokémon does (LIFE_ACT_* where they match, so friends' games can show them)
enum
{
    GA_NONE = LIFE_ACT_NONE,
    GA_WANDER = LIFE_ACT_WANDER,
    GA_NAP = LIFE_ACT_NAP,
    GA_FORAGE = LIFE_ACT_FORAGE,
    GA_PLAY = LIFE_ACT_PLAY,
    GA_COMPANY = LIFE_ACT_COMPANY,
    GA_QUIET = LIFE_ACT_QUIET,
    GA_TYPE = LIFE_ACT_TYPE,
    GA_LOOK = LIFE_ACT_LOOK,
    GA_CHASE = LIFE_ACT_COUNT,   // tag
    GA_SQUABBLE,
    GA_DECOR,
};

struct Actor
{
    bool8 used;
    s8 owner;               // -1: ours (a party slot); else the friend's player id
    u8 slot;                // ours: party slot; a friend's: their index
    u32 key;
    u16 dex;
    bool8 female;
    s16 hue;
    u8 spriteId;
    u8 size;
    struct SpriteFrameImage frames[8];
    struct SpriteTemplate tmpl;
    struct Coords16 prev, cur, target;
    u8 moved;               // pixels into the step prev -> cur (16 = there)
    u8 speed;
    u8 dir, pose, anim;
    u16 animTimer;
    u16 stuck;
    // its own mind (ours only)
    u8 act, phase;
    u16 timer, duration, cooldown, reactTimer;
    s16 fx, fy;             // the tile it faces while doing it (0x7FFF none)
    u8 emote;
    s8 partner;             // another actor (play, tag, a squabble), -1 none
    u8 tneed;
    s8 decor;               // the decoration it's using (-1 none)
    u8 arrivePose;          // the pose it takes once there
};

static struct
{
    struct Actor a[MAX_ACTORS];
    bool8 active;           // set up for this visit
    u32 frame;
    // where the player is, in 1/256 tiles (as mp_follower.c does for its owner)
    struct Coords16 pLast;
    s16 pSprX, pSprY;
    u16 pMoved;
    s32 ovx, ovy;
} sL;

bool8 MP_Garden_StoneShown(void);
void MP_GardenDecor_Update(void);
bool8 MP_GardenDecor_At(s16 x, s16 y);
s8 MP_GardenDecor_Near(s16 x, s16 y, u8 radius, u8 kind, s16 *dx, s16 *dy);
void MP_GardenDecor_Used(s8 decor, struct MpLifeRec *rec);
bool8 MP_GardenDecor_IsFavorite(s8 index, struct MpLifeRec *rec);
void MP_GardenWatch_Update(void);
bool8 MP_GardenWatch_Active(void);
void MP_GardenNet_Update(void);

bool8 MP_Garden_HidesFollowers(void)
{
    return MP_Garden_IsHere();
}

static void SpriteCB_Actor(struct Sprite *sprite)
{
}

static bool8 SpriteIsOurs(struct Actor *a)
{
    return a->spriteId < MAX_SPRITES && gSprites[a->spriteId].inUse && gSprites[a->spriteId].callback == SpriteCB_Actor
        && gSprites[a->spriteId].data[6] == ACTOR_MAGIC && gSprites[a->spriteId].data[7] == (s16)(a - sL.a);
}

static void DestroyActorSprite(struct Actor *a)
{
    if (SpriteIsOurs(a))
    {
        DestroySprite(&gSprites[a->spriteId]);
        FreeSpritePaletteByTag(TAG_GARDEN_PAL + (a - sL.a));
    }
    a->spriteId = MAX_SPRITES;
}

static void FreeActor(struct Actor *a)
{
    DestroyActorSprite(a);
    MP_LifeEmote_Place(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), FALSE, 0, 0, 0);
    a->used = FALSE;
}

// ---------------------------------------------------------------------------
// The ground

static u8 Behavior(s16 x, s16 y) { return MapGridGetMetatileBehaviorAt(x, y); }
static bool8 IsWater(s16 x, s16 y) { return MetatileBehavior_IsSurfableWaterOrUnderwater(Behavior(x, y)); }

static bool8 InGarden(s16 x, s16 y)
{
    s16 mx = x - MAP_OFFSET, my = y - MAP_OFFSET;
    if (my < GARDEN_Y0 || my > GARDEN_Y1)
        return FALSE;
    if (mx >= GARDEN_EAST_X0 && mx <= GARDEN_EAST_X1)
        return TRUE;
    // the extra meadow (and the hedge's gap) only when it's open: the ground says so
    return mx >= GARDEN_WEST_X0 && mx <= GARDEN_HEDGE_X + 1;
}

static struct Actor *ActorAtTile(s16 x, s16 y, struct Actor *except)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *a = &sL.a[i];
        if (!a->used || a == except)
            continue;
        if ((a->cur.x == x && a->cur.y == y) || (a->moved < 16 && a->prev.x == x && a->prev.y == y))
            return a;
    }
    return NULL;
}

bool8 MP_Garden_ActorAt(s16 x, s16 y)
{
    return sL.active && MP_Garden_IsHere() && (ActorAtTile(x, y, NULL) != NULL || MP_GardenDecor_At(x, y));
}

static bool8 CanStand(s16 x, s16 y, struct Actor *self)
{
    if (!InGarden(x, y) || MapGridGetCollisionAt(x, y) != 0 || IsWater(x, y))
        return FALSE;
    if (GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT || MP_GardenDecor_At(x, y))
        return FALSE;
    return ActorAtTile(x, y, self) == NULL;
}

static u8 WallsAround(s16 x, s16 y)
{
    return (MapGridGetCollisionAt(x + 1, y) != 0) + (MapGridGetCollisionAt(x - 1, y) != 0)
         + (MapGridGetCollisionAt(x, y + 1) != 0) + (MapGridGetCollisionAt(x, y - 1) != 0);
}

typedef bool8 (*GroundTest)(s16 x, s16 y, s16 *fx, s16 *fy);

static bool8 T_Any(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return TRUE; }
static bool8 T_Tall(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return MetatileBehavior_IsTallGrass(Behavior(x, y)); }
static bool8 T_Sand(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return MetatileBehavior_IsSandOrDeepSand(Behavior(x, y)); }
static bool8 T_Puddle(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return MetatileBehavior_IsPuddle(Behavior(x, y)); }
static bool8 T_Flower(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return MapGridGetMetatileIdAt(x, y) == (GARDEN_FLOWER & 0x3FF); }
static bool8 T_Cozy(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return WallsAround(x, y) >= 1 && !IsWater(x + 1, y) && !IsWater(x - 1, y); }
static bool8 T_Open(s16 x, s16 y, s16 *fx, s16 *fy) { *fx = 0x7FFF; return WallsAround(x, y) == 0; }

static bool8 T_ByWater(s16 x, s16 y, s16 *fx, s16 *fy)
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

// Beside one of the grove's trees (FORTREE's mark)
static bool8 T_ByTree(s16 x, s16 y, s16 *fx, s16 *fy)
{
    s16 tx = GX(GROWTH_GROVE_X), ty = GY(GROWTH_GROVE_Y + 1);
    if (y < ty - 1 || y > ty + 1 || x < tx - 1 || x > tx + GROWTH_GROVE_W)
        return FALSE;
    if (MapGridGetCollisionAt(x, y - 1) == 0 && MapGridGetCollisionAt(x - 1, y) == 0 && MapGridGetCollisionAt(x + 1, y) == 0)
        return FALSE;
    *fx = x;
    *fy = y - 1;
    return TRUE;
}

static bool8 T_ByStone(s16 x, s16 y, s16 *fx, s16 *fy)
{
    s16 sx = GX(GROWTH_STONE_X), sy = GY(GROWTH_STONE_Y);
    if (!MP_Garden_StoneShown() || abs(x - sx) + abs(y - sy) != 1)
        return FALSE;
    *fx = sx;
    *fy = sy;
    return TRUE;
}

static struct Actor *sSearcher;

// The nearest standable tile around (x, y) that passes the test (a little random among equals)
static bool8 FindNear(s16 cx, s16 cy, GroundTest test, u8 radius, s16 *ox, s16 *oy, s16 *fx, s16 *fy)
{
    s16 r, dx;
    u8 start = Random() & 3, k;
    for (r = 0; r <= radius; r++)
        for (k = 0; k < 4; k++)
            for (dx = -r; dx <= r; dx++)
            {
                s16 dy = r - abs(dx), x, y;
                switch ((start + k) & 3)
                {
                case 0: x = cx + dx; y = cy + dy; break;
                case 1: x = cx + dx; y = cy - dy; break;
                case 2: x = cx - dx; y = cy + dy; break;
                default: x = cx - dx; y = cy - dy; break;
                }
                if (CanStand(x, y, sSearcher) && test(x, y, fx, fy))
                {
                    *ox = x;
                    *oy = y;
                    return TRUE;
                }
            }
    return FALSE;
}

static bool8 IsNight(void)
{
    RtcCalcLocalTime();
    return gLocalTime.hours < 6 || gLocalTime.hours >= 19;
}

static bool8 Sunny(void)
{
    u8 w = GetCurrentWeather();
    return !IsNight() && (w == WEATHER_NONE || w == WEATHER_SUNNY_CLOUDS || w == WEATHER_SUNNY || w == WEATHER_DROUGHT);
}

// ---------------------------------------------------------------------------
// Bringing them out

static struct MpLifeRec *RecOf(struct Actor *a)
{
    return a->owner < 0 ? MP_Life_FindKey(a->key) : NULL;
}

static bool8 SpawnNear(struct Actor *a, s16 x, s16 y)
{
    s16 ox, oy, fx, fy;
    sSearcher = a;
    if (!FindNear(x, y, T_Any, 7, &ox, &oy, &fx, &fy))
        return FALSE;
    a->prev.x = a->cur.x = a->target.x = ox;
    a->prev.y = a->cur.y = a->target.y = oy;
    a->moved = 16;
    a->dir = FDIR_DOWN;
    return TRUE;
}

static struct Actor *NewActor(void)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
        if (!sL.a[i].used)
        {
            memset(&sL.a[i], 0, sizeof(sL.a[i]));
            sL.a[i].used = TRUE;
            sL.a[i].spriteId = MAX_SPRITES;
            sL.a[i].partner = -1;
            sL.a[i].decor = -1;
            sL.a[i].speed = 1;
            sL.a[i].anim = 0xFF;
            return &sL.a[i];
        }
    return NULL;
}

// Ours: the party (follower included), each Pokémon once, eggs not
static void SyncLocalActors(void)
{
    u8 p, i;
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *a = &sL.a[i];
        bool8 still = FALSE;
        if (!a->used || a->owner >= 0)
            continue;
        if (a->slot < PARTY_SIZE && GetMonData(&gPlayerParty[a->slot], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[a->slot], MON_DATA_IS_EGG) && MP_Life_BoxKey(&gPlayerParty[a->slot].box) == a->key)
            still = TRUE;
        if (!still)
            FreeActor(a);
    }
    for (p = 0; p < PARTY_SIZE; p++)
    {
        struct Pokemon *mon = &gPlayerParty[p];
        u32 key;
        bool8 have = FALSE;
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        struct Actor *a;
        if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG) || !MP_Follower_HasGraphics(mon))
            continue;
        key = MP_Life_BoxKey(&mon->box);
        for (i = 0; i < MAX_ACTORS; i++)
            if (sL.a[i].used && sL.a[i].owner < 0 && sL.a[i].key == key)
            {
                have = TRUE;
                sL.a[i].slot = p;
                sL.a[i].dex = SpeciesToNationalPokedexNum(species);   // (an evolution here)
            }
        if (have || (a = NewActor()) == NULL)
            continue;
        a->owner = -1;
        a->slot = p;
        a->key = key;
        a->dex = SpeciesToNationalPokedexNum(species);
        a->female = GetMonGender(mon) == MON_FEMALE;
        a->hue = MP_Hue_OfBoxMon(&mon->box);
        MP_Life_Get(&mon->box, TRUE);
        if (!SpawnNear(a, me->currentCoords.x, me->currentCoords.y))
        {
            a->used = FALSE;
            continue;
        }
        a->cooldown = 20 + Random() % 60;
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), LIFE_EMOTE_NOTE);
        MPNet_Log("garden: %u out (slot %u)", a->dex, p);
    }
}

// ---------------------------------------------------------------------------
// Moving and drawing

static u8 DirToward(struct Coords16 from, s16 x, s16 y, u8 fallback)
{
    s16 dx = x - from.x, dy = y - from.y;
    if (dx == 0 && dy == 0)
        return fallback;
    if (abs(dx) >= abs(dy))
        return dx > 0 ? FDIR_RIGHT : FDIR_LEFT;
    return dy > 0 ? FDIR_DOWN : FDIR_UP;
}

static void Step(struct Actor *a)
{
    s16 dx, dy;
    struct Coords16 n;
    if (a->moved < 16)
    {
        a->moved += a->speed;
        if (a->moved >= 16)
        {
            a->moved = 16;
            a->prev = a->cur;
        }
        return;
    }
    if (a->cur.x == a->target.x && a->cur.y == a->target.y)
        return;
    dx = a->target.x - a->cur.x;
    dy = a->target.y - a->cur.y;
    n = a->cur;
    // the longer way first; around an obstacle by the other
    if (abs(dx) >= abs(dy) && dx)
        n.x += dx > 0 ? 1 : -1;
    else
        n.y += dy > 0 ? 1 : -1;
    if (!CanStand(n.x, n.y, a))
    {
        n = a->cur;
        if (abs(dx) >= abs(dy) && dy)
            n.y += dy > 0 ? 1 : -1;
        else if (dx)
            n.x += dx > 0 ? 1 : -1;
        if (!CanStand(n.x, n.y, a))
        {
            if (++a->stuck > 40)
            {
                a->target = a->cur;   // give up: it does it here
                a->stuck = 0;
            }
            return;
        }
    }
    a->stuck = 0;
    a->dir = DirToward(a->cur, n.x, n.y, a->dir);
    a->prev = a->cur;
    a->cur = n;
    a->moved = 0;
}

static void UpdateAnchor(void)
{
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Sprite *os = &gSprites[owner->spriteId];
    s16 ownerStep, total, ddx, ddy, d;
    s32 frac;
    if (owner->currentCoords.x != sL.pLast.x || owner->currentCoords.y != sL.pLast.y)
    {
        sL.pLast = owner->currentCoords;
        sL.pMoved = 0;
    }
    ddx = os->x - sL.pSprX;
    ddy = os->y - sL.pSprY;
    d = abs(ddx) + abs(ddy);
    if (d <= 8)
        sL.pMoved += d;
    sL.pSprX = os->x;
    sL.pSprY = os->y;
    ownerStep = abs(owner->currentCoords.x - owner->previousCoords.x) + abs(owner->currentCoords.y - owner->previousCoords.y);
    total = 16 * (ownerStep ? ownerStep : 1);
    if (sL.pMoved > total)
        sL.pMoved = total;
    frac = sL.pMoved * 256 / total;
    sL.ovx = owner->previousCoords.x * 256 + (owner->currentCoords.x - owner->previousCoords.x) * frac;
    sL.ovy = owner->previousCoords.y * 256 + (owner->currentCoords.y - owner->previousCoords.y) * frac;
}

static u8 PoseFrame(struct Actor *a, struct Sprite *sp)
{
    u16 t = a->animTimer;
    bool8 moving = a->moved < 16 && (a->prev.x != a->cur.x || a->prev.y != a->cur.y);
    sp->x2 = 0;
    sp->y2 = 0;
    if (moving)
        return a->dir * 2 + ((t / 8) & 1);
    switch (a->pose)
    {
    case LIFE_POSE_NAP:
        sp->y2 = 1;
        return FDIR_DOWN * 2;
    case LIFE_POSE_SIT:
        return a->dir * 2;
    case LIFE_POSE_HOP:
    {
        u8 k = t % 20;
        sp->y2 = -(k < 10 ? k : 20 - k) / 2;
        return a->dir * 2 + ((t / 10) & 1);
    }
    case LIFE_POSE_SHAKE:
        sp->x2 = ((t / 3) & 1) ? 1 : -1;
        return a->dir * 2 + ((t / 6) & 1);
    case LIFE_POSE_SPIN:
    {
        static const u8 spin[4] = {FDIR_DOWN, FDIR_LEFT, FDIR_UP, FDIR_RIGHT};
        return spin[(t / 16) & 3] * 2;
    }
    case LIFE_POSE_LOOKUP:
        return FDIR_UP * 2 + ((t / 30) & 1);
    }
    return a->dir * 2 + ((t / 24) & 1);
}

static void Draw(struct Actor *a)
{
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Sprite *os = &gSprites[owner->spriteId], *sp;
    s32 ax, ay;
    u8 frame, idx = a - sL.a;
    if (!SpriteIsOurs(a))
    {
        a->spriteId = MP_Follower_CreateMonSprite(a->dex, a->female, a->hue, TAG_GARDEN_PAL + idx, a->frames, &a->tmpl, &a->size, SpriteCB_Actor);
        if (a->spriteId >= MAX_SPRITES)
        {
            a->spriteId = MAX_SPRITES;
            return;
        }
        gSprites[a->spriteId].data[6] = ACTOR_MAGIC;
        gSprites[a->spriteId].data[7] = idx;
        a->anim = 0xFF;
    }
    sp = &gSprites[a->spriteId];
    {
        u8 pal = IndexOfSpritePaletteTag(TAG_GARDEN_PAL + idx);
        if (pal == 0xFF)
        {
            DestroyActorSprite(a);
            return;
        }
        sp->oam.paletteNum = pal;
    }
    ax = a->prev.x * 256 + (a->cur.x - a->prev.x) * a->moved * 16;
    ay = a->prev.y * 256 + (a->cur.y - a->prev.y) * a->moved * 16;
    sp->x = os->x + (s16)((ax - sL.ovx) * 16 / 256);
    sp->y = os->y + (s16)((ay - sL.ovy) * 16 / 256) - (a->size == 64 ? 16 : 0);
    a->animTimer++;
    frame = PoseFrame(a, sp);
    if (frame != a->anim)
    {
        StartSpriteAnim(sp, frame);
        a->anim = frame;
    }
    sp->oam.priority = os->oam.priority;
    SetObjectSubpriorityByElevation(3, sp, 1);
    sp->invisible = FALSE;
    {
        s16 x = sp->x + sp->x2 + gSpriteCoordOffsetX, y = sp->y + sp->y2 + gSpriteCoordOffsetY;
        bool8 onScreen = x > -32 && x < DISPLAY_WIDTH + 32 && y > -32 && y < DISPLAY_HEIGHT + 48;
        MP_LifeEmote_Place(LIFE_EMOTE_SLOT_GARDEN(idx), onScreen, x, y - a->size / 2 + (a->size == 64 ? 16 : 4),
                           sp->subpriority > 0 ? sp->subpriority - 1 : 0);
    }
}

// ---------------------------------------------------------------------------
// Their own minds

static void Plan(struct Actor *a, u8 act, s16 x, s16 y, s16 fx, s16 fy, u16 duration, u8 pose, u8 emote)
{
    a->act = act;
    a->phase = 0;
    a->timer = 0;
    a->stuck = 0;
    a->target.x = x;
    a->target.y = y;
    a->fx = fx;
    a->fy = fy;
    a->duration = duration;
    a->arrivePose = pose;
    a->pose = LIFE_POSE_NONE;
    a->emote = emote;
    a->speed = act == GA_CHASE ? 2 : 1;
    if (act != GA_PLAY && act != GA_CHASE && act != GA_SQUABBLE)
        a->partner = -1;
    if (act != GA_DECOR)
        a->decor = -1;
}

static u8 LeadTypeNeed(struct MpLifeRec *rec, u8 *amount)
{
    u8 ta = MP_Life_TypeNeed(rec, FALSE), tb = MP_Life_TypeNeed(rec, TRUE);
    if (ta == tb || rec->need[NEED_TYPE_A] * (128 + rec->typeLean / 2) >= rec->need[NEED_TYPE_B] * (255 - rec->typeLean / 2))
    {
        *amount = rec->need[NEED_TYPE_A];
        return ta;
    }
    *amount = rec->need[NEED_TYPE_B];
    return tb;
}

static u8 NeedOfType(struct MpLifeRec *rec, u8 tneed)
{
    return MP_Life_TypeNeed(rec, FALSE) == tneed ? NEED_TYPE_A : NEED_TYPE_B;
}

static struct MpLifeBond *BondWith(struct Actor *a, struct Actor *b)
{
    return MP_Life_Bond(a->key, b->key, FALSE);
}

static bool8 Available(struct Actor *b)
{
    return b->used && b->moved >= 16 && b->reactTimer == 0
        && (b->owner >= 0 || b->act == GA_NONE || b->act == GA_WANDER || b->act == GA_LOOK);
}

// The one it'd most like to play with nearby (a warm bond), or a rival to square up to
static struct Actor *FindPartner(struct Actor *a, bool8 rival)
{
    u8 i;
    struct Actor *best = NULL;
    int bestScore = 0;
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *b = &sL.a[i];
        struct MpLifeBond *bond;
        int score;
        if (b == a || !Available(b) || abs(b->cur.x - a->cur.x) + abs(b->cur.y - a->cur.y) > 8)
            continue;
        bond = BondWith(a, b);
        if (rival)
        {
            if (bond == NULL || !((bond->flags & LIFE_BOND_RIVAL) || bond->v[BOND_FRICTION] >= 60))
                continue;
            score = bond->v[BOND_RESPECT] + bond->v[BOND_FRICTION];
        }
        else
        {
            score = 60 + (bond ? MP_Life_BondWarmth(bond) : 0) - (bond && bond->v[BOND_FRICTION] >= 80 ? 100 : 0);
        }
        if (score > bestScore)
            bestScore = score, best = b;
    }
    return best;
}

static bool8 BesideActor(struct Actor *a, struct Actor *b, s16 *x, s16 *y)
{
    s16 fx, fy;
    sSearcher = a;
    return FindNear(b->cur.x, b->cur.y, T_Any, 1, x, y, &fx, &fy) && !(*x == b->cur.x && *y == b->cur.y);
}

static void Choose(struct Actor *a, struct MpLifeRec *rec)
{
    u16 score[GA_DECOR + 1];
    u16 total = 0, pick;
    u8 i, tneed, tamount;
    s16 x, y, fx, fy;
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Actor *friendA = NULL, *rivalA = NULL;
    s16 dX = 0, dY = 0;
    s8 decor;
    memset(score, 0, sizeof(score));
    sSearcher = a;
    tneed = LeadTypeNeed(rec, &tamount);
    score[GA_WANDER] = 15 + rec->cur[LIFE_LIVELY] / 6 + rec->need[NEED_EXPLORE] / 6;
    score[GA_LOOK] = 10 + rec->cur[LIFE_CURIOUS] / 10;
    if (rec->need[NEED_REST] >= 110)
        score[GA_NAP] = rec->need[NEED_REST] / 2 + (IsNight() ? 30 : 0);
    if (rec->need[NEED_HUNGER] >= 110)
        score[GA_FORAGE] = rec->need[NEED_HUNGER] / 2;
    if (tneed != TNEED_NONE && tamount >= 90)
        score[GA_TYPE] = tamount / 2;
    if ((rec->need[NEED_PLAY] >= 90 || rec->cur[LIFE_LIVELY] >= 150) && (friendA = FindPartner(a, FALSE)) != NULL)
    {
        score[GA_PLAY] = rec->need[NEED_PLAY] / 3 + rec->cur[LIFE_LIVELY] / 5 + rec->cur[LIFE_SOCIAL] / 6;
        if (rec->cur[LIFE_LIVELY] >= 140 && friendA->owner < 0)
            score[GA_CHASE] = rec->cur[LIFE_LIVELY] / 5;
    }
    if ((rivalA = FindPartner(a, TRUE)) != NULL)
        score[GA_SQUABBLE] = 15 + rec->cur[LIFE_PROUD] / 8;
    if (rec->cur[LIFE_SOCIAL] >= 120 || rec->cur[LIFE_WARM] >= 170 || rec->need[NEED_COMPANY] >= 150)
        score[GA_COMPANY] = rec->need[NEED_COMPANY] / 3 + rec->cur[LIFE_WARM] / 8;
    if (rec->cur[LIFE_SOCIAL] < 110 && rec->need[NEED_QUIET] >= 90)
        score[GA_QUIET] = rec->need[NEED_QUIET] / 3;
    if ((decor = MP_GardenDecor_Near(a->cur.x, a->cur.y, 8, 0, &dX, &dY)) >= 0)
        score[GA_DECOR] = 20 + rec->cur[LIFE_CURIOUS] / 8 + (MP_GardenDecor_IsFavorite(decor, rec) ? 40 : 0);
    for (i = 0; i <= GA_DECOR; i++)
        total += score[i];
    if (total == 0)
        return;
    pick = Random() % total;
    for (i = 0; i <= GA_DECOR; i++)
    {
        if (pick < score[i])
            break;
        pick -= score[i];
    }
    switch (i)
    {
    case GA_WANDER:
        if (FindNear(a->cur.x + (Random() % 7) - 3, a->cur.y + (Random() % 5) - 2, T_Any, 3, &x, &y, &fx, &fy))
            Plan(a, GA_WANDER, x, y, 0x7FFF, 0, 30 + Random() % 60, LIFE_POSE_NONE, LIFE_EMOTE_NONE);
        break;
    case GA_LOOK:
        Plan(a, GA_LOOK, a->cur.x, a->cur.y, 0x7FFF, 0, 90 + Random() % 90, LIFE_POSE_SPIN,
             rec->cur[LIFE_CURIOUS] >= 170 ? LIFE_EMOTE_QUESTION : LIFE_EMOTE_NONE);
        break;
    case GA_NAP:
    {
        // a cushion first; else beside a friend who's napping, a cozy spot by the trees, or anywhere
        s8 cushion = MP_GardenDecor_Near(a->cur.x, a->cur.y, 10, 1, &dX, &dY);
        if (cushion >= 0 && FindNear(dX, dY, T_Any, 1, &x, &y, &fx, &fy))
        {
            a->decor = cushion;
            Plan(a, GA_NAP, x, y, dX, dY, 60 * 25, LIFE_POSE_NAP, LIFE_EMOTE_ZZZ);
            a->decor = cushion;
        }
        else if (FindNear(a->cur.x, a->cur.y, T_Cozy, 6, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_Any, 2, &x, &y, &fx, &fy))
        {
            Plan(a, GA_NAP, x, y, 0x7FFF, 0, 60 * 25, LIFE_POSE_NAP, LIFE_EMOTE_ZZZ);
        }
        break;
    }
    case GA_FORAGE:
        if (FindNear(a->cur.x, a->cur.y, T_Tall, 12, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_Flower, 6, &x, &y, &fx, &fy))
            Plan(a, GA_FORAGE, x, y, 0x7FFF, 0, 200 + Random() % 100, LIFE_POSE_SHAKE, LIFE_EMOTE_FOOD);
        break;
    case GA_TYPE:
    {
        bool8 ok = FALSE;
        u8 pose = LIFE_POSE_SIT, emote = LIFE_EMOTE_CALM;
        switch (tneed)
        {
        case TNEED_SOAK:    ok = FindNear(a->cur.x, a->cur.y, T_ByWater, 12, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_WATER; break;
        case TNEED_BASK:
        case TNEED_SUN:     ok = Sunny() && (FindNear(a->cur.x, a->cur.y, T_Flower, 10, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_Open, 6, &x, &y, &fx, &fy)); emote = LIFE_EMOTE_SUN; break;
        case TNEED_DIG:     ok = FindNear(a->cur.x, a->cur.y, T_Sand, 14, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_DOTS; break;
        case TNEED_FOLIAGE: ok = FindNear(a->cur.x, a->cur.y, T_Tall, 14, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_LEAF; break;
        case TNEED_HEIGHT:
        case TNEED_SKY:     ok = FindNear(a->cur.x, a->cur.y, T_ByTree, 14, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_Open, 4, &x, &y, &fx, &fy); pose = LIFE_POSE_LOOKUP; emote = LIFE_EMOTE_SPARKLE; break;
        case TNEED_STORM:   ok = FindNear(a->cur.x, a->cur.y, T_ByStone, 14, &x, &y, &fx, &fy); pose = LIFE_POSE_HOP; emote = LIFE_EMOTE_EXCLAIM; break;
        case TNEED_DAMP:    ok = FindNear(a->cur.x, a->cur.y, T_Puddle, 14, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_ByWater, 12, &x, &y, &fx, &fy); break;
        case TNEED_SHADE:
        case TNEED_STILL:
        case TNEED_COZY:
        case TNEED_STONE:   ok = FindNear(a->cur.x, a->cur.y, T_Cozy, 8, &x, &y, &fx, &fy); break;
        case TNEED_COOL:    ok = FindNear(a->cur.x, a->cur.y, T_ByWater, 12, &x, &y, &fx, &fy); break;
        case TNEED_NIGHT:   ok = IsNight() && FindNear(a->cur.x, a->cur.y, T_Open, 6, &x, &y, &fx, &fy); pose = LIFE_POSE_HOP; emote = LIFE_EMOTE_NOTE; break;
        case TNEED_SPAR:    ok = FindNear(a->cur.x, a->cur.y, T_Open, 6, &x, &y, &fx, &fy); pose = LIFE_POSE_SHAKE; emote = LIFE_EMOTE_SPARKLE; break;
        case TNEED_SHINE:   ok = FindNear(a->cur.x, a->cur.y, T_Flower, 10, &x, &y, &fx, &fy); emote = LIFE_EMOTE_SPARKLE; break;
        }
        if (ok)
        {
            a->tneed = tneed;
            Plan(a, GA_TYPE, x, y, fx, fy, 240 + Random() % 180, pose, emote);
        }
        break;
    }
    case GA_PLAY:
    case GA_CHASE:
        if (friendA && BesideActor(a, friendA, &x, &y))
        {
            a->partner = friendA - sL.a;
            Plan(a, i, x, y, friendA->cur.x, friendA->cur.y, i == GA_CHASE ? 60 * 6 : 60 * 7, LIFE_POSE_HOP,
                 i == GA_CHASE ? LIFE_EMOTE_NOTE : LIFE_EMOTE_BALL);
            a->partner = friendA - sL.a;
            if (friendA->owner < 0)
            {
                // it's invited: it joins (tag: it runs; play: it plays too)
                friendA->partner = a - sL.a;
                if (i == GA_CHASE && FindNear(friendA->cur.x + (friendA->cur.x - a->cur.x) * 3, friendA->cur.y + (friendA->cur.y - a->cur.y) * 2, T_Any, 3, &x, &y, &fx, &fy))
                    Plan(friendA, GA_CHASE, x, y, 0x7FFF, 0, 60 * 6, LIFE_POSE_HOP, LIFE_EMOTE_NOTE);
                else
                    Plan(friendA, GA_PLAY, friendA->cur.x, friendA->cur.y, a->cur.x, a->cur.y, 60 * 7, LIFE_POSE_HOP, LIFE_EMOTE_NOTE);
                friendA->partner = a - sL.a;
            }
            MPNet_Log("garden: %u %s with %u", a->dex, i == GA_CHASE ? "plays tag" : "plays", friendA->dex);
        }
        break;
    case GA_SQUABBLE:
        if (rivalA && BesideActor(a, rivalA, &x, &y))
        {
            a->partner = rivalA - sL.a;
            Plan(a, GA_SQUABBLE, x, y, rivalA->cur.x, rivalA->cur.y, 60 * 3, LIFE_POSE_SHAKE, LIFE_EMOTE_ANGRY);
            a->partner = rivalA - sL.a;
            if (rivalA->owner < 0)
            {
                Plan(rivalA, GA_SQUABBLE, rivalA->cur.x, rivalA->cur.y, a->cur.x, a->cur.y, 60 * 3, LIFE_POSE_SHAKE,
                     LIFE_EMOTE_ANGRY);
                rivalA->partner = a - sL.a;
            }
            MPNet_Log("garden: %u squares up to %u", a->dex, rivalA->dex);
        }
        break;
    case GA_COMPANY:
        if (FindNear(me->currentCoords.x, me->currentCoords.y, T_Any, 1, &x, &y, &fx, &fy) && !(x == me->currentCoords.x && y == me->currentCoords.y))
            Plan(a, GA_COMPANY, x, y, me->currentCoords.x, me->currentCoords.y, 60 * 6, LIFE_POSE_SIT,
                 rec->cur[LIFE_WARM] >= 130 ? LIFE_EMOTE_HEART : LIFE_EMOTE_CALM);
        break;
    case GA_QUIET:
    {
        // the corner farthest from everyone else nearby
        s16 bx = a->cur.x, by = a->cur.y, k;
        int best = -1;
        for (k = 0; k < 12; k++)
        {
            s16 tx = GX(GARDEN_EAST_X0 + Random() % (GARDEN_EAST_X1 - GARDEN_EAST_X0 + 1));
            s16 ty = GY(GARDEN_Y0 + Random() % (GARDEN_Y1 - GARDEN_Y0 + 1));
            int dmin = 99;
            u8 j;
            if (!CanStand(tx, ty, a))
                continue;
            for (j = 0; j < MAX_ACTORS; j++)
                if (sL.a[j].used && &sL.a[j] != a)
                {
                    int d = abs(sL.a[j].cur.x - tx) + abs(sL.a[j].cur.y - ty);
                    if (d < dmin)
                        dmin = d;
                }
            if (dmin > best)
                best = dmin, bx = tx, by = ty;
        }
        Plan(a, GA_QUIET, bx, by, 0x7FFF, 0, 60 * 10, LIFE_POSE_SIT, LIFE_EMOTE_CALM);
        break;
    }
    case GA_DECOR:
        if (decor >= 0 && FindNear(dX, dY, T_Any, 1, &x, &y, &fx, &fy))
        {
            Plan(a, GA_DECOR, x, y, dX, dY, 60 * 6, LIFE_POSE_HOP, LIFE_EMOTE_BALL);
            a->decor = decor;
        }
        break;
    }
}

static void Satisfy(struct Actor *a, struct MpLifeRec *rec)
{
    switch (a->act)
    {
    case GA_FORAGE:   MP_Life_Satisfy(rec, NEED_HUNGER, 18); break;
    case GA_NAP:      MP_Life_Satisfy(rec, NEED_REST, 10); MP_Life_Feel(rec, EMO_CALM, 2); break;
    case GA_PLAY:
    case GA_CHASE:
    case GA_DECOR:    MP_Life_Satisfy(rec, NEED_PLAY, 8); MP_Life_Satisfy(rec, NEED_COMPANY, 6); MP_Life_Feel(rec, EMO_JOY, 2); break;
    case GA_COMPANY:  MP_Life_Satisfy(rec, NEED_COMPANY, 14); MP_Life_Feel(rec, EMO_AFFECTION, 1); break;
    case GA_QUIET:    MP_Life_Satisfy(rec, NEED_QUIET, 14); MP_Life_Feel(rec, EMO_CALM, 2); break;
    case GA_TYPE:     MP_Life_Satisfy(rec, NeedOfType(rec, a->tneed), 16); MP_Life_Feel(rec, EMO_JOY, 1); break;
    case GA_SQUABBLE: MP_Life_Feel(rec, EMO_FRUSTRATION, 2); MP_Life_Feel(rec, EMO_PRIDE, 1); break;
    default:          MP_Life_Satisfy(rec, NEED_EXPLORE, 5); break;
    }
}

// Something shared: bonds grow from shared life (docs 5.3); a friend made is a memory (not about the Trainer:
// no Friendship)
static void Shared(struct Actor *a, struct MpLifeRec *rec)
{
    struct Actor *b = a->partner >= 0 ? &sL.a[a->partner] : NULL;
    struct MpLifeBond *bond;
    u8 aff0;
    if (b == NULL || !b->used)
        return;
    bond = MP_Life_Bond(a->key, b->key, TRUE);
    aff0 = bond ? bond->v[BOND_AFFECTION] : 0;
    if (a->act == GA_SQUABBLE)
    {
        bool8 competitive = rec->cur[LIFE_PROUD] >= 170 || rec->cur[LIFE_BOLD] >= 180;
        MP_Life_BondShare(a->key, b->key, 2, 0, 0, 2, competitive ? 2 : 0);
        return;
    }
    MP_Life_BondShare(a->key, b->key, 3, 2, rec->cur[LIFE_WARM] >= 140 ? 3 : 2, a->act == GA_CHASE ? 1 : 0, 0);
    bond = MP_Life_Bond(a->key, b->key, FALSE);
    if (bond && aff0 < 150 && bond->v[BOND_AFFECTION] >= 150 && bond->v[BOND_TRUST] >= 120)
        MP_Life_Remember(rec, MEM_FRIEND, 160, TIER_PART, b->key, 0);
    else if (bond && bond->shared % 6 == 1)
        MP_Life_Remember(rec, MEM_PLAYED, 110, TIER_PART, b->key, 1);   // (played with a friend: not a moment)
}

static void Run(struct Actor *a, struct MpLifeRec *rec)
{
    if (a->reactTimer)
    {
        if (--a->reactTimer == 0)
        {
            a->pose = LIFE_POSE_NONE;
            a->cooldown = 30;
        }
        return;
    }
    if (a->act == GA_NONE)
    {
        if (a->cooldown)
            a->cooldown--;
        else
            Choose(a, rec);
        return;
    }
    a->timer++;
    if (a->phase == 0)
    {
        // a partner moving off (tag): follow it
        if (a->act == GA_CHASE && a->partner >= 0 && sL.a[a->partner].used && (a->timer % 20) == 0)
        {
            struct Actor *b = &sL.a[a->partner];
            if (b->act == GA_CHASE && b->target.x != a->target.x)
            {
                s16 x, y;
                if (BesideActor(a, b, &x, &y))
                    a->target.x = x, a->target.y = y;
            }
        }
        if ((a->moved >= 16 && a->cur.x == a->target.x && a->cur.y == a->target.y) || a->timer > 420)
        {
            a->phase = 1;
            a->timer = 0;
            a->target = a->cur;
            if (a->fx != 0x7FFF)
                a->dir = DirToward(a->cur, a->fx, a->fy, a->dir);
            if (a->act == GA_PLAY && a->partner >= 0 && sL.a[a->partner].used)
                a->dir = DirToward(a->cur, sL.a[a->partner].cur.x, sL.a[a->partner].cur.y, a->dir);
            a->pose = a->arrivePose;
            if (a->emote != LIFE_EMOTE_NONE)
                MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), a->emote);
            if (a->act == GA_DECOR || a->act == GA_NAP)
                MP_GardenDecor_Used(a->decor, rec);
        }
        return;
    }
    if ((a->timer % 30) == 0)
        Satisfy(a, rec);
    if (a->emote != LIFE_EMOTE_NONE && a->timer && (a->timer % 180) == 0)
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), a->emote);
    if ((a->act == GA_PLAY || a->act == GA_SQUABBLE) && a->partner >= 0 && sL.a[a->partner].used && (a->timer % 30) == 0)
        a->dir = DirToward(a->cur, sL.a[a->partner].cur.x, sL.a[a->partner].cur.y, a->dir);
    if (a->timer >= a->duration)
    {
        if (a->act == GA_PLAY || a->act == GA_CHASE || a->act == GA_SQUABBLE)
            Shared(a, rec);
        if (a->act == GA_FORAGE)
            MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), LIFE_EMOTE_NOTE);
        a->act = GA_NONE;
        a->pose = LIFE_POSE_NONE;
        a->partner = -1;
        a->decor = -1;
        a->cooldown = 40 + Random() % 120;
    }
}

// ---------------------------------------------------------------------------
// Answers to interactions (mp_life_interact.c) and what friends' games can see

void MP_Garden_ActorReaction(s8 actor, u8 emote, u8 pose, u16 frames)
{
    struct Actor *a;
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (actor < 0 || actor >= MAX_ACTORS || !sL.a[actor].used)
        return;
    a = &sL.a[actor];
    if (emote != LIFE_EMOTE_NONE)
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(actor), emote);
    a->act = GA_NONE;
    a->target = a->cur;
    a->dir = DirToward(a->cur, me->currentCoords.x, me->currentCoords.y, a->dir);
    a->pose = pose;
    a->reactTimer = frames ? frames : 40;
}

u8 MP_Garden_ActorActivity(s8 actor)
{
    if (actor < 0 || actor >= MAX_ACTORS || !sL.a[actor].used)
        return LIFE_ACT_NONE;
    if (sL.a[actor].act == GA_CHASE || sL.a[actor].act == GA_DECOR)
        return LIFE_ACT_PLAY;
    if (sL.a[actor].act == GA_SQUABBLE)
        return LIFE_ACT_NONE;
    return sL.a[actor].phase == 1 ? sL.a[actor].act : LIFE_ACT_NONE;
}

bool8 MP_Garden_TryInteract(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    struct Actor *a;
    if (!sL.active || !MP_Garden_IsHere() || !JOY_NEW(A_BUTTON) || !MP_IsLocalFree() || MP_GardenWatch_Active())
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    a = ActorAtTile(x, y, NULL);
    if (a == NULL)
        return FALSE;
    if (a->owner >= 0)
        return MP_LifeSocial_TryFriendPokemon(a->owner, a->key, a->dex);   // a friend's: their game answers
    a->act = GA_NONE;
    a->target = a->cur;
    a->reactTimer = 600;   // stays for the talk (the answer sets its own time)
    a->dir = DirToward(a->cur, me->currentCoords.x, me->currentCoords.y, a->dir);
    MPNet_Log("garden: interact with %u", a->dex);
    return MP_LifeInteract_StartFor(&gPlayerParty[a->slot], a - sL.a);
}

// For WATCH mode (mp_garden_watch.c) and the network (mp_garden_net.c)
u8 MP_GardenLife_Count(void)
{
    return MAX_ACTORS;
}

bool8 MP_GardenLife_Info(u8 i, s16 *x, s16 *y, u8 *act, u8 *phase, s8 *partner, u16 *dex, s8 *owner, u32 *key)
{
    struct Actor *a;
    if (i >= MAX_ACTORS || !sL.a[i].used)
        return FALSE;
    a = &sL.a[i];
    *x = a->cur.x;
    *y = a->cur.y;
    *act = a->act;
    *phase = a->phase;
    *partner = a->partner;
    *dex = a->dex;
    *owner = a->owner;
    *key = a->key;
    return TRUE;
}

u8 MP_GardenLife_Pose(u8 i)
{
    return i < MAX_ACTORS && sL.a[i].used ? sL.a[i].pose : 0;
}

u8 MP_GardenLife_Dir(u8 i)
{
    return i < MAX_ACTORS && sL.a[i].used ? sL.a[i].dir : 0;
}

// A friend's Pokémon, as their game says (mp_garden_net.c): it walks to its tile here
s8 MP_GardenLife_SetRemote(s8 owner, u8 index, u32 key, u16 dex, bool8 female, s16 hue, s16 x, s16 y, u8 dir, u8 pose, u8 act)
{
    u8 i;
    struct Actor *a = NULL;
    for (i = 0; i < MAX_ACTORS; i++)
        if (sL.a[i].used && sL.a[i].owner == owner && sL.a[i].slot == index)
            a = &sL.a[i];
    if (a && (a->key != key || a->dex != dex))
    {
        FreeActor(a);
        a = NULL;
    }
    if (a == NULL)
    {
        if (dex == 0 || !MP_Follower_DexHasGraphics(dex) || (a = NewActor()) == NULL)
            return -1;
        a->owner = owner;
        a->slot = index;
        a->key = key;
        a->dex = dex;
        a->female = female;
        a->hue = hue;
        a->prev.x = a->cur.x = a->target.x = x;
        a->prev.y = a->cur.y = a->target.y = y;
        a->moved = 16;
        MPNet_Log("garden: player %d's %u here", owner + 1, dex);
    }
    if (abs(x - a->cur.x) + abs(y - a->cur.y) > 4)
    {
        a->prev.x = a->cur.x = x;
        a->prev.y = a->cur.y = y;
        a->moved = 16;
    }
    a->target.x = x;
    a->target.y = y;
    a->speed = abs(x - a->cur.x) + abs(y - a->cur.y) > 1 ? 2 : 1;
    if (a->moved >= 16 && a->cur.x == x && a->cur.y == y)
    {
        a->dir = dir;
        a->pose = pose;
    }
    else
    {
        a->pose = LIFE_POSE_NONE;
    }
    a->act = act;
    a->timer = 0;   // (seen: kept)
    return a - sL.a;
}

// Ours, for the network: party slot order (index), its look and where it is
bool8 MP_GardenLife_Local(u8 n, u32 *key, u16 *dex, u8 *female, s16 *hue, s16 *x, s16 *y, u8 *dir, u8 *pose, u8 *act, u8 *actor)
{
    u8 i, k = 0;
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *a = &sL.a[i];
        if (!a->used || a->owner >= 0)
            continue;
        if (k++ != n)
            continue;
        *key = a->key;
        *dex = a->dex;
        *female = a->female;
        *hue = a->hue;
        *x = a->target.x;
        *y = a->target.y;
        *dir = a->dir;
        *pose = a->pose;
        *act = a->act;
        *actor = i;
        return TRUE;
    }
    return FALSE;
}

// A friend's Pokémon answering (mp_life_social.c, by its key): ours, here
void MP_GardenLife_ReactKey(u32 key, u8 emote, u8 pose, u16 frames)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
        if (sL.a[i].used && sL.a[i].owner < 0 && sL.a[i].key == key)
            MP_Garden_ActorReaction(i, emote, pose, frames);
}

void MP_GardenLife_DropMissing(s8 owner, u8 present)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
        if (sL.a[i].used && sL.a[i].owner == owner && !(present & (1 << sL.a[i].slot)))
            FreeActor(&sL.a[i]);
}

void MP_GardenLife_DropRemote(s8 owner)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
        if (sL.a[i].used && sL.a[i].owner == owner && (owner >= 0))
            FreeActor(&sL.a[i]);
}

bool8 MP_GardenLife_ActorScreenPos(u8 i, s16 *x, s16 *y)
{
    struct Actor *a;
    if (i >= MAX_ACTORS || !sL.a[i].used || !SpriteIsOurs(&sL.a[i]))
        return FALSE;
    a = &sL.a[i];
    *x = gSprites[a->spriteId].x + gSpriteCoordOffsetX;
    *y = gSprites[a->spriteId].y + gSpriteCoordOffsetY;
    return TRUE;
}

// ---------------------------------------------------------------------------

void MP_GardenLife_OnMapLoad(void)
{
    u8 i;
    // a fresh visit: everyone comes out again around the player (friends' arrive with their games' word)
    for (i = 0; i < MAX_ACTORS; i++)
        sL.a[i].used = FALSE;
    sL.active = FALSE;
}

void MP_GardenLife_Update(void)
{
    u8 i;
    if (!MP_IsInOverworld())
        return;
    if (!MP_Garden_IsHere())
    {
        if (sL.active)
        {
            for (i = 0; i < MAX_ACTORS; i++)
                if (sL.a[i].used)
                    FreeActor(&sL.a[i]);
            sL.active = FALSE;
        }
        return;
    }
    if (gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT || gObjectEvents[gPlayerAvatar.objectEventId].spriteId >= MAX_SPRITES)
        return;
    sL.frame++;
    if (!sL.active)
    {
        struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
        sL.active = TRUE;
        sL.pLast = me->currentCoords;
        sL.pSprX = gSprites[me->spriteId].x;
        sL.pSprY = gSprites[me->spriteId].y;
        sL.pMoved = 16;
        SyncLocalActors();
    }
    else if ((sL.frame % 60) == 0)
    {
        SyncLocalActors();
    }
    MP_GardenDecor_Update();
    UpdateAnchor();
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *a = &sL.a[i];
        if (!a->used)
            continue;
        if (a->owner < 0)
        {
            struct MpLifeRec *rec = RecOf(a);
            if (rec)
                Run(a, rec);   // (ours: their own minds; friends' Pokémon move as their games say)
        }
        Step(a);
        Draw(a);
    }
    MP_GardenWatch_Update();
    MP_GardenNet_Update();
}

void MP_GardenLife_Debug(const char *args)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *a = &sL.a[i];
        if (!a->used)
            continue;
        printf("[test] garden actor %u: dex %u owner %d at %d,%d act %u phase %u pose %u partner %d\n", i, a->dex, a->owner,
               a->cur.x - MAP_OFFSET, a->cur.y - MAP_OFFSET, a->act, a->phase, a->pose, a->partner);
    }
}

// Tests: talk to actor N (as if A were pressed in front of it)
void MP_GardenLife_DebugTalk(u8 actor)
{
    if (actor < MAX_ACTORS && sL.a[actor].used && sL.a[actor].owner >= 0)
        MP_LifeSocial_TryFriendPokemon(sL.a[actor].owner, sL.a[actor].key, sL.a[actor].dex);
    else if (actor < MAX_ACTORS && sL.a[actor].used && sL.a[actor].owner < 0)
    {
        sL.a[actor].act = GA_NONE;
        sL.a[actor].reactTimer = 600;
        MP_LifeInteract_StartFor(&gPlayerParty[sL.a[actor].slot], actor);
    }
}

// Tests: an actor's next activity / a partner
void MP_GardenLife_DebugForce(u8 actor, u8 act)
{
    struct MpLifeRec *rec;
    if (actor >= MAX_ACTORS || !sL.a[actor].used || (rec = RecOf(&sL.a[actor])) == NULL)
        return;
    sL.a[actor].act = GA_NONE;
    sL.a[actor].cooldown = 0;
    sL.a[actor].reactTimer = 0;
    {
        // raise what that activity answers to, then choose
        switch (act)
        {
        case GA_NAP: rec->need[NEED_REST] = 250; break;
        case GA_FORAGE: rec->need[NEED_HUNGER] = 250; break;
        case GA_PLAY: case GA_CHASE: rec->need[NEED_PLAY] = 250; rec->cur[LIFE_LIVELY] = 200; break;
        case GA_COMPANY: rec->need[NEED_COMPANY] = 250; rec->cur[LIFE_SOCIAL] = 220; break;
        case GA_QUIET: rec->need[NEED_QUIET] = 250; rec->cur[LIFE_SOCIAL] = 40; break;
        }
    }
    Choose(&sL.a[actor], rec);
}

// Where a map tile is on screen, for sprites anchored like the actors (decorations: mp_garden_decor.c)
bool8 MP_GardenLife_TileToSprite(s16 tx, s16 ty, s16 *x, s16 *y)
{
    struct ObjectEvent *owner;
    if (!sL.active || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return FALSE;
    owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (owner->spriteId >= MAX_SPRITES)
        return FALSE;
    *x = gSprites[owner->spriteId].x + (s16)(((s32)tx * 256 - sL.ovx) * 16 / 256);
    *y = gSprites[owner->spriteId].y + (s16)(((s32)ty * 256 - sL.ovy) * 16 / 256);
    return TRUE;
}
