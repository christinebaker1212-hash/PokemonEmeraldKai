# Pictures from the user

Every picture the user sent, kept so a later session can look at them when the task they belong to comes up.
Numbered in the order they were sent. Add new ones with the next number and a line here.

| # | Sent with | Used for |
|---|-----------|----------|
| 01-03 | "add the ability to have follower Pokémon behind you... every sprite needed for every Pokémon" | follower sprite sheets (followers) |
| 04-06 | "A number of fixes": use the default cursor, text overflow in the shared PC | shared PC fixes |
| 07-08 | "the online menu looks fine indoors, weird outdoors" | co-op menu look outdoors |
| 09 | "the background of the gatcha, the black part, drifts with the rest" | MoeGatcha background fix |
| 10 | "After speaking to an NPC once, all text in messages is invisible" | text bug |
| 11-13 | new overworld / trainer sprites, menu frames not in Emerald | LOOK sprites, frames |
| 14 | trainer back sprites, cut to Emerald's size and frames | back pictures |
| 15 | pixel graphics for button labels | controller button icons |
| 16 | the gap between the message box and the window on co-op menus | co-op menu frames |
| 17 | the app's icon | icon |
| 18 | Poké Ball art (two frames) | ball art |
| 19 | MoeGatcha free pulls, 8 pages of custom phrases | MoeGatcha, phrases |
| 20-21 | Genesis Ball and Epoch Ball plan | GENESIS / EPOCH BALLS |
| 22 | Triple Battle screen mock-up (cards, type icons) | co-op Triple Battles, tools/moemon_tricards (tri_cards) |
| 23 | Triple Battle art detail | co-op Triple Battles |
| 24 | LOOK menu (CUSTOM / CREATE) and the field with nametags | LOOK, nametags |
| 25 | Triple Battle party screen mock-up | PARTY_LAYOUT_TRIPLE, tools/moemon_tricards/party_mock.png (tri_party) |
| 26 | Candy Jar menu mock-up | CANDY JAR (tools/moemon_candyjar) |
| 27 | a friend's Surf blob off-axis | 1.17.2 fix |
| 28 | "I liked the right message box for the Candy Jar much better" | CANDY JAR message box |
| 29-30 | JOURNAL with one page: the page prompt is missing, INFO shows and the text garbles | summary JOURNAL fix (1.19.0, test summary_wrap) |
| 31 | bag design (HGSS-style BAG: 8 pockets Items, Medicine, Poké Balls, TMs & HMs, Berries, Battle Items, Key Items, Favorites; item list with "x N" and CLOSE BAG; description box) | the BAG screen (1.24.0: tools/moemon_bag/gen_bag.py, mp_bagui.c; tests bag_pockets, bag_more) |
| 32-33 | summary: the JOURNAL with an entry changing stripe color on its second line; INFO garbled after RIGHT from the last page | summary wrap / JOURNAL stripes (1.23.0, test summary_wrap_shots) |
| 34 | (not saved: the picture came without a file) co-op Triple Battle after walking into the WEATHER INSTITUTE from rainy ROUTE 119: the host stuck on "COOKIE is deciding what to do…", the two friend panels at their action menus | battle weather on every screen (1.25.0: B_COMM_MP_WEATHER; tests coop_fieldweather, coop_fieldweather2, tri_rain) |
| 35 | HEARTGOLD / SOULSILVER POKé BALL sheet (ripped by Sunday-Doneko): every ball spinning, squashed, open; SAFARI rock and mud. "Replace the thrown ball graphics" (no DREAM, GENESIS, EPOCH) | battle / summary balls (1.26.0: tools/moemon_balls/hgss_balls.py, graphics/balls; test hgss_balls) |
| 36 | (remade from 35's first row: the picture came without a file) "the correct POKé BALL sprite to display in menus, or when the POKé BALL is closed", not the squashed one | the closed-ball frame = the sheet's first row (1.26.0) |
| 37 | DS Pokémon Card Game duel screen (both screens: benches top/bottom, Active cards on the Poké Ball circle, HP plates, damage counters, WEAKNESS ×2 banner, -40, END TURN, hand bar): "adjust the in-battle sprites to appear like this... the full area", no Japanese text | the dev card duel board (src/tcg/tcg_board.c, docs/TCG_IMPLEMENTATION.md) |
| 38 | GAME CENTER building (GameCenterBuilding.png, 155x179): a purple tower, a big screen on the roof, two doors with striped awnings | the GAME CENTER outside (planned) |
| 39 | GAME CENTER 1F (gamecenter1f.png, 13x17 tiles): reception counter in the middle, stairs up at top left, elevator, two machines at right, two doormats | GAME CENTER 1F: reception, pay to get in (planned) |
| 40 | GAME CENTER 2F (gamecenter2f.png): card shop counter at left, display cases, a table with four chairs, card racks, stairs both sides | GAME CENTER 2F: the card shop (planned) |
| 41 | GAME CENTER 3F (gamecenter3f.png): two rows of consoles with blue chairs, arcade cabinets at top right, a prize counter, posters | GAME CENTER 3F: games played with coins (planned) |
| 42 | GAME CENTER rooftop (gamecenterrooftop.png, 287x248): green railings, a table with chairs, vending machines, the stair house at right, the tower's windows below | GAME CENTER rooftop: flavor NPCs (planned) |
| 43 | GAME CENTER 3F with a new machine at the top left (a DDR cabinet: the screen and two red dance pads) replacing the posters | GAME CENTER 3F picture, the DANCE machine (graphics/moemon/gamecenter/3f.png; docs/RHYTHM_DECISIONS.md) |
| 44 | 43 marked up: blue = drawn over the player (stair rails, the consoles' tops, the COIN counter's rim, the plants), red = walkable where it isn't obvious (in front of the arcade cabinets). "Red does not show all walkable surfaces, only exceptions" | 3f_marks.png, read by tools/moemon_gamecenter/gen_gamecenter.py |
| 45 | 3F with two cells marked red above the console row: walkable, so the DANCE machine isn't a walk around the consoles | 3F FLOORS row 7 (1.39.0) |
| 46 | The TRIPLE TRIAD card back (105x128): carved bronze filigree, a silver star-shaped diamond in the middle | the card back (graphics/tt/back.png, tools/tt/build_art.py) |
| 47 | Large card numbers 0-9 (160x18 strip on blue): grey serif digits with a dark outline | the numbers on large cards (graphics/tt/numbers.png; the A drawn to match) |
| 48 | Final Fantasy VIII's menu and battle font sheet (391x350, "ripped by Ultimecia"): digits, letters, icons; the large A at (178,84) | the large numbers' A (graphics/tt/ff8_menu.png, tools/tt/build_art.py) |
| 49 | GAME CENTER 3F as it is (224x268 crop): the DANCE machine at the left wall, the card tables, the console row | reference for 50 |
| 50 | The same 3F with the DANCE machine's front bars (the pink rail in front of the pads) painted magenta: those draw over the player | the DANCE machine's bar overlay (1.42.0) |
| 51 | TRIPLE TRIAD CHALLENGE screen (TRIAD SELLER, RUSTBORO RULES): the opponent's five-card hand spills past the dark box's right edge | the hand preview fits its box (5.1) |
| 52 | 52_gb_card_renders/: 26 English-style card renders (600x825) of Game Boy-only and promo cards (Articuno Quickfreeze, Ditto, Zapdos Peal of Thunder, Bulbasaur, Ivysaur, Squirtle (createcard.jpg), Dragonite, Electrode, Flareon, Flying Pikachu, Imakuni?, Jigglypuff, Jolteon, Magnemite, Marowak, Mew x2, Moltres Firegiver, Ninetales, Pidgeot, Pikachu, Slowpoke, Surfing Pikachu, Venusaur Solar Power, Vaporeon, Tangela (tangela2.png)) | printed art for those cards (art only: the rules stay the Game Boy games') |
| 53 | A POKéMON card duel won (YOU WIN! vs. LASS): the result box cuts off its last line under the LASS's quote | the result box fits its text; Yu-Gi-Oh!'s win screen made the same (1.56.1) |
| 54 | A Yu-Gi-Oh! duel's Battle Phase: the red strip naming the card under the cursor (Change of Heart, SPELL CARD) in the middle band | fade that strip while a phase banner plays (1.56.2) |
