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
   the AI. (Can ship with other fixes before the sets.)
2. Data model: card count, types (Darkness, Metal), subtypes (Stadium, Tool, Supporter, TM, Baby, Owner's, Shining,
   Light / Dark, Crystal), Poké-Powers / Bodies, the collection format (migration), boosters per region.
3. Importer (pokemon-tcg-data -> catalog), printed art (fetch_art.py / build_printed_art.py), a coverage report like
   MAGIC's (docs/TCG_REGIONS_SCOPE.md: which cards fully play).
4. Effects by families, tested on staged boards; the board's new zones (a Stadium, Tools: **mock-ups first**).
5. Per region: boosters, shops, NPC decks (built from the region's sets and today's), special trainers with their own
   decks and promos (UNOVA's too), CARD SCALING ratings rerun, the card guide rebuilt.
6. Retroactive promos: a courier NPC brings each promo whose (newly special) trainer the save's card record shows as
   beaten (TcgCollection's records by the opponent's key; as mp_battleisland.c's courier / mp_tmrefund.c), once.
