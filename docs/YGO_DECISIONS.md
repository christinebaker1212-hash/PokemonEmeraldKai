# Yu-Gi-Oh! card duels: decisions

The user's answers to the 30 design questions (keep to them), then the choices made while building.

## The user's answers
1. Stairway to the Destined Duel (GBA) is the source: its card list and Forbidden/Limited list, read from the
   player's ROM by tools/ygo/import_sdd.py. The ROM is never committed; the derived data is.
2. 8000 Life Points.
3. The first player doesn't draw and can't attack on their first turn.
4. Today's rulings.
5. Today's card wording.
6. A Fusion Deck of up to 15 cards.
7. Stop to ask only when a response is possible.
8. No side deck; single duels.
9. Every English booster to the end of 2003: LOB, MRD, SRL, PSV, LON, LOD, PGD, MFC, DCR (and the four Starter Decks).
10. Cards from those packs that SDD lacks are in the pool anyway.
11. SDD cards with no 2003 pack: the card shop and the special duelists.
12. The same CARD POINTS as the Pokemon cards.
13. Rarities shown as printed: Rare a silver name, Super shiny art, Ultra gold, Secret a rainbow.
14. Who plays Yu-Gi-Oh!: Psychics, Hex Maniacs, Ninja Boys, Black Belts, Battle Girls, Cool Trainers, Ruin Maniacs,
    Kindlers, Collectors, Experts, Aqua / Magma grunts, most teen townsfolk. Pokemon cards: kids, Bug Catchers,
    Swimmers, Fishermen, Hikers, Picnickers, Lasses, Sailors, Aroma Ladies, Beauties, older townsfolk.
15. Both games (they ask which): the CARD CLUB staff, the rival, WALLY, STEVEN.
16. Yu-Gi-Oh! decks themed by who the duelist is, stronger with badges.
17. The Elite Four, MAXIE, ARCHIE and the Frontier Brains play Yu-Gi-Oh! (SIDNEY Fiends, PHOEBE Zombies, GLACIA Aqua,
    DRAKE Dragons, MAXIE Pyro, ARCHIE Fish / Sea Serpents); Gym Leaders keep Pokemon cards.
18. Nods to the show's duelists within a type.
19. A signature card as the first-win prize.
20. Today's table layout.
21. Card-game effects like the Pokemon table's.
22. Skip phases where nothing can happen.
23. A Game Boy-style frame around the printed illustration.
24. A Yu-Gi-Oh! duelist joins the CARD CLUB, and both CARD CLUB NPCs move to the POKéMON MARTS.
25. Pick one of the four real Starter Decks.
26. A teaching duel is offered.
27. Yu-Gi-Oh! duels and trades with friends.
28. Rewards like the Pokemon cards': a booster every win, one card of their deck daily.
29. A YU-GI-OH! achievements tab.
30. Ship in stages.

## Choices made
- Pull rates: the rare slot is a Super Rare 1 pack in 6, Ultra 1 in 12, Secret 1 in 24; short prints weigh half a
  common, super short prints a quarter. 9 cards a pack.
- Printed art stored at 160x233 (the big picture shows a card about 150 pixels tall).
- The CARD CLUB's Yu-Gi-Oh! duelist uses the MAN_5 sprite; the CARD CLUB members stand in OLDALE's and MAUVILLE's
  POKéMON MARTS and LILYCOVE's DEPARTMENT STORE 1F.
- The ELITE FOUR switching to Yu-Gi-Oh! would make their Pokemon promo cards unobtainable, so beating one at
  Yu-Gi-Oh! also hands over their old promo once (Tcg_GivePromoOf).
- '#' in card names becomes "No." (the font has no '#').
- Answer 7 and free-chain cards: Reverse Trap, Waboku and the like can legally be activated at every window, and
  asking at each phase change made duels tedious. The table passes for the player at quiet moments (a phase
  changing with no chain, summon, attack or Damage Step) unless L is held; cards made for such a moment (YF_PHASE)
  are still asked. The engine still offers every window, so online duels stay in step.
- CARD BACKS: PRINTED shows the real English back (Yugipedia's Back-EN.png scan), PIXEL the drawn swirl; the one
  option covers both games.
- Stage 1 = the Starter Decks, LOB and MRD (their effects written); later sets join as their effects are written
  (Ygo_SetAvailable needs 90% of a set ready).
