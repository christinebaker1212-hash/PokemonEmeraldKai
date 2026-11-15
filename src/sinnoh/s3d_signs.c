// Moemon Kai: SINNOH's signposts, signboards, mailboxes and books, drawn as Platinum draws them: its map objects with their
// own 3D models (res/graphics/field_effects; packed through tools/sinnoh/gymprops.py). The maps keep those objects
// hidden and read each sign where it stands (tools/sinnoh/gen_maps.py), so none of them showed (the user, 2026-11-06:
// "A sign is invisible that tells you where you are"). Where they stand, which model and which way each faces:
// src/data/sinnoh_signs.h (tools/sinnoh/gen_signs.py). Placed on the ground under them as the map loads
// (s3d_gyms.c S3D_Gyms_MapLoaded), in dynamic prop slots 96-111 (S3D_SIGN_SLOTS).
// The rest of Platinum's objects with models of their own come and go by its scripts (the ELITE FOUR's room doors
// shutting behind the player and opening once the member is beaten, the wall before ROTOM's room): the maps keep them
// as solid people nobody sees, there by their flags (tools/sinnoh/gen_maps.py), and each frame the model is drawn
// where one is (sSinnohModelObjects, slots 112-119). Before, they were left out: invisible, and the way out of each
// ELITE FOUR room open before the battle.
#include <string.h>
#include "global.h"
#include "s3d.h"
#include "constants/map_groups.h"
#include "mp.h"
#include "fieldmap.h"
#include "event_object_movement.h"
#include "event_data.h"
#include "sprite.h"
#include "mp_bigsprite.h"
#include "constants/sinnoh_gfx.h"
#include "../data/sinnoh_signs.h"

#define FIRST_SLOT 96
#define SLOTS 16
#define MODEL_SLOT 112
#define MODEL_SLOTS 8

static u8 sShown;   // (slots in use on this map)
static float sOx, sOz;
static bool8 sModels;   // (this map has objects drawn as models)
static u8 sWas[MODEL_SLOTS];   // (each one's last state, for the log: 0xFF none yet)

void Sinnoh_Signs_Update(void);
const struct MpBigSprite *S3D_Gfx_Wide(u16 index);

void Sinnoh_Signs_MapLoaded(float ox, float oz)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;
    sShown = 0;
    sOx = ox, sOz = oz;
    sModels = FALSE;
    memset(sWas, 0xFF, sizeof(sWas));
    for (i = 0; i < ARRAY_COUNT(sSinnohModelObjects); i++)
        if (sSinnohModelObjects[i].map == map)
            sModels = TRUE;
    Sinnoh_Signs_Update();
    for (i = 0; i < ARRAY_COUNT(sSinnohSigns) && sShown < SLOTS; i++)
    {
        const struct SinnohSign *s = &sSinnohSigns[i];
        float x, z, h;
        if (s->map != map)
            continue;
        x = ox + s->x * 16 + 8;
        z = oz + s->y * 16 + 8;
        h = S3D_TestGround(s->x * 16 + 8, s->y * 16 + 8, 0);
        if (h != h)   // (no ground there: NaN)
            h = 0;
        // (Platinum's signs are people: they stand in the way. Here they're read as signs where they stand, so the tile
        // is made solid; the player walked through them: the user, 2026-11-11)
        MapGridSetMetatileImpassabilityAt(s->x + MAP_OFFSET, s->y + MAP_OFFSET, TRUE);
        if (S3D_DynProp(FIRST_SLOT + sShown, s->model, x, h, z, 0))   // (Platinum draws them unturned, whatever their facing)
            sShown++;
    }
    if (sShown)
        MPNet_Log("sinnoh: %d signs on this map", sShown);
}

// Each frame: the doors and walls drawn where their objects are (none: the object is gone, removeobject)
void Sinnoh_Signs_Update(void)
{
    u16 map;
    u32 i, slot = 0;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        // (GALACTIC HQ's crowds of grunts: pictures wider than a sprite, drawn whole: s3d_gfx.c S3D_Gfx_Wide)
        struct ObjectEvent *o = &gObjectEvents[i];
        u16 g = OBJ_GFX_ID(o);
        const struct MpBigSprite *big;
        if (o->active && g >= OBJ_EVENT_GFX_SINNOH_FIRST && g < OBJ_EVENT_GFX_SINNOH_FIRST + SINNOH_SPRITE_COUNT
         && (big = S3D_Gfx_Wide(g - OBJ_EVENT_GFX_SINNOH_FIRST)) != NULL && o->spriteId < MAX_SPRITES)
            gSprites[o->spriteId].mpBig = big;
    }
    if (!sModels)
        return;
    map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    for (i = 0; i < ARRAY_COUNT(sSinnohModelObjects) && slot < MODEL_SLOTS; i++)
    {
        const struct SinnohModelObject *m = &sSinnohModelObjects[i];
        const struct ObjectEventTemplate *t = NULL;
        u8 id;
        u32 k;
        bool8 shown = FALSE;
        if (m->map != map)
            continue;
        // (by its flag, where its template stands: the field lets go of people out of view, and the 3D view sees
        // further; one walking about is drawn where it is)
        for (k = 0; k < gMapHeader.events->objectEventCount; k++)
            if (gMapHeader.events->objectEvents[k].localId == m->localId)
                t = &gMapHeader.events->objectEvents[k];
        if (t != NULL && !(t->flagId != 0 && FlagGet(t->flagId)))
        {
            s32 x = t->x, y = t->y;
            float h;
            if (!TryGetObjectEventIdByLocalIdAndMap(m->localId, map & 0xFF, map >> 8, &id))
                x = gObjectEvents[id].currentCoords.x - MAP_OFFSET, y = gObjectEvents[id].currentCoords.y - MAP_OFFSET;
            h = S3D_TestGround(x * 16 + 8, y * 16 + 8, 0);
            if (h != h)
                h = 0;
            shown = S3D_DynProp(MODEL_SLOT + slot, m->model, sOx + x * 16 + 8, h, sOz + y * 16 + 8, 0);
        }
        if (!shown)
            S3D_DynPropHide(MODEL_SLOT + slot);
        if (sWas[slot] != shown)
            MPNet_Log("sinnoh: model object %d (model %d) %s", m->localId, m->model, shown ? "shown" : "gone");
        sWas[slot] = shown;
        slot++;
    }
}

// Tests: the signs placed on this map
u8 Sinnoh_Signs_Count(void)
{
    return sShown;
}
