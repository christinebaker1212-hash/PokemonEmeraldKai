#ifndef GUARD_MP_REGION_H
#define GUARD_MP_REGION_H
// Moemon Kai: HOENN, JOHTO and KANTO (src/mp/mp_region.c, docs/REGIONS_PLAN.md)

#define REGION_HOENN 0
#define REGION_JOHTO 1
#define REGION_KANTO 2
#define REGION_COUNT 3

// Johto / Kanto flags and vars past the save block's (tools/quetzal_port/import.py places them)
#define REGION_FLAGS_START 0x1000
#define REGION_FLAGS_END   0x4000   // SPECIAL_FLAGS_START
#define REGION_FLAG_BYTES  ((REGION_FLAGS_END - REGION_FLAGS_START) / 8)
#define REGION_VARS_START  0x5000
#define REGION_VARS_COUNT  0x300

u8 MP_Region_OfGroup(u8 group);
u8 MP_Region_Current(void);          // the region of the map the player is on
u8 *MP_Region_FlagPointer(u16 id);   // NULL outside REGION_FLAGS_START..END
u16 *MP_Region_VarPointer(u16 id);
void MP_Region_NewGame(void);
void MP_Region_WriteSidecar(void);   // "<save>.region", after the save file is written
void MP_Region_LoadFromSave(void);
#endif
