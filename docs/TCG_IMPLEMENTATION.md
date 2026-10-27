# Optional card system: implementation status

Shipped in main 1.27.0 ("Let's Duel!"), reworked in 1.28.0 from the user's feedback list
(every decision: `docs/TCG_DECISIONS.md`). Main builds save the collection in a
`<save>.cards` sidecar (owner = the save's Trainer ID, kept with the save's backups);
dev builds still never write it. Not yet complete: see the end.

## Agreed scope

- Optional FF8-style challenges: with the CARD CASE registered to a button, that button
  in front of someone asks them for a duel (1.28.0; the CO-OP button no longer does).
  Normal interaction unchanged; no floating icons or unsolicited prompts.
- The CARD CLUB member in the POKeMON CENTERs of OLDALE, MAUVILLE and LILYCOVE hands over
  the CARD CASE in dialogue, the first deck, an optional teaching duel and the counter.
- Full original 228-card rules/AI reference, NPC challenges across Hoenn, every win
  earns a booster, no loss penalties or rematch cooldown. No cards from MoeGatcha.
- Collection, multiple decks, deck legality, inspection, unopened packs, practice,
  duplicate exchange and dedicated optional card rewards/banner.
- Smooth card movement; printed card images where appropriate, explicitly custom
  presentation for GB-only cards; keyboard/controller navigation.
- Separate **one-time versioned artwork pack**, confirmed by the user.
- Main builds save the collection with the game (`<save>.cards`, written only when the
  player opted in, never by dev builds). A new game with another Trainer ID starts empty.
- Online card duels between players (explicitly requested; must keep working).

## The duel screen (user's design, docs/user_images/37.webp)

The user sent a DS duel screenshot (both DS screens) as "the full area" and chose:

| Question | Answer |
|---|---|
| Layout | DS arrangement kept: opponent on top, you at the bottom, a Bench row of 5 at each edge, Active cards on the Poke Ball circle, side columns for Prizes / deck / discard |
| Resolution | native 240x160, Emerald fonts and windows |
| Card faces | the printed illustration in a type-colored Game Boy card frame; OPTIONS also offer the GB picture or the whole printed card (later request) |
| Controls | a cursor on the board itself + the original 6-option menu on SELECT |
| Hand | a strip peeking at the bottom that slides up (the mat scrolls up with it so your Bench stays visible), and drops while you pick a target |
| Animation | card-game effects (lunge, shake, -40, HP plate counting down, WEAKNESS x2 / RESISTANCE -30 banners, KO, every card sliding between zones) |
| Sound | Emerald sound effects + duel music (MUS_B_ARENA), victory music |
| Labels | the original GB TCG words: BENCH, PRIZES, DECK, DISCARD, END TURN, WEAKNESS x2 |
| Also polished | booster opening, card inspector, Card Case and deck builder, coin flips and turn banners |
| Rules bugs | fixed and listed (below) |

Also asked: controller button icons everywhere (the game's own icons for the pad used
last, GBA keys on a keyboard), and online duels between players must keep working.

## 1.28.0: the rework

- **The card screen is the game picture.** Every card screen draws into a 240x160 canvas
  that display.c shows in place of the GBA picture (`Tcg_Canvas`: same rect, scale mode and
  filter), so it fills the window with the game's own scaling, and `Tcg_Frame` hands it to
  `Platform_GetFrame` so SPECTATE panels and photos show the duel. The online opponent is
  left out of the stream (`Tcg_HiddenFrom`: they'd see your hand). Friends see a
  "CARD DUEL" line on the nametag (MpState.cardDuel, bit 7 of the light byte; protocol 52).
- **Board**: the mat is drawn at the layer's 240x200 (it was stretched from 240x160: the
  oval Poke Ball and the zones off their cards), a round ring, card spots under every zone,
  one BENCH label, Prizes as overlapping 2x3 stacks. Mats per opponent: `TCG_MAT_*` themes
  (classic, trainer kinds, gym by type, Elite Four, Team Magma / Aqua, Frontier, online).
  The opponent's cards are turned toward them. B leaves the hand, then asks "End your
  turn?" (it never concedes); the END TURN button is CONCEDE, with a confirm. A drawn
  Prize card is held up in the middle ("PRIZE CARD"); Trainers that change the hand
  (PROFESSOR OAK, BILL...) slide the hand up; after a Knock Out the cursor waits on the
  Bench. Tips in the first duels (`TcgBoard_Tutorial`).
