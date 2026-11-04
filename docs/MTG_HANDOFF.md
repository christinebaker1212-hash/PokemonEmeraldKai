# Magic: The Gathering: handoff

The user's decisions: `docs/MTG_DECISIONS.md` (keep to them). How much of the pool plays: `docs/MTG_SCOPE.md`
(generated: `tools/mtg/scope.py`). This file: what is built, how it is built, what comes next.

## Status (2026-11-04: in the game, 1.95.0)
- **Playable in the game at MOSSDEEP's CARD SHOP** (COOP_NOTES "MAGIC"): the clerk's starter decks, six regulars
  upstairs and down, the CARD CASE's MAGIC side (hidden until a deck), decks and the builder, the BINDER, boosters
  for wins, the duel table. Scenarios mtg_shop, mtg_play. Not yet: the other venues (CIANWOOD, SAFFRON, SINNOH's
  PAL PARK), Magic players out in the world, booster drafts with friends, online duels, card art, HOW TO PLAY pages
  in tcg_guide.c, the phone app.

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

## NEXT: the presentation pass (planned 2026-11-04, stopped by the user before it was built)
The user tested 1.95.0: it works, but the MAGIC side's style and presentation are "not similar to Pokemon or Yugioh
at all" and feel inconsistent. Bring every Magic screen and the table up to the other card games' look. Compare with
their screens first (tools/ygo/test_ui.sh ONLY="screens table" OUT=dir: home, binder, builder, inspect; the scenarios
ygo_online / tcg_ask_both a_duel shots for the tables).

What was found missing, against YU-GI-OH! (ygo_ui.c, ygo_board.c, ygo_gfx.c) and POKeMON (tcg_ui.c, tcg_board.c):
- **Card art.** They draw every card as a face of the game's own frame (colored by kind) around the printed
  illustration (ygo_gfx.c Face / ArtInto / Resample, textures cached per size; G_ART_FULL_CARD shows the whole scan),
  the printed back too. Magic draws flat colored tiles with names.
- Screens: HOME shows the deck's ace card as a floating face; the BINDER is a grid of faces with xN counts, color
  filter chips and "owned/total"; the builder is a face grid plus a deck-list panel ("50 +0 READY"); a card's view is
  the big face on the left and an info panel on the right (name, type line, text box, stats, set / rarity, "You own
  N", A BIG PICTURE); the challenge shows the opponent's battle picture in a frame (tcg_field.c CaptureChallengePic).
- The table: a themed mat (YU-GI-OH!'s navy grid with zone outlines, POKeMON's teal with the POKe BALL emblem), LP
  boxes, deck piles with the back and a count, a phase bar (DP SP M1 BP M2 EP), the "YOUR TURN" sweep (G_Banner,
  the shared FX_SWEEP), card faces with art in a fanned hand, an info window for the card under the cursor.

The plan, in order (tasks were written up; none done):
1. **Art** (the user's decision 8: Scryfall). `tools/mtg/fetch_art.py` (written, NOT yet run to the end: the run
   was stopped; its cache is ~/.cache/moemon/mtg_art, maybe partly filled): for each card the engine plays (or
   `--all`), its earliest old-frame printing (Scryfall `format:premodern unique:prints (frame:1993 or frame:1997)`,
   released ascending: 14279 printings, ~82 pages), the "normal" scan and the art crop, the card back
   (backs.scryfall.io), and tools/mtg/art.json (set, number, artist, frame). Then a `tools/mtg/build_printed.py`
   like tools/ygo/build_printed.py: JPEG at 160x223 per card + the illustration's rectangle (measured from the art
   crop, or fixed per frame), the back after the cards, into graphics/mtg/printed.bin, built in by a
   src/mtg/mtg_art_data.c .incbin (json_data_rules.mk dependency). Keep each artist: Scryfall asks that the
   illustration shown without its printed card credit the artist (show it in the card view). Re-run as more cards
   play (keep what's there, add the new). The game's own frame is an original design, never the printed one.
2. **Faces**: src/mtg/mtg_gfx.c modeled on ygo_gfx.c: a frame colored by the card's color (white, blue, black, red,
   green, gold for more than one, silver artifacts, brown lands), name bar, mana cost dots, the illustration,
   a type line, text-box lines, a P/T plate on creatures; G_ART_FULL_CARD / CARD BACKS options like the others;
   Mtg_DrawCard(card, x, y, w, h, angle, alpha) (angle 90 for tapped cards).
3. **Screens** (mtg_ui.c) laid out as YU-GI-OH!'s: the ace card on HOME, the BINDER grid with chips (W U B R G,
   colorless, multicolor, lands) and L ALL, the builder grid + deck list, the card view + BIG PICTURE, the starters
   each shown by their face card, the challenge with the regular's battle picture, the booster opened card by card
   like the packs' reveal. The theme color can stay MTG_THEME but in the kit's way (side panel, strip, hints).
4. **The table** (mtg_board.c drawing; the rules side stays): a Magic mat of its own (an original pattern, e.g. a
   felt with a faint grid and the five colors' dots as its emblem), zone outlines for lands and creatures, life boxes
   like the LP boxes, library and graveyard piles, card faces with art (tapped ones turned), a fanned hand that
   lifts the focused card, a phase bar (UNT UPK DRW M1 CMB M2 END), the YOUR TURN / THEIR TURN sweep, the info
   window over the hand, the stack shown as faces with lines to their targets, short slides for cards played.
5. Rerun mtg_shop / mtg_play (add rendershots of every screen), the card games' scenarios, tools/mtg/test.sh; then a
   release (notes for players: the new look). Push only to the default branch (CLAUDE.md).
