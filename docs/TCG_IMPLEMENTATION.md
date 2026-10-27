# Optional card system: implementation status

Shipped in main 1.27.0 ("Let's Duel!"), no longer gated by `MOEMON_DEV_BUILD`. Main
builds save the collection in a `<save>.cards` sidecar (owner = the save's Trainer ID, kept
with the save's backups); dev builds still never write it. Not yet complete: see the end.

## Agreed scope

- Optional FF8-style challenge of participating NPCs with Triangle / configurable
  equivalent. Normal interaction unchanged; no floating icons or unsolicited prompts.
- First valid challenge offers opt-in, Card Case, starter choice and optional tutorial.
  Declining leaves the system inactive and never removes an ordinary reward.
- Full original 228-card rules/AI reference, NPC challenges across Hoenn, every win
  earns a booster, no loss penalties or rematch cooldown.
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
| Card faces | the printed scan when the art pack is installed, else a type-colored card frame around the GB illustration |
| Controls | a cursor on the board itself + the original 6-option menu on SELECT |
| Hand | a strip peeking at the bottom that slides up (the mat scrolls up with it so your Bench stays visible), and drops while you pick a target |
| Animation | card-game effects (lunge, shake, -40, HP plate counting down, WEAKNESS x2 / RESISTANCE -30 banners, KO, every card sliding between zones) |
| Sound | Emerald sound effects + duel music (MUS_B_ARENA), victory music |
| Labels | the original GB TCG words: BENCH, PRIZES, DECK, DISCARD, END TURN, WEAKNESS x2 |
| Also polished | booster opening, card inspector, Card Case and deck builder, coin flips and turn banners |
| Rules bugs | fixed and listed (below) |

Also asked: controller button icons everywhere (the game's own icons for the pad used
last, GBA keys on a keyboard), and online duels between players must keep working.

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
  CO-OP button challenges) and the collection file (format 2 with an owner).
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
tcg_effects.c). Pokemon Powers are still unported (POWER shows "not ready").

## Online card duels

Both players open CARD CASE > ONLINE CARDS in the same co-op room; one invites, the
other accepts. Each uses their active validated deck; nothing is wagered or awarded.
Relay extension 255, card protocol **3** (bump `TCG_LINK_VERSION` on any rules,
catalogue or wire change: protocol 2 added retreat-once, 3 the substatus fields).
The inviter is canonical player 0; the duel screen always shows the local player at
the bottom (`OnlineView` swaps both sides and every event). Both peers validate decks
and every action, with sequence numbers, before/after rules hashes and an ACK before
the next local action. Timeout/disconnect returns to the Card Case with no result.
This is deterministic peer replication (both processes hold the whole duel): the
interface never reveals hidden cards, but modified clients could.

## Tests

```
tools/tcg/test.sh                       # rules, effects, Trainers, AI, link (ASan/UBSan)
NOBUILD=1 tools/tcg/test_ui.sh          # real keys: opt-in, deck builder, field challenge,
                                        # board setup/inspect/discard/quick menu/attach
NOBUILD=1 tools/tcg/test_online.sh [250]        # two clients, raw link actions (+lag)
NOBUILD=1 tools/tcg/test_online_board.sh [250]  # two clients, whole duel via both duel screens
NOBUILD=1 tools/tcg/test_save.sh        # main: saved with the game, a new Trainer ID starts empty
NOBUILD=1 tools/test/dev_save_test.sh   # dev builds stay read-only
```

Harness hooks (`mp cards ...`): `open`, `starter N`, `ready`, `fast`, `auto` (the AI
plays your seat through the board, online too), `mine`, `give ID [SLOT]`, `focus KIND
INDEX` (0 hand, 1 yours, 2 theirs, 4 your discard, 10 END TURN; index 99 = last hand
card), `state`, `snapshot`, `netstate`, `netstep`, `netconcede`. `rendershotburst
PREFIX FRAMES STEP` captures the window output (overlays included) every STEP frames;
the board prints `[tcg] show event ...` for KOs, coins, status, promotions and wins.

## Still to do

- Remaining attack effects and all Pokemon Powers; player choice where the engine picks.
- Trainer flows exist for all 34 Trainers but deserve play-testing per card.
- NPC definitions across Hoenn, victory rewards and card-only MoeGatcha, tutorial.
- Artwork pack: resolve remaining mappings, holo variants and GB-only cards;
  installation/versioning/checksums. Without the pack the GB art frames are used.
- Emerald attack animation adapter (not used: the board has its own card effects).
- Windows: the mingw build compiles; not run on Windows.

## Commands and local assets

```
python3 tools/tcg/import_poketcg.py /tmp/moemon-poketcg-reference
python3 tools/tcg/generate_catalog.py
python3 tools/tcg/generate_effects.py
python3 tools/tcg/fetch_art.py
python3 tools/tcg/benchmark_art.py
python3 tools/tcg/build_art_pack.py
```

Original full scans under `graphics/tcg/printed/` and the metadata download cache are
ignored; the compressed content pack is in `dev/artwork/tcg-art/` (ID/hash in
`tools/tcg/art_pack.json`). `MOEMON_TCG_ART_DIR` points the game at a pack.

## Layout references

pret/poketcg (pinned revision): `src/engine/menus/deck_configuration.asm` for the deck
builder (type filters, in-deck/owned counts, LEFT/RIGHT remove/add, START checks the
deck), `src/engine/duel/core.asm` DuelMenuData for the SELECT quick menu (Hand /
Attack / Check / PKMN Power / Retreat / Done), `effect_functions.asm`, `duel.asm` and
`substatus.asm` for effect semantics and the damage order.
