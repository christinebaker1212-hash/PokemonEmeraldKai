# POKESTADIUM 2 THEME: handoff

**POKESTADIUM 2 THEME** is the user's name for this design (2026-11-12): use it for the work, its notes and its
commits.

The user wants POKéMON STADIUM's screens on BATTLE ISLAND (src/mp/mp_stadium_ui.c) redone to look and flow like
POKéMON STADIUM 2. A session on 2026-11-11 / 12 studied the user's screenshots and recording and drew every scene as a
mock-up.

**Built (2026-11-12), both themes, every screen**: OPTIONS > BATTLE > STADIUM SETTINGS > STYLE (ORIGINAL / POKESTADIUM /
POKESTADIUM 2, gMpConfig.stadiumStyle, ini `stadium_style`), ANNOUNCER VOICE beside it. How it's made:
- tools/stadium/restyle/build_themes.py draws each screen's still layer and every piece (cursors, lit rows, chips,
  switches, neon / fat words, boxes cut into nine) with the mock-ups' own materials into graphics/stadium/themes.bin
  (committed; no fonts or numpy needed by the release) and src/data/stadium_themes.h (TH_* pieces, SX_ / SY_ places
  in real pixels of the 480x320 canvas, sThFatWords: POKESTADIUM's fat letters for every cup / stage / battle name).
- src/mp/mp_stadium_ui_themes.inc.c (included by mp_stadium_ui.c just before the canvas) draws the game's own things
  over them: ThemedDraw() picks S2* / S1* per screen; ORIGINAL draws as before.
- Words: the game's fonts with a **hard 2-pixel outline** (the user, 2026-11-12: never a soft outline or drop shadow;
  G_TextStyle with a negative PX in src/tcg/tcg_gfx.c). Hints are the game's own (controller buttons) on the foot.
- STADIUM 2's flow, themes only (ORIGINAL keeps its own): REGISTER's list 3 x 3 (REG_COLS / REG_ROWS follow the
  style), A opens the POKéMON's panel (ENTER Yes / No, SELECT the SUMMARY), six in asks OK to Register / Reselect
  some / Reselect all (U.sixMenu); the run goes ladder -> VS (shows) -> choice (U.vsStep: VS_LADDER / VS_SHOW /
  VS_CHOICE; VS. MEWTWO has no ladder; B on the choice goes back to VS); the result shows the three that fought on
  each side with their HP (StadiumCup_Fought, kept at the battle's end), crosses on the fainted, then (after
  TH_RESULT_HOLD frames) the badge board, Keep battling? or GAME OVER; the cup won shows its prizes.
- POKESTADIUM's cup screen shows all seven cups (the user), four over three, LEFT / RIGHT along a row, UP / DOWN
  between rows.
- Trainers' lines are left out (the user's answer). Tests: scenarios stadium_theme1 / stadium_theme2 (every screen
  shot), `mp stadiumstyle N`, `mp stadiumfill` (fill the team from the REGISTER list), `mp stadiumresult N`.
- Headless shots: the canvas is drawn only when a shot is taken, so the crossfade starts there: take a shot, wait
  14 frames, take it again.

The rest of this file is the design as it was handed over.

## The user's words (decisions, in order)

- "Show me what that might look like before committing anything." Mock-ups came first; the user has now seen the
  whole set and asked for these notes so another session finishes. **Show each screen running in the real game before
  releasing it** (they asked to see things before they land).
- "No agents, please."
- "I want the one I sent you to act as the choice screen" (docs/user_images/81.png: the staggered 1P / COM plates).
- "Follow the actual flow Pokemon Stadium 2 uses."
- "Ours should show held items only" (the entry screen never changes items: no ITEM row, no Change Item).
- "Here's an example of how I want it to look, all scenes" (the recording: docs/stadium/restyle/recording/).
- "Save this as \"POKESTADIUM 2 THEME\"" (the whole set of mock-ups below, as approved on 2026-11-12).
- "Please make sure everything is readable. Please be sure aspects are not stretched out - the video I sent you was
  4:3 stretched to 16:9." The frames kept here are squeezed back to 4:3. **Never copy a proportion from a stretched
  frame**: circles are circles, portraits square, letters their own shape.

