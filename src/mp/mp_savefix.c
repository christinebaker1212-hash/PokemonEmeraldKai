// Moemon Kai: older saves into today's layout (include/mp_savefix.h has the format versions).
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_savefix.h"

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
    MPNet_Log("save: format %d -> %d%s", fromFormat, MP_SAVE_FORMAT, sWin160 ? " (1.60.0's Windows layout)" : "");
}
