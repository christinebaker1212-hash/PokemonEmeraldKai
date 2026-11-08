# Key items between regions (design, the user's rules of 2026-11-08)

Status: designed, not built. The open points at the end need the user's answer first.

## The user's rules (their words, 2026-11-08)

- "story-based key items and items that have the same purpose be left in the old region, so that the player doesn't
  start with a Bike, or a coin case, any of the rods, fossils, tickets, or an item finder."
- "The CARD CASE and CANDY JAR are exceptions to this rule."
- "The items that function differently are named based on their version, for the Bike, it would be Bicycle for Kanto
  and Johto, Mach Bike or Acro Bike for Hoenn, and Mtn. Bike for Sinnoh. Once a region is beaten, all variations can be
  used freely."
- "items with identical function, like identical number HMs, the Item Finder, the Coin Case, the Rods, etc, are merged
  into one item upon a region's defeat, with the Global PC counterparts being merged into the current Bag."

## Today (1.101.0's rule)

Customs (mp_region.c MP_Region_SendLuggage) sends every BAG item outside the KEY ITEMS pocket to the GLOBAL PC; every
key item travels. The GLOBAL PC hands its items over in a region whose LEAGUE is beaten.

## Proposed

Each region keeps a stash of key items (a new region chunk "KI0 ".."KI3 ", like the item PCs' "PI0 "..).

- **Leaving region R** (customs): every key item goes into R's stash, except the travellers below. Arriving in region S,
  S's stash comes back into the BAG.
- **Travellers** (always with the player): CARD CASE, CANDY JAR, and Kai's own co-op items (BATTLE FLAG). Open point 1
  for the devices.
- **Regional versions** (same purpose, different item): HOENN's MACH BIKE / ACRO BIKE, KANTO / JOHTO's BICYCLE, SINNOH's
  MTN. BIKE (a new item: SINNOH's bike is the BICYCLE today). Each stays in its region until that region's LEAGUE is
  beaten; from then on it travels and works everywhere.
- **Identical items** (the same item in every region: OLD / GOOD / SUPER ROD, COIN CASE, ITEMFINDER, and the HMs):
  stay in their region like the rest. When a region's LEAGUE is beaten, every copy in the stashes and the GLOBAL PC is
  merged into one in the current BAG, and from then on it travels.
- **Story items** (tickets, fossils, keys, letters, scopes, orbs, region quest items): always stay in their region.

Save format: a sidecar chunk (no change to the save's layout: tools/test/save_layout.py), converted on load from
MP_SAVE_FORMAT's older sidecars (a save's current BAG is its current region's).

## Open points (the user's answer needed)

1. **The devices**: MOéNAV (HOENN), POKéGEAR (JOHTO), the KANTO TOWN MAP / FAME CHECKER / TEACHY TV, POKéTCH (SINNOH),
   POKé RADAR, VS SEEKER, JOURNAL. Proposed: each stays in its region like a regional version (each region gives its
   own), travelling once that region is beaten.
2. **"Once a region is beaten"**: proposed meaning, the region the item came from (HOENN beaten: the MACH and ACRO BIKE
   come along to KANTO). Or every region's versions once any region is beaten?
3. **The HMs** sit in the TMs & HMs pocket, which customs already sends to the GLOBAL PC with the other pockets.
   Proposed: they stay in the region's stash (with the key items) instead, merged on the region's defeat.
4. **Containers**: TM CASE, BERRY POUCH (KANTO's), the POKéBLOCK CASE, POFFIN CASE, SEAL CASE: proposed regional
   versions (stay until beaten).
