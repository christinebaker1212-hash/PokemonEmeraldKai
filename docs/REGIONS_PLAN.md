# Kanto and Johto in Emerald Kai: the plan (locked in)

The content is Moemon Quetzal's Kanto and Johto (`quetzal_rip/`, capped to Gen 4: `quetzal_rip/gen4/`). Kai's own
systems always win where they overlap (followers, the Garden, co-op, OPTIONS, level scaling, HMs without badges, the
card games). Quetzal's region system is documented in `docs/quetzal/REGIONS.md`; this is how Kai does it.

## Regions
- Three regions: HOENN (Kai's maps), JOHTO, KANTO (Sevii Islands included). A map's region comes from its map group
  (`MP_Region_OfGroup`); a few transit maps (the boat) take it from the trip in progress.
- A NEW GAME can start in any of the three (picked after the LOOK step). Hoenn starts as Emerald does; Johto starts in
  New Bark, Kanto in Pallet: in the house the player stays in (a visitor's room, "your host" instead of "Mom"), with the
  region's professor giving the starter and the region's dex.
- Every region keeps its own progress in the save: started, Elite Four beaten, badges, last heal spot, fly spots,
  trainers beaten, its personal PC boxes. The party, the bag (minus region-only key items) and money travel with you.

## Travel (a commitment)
- After a region's Elite Four you get its ticket: HOENN = the S.S. TICKET from Dad (Emerald's own), JOHTO = Elm's,
  KANTO = Bill's. Before that region's Elite Four you can't leave it.
- Ports: HOENN = Slateport (the S.S. TIDAL), JOHTO = Olivine, KANTO = Vermilion. The boat goes to any other region.
- First arrival in a region: the boat docks, a short sightseeing walk in the port city with a guide, then a ride to the
  region's beginner town, where the professor gives a starter and the region's dex. Later trips land at the port.
- Bullet train: Goldenrod's station opens after Johto's Elite Four, Saffron's after Kanto's. It is region travel too.
- Route 22's gate (Johto's Indigo Plateau to Kanto): champions of both leagues only (Quetzal's rule).

## Customs (every port and station)
- Before boarding: everything but one Pokémon is deposited into the GLOBAL PC. The player picks the one who comes.

## PCs
| PC | Holds | Open |
|---|---|---|
| Personal PC | this region's boxes | always, in its region |
| Co-op shared PC | the room's boxes for this region | online, in its region |
| GLOBAL PC | what customs deposits, and anything moved in | after the first Elite Four, from any region whose Elite Four you've beaten |
- In a region whose Elite Four isn't beaten the GLOBAL PC shows "link not established" (Celio's Network Machine is the
  story's reason: its link levels connect the regions).

## Levels per region
- Each Pokémon keeps a level and EXP per region. Its first time in a new region it is Lv 5 there; species, moves,
  nature, IVs, EVs and the Garden record stay. Back in Hoenn it is its Hoenn level again. Kept in a side file next to the
  save and carried in trades. Kai's level scaling uses the team's levels in the current region.

## Co-op
- A room is in its host's region. Joining a host in another region is refused ("This room is in KANTO.").
- The host decides the trip: when the host boards, everyone in the room gets customs and follows.
- Each region has its own co-op shared PC on the server.

## The three card games
POKéMON cards, YU-GI-OH! and TRIPLE TRIAD work in Johto and Kanto as in Hoenn: card players among the trainers (by
class / sprite), CARD CLUBs and Triple Triad sellers in Pokémon Centers, the Yu-Gi-Oh! duelist in marts, Triple
Triad's regions with their own rules for Johto's and Kanto's areas (the CARD QUEEN travels there too), the Game Corners
(Goldenrod, Celadon, Two Island).

## Gen 4
Gen 4 Pokémon only after an Elite Four, outside the region you cleared. Since a game can start in Johto or Kanto, a
Gen 4 Pokémon in those regions' wild tables or trainer teams is swapped for its Gen 1-3 stand-in until the player has
beaten an Elite Four somewhere else (the stand-ins come from the Gen 3 cap of the same rip).

## Build order
1. Engine: region id, FRLG-style 640-tile primaries per layout, 16-bit object graphics ids, flags 0x1000-0x3FFF and
   vars 0x5000-0x51FF (kept in "<save>.region"), the maps imported (tools/quetzal_port/import.py) and walkable.
2. Content: Quetzal's specials and script commands on Kai, trainers, wild encounters, marts, music, hidden items,
   object graphics.
3. Region system: travel, customs, per-region state, the PCs, levels per region.
4. Story: new game in any region, tickets, boat arrival and sightseeing, starters, trains, Route 22; card games.
5. Co-op rules; play-through tests; release.

## After Kanto and Johto: Gens 5-9 and the GENERATION setting (the user's order)
1. Kanto and Johto complete, built and pushed.
2. Pokémon of Gens 5 through 9 (species, moves, abilities, items, types, art, learnsets, evolutions, followers,
   battle effects), on top of Gen 4.
3. A host setting, GENERATION (3-9), chosen each time a room starts hosting (and in OPTIONS for solo play), and
   GIMMICKS beside it, each on or off on its own whatever the generation: MEGA EVOLUTION, Z-MOVES, DYNAMAX /
   GIGANTAMAX, TERASTALLIZATION (and later ones as they come). GEN 3 with every gimmick off is the vanilla game;
   GEN 9 is everything Quetzal has.
   - What the generation decides: wild Pokémon and trainers' teams (later species swapped for their stand-ins, as the
     Gen 4 cap of the rip does), gifts and eggs, the regional and National Dex sizes, items in marts / on the ground /
     as prizes, moves (learnsets, TMs, tutors), abilities, and types (FAIRY only from Gen 6; before it, Pokémon and
     moves take their older typing).
   - A change applies from then on; nothing in the save is deleted or rewritten.
   - Lowering it (say Gen 4 to Gen 3) after catching, using and learning Gen 4 things:
     - Pokémon from past the cap stay yours but are SEALED: they stay in the PC (or in the party, where they can't be
       sent out, like an EGG), can't be traded or shown to others online, and don't evolve. They count in the dex again
       once the cap allows them.
     - Pokémon of an allowed species keep everything, but a move past the cap can't be picked in battle (greyed out
       like a DISABLEd move; STRUGGLE if nothing is left), an ability past the cap has no effect, and a held item past
       the cap does nothing.
     - Items past the cap stay in the bag (marked, not usable, not sold in marts); evolutions that need them wait.
     - Types follow the cap at once: under Gen 6 a FAIRY Pokémon or move uses its older type.
     - Raising the cap again brings it all back as it was.
   - Online the host's setting is the room's: a joining player's sealed Pokémon follow the host's cap.
