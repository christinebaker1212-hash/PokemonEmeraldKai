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
    // long-term life (docs 12)
    u8 rare;                // GA_RARE: LIFE_RARE_*
    bool8 lead;             // GA_RARE / GA_SONG: it started it (and keeps the books for both)
    bool8 learner;          // GA_PURSUIT: it's the one being taught
    bool8 seen;             // GA_RARE: the Trainer saw it happen
    u8 newBody;             // times it showed it's still getting used to a new body (this visit)
    // the network: the tile it last stepped from (ours: sent with cur); a friend's: the tiles their game says it
    // stepped on, replayed in order (the same path and facings on every screen)
    struct Coords16 from;
    u8 qLen;
    struct Coords16 q[6];
    struct Coords16 qLast;
    s16 traceX, traceY;     // tests: the last tile and facing logged (MOEMON_MOVETRACE)
    u8 traceDir;
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
    u8 rareDone;            // LIFE_RARE_* bits that happened this visit (once is plenty)
    bool8 moveTrace;        // MOEMON_MOVETRACE (tests)
} sL;

bool8 MP_Garden_StoneShown(void);
void MP_GardenDecor_Update(void);
bool8 MP_GardenDecor_At(s16 x, s16 y);
s8 MP_GardenDecor_Near(s16 x, s16 y, u8 radius, u8 kind, s16 *dx, s16 *dy);
void MP_GardenDecor_Used(s8 decor, struct MpLifeRec *rec);
bool8 MP_GardenDecor_IsFavorite(s8 index, struct MpLifeRec *rec);
void MP_GardenWatch_Update(void);
bool8 MP_GardenWatch_Active(void);
bool8 MP_GardenWatch_View(s16 *x, s16 *y);
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
    if (my < GARDEN_Y0 || my > GARDEN_Y1 || mx < GARDEN_X0 || mx > GARDEN_X1)
        return FALSE;
    // never on the escalator's landing: the way in and out stays clear
    return !(mx >= GARDEN_PAD_X0 && mx <= GARDEN_PAD_X1 && my >= GARDEN_PAD_Y0 && my <= GARDEN_PAD_Y1);
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

// Below one of the trees around the GARDEN (the grove's not grown yet): a trunk to climb
static bool8 T_ByEdgeTree(s16 x, s16 y, s16 *fx, s16 *fy)
{
    if (MapGridGetCollisionAt(x, y - 1) == 0 || IsWater(x, y - 1) || MP_GardenDecor_At(x, y - 1)
     || GetObjectEventIdByXY(x, y - 1) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if ((x == GX(GARDEN_BOARD_X) && y - 1 == GY(GARDEN_BOARD_Y))
     || (x <= GX(GARDEN_PAD_X1) && y - 1 >= GY(GARDEN_PAD_Y0) && y - 1 <= GY(GARDEN_PAD_Y1)))
        return FALSE;   // (the GARDEN BOARD, the escalator's landing: not trees)
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
    a->from = a->cur;
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
            sL.a[i].traceX = -1;   // (logged once it's out)
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
                u16 dex = SpeciesToNationalPokedexNum(species);
                have = TRUE;
                sL.a[i].slot = p;
                if (sL.a[i].dex != dex)
                {
                    // it evolved here (a stone, a RARE CANDY): its new body's sprite
                    sL.a[i].dex = dex;
                    DestroyActorSprite(&sL.a[i]);
                }
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
        a->moved += a->owner >= 0 && a->qLen >= 2 ? a->speed * 2 : a->speed;   // (a friend's, behind: catching up)
        if (a->moved >= 16)
        {
            a->moved = 16;
            a->prev = a->cur;
        }
        return;
    }
    if (a->owner >= 0)
    {
        // a friend's: their game's steps in order (no searching of its own)
        while (a->qLen && a->q[0].x == a->cur.x && a->q[0].y == a->cur.y)
        {
            memmove(&a->q[0], &a->q[1], (a->qLen - 1) * sizeof(a->q[0]));
            a->qLen--;
        }
        if (a->qLen == 0)
            return;
        if (abs(a->q[0].x - a->cur.x) + abs(a->q[0].y - a->cur.y) == 1)
        {
            n = a->q[0];
            memmove(&a->q[0], &a->q[1], (a->qLen - 1) * sizeof(a->q[0]));
            a->qLen--;
            a->dir = DirToward(a->cur, n.x, n.y, a->dir);
            a->from = a->prev = a->cur;
            a->cur = n;
            a->moved = 0;
            return;
        }
        a->target = a->q[0];   // (a gap: steps lost to a lag spike) straight there, then on along its path
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
    a->from = a->prev = a->cur;
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
    if (act != GA_PLAY && act != GA_CHASE && act != GA_SQUABBLE && act != GA_RARE && act != GA_SONG
     && act != GA_REACT && act != GA_PURSUIT)
        a->partner = -1;
    if (act != GA_DECOR)
        a->decor = -1;
    a->rare = 0;
    a->lead = FALSE;
    a->learner = FALSE;
    a->seen = FALSE;
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

// ---------------------------------------------------------------------------
// Long-term life (docs 12): its own pursuit, rare moments, a family song, an evolution's new body

static bool8 HasType(struct MpLifeRec *rec, u8 type)
{
    return gSpeciesInfo[rec->species].types[0] == type || gSpeciesInfo[rec->species].types[1] == type;
}

static bool8 ClearSky(void)
{
    u8 w = GetCurrentWeather();
    return w == WEATHER_NONE || w == WEATHER_SUNNY_CLOUDS || w == WEATHER_SUNNY;
}

static bool8 Rainy(void)
{
    u8 w = GetCurrentWeather();
    return w == WEATHER_RAIN || w == WEATHER_RAIN_THUNDERSTORM || w == WEATHER_DOWNPOUR;
}

static u8 Hour(void)
{
    RtcCalcLocalTime();
    return gLocalTime.hours;
}

// The Trainer can see it: on screen around them (or around the view, while watching)
static bool8 Seen(struct Actor *a)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 vx = me->currentCoords.x, vy = me->currentCoords.y;
    MP_GardenWatch_View(&vx, &vy);
    return abs(a->cur.x - vx) <= 7 && abs(a->cur.y - vy) <= 5;
}

static u8 Level(struct Actor *a)
{
    return a->owner < 0 && a->slot < PARTY_SIZE ? GetMonData(&gPlayerParty[a->slot], MON_DATA_LEVEL) : 0;
}

// a free tile next to (x, y) for b (not (x, y) itself)
static bool8 BesideTile(struct Actor *b, s16 x, s16 y, s16 *ox, s16 *oy)
{
    static const s8 d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    u8 i, start = Random() & 3;
    for (i = 0; i < 4; i++)
    {
        s16 nx = x + d[(start + i) & 3][0], ny = y + d[(start + i) & 3][1];
        if (CanStand(nx, ny, b))
        {
            *ox = nx;
            *oy = ny;
            return TRUE;
        }
    }
    return FALSE;
}

// One of ours, free and close by, that passes a test
static struct Actor *OursNear(struct Actor *a, u8 radius, bool8 (*ok)(struct Actor *a, struct Actor *b))
{
    u8 i;
    struct Actor *best = NULL;
    int bestD = 99;
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *b = &sL.a[i];
        int d;
        if (b == a || b->owner >= 0 || !Available(b) || RecOf(b) == NULL)
            continue;
        d = abs(b->cur.x - a->cur.x) + abs(b->cur.y - a->cur.y);
        if (d <= radius && d < bestD && ok(a, b))
            bestD = d, best = b;
    }
    return best;
}