## Where everything is

| What | Where |
|---|---|
| The user's screenshots | docs/user_images/76-81 (index in its README.md) |
| Frames of the user's recording (4:3) | docs/stadium/restyle/recording/ (`f_<second>.jpg`, `sheet_a/b.jpg` = a frame a second) |
| The mock-ups, 480x320 | docs/stadium/restyle/mockups/ |
| The scripts that draw them | tools/stadium/restyle/ (`prep.sh` once, then `screens.py`, `screens2.py`, `screens3.py`) |

The scripts are the specification: every colour, bevel and gradient in them was measured from the screenshots, and
they draw at the screens' own size with the game's own fonts (src/fonts.c, graphics/moemon/gFont*.bin) and pictures.
`s2.py` and `s3.py` hold the materials, the `screens*.py` files the layouts (coordinates are real pixels of the
480x320 canvas; text positions are the game's 240x160 units, as G_Text takes them).

## The flow to build (STADIUM 2's, from the recording)

| # | Scene | Mock-up | Today in the game |
|---|---|---|---|
| 1 | Entry: the 1P plate (six cells: icon, name, level, held item), tab keys PARTY / BOXES / SHARED / GLOBAL / RENTALS between L and R, the blue list (3 x 3 names, scrollbar, yellow bracket cursor) | s4_entry.png | DrawRegister (SCR_REGISTER) |
| 1b | A on a POKéMON: the list gives way to its blue info panel (picture, HP, types, five stats, four moves) and ENTER Yes / No, the held item under it | s4_entry_info.png | DetailCard + the A pop-up (sMenuRows) |
| 1c | Six in: OK to Register / Reselect some / Reselect all | s4_entry_menu.png | new (today: START = TEAM, B = DONE) |
| 2 | The cup's ladder: the opponent's portrait, "?" tiles for the battles to come, the cup and the battle on two bars | s4_ladder.png | new |
| 3 | The trainer's line under the ladder's top row | s4_line.png | new (needs a line per trainer: see open questions) |
| 4 | VS: two silver plates, portrait + six windows each, glowing VS, the clause chips between | s3_vs2.png | half of DrawVs (SCR_VS) |
| 5 | Choice: the staggered 1P / COM plates (each cell: pick button or empty socket, picture, level, held-item mark, name), the message in black glass, pick three in order | s4_choice.png (redrawn after the recording, 2026-11-12; s2_vs.png is the earlier draft) | the other half of DrawVs |
| 6 | the battle | | |
| 7 | Result: the three that fought on each side with HP bars, WIN / LOSE in neon | s3_result.png | DrawResult (SCR_RESULT) |
| 7b | then crosses over the fainted, the winner's plate leaves, the loser's line | s4_result2.png | new |
| 8 | GYM LEADER CASTLE only: the badge board, the badge won set into its socket | s4_badges.png | Badge() in DrawResult (RES_STAGE) |
| 9 | Keep battling? Continue / Suspend, the continues left | s4_continue.png | RES_CONTINUE in DrawResult |

Also drawn in the same materials: the cup list (s2_select.png, DrawSelect) and the class screen (s2_class.png,
DrawClass). The recording doesn't show STADIUM 2's own versions of those two; the user hasn't said whether to keep
these, so ask before building them. DrawCastle, DrawOptions, DrawTeams and DrawPrizes have no mock-up yet: restyle
them with the same pieces (black glass, plates, slabs) and show the user.

## The materials (what to port to C)

All of it is per-pixel maths on the 480x320 canvas, so C can match the pictures exactly. Build each piece once into an
SDL texture (or a nine-slice) when the screens open, not every frame. Functions to port, by name in the scripts:

- `Canvas.tube` / `glass` / `frame` (s2.py): the chrome tube round a rounded box (cross-section `TUBE`, lit from the
  top left: the bright crest sits toward the top / left on every edge), black glass inside (black at the top to
  0x63 grey at the foot, a light lip along the bottom), the portraits' double frame. Hard black shadow 3 px down-right.
- `Canvas.slab` + `sheen` + `engrave_*` (s2.py): the team plates: 1 px black line, 3 px bevel (light top / left, dark
  right / bottom, mitred), the sheen across (`SHEEN_BLUE` / `GREEN` / `GOLD` / `GREY`: left to right, dark, a bright
  band at 28-42%, a trough at 82-90%, bright again at the right edge), dividers as a dark line with a white one beside.
- `Canvas.tab`, `sphere`, `ball`, `pill`, `ring`, `strip` (s2.py): the side banners, glossy buttons, POKé BALL
  tokens, rule chips, the gold cursor ring, the black glass strip the hints sit in.
- `C3.plate` / `window` / `hpbar` (s3.py): the silver plates (3 px bevel, a soft sheen, fine brushed grain), picture
  windows set into them (shadowed top / left, white ledge bottom / right, black inside with the violet band 0xa0a2fa;
  gold 0xf0c838 for a pick), HP bars (cyan 0xadefef to 0x6fb0b3).
- `C3.neon` (s3.py): WIN (0xf8a010) / LOSE (0x30e0e8) hollow with a white line, VS solid white with a cyan glow. In
  the game these three words (and 1P / COM / the ladder's labels) are best shipped as small pre-drawn pictures built
  by a script in tools/stadium/ (the scripts use Carlito Bold Italic / DejaVu Sans Bold only as stand-ins: pick or
  draw lettering closer to STADIUM 2's, and keep its true proportions).
- `C3.night`, `sky`, `ribbon`, `tag`, `key`, `listpanel`, `cell`, `thumb`, `brackets`, `typepill` (s3.py): the black
  backdrop with dotted violet lines, the entry screen's sky, the titles' ribbon (orange here; the recording's LITTLE
  CUP uses cyan and a pink backdrop: a colour per cup is a nice touch), the 1P tag, tab keys, the blue list, its
  cells, the scroll thumb, the yellow bracket cursor.
- `C4.bluepanel`, `cyanbox`, `qtile`, `cross`, `pointer`, `itembox` (screens3.py).
- Text: the game's SMALL font, white with a 2 px (one unit) dark outline on plates and backdrops; on black glass and
  the strip, white with a 1 px outline drawn twice one pixel apart (`bold=True`). G_Text has no outline today: add
  one to src/tcg/tcg_gfx.c (the glyph texture drawn at the eight neighbours in the outline colour, then the face),
  kept off by default so the card games' screens don't change.

Readability rules that came out of the mock-ups (the user asked for these by name):
- A line of the SMALL font needs 10 units; never pack lines closer.
- A name is up to 10 letters (50 units), an item name up to 12-13; give every label its full width or it collides.
- No words over a busy picture without the outline; nothing under the hints' strip (it starts at y = 289 px).
- NORMAL (thin) only in large sizes on slabs; SMALL everywhere else.

## How to implement (suggested order)

1. **Drawing layer**: a new file (src/mp/mp_stadium_style.c + include) with the materials above, drawing into
   cached textures; an outline option for G_Text. Check each piece against the mock-up pixel for pixel in a
   headless test shot (the harness in CLAUDE.md; scenario stadium_screens walks the screens).
2. **Restyle the screens that exist**, one at a time, layout from the scripts: REGISTER (1, 1b), VS (4), RESULT (7).
   REGISTER's list becomes 3 x 3 names (REG_COLS / REG_ROWS, the scrollbar maths); the detail card becomes the info
   panel opened by A (its Yes = today's REGISTER / REMOVE; SUMMARY stays on SELECT).
3. **Split VS in two**: a new SCR_CHOICE after SCR_VS. SCR_VS only shows (A goes on, SELECT rules, B quit asks);
   SCR_CHOICE takes today's picking code (U.vsCursor, r->picks, START battles, R summary).
4. **New scenes**: SCR_LADDER before SCR_VS (the run's trainers: StadiumCup_TrainerCount, r->trainer; the castle has
   4 or 5 a stage), the trainer's line, the result's second step, the confirm menu, the continue box, the badge
   board (the real badge art: graphics/moemon/badges/badges.bin, as the BADGE CASE draws it; names from
   StadiumCastle_BadgeName).
5. **Movement**, from the recording (recording/sheet_a.jpg, sheet_b.jpg, a frame a second): plates slide in from
   the sides, the VS plates flash cyan before the teams show, WIN / LOSE land after the HP bars, the crosses stamp
   on, the winner's plate slides away before the loser's line. The screens already crossfade (U.xfade); keep the
   fades the user asked for on 2026-11-07.
6. Result data: the three that fought and their HP. mp_stadium_cup.c restores gPlayerParty from sSavedParty right
   after the battle (around the fainted count), so copy the three's species / HP / max HP for both sides just before
   that, for the result screen.
7. Tests: extend the stadium_* scenarios for the new steps (they press through VS today); stadium_screens,
   stadium_picks, stadium_win, stadium_fades at least. Then the usual release (CLAUDE.md), patch notes for players.

## Open questions for the user

- The trainers' lines (before the battle, after losing): STADIUM's own text from the ROM if the data has it
  (tools/stadium/), or leave the line boxes out? Don't write original lines without asking.
- Cups other than the castle: anything between the result and "Keep battling?" (the recording shows the badge
  board only because it was the castle)?
- The cup list and class screens: keep the mock-ups' look (s2_select.png, s2_class.png) or wait for references?
- The hints: a black glass strip along the bottom in the mock-ups; STADIUM 2 writes straight on the backdrop.
- The entry list shows names without levels (the font is too large for both in three columns); levels show on the
  1P plate and the info panel.
- Suspend (leave a cup and come back): does the game's run already allow it, or is the box Continue / Give up?

## Known rough spots in the mock-ups

- The pictures are the standard POKéMON art (graphics/pokemon); the game draws whichever art the player chose.
- Tall sprites lose their feet in the VS windows (52 px tall), as STADIUM 2 crops its models.
- s4_entry_info.png is tight along its bottom row; give it another unit when building.
- The badge board's badges are plain spheres standing in for the real art.

## POKESTADIUM THEME (the first STADIUM's look), 2026-11-12

"Now we'll build POKESTADIUM THEME from this video": a second theme, after the first POKéMON STADIUM, from a second
recording (4:3 as recorded, not stretched). Mock-ups only, like the first; nothing in the game has changed.
**Ask the user how the two themes live together** (the session assumed a setting that picks one; they haven't said).

| What | Where |
|---|---|
| Frames of the recording | docs/stadium/theme1/recording/ (`g_<second>.jpg`; `sheet_1/2.jpg` = a frame every two seconds) |
| The mock-ups, 480x320 | docs/stadium/theme1/mockups/ |
| The script | tools/stadium/restyle/stadium1.py (after `prep.sh`; it builds on s2.py / s3.py / screens3.py) |

The same flow as POKESTADIUM 2 THEME, in STADIUM's own dress:

| Scene | Mock-up | Its look |
|---|---|---|
| The cups | p1_cups.png | the cups' pictures as cards over the sky, the chosen one in yellow with white corner brackets, name / levels / a line in a violet band |
| The classes | p1_class.png | the cup's picture filling the screen, the title bar (a POKé BALL, a violet bar, yellow words), STADIUM's ball banners, a navy box in a pale line |
| Entry | p1_entry.png | the entry plate (violet cells: picture, name, level, item mark), the olive tab row (the chosen tab magenta in white), the violet list (3 x 3, name over level, white corner brackets) |
| Info panel | p1_info.png | olive-gold, its ENTER Yes / No in navy; held item shown only |
| The ladder | p1_ladder.png | an olive frame on the starfield, the cup and battle in fat gold letters over a picture, eight tiles with a red ? over a silhouette |
| VS | p1_vs.png | the blue grid plate (1P, name, portrait, six) over the green one, VS in orange |
| Choice | p1_choice.png | deep blue with the great POKé BALL, navy and green plates in a gold line, pick buttons on the pictures, the message in a green box |
| Result | p1_result.png | the three at twice their size on the grids, thin green HP bars, WIN red into yellow, LOSE in blue, red crosses and FAINTED |
| Badge board / continue | p1_badges.png, p1_continue.png | eight speckled colour tiles with the badge names up their sides, the badge set on its tile; Keep battling? in the navy box |

Materials are in `class S1` (stars, cup_backdrop, deep, titlebar, box, veil, gridplate, greenbar, fat, button, hints).
The hints sit straight on the backdrop here (no strip), as STADIUM writes them. STADIUM also has a Battle / Rules /
Registration menu and a Registered Pokémon list (g_7.jpg, g_13.jpg): not drawn, because the game's own flow goes cup,
class, options, register; ask before adding them. Stand-ins as in the first theme (lettering fonts, the badge, the
trainers' lines); the cup card for the ladder uses one of STADIUM's field pictures (stadium.bin picture 21).

## Every other screen the game has, in both themes (2026-11-12)

"Let's reconcile every screen that isn't represented here that needs to be represented in the game, such as
restriction picking, battle/rules/registration menus, and other modes like the Gym Leader Castle or Rival Battle, for
both." Drawn by tools/stadium/restyle/reconcile.py; `r_<screen>_s2.png` in docs/stadium/restyle/mockups/ (POKESTADIUM
2 THEME), `r_<screen>_s1.png` in docs/stadium/theme1/mockups/ (POKESTADIUM THEME).

| The game's screen (mp_stadium_ui.c) | Mock-up | What it is in STADIUM's terms |
|---|---|---|
| DrawOptions (SCR_OPTIONS): the four RESTRICTIONS switches and their stars, the help line, the team, REGISTER / SAVED TEAMS / START! | r_options | STADIUM's Battle / Rules / Registration menu: START! is Battle, REGISTER is Registration, the restrictions and THE RULES are Rules. The game keeps its own three buttons and its switches (STADIUM has no restrictions) |
| DrawCastle (SCR_SELECT's castle): eight gyms and the ELITE FOUR as rows (face, name, leader, badge), the stage under the cursor (its trainers, the badges won) | r_castle | the GYM LEADER CASTLE's stage list |
| DrawTeams (SCR_TEAMS): 99 saved teams, the slot under the cursor | r_teams | STADIUM's Registered Pokémon list (POKESTADIUM THEME follows it: a row a team with its six) |
| DrawVs with SELECT held: THE RULES | r_rules | Rules |
| DrawResult RES_CUP + DrawPrizes: the cup won, the road lit, the prizes | r_prizes | (the game's own) |

Covered by screens already drawn, not drawn again:
- **The RIVAL**: the castle's ninth stage is the ELITE FOUR then the RIVAL (five battles); it uses the ladder (five
  tiles, the last a "?" until reached), the VS, choice and result screens as they are. The game picks the RIVAL's team
  against the player's six, so the VS screen is where it's first seen.
- **A gym**: the ladder with four tiles; the badge board after it.
- **VS. MEWTWO**: one battle: the cup's menu (r_options), then VS with MEWTWO alone on the opponent's plate (five
  windows empty), choice, result. No ladder.
- **The CHALLENGE CUP**: r_options without the switches (the game's line about the six lent), no REGISTER.
- **The pop-ups** ("Quit this cup?", "Clear this cup's team?", SAVED TEAMS' menu and ERASE): the theme's menu box
  (the confirm menu's blue panel in POKESTADIUM 2 THEME, the navy box in POKESTADIUM THEME).
- RES_STAGE (a badge won) is the badge board; RES_CONTINUE / RES_OVER the continue box (GAME OVER: the same box
  without Continue).

Stand-ins here: the gym leaders' faces are HOENN's leaders (the game draws STADIUM's trainers' own pictures), the
stage and badge names are KANTO's as examples (StadiumCastle_StageName / BadgeName give the real ones), the prizes
are examples.

## The battle HUD in both themes (2026-11-12)

"How would you use POKESTADIUM and POKESTADIUM 2's presets to adjust the actual Battle UI?", then "just mock it up".
References: docs/user_images/82-85 (STADIUM 2's battle; none for STADIUM 1's, so POKESTADIUM THEME's battle HUD is the
theme's materials applied to the same layout, not copied from anything). Script tools/stadium/restyle/battleui.py;
mock-ups `b_s2_action.png` / `b_s2_move.png` (docs/stadium/restyle/mockups/) and `b_p1_*` (docs/stadium/theme1/mockups/).

**The battle is not a canvas.** The STADIUM menus own a 480x320 picture; a battle overlay goes over the GBA frame, so
these mock-ups are drawn at **240x160 in the game's fonts at 1x** - what the game can really put on screen. The way in
already exists: src/mp/mp_tricards.c parks the stock healthboxes off-screen (every battle routine keeps using them)
and composites its own cards over the finished frame from VDraw, in full colour past the GBA's palettes, fading with
MP_FadeFit. A themed HUD is that same hook with the theme's materials; MP_BattleOverlay_Composite is where it joins.

What each piece becomes (both mock-ups show the action menu and the move list):

| Piece | POKESTADIUM 2 | POKESTADIUM |
|---|---|---|
| The box | a chrome tube round black glass: name, L5, condition, gender, HP bar, numbers | the navy box in a pale line, name in yellow |
| HP bar | green / yellow / red over a dark track, a lit top row | the same, one pixel thinner |
| Portrait + team dots | a glass-framed portrait beside the box, dots for the team left | the navy frame, same dots |
| 1P tab, trainer's plate | small glass tabs | navy and green tabs |
| Action menu | chrome capsules, the chosen one lit gold with a cursor | navy capsules, gold edge on the chosen |
| Move list | chrome bars: name, type, PP; the chosen one in its type's colour | navy bars, the chosen in its type's colour |

Decisions made while drawing, for the user to confirm:
- **The boxes stay where the game puts them** (foe top left, yours bottom right). STADIUM 2 has 1P top left, but
  swapping sides would put each box over the other's POKéMON, and the type icons are placed off the healthbox
  sprites (COOP_NOTES "Type icons"), so they would have to move too.
- **The move diamond with its D-pad arrows does not fit 240x160** next to two boxes: the mock-up keeps STADIUM's bars
  (name, type, PP, the type's colour) as a 2x2 grid where the game already puts its move list.
- **The condition** reads as STADIUM's two letters (Nm, Ft...) rather than the game's status tag.
- Three team dots are drawn (STADIUM's three-POKéMON battles); the game would draw up to six.

Also watch: the bar must read the animated value (BarIsMoving, MP_TriCards_SetBarHp) or healing looks wrong;
battlers 4/5 exist in Triple Battles, so reuse mp_tricards.c's placement rather than a new one; compositing costs
time every frame, and turbo makes it worse. The arena behind the POKéMON in these mock-ups is a stand-in: map.bin
holds only the terrain platforms (those are the game's own), not the sky and crowd.
