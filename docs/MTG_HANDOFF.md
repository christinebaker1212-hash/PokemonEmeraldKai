# Magic: The Gathering: handoff

The user's decisions: `docs/MTG_DECISIONS.md` (keep to them). How much of the pool plays: `docs/MTG_SCOPE.md`
(generated: `tools/mtg/scope.py`). This file: what is built, how it is built, what comes next.

## Status (2026-11-04: in the game, 1.95.0)
- **1.96.0: the presentation pass** (below): printed art, faces, every screen and the table like the other games'.
- **Playable in the game at MOSSDEEP's CARD SHOP** (COOP_NOTES "MAGIC"): the clerk's starter decks, six regulars
  upstairs and down, the CARD CASE's MAGIC side (hidden until a deck), decks and the builder, the BINDER, boosters
  for wins, the duel table. Scenarios mtg_shop, mtg_play. Not yet: the other venues (CIANWOOD, SAFFRON, SINNOH's
  PAL PARK), Magic players out in the world, booster drafts with friends, online duels, card art, HOW TO PLAY pages
  in tcg_guide.c (done in 1.96.0), the phone app.

## The first pass (2026-11-04)
- **The engine is Forge translated into C** (the user: "the best of both worlds", Forge's AI too). No Java.
  It reads Forge's own card scripts for the pool only (Premodern: 5408 cards with the 33 banned ones).
- **798 of the 5408 cards play**, by the rules below. A card the engine doesn't fully understand is marked
  unsupported (with the first missing piece) and is never put in a duel, so nothing plays wrong.
- Tests: `tools/mtg/test.sh` (132 checks: set-up tables for each rule, and 40 whole duels between the stand-in AIs:
  every one ends, and a memcpy copy taken mid-duel plays exactly the same duel as the original).
- (Since 1.95.0 the game around it is built: see Status above and COOP_NOTES "MAGIC".)

## Files
- `tools/mtg/fetch_pool.py` -> `tools/mtg/pool.json`: the pool from Scryfall (`format:premodern`, plus
  `banned:premodern` marked `"banned": true`).
- `tools/mtg/fetch_forge.sh [DIR]`: Card-Forge/forge at the pin (`tools/mtg/forge_commit.txt`), sparse: the card
  scripts (forge-gui/res/cardsfolder) and the Java to translate (forge-game, forge-ai, forge-core). Default
  ~/.cache/moemon/forge.
- `tools/mtg/build_pack.py FORGE_DIR` -> `graphics/mtg/forge.pak` (committed, 3.1 MB, like edo.pak): every pool card's
  script, sorted by name. Format in the script's docstring. Refuses a Forge that isn't at the pin.
- `include/mtg/mtg.h`: the public API. `src/mtg/mtg_internal.h`: the engine's own.
- `src/mtg/mtg_script.c`: the scripts into card definitions (CardRules / CardFactory / AbilityFactory).
- `src/mtg/mtg_valid.c`: "Valid" text (CardProperty / PlayerProperty).
- `src/mtg/mtg_mana.c`: mana sources and auto-payment (ManaPool, ComputerUtilMana).
- `src/mtg/mtg_game.c`: the turn, priority, the stack, zones, state-based actions, combat (PhaseHandler,
  MagicStack, GameAction, Combat, CombatUtil).
- `src/mtg/mtg_effects.c`: the effects (forge/game/ability/effects) and what each understands.
- `src/mtg/mtg_ai.c`: the STAND-IN AI, only until forge-ai is translated.
- `include/mtg/collection.h`, `src/mtg/mtg_collection.c`: the save's cards and decks ("<save>.mtg"), boosters.
- `src/mtg/mtg_decks.c`: the five starters and the six MOSSDEEP regulars' decks (tools/mtg/test.sh checks every
  card is playable).