// --- Pursuits: no stats, no prizes; it visibly gets better, and the journal celebrates it

static const u8 sPursuitStep[LIFE_PURSUIT_MILESTONES] = {60, 150, 250};

// how it looks at it: [pursuit][0 a beginner, 1 getting there, 2 a natural] = pose, emote
static const u8 sPursuitLook[LIFE_PURSUIT_COUNT][3][2] = {
    [LIFE_PURSUIT_CLIMB]   = {{LIFE_POSE_SHAKE, LIFE_EMOTE_SWEAT}, {LIFE_POSE_HOP, LIFE_EMOTE_NOTE}, {LIFE_POSE_LOOKUP, LIFE_EMOTE_SPARKLE}},
    [LIFE_PURSUIT_SWIM]    = {{LIFE_POSE_SHAKE, LIFE_EMOTE_SWEAT}, {LIFE_POSE_SHAKE, LIFE_EMOTE_WATER}, {LIFE_POSE_HOP, LIFE_EMOTE_WATER}},
    [LIFE_PURSUIT_SING]    = {{LIFE_POSE_SIT, LIFE_EMOTE_DOTS}, {LIFE_POSE_SIT, LIFE_EMOTE_NOTE}, {LIFE_POSE_HOP, LIFE_EMOTE_NOTE}},
    [LIFE_PURSUIT_COLLECT] = {{LIFE_POSE_SHAKE, LIFE_EMOTE_QUESTION}, {LIFE_POSE_SHAKE, LIFE_EMOTE_SPARKLE}, {LIFE_POSE_HOP, LIFE_EMOTE_SPARKLE}},
    [LIFE_PURSUIT_DIG]     = {{LIFE_POSE_SHAKE, LIFE_EMOTE_DOTS}, {LIFE_POSE_SHAKE, LIFE_EMOTE_EXCLAIM}, {LIFE_POSE_HOP, LIFE_EMOTE_EXCLAIM}},
    [LIFE_PURSUIT_TEACH]   = {{LIFE_POSE_HOP, LIFE_EMOTE_DOTS}, {LIFE_POSE_HOP, LIFE_EMOTE_NOTE}, {LIFE_POSE_HOP, LIFE_EMOTE_SPARKLE}},
};

static bool8 IsYounger(struct Actor *a, struct Actor *b)
{
    return Level(b) + 5 <= Level(a);
}

