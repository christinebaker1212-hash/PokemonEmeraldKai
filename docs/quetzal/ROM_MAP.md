# Moemon Quetzal (English Alpha 9 v1): where things are in the ROM

`MoemonQuetzalEnglishAlpha9v1.gba`, 32 MB, header `PKM QUETZAL` / `BPEE01`. A pokeemerald build with parts of
pokeemerald-expansion (Gen 1-9 species, "smol" sprite compression, 13-byte species names). Addresses are ROM
addresses; file offset = address - 0x08000000. The constants in `tools/quetzal_rip/*.py` are the same numbers.

## Maps and events

| What | Address | Format / notes |
|---|---|---|
| gMapGroups | 0x0893ADCC | 40 groups. 0-33 = stock Emerald's groups (same sizes and order). 34-35 = Johto (126 + 123 maps), 36-39 = Kanto (126+126+126+37) |
| map headers | from groups | stock 0x1C-byte MapHeader |
| layouts | from headers | stock MapLayout (+0x18 border width/height bytes); table of 1182 layouts at 0x08939B54 |
| object events | from MapEvents | 24 bytes: u8 localId, u8 kind, **u16 graphicsId**, s16 x, s16 y, u8 elevation, u8 movementType, u8 range, u8 pad, u16 trainerType, u16 sight/berry, script*, u16 flag, pad |
| warps / coord / bg events | | stock (bg hidden items store flag - FLAG_HIDDEN_ITEMS_START; Kanto's numbering restarts at 0) |
| region map sections | 0x09232A00 (main), 0x09233C94 (Johto) | 8 bytes {x, y, w, h, names**}; names** = 4 strings (EN, ES, ES, short). Johto maps index the Johto table with the same mapsec byte |
| GetCurrentRegion | 0x082A0BB4 | from the map group (34-35 Johto, 36-39 Kanto) |

## Scripts

| What | Address | Notes |
|---|---|---|
| gScriptCmdTable | 0x082BE9D0 | 246 commands; 0xE3-0xF5 new (docs/quetzal/SCRIPT_COMMANDS.md) |
| gSpecials | 0x082BEE04 | 837 entries; 13 inserted into the stock part (docs/quetzal/SPECIALS.md) |
| gStdScripts | 0x082BFB18 | 13 (11, 12 new) |
| gSpecialVars | 0x082BEDAC | stock |
| trainerbattle setup | 0x08105EA0 | types 0-12 stock, 13 = FRLG early-rival battle |
| flags / vars | 0x080E46BC / 0x080E451C | flags up to 0xE97 + 0x1000-0x2FFF; vars 0x4000-0x40FF + 0x5000-0x51FF (docs/quetzal/ENGINE.md) |

## Data

| What | Address | Format |
|---|---|---|
| species names | 0x08516B48 | 13 bytes each, 3058 entries (national dex 1-905, then megas / forms / Gen 9) |
| move names | 0x08520692 | 17 bytes, 849 |
| item names | 0x091EECE8 | 20 bytes, 632+ |
| trainer class names | 0x0850ED90 | 13 bytes, 78 (66-77 new: Burglar ... Team Rocket) |
| trainers | Hoenn 0x0850F188 (ids 0-973), Johto 0x095A8028 (0x1000-0x1380), Kanto 0x095B357C (0x2000-0x22E8) | 32-byte struct (see tools/quetzal_rip/data.py); presence bytes 0x095AF048 / 0x095B929C; GetTrainer 0x082A164C |
| trainer front pics | 0x084F4910 (+ palettes 0x084F51F8) | {data*, size, tag}; data is "smol" compressed (tools/quetzal_rip/trainer_pics.py); Kanto trainers add 0xDF to their pic id (0x082A16D4) |
| trainer back pics | 0x084F6CF8 (+ palettes 0x084F70C8) | raw 4bpp, 4-5 frames |
| wild encounter headers | 0x0918955C | 28 bytes {group, num, pad, 6 x WildPokemonInfo*}: land 12, water 5, rock smash 3, headbutt 5, fishing 10, unused |
| object event graphics | 0x090EA9E8 | 20000 u16-indexed pointers to stock 0x24-byte GraphicsInfo; Pokémon overworld sprites are "FFP2" blobs (tools/quetzal_rip/ffp2.py), ids = species + 256 (+5000 shiny), table in tools/quetzal_rip/pokemon_ow_gfx.json |
| object event palettes | 0x090FE26C - 0x09104584 | {colors*, u16 tag}; new NPCs use tags 0xB0xx/0xB1xx |
| tilesets | 0x08791xxx | stock struct; 8 primaries are FRLG-style 640 tiles / 640 metatiles / 7 palettes (ENGINE.md) |
| song table | 0x09397008 | 666 songs; 0-558 stock Emerald (incl. FRLG), 559-665 new |
| static encounters | Johto 0x095BAA00, Kanto 0x095BB74C | see SPECIALS.md |
