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

The user's cup options (2026-11-06, verbatim):

```
I believe that for each cup, there should be the ability to turn on or off certain restrictions in order to gain more rewards.

Here are some of the restrictions available.

LEVEL CAP - Any entry Pokemon will be adjusted to the level cap of the Stadium Cup in question, still keeping the same exclusions as uncapped. If turned off, greater rewards are given.
SPECIES - Only species from GEN 1 are allowed. If this setting is turned off, then any Pokemon that is NOT a restricted & Legendary Pokémon, such as Major box-art legendaries and mythicals (e.g., Mewtwo, Groudon, Kyogre, Dialga, Palkia, Zacian, Koraidon, Miraidon) are permitted to enter, with the exception of Cups that would allow all Pokemon in Gen 1.  If turned on, greater rewards are given.
HELD ITEMS - If this setting is on, allows you to have Held Items on entry Pokemon.  If turned off, greater rewards are given.
RENTALS ONLY - if a trainer registers a team of Pokemon consisting only of Rentals and completes a Cup, they will greater rewards.

If every restriction is in place, the player receives great rewards.
```

How it's read (noted for the user): each switch's harder side earns more: LEVEL CAP OFF (the POKéMON must fit the
cup's levels as they are; ON scales them to the cap), SPECIES ON (the first 151 and Generation I moves; OFF opens every
POKéMON but the restricted legendaries and mythicals, and Generation I moves aren't required), HELD ITEMS OFF, RENTALS
ONLY. "Every restriction in place" = all four on their hard side: the greatest rewards. In a cup that allows all of
Generation I (the PRIME CUP, MEWTWO and MEW included) SPECIES OFF also allows the restricted legendaries. The CHALLENGE
CUP's POKéMON are given, so its switches don't apply (its own rewards).

## Status

- T15b (data) done: every STADIUM trainer, team and rental list (src/data/mp_stadium_data.h, tools/stadium/gen_data.py)
  and the CHALLENGE CUP's pools, templates and trainers (src/data/mp_stadium_cc.h, tools/stadium/gen_challenge.py from
  tools/stadium/challenge_cup.json); src/mp/mp_stadium.c builds them (Stadium_BuildMon, StadiumCc_MakeTeam); test
  stadium_data (mp stadium / mp ccteam). The cups' rules: docs/stadium/RULES.md (from the six pages the user sent).
- T15a (BATTLE ISLAND, ticket, travel, the building), T15c (screens), T15d (music, rewards) next.

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
- Conversions (the user's rules, src/mp/mp_stadium.c Stadium_Convert): Gen 1 moves only (move ids 1-165 are the same numbers); DVs to IVs as 2 x DV + 1
  (15 -> 31); stat EXP to EVs; natures that raise the attacking stat the moveset uses (and the stat the user named:
  speed for the fast ones, HP-like bulk for the slow ones go to the EVs, since no nature raises HP).
- Trainer IDs: STADIUM stores 0 for every trainer's and rental's POKéMON; they keep it (00000), with STADIUM's OT names.
- The rival's seven teams (VENUSAUR, BLASTOISE, CHARIZARD, VAPOREON, JOLTEON, FLAREON leads, and a seventh): the one
  that counters the player's six best is chosen (StadiumRival_Pick, with the screens).
- Pictures: STADIUM's classes as KANTO's FireRed pictures (gen_data.py PICS); MEWTWO shows MEWTWO.

## Saved teams (the user, 2026-11-06)

"players must be able to register up to 99 teams per Server, and erase teams as well without prior authorization, copy
teams as well, and be able to use each others."

A cup's options screen has SAVED TEAMS: 99 slots. Online they are the server's (mp_scores.c, store ops MP_ST_TEAM_PUT /
ERASE / GET / LIST, "stadium_teams.dat" where the server runs): everyone on the server sees the same 99, anyone can USE
(the six become the cup's registered team), COPY (to the first free slot), SAVE OVER or ERASE any of them, and the
room hears of each change at once. Offline the save keeps its own 99 ("<save>.stadiumteams"). ERASE asks once on the
player's own screen; nobody else's permission is needed. src/mp/mp_stadium_teams.c; test stadium_teams_online.
