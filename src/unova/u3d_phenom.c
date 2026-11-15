// Moemon Kai: UNOVA's phenomena, as Black has them (aletheia's ov021 effect_encount.c, field_encount.c, event_fishing.c):
// rustling grass, dust clouds in caves, rippling water and the shadows of birds over bridges.
//
// - Once UNOVA's first badge is won, on a zone with an encounter table, each step counts; after the zone's setting's
//   steps (Black's EncountData.effectKind: 20 steps at 10%, or 5 at 15%: U3D_PhenomSetting) a roll; on success a
//   tile is picked at random in a window round the player (Black's sSearchAreaOffsets, inside the player's 32 x 32 block
//   of Black's grid: one of four half-windows, or the whole 11 x 11 for setting 2) among the tiles that can show one
//   (the pack keeps Black's MAP_ATTR_GetEncounterEffectKind of each tile: u3d.c U3D_PhenomKindAt), never under a person.
// - Stepping onto it: a dust cloud is a battle 40 in 100 (else an item: a gem 85%, an EVERSTONE 5%, a rare stone 10%),
//   a bird's shadow 20 in 100 (else a wing: HEALTH to SWIFT 90%, PRETTY 10%), the rest always a battle, from Black's
//   other tables (u3d_wild.c U3D_WildSpecial: type 2 on land, 4 on water), repels ignored. Fishing at the ripples
//   always bites, from the rippling water's fishing table (wild_encounter.c). The items come by Black's own script
//   (10133: "found" with its words), reading the item through PROC_FLD_StoreEffectEncountItemId (u3d_vm.c).
// - Drawn with the game's own field effects at the tile, over and over as Black's models loop: leaves flying from the
//   grass, dust, ripples, and a bird's shadow (a shadow sprite, darkening what's under it) going round over the planks.
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "main.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "trig.h"
#include "wild_encounter.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "mp.h"
#include "mp_region.h"
#include "u3d.h"

#define KIND_CAVE   4
#define KIND_WATER  5
#define KIND_DEEP   6
#define KIND_BRIDGE 7
#define SCRIPT_ITEM 10133   // (Black's 0x2795: the item a phenomenon leaves)

extern const struct SpriteTemplate *const gFieldEffectObjectTemplatePointers[];
bool8 U3D_Vm_RunScene(u16 scriptId);
bool8 MP_U3dSpecialWild(u8 type);   // (wild_encounter.c)
void CB2_Overworld(void);

static struct
{
    bool8 active;
    u8 kind;
    s16 x, y;          // (map tiles)
    u16 zone;
    u32 steps;
    u16 item;          // (Black's number: the VM's StoreEffectEncountItemId)
    u8 shadow;         // (the bird's shadow sprite, MAX_SPRITES: none)
    u16 t;
    u8 rig;            // (tests: 1 the next one an item, 2 a battle)
} P = {.shadow = MAX_SPRITES};

// Black's sSearchAreaOffsets (ov021 0x021D8E76): x min, x max, z min, z max round the player; [0] for setting 2, else one
// of the other four at random
static const s8 sAreas[5][4] = {{-5, 5, -5, 5}, {0, 5, -5, 5}, {-5, 0, -5, 5}, {-5, 5, -5, 0}, {-5, 5, 0, 5}};
// Black's sRareItemIds (0x021D8E8A): SUN, MOON, FIRE, THUNDER, WATER, LEAF, SHINY, DUSK, DAWN STONE, OVAL STONE
static const u16 sRare[10] = {80, 81, 82, 83, 84, 85, 107, 108, 109, 110};

#define SHADOW_MAGIC 0x5048

static void DropShadow(void)
{
    if (P.shadow < MAX_SPRITES && gSprites[P.shadow].inUse && gSprites[P.shadow].data[7] == SHADOW_MAGIC)
        DestroySprite(&gSprites[P.shadow]);
    P.shadow = MAX_SPRITES;
}

