# The POKéMON card game in every region: plan and decisions

Started 2026-11-18. **Start here** for the card game's regional sets, special duels, NPC decks and promos.

## What the user asked for (2026-11-18)
- Special duels (special trainers with their own decks and promo cards found nowhere else) in KANTO, JOHTO, SINNOH
  and UNOVA, as HOENN has.
- KANTO: Gym Heroes and Gym Challenge cards and decks, mixed with relevant boosters and decks from every trainer and
  the current selection (today's 11 boosters).
- JOHTO: Neo Genesis, Neo Discovery, Neo Revelation, Neo Destiny, mixed the same way.
- HOENN, SINNOH and UNOVA share Expedition Base Set, Aquapolis and Skyridge, mixed the same way (HOENN gets them too).
- Audit every current card so every effect works properly (e.g. Electrode's Energy Spike never let the player pick
  the Energy or the Pokémon).
- TCG Island: later, not now (only one region counts towards it: all or nothing).
- Retroactive promos (the user, 2026-11-18): some NPCs players have already beaten at cards become special card
  trainers. Their win / loss record is kept already, so a special NPC (a courier) delivers the promos those players
  should have received, once.
- A GEN switch for the card games (the user, 2026-11-18), built so every set added later gets its own place in the
  GEN selection:
  - POKéMON and MAGIC: chosen by set (each set a step in the GEN list; cards from the sets up to the one chosen).
  - YU-GI-OH!: chosen by summon type: FUSION (the base, today's behaviour), then SYNCHRO, XYZ, PENDULUM, LINK as those
    cards are added; at a step, every set released before the next summon type's first set is legal. No YU-GI-OH!
    switch until a second summon type exists: at FUSION it behaves as now.
  - It's the room host's setting online (the user, 2026-11-18). It changes how NPC decks are built (only cards inside
    the GEN), but NOT which Prize cards (promos, rewards) are given. A card outside the host's GEN can't be used in a
    deck; players are told before a duel when their deck holds such cards.
  - Open (mock-ups before building): where it's set offline (OPTIONS row) and on HOST A GAME > GAME RULES; what the
    warning before a duel looks like; packs and shops (not said: ask).

## Decisions (asked 2026-11-18; the user's answers)
| # | Question | Answer |
|---|---|---|
| 1 | How the new sets ship | **All at once**, one release when every set, deck and trainer is done (HOENN gets the three e-Card sets too) |
| 2 | Cards whose effects aren't translated yet | **Only finished cards**: a card enters packs, decks and shops only once every attack and power on it plays correctly (as MAGIC does) |
| 3 | The special trainers' cards | **The era's real promos**: Wizards Black Star Promos (Gym / Neo era) for KANTO's and JOHTO's trainers, Nintendo Black Star Promos (e-Card era) for HOENN's new ones, SINNOH's and UNOVA's; each handed out by one trainer only |
| 4 | Rules | **One ruleset, extended**: today's rules everywhere, each newer mechanic added as its cards need it (one Stadium in play, Pokémon Tools, one Supporter a turn, Darkness / Metal Energy, Baby rule, Poké-Powers / Poké-Bodies...) |

## The sets (PokemonTCG/pokemon-tcg-data, cards/en/<id>.json; scans from images.pokemontcg.io)
Cached in ~/.cache/moemon/tcgdata (never committed).

| Region | Set | id | Cards | Notes |
|---|---|---|---|---|
| KANTO | Gym Heroes | gym1 | 132 | Owner's Pokémon, 7 Stadiums, Rocket's Secret Machine |
| KANTO | Gym Challenge | gym2 | 132 | 7 Stadiums |
| JOHTO | Neo Genesis | neo1 | 111 | Darkness / Metal types and Energy, Baby, Pokémon Tools, Stadiums |
| JOHTO | Neo Discovery | neo2 | 75 | |
| JOHTO | Neo Revelation | neo3 | 66 | Shining Pokémon |
| JOHTO | Neo Destiny | neo4 | 113 | Light / Dark Pokémon, Shining |
| HOENN / SINNOH / UNOVA | Expedition Base Set | ecard1 | 165 | Poké-Powers / Poké-Bodies, Supporters, Technical Machines |
| HOENN / SINNOH / UNOVA | Aquapolis | ecard2 | 182 | Crystal-era types, TMs, Stadiums |
| HOENN / SINNOH / UNOVA | Skyridge | ecard3 | 182 | Crystal Pokémon |
| promos | Wizards Black Star Promos | basep | 53 | KANTO / JOHTO special trainers |
| promos | Nintendo Black Star Promos | np | 40 | e-Card era special trainers (8 are EX: check they fit rule 4) |

Total 1,158 set cards (about 1,100 attacks with effect text, 250 Pokémon Powers / Bodies) and 93 promos.

## What the engine has today (to extend)
- 442 cards (TCG_CARD_COUNT: the first Game Boy game's 228, GB2's 214), 7 types (no Darkness / Metal), Pokémon /
  Energy / Trainer kinds only (no Stadium, Tool, Supporter, TM), Pokemon Powers only (no Poké-Bodies), 11 boosters
  (TCG_PACK_TYPES), the collection's bitsets sized by TCG_CARD_COUNT ("<save>.cards" format 6).
- Effects: tcg_effects.c FX_* kinds (the original's command lists), GB2's Gb2Effect, tcg_powers.c, tcg_trainers.c;
  choices through TcgAction (target, option, selections) and tcg_flow.c steps (TcgFlow_AttackSteps / TrainerSteps /
  PowerSteps), the board and the phone sharing them.
- Regional special trainers (tcg_field.c QzSpecial): KANTO / JOHTO / SINNOH ones stand in for HOENN's (same deck,
  promo and "beaten" mark). To become their own (decisions 3).

## Work plan (one release at the end, decision 1)
1. Audit today's 442 cards: every place the engine picks for the player (tcg_effects.c: Energy Spike, Gigashock,
   Call for Family, Energy Conversion / Absorption, Hyper Beam / Whirlpool and DiscardFoeEnergy, DiscardEnergy's
   "any Energy" costs; then GB2's Gb2Effect, the Trainers and the Powers) becomes a choice on the board, the phone and
   the AI. (Can ship with other fixes before the sets.) **Done 2026-11-18 (1.204.6)**: COOP_NOTES "The card audit:
   every choice is the player's".
2. Data model: card count, types (Darkness, Metal), subtypes (Stadium, Tool, Supporter, TM, Baby, Owner's, Shining,
   Light / Dark, Crystal), Poké-Powers / Bodies, the collection format (migration), boosters per region.
3. Importer (pokemon-tcg-data -> catalog), printed art (fetch_art.py / build_printed_art.py), a coverage report like
   MAGIC's (docs/TCG_REGIONS_SCOPE.md: which cards fully play). The report exists (tools/tcg/regions/scope.py, matching
   wordings against today's catalogue): 2026-11-18, 305 of 1,251 cards play with today's effects; 946 wait on 899
   distinct wordings (798 attack texts, 271 Poke-Powers / Bodies, 187 Trainer / Energy rules), most of them single.
4. Effects by families, tested on staged boards; the board's new zones (a Stadium, Tools: **mock-ups first**). The
   board mock-up (2026-11-18, waiting for the user): docs/tcg_regions/board_mockup.png, drawn by the board's own code
   (`mp cards mock 2`, tcg_board.c MockUnder / MockOver over a staged duel): the Stadium a plate on the centre line,
   the turn's Supporter face up beside its player's Active, Tools tucked under their Pokemon, a BRN badge, Darkness /
   Metal orbs. **Approved and built** (2026-11-18: docs/tcg_regions/board_table.png from a staged duel).
   **Status (2026-11-18): 1,122 of 1,227 finished.** Where each part lives:
   - Attacks: tools/tcg/regions/families.py (regex families + EXACT, one script per wording; ops in
     src/tcg/tcg_script.inc, listed at its top and in each op's comment).
   - Trainers: src/tcg/tcg_rtrainers.inc (rules), tcg_rtrainers_flow.inc (steps, playability, menus),
     tcg_rtrainers_ai.inc (AI); effects.py RTRAINERS lists the first printings handled (reprints follow: TcgCard.canon).
     Supporters (one a turn), Stadiums (TcgPlayer.stadium), Tools (attached, TcgDuel_Tool) in TcgDuel_Trainer.
   - Powers: tools/tcg/regions/powers.py (id, used / passive, flags, script) -> include/tcg/region_powers.h and
     src/tcg/tcg_region_powers.inc (generate_effects.py); rules src/tcg/tcg_rpowers.inc, steps / AI at the ends of
     tcg_rtrainers_flow.inc / tcg_rtrainers_ai.inc.
   - Rules: special Energy (Tcg_EnergyRule), Baby (TcgDuel_EvolvesFrom, the coin), Shining / Miracle one a deck, ex 2
     Prizes, Burned. Technical Machines: attached by TcgDuel_Trainer where their words allow (TcgDuel_TmFits: "your
     Darkness Pokemon", "your Evolved Fire, Lightning, or Fighting Pokemon"), their attack the action's attack
     TCG_ATTACK_TM (TcgDuel_AttackOf everywhere an attack is looked up: rules, flow, board's fourth row, AI, phone's
     "i" index), discarded in EndTurn. Effects that last until Benched / evolved: TcgFieldCard stuck / blindAll /
     noAttackAll, cleared wherever a Pokemon leaves the Active spot. Crystal Type (R_CRYSTAL_TYPE: the types its words name, as Shift for the turn; one of a Crystal Pokemon a deck).
   - Stadiums used during a turn (Healing Field, Power Plant...): TCG_USE_STADIUM, A on the Stadium's plate (USE /
     CHECK), steps TcgFlow_StadiumSteps, rules RegionStadiumUse / RegionStadiumBlock (tcg_rtrainers.inc), AI
     RegionStadiumAi; once a turn (TcgPlayer.stadiumUsed) but Celadon / Saffron City Gym.
   - Attack options (opt:K: YES / NO, a Special Condition of the player's choice, either deck, how many Energy) through
     TcgDuel_AttackOptions; attack target kinds 5 (their undamaged Bench) and 6 (any damaged one of theirs); picks
     TCG_APICK_FIELD_MOVE / DISCARD_CARD / SEARCH / HAND_ENERGY.
   - Tests: tools/tcg/test_regions.c (every finished attack directly and built through the flow with every option,
     Trainer, used power and Stadium through the flow, every Technical Machine attached and its attack used, AI duels).
   - Printed cards (CARD ART): graphics/tcg/regional.bin (tools/tcg/regions/build_art.py: `--fetch` keeps each scan
     at 300x412 in ~/.cache/moemon/tcgart/regions; the illustration's window from LAYOUTS by era and kind (WotC, e-Card,
     the EX-era promos), each edge moved to the card's own frame line within 10 scan pixels, 6 in; `--check SHEET`
     draws samples), built in by tcg_art_data.c; tcg_gfx.c PrintedEntry reads ids past 442 from it (the back stays
     printed.bin's), CleanArtCorners fills the e-Card windows' rounded corners. The phone: android/build_assets.py
     joins it after printed.bin into one tcg_cards.bin (joined_packs), so CardPack.java reads ids past 442 unchanged.
   - Special trainers: tools/tcg/regions/specials.py (67: JOHTO 16, KANTO 15, SINNOH 16, UNOVA 20; decks from their
     region's finished cards, a promo each) -> gen_special_decks.py -> include/tcg/region_specials.h, tcg_decks.c;
     tcg_field.c sQzSpecials in the same order (people by their sprite: UNOVA's by Black's object code); rated against
     the round robin by rate_decks.c. Rerun gen_special_decks.py whenever cards finish (their decks change).
5. Per region: boosters, shops, NPC decks (built from the region's sets and today's), special trainers with their own
   decks and promos (UNOVA's too), CARD SCALING ratings rerun, the card guide rebuilt.
6. Retroactive promos: a courier NPC brings each promo whose (newly special) trainer the save's card record shows as
   beaten (TcgCollection's records by the opponent's key; as mp_battleisland.c's courier / mp_tmrefund.c), once.
7. The GEN switch (POKéMON / MAGIC by set, YU-GI-OH! by summon type, FUSION the base): every set a GEN step from the
   catalogue's own set list, so a new set needs no switch code; mock-ups of where it's set first.