static bool8 StartPursuit(struct Actor *a, struct MpLifeRec *rec, u8 p)
{
    s16 x, y, fx = 0x7FFF, fy = 0;
    bool8 ok = FALSE;
    u8 band, pose, emote;
    struct Actor *student = NULL;
    if (p == LIFE_PURSUIT_NONE || p >= LIFE_PURSUIT_COUNT)
        return FALSE;
    sSearcher = a;
    switch (p)
    {
    case LIFE_PURSUIT_CLIMB:   ok = FindNear(a->cur.x, a->cur.y, T_ByTree, 14, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_ByEdgeTree, 14, &x, &y, &fx, &fy); break;
    case LIFE_PURSUIT_SWIM:    ok = FindNear(a->cur.x, a->cur.y, T_ByWater, 14, &x, &y, &fx, &fy); break;
    case LIFE_PURSUIT_SING:    ok = FindNear(a->cur.x, a->cur.y, T_Flower, 10, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_Open, 6, &x, &y, &fx, &fy); break;
    case LIFE_PURSUIT_COLLECT: ok = FindNear(a->cur.x, a->cur.y, T_ByStone, 14, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_Sand, 14, &x, &y, &fx, &fy); break;
    case LIFE_PURSUIT_DIG:     ok = FindNear(a->cur.x, a->cur.y, T_Sand, 14, &x, &y, &fx, &fy); break;
    case LIFE_PURSUIT_TEACH:
        if ((student = OursNear(a, 8, IsYounger)) != NULL && BesideActor(a, student, &x, &y))
        {
            fx = student->cur.x;
            fy = student->cur.y;
            ok = TRUE;
        }
        break;
    }
    if (!ok)
        return FALSE;
    if (rec->pursuit != p)
    {
        // it takes it up (docs 12: one that fits its interests)
        rec->pursuit = p;
        rec->pursuitLevel = 0;
        MPNet_Log("garden: %u takes up pursuit %u", a->dex, p);
    }
    band = rec->pursuitLevel < sPursuitStep[0] ? 0 : rec->pursuitLevel < sPursuitStep[1] ? 1 : 2;
    pose = sPursuitLook[p][band][0];
    emote = sPursuitLook[p][band][1];
    Plan(a, GA_PURSUIT, x, y, fx, fy, 60 * (7 + band * 2), pose, emote);
    a->partner = -1;
    if (student)
    {
        a->partner = student - sL.a;
        Plan(student, GA_PURSUIT, student->cur.x, student->cur.y, x, y, 60 * (7 + band * 2), LIFE_POSE_SIT, LIFE_EMOTE_QUESTION);
        student->partner = a - sL.a;
        student->learner = TRUE;
    }
    MPNet_Log("garden: %u pursuit %u (level %u)", a->dex, p, rec->pursuitLevel);
    return TRUE;
}

// a session done: a little better at it; a milestone is a memory (TRUE: it celebrates)
static bool8 PursuitDone(struct Actor *a, struct MpLifeRec *rec)
{
    u8 before = rec->pursuitLevel, gain, m;
    bool8 milestone = FALSE;
    if (a->learner)
    {
        // the one being taught: it looks up to its teacher
        struct Actor *t = a->partner >= 0 ? &sL.a[a->partner] : NULL;
        if (t && t->used)
            MP_Life_BondShare(a->key, t->key, 2, 3, 1, 3, 0);
        MP_Life_Feel(rec, EMO_JOY, 10);
        return FALSE;
    }
    if (rec->pursuit == LIFE_PURSUIT_NONE)
        return FALSE;
    if (a->partner >= 0 && sL.a[a->partner].used)
        MP_Life_BondShare(a->key, sL.a[a->partner].key, 2, 2, 2, 0, 0);
    gain = 2 + Random() % 3 + (rec->cur[LIFE_LIVELY] >= 150 ? 1 : 0);
    rec->pursuitLevel = before + gain > 255 ? 255 : before + gain;
    MP_Life_Feel(rec, EMO_JOY, 6);
    for (m = 0; m < LIFE_PURSUIT_MILESTONES; m++)
        if (before < sPursuitStep[m] && rec->pursuitLevel >= sPursuitStep[m])
        {
            MP_Life_Remember(rec, MEM_PURSUIT, 140 + m * 40, TIER_PART, 0, (rec->pursuit << 8) | m);
            MP_Life_Feel(rec, EMO_PRIDE, 30);
            MP_Life_Reveal(rec, LIFE_FACT_PURSUIT);
            MPNet_Log("garden: %u pursuit %u milestone %u", a->dex, rec->pursuit, m);
            milestone = TRUE;
        }
    if (Seen(a))
        MP_Life_Reveal(rec, LIFE_FACT_PURSUIT);
    return milestone;
}

// --- Rare moments: only when temperament, bonds, place, weather and the time of day line up

