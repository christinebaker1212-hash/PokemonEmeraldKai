# Triple Triad: decisions

The user's request (the third card game), how each part was read, and the choices made while building it.

## The request
- Triple Triad as another card game, from the FF8 decompilation (github.com/roengstrom/ff8-decomp): its rules,
  "the local rulesets, as well as their spread across Hoenn, and the ability to manipulate rulesets and cause
  beneficial or negative rulesets to spread".
- Deckbuilding at a smaller scale, no deck recipes, categories sorted by card stars.
- Who plays Pokemon cards, Yu-Gi-Oh! or Triple Triad (some more than one), with unique dialogue.
- A Triple Triad NPC in the GAME CENTER card shop, and NPCs nearby who play.
- Every card of Final Fantasy XIV (not FF8's), both picture sizes from v2.xivapi.com.
- Sellers: each sells a small curated collection, never more than 15% of the cards. "All Triple Triad cards must be
  earned from NPCs." Packs sold by rarity, the higher rarities never sold.
- Triple Triad achievements. Snazzy card animations on the table.
- Losing never costs the player cards; only the player takes cards from the loser.

## How it was read
- "All cards must be earned from NPCs" next to "every vendor sells a curated collection" and "packs sold by
  rarity": every card can be won from someone (tools/tt/test_hands.c checks it: over 60 days every one of the 475
  cards turns up in a hand), while the sellers and packs offer some of them for money too. Packs of the two
  highest rarities (MYTHRIL four stars, IMPERIAL five) are never sold.