- **Card art** (`tcg_gfx.c`, `G_SetArtMode` / `G_SetBackMode`): OPTIONS > DISPLAY > CARD ART
  (`card_art` in the ini) is PRINTED ART (the printed illustration in the Game Boy frame, the
  default), GAME BOY (the Game Boy picture in it) or FULL CARD (the whole printed card); CARD
  BACKS (`card_back`) is PRINTED or PIXEL. All of it is built into the exe: the Game Boy
  pictures (`graphics/tcg/original.bin`) and 209 printed cards plus the back
  (`graphics/tcg/printed.bin`, 360x495 JPEGs with each illustration's rectangle, ~9 MB). The
  19 cards the Game Boy game invented have no printing and keep the Game Boy picture. The
  illustration rectangles were measured per layout (Base / Jungle / Fossil / promo Pokemon,
  Base / Jungle / Fossil Trainers, Energy): each layout's cards sit within a couple of pixels,
  so one rectangle per layout (the inner edge of the window's bevel, found on the average of
  its cards) with a 3-pixel inset; Energy frames the symbol. The frame's picture box is 4:3,
  a printed Trainer's about 9:5 (its window is ~1.85:1). See `tools/tcg/build_printed_art.py`
  and `tools/tcg/printed_art.json`; `tools/tcg/test_art.sh` draws every mode.
- **Who plays** (`src/tcg/tcg_field.c`): people by look (`sPeople`: category, deck type,
  label), decks by type (`sPools`; with CARD SCALING off, by badge tier: badges 0-1 / 2-3 /
  4-6 / 7-8; see CARD SCALING below). Trainers
  play once beaten, townsfolk 65% of the time (a hash of map + person), CARD CLUB staff
  always. 38 lines in the challengers' words (7 kinds x 3 yes + 2 no, staff, "battle me
  first"). Their sprite becomes the challenge window's portrait (`CapturePortrait`).
- **Special decks**: 24 trainers with decks of their own (`tools/tcg/gen_special_decks.py`
  -> `src/tcg/tcg_decks.c`): the 8 Gym Leaders (after their badge), the Elite Four (after
  beating them), WALLACE (Champion), STEVEN, the rival, WALLY, MAXIE, ARCHIE and the six
  Frontier Brains. 15 hand over a promo card on the first win (`gTcgSpecialPromos`).
- **Packs** (1.32.0, `Tcg_OpenPack`): 5 commons, 3 uncommons, a rare and a Basic Energy. The
  rare is a holo (`Tcg_IsHolo`: the rares WotC printed as holofoils, Base Set / Jungle 1-16,
  Fossil 1-15, 45 cards) one pack in `TCG_HOLO_ODDS` (3), a plain rare otherwise; after
  `TCG_HOLO_PITY` (20) packs of a set without a holo the player didn't have
  (`TcgCollection.drought`, collection format 5) the next one brings one. Holos are never
  picked with CARD POINTS or as a win's card. Simulated (tools/tcg/pack_math.py, one 16-rare set,
  spares traded in): a set takes ~55 won packs (the old rules ~19 with CARD POINTS buying the
  last cards, WotC's ~91), the last holo ~11 more packs and never more than 20 (WotC: ~22,
  1 in 10 players 73+). The reveal marks NEW cards, HOLO! with a foil sweep, and
  "SET COMPLETE!" with a fanfare.
- **Rewards**: every win, a booster of the opponent's pack; once a day per opponent, one
  card of their deck, your pick, holos aside (`Tcg_DailyAvailable` / `Tcg_DailyTake`, 64 slots,
  the oldest replaced). CARD POINTS for spare copies at the counter (`Tcg_PointValue` 1/2/6
  by rarity, a holo 20, a promo 20; `Tcg_PointCost` 5/15/80, holos and promos not sold), which
  sells boosters (40), cards,
  RARE CANDY, PP UP and MOEGATCHA TICKETs. 75% of a set gives that set's milestone promo;
  the whole collection gives a PIKACHU Lv.25 (THUNDERBOLT, FLY, SURF, AGILITY, LIGHT BALL,
  PREMIER BALL, OT PROMO) at the CARD CLUB and a star on the Trainer Card.
  Achievements: the CARDS tab (`mp_achieve.c`, `card*`). Collection file format 3
  (format 2 still loads).