static bool8 RareFits(u8 kind, struct Actor *a, struct MpLifeRec *ra, struct Actor *b, struct MpLifeRec *rb, bool8 force)
{
    struct MpLifeBond *ab = BondWith(a, b), *ba = BondWith(b, a);
    u8 h;
    if (ab == NULL || ba == NULL)
        return FALSE;
    h = Hour();
    switch (kind)
    {
    case LIFE_RARE_STARGAZE:
        // two best friends on a clear night, one of them a dreamer
        return (force || (IsNight() && ClearSky()))
            && ab->v[BOND_AFFECTION] >= 180 && ba->v[BOND_AFFECTION] >= 180 && ab->v[BOND_TRUST] >= 140 && ba->v[BOND_TRUST] >= 140
            && (ra->cur[LIFE_CURIOUS] >= 120 || ra->cur[LIFE_WARM] >= 160);
    case LIFE_RARE_PADDLE:
        // a warm Water type, and a nervous Fire type who trusts it, on a calm day
        return (force || (!IsNight() && !Rainy()))
            && HasType(ra, TYPE_WATER) && ra->cur[LIFE_WARM] >= 130 && HasType(rb, TYPE_FIRE)
            && (rb->cur[LIFE_BOLD] < 120 || MP_Life_FindTrait(rb, TRAIT_AFRAID, LIFE_TRIG_WATER))
            && ba->v[BOND_TRUST] >= 110 && ab->v[BOND_AFFECTION] >= 100;
    case LIFE_RARE_RIVAL_NAP:
        // old rivals, both worn out, on a lazy afternoon
        return (force || (h >= 12 && h < 17 && ra->need[NEED_REST] >= 100 && rb->need[NEED_REST] >= 100))
            && ((ab->flags | ba->flags) & LIFE_BOND_RIVAL) && ab->shared >= 8 && ab->v[BOND_RESPECT] >= 120;
    case LIFE_RARE_FAMILY:
        // family, a quiet evening, a warm one in a good mood
        return (force || (h >= 17 && h < 21 && ra->mood > 20 && rb->mood > 0))
            && (ab->flags & LIFE_BOND_FAMILY) && ra->cur[LIFE_WARM] >= 120;
    }
    return FALSE;
}

static bool8 StartRare(u8 kind, struct Actor *a, struct Actor *b)
{
    s16 x = a->cur.x, y = a->cur.y, fx = 0x7FFF, fy = 0, bx, by;
    u16 dur = 60 * 20;
    u8 ia = a - sL.a, ib = b - sL.a;
    sSearcher = a;
    switch (kind)
    {
    case LIFE_RARE_STARGAZE:
        if (!FindNear(a->cur.x, a->cur.y, T_Open, 8, &x, &y, &fx, &fy) || !BesideTile(b, x, y, &bx, &by))
            return FALSE;
        Plan(a, GA_RARE, x, y, 0x7FFF, 0, dur, LIFE_POSE_LOOKUP, LIFE_EMOTE_SPARKLE);
        Plan(b, GA_RARE, bx, by, 0x7FFF, 0, dur, LIFE_POSE_LOOKUP, LIFE_EMOTE_SPARKLE);
        break;
    case LIFE_RARE_PADDLE:
        if (!FindNear(a->cur.x, a->cur.y, T_ByWater, 14, &x, &y, &fx, &fy) || !BesideTile(b, x, y, &bx, &by))
            return FALSE;
        Plan(a, GA_RARE, x, y, fx, fy, dur, LIFE_POSE_SHAKE, LIFE_EMOTE_WATER);
        Plan(b, GA_RARE, bx, by, fx, fy, dur, LIFE_POSE_SIT, LIFE_EMOTE_SWEAT);
        break;
    case LIFE_RARE_RIVAL_NAP:
        if (!(FindNear(a->cur.x, a->cur.y, T_Cozy, 8, &x, &y, &fx, &fy) || FindNear(a->cur.x, a->cur.y, T_Any, 3, &x, &y, &fx, &fy))
         || !BesideTile(b, x, y, &bx, &by))
            return FALSE;
        Plan(a, GA_RARE, x, y, 0x7FFF, 0, dur, LIFE_POSE_NAP, LIFE_EMOTE_ZZZ);
        Plan(b, GA_RARE, bx, by, 0x7FFF, 0, dur, LIFE_POSE_NAP, LIFE_EMOTE_ZZZ);
        break;
    case LIFE_RARE_FAMILY:
        if (!BesideActor(a, b, &x, &y))
            return FALSE;
        dur = 60 * 12;
        Plan(a, GA_RARE, x, y, b->cur.x, b->cur.y, dur, LIFE_POSE_SHAKE, LIFE_EMOTE_HEART);
        Plan(b, GA_RARE, b->cur.x, b->cur.y, x, y, dur, LIFE_POSE_SIT, LIFE_EMOTE_CALM);
        break;
    default:
        return FALSE;
    }
    a->rare = b->rare = kind;
    a->lead = TRUE;
    a->partner = ib;
    b->partner = ia;
    sL.rareDone |= 1 << kind;
    MPNet_Log("garden: rare %u: %u and %u", kind, a->dex, b->dex);
    return TRUE;
}

static bool8 TryRare(struct Actor *a, struct MpLifeRec *ra, u8 forceKind)
{
    u8 i, k;
    if (!forceKind && (Random() % 12) != 0)
        return FALSE;   // even when it all lines up, not every time
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *b = &sL.a[i];
        struct MpLifeRec *rb;
        if (b == a || b->owner >= 0 || !Available(b) || (rb = RecOf(b)) == NULL)
            continue;
        if (!forceKind && abs(b->cur.x - a->cur.x) + abs(b->cur.y - a->cur.y) > 10)
            continue;
        for (k = 1; k < LIFE_RARE_COUNT; k++)
        {
            if (forceKind ? k != forceKind : (sL.rareDone & (1 << k)) != 0)
                continue;
            if (RareFits(k, a, ra, b, rb, forceKind != 0) && StartRare(k, a, b))
                return TRUE;
        }
    }
    return FALSE;
}

