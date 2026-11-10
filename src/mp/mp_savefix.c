// Moemon Kai: older saves into today's layout (include/mp_savefix.h has the format versions).
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_savefix.h"
#include "mp_region.h"
#include "event_data.h"
#include "constants/map_groups.h"

static u8 sRaw[sizeof(struct SaveBlock1) + MP_SAVE_WIN160_EXTRA];
static bool8 sWin160;         // the save was 1.60.0's Windows layout (converted)
static bool8 sFreshlyLoaded;  // SaveBlock1 came from the save file and hasn't been migrated yet

void MP_SaveFix_BeginSaveBlock1(void)
{
    memcpy(sRaw, gSaveBlock1Ptr, sizeof(struct SaveBlock1));   // a sector that doesn't check out keeps what was there
    memset(sRaw + sizeof(struct SaveBlock1), 0, MP_SAVE_WIN160_EXTRA);
}

u8 *MP_SaveFix_RawSaveBlock1(void)
{
    return sRaw;
}

// 1.60.0 on Windows: struct ObjectEvent was 40 bytes (Windows' bitfield layout had no padding byte at its end, where
// 1.60.0 put the graphics id's high byte), so the 16 object events took 64 bytes more and everything after them
// moved along. Each event is its first 36 bytes (the layout Windows always had) and its high byte at 36.
static void FromWin160(void)
{
    u8 *dst = (u8 *)gSaveBlock1Ptr;
    const u32 events = offsetof(struct SaveBlock1, objectEvents);
    const u32 oldStride = sizeof(struct ObjectEvent) + MP_SAVE_WIN160_EXTRA / OBJECT_EVENTS_COUNT;
    u32 i;

    memcpy(dst, sRaw, events);
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        const u8 *src = sRaw + events + oldStride * i;
        struct ObjectEvent *obj = &gSaveBlock1Ptr->objectEvents[i];
        memcpy(obj, src, sizeof(struct ObjectEvent));
        obj->graphicsIdHi = src[sizeof(struct ObjectEvent)] & 0xF;
    }
    memcpy(dst + events + sizeof(struct ObjectEvent) * OBJECT_EVENTS_COUNT,
           sRaw + events + oldStride * OBJECT_EVENTS_COUNT,
           sizeof(struct SaveBlock1) - events - sizeof(struct ObjectEvent) * OBJECT_EVENTS_COUNT);
}

void MP_SaveFix_StoreSaveBlock1(bool8 win160)
{
    sWin160 = win160;
    if (win160)
        FromWin160();
    else
        memcpy(gSaveBlock1Ptr, sRaw, sizeof(struct SaveBlock1));
    sFreshlyLoaded = TRUE;
}

void MP_SaveFix_Migrate(u8 fromFormat)
{
    u32 i;
    if (!sFreshlyLoaded)
        return;   // (only data just read from the file; a second call would undo a step)
    sFreshlyLoaded = FALSE;
    if (fromFormat >= MP_SAVE_FORMAT)
        return;
    if (fromFormat == 0)
    {
        // Emerald's layout: the bytes the graphics ids' high parts use now were padding
        for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
            gSaveBlock1Ptr->objectEventTemplates[i].graphicsIdHi = 0;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            gSaveBlock1Ptr->objectEvents[i].graphicsIdHi = 0;
    }
    else if (fromFormat == 1)
    {
        // 1.60.0: templates had kind at 0x01 and a 16-bit graphics id at 0x02-0x03
        for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
        {
            u8 *t = (u8 *)&gSaveBlock1Ptr->objectEventTemplates[i];
            u8 kind = t[1];
            t[1] = t[2];   // the id's low byte
            t[2] = kind;   // (its high byte stays at 0x03)
        }
#ifndef _WIN32
        // 1.60.0 on Linux kept the object events' high byte at 0x23, padding in today's layout there
        if (!sWin160)
        {
            for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            {
                u8 *o = (u8 *)&gSaveBlock1Ptr->objectEvents[i];
                gSaveBlock1Ptr->objectEvents[i].graphicsIdHi = o[0x23] & 0xF;
                o[0x23] = 0;
            }
        }
#endif
    }
    if (fromFormat < 3)
    {
        // JOHTO's phone kept its state in region vars 0x5240-0x5258, where SINNOH's Platinum vars 0x5180-0x525D also
        // are (every region shares 0x5000-0x52FF): its own block now, 0x5260. What was there goes to the phone if the
        // player has been to JOHTO; it stays for SINNOH's story only if they have been to SINNOH (a JOHTO-only save's
        // phone numbers there would have read as SINNOH's story already played)
        u16 old[25];
        bool8 johto = (gRegionState.started >> REGION_JOHTO) & 1, sinnoh = (gRegionState.started >> REGION_SINNOH) & 1;
        for (i = 0; i < ARRAY_COUNT(old); i++)
            old[i] = VarGet(0x5240 + i);
        for (i = 0; i < ARRAY_COUNT(old); i++)
        {
            VarSet(0x5260 + i, johto ? old[i] : 0);
            if (!sinnoh)
                VarSet(0x5240 + i, 0);
        }
        // the first boat's arrival (a frame script's, never kept past the port): 0x52FD is JOHTO's rival name's again
        VarSet(VAR_REGION_ARRIVAL, 0);
    }
    if (fromFormat < 4)
    {
        // Saved inside GALACTIC HQ or its warehouse while their doors weren't there, maybe without the GALACTIC KEY:
        // the doors open, as their GALACTIC KEY scripts leave them (FLAG_HIDE_..._DOOR, tools/sinnoh/ids.py numbers:
        // 1F, 3F, 4F, B2F and the warehouse's). (Platinum's first arrival flag, FLAG_FIRST_ARRIVAL_GALACTIC_HQ, was
        // never set then: GALACTIC HQ 1F's own entry script didn't run, tools/sinnoh/gen_scripts.py inline_ours)
        static const u16 sDoors[] = {0x3421, 0x3423, 0x3425, 0x3426, 0x3507};
        static const u16 sInside[] = {MAP_SINNOH_GALACTIC_HQ1F, MAP_SINNOH_GALACTIC_HQ2F, MAP_SINNOH_GALACTIC_HQ3F,
            MAP_SINNOH_GALACTIC_HQ4F, MAP_SINNOH_GALACTIC_HQ_B1F, MAP_SINNOH_GALACTIC_HQ_B2F,
            MAP_SINNOH_GALACTIC_HQ_CONTROL_ROOM, MAP_SINNOH_GALACTIC_HQ_HALL, MAP_SINNOH_GALACTIC_HQ_LABORATORY,
            MAP_SINNOH_VEILSTONE_CITY_GALACTIC_WAREHOUSE};
        u16 here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
        for (i = 0; i < ARRAY_COUNT(sInside) && sInside[i] != here; i++)
            ;
        if (i < ARRAY_COUNT(sInside))
            for (i = 0; i < ARRAY_COUNT(sDoors); i++)
                FlagSet(sDoors[i]);
    }
    MPNet_Log("save: format %d -> %d%s", fromFormat, MP_SAVE_FORMAT, sWin160 ? " (1.60.0's Windows layout)" : "");
}
