#ifndef GUARD_MP_SAVEFIX_H
#define GUARD_MP_SAVEFIX_H

// Moemon Kai: older saves into today's layout (src/mp/mp_savefix.c). Every save a released build wrote must load:
// the save's format version (MP_SAVE_FORMAT) is kept in "<save>.region", and each step from an older one is here.
//   0  1.59.x and before (no region side file): Emerald's layout
//   1  1.60.0 (a region side file without the format): object event templates with a 16-bit graphics id at 0x02;
//      object events with the id's high byte at 0x23 (Linux) or 40 bytes long (Windows: SaveBlock1 64 bytes longer)
//   2  1.60.1: templates graphicsIdLo 0x01 / kind 0x02 / graphicsIdHi 0x03, object events' high bits in the first
//      word's padding (the same on every platform; tools/test/save_layout.py checks it)
#define MP_SAVE_FORMAT 2
#define MP_SAVE_WIN160_EXTRA 64   // 16 object events x 4 bytes more in 1.60.0's Windows build

void MP_SaveFix_BeginSaveBlock1(void);          // save.c: SaveBlock1's sectors are gathered whole ...
u8 *MP_SaveFix_RawSaveBlock1(void);
void MP_SaveFix_StoreSaveBlock1(bool8 win160);  // ... then put in gSaveBlock1Ptr (converted from 1.60.0 Windows)
void MP_SaveFix_Migrate(u8 fromFormat);         // after the save and its region side file are read

#endif // GUARD_MP_SAVEFIX_H
