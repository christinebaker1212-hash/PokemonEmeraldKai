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
   - SINNOH: the PAL PARK on ROUTE 221 reopens as a card hall where all four games are played (POKéMON cards,
     Yu-Gi-Oh!, Triple Triad, Magic). The user accepts original dialogue and scripts there: an exception to "SINNOH's
     words and scripts are Platinum's own", for the PAL PARK only. Its catching show stays closed. Today it is left out
     by tools/sinnoh/places.py (DROPPED: PAL_PARK) and gen_scripts.py (the catching show), and ROUTE 221's sign says
     "PAL PARK|Coming soon!" (tools/sinnoh/words_outdoor.py Sign7).
4. 1 vs 1 is the main format. BOOSTER DRAFT is an optional Friend Duel format.
5. Magic is hidden in every game picker until someone gives the player a starter deck.
6. As soon as the player has a deck, NPCs who play Magic start offering Magic duels. Before that, no NPC mentions it.

## Proposed, not yet confirmed
- Card pool: Premodern (4th Edition / Ice Age, 1995, to Scourge, 2003): Magic's fan-revived classic format, the
  counterpart of the Yu-Gi-Oh! side's 2002-2006 pool (Goat Format era). The same calendar years in Magic (Onslaught to
  Ravnica) were a weaker time for the game (Mirrodin's Affinity bans, Kamigawa).
- Starter decks: Portal / Starter 1999 / Starter 2000, made for teaching (the four Starter Decks' place). The
  1997-2003 theme decks (precons) are the Structure Decks' counterpart at the shops.
- Today's card wording and rules (Oracle, today's mulligan), as the Yu-Gi-Oh! side uses today's rulings.
- Opponents in the spirit of Shandalar (MicroProse, 1997): a handful of named players around each region, a color and
  a signature card each, special duelists rather than whole trainer classes. The five colors map onto types (red Fire,
  blue Water, green Grass, black Ghost / Dark, white Normal / Fairy).
- Hidden means: the CARD CASE's L/R round skips Magic, the which-game menu (tcg_field.c) leaves it out, HOW TO PLAY
  has no Magic pages, and friends' duel / trade requests offer it only when both players have it.
- MOSSDEEP: 1F the shop, B1F the room where the regulars play.
- The board drawn at 480x320 like a VS fight or the BADGE CASE: a Magic table has no zone limit.
- An auto-pass stronger than the Yu-Gi-Oh! table's quiet chain windows: Magic offers priority at almost every step.

## Open
- The rules engine, decided first: Forge (Java, GPL) or XMage (Java, MIT) run beside the game the way ygo_edo.c drives
  EDOPro (a bundled Java runtime, roughly 50 MB), or an engine of our own in C for the fixed pool, built in stages.
- Who gives the starter deck, and where (MOSSDEEP's shop is the natural place).
- Which NPCs play Magic, and how many.
- Card art and the card back.