// it happened: both remember (the journal only keeps it if the Trainer saw it, docs 12)
static void RememberRare(struct MpLifeRec *rec, u8 kind, u8 role, u32 who, bool8 seen)
{
    struct MpLifeMemory *m = MP_Life_Remember(rec, MEM_RARE, 200, TIER_PART, who, kind | (role << 8));
    u8 fact;
    if (m == NULL)
        return;
    fact = LIFE_FACT_MEMORY(m - rec->mem);
    if (seen)
    {
        rec->rareSeen |= 1u << kind;
        MP_Life_Reveal(rec, fact);
    }
    else
    {
        rec->known[fact / 32] &= ~(1u << (fact % 32));
    }
}

static void RareDone(struct Actor *a, struct MpLifeRec *ra)
{
    struct Actor *b = a->partner >= 0 && sL.a[a->partner].used ? &sL.a[a->partner] : NULL;
    struct MpLifeRec *rb = b ? RecOf(b) : NULL;
    bool8 seen = a->seen || (b && b->seen);
    u8 kind = a->rare;
    if (b == NULL || rb == NULL)
        return;
    RememberRare(ra, kind, 0, b->key, seen);
    RememberRare(rb, kind, 1, a->key, seen);
    switch (kind)
    {
    case LIFE_RARE_STARGAZE:
        MP_Life_BondShare(a->key, b->key, 3, 3, 4, 0, 0);
        MP_Life_BondShare(b->key, a->key, 3, 3, 4, 0, 0);
        break;
    case LIFE_RARE_PADDLE:
        // the nervous one found its courage
        if (MP_Life_FindTrait(rb, TRAIT_AFRAID, LIFE_TRIG_WATER))
            MP_Life_SoftenTrait(rb, TRAIT_AFRAID, LIFE_TRIG_WATER, 40);
        MP_Life_Feel(rb, EMO_PRIDE, 25);
        MP_Life_BondShare(b->key, a->key, 2, 5, 3, 2, 0);
        MP_Life_BondShare(a->key, b->key, 2, 2, 3, 0, 0);
        break;
    case LIFE_RARE_RIVAL_NAP:
        MP_Life_BondShare(a->key, b->key, 2, 3, 2, 1, -8);
        MP_Life_BondShare(b->key, a->key, 2, 3, 2, 1, -8);
        break;
    case LIFE_RARE_FAMILY:
        MP_Life_BondShare(a->key, b->key, 2, 3, 4, 0, 0);
        MP_Life_BondShare(b->key, a->key, 2, 3, 4, 0, 0);
        break;
    }
    MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), LIFE_EMOTE_HEART);
    MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(b - sL.a), LIFE_EMOTE_HEART);
    MPNet_Log("garden: rare %u done (seen %d)", kind, seen);
}

// --- Family: a song its family sings, together

static bool8 IsFamilyOf(struct Actor *a, struct Actor *b)
{
    return MP_Life_IsFamily(a->key, b->key);
}

static void SongDone(struct Actor *a)
{
    struct Actor *b = a->partner >= 0 && sL.a[a->partner].used ? &sL.a[a->partner] : NULL;
    if (b == NULL)
        return;
    MP_Life_BondShare(a->key, b->key, 2, 2, 3, 0, 0);
    MP_Life_BondShare(b->key, a->key, 2, 2, 3, 0, 0);
    MP_Life_Feel(RecOf(a), EMO_JOY, 12);
    if (RecOf(b))
        MP_Life_Feel(RecOf(b), EMO_JOY, 12);
}

// --- Evolution: friends come to see the new body; the evolved one isn't used to it yet

static bool8 TryReact(struct Actor *a, struct MpLifeRec *rec)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *b = &sL.a[i];
        struct MpLifeRec *rb;
        u8 kind, pose, emote;
        s16 x, y;
        if (b == a || !b->used || b->owner >= 0 || (rb = RecOf(b)) == NULL || !MP_Life_NewBody(rb))
            continue;   // (anywhere in the GARDEN: it's small enough that everyone sees the new body)
        kind = MP_Life_EvolutionReaction(rec, rb);
        if (kind == LIFE_EVO_REACT_NONE)
            continue;
        pose = kind == LIFE_EVO_REACT_PROUD ? LIFE_POSE_HOP : LIFE_POSE_SIT;
        emote = kind == LIFE_EVO_REACT_PROUD ? LIFE_EMOTE_SPARKLE : kind == LIFE_EVO_REACT_AWED ? LIFE_EMOTE_EXCLAIM : LIFE_EMOTE_ANGRY;
        if (!BesideActor(a, b, &x, &y))
            x = a->cur.x, y = a->cur.y;
        Plan(a, GA_REACT, x, y, b->cur.x, b->cur.y, 60 * 3, pose, emote);
        a->partner = i;
        MPNet_Log("garden: %u reacts (%u) to %u's new body", a->dex, kind, b->dex);
        return TRUE;
    }
    return FALSE;
}

