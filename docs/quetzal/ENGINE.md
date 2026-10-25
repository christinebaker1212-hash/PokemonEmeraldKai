# Moemon Quetzal: engine notes for the Kanto/Johto port

ROM addresses (file offset = address - 0x08000000). "Established" = read from code; GUESS = inferred.

## Tileset split (512/640) and palette split (6/7)

Established. The split is chosen **per map layout, by the identity of its primary tileset**:

- `0x080C6CFC IsFrlgStylePrimary(layout)`: compares `layout->primaryTileset` (MapLayout +0x10) with eight hard-coded
  tileset structs and returns TRUE for them:
  0x08791B40 Johto_Primary1, 0x08791B58 Johto_Primary2, 0x08791B70 Johto_Primary3, 0x08791B88 Johto_Primary4,
  0x08791BA0 Johto_Primary5, 0x08791BB8 Johto_Primary6, 0x08791EA0 Kanto_Primary7, 0x08792038 Kanto_General
  (names as in quetzal_rip/tilesets/tilesets.json). Every other primary uses the Emerald split.
- `0x080C6D60` / `0x080C6D7C`: number of primary tiles / primary metatiles = 640 if FRLG-style, else 512.
  Callers: tileset VRAM loaders 0x080C84C4 (primary tiles: count n at VRAM 0), 0x080C84E0/0x080C8508/0x080C8530
  (secondary: `1024 - n` tiles starting at tile n), and metatile lookups 0x080C7A2C (metatile attributes),
  0x080C9E90, 0x080CA634, 0x0815346C, 0x0826FE12. Total stays 1024 tiles / 1024 metatiles, so FRLG secondaries
  have 384 tiles and 384 metatiles (FRLG's NUM_*_IN_PRIMARY = 640, as in pokefirered's fieldmap.h).
