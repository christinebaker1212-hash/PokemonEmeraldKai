# Magic: The Gathering: handoff

The user's decisions: `docs/MTG_DECISIONS.md` (keep to them). How much of the pool plays: `docs/MTG_SCOPE.md`
(generated: `tools/mtg/scope.py`). This file: what is built, how it is built, what comes next.

## Pick up here (2026-11-07, 1.167.0: the CARD SHOP counter and structure decks)
**The user's rules from this session (keep them):**
- **MOSSDEEP's CARD CLUB maps are the user's** (they're editing them): change only the scripts that point there
  (data/scripts/mtg_shop.inc) and the C behind them. No new map objects / people there without asking. (1.168.0 did
  what the user then asked for: the counter's door, B1F's exit mat, six rotating chairs, talkers: src/mtg/mtg_club.c.)
- The play room's guests bring any deck that plays (mtg_club.c Decks): newly playable structure decks show up in
  guests' hands by themselves too.
- Structure decks are **curated** ("Let's not overwhelm the player"): tools/mtg/fetch_precons.py CURATED (16 now).
  Add to it sparingly; keep the order (starters, theme decks by era, World Championship decks).
- "Remember to add them as cards are available": the shop shows a curated deck by itself once its last card plays,
  so the work is **translating the cards the curated decks wait for**. tools/mtg/test.sh prints each deck's count and
  first missing card. 4 of 16 play now (SIXTH STARTER A, INFESTATION, ARMADA, BOMBER).
- Every card-game screen keeps parity with POKeMON's (the counter copies tcg_ui.c CounterScreen; CARD POINTS on HOME).
- Structure decks are sold for money on the clerk's DECKS tab, one of each, as POKeMON's theme decks at the GAME CENTER
  (the user, 2026-11-07: never on the CARD POINTS list).

**Next, by deck (the blockers, from the scope dump):**
- SIXTH STARTER B: Pyrotechnics (DividedAsYouChoose: damage divided among targets; needs a division prompt on the
  table, also Fireball / Arc Lightning / Rolling Thunder).
- OPPOSITION: Darting Merfolk (ChangeZone of itself to the hand as an ability), Static Orb (static: untap at most two).
- GREEN STOMPY: Echo (Albino Troll, Pouncing Jaguar; also Cradle Guard for CRUSHER and FIENDISH NATURE: an upkeep
  trigger, "sacrifice unless you pay" while it came under control since the last upkeep), Cursed Scroll (NameCard),
  Wild Dogs (T:APlayerHasMoreLifeThanEachOther).
- MACHINE HEAD: Kicker (Phyrexian Scuta, Skizzik, Urza's Rage), Duress (Discard with a reveal and a chooser).
- CRUSHER: Echo, Humble (RemoveAllAbilities), Lhurgoyf (*/* by graveyard card types).
- WAY WILD: Lure / Pride of Lions / Thorn Elemental (statics), Wild Growth (T:TapsForMana).
- SLIGH: Cursed Scroll, Goblin Vandal (T:Execute), Hammer of Bogardan (ActivationZone Graveyard), Incinerate
  (RememberDamaged: can't regenerate), Mogg Flunkies (can't attack or block alone).
- Larger: REBELS CALL, FIENDISH NATURE, SLIVER SHIVERS (ChooseType / ChooseColor ETB), GOBLIN MOB (Morph, Provoke),
  PSYCHATOG (Flashback, Madness, Wish).
Echo and Kicker each unlock many pool cards besides (docs/MTG_SCOPE.md); they're the best value next.

## Earlier pick-up notes (2026-11-05, end of the session that built 1.96.0-1.100.0)
**Check first:** the 1.100.0 release run (release.yml, commit ed713ed6 on the default branch
`claude/clever-clarke-hqati2`) was still building when this was written. If it failed, read its job logs, fix, push
to the default branch and re-queue it (workflow_dispatch). The Windows build was not tried locally (no MinGW in that
container): watch for a static helper named like a Win32 function.

**What the user asked for that is still open, in their order:**
1. "continue translating the rest of the Magic cards": by docs/MTG_SCOPE.md's table. Biggest blockers now: static
   ability 533 (StaticAbilityContinuous first: lords like Lord of Atlantis, "has first strike while attacking"; then
   CantBlock / CantAttack / cost changes), cost 318 (Sac<1/Creature>, Discard<1/Card>, PayLife, SubCounter...),
   Enchant:Creature 282 (Auras: attach, the Enchanted / EnchantedController Defined$ names, falling off), replacement
   effect 191, Effect 127, ChangeType 102, TargetMin 84, Regenerate 76, Animate 70, Token 64 (Forge's tokenscripts),
   the trigger modes not done (BecomesTarget, TurnFaceUp, Cycled, DamageDoneOnce, Taps, Drawn, LandPlayed) and
   `K:etbCounter`. Each: translate, test in tools/mtg/test_rules.c with real cards, rerun tools/mtg/scope.py, then
   tools/mtg/fetch_art.py && tools/mtg/build_printed.py for the new cards (tools/mtg/test.sh fails if one has no art),
   then `python3 tools/mtg/fetch_sets.py --offline` is NOT needed (sets hold every card; Mtg_SetAvailable follows
   what plays by itself).
2. "adding ... NPC decks": only the six MOSSDEEP regulars (src/mtg/mtg_decks.c gMtgDuelists) exist. Give them decks
   that use the newly playable cards (triggers: Gravedigger, Nekrataal, Man-o'-War, Flametongue Kavu, the Specters,
   Mana Leak...) and add more regulars / venues (MTG_DECISIONS.md: CIANWOOD, SAFFRON, PAL PARK; a few players out in
   the world through tcg_field.c's which-game menu). tools/mtg/test.sh checks every duelist deck plays (40 cards).
   New regulars need map objects in MOSSDEEP's maps and EventScript_MtgDuelistN entries (data/scripts/mtg_shop.inc);
   MTG_DUELISTS and the mtgregulars6 achievement's target (src/mp/mp_achieve.c) follow the count.
3. Every new card kind must keep the table's conveyance (the user: "clear animations that signal to the player what is
   happening on the board, especially on the enemy turn", costs visible on the focused card, keywords under names):
   a new MtgEvent kind needs its look in mtg_board.c StartEvent / EndEvent, and a new decision its board mode.

**The user's standing rules for MAGIC (keep them):** screens and the table copy the POKeMON / YU-GI-OH! sides' in
every detail (faded EMPTY SLOT, wording, keys, the result screen's words); the opponent's cards face them; nothing in
the mat's middle; TRIPLE TRIAD always last in the case's L / R round; the cost pill only on the card under the
cursor; no descriptions for regulars; the win message on the duel's own result screen (MtgBoard_Result, as
TcgBoard_Result). Release after each batch once the tests pass (bump MOEMON_VERSION, notes for players only,
gen_patchnotes.py), push only to the default branch, never to a session's `claude/...` branch even if a hook asks.

**How to check a change:** `tools/mtg/test.sh` (217 checks + the art check); `make linux -j8`; `NOBUILD=1
tools/test/regress.sh mtg_shop mtg_play mtg_show mtg_triggers ach_hooks` and look at the shots (rendershot). Set-ups on
the table: `mp mtg put PLAYER hand|field|grave NAME` (scenario mtg_triggers shows how), `mp mtg points N`,
`mp mtg pack CODE N`. Forge's Java: `tools/mtg/fetch_forge.sh` (~/.cache/moemon/forge).

## Tabled (2026-11-07, the user: "Table the rest of the Magic cards for now") - resumed the same day
- The user asked to continue MAGIC (1.167.0, 2793 of 5408 playable); the "Pick up here" section above leads.
- Where it stood: the next step was triggers whose Execute$ is an "AB$ ... | Cost$ ..." ("you may pay {cost}. If you
  do, ..."; about 45 cards: ACADEMY RECTOR, BRASS MAN, AUSPICIOUS ANCESTOR, CUSTOMS DEPOT...). Plan: let mtg_script.c
  keep a triggered ability's Cost$ (today "optional cost" is unsupported) in gMtgUnlessCosts, and in Mtg_ResolveStep
  ask its controller MTG_CHOICE_PAY before the root resolves, the reverse of UnlessCost (paid: the chain resolves; not
  paid or can't pay: the whole chain is passed by).
- After that, by count: TapsForMana (15, mostly extra mana), ChooseType (37: needs a type menu on the table), the
  replacement effects left (Moved 22, DamageDone ReplaceWith 28).

## Status (2026-11-07: 1.167.0)
- The CARD SHOP counter (TRADE IN, CARD SHOP, PRIZES), structure decks, CARD POINTS on HOME; DamageDealtOnce, Counter
  destinations, "may draw", RearrangeTopOfLibrary, dies-to-library: COOP_NOTES "MAGIC's CARD SHOP counter and
  structure decks". 2793 of 5408.

## Status (2026-11-07: 1.162.0)
- State triggers (Mode$ Always) and DamageDoneOnce: COOP_NOTES "MAGIC's state triggers". 2763 of 5408. Next by count:
  T:Execute (52), T:TapsForMana (15, mostly extra mana), ChooseType (37), replacement effects left (Moved 22,
  DamageDone ReplaceWith 28).

## Status (2026-11-07: 1.161.0)
- "Becomes the target" (Mode$ BecomesTarget): COOP_NOTES "MAGIC's becomes the target". 2734 of 5408. Next by count:
  T:Execute (52), T:Always (21), T:DamageDoneOnce (17), T:TapsForMana (15), ChooseType (37), replacement effects left.

## Status (2026-11-07: 1.160.0)
- Trigger modes Taps / LandPlayed, other players' OptionalDecider$, trigger CheckSVar$, Player.EnchantedController:
  COOP_NOTES "MAGIC's trigger modes Taps and LandPlayed". 2715 of 5408. Next by count: T:Execute (52), T:BecomesTarget
  (22), T:Always (21), T:DamageDoneOnce (17), T:TapsForMana (15), ChooseType (37), replacement effects left.

## Status (2026-11-07: 1.159.0)
- "Can't be countered", Untap with conditions, inZoneBattlefield: COOP_NOTES "MAGIC's can't be countered". 2678 of 5408.
  Next by count: replacement effects left (DamageDone ReplaceWith 28, Moved 22), ChooseType (37: needs a type menu on the
  table), T:Execute (52).

## Status (2026-11-07: 1.158.0)
- "A source of your choice" (ChooseSource; the Circles of Protection): COOP_NOTES "MAGIC's a source of your choice".
  2667 of 5408. Next by count: replacement effects left (ReplaceWith$ DealDamage redirects, Moved, Draw), ChooseType (37),
  T:Execute (52).

## Status (2026-11-07: 1.157.0)
- Damage prevention (R: DamageDone Prevent$, Fog, Effects with ReplacementEffects$) and "doesn't untap" for a while:
  COOP_NOTES "MAGIC's damage prevention and doesn't untap". 2641 of 5408. Next by count: replacement effects left
  (87: ReplaceWith$, Moved, Draw), ChooseSource (45), ChooseType (37), T:Execute (52).

## Status (2026-11-07: 1.156.0)
- Effects for a while (Effect with StaticAbilities$): COOP_NOTES "MAGIC's effects for a while". 2599 of 5408. Next:
  replacement effects (97, and 78 Effects that carry them), Effects with Triggers$ (21), ChooseType (35).

## Status (2026-11-07: 1.155.0)
- Auras that watch their creature (AttachedBy), DamagedBy, MultiColor: COOP_NOTES "MAGIC's Auras that watch their
  creature". 2574 of 5408. Next by count: Effect (163), replacement effects (97), ChooseType (35).

## Status (2026-11-07: 1.154.0)
- The AI tries what it has no score for on a copy of the duel (Lookahead); random-deck duels in tools/mtg/test.sh:
  COOP_NOTES "MAGIC's AI look-ahead and random-deck duels". 2541 of 5408. Next by count: Effect (163), replacement
  effects (97), ChooseType (35), T:ValidCard (32).

## Status (2026-11-07: 1.153.0)
- "For each" spells (RepeatEach) and SetLife: COOP_NOTES "MAGIC's for each spells and SetLife". 2541 of 5408. Next by
  count: Effect (163), replacement effects (97), ChooseType (35), T:ValidCard (32); the AI for RepeatEach / SetLife /
  Reveal (it doesn't cast them yet: a look-ahead on a copy of the duel would serve every kind).

## Status (2026-11-07: 1.152.0)
- Choices among a player's own cards (ChangeType$ from the battlefield / a graveyard): COOP_NOTES "MAGIC's choices
  among a player's own cards". 2530 of 5408. Next by count: Effect (163), replacement effects (97), RepeatEach (41),
  ChooseType (35), T:ValidCard (32).

## Status (2026-11-07: 1.151.0)
- Remembered cards, conditions, reveals, a chain's target in a sub-ability: COOP_NOTES "MAGIC's remembered cards,
  conditions and reveals". 2515 of 5408. Next by count: Effect (163), replacement effects (97), RepeatEach (41),
  ChooseType (35), Hidden (34), T:ValidCard (32).

## Status (2026-11-07: 1.150.0)
- Mill and coin flips: COOP_NOTES "MAGIC's mill and coin flips". 2476 of 5408. Next by count: replacement effects (97),
  RepeatEach (36), ChooseType (34), RememberChanged (33), T:ValidCard (32), Reveal (24).

## Status (2026-11-07: 1.149.0)
- Activation limits: COOP_NOTES "MAGIC's activation limits". 2458 of 5408. Next by count: replacement effects (97),
  RepeatEach (36), ChooseType (34), RememberChanged (33), T:ValidCard (32), Reveal / Mill / FlipCoin (21-24 each).

## Status (2026-11-07: 1.148.0)
- Look at the top cards (Dig): COOP_NOTES "MAGIC's look at the top cards". 2417 of 5408. Next by count:
  replacement effects (96), RepeatEach (36), ChooseType (32), T:ValidCard (30), RememberChanged (30).

## Status (2026-11-07: 1.147.0)
- Library positions and tutors: COOP_NOTES "MAGIC's library positions and tutors". 2396 of 5408. Next by count:
  replacement effects (96), RepeatEach (36), Dig (35), ChooseType (32), T:ValidCard (30), RememberChanged (30).

## Status (2026-11-07: 1.146.0)
- Counters of every kind: COOP_NOTES "MAGIC's counters of every kind". 2366 of 5408. Next by count: replacement effects
  (96), LibraryPosition (40), RepeatEach (36), Dig (35), ChooseType (32), T:ValidCard (30), RememberChanged (30).

## Status (2026-11-07: 1.145.0)
- GainControl effects: COOP_NOTES "MAGIC's control changes". 2322 of 5408. Next by count: replacement effects (96),
  counters beyond +1/+1 (PutCounter 41), LibraryPosition (40), RepeatEach (36), Dig (35), ChooseType (32).

## Status (2026-11-07: 1.144.0)
- PreventDamage shields: COOP_NOTES "MAGIC's damage prevention". 2305 of 5408. Next by count: replacement effects
  (96), counters beyond +1/+1 (PutCounter 41), GainControl (40), LibraryPosition (39), RepeatEach (36), Dig (35).

## Status (2026-11-07: 1.143.0)
- Delayed triggers (Mode$ Phase): COOP_NOTES "MAGIC's delayed triggers". 2263 of 5408. Next by count: replacement
  effects (96), PreventDamage (54), counters beyond +1/+1 (PutCounter 41), GainControl (40), LibraryPosition (39).

## Status (2026-11-07: 1.142.0)
- UnlessCost$ forms that aren't mana (return, discard, sacrifice, life, DamageYou): COOP_NOTES "MAGIC's unless costs
  that aren't mana". 2227 of 5408. Next by count: replacement effects (96), DelayedTrigger (58), PreventDamage (54),
  counters beyond +1/+1 (PutCounter 41), GainControl (40), LibraryPosition (39).

## Status (2026-11-07: 1.140.0)
- Animate (manlands, Animate Land, Chimeric Staff...): COOP_NOTES "MAGIC's Animate". 2190 of 5408. Still out:
  RemoveCardTypes / RemoveKeywords / RemoveLandTypes, Abilities$ / Triggers$ / staticAbilities$ granted by an Animate,
  creature types of animated permanents for Valid specs (Mishra's Factory's Assembly-Worker), UntilYourNextUpkeep.

## Status (2026-11-07: 1.139.0)
- ChangeZoneAll, painlands, Pump's landwalk / can't block / can't attack: COOP_NOTES "MAGIC's mass moves, painlands
  and pump extras". 2162 of 5408. Next: counters beyond +1/+1 (CHARGE, STORAGE, P2P2...: PutCounter 41, SubCounter
  costs), replacement effects (96), Animate (84), PreventDamage (54), DelayedTrigger (53), UnlessCost forms (49).

## Status (2026-11-07: 1.138.0)
- Library and hand searches (ChangeType$): COOP_NOTES "MAGIC's library searches". 2104 of 5408. Still out: Reveal$,
  LibraryPosition$ (to the library's top: Worldly Tutor), searches of other players (DefinedPlayer$), the sideboard
  (Wishes), "may" finds left unpicked.

## Status (2026-11-07: 1.137.0)
- Several targets (TargetMin$ / TargetMax$, numbers and X): COOP_NOTES "MAGIC's spells with several targets". 2032 of
  5408. Still out: dividing damage or counters among them (DividedAsYouChoose: Arc Lightning, Fireball), MaxTgts SVars,
  player targets among several, triggered abilities with several targets.

## Status (2026-11-06: 1.136.0)
- ExileFromGrave costs, graveyard statics, CantBeActivated, GainControl statics, CheckSVar conditions, self cost
  reduction, the Legates' free casts: COOP_NOTES "MAGIC's 2000 cards". 2000 of 5408. Next statics: AddType (21),
  ValidPlayer, SetColor, MayLookAt, AddHiddenKeyword, CantAttackUnless (Propaganda: needs paying per attacker); then
  TargetMin (94: several targets), Effect (142), ChangeType, mana abilities with sacrifice / discard costs.

## Status (2026-11-06: 1.135.0)
- Granted abilities and triggers (AddAbility$ / AddTrigger$ / AddSVar$), MustAttack, CantBeCast, CantTarget,
  CantPlayLand, Count$ValidHand / HalfUp / HalfDown: COOP_NOTES "MAGIC's granted abilities and the newer statics".
  1953 of 5408. Next by count: AddType (21), CheckSVar statics (19: the Avatars' ReduceCost, the Legates), ValidPlayer
  (14), GainControl statics (12), SetColor (11), EffectZone$ All / Graveyard, MayLookAt, AddHiddenKeyword, keywords
  Banding / Phasing / Menace / Flanking; then Effect, ChangeType, TargetMin by docs/MTG_SCOPE.md.

## Status (2026-11-06: 1.134.0)
- Alternative costs (S:Mode$ AlternativeCost, ValidSA$ Spell.Self) and the cost parts ExileFromHand / Return /
  tapXType: COOP_NOTES "MAGIC's alternative costs". 1887 of 5408. Still out: the CheckSVar Legates (Cost$ 0 when an
  opponent controls ...), Reveal / GainLife alt costs, alt costs beside an additional cost.

## Status (2026-11-06: 1.133.0)
- Static Condition$ (Threshold, PlayerTurn, NotPlayerTurn), Secondary$, CantAttack's UnlessDefender$ (controls<Valid>,
  "!" negated, hasFewer<Creatures|Lands>InPlayThanYou): COOP_NOTES "MAGIC's Threshold and picky attackers". 1850 of
  5408 cards play. Next statics by count: AlternativeCost (52: pitch spells, needs alternative costs in the cast),
  ValidSA, AddAbility / AddTrigger / AddSVar / AddType (granted abilities), CantBeCast, CantTarget, MustAttack.

## Status (2026-11-07: next release)
- Cycling and "when you cycle" (the Cycled trigger): COOP_NOTES "MAGIC's Cycling". 1809 of 5408 cards play. TypeCycling and LandCycling still out.

## Status (2026-11-06: 1.129.0)
- Charm ("choose one"): COOP_NOTES "MAGIC's \"choose one\"". 1744 of 5408 cards play. Chooser$ Opponent and
  CharmNum > 1 still out.

## Status (2026-11-06: 1.128.0)
- */* creatures and Auras for other permanents: COOP_NOTES "MAGIC's */* creatures and other Auras". 1727 of 5408.

## Status (2026-11-06: 1.127.0)
- Target properties and counted amounts: COOP_NOTES "MAGIC's targets and amounts". 1712 of 5408 cards play.

## Status (2026-11-06: 1.126.0)
- **Tokens**: COOP_NOTES "MAGIC's tokens". 1634 of 5408 cards play. Token pieces still out: TokenPower / Toughness /
  Types / Colors, AtEOT, RememberTokens, tokens with abilities (Forge's tokenscripts files, not read yet).

## Status (2026-11-06: 1.125.0)
- ETB tapped and "doesn't untap" replacements, counted static amounts, unblockable, Wards: COOP_NOTES "MAGIC's lands
  that enter tapped...". 1606 of 5408 cards play. Biggest blockers now: static ability 427, Effect 129, ChangeType 127,
  cost 112, replacement effect 92 (DamageDone prevention 19, Moved / PayBeforeETB 8...), TargetMin 83, Animate 78,
  Token 66, ValidTgts 66, Pump 55.

## Status (2026-11-06: 1.124.0)
- **Regenerate**: COOP_NOTES "MAGIC's regeneration". 1542 of 5408 cards play. Indestructible still to come (in
  Mtg_Destroy, beside the shields).

## Status (2026-11-06: 1.123.0)
- **Costs** (Sac / Discard / Random discard / PayLife / Exile and Return itself): COOP_NOTES "MAGIC's costs beyond
  mana". 1470 of 5408 cards play. Biggest blockers now: static ability 446, replacement effect 207, Effect 127,
  ChangeType 120, cost 110 (X, exile from graveyard / hand / library, tapXType), Regenerate 89, TargetMin 82,
  Animate 73, Token 66.

## Status (2026-11-06: 1.122.0)
- **Auras** (Enchant:Creature): MTG_API_ATTACH (the synthetic aura spell), MtgObj.attachedTo, state-based removal,
  AffectedDefined$ Enchanted, EnchantedBy / enchanted, Defined$ Enchanted, comma mode lists, AttachAILogic in the
  stand-in AI; COOP_NOTES "MAGIC's Auras". Not yet: Enchant other than creatures (land, artifact, "creature you
  control"), auras that move (Attach as an activated ability), AttachedBy in triggers (SPIRIT LINK), the aura's
  controller vs the creature's controller in "you" for Defined$ (fine for the pool's playable ones).
- 1335 of 5408 cards play (77 Auras). Biggest blockers now: static ability 446, cost 326, replacement effect 207,
  Effect 126, ChangeType 103, Regenerate 89, TargetMin 79, Animate 73.

## Status (2026-11-06: 1.121.0)
- **Static abilities** (src/mtg/mtg_static.c, MtgStatic / MtgStaticMods in mtg_internal.h): `S:Mode$ Continuous` with
  Affected$ (or AffectedDefined$ Self, an Affected$ beside it then a condition on itself), AddPower / AddToughness,
  SetPower / SetToughness (layer 7b, the newest wins), AddKeyword (the engine's keywords, landwalk as `Landwalk:Island`
  or `Islandwalk`, `Protection from red`), IsPresent$ with PresentCompare$ GE / EQ / LE n; `CantBlock`, `CantAttack`
  (ValidCard$) and `CantBlockBy` (ValidAttacker$ / ValidBlocker$); `RaiseCost` / `ReduceCost` of spells (Type$ Spell,
  Activator$ You / Opponent, Amount$ n or Cost$ one colored symbol: Mtg_SpellCost, used by the cast actions and the
  cost shown on a card in hand). Mtg_Power / Mtg_Toughness / Mtg_Keywords /
  Mtg_Landwalk / Mtg_Protection include them (an Affected$ looking at power or keywords sees them without statics: no
  loops); CanAttack and Mtg_CanBlock ask Mtg_StaticForbids. Tests: TestStatics in tools/mtg/test_rules.c.
- **Valid specs**: a capitalized word is a subtype only when some card of the pool has it (Mtg_IsSubtype, from every
  Types: line before the cards parse); "DefenderCtrl", "Snow" and the like keep a card out instead of misreading it.
- 1252 of 5408 cards play (docs/MTG_SCOPE.md). Biggest blockers now: static ability 469 (AlternativeCost, ReduceCost /
  RaiseCost, AffectedDefined$ Enchanted with auras, Condition$, AddAbility / AddTrigger, characteristic-defining */*),
  cost 319, Enchant:Creature 282, replacement effect 191.

## Status (2026-11-05: in the game, 1.100.0)
- **1.100.0: triggered abilities, booster packs by set, achievements** (COOP_NOTES "MAGIC"): 1136 of 5408 cards play.
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
- `tools/mtg/fetch_precons.py` -> `tools/mtg/precons.json` (MTGJSON's Premodern-era precons) and
  `src/mtg/mtg_precons_data.c` (the CURATED structure decks; `--offline` rewrites it from the json).
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
- Triggered abilities (mtg_trigger.c, TriggerHandler / Trigger<Mode>): ChangesZone (leaving the battlefield looked
  at before the move, 603.10a; arriving after), Phase (the beginning of upkeep, draw, main 1, combat, end of combat,
  main 2, the end step), SpellCast, DamageDone (combat or not), Attacks, AttackerBlocked, AttackerBlockedByCreature,
  Blocks, AttackerUnblocked; TriggerZones (else correctZones' rule, else the battlefield), IsPresent / PresentCompare
  / PresentZone (checked again as it resolves), OptionalDecider$ You. Waiting triggers (d->pending) go on the stack
  when a player would get priority, the active player's first; no legal target: gone; one: taken; more: asked
  (MTG_Q_TRIGGER_TARGET). Defined$ Triggered<Card|NewCard|Player|Activator|Source|Target|Attacker|Blocker|Defender|
  DefendingPlayer|Attackers>[LKICopy][Controller|Owner], by mode (mtg_effects.c sTriggered).
- Resolution that stops for a choice (MTG_Q_CHOOSE: YESNO, PAY, PICK): d->res / resAbility / resStage / resPicks /
  resChoice hold where it is, Mtg_ResolveStep returns 0 while it waits. UnlessCost$ (mana or an amount SVar) with
  UnlessPayer$ (default TargetedController: Mana Leak), Sacrifice (Self, or SacValid$ chosen by each Defined$ player),
  Discard Mode$ TgtChoose / Hand (DiscardValid$), PutCounter (P1P1 / M1M1), ChangeZone from a graveyard (targets
  there: MtgAbility.tgtZone; to the hand, library, exile or the battlefield, GainControl$).

## Next (in order of what unlocks the most: docs/MTG_SCOPE.md)
1. Triggers: done in 1.100.0 for the modes above; the rest by the report (BecomesTarget, TurnFaceUp, Cycled,
   DamageDoneOnce, Taps, Drawn, LandPlayed...) and their Execute$ effects (Effect, DelayedTrigger, Token...).
2. **Static abilities** (507: StaticAbilityContinuous above all, the layer order of StaticEffects; CantBlock,
   CantAttack, cost changes).
3. **Costs**: Sac / Discard / PayLife done in 1.123.0; 110 left (X amounts, ExileFromGrave, tapXType, untap
   symbol...); Auras done for creatures in 1.122.0 (the other Enchant: kinds remain).
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