static bool8 NewBody(struct Actor *a, struct MpLifeRec *rec)
{
    s16 x, y, fx, fy;
    if (a->newBody >= 3 || !MP_Life_NewBody(rec) || (Random() % 3) != 0)
        return FALSE;
    sSearcher = a;
    a->newBody++;
    if (a->newBody == 1 && FindNear(a->cur.x, a->cur.y, T_ByWater, 12, &x, &y, &fx, &fy))
    {
        // a surprised look at its reflection
        Plan(a, GA_LOOK, x, y, fx, fy, 150, LIFE_POSE_SIT, LIFE_EMOTE_EXCLAIM);
        MPNet_Log("garden: %u looks at its reflection", a->dex);
    }
    else if (FindNear(a->cur.x + (Random() % 5) - 2, a->cur.y + (Random() % 3) - 1, T_Any, 2, &x, &y, &fx, &fy))
    {
        // a stumble: the new legs (or wings, or tail) take some getting used to
        Plan(a, GA_WANDER, x, y, 0x7FFF, 0, 50, LIFE_POSE_SHAKE, LIFE_EMOTE_SWEAT);
        MPNet_Log("garden: %u stumbles", a->dex);
    }
    else
    {
        Plan(a, GA_LOOK, a->cur.x, a->cur.y, 0x7FFF, 0, 90, LIFE_POSE_SPIN, LIFE_EMOTE_QUESTION);
    }
    return TRUE;
}

static void Choose(struct Actor *a, struct MpLifeRec *rec)
{
    u16 score[GA_COUNT];
    u16 total = 0, pick;
    u8 i, tneed, tamount, pursuit;
    s16 x, y, fx, fy;
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Actor *friendA = NULL, *rivalA = NULL, *songMate = NULL;
    s16 dX = 0, dY = 0;
    s8 decor;
    memset(score, 0, sizeof(score));
    // long-term life first (docs 12): a friend's new body, a rare moment, its own new body
    if (TryReact(a, rec) || TryRare(a, rec, 0) || NewBody(a, rec))
        return;
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
    // its pursuit (or one it might take up)
    pursuit = rec->pursuit ? rec->pursuit : MP_Life_PursuitFor(rec);
    if (pursuit != LIFE_PURSUIT_NONE)
        score[GA_PURSUIT] = rec->pursuit ? 18 + rec->cur[LIFE_LIVELY] / 12 + rec->need[NEED_PLAY] / 8 : 6 + rec->cur[LIFE_CURIOUS] / 20;
    // a song its family sings, with family close by
    if (MP_Life_FamilyTrait(rec, TRAIT_FAMILY_SONG) && (songMate = OursNear(a, 8, IsFamilyOf)) != NULL)
        score[GA_SONG] = 20 + rec->cur[LIFE_WARM] / 10;
    for (i = 0; i < GA_COUNT; i++)
        total += score[i];
    if (total == 0)
        return;
    pick = Random() % total;
    for (i = 0; i < GA_COUNT; i++)
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
        {
            // a family greeting: a happy twirl first (docs 12)
            bool8 greet = MP_Life_FamilyTrait(rec, TRAIT_FAMILY_GREETING) != NULL;
            Plan(a, GA_COMPANY, x, y, me->currentCoords.x, me->currentCoords.y, 60 * 6, greet ? LIFE_POSE_SPIN : LIFE_POSE_SIT,
                 greet || rec->cur[LIFE_WARM] >= 130 ? LIFE_EMOTE_HEART : LIFE_EMOTE_CALM);
        }
        break;
    case GA_PURSUIT:
        StartPursuit(a, rec, pursuit);
        break;
    case GA_SONG:
        if (songMate && BesideActor(a, songMate, &x, &y))
        {
            Plan(a, GA_SONG, x, y, songMate->cur.x, songMate->cur.y, 60 * 8, LIFE_POSE_SIT, LIFE_EMOTE_NOTE);
            Plan(songMate, GA_SONG, songMate->cur.x, songMate->cur.y, x, y, 60 * 8, LIFE_POSE_SIT, LIFE_EMOTE_NOTE);
            a->lead = TRUE;
            a->partner = songMate - sL.a;
            songMate->partner = a - sL.a;
            MPNet_Log("garden: %u and %u sing their family's song", a->dex, songMate->dex);
        }
        break;
    case GA_QUIET:
    {
        // the corner farthest from everyone else nearby
        s16 bx = a->cur.x, by = a->cur.y, k;
        int best = -1;
        for (k = 0; k < 12; k++)
        {
            s16 tx = GX(GARDEN_X0 + Random() % (GARDEN_X1 - GARDEN_X0 + 1));
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
    case GA_PURSUIT:  MP_Life_Satisfy(rec, NEED_PLAY, 8); MP_Life_Feel(rec, EMO_JOY, 1); break;
    case GA_RARE:
    case GA_SONG:     MP_Life_Satisfy(rec, NEED_COMPANY, 10); MP_Life_Feel(rec, EMO_JOY, 2); MP_Life_Feel(rec, EMO_CALM, 2); break;
    case GA_REACT:    MP_Life_Satisfy(rec, NEED_COMPANY, 4); break;
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
    {
        Satisfy(a, rec);
        if (a->act == GA_RARE && Seen(a))
            a->seen = TRUE;
    }
    // the nervous one finds its courage halfway (a paddle), a family greeting's twirl ends
    if (a->act == GA_RARE && a->rare == LIFE_RARE_PADDLE && !a->lead && a->timer == a->duration / 2)
    {
        a->pose = LIFE_POSE_SHAKE;
        a->emote = LIFE_EMOTE_WATER;
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), LIFE_EMOTE_EXCLAIM);
    }
    if (a->act == GA_COMPANY && a->pose == LIFE_POSE_SPIN && a->timer == 45)
        a->pose = LIFE_POSE_SIT;
    if (a->emote != LIFE_EMOTE_NONE && a->timer && (a->timer % 180) == 0)
        MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), a->emote);
    if ((a->act == GA_PLAY || a->act == GA_SQUABBLE) && a->partner >= 0 && sL.a[a->partner].used && (a->timer % 30) == 0)
        a->dir = DirToward(a->cur, sL.a[a->partner].cur.x, sL.a[a->partner].cur.y, a->dir);
    if (a->timer >= a->duration)
    {
        bool8 celebrate = FALSE;
        if (a->act == GA_PLAY || a->act == GA_CHASE || a->act == GA_SQUABBLE)
            Shared(a, rec);
        if (a->act == GA_FORAGE)
            MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(a - sL.a), LIFE_EMOTE_NOTE);
        if (a->act == GA_PURSUIT)
            celebrate = PursuitDone(a, rec);
        if (a->act == GA_RARE && a->lead)
            RareDone(a, rec);
        if (a->act == GA_SONG && a->lead)
            SongDone(a);
        a->act = GA_NONE;
        a->pose = LIFE_POSE_NONE;
        a->partner = -1;
        a->decor = -1;
        a->rare = 0;
        a->lead = a->learner = FALSE;
        a->cooldown = 40 + Random() % 120;
        if (celebrate)
            MP_Garden_ActorReaction(a - sL.a, LIFE_EMOTE_SPARKLE, LIFE_POSE_HOP, 90);   // a milestone!
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

