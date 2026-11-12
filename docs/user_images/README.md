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
| 55 | (sent in chat, no file) KANTO Route 2: an ARCEUS standing on the path in front of the player; the battle it starts is against a VICTINI | the right POKéMON in the event, and the event only when it should be there |
| 56 | (sent in chat, no file) A wild battle stuck at its intro with OPTIONS > CREATURES: POKEMON (1.61.4): the trainer back picture holding the ball, the grass circles, no wild POKéMON, the text box empty | every battle begins with its POKéMON shown |
| 57 | 57.png: the starter table for KANTO and JOHTO: three POKé BALLS on a green-topped table, the hand cursor over the middle one | choosing the starter from the lab table with Emerald-style hand, wiggling balls and preview |
| 58 | 58.png: SINNOH's briefcase starter screen: the glove cursor over the left POKé BALL drawn as broken bands of pixels | the glove drawn as Platinum's white pointing hand (1.63.8) |
| 59 | (sent in chat, no file) A HOENN town (FALLARBOR?): a house drawn right, the building beside it shown as a plain green field, rocks and signs around it | every building drawn with its own tiles (1.63.9) |
| 60 | 60.png: KANTO, the VIRIDIAN POKé MART after OAK's PARCEL: "RE put away ????????." | the item's name and the right pocket in the put-away message |
| 61 | 61.png: the same, next page: "????????. Pocket: ITEMS." | (as 60) |
| 62 | (sent in chat, no file) JOHTO's CHERRYGROVE on a new game: a TEAM ROCKET grunt and a GROWLITHE on the road by the POKéMON CENTER, nothing to say | Quetzal's Giovanni-story Rockets only in the past (1.63.11) |
| 63 | 63.zip: 36 Battle Pass designs from Pokemon Battle Revolution (the "recolorable" ones named so) | the TRAINER CARD's designs to pick from (graphics/moemon/cards/passes/, 1.66.0) |
| 64 | 64.png (sent in chat; the same picture as graphics/moemon/cards/passes/rental_pass_blue.png): the RENTAL PASS BLUE Battle Pass as the reference | the TRAINER CARD laid out on the pass's own grid: its border, the BATTLE PASS label's line, the P emblem and the notch kept clear (1.66.0) |
| 65 | (sent in chat, no file) Crystal's POKéGEAR, four screens: the PHONE card (green list "MOM:", "PROF.ELM:", "JOEY: YOUNGSTER", empty "----------:" rows; box "Whom do you want to call?"), the CLOCK card (orange, a framed LCD "THURSDAY 11:14 PM", "SWITCH▶" top right, box "Press any button to exit."), the RADIO card (orange, a black TUNING dial 5 / 10 / 15 with a red needle, a station box "OAK's PKMN Talk", box "MARY: PROF.OAK'S POKéMON TALK!"); the card icons top left in a black strip with an arrow under the current one; and promotional art of the POKéGEAR device | the POKéGEAR's look (src/mp/mp_pokegear.c, 1.68.0): "it's almost too functional. It's not stylish at all" |
| 66 | 66.png: the Companion App design doc's roadmap (four phases: 1 the link, 2 the device, 3 the walk, 4 friends; a gate after each) | docs/COMPANION_APP.md (the whole doc, on hold until "Let's do the app") |
| 67 | 67.webp: a SINNOH wild battle, "Give a nickname to the captured SHINX?" with the YES / NO box frameless (white) | the ball's fade-out after a catch faded BG palette 1 by a wrapped mask bit (battle_anim_throw.c SpriteCB_Ball_FadeOut, MP_SPRITE_PAL_BIT; 1.83.0) |
| 68 | (sent in chat, no file) JOHTO in 1.89.0: Emerald's ripe BERRY tree sprite (blue BERRIES) standing beside a big round JOHTO tree, the player next to it | JOHTO's BERRY patches never draw a sprite of their own (event_object_movement.c, mp_apricorn.c; docs/REGIONS_AUDIT.md 158, 1.91.0) |
| 69 | 69.png (16x168, 168-color indexed, color 167 clear): a silver / chrome POKé BALL's battle sheet, eleven 16-wide frames top to bottom (closed, turns, the open ball last) | the new SHINY-making POKé BALL's battle art (the user, 2026-11-04) |
| 70 | 70.png (32x32, color 120 clear): the same ball's BAG icon | that ball's item icon |
| 71 | (sent in chat, and as a Bulbagarden link the session couldn't download: archives.bulbagarden.net Gym_Leader_Castle_Stadium.png) STADIUM 2's GYM LEADER CASTLE: a castle of towers on a red cliff, a winding path of little houses up to it, a misty sunset | the GYM LEADER CASTLE's picture on the STADIUM's cup screen (src/mp/mp_stadium_ui.c; waiting for the file) |
| 72 | 72.png (320x237): STADIUM 2's CHALLENGE CUP picture (PORYGON, HITMONLEE, GYARADOS, SEAKING in a stadium, glowing outlines) | the CHALLENGE CUP's picture on the STADIUM's cup screen (graphics/stadium/challenge_cup.png) |
| 73 | 73.png (511x437): the user's BATTLE ISLAND in Moemon Maker: the STADIUM up top, the stairs and arch, the pier at the bottom with the sailor at its end and the boat's spot below (2026-11-07) | the boat's landing follows the island's sailor (mp_battleisland.c LandingSpot); BATTLE ISLAND joins the ferry menus (1.163.0) |
| 74 | 74.png (1290x863): ORRE's OUTSKIRT STAND after the boat (2026-11-07): the player and the sailor on a plain teal tile background, no 3D world at all ("Traveling to Outskirt stand worked, but there's nothing there...") | the boat must not sail without ORRE's packs (src/orre/orre_ferry.c Orre_Ready; ORRE_DECISIONS "A post-game place") |
| 75 | 75.jpg (2000x1500, a photo of a monitor, 2026-11-10): PLAY WITH FRIENDS after HOST A GAME: "Landon's server is open on your home network. Your router has automatic port opening (UPnP) turned off, so friends elsewhere can't reach you. See README: UPnP / Radmin VPN." | the host's router has UPnP off: turn it on, or forward TCP 27015 to the host PC by hand and have friends type its public address (dist_extras/README.txt "PLAYING OVER THE INTERNET") |
| 76 | 76.webp (1899x1371, 2026-11-11): MAJORA'S MASK's SELECT ITEM screen (an HD version): an olive, weathered painted plate with raised bevelled frames, SELECT ITEM cut into it, Z / R arrow tabs, the item name on a raised plate in white serif letters with a dark outline ("Majora's menu and Stadium 2's menus look like this") | the look the user is describing for menus (talked over in chat; nothing built yet) |
| 77 | 77.webp (1170x867, 2026-11-11): STADIUM 2's team screen: rounded chrome frames like metal tubes, black glossy boxes (1P, All ready!, MARTY, the portraits), a pale blue 1P team panel and a pale green COM one in grids of six, pointed side banners, a faded grey picture behind | the same (STADIUM's screens: src/mp/mp_stadium_ui.c) |
| 78 | 78.webp (822x617, 2026-11-11): STADIUM 2's entry screen (GYM LEADER CASTLE, Entry POKéMON): the 1P plate, "First POKéMON" in yellow brackets, tab keys L P 1-14 R R, the blue list of rentals with names and levels ("Image 1: Picking Pokemon for battle") | POKESTADIUM 2 THEME: docs/POKESTADIUM_2_THEME_HANDOFF.md |
| 79 | 79.webp (822x617): STADIUM 2's result: two silver plates, portraits, three picture windows each with HP bars, WIN in orange neon, LOSE in cyan ("Image 2: Victory") | the same |
| 80 | 80.webp (810x617): STADIUM 2's pre-battle: two silver plates, six windows each, a glowing VS ("Image 3: Pre-battle (ignore the youtube UI)") | the same |
| 81 | 81.png (960x640, 2026-11-12): the session's own mock-up of the staggered 1P / COM screen, sent back with "What happened to this scene?" then "I want the one I sent you to act as the choice screen" | the same: the choice screen |
| 82 | 82.webp (822x617, 2026-11-12): STADIUM 2's battle, the action menu: the 1P box (name, L5, condition, gender, HP bar, numbers) with the POKéMON's portrait under it and three team dots, (A) BATTLE / (B) POKéMON / (S) RUN as chrome capsules, the foe's box and BERNIE's plate bottom right, L CANCEL / R CHECK at the foot | the battle HUD in POKESTADIUM 2 THEME: docs/POKESTADIUM_2_THEME_HANDOFF.md |
| 83 | 83.webp (822x617): the same battle's move menu: four moves as chrome bars in a diamond round the D-pad arrows, each with its name, type and PP; the picked one in its type's colour | the same |
| 84 | 84.png (247x120): a close-up of STADIUM 2's healthbox at 7/20 (a yellow bar) | the same |
| 85 | 85.png (247x120): the same healthbox fainted, 0/20 ("Ft") | the same |