static void Clear(void)
{
    if (P.active)
        MPNet_Log("phenomenon: gone (kind %u at %d,%d)", P.kind, P.x - MAP_OFFSET, P.y - MAP_OFFSET);
    DropShadow();
    P.active = FALSE;
}

static struct ObjectEvent *Player(void)
{
    return &gObjectEvents[gPlayerAvatar.objectEventId];
}

static bool8 Allowed(void)
{
    u8 kind, steps, pct;
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) || U3D_CurrentZone() == 0xFFFF)
        return FALSE;
    if (MP_Region_ScaleBadges() == 0)   // (Black: MISC_GetBadge(0), the TRIO BADGE)
        return FALSE;
    return U3D_PhenomSetting(&kind, &steps, &pct);
}

// Black's EFFECT_ENCOUNT_CollectCandidates + PickSpot
static void TrySpawn(u8 setting)
{
    static s16 sCx[121], sCy[121];
    static u8 sCk[121];
    struct ObjectEvent *pl = Player();
    s16 px = pl->currentCoords.x, py = pl->currentCoords.y, dx, dy;
    s32 gx, gz, ox, oz;
    const s8 *a = sAreas[setting == 2 ? 0 : 1 + Random() % 4];
    s32 minX, maxX, minZ, maxZ;
    u32 n = 0, pick;
    U3D_GridOfMap(px, py, &gx, &gz);
    ox = ((gx % 32) + 32) % 32, oz = ((gz % 32) + 32) % 32;   // (the player's place in Black's block)
    minX = ox + a[0] < 0 ? 0 : ox + a[0];
    maxX = ox + a[1] > 31 ? 31 : ox + a[1];
    minZ = oz + a[2] < 0 ? 0 : oz + a[2];
    maxZ = oz + a[3] > 31 ? 31 : oz + a[3];
    for (dy = minZ - oz; dy <= maxZ - oz; dy++)
        for (dx = minX - ox; dx <= maxX - ox; dx++)
        {
            u8 k = U3D_PhenomKindAt(px + dx, py + dy);
            if (k != 0xFF && k != 9 && n < ARRAY_COUNT(sCx))   // (9: no-dash ground, no phenomenon)
            {
                sCx[n] = px + dx, sCy[n] = py + dy, sCk[n] = k;
                n++;
            }
        }
    if (n == 0)
        return;
    pick = Random() % n;
    if (GetObjectEventIdByXY(sCx[pick], sCy[pick]) != OBJECT_EVENTS_COUNT)
        return;   // (a person on it: Black's IsTileOccupied, nothing this time)
    P.active = TRUE;
    P.kind = sCk[pick];
    P.x = sCx[pick], P.y = sCy[pick];
    P.zone = U3D_CurrentZone();
    P.steps = 0;
    P.t = 0;
    MPNet_Log("phenomenon: kind %u at %d,%d (zone %u, %u candidates)", P.kind, P.x - MAP_OFFSET, P.y - MAP_OFFSET, P.zone, n);
}

// Black's EFFECT_ENCOUNT_PickItemKind4 / Kind7
static u16 PickItem(u8 kind)
{
    u16 roll = Random() % 1000;
    if (kind == KIND_BRIDGE)
        return roll < 900 ? 565 + (Random() % 600) / 100 : 571;
    if (roll < 100)
        return sRare[(Random() % 1000) / 100];
    if (roll < 950)
        return 548 + (Random() % 1700) / 100;
    return 229;
}

u16 Unova_Phenom_Item(void)
{
    return P.item;
}

