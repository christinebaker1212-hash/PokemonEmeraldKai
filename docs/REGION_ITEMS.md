# Key items between regions (the user's rules of 2026-11-08)

Status: built in 1.183.0 (src/mp/mp_regionitems.c; scenario region_key_items).

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

## How it works

- **Customs** (mp_region.c MP_Region_SendLuggage, every trip and the VICTORY ROAD gate): the BAG's other pockets go to
  the GLOBAL PC as before (1.101.0). The KEY ITEMS pocket: what doesn't travel waits in the region's stash (chunk
  "KIST" of the region side file); HMs that don't travel go to the GLOBAL PC with the region they came from noted.
  Customs says so (data/scripts/mp_region.inc gText_RegionKeyItemsStay).
- **Back in a region** (MP_Region_Travel): its stash returns to the BAG.
- **Always along** (sTravel): the CARD CASE, the CANDY JAR, the BATTLE FLAG, the ORRE and BATTLE ISLAND tickets.
- **Tools** (sTools: bikes, devices, containers, and what every region has: rods, COIN CASE, ITEMFINDER, S.S. TICKET):
  stay until the region they came from is beaten (the player is in it with the tool in the BAG: MP_RegionItems_Update,
  on every map load), then travel for good. A tool with the same name elsewhere is the same item: every copy in the
  stashes and the GLOBAL PC merges into the one in the BAG.
- **HMs** (the user, 2026-11-10: "they're put into the GLOBAL PC, and like-HMs are merged in the bag after the Elite
  Four"): customs always puts them in the GLOBAL PC, from a beaten region too; they come back, each merged with the
  BAG's own into one, once the region the player is in is beaten (MP_RegionItems_Update, on every map load). Until
  1.187.1 an HM from a beaten region travelled, so a champion arrived with SURF. The GLOBAL PC never hands HMs to an
  item PC.
- **Story items** (everything else in the KEY ITEMS pocket: tickets, fossils, keys, letters, scopes, orbs...): always
  stay in their region.
- **SINNOH's bike** is the MTN. BIKE (item 811, Platinum's BICYCLE in its converted scripts: tools/sinnoh/gen_scripts.py,
  items.py); the BICYCLE is KANTO's and JOHTO's. How a bike rides still follows the map (SINNOH's two gears there).
- **Saves from before** (no "KIST" chunk): the BAG's key items are sorted once: a tool from a beaten region (or one
  every region has, once any is beaten) travels; a story item or tool from another region goes to its home's stash
  (sStoryHome / sTools); a save begun in SINNOH that hasn't seen KANTO or JOHTO has its BICYCLE as the MTN. BIKE.
