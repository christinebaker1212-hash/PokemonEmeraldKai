// Moemon Kai: ORRE's data pack, orre/orre_data.pak (tools/orre/build_pack.py): the game's own bytes of Colosseum's
// text, tables, screens' textures and the rest, read by name (src/orre/orre_data.c)
#ifndef GUARD_ORRE_DATA_H
#define GUARD_ORRE_DATA_H

#include <stdint.h>

// A file of the pack ("<archive>/<member>.<type>", "dol/menu_items.bin"...), inflated, malloc'd (the caller frees
// it); NULL: no pack or no such file
uint8_t *OrreData_Read(const char *name, uint32_t *size);
// The pack's name of the member a resource id names (index/ids.txt: "world_map/mapsnap000.tex"); FALSE none
int OrreData_ResourceName(uint32_t id, char *out, int outSize);
// The pack's path (MOEMON_ORRE_DATA, else orre/orre_data.pak next to the game)
const char *OrreData_Path(void);

#endif