// Black's EFFECT_ENCOUNT_TriggerSpot: the player has stepped onto it
static bool8 Trigger(void)
{
    u16 roll = Random() % 1000;
    u8 kind = P.kind;
    struct ObjectEvent *pl = Player();
    bool8 water = MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(pl->currentCoords.x, pl->currentCoords.y));
    if (P.rig)
        roll = P.rig == 1 ? 999 : 0, P.rig = 0;
    if ((kind == KIND_BRIDGE && roll >= 200) || (kind == KIND_CAVE && roll >= 400))
    {
        P.item = PickItem(kind);
        Clear();
        MPNet_Log("phenomenon: an item (Black's %u)", P.item);
        return U3D_Vm_RunScene(SCRIPT_ITEM);
    }
    Clear();
    MPNet_Log("phenomenon: a battle (%s)", water ? "rippling water" : "rustling");
    return MP_U3dSpecialWild(water ? 4 : 2);
}

// field_control_avatar.c ProcessPlayerFieldInput, each step taken on an UNOVA map (before the step's own encounter):
// TRUE when a battle or the item's script started
bool8 Unova_Phenom_Step(void)
{
    struct ObjectEvent *pl;
    u8 setting, steps, pct;
    if (!Allowed())
    {
        if (P.active)
            Clear();
        return FALSE;
    }
    U3D_PhenomSetting(&setting, &steps, &pct);
    pl = Player();
    if (P.active && P.zone != U3D_CurrentZone())
        Clear();
    if (P.active)
    {
        if (pl->currentCoords.x == P.x && pl->currentCoords.y == P.y)
            return Trigger();
        if (GetObjectEventIdByXY(P.x, P.y) != OBJECT_EVENTS_COUNT)   // (Black's ClearSpotIfOccupied)
            Clear();
        return FALSE;
    }
    if (++P.steps < steps)
        return FALSE;
    if ((Random() % 1000) >= pct * 10)
    {
        P.steps = 0;
        return FALSE;
    }
    TrySpawn(setting);
    return FALSE;
}

// event_fishing.c: fishing at the tile in front: TRUE when it's the ripples (a sure bite from the rippling table)
bool8 Unova_Phenom_FishingSpot(void)
{
    s16 x, y;
    if (!P.active || (P.kind != KIND_WATER && P.kind != KIND_DEEP) || !U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup))
        return FALSE;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    return x == P.x && y == P.y;
}

// wild_encounter.c: the ripples fished in are gone once the battle starts
void Unova_Phenom_Fished(void)
{
    Clear();
}

static void ShadowCallback(struct Sprite *s)
{
    s16 x = P.x, y = P.y;
    s->data[0]++;
    SetSpritePosToOffsetMapCoords(&x, &y, 8, 8);
    // (a bird going round overhead: a slow circle, two tiles across)
    s->x = x + (gSineTable[(s->data[0] * 2) & 0xFF] * 10 >> 8);
    s->y = y + (gSineTable[(s->data[0] * 2 + 64) & 0xFF] * 5 >> 8);
}

