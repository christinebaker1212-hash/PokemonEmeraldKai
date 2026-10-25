# How Moemon Quetzal runs three regions

Quetzal plays Hoenn, Johto and Kanto as three campaigns in one save. This is everything the ROM does to make that
work, read from its code (addresses are ROM addresses; file offset = address - 0x08000000). "Established" means read
from the code; "inference" means deduced from behaviour or data. For Kai, **Kai's own systems take precedence**:
use this as a description of what Quetzal needs, not as a design to copy.

## 1. Which region the player is in

Established. The region is never stored as "the current region"; it is computed from the map the player stands on.

- `GetCurrentRegion` 0x082A0BB4: reads the map group from the save's location (SaveBlock1 + 0x474; stock +0x4):
  groups 34-35 -> 1 (Johto), 36-39 -> 2 (Kanto), anything else -> 0 (Hoenn).
- `GetRegionOfMapGroup` 0x082A0BF0: the same test for a given group (used for warp targets).
- `IsJohto` 0x08194C2C / `IsKanto` 0x08194C5C: wrappers used by the region map, map names and fly code. They honour
  an override byte at 0x030012C4 (the region map can show another region than the one you stand in: 0 = current,
  1-3 = forced).
- Port / transit maps (Hoenn group 25, maps 24-27 and 60, checked at 0x082A10A0) take the region from the
  dynamic warp (0x082A10CC) or from SaveBlock1 + 0x3761 (the region the player last entered), because they sit
  between regions.

So adding a region = a block of map groups + one line in GetCurrentRegion. Every other system below asks it.

## 2. What each region has of its own

| System | How Quetzal splits it | Where |
|---|---|---|
| Map names | Johto has its own region-map-section table (same 8-byte entries, same section numbers); Kanto uses the stock table's FRLG sections | 0x09233C94 (Johto), 0x09232A00 (others); chosen at 0x08194D34 |
| Region map | own graphics per region; Kanto has 4 maps (Kanto + 3 Sevii) switched like FRLG; own 15x28 section grids | ENGINE.md "Region map"; `quetzal_rip/region_map/` |
| Fly | own list per region {mapsec, heal location, unlock flag}; Johto adds two story checks | Johto 0x092347AC, Kanto 0x0923470C, Hoenn stock 0x09234674; SetFlyDestination 0x08194F3C |
| Heal / respawn points | one shared table, ids 0x17-0x25 Johto, 0x26-0x39 Kanto | 0x09225C60 |
| Badges | 8 badge flags per region: Hoenn 0x0A7F-0x0A86, Johto 0x11B2-0x11B9, Kanto 0x2100-0x2107 | table pointers at 0x082A0D38 (0x095A4C24 / 0x095A4C34 / 0x095A4C44); GetRegionBadgeFlag 0x082A0D38, HasRegionBadge 0x082A0DA4, CountRegionBadges 0x082A0DEC |
| Field moves (HM badges) | which badge each field move needs, per region (table below) | 0x082A0E98 Hoenn, 0x082A0F04 Johto, 0x082A0F64 Kanto; dispatcher 0x082A0FC0 |
| Stock badge flags | 0x082A0E34 re-syncs stock flag 0x7B and the region's badges when the count changes (inference: keeps stock code that reads FLAG_BADGE0x_GET working) | 0x082A0E34 |
| Pokédex "caught" | a bit array per region; "seen" and dex knowledge are shared ("What we know about each Pokémon is shared across all regions") | Hoenn SaveBlock1 + 0x3850, Johto + 0x304C, Kanto + 0x3254; 0x082A1140 / 0x082A11A0 (region 0xFF = any, 0xFE = current) |
| Champion / "region cleared" | one bit per region | SaveBlock1 + 0x3762 bits 0-2; 0x082B49D4 |
| Trainers | separate id ranges and tables: Hoenn 0-973, Johto 0x1000-0x1380, Kanto 0x2000-0x22E8; trainer flags are the ids | GetTrainer 0x082A164C; front pic ids for Kanto +0xDF (0x082A16D4) |
| Flags / vars | content convention: Johto flags 0x10xx-0x14xx / vars 0x50xx, Kanto flags 0x2xxx / vars 0x51xx; storage is one global block, not per region | SaveBlock1 + 0x2F28 block (ENGINE.md) |
| Hidden items | Kanto's hidden-item ids restart at 0 (FRLG's) | bg events |
| Static legendaries | one table per region | Johto 0x095BAA00 (106), Kanto 0x095BB74C (123) (SPECIALS.md) |
| Per-region options | Nuzlocke and berry respawn are chosen per region ("Respawning depends on the option you chose for this region"; "Come back after completing this region's challenge") | SaveBlock2 option bytes (ENGINE.md), specials 139/140 |
| Music | Johto's own songs; Kanto uses FRLG's | map headers |
| Tilesets | Johto / Kanto primaries are FRLG-style 640 / 7 | ENGINE.md |
| Encounter / story scripts | per-region "traveling event" checks on every town's map load: special 770 (Johto), 773 (Kanto) | SPECIALS.md |

