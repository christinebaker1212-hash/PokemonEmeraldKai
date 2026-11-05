# Headless tests

Runs the game with no window (SDL dummy driver), driven by the script harness in
`src/platform/sdl2.c`. Linux only; needs `make linux` to work. `convert` (ImageMagick) turns
screenshots into PNGs if it's installed.

    tools/test/regress.sh              # build, run every scenario, report ok / FAIL
    tools/test/regress.sh coop_wild    # just one
    NOBUILD=1 OUT=/tmp/t tools/test/regress.sh

Three-client scenarios (`MODE=3`) use `runN.sh` (2-4 clients; `c.txt`, `WARP_C`, `ENV_C`, `EXPECT_C`).
Every multi-client scenario also runs at the worst connection we plan for with `WORST=1`
(250 ms each way + 80 ms jitter through lagproxy, like NA <-> JP with the relay far from both).
`JOBS=3` runs three at once.

| scenario | what it checks |
|---|---|
| `solo_wild` | one client: wild battle to the action menu, win, back to the field |
| `coop_wild` | Route 101: BOB's encounter pulls ALICE in, they win together; record mixing on join |
| `coop_wild_lag` | the same, always at worst-case lag |
| `coop_catch` | co-op 2-on-2, a MASTER BALL (L = last ball) catches one, they finish the other |
| `coop_levelup` | co-op win one EXP short of Lv16 on both: level up + Torchic -> Combusken on both |
| `coop_icons_flash` | co-op 2-on-2 at NA-JP lag with TYPE ICONS on: THUNDER's screen flashes leave the icons and the window frame alone (screenshots) |
| `coop_weather` | co-op 2-on-2 at NA-JP lag: SANDSTORM hits all four at once (one line, then quiet turns), win on both |
| `coop_fieldweather` / `coop_fieldweather2` | NA-JP lag, field weather differs between the games (`mp fieldweather`): every screen shows the battle's weather (the leader's field), RAIN DANCE's rain reaches the friend's screen |
| `coop_two_join3` | NA-JP lag, three players: two trainers spot ALICE, BOB teams up, CARL joins once it started (`mp joinask 2`, sent on to ALICE): a Triple Battle against both trainers, the third foe spot from the trainer with more left, both trainers beaten on every game |
| `coop_merge_join3` / `coop_held_join3` | three players: two players grouped on one trainer (waiting merge / held trainer spots the second), then a third one spotted by that trainer joins (JOIN BATTLE) |
| `hgss_balls` | shots only (looked at by hand): the HGSS balls' send-out spin, a thrown MASTER BALL, a blue POKé BALL with a STAR, a GENESIS BALL, the summary's ball |
| `astral_ball` | the ASTRAL BALL: four full-HP catches (one a RAYQUAZA) on the first throw, each shiny with nature, gender, UNOWN letter and WURMPLE evolution kept (check.py); shots of the sheen and sparkles, the click (fade to grey, sparkles drawn in), the summary's plain ball |
| `fishing_gen1` | fishing as Gen 1 had it: with no button pressed, a cast with no bite ends on "Not even a nibble!", a SUPER ROD bite says "Oh! A bite!" and the battle starts by itself (the bite roll rigged with `mp fishbite`) |
| `tri_rain` | wild triple at NA-JP lag in the leader's rain, one friend's field dry: rain on all three screens, turns go on to the win |
| `coop_life_emote` | NA-JP lag: a partner save; each game's Pokemon emote shows on the other game over the same Pokemon (check.sh) |
| `coop_spot_join` / `coop_spot_alone` | ROUTE 116: JERRY, battling ALICE, spots BOB: "join their battle, or battle alone?"; JOIN BATTLE (ALICE lets him in, both win, JERRY beaten on both) / BATTLE ALONE |
| `hue_castform` | a nicknamed CASTFORM (four form palettes) in the PC under the cursor and in battle, its hue on every form, no crash |
| `coop_flash` | Granite Cave B2F (dark) at NA-JP lag: each sees the other's light, BOB's FLASH lights up around him on ALICE's screen; co-op notes show in the dark |
| `coop_world` | NA-JP lag: a CUT tree is gone for a friend who walks onto the map later (list), a smashed rock vanishes live, a STRENGTH boulder moves on both games |
| `coop_world2` | NA-JP lag: CUT grass is cut on the friend's game; Route 113's ash ALICE walks through clears for BOB, the soot is only hers |
| `coop_heal` | ALICE uses a POTION on BOB's Moemon from her bag (partner's gold slot) |
| `coop_trainer` | Roxanne: ALICE waits for a partner, BOB walks up and joins, BOB's RUN is refused, they win |
| `coop_run` | co-op wild: ALICE alone picks RUN (it only says so), then both pick RUN until they get away |
| `coop_ball_target` | co-op 2-on-2: both throw at the same wild Pokemon (BOB moves his target cursor); ALICE catches it, BOB's ball comes back |
| `coop_abort_catch` / `coop_abort_catch2` | a catch, then the other player disconnects: the catcher keeps it (master / non-master) |
| `pvp_battle` | BATTLE FLAG: BOB (no CHALLENGE phrase: no phrase box) spots ALICE, link battle to the end, the winner gets a tenth of the loser's money |
| `pvp_switch` | PvP where ALICE's lead faints and she sends in her second Moemon |
| `fight_resync` | online, BOB's game alone changes a life bar (a desync): both stop at the same frame, ALICE's game (the host) sends the pages that differ, then every check agrees to the end (also at `WORST=1`) |
| `fight_display` | a CPU fight shown in each scaling mode (PIXEL PERFECT at 4x, 3x and 1x of the GBA screen, SHARP FIT, STRETCH: the game's own 4:3 picture over the window) and with the LCD grid and scanlines: each window output against check.py's reference rendering of the fight's own 480x320 picture of the same frame (`fightshot`) |
| `fight_packs` | online, BOB's SSF2X Ken has one more file: Ken greyed out on both select screens, ALICE can't pick him |
| `fight_packs_core` | online, BOB's engine data differs: ALICE says no, BOB is told the fight files aren't the same |
| `fight_buttons` | OPTIONS > CONTROLS > FIGHT BUTTONS with a virtual pad: a button bound, one taken from another fight button (they swap), one left alone after 8 s; saved in the controls file |
| `dance_solo` | the DANCE machine (GAME CENTER 3F): step up, PLAY ALONE, every screen to a song played by `MOEMON_DANCE_AUTOPLAY` (MARVELOUS full combo, feet x 10,000,000), RESULTS, the next song given up (START held): FAILED, GAME OVER, back on the map |
| `dance_versus` | ALICE waits on the left pad, BOB steps up to the right one and joins: VERSUS, ALICE picks, both play their own field and see the other's, the records reach the server (`rhythm_scores.dat`); a tie is no win |
| `dance_hop` | ALICE plays alone: hops up (BOB, beside her, sees the arc and every double hop's facing, never south), one stage, gives up the next; the screen turns off like an old TV, she hops down and says a line from her grades |
| `lowhp_beep` | a single battle starting in the red, a move animation during the beep at ANIM SPEED 30: the beep stops after 100 ticks (three "doo duh"s; `MOEMON_SE_TRACE=90`) |
| `lowhp_pvp` | the same in a PvP single battle (a link battle), on both screens |
| `join_gym` | ALICE challenges ROXANNE, BOB joins from another town: story battles can be joined; both get the STONE BADGE (BOB through the leader's own "you won" script from the gym's map) |
| `coop_two_wait3` | TEAM PADDING never turns a third away: two trainers spot ALICE, BOB teams up, CARL asks too and is told he'll join; his game JOINs by itself once it starts: a Triple Battle against both |
| `coop_two_solo_join` | two trainers spot ALICE alone; BOB joins the battle in progress: a co-op battle against both, both trainers beaten on both games (their continue-script mode too) |
| `sim_tent` | the BATTLE SIMULATOR: BOB's record reaches ALICE, she beats his digital copy at level 50 in SLATEPORT's tent, gets her prize, stops at 1, her party back at its own levels |
| `sim_nodata` | the BATTLE SIMULATOR on a save that never played online: no battle data, said politely |
| `anim_classic` | every move animation on BATTLE ANIMS > CLASSIC, each one's length logged (`MOEMON_ANIM_FRAMES`), shots of a few |
| `anim_modern` | the same on MODERN (run after anim_classic): `tools/test/check_anim_frames.sh` checks all 716 lengths match, no crash |
| `mtg_shop` | MAGIC at MOSSDEEP's CARD SHOP: a regular ignores it before a deck, the clerk's starter, the CARD CASE on its MAGIC side (four tabs), PIP's duel (a few turns by the computer, then a win) and its booster, the clerk's way down, RHEA's challenge, the play room's stairs up by the front door |
| `mtg_show` | the MAGIC table plays each event out before the next (a land, a spell to the stack, an attack and its block, the opponent's turn): bursts of shots to look at; "mtg: ready" once the player may act |
| `mtg_triggers` | MAGIC's triggered abilities on the table (`mp mtg put` sets it up): Nekrataal asks for its target among two, the trigger destroys it; Gravedigger's "you may" asks YES / NO and returns a creature card from the graveyard |
| `mtg_play` | a MAGIC duel by hand (the same deal under the harness): a land, a haste creature cast and attacking, SHOCK through the target cursor onto the opponent, a card's whole text, a loss and its words |
| `badge_case` | the BADGE CASE (SELECT on the card): badges of three regions won now, six days of fading, rubbed to each look (4 / 2 sparkles, clean, dirty, filthy); shots of the trays and a badge up close; B leaves the card open; a badge up close polished by A held and the stick going round (a virtual pad) |
| `card_design` | the TRAINER CARD (one design for every region): PROFILE, BADGES & STARS, RECORDS, TEAM & SIGNATURE; MY CARD: a design, its hue, accent, frame, panels, the showcase; a signature drawn on the pad; the card as styled; B back to the map |
| `card_still` | the TRAINER CARD holds still with nothing pressed, turns with the right stick, settles flat when it's let go (check.py) |
| `card_designs` | the PROFILE on all 36 Battle Pass designs, the other pages, the four panels, the five frames: kept on the pass's grid (label, emblem, notch) |
| `card_friends` | two clients: a friend's card with its add-on, as they styled it: their BADGES & STARS, their BADGE CASE on JOHTO, YOU AND BOB; then the album card on MY CARD > FRIENDS' CARDS |
| `spec_card` | two clients: BOB's SPECTATE panel shows ALICE's TRAINER CARD and her BADGE CASE up close |
| `hm_rescue` / `hm_rescue_carried` | HM04 STRENGTH's scene gone (RUSTURF TUNNEL open) without it: Wanda's boyfriend walks up and hands it over; nobody comes when it's already carried |
| `coop_rusturf` | NA-JP lag: ALICE smashes RUSTURF TUNNEL's boulders while BOB stands across them: the tunnel scene plays on BOB's game too and he gets HM04 STRENGTH (his story had stayed shut) |
| `johto_roamer` | JOHTO's RAIKOU / ENTEI set going (Crystal's rules): ENTEI met on ROUTE 37, flees keeping its HP, both move on |
| `kanto_boulder_hole` | SEAFOAM 1F: a STRENGTH boulder pushed into the hole falls (gone, B1F's twin shown) |
| `kanto_birth_island` | KANTO's BIRTH ISLAND: touching the triangle moves it on its own progress, no freeze |
| `johto_kiln` | CHARCOAL KILN: the apprentice's CHARCOAL with Crystal's words and its own flag; FARFETCH'D home |
| `johto_radio_card` | RADIO TOWER 1F: Crystal's RADIO CARD quiz, YES YES NO YES NO gives the RADIO CARD |
| `region_gate_walk` | VICTORY ROAD gate: the KANTO door refused without both LEAGUEs, then customs: one POKéMON along, the rest to the PC, KANTO's side |
| `kanto_selphy` | FIVE ISLAND: SELPHY asks to see a POKéMON, 250 steps later it's shown and she rewards it |
| `kanto_marowak_ghost` | POKéMON TOWER 6F with the SILPH SCOPE: MAROWAK's ghost battle, a thrown ball is dodged |
| `kanto_icefall_ice` | ICEFALL CAVE 1F: thin ice cracks under a step, breaks on the next and drops the player to B1F |
| `johto_rival_days` | Crystal's rival after the LEAGUE: INDIGO PLATEAU on a Monday, gone on a Tuesday, training in DRAGON'S DEN |
| `johto_crystal_people` | MANIA lends SHUCKIE and takes it back the next day; the SANDSTORM house's loyalty check; LAKE OF RAGE's MAGIKARP record and ELIXIR; the ROUTE 43 toll |
| `johto_move_deleter` | BLACKTHORN's MOVE DELETER makes the lead forget its first move |
| `johto_floria` | ROUTE 36's FLORIA by the odd tree, then in the FLOWER SHOP and back; the SQUIRTBOTTLE on Crystal's conditions |
| `johto_national_park` | Crystal's NATIONAL PARK people (PERSIAN, the POKéDEX-printing boy...); ROUTE 35's OFFICER DIRK battles only at night |
| `johto_ruins_walls` | The RUINS OF ALPH walls open as Crystal's: HO-OH leading, a WATER STONE, an ESCAPE ROPE / FLASH in the chamber |
| `johto_ruins_word` | A closed RUINS wall shows its UNOWN word in UNOWN pictures |
| `johto_elm_lab` | ELM's lab window (open / broken into), the aide by stage, ELM's research-paper PC at home |
| `johto_crystal_lines` | Quetzal's small rewrites back to Crystal: no CHANSEY, the EON MAIL lady, ROUTE 38's TRAINER TIPS, no cheering crowd |
| `johto_weekday_siblings` | Crystal's WEEKDAY siblings stand there only on their day, meet you once and give their gift |
| `johto_friday_lapras` | UNION CAVE B2F's LAPRAS is there on Fridays only, until it's fought that day |
| `johto_catch_tutorial` | ROUTE 29's DUDE shows how to catch a RATTATA (his own back picture, throw first) |
| `johto_hidden_item` | Crystal's hidden items: ROUTE 30's POTION is found once |
| `johto_sunday_lady` | GOLDENROD DEPT. STORE 5F's receptionist is out on Sundays and gives TM27 RETURN to a loving lead, once that day |
| `johto_goldenrod_tutor` | GOLDENROD's tutor (Wednesdays after the LEAGUE) teaches FLAMETHROWER for 4000 coins, then leaves for the day |
| `johto_poke_seer` | CIANWOOD's POKé SEER reads the lead's meeting and gives her advice |
| `johto_tm_prizes` | GOLDENROD GAME CORNER's TM counter sells TM25 for 5500 coins |
| `johto_name_rater` | GOLDENROD's NAME RATER asks to rate a nickname and leaves the field free |
| `johto_haircut` | GOLDENROD UNDERGROUND's older HAIRCUT BROTHER (Tuesdays) cuts for 500 and raises friendship, once a day |
| `johto_bargain` | the Monday-morning bargain merchant sells a NUGGET at Crystal's price, one a day |
| `johto_gs_ball` | GOLDENROD's POKéMON CENTER brings the GS BALL only after JOHTO's LEAGUE; KURT keeps it until the next day, then hands it back in AZALEA |
| `kanto_move_people` | FUCHSIA's MOVE DELETER forgets THUNDERSHOCK, the COPYCAT teaches MIMIC for a POKé DOLL, TWO ISLAND's MOVE MANIAC remembers GROWL for two TINYMUSHROOMS |
| `kanto_cape_brink` | CAPE BRINK's starter tutor teaches a lead VENUSAUR at full friendship FRENZY PLANT |
| `kanto_daisy_bike` | DAISY gives the TOWN MAP, grooms after the LEAGUE (friendship 100 -> 103); the BIKE SHOP swaps the BIKE VOUCHER for the BICYCLE |
| `kanto_size_records` / `kanto_size_heracross` | ROUTE 12's MAGIKARP record (NET BALL), SIX ISLAND's HERACROSS record (NEST BALL) |
| `kanto_giovanni_leaves` | GIOVANNI's farewell in VIRIDIAN's GYM, then he's gone; FIVE ISLAND's POKéMON JOURNAL after the LEAGUE |
| `kanto_ruin_valley_cut` | RUIN VALLEY: CUT used from the party menu below the braille door opens it |
| `kanto_togepi_egg` | the WATER LABYRINTH's gentleman gives a TOGEPI EGG for a lead POKéMON at full friendship |
| `johto_hideout_studio` | MAHOGANY's hideout plays as Crystal's (LANCE, the EXECUTIVE); CIANWOOD's PHOTO STUDIO is back beside the GYM |
| `kanto_fossil` | CINNABAR's fossil doctor as FireRed's: a HELIX FOSSIL, a walk outside, a Lv 5 OMANYTE |
| `johto_trade_kyle` / `kanto_trade_zynx` | Crystal's KYLE trade (ONIX ROCKY), FireRed's ZYNX trade with her letter |
| `johto_mahogany_mart` | MAHOGANY's shop run by TEAM ROCKET, then the old woman's stock once the RADIO TOWER is freed |
| `kanto_safari` | FUCHSIA's SAFARI ZONE as FireRed's: ¥500, 30 SAFARI BALLS, 600 steps that count down, RETIRE ends it at the gate |
| `kanto_oak_rating` | OAK rates KANTO's POKéDEX with FireRed's words for every ten caught |
| `johto_legends_original` | Quetzal's SUICUNE by the LAKE OF RAGE stays hidden before and after the LEAGUE (only the original games' legendaries) |
| `johto_buena` | BUENA's PASSWORD: the radio says the day's password, BUENA gives the BLUE CARD and takes the right answer for a point, the prize desk trades points for an ULTRA BALL |
| `johto_dance_theater` | Crystal's DANCE THEATER: no SURF before the five KIMONO GIRLS are beaten, NAOKO's Lv 17 FLAREON, HM03 once all five are |
| `johto_wise_trio` | The WISE TRIO: turned back without the CLEAR BELL, KOJI battles and steps aside, TIN TOWER 1F's SUICUNE battle ends the chase |
| `johto_wings` | The RAINBOW WING after the LEAGUE with the three beasts (the stairs appear), HO-OH and LUGIA only with their wings, PEWTER's SILVER WING |
| `johto_suicune_chase` | SUICUNE's chase: not at CIANWOOD before the BURNED TOWER, then CIANWOOD, ROUTE 36, back to CIANWOOD |
| `johto_crystal_teams` | JOHTO's trainers on Crystal's teams at GEN 3: FALKNER's PIDGEY Lv 7 with TACKLE / MUD-SLAP, PIDGEOTTO Lv 9 |
| `kanto_firered_teams` | KANTO's trainers on FireRed's teams at GEN 3: ROUTE 3's BEN with RATTATA and EKANS Lv 11 |
| `johto_bugcontest_crystal` | the Bug-Catching Contest on a THURSDAY: five of Crystal's contestants in the park, a second catch asks to switch, "finish now?" at the way out, the judging in a line, the catch joins the party and the lead keeps its EXP |
| `trade` | CO-OP > TRADE: both offer their lead, READY, confirm, swapped (and their MOéMON GARDEN records travel) |
| `nurse_no_union` | CHERRYGROVE's nurse heals a game started in JOHTO: she never says "It appears as if <junk> is playing right now" (the wireless UNION ROOM check; flag 0x2A07 stays clear) |
| `pmd_scale_hop` | a scaled PMD follower (PIKACHU, FOLLOWERS: PMD everywhere) mid-hop is drawn whole: check.py counts its rows and pixels (its lower part was cut off) |
| `ledge_shadow` | ROUTE 101's ledge jumped with SHADOWS on (no jump shadow, the drop shadow stays) and off (the game's own jump shadow) |
| `blender` / `crush` | BERRY GAMES invite, accept, play to the results |
| `contest` | link CONTEST at the Lilycove counter, start to finish (long) |
| `card_base` | FRIENDS > TRAINER CARD and VISIT SECRET BASE |
| `ygo_online` | a Yu-Gi-Oh! duel between friends (ygo_net.c): asked, both pick a deck, the AI plays both seats to the end; both games hold the same duel |
| `ygo_online_nodeck` | the same with BOB holding only POKEMON cards: he's asked anyway, takes a Starter Deck to keep, and they duel |
| `tt_world` | HOENN's Triple Triad rules from the room host: BOB sees ALICE's region rules, TRADE RULE and CARD QUEEN, not his own; his change reaches her and comes back |
| `ygo_trade` | a Yu-Gi-Oh! card trade between friends: both offer, both confirm, each gets the other's card |
| `spectate` | A on a friend in a battle: watch it, B to stop |
| `rescue` | BOB loses, ALICE gets HELP? and revives his lead |
| `lend` | BOB cuts a tree with ALICE's Moemon's CUT |
| `piggyback` | BOB rides on ALICE's back into Oldale |
| `photo` | SOCIAL > TEAM PHOTO saved |
| `gift` | DAILY > GIFT sent, received, thanked |
| `tri_wild` | three clients: CARL's encounter pulls ALICE and BOB in, a wild Triple Battle to the win |
| `tri_target_level` | Roxanne co-op Triple at NA-JP lag: every player opens the target window; its Lv must match the cards (no "target level mismatch" in any log) |
| `tri_trainer` | Roxanne: ALICE waits, BOB joins, ALICE keeps waiting, CARL joins: a trainer Triple Battle |
| `tri_catch` | wild triple: ALICE and BOB each catch a different wild Pokemon (each gets their own) |
| `tri_heal` | wild triple: ALICE uses a POTION on CARL's Moemon (the third panel on the left: the second ally's Moemon out) |
| `tri_party` | wild triple: ALICE's party screen against the user's mock-up pixel for pixel (actives left, reserves right), cursor RIGHT / DOWN / CANCEL |
| `tri_abort` | wild triple: ALICE catches one, BOB disconnects: called off, ALICE keeps the catch |
| `look_chars` | NA-JP lag: ALICE wears new characters: PROF. BIRCH's 64x64 bikes, SERENA's ridden RHYHORN then a bicycle (LOOK > BIKE), her diving sprite, an old ini name (`steven`); BOB sees each |
| `look_created_remote` | NA-JP lag: BOB receives ALICE's whole CREATE recipe (FIELD HAIR 5) and later her LOOK > BIKE (`mp remoteapp`); shots of her sprite on both screens |
| `shared_pc_cursor` | the shared PC's hand on PARTY / MY PC / CLOSE: over the buttons, never upside down (also holding a Moemon, and after MY PC and back) |
| `catch_exp` | solo: a MASTER BALL at a full-HP ZIGZAGOON gives 10% of a faint's EXP; the lead TORCHIC grows and evolves after the catch |
| `coop_catch_exp` | NA-JP lag, co-op wild: ALICE catches a ZIGZAGOON at 1 HP (95%); both leads get EXP, grow and evolve after the battle |
| `flag_phrases` | NA-JP lag: BOB uses the BATTLE FLAG (PHRASES: the phrase builder and back to the list, then PLANT), both screens show his CHALLENGE, the result quotes his phrase; check.py: the winner got a tenth, the loser kept their money |
| `spec_sides` | 4 players, NA-JP lag: ALICE (1920x1080) shows BOB LEFT 1X, CARL RIGHT 2X, DANA ABOVE 1X, then DANA in her own window, then closes it (DANA only goes off); check_spectrace.py |
| `options_tabs` | START > OPTIONS with real keys: TEXT SPEED, L/R tabs, BUTTON ICONS PLAYSTATION (icons switch), AUTOSAVE 10, MOENAV CALLS BATTLES (random chat calls stop); check.sh: saved to the inis |
| `music_options` | BIKE MUSIC / SURF MUSIC: the Mach Bike, surfing Petalburg's pond and diving, songs checked in order (check.py) |
| `dex_text` | POKéDEX TEXT tab: the long entry's start with an arrow, A / DOWN / UP / B scrolling, DOWN to the next Pokémon after, seen-only does nothing, AREA still opens; `mp dexcheck` builds all 386 texts |
| `life_core` | MOéMON GARDEN records: temperament, memories, a moment raises Friendship once, the .life sidecar saved and loaded, the JOURNAL page and its pages |
| `life_field` | Route 104: tap A (the fitting verb), hold A (the wheel, FEED A BERRY: a memory and a moment), the hidden-item hint, a nap, hurrying back |
| `life_story` | a badge becomes a memory (part / witness) and news; a curious MUDKIP asks to come out: party icon wiggle, WANTS TO COME in the follower picker |
| `life_battle_gym` | the first Gym (ROXANNE): the rare pink line after "... defeated LEADER ROXANNE!" with a chime, a boss's final knockout as a memory |
| `life_battle_save` | wild double: TORCHIC knocks out a foe while its best friend MUDKIP hangs on at 3 HP: sparkle / heart bubbles, the line after "fainted!", MEM_PARTNER for both |
| `life_word` | word spreads: a proud TORCHIC tells MUDKIP (who wasn't there) about the first badge: MEM_HEARD from TORCHIC |
| `life_social` | NA-JP lag, Oldale: ALICE's follower meets BOB's (a bond in her save), its bubbles show on BOB's screen, BOB pets it (her game answers: a sidestep, PET is beyond its boundary), BOB's new LOOK gets a double take |
| `garden_map` | OLDALE's CENTER escalator rides up into the MOéMON GARDEN (first visit: welcome, a memory; every growth mark with all badges) and its escalator back down to OLDALE |
| `garden_centers` | all 17 CENTERs (LEAGUE and BATTLE FRONTIER too): up into the Garden and back down to the same CENTER |
| `tm_reuse` | TMs reusable: a save with TM39's flag but no TM39 (and TM08 still in the BAG) gets only TM39 back from the LOTTERY CORNER clerk, once; TM39 taught from the BAG stays there; a POKé MART sells TM01 once, one at a time, and refuses TM01 and TM39 after |
| `surf_remote` | a friend surfing Petalburg's pond: their blob under them, again after a map reload (OPTIONS and back), and hidden with them once BOB is 28 tiles away on ROUTE 102 (before 1.17.2 it wrapped round onto his screen) |
| `garden_decorate` | DECORATE with the real buttons, the SECRET BASE's way: DOLL > the PIKACHU DOLL moved with the decorating cursor, "Place it here?" YES, back out and walk (1.17.2 locked the player); the copy shows in use; PUT AWAY with the cursor; walk again; a jump down the plateau's ledge lands below it |
| `garden_places` | a place for every type: TORCHIC is sent to each of the 17 types' places with no badges, then with all eight, and walks all the way there round the trees, rocks and water (check_garden_places.py) |
| `garden_stuck` | an older save standing inside a tree goes back to the landing; with no CENTER recorded the way down follows the heal location |
| `garden_life` | three Pokémon live in the Garden: play, talking to one, a PIKACHU DOLL placed and played with, WATCH mode (A tells, B back) |
| `garden_travel` | START > GARDEN from OLDALE, back down to its CENTER; GARDEN in the START menu; a CENTER PC's row; refused in RUSTURF TUNNEL and before having a Pokémon |
| `garden_longterm` | TREECKO's pursuit reaches a milestone; MUDKIP coaxes a nervous TORCHIC into the pond (seen: in the journal), stargazing; TORCHIC evolves there (new sprite) and the others come to see; COMBUSKEN's JOURNAL page |
| `garden_doing` | What Garden Pokémon do shows: MUDKIP swims in the pond (cut off at the water line, a dive under), TREECKO climbs a tree, SANDSHREW digs half in the ground (sink / up / water checked from the log) |
| `bag_pockets` | The BAG's eight pockets, RIGHT through all of them and round again; POTION marked FAVORITE shows in FAVORITES, UNFAVORITE takes it out (`mp bagview`) |
| `bag_more` | MAY's bag: MEDICINE scrolls, SELECT moves an item (gray frame where it was, the save pocket in its new order), TM info, BERRY tag and back |
| `item_desc` | Gen 4 item descriptions wrapped to their windows: the BAG's box fits them, a POKé MART's narrow box turns pages by itself (`mp itemdesc`) |
| `life_family` | MUDKIP and TREECKO make an EGG (the DAY CARE's code), it hatches on a step, meets its parents ("Met its parent"), TREECKO evolves and the follower reacts |
| `levelscale_wild` | a Lv46 team on Route 101: wild Pokémon capped at 18 with no badges, halfway (22-24) with eight; the rare evolved form (forced on) |
| `levelscale_options` | OPTIONS > GAME > LEVEL SCALING's page with the keys: WILD HIGH, BOSS MARGIN +0, saved in the ini; a wild one at 40-43, never the evolved form (forced off) |
| `host_rules` | HOST A GAME > GAME RULES: TRAINERS and PAST 8 BADGES changed with the keys and saved (the room's rules) |
| `levelscale_trainer` | ROXANNE's ace Lv15 -> 20 against a team averaging 42 (boss margin, no-badge cap) |
| `levelscale_off` | LEVEL SCALING off: no scaling |
| `coop_levelscale` | NA-JP lag: ROXANNE scaled for the average of both teams (Lv30 and Lv50) |
| `pmd_depth` | a PMD follower right below its Trainer is drawn in front of them at every step (scroll); ONIX stands a little farther back; 4 ways only |
| `follow_sprites` | OPTIONS > FOLLOWERS: DEFAULT (original follower, PMD Garden Pokémon), PMD, ORIGINAL, switched live |
| `battle_weather_intro` | rainy ROUTE 120: the rain shows during the battle's intro, before the battle's own weather is set |
| `farm_solo` | the Garden's BERRY farm: 4 plots open, plant a CHERI BERRY by talking to plot 0, grow, pick it; TREECKO tends, MUDKIP waters, a hungry MUDKIP nibbles |
| `farm_together` | NA-JP lag: BOB sees ALICE's (the host's) farm, picks her ripe plot (her game hands the BERRIES over) and plants one back |
| `pmd_follower` | a shiny TORCHIC follows as a PMD Collab picture (big sprite): walking right, then idle (mp pmd), shots |
| `pmd_remote` | NA-JP lag: BOB sees ALICE's shiny ONIX follower as the same shiny PMD picture (followerFlags bit 7) |
| `patch_notes` | PATCH NOTES on the CO-OP setup screen: the list of version lines, 1.18's five updates full screen (scroll, page), 0.1 at the end (shots) |
| `summary_wrap` | the summary screen wraps (LEFT on the first page -> the one-page JOURNAL, RIGHT -> back); A on a one-page journal does nothing and opens no move selection |
| `summary_wrap_shots` | on screen: INFO after RIGHT round from the last page and after SKILLS and back is pixel for pixel the INFO it opened on; the JOURNAL reached by LEFT round from INFO is the one reached page by page; a two-row sentence keeps one stripe color |
| `battle_weather` | rain, hail, sandstorm and sunlight on screen in a wild battle (shots), a turn in the rain with no repeated line |
| `battle_weather_map` | a wild battle on rainy Route 120 starts in the rain, on screen |
| `follow_ground` | the follower in tall grass (Route 101), its reflection (Petalburg's pond), footprints (Route 109), puddle splashes (Route 120), hot springs (Lavaridge) |
| `follow_ground_remote` | NA-JP lag: BOB sees the grass shake around ALICE's follower |
| `follow_reflect_remote` | NA-JP lag at Petalburg's pond: each screen shows the friend's and the TORCHIC's reflections washed out like the player's (shots) |
| `life_return` | Route 104: the follower off on its own runs back after its Trainer and falls in behind them, never popping over (check_followtrace.py) |
| `life_arrival` | arrival bubbles only in a new area: nothing at the start or in and out of a house, a curious look in PETALBURG (check_arrival.py) |
| `life_watch` | WATCH QUIETLY: learned, nothing new, nothing new, then learned again half an hour later (check_watch.py) |
| `move_parity` | NA-JP lag, Oldale: both players' and followers' steps and facings the same on both screens, a follower on its own and running back included (check_movetrace.py) |
| `coop_switch_twice` | Co-op wild double at lag: ALICE switches, then opens the party menu again (the party in battle order crashed 1.15.0); both finish |
| `soft_reset` | A+B+START+SELECT on the overworld closes the game like CTRL+R (it crashed in the wireless adapter code) |
| `garden_parity` | NA-JP lag: both players' Garden Pokémon take the same steps and face the same ways on both screens (check_movetrace.py) |
| `garden_together` | NA-JP lag: two in the Garden: each sees the other's Pokémon and decorations, BOB talks to ALICE's MUDKIP; BOB's eight badges grow ALICE's Garden live |
| `version_check` | (`version_check.sh`, part of the full run) a server one co-op version ahead: the game says so |

A scenario is a folder in `scenarios/` with `a.txt` (+ `b.txt` for a second client) and `conf`:
`MODE=1|2`, optional `DELAY`/`JIT` (ms, through lagproxy), `FPS`, `TMO` (seconds), `WARP`/`WARP_B`,
`ENV_A`/`ENV_B` (extra environment, e.g. `MOEMON_DEBUG_PARTY=280:12`; LEVEL SCALING is off in tests unless a scenario
sets `MOEMON_LEVEL_SCALING=1`, common.sh),
and `EXPECT_A=(...)` / `EXPECT_B=(...)`: text that must show up in that client's log.

Scripts wait on game state (`waitfor`/`pressuntil` with a condition such as `field`, `action`, `battle`,
`partymenu`, `down`, `helpask`) or on a log line: `waitlog N TEXT` waits up to N frames for an `[mp]`
line containing TEXT, `pressuntillog KEYS N TEXT` taps KEYS meanwhile, and `watchlog TEXT` starts
watching before the key press that causes it (then a bare `waitlog N`). Fixed `wait`s break at high ping. `barrier NAME COUNT` holds a client until COUNT clients (they share the result folder) reach it: the
next lines run on the same frame (shared_pc_race grabs the same Moemon at once).
Species ids are the game's internal ones (Torchic 280, Mudkip 283; 252-276 are unused "?" slots). A log with
`TIMEOUT` (a `waitfor`/`pressuntil` that never got there) or a crash fails the scenario too.

The Windows build under Wine (`apt install wine64`), from the unzipped package:

    tools/test/wine_smoke.sh PKG_DIR     # solo boot + battle, then Windows server + Windows host + Linux joiner at 250 ms

REJOIN through a friend's own server (three sessions of one joiner, a host on a local server):

    tools/test/rejoin_smoke.sh [OUT]     # invite code + password once, then REJOIN and the remembered code (no typing)
    tools/test/kaisong_test.sh           # custom songs: the studio's exports play in the game sample for sample like
                                         # the studio (15 tracks / 64 voices too), by place, broken files skipped

Lower level, for trying things out (results and logs land in DIR):

    tools/test/run1.sh DIR [VAR=value...]       # DIR/a.txt, one client
    tools/test/run2.sh DIR [DELAY_MS [JIT_MS]]  # DIR/a.txt host + DIR/b.txt joiner, own relay server

`run2.sh` builds the relay server into `tools/test/.bin/` and picks free ports, so several runs can go at once.
Harness commands and conditions: see CLAUDE.md ("Headless testing") and `MP_TestCondition` in `src/mp/`.
Anything network related should pass `coop_wild_lag`-style settings: at 250 ms an ask-and-answer
through the relay takes over a second.

Move animations: `mp animall FIRST LAST ATK TGT [TURN]` plays every move animation from FIRST to LAST in a row (in a
battle, at the action menu); with `MOEMON_ANIMCHECK=1` the log lists every sprite animation table and number started,
to check against the tables' sizes (HYPER BEAM read past its 7-entry orb table).
