# STADIUM's cups: the rules

From the six Bulbapedia pages the user sent (2026-11-06: Challenge Cup, Fancy / Petit Cup, Pika Cup, Poké Cup, Prime
Cup, Gym Leader Castle), Serebii's Stadium pages and the Supplementary Series' Challenge Cup notes. The trainers, teams
and rentals themselves come from the user's ROM (tools/stadium/extract.py); the Challenge Cup's pools, templates and
stat EXP are in tools/stadium/challenge_cup.json.

Every cup: register six (from the PCs or rentals, or both), pick three of them before each battle after seeing the
opponent's six. Eight trainers a division (BATTLE 1-6, SEMI-FINAL, FINAL BATTLE). A battle won without losing a
POKéMON earns a continue. Round 2 opens once MEWTWO is beaten (Round 1 of every cup and the castle first).

| cup | levels | the three's total | other rules | banned |
|---|---|---|---|---|
| PIKA CUP | 15-20 | 50 | | MEWTWO, MEW |
| PETIT CUP | 25-30 | 80 | height 2.0 m or less, weight 20.0 kg or less (the POKéDEX's own), lowest stage only (no pre-evolution among the first 151) | MEW, everything evolved, too tall or too heavy |
| POKé CUP (POKé / GREAT / ULTRA / MASTER BALL) | 50-55 | 155 | so at most four level 55s in the six | MEWTWO, MEW |
| PRIME CUP (POKé / GREAT / ULTRA / MASTER BALL) | 1-100 | none | | none (MEWTWO and MEW allowed) |
| GYM LEADER CASTLE | 50-55 | 155 | 8 gyms of three trainers and the leader, then the ELITE FOUR, then the RIVAL with the same team as against the ELITE FOUR; a new team at each gym | MEWTWO, MEW |
| CHALLENGE CUP (POKé / GREAT / ULTRA / MASTER BALL) | 30 / 45 / 60 / 75 | | six random POKéMON for the player and every opponent, never your own or rentals (below) | the 19 below |

Clauses, explained on screen before each battle:
- **SLEEP CLAUSE**: once one of the opponent's POKéMON is asleep from your move, a sleep move on a second one fails
  (REST on your own side still works).
- **FREEZE CLAUSE**: once one of the opponent's POKéMON is frozen, nothing freezes a second one.
- **SELF-KO CLAUSE**: when the last POKéMON on both sides faint at once from SELFDESTRUCT, EXPLOSION, DESTINY BOND or
  PERISH SONG, the user's side loses.
- **SPECIES CLAUSE**: no two of the same POKéMON in a team.
- **EVENT CLAUSE**: no event POKéMON (MEW) outside the PRIME CUP.
- **ITEM CLAUSE** (CHALLENGE CUP): no two of the same held item.

STADIUM's cups are Generation I's (the user: "No moves outside of Gen 1 will be given or can be used"): only the first
151, only moves 1-165, and no held items (STADIUM's trainers hold none). The CHALLENGE CUP is STADIUM 2's (the user's
special case): its own pools, the moves to 251 and its items.

## The GYM LEADER CASTLE

| gym | trainer 1 | trainer 2 | trainer 3 | leader |
|---|---|---|---|---|
| PEWTER | BUG BOY | LAD | JR.♂ | BROCK |
| CERULEAN | FISHER | JR.♀ | SWIMMER | MISTY |
| VERMILION | SAILOR | ROCKER | OLD MAN | SURGE |
| CELADON | LASS | BEAUTY | COOL♀ | ERIKA |
| FUCHSIA | BIKER | TAMER | JUGGLER | KOGA |
| SAFFRON | CUEBALL | BURGLAR | MEDIUM | SABRINA |
| CINNABAR | JUDOBOY | PSYCHIC | NERD | BLAINE |
| VIRIDIAN | ROCKET | LAB MAN | COOL♂ | GIOVANI |
| ELITE FOUR | LORELEI | BRUNO | AGATHA | LANCE |

Then the RIVAL (seven teams in the ROM: led by VENUSAUR, BLASTOISE, CHARIZARD, VAPOREON, JOLTEON, FLAREON, and one of
their own; the one that best counters the player's six is chosen). Stadium's prize, one of eight at random: BULBASAUR 5,
CHARMANDER 5, SQUIRTLE 5, HITMONLEE 20, HITMONCHAN 20, EEVEE 25, OMANYTE 20, KABUTO 20 (here made far better: docs/STADIUM.md
"Rewards").

## The CHALLENGE CUP

- Classes: POKé BALL level 30 (average BST aimed at 280), GREAT BALL 45 (360), ULTRA BALL 60 (447), MASTER BALL 75 (507).
  Each draws only from its own pool (challenge_cup.json, with Bulbapedia's appearance rates).
- The player's six follow one of six type templates (ELECTRIC / GROUND / WATER, ELECTRIC / FIRE / WATER, ROCK / GRASS /
  WATER, ELECTRIC / GROUND / FLYING, ELECTRIC / GROUND / FIRE, ROCK / FIRE / GRASS) plus three slots of types not yet
  present; each trainer has their own template and favorite moves. The game steers the team's average BST toward the
  class's.
- Moves: the level-up moves to the class's level and Gold / Silver's TMs (Round 1); Round 2 adds Generation I, Crystal's
  tutors and egg moves. Never RETURN, FRUSTRATION, HIDDEN POWER, REST, RECOVER, SLEEP TALK, RAPID SPIN. First move a
  damaging one (STAB first), second a STAB of the second type, third the trainer's favorite or a good status move,
  fourth a coverage move or a status move. DITTO always TRANSFORM; WOBBUFFET always COUNTER, MIRROR COAT, DESTINY BOND,
  SAFEGUARD; SMEARGLE anything but BATON PASS, METRONOME, MIMIC, MIRROR MOVE, SKETCH, SLEEP TALK, STRUGGLE, TRANSFORM.
- Held items: 116/256 a BERRY, 89/256 one that powers the first move, 38/256 KING'S ROCK / SCOPE LENS / BRIGHTPOWDER /
  QUICK CLAW, 13/256 the species' own (LIGHT BALL...); no duplicates.
- DVs and friendship random. The player's stat EXP about 131 a point of BST below 680; the trainers' fixed per trainer
  (more in Round 2).
- Nicknames: the player's "BULBAsaur" (second half lowercase), the opponents' "bulbaSAUR".
- Never: CATERPIE, METAPOD, WEEDLE, KAKUNA, MAGIKARP, ARTICUNO, ZAPDOS, MOLTRES, DRAGONITE, MEWTWO, MEW, UNOWN, RAIKOU,
  ENTEI, SUICUNE, TYRANITAR, LUGIA, HO-OH, CELEBI.
