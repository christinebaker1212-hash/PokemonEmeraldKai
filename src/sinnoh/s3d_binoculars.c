// Moemon Kai: Platinum's binoculars in SINNOH (tools/sinnoh/gen_scripts.py turns their two commands into scripts that
// warp, look and warp back on these ops: s3d_misc.c Sinnoh_MiscCall 68-72):
// - the VISTA LIGHTHOUSE's (binoculars_vista_lighthouse.c UseVistaLighthouseBinoculars): the view across the sea to the
//   POKéMON LEAGUE, from Platinum's own camera (VistaLighthouseBinoculars_SetCamera: its target, angles, distance and
//   field of view; s3d.c's view takes them while it lasts), until A or B;
// - the GREAT MARSH lookout's (great_marsh_lookout.c, PASTORIA's observatory): five looks at spots in the marsh
//   (src/data/sinnoh_marsh_lookout.h: tools/sinnoh/gen_marsh_lookout.py), each with one of the area's POKéMON (its
//   wild table, the day's pick in its 5 percent slots: GreatMarshBinoculars_GetMonSpecies) shown in the middle as
//   Platinum shows its front picture, for 60 of Platinum's frames or until A.
// The player is hidden while looking; the view is framed by the binoculars' black mask, two round lenses that open
// and close (Platinum's screen wipe 17 / 16 with the "kasha" sound), drawn over the frame (sdl2.c S3D_BinocularsComposite).
#include <math.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "decompress.h"
#include "data.h"
#include "pokemon.h"
#include "random.h"
#include "sound.h"
#include "wild_encounter.h"
#include "fieldmap.h"
#include "malloc.h"
#include "mp_region.h"
#include "constants/songs.h"
#include "constants/map_groups.h"
#include "constants/species.h"
#include "mp.h"
#include "s3d.h"

#include "../data/sinnoh_marsh_lookout.h"

#define LOOKS 5        // (BINOCULARS_CYCLE_COUNT)
#define OPEN_FRAMES 16

bool8 Sinnoh_GreatMarshWildMap(u8 *group, u8 *num);
u16 Sinnoh_GreatMarshDailySpecies(u8 area);

static struct
{
    u8 kind;           // (0 the VISTA LIGHTHOUSE's, 1 the GREAT MARSH lookout's)
    u8 view;           // (0 off, 1 opening / open, 2 closing)
    u8 t;
    u8 look;
    u8 spots[LOOKS];
    s16 backX, backY;
    u16 species;
    u8 pic[0x800];     // (the POKéMON's front picture, 64 x 64 4bpp tiles)
    u16 pal[16];
} sBino;

static void HidePlayer(bool8 hidden)
{
    gObjectEvents[gPlayerAvatar.objectEventId].invisible = hidden;
}

// 68: start looking (A: the kind): the place to come back to kept, the marsh's five spots picked
void Sinnoh_BinocularsStart(u16 kind)
{
    const struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 i;
    memset(&sBino, 0, sizeof(sBino));
    sBino.kind = kind;
    sBino.backX = p->currentCoords.x - MAP_OFFSET;
    sBino.backY = p->currentCoords.y - MAP_OFFSET;
    for (i = 0; i < LOOKS; i++)
        sBino.spots[i] = Random() % ARRAY_COUNT(sMarshLookout);   // (LCRNG_RandMod(36) each, as Platinum's)
    MPNet_Log("sinnoh binoculars: kind %d from %d,%d", kind, sBino.backX, sBino.backY);
}

// 69: the marsh's next spot into VAR_0x8008 / 0x8009 (VAR_RESULT TRUE), or, all five seen, the way back (FALSE)
void Sinnoh_BinocularsNext(void)
{
    if (sBino.kind == 1 && sBino.look < LOOKS)
    {
        gSpecialVar_0x8008 = sMarshLookout[sBino.spots[sBino.look]][0];
        gSpecialVar_0x8009 = sMarshLookout[sBino.spots[sBino.look]][1];
        sBino.look++;
        gSpecialVar_Result = TRUE;
        return;
    }
    gSpecialVar_0x8008 = sBino.backX;
    gSpecialVar_0x8009 = sBino.backY;
    gSpecialVar_Result = FALSE;
}

