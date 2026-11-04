# Magic: The Gathering card duels: decisions

Planned, not started (2026-11-03). A fourth card game beside the POKéMON cards, Yu-Gi-Oh! and Triple Triad, joining
what the three share (COOP_NOTES "The three card games together"). The user's answers first (keep to them), then
what was proposed and not yet confirmed, then what is still open.

## The user's answers (2026-11-03)
1. Four card games are fine: Magic joins the CARD CASE as its fourth side, through the systems the three share.
2. Magic is the less popular game: a niche hobby with few players in the world, so the other three keep their
   opponents.
3. Where it is played: late-game places that held a feature only one version had.
   - HOENN: MOSSDEEP's Game Corner (MOSSDEEP_CITY_GAME_CORNER_1F / B1F). In Emerald it only explains the two Wireless
     Adapter minigames (POKéMON JUMP, DODRIO BERRY PICKING: cable_club.inc MossdeepCity_GameCorner_1F_EventScript_InfoMan2)
     and Kai never brought those two online, so the building is free. It already looks like a card shop.
   - JOHTO: a copy of MOSSDEEP's Game Corner in CIANWOOD.
   - KANTO: another copy of MOSSDEEP's Game Corner in SAFFRON (the user, 2026-11-04; not Two Island's Joyful Game
     Corner, which keeps FireRed's story).
   - SINNOH: the PAL PARK on ROUTE 221 reopens as a card hall where all four games are played (POKéMON cards,
     Yu-Gi-Oh!, Triple Triad, Magic). The user accepts original dialogue and scripts there: an exception to "SINNOH's
     words and scripts are Platinum's own", for the PAL PARK only. Its catching show stays closed. Today it is left out
     by tools/sinnoh/places.py (DROPPED: PAL_PARK) and gen_scripts.py (the catching show), and ROUTE 221's sign says
     "PAL PARK|Coming soon!" (tools/sinnoh/words_outdoor.py Sign7).
4. 1 vs 1 is the main format. BOOSTER DRAFT is an optional Friend Duel format.
5. Magic is hidden in every game picker until someone gives the player a starter deck.
6. As soon as the player has a deck, NPCs who play Magic start offering Magic duels. Before that, no NPC mentions it.
7. In every region's venue the main floor and B1F are where most Magic duelists gather; only a few are out in the
   world.
8. Card art from an API: Scryfall (see below).

## Proposed, not yet confirmed
- Card pool: Premodern (4th Edition / Ice Age, 1995, to Scourge, 2003): Magic's fan-revived classic format, the
  counterpart of the Yu-Gi-Oh! side's 2002-2006 pool (Goat Format era). The same calendar years in Magic (Onslaught to
  Ravnica) were a weaker time for the game (Mirrodin's Affinity bans, Kamigawa).
- Starter decks: Portal / Starter 1999 / Starter 2000, made for teaching (the four Starter Decks' place). The
  1997-2003 theme decks (precons) are the Structure Decks' counterpart at the shops.
  (Premodern has only the cards Portal / Starter reprinted from other sets: 91 of Portal's are in the pool. Portal-only
  cards, like Alaborn Grenadier, would join the pool for those decks, as the Yu-Gi-Oh! side took in SDD's cards.)
- Today's card wording and rules (Oracle, today's mulligan), as the Yu-Gi-Oh! side uses today's rulings.
- Opponents in the spirit of Shandalar (MicroProse, 1997): a handful of named players around each region, a color and
  a signature card each, special duelists rather than whole trainer classes. The five colors map onto types (red Fire,
  blue Water, green Grass, black Ghost / Dark, white Normal / Fairy).
- Hidden means: the CARD CASE's L/R round skips Magic, the which-game menu (tcg_field.c) leaves it out, HOW TO PLAY
  has no Magic pages, and friends' duel / trade requests offer it only when both players have it.
- MOSSDEEP: 1F the shop, B1F the room where the regulars play.
- The board drawn at 480x320 like a VS fight or the BADGE CASE: a Magic table has no zone limit.
- An auto-pass stronger than the Yu-Gi-Oh! table's quiet chain windows: Magic offers priority at almost every step.

## The rules engine (the user, 2026-11-04)
- Forge translated into C: "the best of both worlds". No Java runtime: Forge's rules (forge-game) written as C in
  src/mtg/, reading Forge's own card scripts (cardsfolder, its text language: `A:SP$ DealDamage | ValidTgts$ Any |
  NumDmg$ 3`) for the cards of our pool only ("I don't need every card ever made, just our selection"). Forge is
  GPL-3: credits in dist_extras/, as EDOPro's AGPL ones.
- Forge's AI too (forge-ai, the user 2026-11-04), translated the same way, under the shared AI levels.
- Built to fit the other three: deterministic from the answers (online duels by answers, like Yu-Gi-Oh!'s), the
  shared AI levels, presentation events for the table, and the phone app through JNI like tt / tcg / ygo.

Built from 2026-11-04: docs/MTG_HANDOFF.md (status and next steps), docs/MTG_SCOPE.md (how many cards play).

## Choices made while building (1.95.0; the user can change any)
- The starter comes from MOSSDEEP's clerk (the counter man): a CARD CASE first if needed, then one of five 40-card
  single-color starters of the game's own (EMBER, TIDEWATER, NIGHTFALL, WILDWOOD, DAYBREAK), built from the cards
  the engine plays. The proposed Portal / Starter decks wait until enough of their cards play.
- Decks of 40 cards at least (the starters' size, quicker duels), four of a card but basic lands, which are free.
- The regulars: six at MOSSDEEP with names, words and decks of their own (two upstairs, four in the play room). B1F is
  reached through the clerk (1F's own stairs are behind the counter); its stairs come up by the front door.
- A win brings a booster of 8 (5 commons, 2 uncommons, an uncommon or one in four a rare); a loss costs nothing.
- The table is 240x160 like the other card games' for now; the proposed 480x320 stays open.

## The user's answers on the look (2026-11-04, 1.96.0)
- Every MAGIC screen matches the other card games' in its tiny details (layout, colors, wording, keys).
- The table plays like the POKeMON card table's: its mat, hand tray and field; no POKe BALL-like mark in the middle.
- The opponent's cards face them (turned half round), as on the POKeMON and YU-GI-OH! tables.
- TRIPLE TRIAD is always last on the CARD CASE's L / R round: POKeMON, YU-GI-OH!, MAGIC, TRIPLE TRIAD.
- Card art: each card's earliest old-frame printing from 4th Edition to Scourge (the pool's years), in the game's own
  frame; the artist credited in the card view.
- The table passes for the player when nothing else can be done, when their own spell is on top, and outside their
  main phases, the blocks and the opponent's end step; L held stops at every chance.

## Open
- Which NPCs out in the world play Magic, and how many (only MOSSDEEP's regulars so far).

## Card art: Scryfall
- api.scryfall.com has every printing of every card (checked 2026-11-03 with 4th Edition's Lightning Bolt):
  image_uris png (745x1040), large, normal, border_crop, art_crop; `frame` ("1993" = the old frame), `artist`,
  `highres_image`. The card back too.
- Like the other games' printed art: tools/mtg/fetch_art.py fetches once (a named User-Agent and Accept header, a
  short pause between requests, as Scryfall asks) into a built-in graphics/mtg/printed.bin; the game never goes online
  for art. Each card shows its old-frame printing from the pool's years; artist credits from the data.
- Wizards' Fan Content Policy: free fan content may show the cards; never sold, the artist and copyright line not
  cropped off (the full printed card keeps both). The old frame's art box is the same on every card: one measured
  rectangle for the shared foil.