- **Friends** (`src/tcg/tcg_online.c`, include/tcg/online.h): CO-OP > FRIENDS > a friend >
  CARD DUEL / CARD TRADE, or the CARD CASE button in front of them. A request (social
  messages 0x10+ on extension 255) shows them YES / NO on the map; busy and no-case answers
  are automatic; 30 seconds; B asks "Stop waiting for NAME?" (NO first: B moves text along
  everywhere else, and a tap that withdrew at once was what friends online kept seeing as
  "NAME stopped waiting."). Two friends asking each other at once: the one later in the room
  says yes to the other's request (Tcg_SocialReceive). Tests tcg_ask_field, tcg_ask_menu,
  tcg_ask_both (regress.sh, also at WORST=1 lag). YES opens the deck pick on both sides (the
  winner gets a booster) or the trade screen (each offers a card, both confirm).
- **Menus** (`src/tcg/tcg_ui.c`, rewritten in the BAG's style): home, decks, a grid deck
  builder (5x2 cards, the deck list on the right; A adds, SELECT takes one out, RIGHT past
  the last column goes to the list, A there takes out, LEFT returns to the card it points
  at; the decks screen's SELECT deletes a deck, and deleting the one in use hands over to
  the next playable deck), binder, inspector, packs, the challenge window, spoils, the counter, the
  three-deck reveal, trades.

## Code map

- `src/tcg/tcg_duel.c`, `tcg_effects.c`, `tcg_trainers.c`, `tcg_ai.c`: rules engine
  (pure, deterministic, transactional `TcgDuel_Apply`). Every action also records
  **presentation events** (`struct TcgEvent` ring in `TcgDuel`: coin, damage with
  Weakness/Resistance/poison/self flags, heal, status, KO, prize, Trainer, attach,
  evolve, switch, turn, win...). Events never name a hidden card; they are not hashed.
- `src/tcg/tcg_board.c`: the duel screen. It keeps a *shown* copy of the duel and a
  queue of newer states; for each state it plays the events that need the old board
  (attack, coin, damage, KO, Trainer showcase), commits (every physical card's sprite
  slides to its new zone: deck, hand, field, discard, prizes), then plays the rest
  (turn banner, evolve sparkle, win). The AI waits for `TcgBoard_Idle`. Board cursor
  (geometric navigation), context menus, attack panel, target choice with the held
  card floating over the space, Trainer choice flows (`TrainerSteps`: Bench/Active
  targets and card pickers for hand, searched deck, discard, attached Energy, Pokedex
  order), retreat Energy choice, promotion and Whirlwind-style switch choice, SELECT
  quick menu, concede confirm, results.
- `src/tcg/tcg_gfx.c`: 240x160 primitives, the Emerald fonts (normal/small/narrow)
  and window frames, card faces (scan box-filtered down, or the procedural frame
  around the GB art), card backs, LCD-style HP plates, energy orbs, status tags,
  buttons, banners, coin, controller button icons (`G_Hint` draws `{A}`, `{START}`,
  `{L}{R}`... inline from mp_buttons.c's art).
- `src/tcg/tcg_ui.c`: the other screens (invite, starter, Card Case, collection
  binder with type tabs, deck builder in the original GB style, deck slots, booster
  packs with tear/flip/rare shine, inspector, big art) and `Tcg_Debug` harness hooks.
- `src/tcg/tcg_link.c`: online duels (see below).
- `src/tcg/tcg_integration.c` / `tcg_collection.c`: the game hooks (CARD CASE item 399,
  the canvas, spectate, relay dispatch) and the collection file (format 3 with an owner).
- `src/tcg/tcg_field.c`: challenges on the map, special trainers, CARD CLUB specials
  (data/scripts/card_club.inc). `src/tcg/tcg_online.c`: friends' requests and trades.
- `src/tcg/tcg_art_data.c`: the GB illustrations embedded from `graphics/tcg/original.bin`
  (`tools/tcg/build_original_art.py`; `tools/tcg/test.sh` checks it is up to date).

## Rules fixes in this pass (audit of the first pass)

- Retreat only once per turn (was unlimited).
- Damage order follows the original `ApplyDamageModifiers_DamageToTarget`: double-
  damage substatus, Weakness x2, Resistance -30, PlusPower, Defender, then the
  defender's protection and the attacker's weakening; PlusPower still counts when
  Resistance brought damage to 0 (it used to be skipped), never adds to a 0-damage
  attack. Confusion damage to itself uses the attacker's own Weakness/Resistance.
- Agility (Fearow, Raichu, Rapidash, Seadra) now prevents every effect of attacks,
  not only damage; Hide in Shell and Exeggcute's Leech Seed are mapped.
- Opponent AI: picks attacks by damage after Weakness/Resistance and Knock Outs, picks
  targets for attacks that need them, checks every attack is legal before using it.
- Online: a concession crossing the other player's move on the wire used to end the
  session as "out of order" (the old 250 ms lag test passed only by timing); the
  receiver now drops its own unacknowledged move first so both clients agree.

## Attack effects

`tools/tcg/generate_effects.py` maps reviewed original effect functions (single-
function lists) and, new in this pass, whole command lists ported by hand from
`effect_functions.asm` (pret/poketcg 2d81ec3): Ember / Flamethrower / Fire Blast /
Fire Spin / Thunderbolt / Flames of Rage (Energy discards), Recover, Mewtwo's Barrier,
recoil (Take Down, Submission, Double-edge, Thunder Jolt, Thunder, Thrash,
Thunderpunch), coin-or-nothing (Horn Hazard, Dive Bomb, Leek Slap once per duel,
Clamp, Fly), Fetch / Pay Day, First Aid / Spacing Out, Mega Drain / Absorb / Leech Life
(damage done), Foul Odor, Petal Dance, Foul Gas, Venom Powder, Tantrum, Rampage, Flail,
Big Eggsplosion, Stone Barrage, Boyfriends, Dancing Embers, Sonicboom, Dream Eater,
Selfdestruct, Earthquake, Blizzard, Thunderstorm, Chain Lightning, Cat Punch / Slicing
Wind / Ice Breath / Big Thunder (random targets), Spark / Dark Mind / Stretch Kick
(chosen Bench target), Gigashock, Lure, Teleport, Whirlwind / Terror Strike / Ram (the
defender's owner chooses), Minimize / Harden / Light Screen / Expand, Growl / Pounce /
Snivel / Smokescreen / Sand-attack, Leer / Tail Wag / Bone Attack, Acid, Headache,
Swords Dance / Focus Energy, Call for Family / Sprout / Call for Friend, Friendship
Song, Energy Conversion / Absorption / Spike, Hyper Beam / Whirlpool.

Coverage: 229 command lists (was 118). Of 319 attacks, 20 remain unported (was 131):
Metronome, Mirror Move, Amnesia, Conversion 1/2, Morph, Mystery Attack, Wail, Prophecy,
Mix-Up, Magnetic Storm, Wildfire, Hurricane, Gale, Devolution Beam, Destiny Bond and
Scavenge; they show as "not ready" in the attack panel. Simplifications to revisit: where the
original asks the *attacker* to pick cards (Fire Spin's 2 Energy, Hyper Beam's Energy,
Gigashock's 3 targets, Call for Friend's card, Energy Spike's Energy and Pokemon,
Energy Conversion / Absorption's cards) the engine picks (documented in
tcg_effects.c).

## The effects audit (1.29.0)

Every attack effect and Trainer was read against the card text and the original rulings.
Fixed: Kangaskhan's Fetch looked like both players drew (the next turn's draw slid in with
it: the board now shows a new turn as a step of its own, banner then draw); Zubat / Golbat's
Leech Life heals the damage done, not 10; Fury Attack is 2 coins; Psywave / Psychic count
Energy *cards* (a Double Colorless is one); Water Gun / Hydro Pump count only the Water not
needed for the cost (Colorless paid with other Energy first, +20 at most); Big Thunder picks
evenly among every Pokemon but Zapdos; Growl, Pounce, Snivel, Leer and Tail Wag are bound to
the Pokemon that used them (`effectBy`: they end if it leaves the Active spot); Ram switches
even when Rhydon is Knocked Out (and not when the Defender is); Full Heal only cures
conditions (Withdraw, Barrier, Agility stay; Cure-like effects still clear them); Energy
Retrieval / Super Energy Retrieval retrieve first and can't pay with the retrieved cards;
Devolution Spray no longer makes the Pokemon "new" (it can evolve again this turn); Clefairy
Doll / Mysterious Fossil can be discarded from play (DISCARD on its menu, no Prize). Self
damage (confusion, recoil) still applies the attacker's own Weakness / Resistance, as the
Game Boy game and the 1999 rules did. Tests: `tools/tcg/test_powers.c` (staged boards).

## Pokemon Powers (src/tcg/tcg_powers.c)

All 26, by their card text: `TcgPower_Working` (none while Muk's Toxic Gas is in play, Muk's
own excepted, or while its Pokemon is the Active and Asleep / Confused / Paralyzed).
Always on: Toxic Gas, Clairvoyance (the opponent's hand face up: A on their hand), Strikes
Back, Kabuto Armor, Prehistoric Power (no evolving, Breeder included), Transparency,
Invisible Wall, Neutralizing Shield, Retreat Aid (the retreat menus count it), Thick
Skinned. Into play (`TcgPower_OnPlay`, the owner's turn, not during setup): Firegiver,
Quickfreeze, Peal of Thunder, Healing Wind; the board shows them after the card lands.
Used (`TCG_USE_POWER`, slot = the Pokemon; target / attack = its choices, see
`TcgDuel_Power`): Solar Power, Energy Trans, Heal, Shift, Energy Burn, Rain Dance,
Cowardice, Peek, Damage Swap, Strange Behavior, Curse, Step In; once a turn where the card
says so (`powerUsed`). `TcgPower_Block` gives the reason one can't be used (the menu's
grey line). On the table: PKMN POWER on the Pokemon's menu (or SELECT > PKMN POWER, the
first usable one), the choices as target steps (`PowerSteps`) or menus (SHIFT's types,
PEEK's five places); `TCG_EV_POWER` puts the name over the card, `TCG_EV_PEEK` shows the
card to its player only. The AI uses Solar Power, Heal, Rain Dance, Energy Trans, Damage
Swap, Strange Behavior, Curse (for a Knock Out), Energy Burn (when it enables an attack)
and Step In (`PowerAction`).

## CARD SCALING (src/tcg/tcg_scale.c, include/tcg/scale.h)

Deck strength is an Elo rating measured by AI-vs-AI duels: `tools/tcg/rate_decks.sh`
(about a minute; `tools/tcg/test.sh` checks it's current) plays every Game Boy and
special deck round robin, fits Bradley-Terry ratings (mean 1500) and rates each special
variant against the field, into `src/tcg/tcg_ratings.c`. In game, the player's deck is
rated against 12 reference decks x 6 games (`TcgScale_RateDeck`, cached by deck hash),
plus a skill correction learned from every duel against a card player (`skill`, saved
in the .cards file, format 4: K 48 for the first 20 duels, then 24, within +-300).
Regular card players take the deck of their type pool closest to the player + 25 (capped
by badges, `sBadgeCap`); special trainers their own deck or one of their variants (their
signature lines and promo on the other same-type Game Boy decks), closest to the player +
75. The challenge window's MATCH line: EASY / EVEN / TOUGH / VERY TOUGH from the expected
score. OPTIONS > GAME > CARD SCALING OFF goes back to the badge tiers. The log line
`[tcg] scaling on: you N (deck D, skill S), target T, ...` shows each choice.

## Online card duels

One player asks a friend in the same co-op room (CO-OP > FRIENDS or the CARD CASE
button in front of them); YES opens the deck pick on both sides. Nothing is wagered; the
winner gets a booster. Relay extension 255, card protocol **4** (bump `TCG_LINK_VERSION`
on any rules, catalogue or wire change: protocol 2 added retreat-once, 3 the substatus
fields, 4 the social layer: request, reply, withdraw, trade offer / confirm / cancel,
5 the effects audit and Pokemon Powers).
The inviter is canonical player 0; the duel screen always shows the local player at
the bottom (`OnlineView` swaps both sides and every event). Both peers validate decks
and every action, with sequence numbers, before/after rules hashes and an ACK before
the next local action. Timeout/disconnect returns to the Card Case with no result.
This is deterministic peer replication (both processes hold the whole duel): the
interface never reveals hidden cards, but modified clients could.

## Tests

```
tools/tcg/test.sh                       # rules, effects, Trainers, Powers, AI, link (ASan/UBSan),
                                        # generated decks and ratings up to date
NOBUILD=1 tools/tcg/test_ui.sh          # real keys: CARD CLUB, challenge with the registered
                                        # CARD CASE, end turn / concede, deck builder, counter,
                                        # Pokemon Powers on a staged board
NOBUILD=1 tools/tcg/test_online.sh [250]        # two clients, request + YES, raw link actions (+lag)
NOBUILD=1 tools/tcg/test_online_board.sh [250]  # two clients, whole duel via both duel screens, booster
NOBUILD=1 tools/tcg/test_trade.sh       # two clients trade a card
NOBUILD=1 tools/tcg/test_art.sh         # CARD ART's three looks and CARD BACKS: PIXEL draw different cards
NOBUILD=1 tools/tcg/test_save.sh        # main: saved with the game, a new Trainer ID starts empty
NOBUILD=1 tools/test/dev_save_test.sh   # dev builds stay read-only
```

Harness hooks (`mp cards ...`): `open`, `starter N`, `ready`, `fast`, `auto` (the AI
plays your seat through the board, online too), `mine`, `give ID [SLOT]`, `focus KIND
INDEX` (0 hand, 1 yours, 2 theirs, 4 your discard, 10 CONCEDE; index 99 = last hand
card), `state`, `snapshot`, `netstate`, `netstep`, `netconcede`, `challenge` (the person
in front), `special N`, `case`, `reveal`, `club`, `lesson`, `builder`, `deck N`, `points N`,
`own ID N`, `giveall`, `win` (the opponent concedes), staging a board: `clear` (every
Pokemon in play back into its deck), `put WHO SLOT ID`, `attach WHO SLOT ID N`, `hand ID`
(a basic Energy card in the deck becomes the card if none is left), `damage WHO SLOT N`;
`skill N` / `rate` (CARD SCALING), `ask PLAYER 0|1` (duel / trade),
`offer ID`, `confirm`. `mp register ITEM` registers a key item to SELECT. `rendershotburst
PREFIX FRAMES STEP` captures the window output (overlays included) every STEP frames;
the board prints `[tcg] show event ...` for KOs, coins, status, promotions and wins.

## Still to do

- The 20 remaining attack effects; player choice where the engine picks.
- Trainer flows exist for all 34 Trainers but deserve play-testing per card.
- Emerald attack animation adapter (not used: the board has its own card effects).
- Windows: the mingw build compiles; not run on Windows.

## Commands and local assets

```
python3 tools/tcg/import_poketcg.py /tmp/moemon-poketcg-reference
python3 tools/tcg/generate_catalog.py
python3 tools/tcg/generate_effects.py
python3 tools/tcg/fetch_art.py           # match every card to its printing, download the scans and the back
python3 tools/tcg/build_printed_art.py   # -> graphics/tcg/printed.bin (committed)
python3 tools/tcg/build_original_art.py  # -> graphics/tcg/original.bin (committed)
```

The full scans (`graphics/tcg/printed/`, 600x825 PNGs from PokemonTCG/pokemon-tcg-data's
links; the back from tcg.pokemon.com) and the metadata cache are ignored;
`tools/tcg/art_manifest.json` records every match with its hash. Jungle / Fossil rares use
the holo printing; a few levels the Game Boy game changed are matched by name, HP, attacks
and Pokemon Powers.

## Layout references

pret/poketcg (pinned revision): `src/engine/menus/deck_configuration.asm` for the deck
builder (type filters, in-deck/owned counts, LEFT/RIGHT remove/add, START checks the
deck), `src/engine/duel/core.asm` DuelMenuData for the SELECT quick menu (Hand /
Attack / Check / PKMN Power / Retreat / Done), `effect_functions.asm`, `duel.asm` and
`substatus.asm` for effect semantics and the damage order.
