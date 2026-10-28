# Yu-Gi-Oh! card duels: handoff

Trigger: when the user says **"Continue Yugioh"**, read this file and pick up at "Next: every card playable".
Also read docs/YGO_DECISIONS.md (the user's 30 answers and the choices since) and COOP_NOTES "Yu-Gi-Oh! card duels".

## Where things stand (2026-10-28)
Released on the default branch `claude/clever-clarke-hqati2` (the release workflow builds, tags and publishes on push):
- **1.35.0 "It's Time to Duel!"**: Yu-Gi-Oh! offline. The CARD CASE's YU-GI-OH! side, the rules engine, the
  Starter Decks + LOB + MRD with every effect, boosters with real pull rates, NPCs by trainer class, special
  duelists with signature cards, the CARD CLUB's duelist, achievements, the real card back (CARD BACKS: PRINTED).
- **1.36.0 "Duel Your Friends!"**: online Yu-Gi-Oh! duels and trades (src/ygo/ygo_net.c), a Starter Deck to keep
  or borrow for anyone asked without a deck (ygo_ui.c ResolveNoDeck), the GAME CENTER 2F Yu-Gi-Oh! clerk, the CARD
  CLUB member behind the POKeMON MART counter.
- **1.37.0 "Same Hands, Two Games!"**: one set of buttons on both duel tables and the Yu-Gi-Oh! screens made to
  work like the Pokemon ones (details below, "Ground rules").

The working branch of the last session was `claude/dreamy-archimedes-az1gbz` (same commit as the default branch
after 1.37.0). A new session gets its own branch name; start it from the default branch.

## Next: every card playable (the user's request, not started)
"Every card available in Stairway to the Destined Duel must be playable, with full functionality." Measure it with
`tools/ygo/progress.sh` (and `tools/ygo/progress.sh todo` for the list by set). At the handoff:

    Stairway to the Destined Duel: 1071 cards, 731 playable, 340 to write
        (Effect 118, Fusion 3, Ritual 15, Spell 121, Trap 83)
    2003 booster cards Stairway lacks: 434, 40 playable, 394 to write
    LOB 126/126, MRD 144/144 and the four Starter Decks done; SRL 42/104, PSV 27/105, LON 23/105,
    LOD 10/101, PGD 6/108, MFC 9/108, DCR 7/106

Answer 10 put the 2003 booster cards Stairway lacks in the pool too, so a set's boosters only reach the shops,
rewards and NPC decks once 90% of its cards work (Ygo_SetAvailable; Ygo_CardReady gates single cards everywhere).
The user asked for the Stairway cards; the booster-only ones come with their sets.

### How cards are written
- Effects live in tables, one file per stage: src/ygo/ygo_fx_s1_monsters.c / _spells.c / _traps.c hold stage 1.
  Add stage 2 files (e.g. ygo_fx_s2_*.c per set or per kind) and list their tables in src/ygo/ygo_fx.c sTables.
- Rows (src/ygo/ygo_fx.h): ACT (a Spell/Trap activation), FX (any effect: type, Spell Speed, flags), TRIG
  (triggers on events), FLIP, PASSIVE (stat / rule hooks), ACT_HOOKS (Equip, Field, Continuous), EQUIP / FIELD for
  the generic ones (ygo_fx_lib.c tables). Each row: can (no questions), cost, target (asks, stores in the chain
  link), op. Helpers in ygo_fx_lib.c: filters, Fx_Targets / Fx_Target, Fx_TargetOk, Fx_DestroyAll, deck searches,
  discards, paying LP, lasting effects (Ygo_AddLasting with YR_* rules and scopes).
- Flags (src/ygo/ygo_internal.h YF_*): YF_OPTIONAL, YF_WHEN (can miss the timing), YF_ONCE_PER_TURN,
  YF_DAMAGE_STEP, YF_SUMMON_NEGATE, YF_TARGETS, YF_FROM_HAND, YF_IN_GRAVE, YF_STAYS, YF_HARD_ONCE, and YF_PHASE:
  an effect made for a quiet moment (a phase: Blast Juggler's Standby Phase). Without YF_PHASE the table passes
  the chain question for the player at phase changes (YgoDuel_QuietChance), so a "during the Standby / End Phase"
  card needs it.
- Today's wording and rulings (answers 4-5): the card text in the catalog is today's; rule by it.
- Expect engine work for mechanics stage 1 never needed. Check each set's cards for them before writing (from
  memory, not verified: Union monsters, Spirit monsters, Spell Counters, banishing, Toon rules past MRD, monsters
  with their own Special Summon conditions, cards that change control, Fusion substitutes, coin / dice effects).
  Engine changes go in src/ygo/ygo_duel.c; keep it deterministic (same seed + answers = same duel, online depends
  on it: ygo_net.c sends only answers).
- The AI (src/ygo/ygo_ai.c) tries each choice on a copy of the duel and scores it; a row's `ai` field nudges it
  (negative: rather not). New cards usually need nothing more; check they get used (tools/ygo/test_effects.c prints
  the cards that never activated).

### How to test each batch
- `tools/ygo/test.sh`: engine, collection, AI, the link between two games, and the card tests:
  - tools/ygo/test_cards.c: one scenario per tricky card or rule (set up a state, answer, check). Add scenarios for
    each new mechanic and for cards whose text is easy to get wrong.
  - tools/ygo/test_effects.c: random duels with random decks of the ready cards (every card kept: YgoDuel_Conserved;
    every duel ends). It says "stage-1" in its output: widen it to all ready cards.
- `tools/ygo/test_ui.sh` (headless screens) and the scenarios `ygo_online`, `ygo_online_nodeck`, `ygo_trade`
  (`tools/test/regress.sh NAME`, also with `WORST=1`).
- Keep card tests repeatable: a fixed seed in tools/ygo tests, and `mp ygo first N` in headless runs (coin flips and
  random discards then come out the same every time).
- Debug: `mp ygo ...` (ygo_ui.c Ygo_Debug): state, starter N, giveall, own ID N, packs SET N, pack N, deck THEME
  TIER, first N, auto, fast, win, concede, mine, focus K I, nodecks, practice, open, case, club, binder, builder.

### Releasing the cards
Ship per set or a few sets at a time (answer 30: stages). Bump MOEMON_VERSION (include/mp_version.h, 1.37.0 now),
add tools/release/notes/v<ver>.md (first line `Moemon Emerald Kai Co-op <ver> "<Catchy Title!>": <one line>`,
player-facing only), build Windows first (`tools/release/build_windows.sh SDL2_DIR`, the mingw SDL2 package
SDL2-devel-2.30.9-mingw), push to the default branch. Ask the user before pushing a release.
Rules changes change duels between friends: the version check already keeps online players on one version.

## Ground rules from the user (keep to them)
- The two card games must handle the same way: only the game systems differ. Both duel tables: A acts on the card
  under the cursor, R looks at it up close, B backs out (the Pokemon side leaves the hand first) and on the open
  table opens the turn menu with END TURN chosen; START and SELECT open the turn menu too and finish a pick of
  several cards. The turn menu is the short list at the bottom right (Pokemon: END TURN / READY, CONCEDE;
  Yu-Gi-Oh!: the next phase, END TURN, CONCEDE). New screens and new card prompts follow the same buttons.
- No speeding up animations by holding a button (it could break online games). The DUEL SPEED option stays.
- Boosters reveal on their own (A: the next card now, B: all); no opening cards one by one by hand.
- Where one side does something better, give it to the other too.
- Patch notes are for players: what changed in the game, never where it came from.
- Dev updates only when the user says so ("As a dev update..."); everything else is a main update.

## Not done (besides the cards)
- Yu-Gi-Oh! card duel TOURNAMENTs (CO-OP > TOURNAMENT plays the Pokemon cards only: mp_tourney.c TK_CARDS).
- Still worth a look on the Yu-Gi-Oh! table at real speed: chain badges, FX_BIGCARD timing, Tribute / target / grid
  picks with real keys.
- The Windows exe is cross-compiled; it has never been run on real Windows (CLAUDE.md).

## Gotchas
- `struct YgoLink` is a chain link (include/ygo/duel.h); the online link is `struct YgoNet` (include/ygo/net.h).
- Strings in C `_("...")` must keep `\n` escaped (a Python edit once wrote a real newline).
- printed.bin: `tools/ygo/build_printed.py` after catalog changes (needs `tools/ygo/fetch_art.py` first: the card
  scans and the back go to the gitignored tools/ygo/metadata/); `--keep-cards` rebuilds only the back.
- The catalog comes from the user's SDD ROM (tools/ygo/import_sdd.py; never commit the ROM) and YGOPRODeck
  (tools/ygo/fetch_db.py, build_catalog.py, gen_catalog.py). Card ids must not move once released: saves store them.
- The .ygo save is format 2 (gcSingles / gcDecks); format 1 still loads. A new field means format 3 with the
  same fallback (src/ygo/ygo_collection.c, tested in tools/ygo/test_collection.c).
- When you wait on a background test with pgrep, don't match a pattern that's in your own command line.
