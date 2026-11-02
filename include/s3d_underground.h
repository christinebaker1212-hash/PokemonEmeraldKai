// Moemon Kai: SINNOH's Underground, its digging and SPIRITOMB's counter only (src/sinnoh/s3d_underground.c,
// src/sinnoh/s3d_mining.c; COOP_NOTES "The Underground")
#ifndef GUARD_S3D_UNDERGROUND_H
#define GUARD_S3D_UNDERGROUND_H

// the save's part ("<save>.region" chunk UNDG, mp_region.c)
const void *Sinnoh_UndergroundData(u32 *size);
void Sinnoh_UndergroundLoad(const void *data, u32 size);
void Sinnoh_UndergroundReset(void);

bool8 Sinnoh_InUnderground(void);
bool8 Sinnoh_UndergroundEverDug(void);
void Sinnoh_UndergroundDugOnce(void);
bool8 Sinnoh_UndergroundPlateDug(u16 plate);
void Sinnoh_UndergroundSetPlateDug(u16 plate);

// the EXPLORER KIT (item_use.c): NULL where it works, else what is said
const u8 *Sinnoh_ExplorerKitCantUse(void);
void Sinnoh_ExplorerKitUse(void);

// the dig spots
void Sinnoh_Underground_MapLoaded(void);
void Sinnoh_Underground_Update(void);
s32 Sinnoh_UndergroundSpotAt(s16 x, s16 y);   // (map coordinates, no MAP_OFFSET; -1: none)
void Sinnoh_UndergroundSpotDug(s32 spot);
bool8 Sinnoh_UndergroundNearestSpot(s16 *x, s16 *y, u8 *dir);
const u8 *Sinnoh_UndergroundDigScript(s16 x, s16 y);   // (field_control_avatar.c: A facing a spot)
s32 Sinnoh_UndergroundDigSpot(void);

// SPIRITOMB: A facing a co-op friend in the Underground
void MP_Underground_TryTalk(void);
void Sinnoh_UndergroundDebug(void);

// the digging screen (s3d_mining.c), 480x320 like the BADGE CASE
void Sinnoh_UndergroundBulging(void);   // (special: gSinnohMessage <- "The wall is bulging slightly...")
void Sinnoh_UndergroundDig(void);       // (special, waitstate: the digging screen for the spot faced)
bool8 Sinnoh_Mining_IsActive(void);
bool8 Sinnoh_Mining_Step(u16 keys);
const u16 *Sinnoh_Mining_Frame(void);
void Sinnoh_MiningDebug(void);
bool8 Sinnoh_MiningTestCell(u8 *x, u8 *y);   // (tests: a cell over an item still buried)

#endif // GUARD_S3D_UNDERGROUND_H