- "Smaller scale" deckbuilding: XIV's decks, five different cards, at most two of four stars or more and one of five
  stars (XIV's limits). Six deck slots. No DECK RECIPES on this side; the card list and the builder are tabbed by
  stars.
- FF8's rules from the decomp, as its code does them: SAME (with SAME WALL: an A against the edge), PLUS, the plain
  capture, ELEMENTAL's +1 / -1 (plain capture only), and the COMBO: when SAME or PLUS took a card, every card taken
  in that play captures from where it lies, again while anything is taken (applyCardRules and its loop). OPEN,
  RANDOM, SUDDEN DEATH as FF8. FF8 elements don't exist on XIV cards: ELEMENTAL uses XIV's four card types
  (PRIMAL, SCION, SOCIETY, GARLEAN), 30% of squares carry one (resetTriadBoard's odds).
- XIV's own rules are in too (THREE OPEN, REVERSE, FALLEN ACE, ASCENSION, DESCENSION, ORDER, CHAOS, SWAP). The
  regions start with FF8's rules only; XIV's arrive through the CARD QUEEN ("a rule from a far-off land") and then
  spread like any other.
- Trade rules (FF8's ONE / DIFF / DIRECT / ALL) only ever apply to the player winning: DIRECT takes the cards of
  theirs that ended blue, DIFF as many as the score difference.

## The rules spreading (FF8's mechanic in HOENN)
- Eight regions by map section (src/tt/tt_field.c RegionOfMapsec): PETALBURG, RUSTBORO, SLATEPORT, MAUVILLE,
  LAVARIDGE, LILYCOVE, SOOTOPOLIS, EVER GRANDE, each with starting rules like FF8's regions (PETALBURG: OPEN, as
  Balamb; EVER GRANDE: nearly all of them, as the Lunar Base) and a TRADE RULE.
- The player carries the rules of the region they last played in (Tt_PlayedIn). Challenging someone in another
  region: one carried rule the region lacks may spread there (30%), else one of its rules the player doesn't carry
  may be abolished (20%). At most seven rules in a region.
- The manipulation: carrying (play in a region with the rules you want, then challenge people where you want them),
  rejecting a challenge after the rules changed (the change stays, as in FF8), and the CARD QUEEN: where she is,
  spreading is 50% and abolishing 40%; beaten, she grants a favor (change the region's TRADE RULE, or abolish one
  of its rules); paid 5000 she travels to the region you name, three times in ten bringing a rule the region lacks
  (FF8's or XIV's). She also moves on by herself every three days. Rules lean "skill" (OPEN, SAME, PLUS...) or
  "luck" (RANDOM, CHAOS, SWAP...): the screens color them blue or orange.

## Who plays what
- Pokemon cards: kids, outdoorsy and sea folk, older townsfolk (unchanged). Yu-Gi-Oh!: teens and young adults, the
  mystic and fighting classes, the Teams (unchanged).
- Triple Triad: the worldly and well-mannered, travelers, students. Only Triple Triad: GENTLEMAN, LADY, PARASOL
  LADY, BIRD KEEPER, DRAGON TAMER, PKMN RANGER, PKMN BREEDER, YOUNG / OLD COUPLE, INTERVIEWER, and the gentleman /
  bard townsfolk. Besides Pokemon cards (they ask which): SAILOR (they carry rules from port to port), TRIATHLETE,
  SCHOOL KID, RICH BOY, BEAUTY, POKEFAN, WINSTRATE and the matching townsfolk (scientists, DEVON workers,
  reporters, artists...). Besides Yu-Gi-Oh!: COLLECTOR, COOLTRAINER, RUIN MANIAC, EXPERT. All three: the CARD CLUB,
  the rival, WALLY, STEVEN.
- Six ways of talking (gentry, travelers, students, romantics, scholars, collectors): three yes lines, two no lines
  each, and their own "which game?" lines; the special players have their own.
- Special players with five-star cards: STEVEN, the rival, WALLY, WALLACE, JUAN, WINONA, NORMAN, ROXANNE, TATE &
  LIZA, MR. STONE (DEVON 3F), MR. BRINEY (his house), SCOTT, PROF. BIRCH, MOM, the CARD QUEEN. Every five-star card
  belongs to one of them (tools/tt/gen_pools.py), and a hand rotates through them day by day. Some nods: STEVEN's
  DIAMOND WEAPON, WALLACE's SAPPHIRE WEAPON, WINONA's EMERALD WEAPON, SCOTT's RUBY WEAPON, TATE & LIZA's twins
  (Alphinaud & Alisaie, 2B / 9S / 2P), MOM's Hildibrand.

## Places
- Sellers in the POKeMON CENTERS of PETALBURG, SLATEPORT, MAUVILLE, FALLARBOR (LAVARIDGE's region), LILYCOVE,
  MOSSDEEP (SOOTOPOLIS's) and EVER GRANDE at (13,4), and RUSTBORO's at the GAME CENTER 2F (11,6), in front of the
  shelf, with a BARD and a SCHOOL KID playing beside it (12,8) / (12,9). The CARD QUEEN at (0,5) of the same
  POKeMON CENTERS (and RUSTBORO's), shown only in her region (FLAG_TT_QUEEN_AWAY, set by Tt_MapSetup on entering).
- Shelves: the region's own cards of one to three stars, its themed ones first, 13 to 34 cards each (the limit is
  71, 15% of 475). One copy of each single per customer. Prices: singles 400 / 1000 / 2600 by stars; packs
  BRONZE 600, SILVER 1600, GOLD 4000 (three cards of their stars). Spares sell for XIV's sale value.

## The table
- XIV's layout: your hand (blue) on the left, theirs (red) on the right, the board between; numbers in a diamond at
  the bottom of each card, so the hand stacks from the bottom up and every card's numbers show. Deal, the rules,
  a light running between the names to pick who starts, cards arcing down with dust and a shake, flips about the
  side they were taken from with sparks in the new color, SAME / PLUS / COMBO bands sweeping across, ASCENSION's
  +1s floating up, YOU WIN with letters bouncing in and confetti.
- The card back is the user's (docs/user_images/46: XIV's own icon is a placeholder). Large cards (80 wide and up)
  show the user's large numbers (docs/user_images/47) with FF8's own A from its menu sheet (48); mid-sized cards (a
  pack's, the strip's) the same at 7x10, shrunk once at build time; the board's 40x49 cards keep the 5x7 pixel font,
  its pixels whole screen pixels. Faces between the board's size and full size are averaged down on the CPU
  (tt_gfx.c Shrunk), not left to the renderer's two-sample scaling, which made them jagged.
- Friends: a game between friends trades no cards whatever the TRADE RULE (only the player ever takes cards, and
  between two players neither would be "the player"), and spreads no rules (the regions are each save's own). The
  one who asks brings the rules of the region they're in. Trades: any card beyond the deck in use, one for one.
- The computer player: negamax with alpha-beta (FF8's searchBestMoveStack does a depth search too), to the end of
  the game when six squares or fewer are left; four levels.
