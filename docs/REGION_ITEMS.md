# Key items between regions (the user's rules of 2026-11-08)

Status: built in 1.183.0, redone as keyed copies in 1.199.2 (src/mp/mp_regionitems.c; scenarios region_key_items,
region_key_items_fix).

## The user's rules (their words, 2026-11-08)

- "story-based key items and items that have the same purpose be left in the old region, so that the player doesn't
  start with a Bike, or a coin case, any of the rods, fossils, tickets, or an item finder."
- "The CARD CASE and CANDY JAR are exceptions to this rule."
- "The items that function differently are named based on their version, for the Bike, it would be Bicycle for Kanto
  and Johto, Mach Bike or Acro Bike for Hoenn, and Mtn. Bike for Sinnoh. Once a region is beaten, all variations can be
  used freely."
- "items with identical function, like identical number HMs, the Item Finder, the Coin Case, the Rods, etc, are merged
  into one item upon a region's defeat, with the Global PC counterparts being merged into the current Bag."

The open points, answered (2026-11-08):
1. The devices (MOéNAV, POKéGEAR, KANTO's TOWN MAP / FAME CHECKER / TEACHY TV, POKéTCH, POKé RADAR, VS SEEKER,
   JOURNAL): each region gives its own; it stays there, travelling once that region is beaten. "Yes, precisely."
2. "Once a region is beaten": "The region the item came from only."
3. HMs: "They stay in the Global PC until region defeat, merging after."
4. Containers (TM CASE, BERRY POUCH, POKéBLOCK CASE, POFFIN CASE, SEAL CASE...): "like the devices".

## The user's rule of 2026-11-13 (it replaces "tools travel once their region is beaten")

- "When the players went to Kanto, they kept every key item - Rods, Bikes, and all! This wasn't supposed to happen!
  I need a hotfix that sends them to the Global PC, and a rectification of the logic!"
- "The only things they can keep are Kai specific items, like the CANDY JAR, and the CARD CASE!"
- "I want the player to need to earn every tool legitimately for the region they're in. But in the POSTGAME, they get
  to use every key item. That's the distinction."
- "KANTO start, got all the rods. In the postgame after E4, they travel to JOHTO. Their rods go to the global PC, they
  get them in Johto. Then they beat Johto's E4, then their rods merge into one and become usable, until they travel to
  Hoenn, where they go to the global PC. ... in any order of region the player can beat regions in."
- "Each item should be uniquely keyed so it's possible to internally tell if an item is Good Rod (HOENN), or Good Rod
  (KANTO), and if the player has Good Rod (HOENN) in Kanto WITHOUT beating the E4, it auto goes to the Global PC."

## How it works (1.199.2)

- **Keyed copies**: every key item copy belongs to the region it was earned in. The GLOBAL PC's key items are kept
  per region (chunk "KIST" of the region side file, `stash[region]`), and the BAG's key items are recorded as the
  copies they are (`held[]`: item and region). A key item the BAG has with no record was earned where the player is.
- **Customs** (mp_region.c MP_Region_SendLuggage at every port and gate, and MP_Region_Travel for any crossing): the
  BAG's other pockets go to the GLOBAL PC as before (1.101.0); every key item but Kai's own goes to the GLOBAL PC, each
  copy under its own key. Kai's own (sTravel, always along): the CARD CASE, the CANDY JAR, the BATTLE FLAG, the ORRE and
  BATTLE ISLAND tickets. HMs go to the GLOBAL PC with the region they came from noted.
- **Arriving** (MP_Region_Travel -> MP_RegionItems_Arrive): the copies keyed to that region come back to the BAG.
- **A region whose LEAGUE isn't beaten**: only what was earned there. On every map load (MP_RegionItems_Update) a copy
  keyed to another region goes to the GLOBAL PC by itself.
- **A region whose LEAGUE is beaten (its postgame)**: every tool copy from every region (bikes, devices, containers,
  rods, the COIN CASE, the ITEMFINDER, the S.S. TICKET: sTools) is in the BAG as one item; leaving, each copy goes back
  to the GLOBAL PC under its own key, so every region still has its own when the player returns. The GLOBAL PC's HMs
  come back too, merged into one. Story items (everything else in the KEY ITEMS pocket) stay their region's.
- **Devices** work where the BAG has them (MP_Region_DeviceHere: their region, or any region in its postgame); the
  MOéNAV joins the BAG in HOENN once HOENN is beaten (MP_Region_GiveDevices), keyed HOENN's.
- **SINNOH's bike** is the MTN. BIKE (item 811, Platinum's BICYCLE in its converted scripts: tools/sinnoh/gen_scripts.py,
  items.py); the BICYCLE is KANTO's and JOHTO's. How a bike rides still follows the map (SINNOH's two gears there).
- **Saves from before 1.199.2** (KIST version under 3, or no KIST: from before 1.183.0) are sorted once (Sort): each key
  item in the BAG is keyed where it belongs: a regional tool or story item its home (sTools / sStoryHome); one every
  region has, the region the player is in if its own giver there has handed it over (KANTO's flags, JOHTO's ITEMFINDER;
  in SINNOH and UNOVA what's in the BAG is kept), else the region it came from (HOENN if beaten, else another beaten
  one). Then the rule above sends what doesn't belong to the GLOBAL PC. A save begun in SINNOH that hasn't seen KANTO
  or JOHTO has its BICYCLE as the MTN. BIKE.

## Before 1.199.2

1.183.0 let a tool travel once the region it came from was beaten, and merged its copies into one on every map load
anywhere: a champion of HOENN landed in KANTO with its bikes, rods and devices (the user, 2026-11-13).