- `include/mtg/integration.h`, `src/mtg/mtg_integration.c` (the pack, the sidecar, specials, `mp mtg`),
  `src/mtg/mtg_pak.c` (the .incbin), `src/mtg/mtg_text.c` (card words in the font's letters), `src/mtg/mtg_ui.c`
  (the screens), `src/mtg/mtg_board.c` (the table), `data/scripts/mtg_shop.inc` (MOSSDEEP).
- `tools/mtg/test_rules.c`, `tools/mtg/test.sh`; `tools/mtg/scope_dump.c`, `tools/mtg/scope.py`.
- `dist_extras/FORGE_CREDITS.txt`: GPL-3 credits and Wizards' Fan Content Policy line (ships next to the exe).

## How the engine works
- **A duel is one plain struct** (`MtgDuel`, no pointers, objects by index): memcpy copies the whole game, for the
  AI looking ahead and for rewinds. Card definitions are read-only and shared (`Mtg_DbLoad` once).
- **Decisions**: the duel runs until it needs one (`Mtg_Query`: PRIORITY, ATTACKERS, BLOCKERS, DISCARD, GAME_OVER)
  and goes on with the answer (`Mtg_Answer`). PRIORITY lists every legal action fully formed (card, ability,
  target, X), so an answer is an index: the same for a player, the AI and a friend online (answers over the relay,
  as the Yu-Gi-Oh! side sends them). Same seed + same answers = the same duel (`MtgDuel.hash`, events).
- **Events** (`MtgEvent`, a ring of the last 128 plus a running count): what the table will animate.
- **Strict support**: each script line, keyword, cost, parameter, Defined$, amount and Valid text is either
  translated or makes the card unsupported (`MtgCardDef.unsupported` says what). Adding a translation = making a
  piece known and doing what Forge's class does; the scope report shows how many cards it unlocks.

## Rules translated so far
- Turn structure with every step; priority passing; the stack (LIFO, resolution, targets checked again: fizzle);
  last known information for "its power" / "its controller" (Swords to Plowshares).
- Casting (sorcery / instant timing, Flash), X (the most that can be paid, for now), activated abilities ({T},
  mana, sacrificing itself), summoning sickness, Haste.
- Mana: basic land types' intrinsic abilities, `AB$ Mana` ({T} only: one color, `Combo`, `Any`, `Amount$`), mana
  spells (Dark Ritual), the floating pool emptying each step, auto-payment that never wastes a dual land.
- Combat: attackers / blockers, Flying / Reach, Fear, landwalk, Defender, Vigilance, protection (blocking, damage,
  targeting), First Strike / Double Strike steps, Trample, Lifelink, damage assignment in order.
- State-based actions: life 0, drawing from an empty library, toughness 0, lethal damage; draws by both losing.
- Effects: DealDamage, DamageAll, Draw, Destroy, DestroyAll, GainLife, LoseLife, Pump, PumpAll (until end of turn,
  keywords too), ChangeZone (targeted from the battlefield), Mana, Discard (at random), Tap, Untap, Counter.
- Shroud. Discard to seven at cleanup.

## Next (in order of what unlocks the most: docs/MTG_SCOPE.md)
1. **Triggers** (1042 cards first blocked; TriggerHandler and TriggerType: ChangesZone, Phase, DamageDone,
   SpellCast, Attacks, blocks...), with mid-resolution choices (a query kind for "choose a card / a player / yes or
   no" during resolution: the engine must stop inside a resolve and go on with the answer; ChooseCard, Dig...).
2. **Static abilities** (507: StaticAbilityContinuous above all, the layer order of StaticEffects; CantBlock,
   CantAttack, cost changes).
3. **Costs** (295: Sac<1/Creature>, Discard<1/Card>, PayLife, untap symbol...) and **Auras** (Enchant: 312).
4. **Replacement effects** (182: Moved, Untap, DamageDone), then Effect, ChangeType, multiple targets
   (TargetMin / TargetMax), Regenerate, Animate, PreventDamage, Tokens (Forge's tokenscripts too), counters,
   Charm, ChangeZoneAll, the rest by the report.
5. Keywords by the report: Cycling, Morph, Kicker, Echo, Flashback, Cumulative upkeep, Shadow, Banding...
6. **forge-ai** (AiController, AiAttackController, AiBlockController, ComputerUtil*, and each effect's Ai class
   in forge/ai/ability, ~45k lines for the pool's effects) in place of mtg_ai.c, under the three card games'
   shared AI levels (they slip like people). The scripts' `AI*` parameters and `AI:` / `SVar:AI...` lines are for it.
7. Mulligans (London), a player's own X, multiple blockers' damage order chosen, split / flip cards (ALTERNATE),
   planeswalkers (none in the pool), hybrid mana (none in the pool).

Then more of the game around it (docs/MTG_DECISIONS.md): the venues still to build (CIANWOOD and SAFFRON each a
copy of MOSSDEEP's CARD SHOP, SINNOH's PAL PARK as a hall for all four games), a few Magic players out in the world
(tcg_field.c's which-game menu, once the player has a deck), new regulars' decks as more cards play, Scryfall art
(tools/mtg/fetch_art.py), HOW TO PLAY pages (tcg_guide.c), the 480x320 table if 240x160 grows too tight, online
duels by answers, booster drafts with friends, the phone app through JNI like tt / tcg / ygo.

## Translating a piece
1. Find it in docs/MTG_SCOPE.md; read the Java (fetch_forge.sh): `forge/game/ability/effects/<X>Effect.java`,
   `forge/game/trigger/Trigger<X>.java`, `forge/game/staticability/StaticAbility<X>.java`...
2. Write it in the matching src/mtg file, keeping Forge's names so it can be checked against the class.
3. Make it known to the parser (the API table and its parameters in mtg_effects.c, keywords in mtg_script.c...),
   only for the forms fully handled.
4. Add set-up tables to tools/mtg/test_rules.c with real cards; run tools/mtg/test.sh; rerun tools/mtg/scope.py.

## The presentation pass (done 2026-11-04, 1.96.0)
The user tested 1.95.0: the MAGIC side's look was "not similar to Pokemon or Yugioh at all". Then, while it was being
built: "tiny details like that to be respected, across all card games" (the faded EMPTY SLOT was the example), the
table "more like Pokemon's ... that style of play area", no POKe BALL-like mark in the middle of the mat, the
opponent's cards facing them, and TRIPLE TRIAD always last on the CARD CASE's L / R round. All of it is in COOP_NOTES
"MAGIC" (faces, art, screens, table). The rule from now on: a MAGIC screen copies its POKeMON / YU-GI-OH! counterpart
in layout, colors, wording and keys; when theirs change, change MAGIC's too.
- Art: tools/mtg/fetch_art.py + tools/mtg/build_printed.py -> graphics/mtg/printed.bin (795 cards: every playable
  one; the three without are banned). Rerun both when more cards play (they keep what's there).
- Faces: src/mtg/mtg_gfx.c. Screens: src/mtg/mtg_ui.c. Table: src/mtg/mtg_board.c (tcg_board.c's mat, tray and pan,
  piles, plates, tooltips, prompts). HOW TO PLAY: eight MAGIC pages in src/tcg/tcg_guide.c. NEW: tcg_new.c
  CARDNEW_MAGIC. The save: "<save>.mtg" format 2 (each deck's ACE CARD).
- 1.98.0 (the user's feedback on 1.96): the table plays every event out before the next (B.view trailing B.duel),
  costs on the cards, tray colors per deck: COOP_NOTES "MAGIC". When translating more cards, give any new event kind
  its look in StartEvent / EndEvent (mtg_board.c) and its line in the middle; a new ability kind's cost shows by
  ActivatedCost.
- Still not like the others (open): SEARCH & SORT (tcg_search.c has no MAGIC game yet), the BINDER's sets / rarity
  views, CARD SCALING and the match line on the challenge (no ratings for Magic decks yet: forge-ai first), a
  registered CARD CASE challenge out in the world (tcg_field.c's which-game menu), card foil beyond the rares' glint,
  the table at 480x320 (decision still open).
