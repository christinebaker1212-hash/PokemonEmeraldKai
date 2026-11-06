// Moemon Kai: SINNOH's signposts, signboards and mailboxes, drawn as Platinum draws them: its map objects with their
// own 3D models (res/graphics/field_effects; packed through tools/sinnoh/gymprops.py). The maps keep those objects
// hidden and read each sign where it stands (tools/sinnoh/gen_maps.py), so none of them showed (the user, 2026-11-06:
// "A sign is invisible that tells you where you are"). Where they stand, which model and which way each faces:
// src/data/sinnoh_signs.h (tools/sinnoh/gen_signs.py). Placed on the ground under them as the map loads
// (s3d_gyms.c S3D_Gyms_MapLoaded), in dynamic prop slots 96-111 (S3D_SIGN_SLOTS).
#include "global.h"
#include "s3d.h"
#include "constants/map_groups.h"
#include "mp.h"
#include "../data/sinnoh_signs.h"

#define FIRST_SLOT 96
#define SLOTS 16

static u8 sShown;   // (slots in use on this map)

void Sinnoh_Signs_MapLoaded(float ox, float oz)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;
    sShown = 0;
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
        if (S3D_DynProp(FIRST_SLOT + sShown, s->model, x, h, z, 0))   // (Platinum draws them unturned, whatever their facing)
            sShown++;
    }
    if (sShown)
        MPNet_Log("sinnoh: %d signs on this map", sShown);
}

// Tests: the signs placed on this map
u8 Sinnoh_Signs_Count(void)
{
    return sShown;
}
