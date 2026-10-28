# Yu-Gi-Oh! card duels: handoff

Trigger: when the user says **"Continue Yugioh"**, read this file and pick up at "Next steps".
Branch: stage 1 started on `claude/stoic-knuth-rh14w7` and continues on `claude/dreamy-archimedes-az1gbz` (merged
with the default branch at 1.34.0); nothing is on the default branch yet, nothing released.

## What the user asked for
Reuse the Pokemon TCG system (src/tcg) for a Yu-Gi-Oh! version. Modern rules over EDS-style cloning.
Stairway to the Destined Duel (SDD, GBA) is the card pool and its Forbidden/Limited list; real English boosters
up to the end of 2003 with real pull rates. The CARD CASE's top menu switches POKeMON / YU-GI-OH! (decks, binder,
packs). NPCs prefer one game or the other, thematically.

The user's answers to the 30 design questions (keep to them):
1 SDD ROM as source (ROM at /tmp/claude-0/ygo/rom/rom.gba in the old container; NEVER commit it; derived data is
  already in the repo) · 2 LP 8000 · 3 first player no draw, can't attack turn 1 · 4 today's rulings · 5 today's
  wording · 6 Fusion Deck up to 15 · 7 stop only when a response is possible · 8 no side deck, single duels ·
  9 every English booster to end of 2003 (LOB MRD SRL PSV LON LOD PGD MFC DCR) · 10 real-pack cards SDD lacks:
  in the pool anyway · 11 SDD cards with no 2003 pack: card shop and special duelists · 12 same CARD POINTS as the
  Pokemon cards · 13 rarities shown as real (Rare silver name, Super shiny art, Ultra gold, Secret rainbow) ·
  14 NPC split (YGO: Psychics, Hex Maniacs, Ninja Boys, Black Belts, Battle Girls, Cool Trainers, Ruin Maniacs,
  Kindlers, Collectors, Experts, Aqua/Magma grunts, most teen townsfolk; Pokemon: kids, Bug Catchers, Swimmers,
  Fishermen, Hikers, Picnickers, Lasses, Sailors, Aroma Ladies, Beauties, older townsfolk) · 15 both games: CARD
  CLUB staff, rival, WALLY, STEVEN (they ask which) · 16 YGO decks themed by who they are, stronger with badges ·
  17 Elite Four, MAXIE, ARCHIE, Frontier Brains play YGO (SIDNEY Fiends, PHOEBE Zombies, GLACIA Aqua, DRAKE Dragons,
  MAXIE Pyro, ARCHIE Fish/Sea Serpents); Gym Leaders keep Pokemon · 18 nods to the show's duelists within a type ·
  19 signature card as first-win prize · 20 today's table layout · 21 card-game effects like the Pokemon table ·
  22 skip phases where nothing can happen · 23 YGO frame in Game Boy style around the printed illustration ·
  24 a YGO duelist joins the CARD CLUB, and BOTH CARD CLUB NPCs move to the POKeMON MARTS (done) · 25 pick one of
  the four real Starter Decks · 26 teaching duel offered · 27 YGO duels and trades with friends · 28 rewards like
  Pokemon (booster every win + one card of their deck daily) · 29 own YU-GI-OH! achievements tab · 30 ship in stages.

## Done (stage 1, on the branch)
- Data: tools/ygo/import_sdd.py (ROM -> tools/ygo/sdd.json, graphics/ygo/sdd_art.bin), fetch_db.py,
  build_catalog.py, gen_catalog.py (src/ygo/ygo_catalog.c, include/ygo/card_ids.h, src/ygo/ygo_fusions.c; '#'
  in names becomes "No." since the font lacks it), fetch_art.py + build_printed.py (graphics/ygo/printed.bin,
  160x233 JPEGs, 17.4 MB), src/ygo/ygo_art_data.c (.incbin, json_data_rules.mk dependency).
- Rules engine src/ygo/ygo_duel.c (+ ygo_internal.h, include/ygo/duel.h): chains, SEGOC, missing timing, damage
  step, summon-negation windows, lasting effects. Stage-1 card effects (Starter Decks + LOB + MRD: 325 cards and
  25 Fusions) in ygo_fx_s1_*.c with ygo_fx_lib.c. AI src/ygo/ygo_ai.c. Ygo_AskOption now takes a prompt
  (YGO_P_COIN_CALL, YGO_P_SUMMON_OR_SET) so screens can word options.
- Collection include/ygo/collection.h + src/ygo/ygo_collection.c ("<save>.ygo", written from Tcg_WriteSidecar,
  backed up with the save in sdl2.c), src/ygo/ygo_integration.c.