static void Draw(void)
{
    s16 x = P.x, y = P.y;
    struct ObjectEvent *pl = Player();
    s32 dist = abs(pl->currentCoords.x - P.x) > abs(pl->currentCoords.y - P.y) ? abs(pl->currentCoords.x - P.x)
                                                                                : abs(pl->currentCoords.y - P.y);
    u8 prio = gSprites[pl->spriteId].oam.priority;
    if (P.kind == KIND_BRIDGE)
    {
        if (P.shadow >= MAX_SPRITES || !gSprites[P.shadow].inUse || gSprites[P.shadow].data[7] != SHADOW_MAGIC)
        {
            u8 id = CreateSpriteAtEnd(gFieldEffectObjectTemplatePointers[FLDEFFOBJ_SHADOW_XL], 0, 0, 254);
            P.shadow = MAX_SPRITES;
            if (id == MAX_SPRITES)
                return;
            gSprites[id].callback = ShadowCallback;
            gSprites[id].coordOffsetEnabled = TRUE;
            gSprites[id].mpShadow = TRUE;
            gSprites[id].oam.priority = prio;
            gSprites[id].data[7] = SHADOW_MAGIC;
            P.shadow = id;
            ShadowCallback(&gSprites[id]);
        }
        if (P.t % 64 == 0 && dist < 10)
            PlaySE(SE_M_WING_ATTACK);
        return;
    }
    // (Black's models loop: here a puff, ripple or leaves every 16 / 24 frames, the sound once a loop)
    if (P.t % (P.kind == KIND_CAVE ? 16 : 24) != 0)
        return;
    gFieldEffectArguments[0] = x;
    gFieldEffectArguments[1] = y;
    gFieldEffectArguments[2] = pl->currentElevation;
    gFieldEffectArguments[3] = prio;
    if (P.kind == KIND_WATER || P.kind == KIND_DEEP)
    {
        SetSpritePosToOffsetMapCoords(&x, &y, 8, 6);   // (its middle on the tile: the 3D view stands it on its bottom row)
        gFieldEffectArguments[0] = x;
        gFieldEffectArguments[1] = y;
        gFieldEffectArguments[2] = 151;
        FieldEffectStart(FLDEFF_RIPPLE);
        if (dist < 10 && P.t % 48 == 0)
            PlaySE(SE_PUDDLE);
    }
    else if (P.kind == KIND_CAVE)
    {
        FieldEffectStart(FLDEFF_DUST);
        if (dist < 10 && P.t % 48 == 0)
            PlaySE(SE_M_SAND_ATTACK);
    }
    else
    {
        FieldEffectStart(FLDEFF_JUMP_TALL_GRASS);
        if (dist < 10 && P.t % 48 == 0)
            PlaySE(SE_M_POISON_POWDER);
    }
}

// u3d_screen.c Unova_Screen_FieldFrame, each frame on an UNOVA map
void Unova_Phenom_FieldFrame(void)
{
    if (!P.active)
        return;
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) || P.zone != U3D_CurrentZone())
    {
        Clear();
        return;
    }
    if (gMain.callback2 != CB2_Overworld || ArePlayerFieldControlsLocked() || gPaletteFade.active)
        return;
    Draw();
    P.t++;
}

// Tests: `mp u3dphenom [KIND [FRONT]]`: one now (a KIND tile if there's one in reach, else any), its place printed;
// FRONT 1: on the tile in front of the player, whatever its kind (3: and it gives its item, 4: a battle); 2: the steps
// that roll for one printed
void Unova_Phenom_Debug(s16 kind, s16 front)
{
    u32 tries;
    Clear();
    if (front == 2)
    {
        u8 setting, steps, pct;
        bool8 ok = U3D_PhenomSetting(&setting, &steps, &pct);
        printf("[test] phenomenon: allowed %d table %d setting %u steps %u odds %u%% counted %u\n", Allowed(), ok, setting, steps,
               pct, (unsigned)P.steps);
        return;
    }
    if (front == 1 || front == 3 || front == 4)
    {
        P.rig = front == 3 ? 1 : front == 4 ? 2 : 0;
        s16 x, y;
        GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
        P.active = TRUE;
        P.kind = kind < 0 ? 0 : kind;
        P.x = x, P.y = y;
        P.zone = U3D_CurrentZone();
        P.t = 0;
        printf("[test] phenomenon: kind %u at %d %d (tile's own %u)\n", P.kind, x - MAP_OFFSET, y - MAP_OFFSET,
               U3D_PhenomKindAt(x, y));
        return;
    }
    for (tries = 0; tries < 200 && !P.active; tries++)
    {
        u8 setting = tries & 1 ? 2 : 0;
        TrySpawn(setting);
        if (P.active && kind >= 0 && P.kind != kind)
            P.active = FALSE;
    }
    if (P.active)
        printf("[test] phenomenon: kind %u at %d %d\n", P.kind, P.x - MAP_OFFSET, P.y - MAP_OFFSET);
    else
        printf("[test] phenomenon: none in reach\n");
}