- `0x080C857C`: primary palette count = 7 if FRLG-style, else 6 (FRLG's NUM_PALS_IN_PRIMARY 7). Callers
  0x080C865A / 0x080C8682 (load primary / secondary tileset palettes; secondary = 13 - n).
- Other callers of 0x080C6CFC: 0x080C1936, 0x080C9EBE, 0x080CA6AE, 0x0813FCD6, 0x0813FE3A, 0x081404A8, 0x0814054C
  (map preview/camera/region map code, not traced).

Port options for Kai (whose include/fieldmap.h is fixed 512/512/6): either make the split a per-layout property the
same way (a flag on struct Tileset or a lookup like the above, used in fieldmap.c's `CopyPrimary/SecondaryTilesetToVram`,
`LoadPrimary/SecondaryTilesetPalette`, `GetMetatileAttributesById`, and every `NUM_METATILES_IN_PRIMARY` user),
or convert the eight Kanto/Johto primaries (plus their secondaries) to 512/6 offline.

## Region

Established. The region is **not stored**; it is derived from the current map group every time:
`0x082A0BB4 GetCurrentRegion()` reads `sb1->location.mapGroup` (sb1+0x474): groups 34-35 -> 1 (Johto),
36-39 -> 2 (Kanto), anything else -> 0 (Hoenn). `0x08194C2C IsJohto()` uses it unless the region-map override byte
0x030012C4 is set (0 = current map, 2 = forced). A few Hoenn-group "port/transit" maps (group 25, nums 24-27, 60:
0x082A10A0) take the region from the dynamic warp instead (0x082A10CC), else from sb1+0x3761.

Per-region save data found (sb1 offsets):

| Data | Hoenn | Johto | Kanto | Code |
|---|---|---|---|---|
| Pokédex caught bits (species-1 bit order) | sb1+0x3850 | sb1+0x304C | sb1+0x3254 | 0x082A1140 / 0x082A11A0 (region 0xFF = OR of all three, 0xFE = current) |
| Badges | 8 flags per region from table 0x082A0D38 | | | 0x082A0DA4, count 0x082A0DEC |
| Region "cleared" (champion) | sb1+0x3762 bit 0 | bit 1 | bit 2 | 0x082B49D4 |
| Last/current region byte | sb1+0x3761 | | | 0x082B49B8 |

The Johto and Kanto caught arrays lie inside the new flag blocks (below): Johto at flags 0x1900+ and Kanto at
0x2940+ (inference from the offsets). This is how "Kanto and Johto catches have their own records" is implemented.
Map names: the region-map section table switches to 0x09233C94 (Johto, 8-byte entries, names at +4) in Johto,
0x09232A00 otherwise (from the earlier rip). Field-move badge requirements are per region (0x082A0FC0 ->
0x082A0E98 Hoenn / 0x082A0F04 Johto / 0x082A0F64 Kanto). Healing / fly locations and region map graphics: see "Region map, fly and heal locations" below.
Inter-region travel: special 782/783 (ports listed in table 0x095C1C88, one per region).

## New flag / var ranges

Established. A 0x800-byte extension block in SaveBlock1, global (not per region), lazily created:

| Range | sb1 offset | Size |
|---|---|---|
| magic "REG\x01" | +0x2F28 | 4 |
| flags 0x1000-0x1FFF | +0x2F2C | 0x200 |
| flags 0x2000-0x2FFF | +0x312C | 0x200 |
| vars 0x5000-0x50FF | +0x332C | 0x200 |
| vars 0x5100-0x51FF | +0x352C | 0x200 |

`GetFlagPointer` 0x080E46BC and `GetVarPointer` 0x080E451C call 0x080E3FBC first, which zeroes the block and writes
the magic if it is missing (0x080E3F00 check, 0x080E3F28 init), so old saves upgrade on first use.
Stock flags go up to 0xE97 (Quetzal enlarged the normal flag array; stock pokeemerald ends at 0x95F);
special flags 0x4000-0x407F at 0x020390D8; vars 0x4000-0x40FF at sb1+0x139C as stock.
By convention Johto content uses flags 0x10xx-0x14xx and vars 0x50xx, Kanto content flags 0x2xxx and vars 0x51xx
(observation from the rip, not enforced by code).

Other save layout facts: `sb1->location` is at +0x474 (stock +0x4) and `pos` at +0x470, i.e. SaveBlock1 has 0x470
extra bytes before the stock fields; dynamic warp at +0x484; saved party at sb1+0x6A8 (count +0x6A4).
SaveBlock2 options used by scripts: +0x2C7 bit 6 Giovanni mode, bit 2 bike type; +0x2D1 bit 0 move randomizer,
bit 6 Nuzlocke; +0x2D2 bits 5-7 party size limit; +0x2D3, +0x2D4, +0x2D5, +0x2D9 bit 5 (egg randomizer),
+0x2E7, +0x2EF bits 3-4 (gym team variant), +0x2F4 bit 4 (GUESS cheat mode); +0x2F8 quest records (8 bytes each).
struct Pokemon is 0x68 bytes (stock 0x64); mon data ids above stock exist (e.g. 0x59, 0x5A, 0x5E).

## Special vars

gSpecialVars table 0x082BEDAC: 0x8000-0x800B at 0x020390B4.., VAR_FACING 0x020390D0, VAR_RESULT 0x020390CC,
VAR_ITEM_ID 0x0203E9B0, VAR_LAST_TALKED 0x020390CE, VAR_MON_BOX_ID 0x020390D2, VAR_MON_BOX_POS 0x020390D4,
VAR_TRAINER_BATTLE_OPPONENT_A 0x0203A6D4 (same ids as stock).

## Other systems seen while reading specials

- Overworld followers (script command E3/E7, 0x08264E7C etc.), Nuzlocke, randomizers (wild/trainer/moves/abilities/
  evolutions/static/eggs), level cap ("exceeds the level cap" strings; 0x08072088 GUESS level-cap getter),
  built-in link play "MPLAYER" (player count 0x030055D4), a quest menu, Mega Stones / Z-Crystals.
- FRLG mechanics ported for Kanto (Seagallop, elevators via ListMenu, Lorelei dolls, Icefall Cave, Vermilion trash
  cans, museum fossil pics, early-rival trainerbattle 13); Quest Log calls left as a no-op (special 737).
- HGSS/GSC events for Johto (Bug-Catching Contest with its own EWRAM state 0x0203EE8C, Kenya/Shuckie, Odd Egg,
  Ruins of Alph puzzles, Celebi/Giovanni alternate past, Darkrai/Cresselia dream events).

## Region map, fly and heal locations

Established (code around 0x08194C00-0x08199000; the rip is `quetzal_rip/region_map/`, made by
`tools/quetzal_rip/region_map.py`, whose docstring lists every address).

- Region map graphics are chosen per region by LoadRegionMapGfx (0x08195244 / 0x08195170): Hoenn stock,
  Johto its own set, Kanto one of four sets (Kanto, Sevii 1-3, 4-5, 6-7) picked by the byte at 0x030012C5,
  which 0x08195134 cycles like FRLG's map switch. Same format as Emerald's (8bpp affine BG, 64x64 byte tilemap,
  palette at BG palette 7); Johto / Kanto palettes have 32 colors.
- The 15x28 section grids: Johto 0x0922FFE0, Kanto 0x0922B030 + 420 * sub map; Kanto's section -> sub map byte
  table is 0x0922B6C0.
- Fly: SetFlyDestination (0x08194F3C) uses a per-region list {mapsec, heal location, unlock flag}: Johto 0x092347AC
  (14 towns, flags 0x11BA-0x11C7), Kanto 0x0923470C (20, flags 0x2234-0x2264, FRLG's FLAG_WORLD_MAP_* by
  kanto_frlg_flags.json); Hoenn keeps the stock table.
- Heal locations: one table, 0x09225C60, ids 0x17-0x25 Johto and 0x26-0x39 Kanto, {group, num, x, y}. No
  per-heal-location Pokémon Center / nurse table was found (Emerald's whiteout respawn tables).