- Card faces src/ygo/ygo_gfx.c (GB-style frames, CARD ART modes PRINTED ART / GAME BOY (SDD's own picture) /
  FULL CARD).
- Duel table src/ygo/ygo_board.c/.h: today's layout, event-by-event presentation, cursor spots, menus for every
  query type, chain numbers, pile viewer, result banner, lesson tips.
- Screens src/ygo/ygo_ui.c: CARD CASE YU-GI-OH! side (L/R switches sides; src/tcg/tcg_ui_kit.h shares the look;
  tcg_ui.c delegates Step/Draw/IsActive), decks, builder (main 40-60 + Fusion 15), binder with rarity foils,
  packs + reveal, inspect + big picture, challenge, spoils, counter (TRADE IN, CARD SHOP), Starter Deck pick,
  lesson, practice. Debug: `mp ygo ...` (state, starter N, case, club, pickstarter, lesson, practice, open, pack N,
  binder, builder, inspect N, giveall, points N, packs SET N, own ID N, deck THEME TIER, first N, auto, fast, win,
  mine, focus K I).
- World src/ygo/ygo_field.c + src/tcg/tcg_field.c Ask(): who plays what (trainer class / townsfolk sprite),
  themed decks by badge tier with the show's duelists' cards, 15 special duelists with signature cards, staff /
  STEVEN / rival / WALLY ask "MOeMON CARDS or YU-GI-OH!" (a menu in Task_CardChallenge). ELITE FOUR wins also
  hand over their old Pokemon promo once (Tcg_GivePromoOf) so those 4 promos stay obtainable. CARD CLUB member
  and the new Yu-Gi-Oh! duelist (MAN_5 sprite, data/scripts/ygo_club.inc, specials Ygo_Club*) stand in
  OldaleTown_Mart (4,2)/(7,2), MauvilleCity_Mart (4,2)/(7,2), LilycoveCity_DepartmentStore_1F (12,2)/(14,2);
  removed from the three POKeMON CENTERs. YU-GI-OH! achievements tab in src/mp/mp_achieve.c (ygo* ids);
  YGO special wins also count LEGENDS OF THE TABLE (cardspecial).
- Tests passing: tools/ygo/test.sh (engine, cards, AI, collection); tools/tcg/test_ui.sh (updated: the club is
  at warp 2,4,4,3 facing UP now). Verified by hand in headless runs: club flow, which-game menu, a YGO challenge
  win with booster + spoils, practice duels, screens.

## Done (second session)
- The default branch (1.34.0) merged in.
- **tools/ygo/test_ui.sh** (headless; `ONLY="club challenge screens monkey"` runs some; `SEED=` for the monkey):
  the CARD CLUB duelist (case, Starter Deck, lesson, counter), a challenge with the registered CARD CASE (the
  which-game menu opens on the first A after ~400 frames of slow text), the case's screens, and a practice duel
  mashed with random keys then played out by `mp ygo auto` (seeds 1-4 and 7 pass).
- Rough edges fixed: the Starter Deck blurb fits two lines; the deck builder's hints fit (the tabs show L/R) and
  its page counter moved into the top bar; the result banner no longer covers the info line / phases.
- Quiet chain windows: a chance to chain while a phase changes (no chain, summon, attack or Damage Step:
  YgoDuel_QuietChance) is passed by the board for the player unless L is held; effects made for such a moment
  carry YF_PHASE (Blast Juggler, Ultimate Offering's quick effect) and are still asked. Engine unchanged (online
  answers stay the same on both sides). The lesson's chain tip says so.
- Real card backs: CARD BACKS (the Pokemon cards' option, gMpConfig.cardBack) also picks the Yu-Gi-Oh! back:
  PRINTED = the English back (Yugipedia's Back-EN.png scan, fetch_art.py -> tools/ygo/metadata/back.png),
  stored after the cards in printed.bin (build_printed.py; `--keep-cards` rebuilds only the back); PIXEL = the
  drawn swirl. Ygo_SetBackMode in ygo_gfx.c.

## Next steps (in order)
1. (done: test_ui.sh)
2. Still worth a look at real speed: chain badges, FX_BIGCARD timing, tribute / target / grid picks with real keys
   (the look so far: action menu, END TURN / CONCEDE, chain prompts, monkey shots all fine).
3. (done) Online Yu-Gi-Oh! duels and trades (ygo_net.c), the no-deck resolver, the GAME CENTER clerk, the CARD
   CLUB behind the Mart counter: see COOP_NOTES "Yu-Gi-Oh! card duels". Not done: Yu-Gi-Oh! card duel TOURNAMENTs
   (mp_tourney.c TK_CARDS is the Pokemon cards only).
4. (done) Docs: CLAUDE.md feature paragraph, docs/COOP_NOTES.md "Yu-Gi-Oh! card duels", docs/YGO_DECISIONS.md,
   dist_extras/README.txt YU-GI-OH! (and the CARD CLUB's new place).
5. (done) Windows build: tools/release/build_windows.sh builds cleanly (mingw-w64 + SDL2-devel-2.30.9-mingw).
6. Release (ASK THE USER FIRST: pushing to the default branch publishes): bump MOEMON_VERSION in
   include/mp_version.h (default branch is 1.34.0 -> 1.35.0), add tools/release/notes/v1.35.0.md starting
   `Moemon Emerald Kai Co-op 1.35.0 "<Catchy Title!>": <one-line note>` with player-facing notes (CARD CASE's
   YU-GI-OH! side, Starter Decks, boosters with rarities, who duels, special duelists, CARD CLUB moved to the
   POKeMON MARTS, achievements; "Not yet": online Yu-Gi-Oh!, cards past Metal Raiders). Merge the default branch
   in first (git merge origin/claude/clever-clarke-hqati2), never force-push.
7. Later stages: card effects for SRL, PSV, LON, LOD, PGD, MFC, DCR and SDD-only cards (Ygo_CardReady gates
   packs/shops/decks; Ygo_SetAvailable needs 90% ready), then their boosters appear automatically.

## Gotchas
- Strings in C `_("...")` must keep `\n` escaped (a Python edit once wrote a real newline).
- printed.bin must be rebuilt with tools/ygo/build_printed.py after catalog changes (needs the images:
  tools/ygo/fetch_art.py, gitignored folder).
- Tests: scripts in the old session lived in /tmp (gone); the commands above are enough to rebuild them.