Field move -> badge index (0 = first badge) per region:

| Move | Hoenn | Johto | Kanto |
|---|---|---|---|
| Flash | 1 | 0 | 0 |
| Rock Smash | 2 | 0 | 5 |
| Cut | 0 | 1 | 1 |
| Strength | 3 | 2 | 3 |
| Surf | 4 | 3 | 4 |
| Fly | 5 | 4 | 2 |
| Whirlpool | - | 6 | - |
| Dive | 6 | - | - |
| Waterfall | 7 | 7 | 7 |

## 3. Moving between regions

Established for the checks; the state change is partly inference.

- Each region has one port: Hoenn = Lilycove harbor (map 13.10), Johto = Olivine Port (35.12), Kanto = Vermilion
  City (37.62), listed in 0x095C1C88 (8 bytes per region: group, num, x, y...).
- The port clerk script (`Quetzal_EventScript_840F954` in `quetzal_rip/scripts/shared.inc`, used at Vermilion and
  the other ports) calls special **782** `GetReachableRegionsMask`: the player must stand on the current region's
  port map, must not be in a blocking state (link play, Giovanni mode, pending events: 0x082A300C, 0x082AF6DC,
  0x082ADE58, 0x082B04AC, 0x082B7BCC, 0x082B6454), the current region must be cleared, and the bag must hold the
  **S.S. TICKET** (item 0x2D7). The result is a bitmask of the other regions that are cleared. The clerk's refusal:
  "To visit another region, you need an S.S. Ticket and must have won both Leagues. Please finish any pending
  events before boarding, too."
- A multichoice (menus 0x8E-0x93) lists the reachable ports; special **783** `TravelToRegion` (region in
  VAR_0x8004) starts task 0x082B5820 with a mode and the region. That task fades out, saves the leaving region's
  state, writes SaveBlock1 + 0x3761 (region entered) and +0x3762-0x3764 (region state bytes), resets several
  subsystems (0x082B69D0 / 0x082B6DE0 call into bag/PC/dex helpers, not identified one by one), sets the
  arrival warp from the port table / last heal location (SaveBlock1 + 0x48C-0x492) and warps.
- Campaigns: the Kanto and Johto scripts start their own story at Pallet / New Bark, with their own starter
  selection (special 691) and their own dex; no script warps the player into Kanto or Johto from Hoenn (inference:
  the first region is chosen by code at new game, mode 2 of the same task, 0x082B5A8C).
- Johto's Indigo Plateau connects to Kanto by a new Route 22 gate that opens only for champions of both leagues
  (KANTO.md).

## 4. What a port into Kai needs from this

Kai's systems win wherever they overlap (followers, co-op, save handling, OPTIONS, level scaling, the Garden). What
Quetzal's region system implies for Kai, in the order the content depends on it:

1. A region id from the map group (one function, like 0x082A0BB4). Everything else keys on it.
2. Map section names and the region map per region (Johto table + Kanto/Sevii maps are ripped).
3. Fly / heal lists per region (ripped: `quetzal_rip/region_map/fly_heal.json`).
4. Badges per region, and the field-move badge table above, if Johto/Kanto keep their own gyms' progression.
   Otherwise map the Johto/Kanto badge flags onto whatever Kai decides.
5. How the player gets there: Quetzal's ports + S.S. Ticket after clearing a region. Gen 4 Pokémon appearing "only
   after the Elite Four and only in other regions" fits this naturally if Johto/Kanto open after Hoenn's League.
6. Optional: per-region caught records and region-cleared bits (Quetzal's "each region's own dex").