// GreatMarshBinoculars_GetMonSpecies: one of the area's land POKéMON at random, the day's pick in slots 6 and 7
static u16 MarshSpecies(void)
{
    u8 group, num, slot = Random() % 12;
    u32 i;
    if (!Sinnoh_GreatMarshWildMap(&group, &num))
        return SPECIES_NONE;
    if (slot == 6 || slot == 7)
    {
        u16 daily = Sinnoh_GreatMarshDailySpecies(num - MAP_NUM(MAP_SINNOH_GREAT_MARSH1));
        if (MP_Gen_Allowed(daily))
            return daily;
    }
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        if (gWildMonHeaders[i].mapGroup == group && gWildMonHeaders[i].mapNum == num && gWildMonHeaders[i].landMonsInfo)
            return gWildMonHeaders[i].landMonsInfo->wildPokemon[slot].species;
    return SPECIES_NONE;
}

// 70: the view opens: the player hidden; the marsh's POKéMON drawn
void Sinnoh_BinocularsOpen(void)
{
    HidePlayer(TRUE);
    sBino.view = 1, sBino.t = 0;
    sBino.species = sBino.kind == 1 ? MarshSpecies() : SPECIES_NONE;
    if (sBino.species != SPECIES_NONE)
    {
        u32 personality = Random() | (Random() << 16);
        u16 *buf = AllocZeroed(0x20 * 16);
        LoadSpecialPokePic(&gMonFrontPicTable[sBino.species], sBino.pic, sBino.species, personality, TRUE);
        LZDecompressWram(GetMonSpritePalFromSpeciesAndPersonality(sBino.species, 0, personality), buf);
        memcpy(sBino.pal, buf, sizeof(sBino.pal));
        Free(buf);
    }
    PlaySE(SE_M_LOCK_ON);   // (SEQ_SE_DP_KASYA: the binoculars' click)
    MPNet_Log("sinnoh binoculars: look %d species %d", sBino.look, sBino.species);
}

// 71: the view closes (the lenses shut over OPEN_FRAMES)
void Sinnoh_BinocularsClose(void)
{
    sBino.view = 2, sBino.t = 0;
    PlaySE(SE_M_LOCK_ON);
}

// 72: looking is over (the warp back follows: VAR_0x8008 / 0x8009 from 69)
void Sinnoh_BinocularsEnd(void)
{
    sBino.view = 0;
    HidePlayer(FALSE);
}

// s3d.c's view while the VISTA LIGHTHOUSE's binoculars look: VistaLighthouseBinoculars_SetCamera (its target in
// Platinum's world units, the angles around it from its fx32 turns, the distance, the half field of view)
bool8 S3D_BinocularView(float *target, float *pitch, float *yaw, float *dist, float *halfFov)
{
    if (sBino.view == 0 || sBino.kind != 0)
        return FALSE;
    target[0] = 13573.1399f, target[1] = 350.8571f, target[2] = 9178.2534f;
    *pitch = 0.5086f * 4096.0f / 65536.0f * 360.0f;
    *yaw = 0.3203125f * 4096.0f / 65536.0f * 360.0f;
    *dist = 248.1074f;
    *halfFov = 0.547f * 4096.0f / 65536.0f * 360.0f;
    return TRUE;
}

// the mask: everything outside two round lenses side by side black; R the lenses' radius (0 shut)
static bool8 InLens(s32 x, s32 y, s32 r)
{
    s32 dy = y - DISPLAY_HEIGHT / 2, dl = x - (DISPLAY_WIDTH / 2 - 52), dr = x - (DISPLAY_WIDTH / 2 + 52);
    return dl * dl + dy * dy <= r * r || dr * dr + dy * dy <= r * r;
}

void S3D_BinocularsComposite(u16 *frame)
{
    s32 x, y, r, full = 92;
    if (sBino.view == 0)
        return;
    if (sBino.t < OPEN_FRAMES)
        sBino.t++;
    r = sBino.view == 1 ? full * sBino.t / OPEN_FRAMES : full * (OPEN_FRAMES - sBino.t) / OPEN_FRAMES;
    if (sBino.species != SPECIES_NONE && sBino.view == 1)
    {
        // (its front picture in the middle, as Platinum's lookout sprite stands)
        s32 ox = DISPLAY_WIDTH / 2 - 32, oy = DISPLAY_HEIGHT / 2 - 32;
        for (y = 0; y < 64; y++)
            for (x = 0; x < 64; x++)
            {
                u8 b = sBino.pic[((y / 8) * 8 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
                u8 c = (x & 1) ? b >> 4 : b & 15;
                if (c != 0)
                    frame[(oy + y) * DISPLAY_WIDTH + ox + x] = 0x8000 | (sBino.pal[c] & 0x7FFF);
            }
    }
    for (y = 0; y < DISPLAY_HEIGHT; y++)
        for (x = 0; x < DISPLAY_WIDTH; x++)
            if (!InLens(x, y, r))
                frame[y * DISPLAY_WIDTH + x] = 0x8000;
}