// What exactly it's doing, for WATCH mode: GA_RARE its LIFE_RARE_* (| 0x80 if the other one started it),
// GA_PURSUIT its LIFE_PURSUIT_* (0x80 the one being taught); 0 when unknown (a friend's Pokémon)
u8 MP_GardenLife_Detail(u8 i)
{
    struct Actor *a;
    struct MpLifeRec *rec;
    if (i >= MAX_ACTORS || !sL.a[i].used || sL.a[i].owner >= 0)
        return 0;
    a = &sL.a[i];
    if (a->act == GA_RARE)
        return a->rare | (a->lead ? 0 : 0x80);
    if (a->act == GA_PURSUIT)
        return a->learner ? 0x80 : (rec = RecOf(a)) != NULL ? rec->pursuit : 0;
    return 0;
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
static void QueueStep(struct Actor *a, struct Coords16 t)
{
    if (t.x == a->qLast.x && t.y == a->qLast.y)
        return;
    if (a->qLen == ARRAY_COUNT(a->q))
    {
        memmove(&a->q[0], &a->q[1], (a->qLen - 1) * sizeof(a->q[0]));   // far behind: the oldest step goes
        a->qLen--;
    }
    a->q[a->qLen++] = t;
    a->qLast = t;
}

// A friend's Garden Pokémon, as their game says: the tile it's on or stepping onto (x, y) and the one it stepped
// from (fx, fy), queued and replayed in order; its pace; its facing and pose once it's there
s8 MP_GardenLife_SetRemote(s8 owner, u8 index, u32 key, u16 dex, bool8 female, s16 hue, s16 x, s16 y, s16 fx, s16 fy,
                           u8 speed, u8 dir, u8 pose, u8 act)
{
    u8 i;
    struct Actor *a = NULL;
    struct Coords16 t, f;
    for (i = 0; i < MAX_ACTORS; i++)
        if (sL.a[i].used && sL.a[i].owner == owner && sL.a[i].slot == index)
            a = &sL.a[i];
    if (a && (a->key != key || a->dex != dex))
    {
        FreeActor(a);
        a = NULL;
    }
    t.x = x, t.y = y;
    f.x = fx, f.y = fy;
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
        a->from = a->prev = a->cur = a->target = a->qLast = t;
        a->moved = 16;
        a->dir = dir;
        MPNet_Log("garden: player %d's %u here", owner + 1, dex);
    }
    if (abs(x - a->qLast.x) + abs(y - a->qLast.y) > 4)
    {
        // far from where it was (a lag spike): it simply goes on from there
        a->qLen = 0;
        a->from = a->prev = a->cur = a->target = a->qLast = t;
        a->moved = 16;
    }
    else if (!(t.x == a->qLast.x && t.y == a->qLast.y))
    {
        // a new step (the same report again changes nothing): the step before first, if it was sent together
        // with this one
        if (!(f.x == t.x && f.y == t.y) && abs(f.x - a->qLast.x) + abs(f.y - a->qLast.y) == 1)
            QueueStep(a, f);
        QueueStep(a, t);
    }
    a->speed = speed ? speed : 1;
    if (a->qLen == 0 && a->moved >= 16 && a->cur.x == x && a->cur.y == y)
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

bool8 MP_GardenLife_Local(u8 n, u32 *key, u16 *dex, u8 *female, s16 *hue, s16 *x, s16 *y, s16 *fx, s16 *fy, u8 *speed, u8 *dir,
                          u8 *pose, u8 *act, u8 *actor)
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
        *x = a->cur.x;     // the tile it's on or stepping onto, and the one it stepped from: friends' games
        *y = a->cur.y;     // replay its steps
        *fx = a->from.x;
        *fy = a->from.y;
        *speed = a->speed;
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
    sL.rareDone = 0;
    sL.moveTrace = getenv("MOEMON_MOVETRACE") != NULL;
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
    if (sL.moveTrace)
    {
        // tests (MOEMON_MOVETRACE, check_movetrace.py): each Garden Pokémon's tile and facing as they change,
        // under its owner's id and its key, so two games' logs can be compared step by step
        for (i = 0; i < MAX_ACTORS; i++)
        {
            struct Actor *a = &sL.a[i];
            if (gMpLocalId < 0)
                a->traceX = -1;   // (not in a room yet: logged once it is, under its owner's id)
            if (!a->used || gMpLocalId < 0 || (a->cur.x == a->traceX && a->cur.y == a->traceY && a->dir == a->traceDir))
                continue;
            a->traceX = a->cur.x;
            a->traceY = a->cur.y;
            a->traceDir = a->dir;
            printf("[test] mv G%d:%08x %d %d %d\n", a->owner < 0 ? gMpLocalId : a->owner, (unsigned)a->key,
                   a->cur.x - MAP_OFFSET, a->cur.y - MAP_OFFSET, a->dir);
        }
    }
}

