# POKéMON STADIUM on BATTLE ISLAND

The user's request (2026-11-05, verbatim; the work list is T15a-T15d). Decisions and status below it.

```
POKEMON STADIUM
Using Pokémon Stadium's decomp, we will port the Stadium modes into Moemon Emerald.
A special ticket to BATTLE ISLAND will be given after defeating the Elite 4 in any region. BATTLE ISLAND can be traveled to from any region as well.
Upon travel to BATTLE ISLAND, the player will happen across a giant building, where entering takes them into a copy of MOEMON EMERALD's Elite Four Pokémon Center.
The NURSE NPC will give information about prior results of Cups, and current champions, if any.
The MART NPC will explain the STADIUM, its rules, and the basics.
Upon entering, the player will be given the selection of different Stadiums to enter and participate in.
If a player doesn't already have a ticket to BATTLE ISLAND upon loading a save, they will be given one by an NPC that fits the region they are currently in.
https://github.com/pret/pokestadium
CORE FEATURES:

* Battle system is unchanged from Kai - it will use Kai's battle system.
* Every cup from Pokémon Stadium.
* Register teams for use in the Stadium, exactly how Pokémon Stadium does it. Stadium's menu is referenced, but the style will be the style of Moemon Emerald.
* View and select Pokémon from all available boxes, including the PERSONAL, SHARED, and GLOBAL PC to create a team of up to 6, or use Rental Pokémon, defined by the original game, or a mixture of both. The manner of selection is Pokémon Stadium's.
* Every rental Pokémon available from said cups.
* Special Stadium and Cup rules apply. Level restrictions on Pokémon apply.
* The rules for special cups like the Petit Cup will still be followed exactly, referencing Pokedex height and weight.
* Rules such as special clauses will be explained on the screen prior to battle.
* Graphics for the STADIUM SELECTION will be taken from Stadium directly, and upon entering POKEMON STADIUM, will be the only prompt available, other than to exit.
* Trainer graphics, pre-battle, and Pokémon selection will work identically to Pokémon Stadium, with Kai's graphic style and Moemon graphics. No Stadium 3D models will be used.
* All music will be Pokémon Stadium's music, converted to Kaisong format.
* Teams saved to the Stadium will remain saved, and be usable by other players as well who have played on the server.

The following Cups will be available, with their Poke Ball, Great Ball, Ultra Ball, and Master Ball variations.
Petit Cup
Pika Cup
Poké Cup
Prime Cup
Challenge Cup
Challenge cup has extremely special rules. The Challenge Cup will ONLY use the Pokémon provided in Stadium 2, as a special case. Please refer to https://bulbapedia.bulbagarden.net/wiki/Challenge_Cup for specific implementation.
Gym Leader Tower will be available as an additional mode, with R1 and R2 available.
All teams, movesets, TRAINER names, TRAINER IDs, and Pokémon nicknames will be identical to their Pokémon Stadium counterparts. Natures given to the Pokémon will specifically compliment their primary attack type, and an additional stat that benefits them, such as speed for Persian, or HP for Snorlax. IVs and DVs will be converted to their closest equivalent to Gen 3. This counts for Rentals as well as TRAINER Pokémon. No moves outside of Gen 1 will be given or can be used.
Achievements will be given for every step of Pokémon Stadium cups that are completed, with great rewards upon full completion. The rewards for beating the Stadiums must be several orders of magnitude better than the original rewards, such as perfect IV Pokémon. A special Stadium ribbon will be given to Pokémon who complete different tiers of the Stadiums. Exceptional rewards will be given for completions using only Rental Pokémon.
```

## Status

- T15a (BATTLE ISLAND, ticket, travel, the building) and T15b (data) under way; T15c (screens), T15d (music,
  rewards) next.

## The data (T15b)

- The user's ROM and soundtrack (2026-11-06): https://drive.google.com/file/d/1BGhq3Tyo2oeNhAfaM4nZQl-FR6dzlbc7
  ("ROM and Music.zip": Pokemon Stadium (USA) 1.0 / Rev 1 / Rev 2, the soundtrack as USF rips of NUS-CP2J = the same
  game). Kept in ~/.cache/moemon/stadium_dl, never committed or shipped. The 1.0 ROM is pret/pokestadium's
  (md5 ed1378bc12115f71209a77844965ba50).
- tools/stadium/extract.py ROM OUT.json reads every trainer and rental: the table of tables at 0x898000 (Data Crystal's
  ROM map), 62 blocks: Round 1 / Round 2 of the Petit, Pika, Prime (POKé / GREAT / ULTRA / MASTER BALL) and Poké
  (the same four) Cups, the Gym Leader Castle (8 gyms of 3 trainers + the leader, the Elite Four, the rival), Mewtwo,
  Battle Now, and the rental lists (Petit 45, Pika 91, Poké / Castle 149, Prime 149 / 150). Record formats in the
  script's header. Trainer names are the game's (JR.♂ / COOL♀ are stored with ¾ / © for the symbols).
- The Challenge Cup is Stadium 2's (the user's rule): pret/pokestadium oldnotes/stadiumgs/rosters/challengecup.
- Conversions (the user's rules): Gen 1 moves only (move ids 1-165 are the same numbers); DVs to IVs as 2 x DV + 1
  (15 -> 31); stat EXP to EVs; natures that raise the attacking stat the moveset uses (and the stat the user named:
  speed for the fast ones, HP-like bulk for the slow ones go to the EVs, since no nature raises HP).
