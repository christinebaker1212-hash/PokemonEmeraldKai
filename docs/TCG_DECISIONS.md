# Card duels 2.0: decisions

Agreed with the project owner (October 2026) after the 1.27.0 "Let's Duel!" release. They
override `TCG_IMPLEMENTATION.md`'s "Agreed scope" where they differ.

## Asked for directly
- The card screen fills the screen and uses the game's own scaling (same rect, scale mode and
  filter as the overworld), and it is part of the game frame, so friends spectating see it and
  side panels are never covered.
- Mat: the Poke Ball circle must not look stretched, BENCH once, the Prize spots must match where
  the Prize cards sit; zones redrawn to fit the cards placed there; a mat per trainer type.
- No cards from MoeGatcha: only from duels with NPCs or players.
- Special trainers have special decks; NPCs are challenged FF8-style with fitting decks.
- The first deck arrives with more pizazz; the NPC simply gives the CARD CASE (no "it won't affect
  anything" pop-up).
- A reward for collecting every card, and spare cards exchanged for rewards (MoeGatcha has been
  pulled 400+ times, so shinies and rare Pokemon are not special to this player).
- The game's own sound effects.
- A Prize you take is shown to you.
- B never concedes: B asks "End turn?". CONCEDE replaces the END TURN button.
- The opponent's cards face them.
- Zoom uses the printed card art; an option switches to the Game Boy art (printed is the default).
  Later (1.28.1): the printed cards come with the update (1.28.0 never shipped them), and CARD ART
  has three settings: the full printed card, the printed illustration in the Game Boy frame (the
  default), or the Game Boy picture in the frame, with the illustration's place measured on the
  printed cards. The card backs can be the real printed back too (CARD BACKS).
- Hand-changing cards (PROFESSOR OAK...) scroll down to the hand to show the effect.
- After a Knock Out the screen moves to the Bench.
- Challenges only with the CARD CASE registered to a button; that button challenges the NPC in
  front, or opens the Card Case when nobody is there. 30-40 phrases by trainer type, for people
  with and without cards, in the voice of their in-game dialogue. A YES opens the menu.
- The online challenge flow and every card menu are remade.
- Achievements for cards.

## Answers (the recommended option every time)
| # | Question | Answer |
|---|---|---|
| 1 | Who can be challenged | Most people: every Trainer-class NPC and many townsfolk; some say no |
| 2 | Unbeaten Trainers | Only after beating them in a Pokemon battle |
| 3 | Deck strength | Themed by trainer class; CARD SCALING (on at first) fits the opponent's deck to the player's deck and results, special trainers a little above; off: stronger with each badge |
| 4 | FF8 card-taking | The winner takes one card from the loser's deck; the player never loses cards |
| 5 | Which card | You choose, face up (FF8 "One") |
| 6 | Rematches | Unlimited; the full reward once a day per NPC, later wins that day a booster |
| 7 | Special decks | Gym Leaders, Elite Four and Champion, May/Brendan, Wally, Maxie and Archie, Steven, Frontier Brains |
| 8 | Special deck content | Their own Pokemon and type, the GB Club Master decks as a base |
| 9 | Promo cards | One exclusive promo card per special trainer |
| 10 | Gym Leaders accept | After you have their badge |
| 11 | The button | The CARD CASE registered like any key item (SELECT / L / R / extra) |
| 12 | Phrases | 30-40 in total, shared by class, for card players and people who don't play |
| 13 | Special trainers' lines | Their own, from their in-game dialogue |
| 14 | After YES | A challenge window: name, deck Energy types, the reward, then pick a deck |
| 15 | No cards | One line in the normal message box |
| 16 | Challenging a friend | Card Case button in front of them or CO-OP menu > name; YES/NO like a trade; both pick a deck |
| 17 | Online reward | A booster for the winner, nothing lost |
| 18 | Card trading | A card trade screen |
| 19 | Spectators | See exactly your screen; face-down cards stay hidden |
| 20 | In the overworld | A card emote above both duellists, who stay put |
| 21 | Screen size | The game's own area and scaling |
| 22 | Mats | About 10 themed mats (beach, forest, cave, city, gyms, League, Aqua, Magma, online...) |
| 23 | Mat style | A cloth play mat with a motif |
| 24 | Prizes | 2 x 3, slightly overlapping, as in docs/user_images/37.webp |
| 25 | Opponent's cards | Turned to face them; HP plates and damage numbers upright |
| 26 | Art setting | OPTIONS > DISPLAY > CARD ART, used everywhere |
| 27 | Prize shown | Flips over big in the middle, then slides into the hand |
| 28 | Music | Matches the opponent (Gym Leader music for leaders, trainer music, a calm theme online) |
| 29 | Full collection | A unique Moemon gift, a gold CARD CASE and a star on the TRAINER CARD |
| 30 | Spare cards | Card Points at a counter: boosters, a chosen card, items or MoeGatcha tickets |
| 31 | Milestones | Small rewards at 25 / 50 / 75 % of each set |
| 32 | Achievements | A CARDS tab: first win, 100 wins, every Gym Leader, every special trainer, a set, all 228, 50 packs, a win without giving up a Prize, a friend beaten, a card trade, a single-type deck win, a promo card |
| 33 | Getting the case | A card club member in a POKeMON CENTER hands it over in dialogue, offers a teaching duel, then three decks flip over to pick from |
| 34 | Teaching duel | Offered once, can be skipped |
| 35 | Menu style | The game's own style in full colour (like the OPTIONS tabs and the BAG) |
| 36 | Deck builder | A grid of card pictures with counts, type tabs on top, the deck down the side |