void MP_GardenLife_Debug(const char *args)
{
    u8 i;
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *a = &sL.a[i];
        if (!a->used)
            continue;
        printf("[test] garden actor %u: dex %u owner %d at %d,%d act %u phase %u pose %u partner %d rare %u lead %d learner %d\n", i,
               a->dex, a->owner, a->cur.x - MAP_OFFSET, a->cur.y - MAP_OFFSET, a->act, a->phase, a->pose, a->partner, a->rare,
               a->lead, a->learner);
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

// Tests: line everything up for a rare moment (bonds, temperament; time and weather waived) and let it happen
void MP_GardenLife_DebugRare(u8 actor, u8 kind)
{
    struct Actor *a;
    struct MpLifeRec *ra;
    u8 i;
    if (actor >= MAX_ACTORS || !sL.a[actor].used || (ra = RecOf(&sL.a[actor])) == NULL || kind == 0 || kind >= LIFE_RARE_COUNT)
        return;
    a = &sL.a[actor];
    for (i = 0; i < MAX_ACTORS; i++)
    {
        struct Actor *b = &sL.a[i];
        struct MpLifeRec *rb;
        struct MpLifeBond *ab, *ba;
        if (b == a || !b->used || b->owner >= 0 || (rb = RecOf(b)) == NULL)
            continue;
        ab = MP_Life_Bond(a->key, b->key, TRUE);
        ba = MP_Life_Bond(b->key, a->key, TRUE);
        if (ab == NULL || ba == NULL)
            continue;
        ab->v[BOND_AFFECTION] = ba->v[BOND_AFFECTION] = 200;
        ab->v[BOND_TRUST] = ba->v[BOND_TRUST] = 160;
        ab->v[BOND_RESPECT] = ba->v[BOND_RESPECT] = 130;
        ab->shared = ba->shared = 10;
        if (kind == LIFE_RARE_RIVAL_NAP)
            ab->flags |= LIFE_BOND_RIVAL, ba->flags |= LIFE_BOND_RIVAL;
        if (kind == LIFE_RARE_FAMILY)
            ab->flags |= LIFE_BOND_FAMILY, ba->flags |= LIFE_BOND_FAMILY;
        if (kind == LIFE_RARE_PADDLE)
            rb->cur[LIFE_BOLD] = 90;
        b->act = GA_NONE;
        b->reactTimer = 0;
    }
    ra->cur[LIFE_CURIOUS] = 160;
    ra->cur[LIFE_WARM] = 170;
    a->act = GA_NONE;
    a->reactTimer = 0;
    printf("[test] garden rare %u: %s\n", kind, TryRare(a, ra, kind) ? "started" : "no");
}

// Tests: its pursuit at this level, now
void MP_GardenLife_DebugPursuit(u8 actor, u8 level)
{
    struct MpLifeRec *rec;
    struct Actor *a;
    u8 p;
    if (actor >= MAX_ACTORS || !sL.a[actor].used || (rec = RecOf(&sL.a[actor])) == NULL)
        return;
    a = &sL.a[actor];
    p = rec->pursuit ? rec->pursuit : MP_Life_PursuitFor(rec);
    a->act = GA_NONE;
    a->reactTimer = 0;
    if (p != LIFE_PURSUIT_NONE)
    {
        rec->pursuit = p;
        rec->pursuitLevel = level;
        StartPursuit(a, rec, p);
    }
    printf("[test] garden pursuit %u level %u: %s\n", rec->pursuit, rec->pursuitLevel, a->act == GA_PURSUIT ? "started" : "no");
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
