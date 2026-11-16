# UI STYLE: the interface in six games' looks

The user's request (2026-11-15): a style selector for the whole interface, KAI (the game as it is, the default) and
RED, CRYSTAL, FIRERED, PLATINUM, BLACK, each from its own game's art, over Kai's own single, double and triple battle
layouts; a mock-up of every style on every screen it changes, shown for approval before anything is built.

**Status (2026-11-16): built, in the game from 1.200.0** ("Approved, build the UI STYLE option."), with two fixes
asked for the same day: RED's party HP bars lost their bottom line (the fill was painted over it; fixed), and BLACK's
interface, its battles above all, "needing a rework" (done: the third pass below, shown again). Then (the user, the
same day): "when selecting something in a menu, or saving, the Emerald menu displays over the new UI styles. It's
jarring. Let's correct this anywhere it would occur": KAI's own pop-ups are drawn in the style too ("Building it").

The user's rules for the second pass: readability first; as much of each original's style as possible; KAI's fitment,
above all in Double and Triple Battles; each original game's own menu layouts (never KAI's layout in another game's
paint); every KAI feature usable in every style (six summary pages and RENAME, eight BAG pockets and SORT, five OPTIONS
tabs, six party slots, the Triple Battle type icons).

## Where it lives

OPTIONS > DISPLAY > UI STYLE (KAI / RED / CRYSTAL / FIRERED / PLATINUM / BLACK), a setting per save file like the rest
of OPTIONS. The style changes how things look, never what they do or what they're called: KAI's words, menus, pockets
and tabs stay; each style draws them.

## The screens a style changes

The message box, the START menu, a battle's command (single, double, triple), its moves, the party, the summary, the
BAG, OPTIONS: `docs/ui_styles/mockups/<screen>.png` shows one screen in all six styles, `<style>.png` one style on all
ten screens (mockups.py also writes `<style>/<screen>.png`, each at 2x, not kept here).

## Each style's art

- KAI: the game's own pictures (tools/test/scenarios/uicap_*; `mp hud 0` hides a battle's boxes for the others' scenes).
- Every style draws into KAI's own rectangles (tools/ui_styles/layout.py, measured from the game's screens with and
  without its boxes), so a style changes how the game looks and never where anything is; base.py holds the screens.
- RED / CRYSTAL (tools/ui_styles/gbstyle.py): tile for tile from pret/pokered and pret/pokecrystal. The battle HUDs are
  their tile grids (PlaceHUDTiles / DrawPlayerHUDBorder: the HP bar as DrawHPBar lays it, the line, corners and
  wedges; Crystal's EXP bar tiles filling from the right); Double and Triple boxes are built from the same pieces (the
  bar opened by RED's own edge or CRYSTAL's HP:), only empty pixel rows taken out to fit KAI's boxes. Text boxes, START,
  the status / STATS screens, ITEM / PACK and OPTION sit on the games' own coordinates (8 x 8 tiles); KAI's extras go in
  the columns the 20-tile screens don't have.
- FIRERED (tools/ui_styles/fr.py): pret/pokefirered's graphics and window templates: its healthboxes in KAI's boxes
  (the drop shadow's outermost row trimmed), its HP / EXP bar tiles stopping at the art's own end caps, the HP numbers
  in the game's number window over the art's "/", Triple Battle cards cut from its doubles box, both battle textbox
  palettes; the message box (menu_message), START with its help bar, the party slots (party_menu.h), the summary
  (six page dots), the BAG (pocket arrows beside the bag, eight pocket pips, START: SORT), OPTION (its help bar).
- tools/ui_styles/check.py runs with every render (and again with KAI's longest words: ten-letter names at level 100
  with 999 HP in the battles, six of them in the party, and a summary with a 7-letter OT, a 12-letter Ability, a
  13-letter item (PEWTERCRUNCHY) and a 16-letter place (EVER GRANDE CITY): `worst/<style>.png`; RED / CRYSTAL draw an eleventh name tile at a 7-pixel pitch as their games never needed,
  and print level 100 without the :L as PrintLevel does) and writes report.txt: anything drawn outside KAI's boxes, one-pixel
  seams, text overlapping text, text inked over other art (seen by drawing each screen again without its words), text
  off the screen. A finding that is the original game's own layout is listed with its reason, never hidden.
- PLATINUM (tools/ui_styles/pt.py): pret/pokeplatinum's graphics (POKEPLATINUM, a sparse checkout) and the user's ROM
  for its fonts (dsfont.Platinum: system, message, subscreen). The healthboxes are its own (enemy / player singles /
  player doubles, their HP and EXP gauges, the Lv icon, the gender marks), sliced at their plain columns to KAI's
  rectangles; the HP bar's arrow sits beside the box pointing at its POKéMON as Platinum draws it (outside KAI's
  rectangles in singles and doubles, the checker lists it; not in triples, where the cards stand in a column). A name
  of ten letters at level 100 doesn't fit Platinum's font in a triple's card: there, and only there, the name falls
  back to KAI's small font. The battle's lower screen is the touch screen's own buttons (FIGHT red, BAG yellow, RUN
  blue, POKéMON green; the moves as its type-coloured buttons with PP and the type's tag) beside its message window;
  the field message box and START are its windows (START's 20-pixel rows with its icons); the party its panels (six,
  with CANCEL); the summary its pages (the six page tabs as its icons, RENAME on SELECT, the trainer memo in place of
  the EXP rows); the BAG its screen and bag (eight pocket icons under the bag, FAVORITES as the MAIL pocket's icon,
  START: SORT), OPTIONS its frame with KAI's five tabs.
- BLACK (tools/ui_styles/bw.py): everything from the user's Black ROM, put together the way the decomp
  (Project-Holon/aletheia) says Black puts it together. The battle gauges are built exactly as btlv_gauge.c builds
  them (its writes of the name, gender, level, bar and number tiles into the frame, its actors' offsets read from
  overlay 94): no box, the name floating over the scene, the bar running in from the screen's edge to its arrow's
  point at KAI's box (outside KAI's rectangles, as Black's own; the checker lists it), the player's EXP bar a row
  under it. A double's box has no HP numbers (as Black's), its empty dark band shortened to KAI's height. A triple
  card follows KAI's own card: the name and gender floating on its first row, the Lv icon and level then Black's
  bar pieces (its point, HP label, gauge and cap, the plain white between them out, the gauge as long as the card
  allows) on its second, so a ten-letter name at level 100 fits whole.
  The battle's message window is Black's (dark, blended over the scene as btlv_scu.c blends it). The commands are
  four of Black's white-edged mesh buttons (the touch screen's long button 0x6B with its middle rows taken out, so its
  bevelled ends meet again as they did) in KAI's order, FIGHT! BAG / MOéMON RUN, on the touch screen's POKé BALL (red
  half, black band, white half), each with the touch screen's own lettering (POKéMON spelt MOéMON from its own
  letters); the D-pad cursor is Black's four pink arcs (btlv_input.c, a/0/1/1 0x77 / 0x78), standing out from the
  chosen button and clipped to it so they frame its ends. (Black's own command screen, a big FIGHT over three
  buttons, can't hold its lettering in KAI's 120 x 48: tried, the words met the panel's edges.) The moves are Black's
  move buttons in each type's palette, cut down the same way, the cursor on the first; the chosen move's type icon
  and PP on a button of its own.
  The field's talk balloon; START in KAI's own column (it used to spread over half the field and the player): Black's
  X-menu ground and folder-tab buttons one under another, the chosen one lit, the words in Black's letters on each
  tab's raised part (Black's icons, 20 to 28 pixels tall, leave no room for readable words in the column, so they go;
  EXIT the touch bar's return button). The party's plates (HP as the plate prints it, the maximum after the slash).
  The summary's INFO page with KAI's INFO in Black's places: Dex No., Name, Type, OT, ID No. and Ability on its six
  rows, the Ability's description in the grey band, the trainer memo on the seventh row and the white box under it,
  the nickname, gender and level in the box at the top right, SELECT: RENAME in the box at the bottom right (its
  header re-spelt MOéMON INFO from its own letters). The BAG (each pocket's own tab as Black draws it; POKé BALLS,
  BATTLE ITEMS and FAVORITES, which Black lacks, take the X menu's POKé BALL, the battle BAG's mark and the
  registered item's check) and OPTIONS (Black's six bars for KAI's six rows, each two rows shorter so all six sit
  whole under the title bar, KAI's tabs on it). Pieces bigger than KAI's room are cut down the way pixel art is: runs
  of identical rows or columns taken out (bw.py carve()), never scaled.
- Fixes found by the wider worst case (2026-11-16): FIRERED prints a held item longer than FireRed's own 12 letters
  in its small font; PLATINUM writes the met place without a stop, as its memo does, so Lv100 EVER GRANDE CITY fits.

    python3 tools/ui_styles/mockups.py KAI_SHOTS EXPORT OUT --pokered DIR --pokecrystal DIR --pokefirered DIR \
            --platinum ROM --black ROM

## Building it (1.200.0)

The game never draws a style itself from code: each style is a pack of pictures, fonts, strings and drawing specs that
tools/ui_styles/build_pack.py makes from the same mock-up code (uipack.py: the format, the ops, fields and flags;
`<style>_build.py` per style), and src/mp/mp_uistyle.c reads and draws over KAI's finished frame.
- Packs: uistyle/red.pak, crystal.pak, firered.pak (from the pret decomps, committed); sinnoh/ui_platinum.pak and
  unova/ui_black.pak (from the user's ROMs: built by the release from its secrets, never committed;
  tools/ui_styles/fetch_platinum.sh fetches pokeplatinum's interface art at the mock-ups' commit). A style whose pack
  isn't there keeps KAI's look (the log says so).
- check_specs.py draws each pack's specs as the game does (uipack.render) and the mock-ups' own code, pixel for
  pixel: the battles (every box kind with five worst-case names / levels / HP), the message box, START, OPTIONS, the
  BAG, the party, the summary's INFO page. Usage at the top of the file (KAI_SHOTS, MOEMON_EXPORT, CHECK_OUT).
- What the game draws in a style: the battle boxes, the command and move panels and the battle's message; the field's
  message box; START; the party; OPTIONS (five tabs); the BAG (eight pockets, SORT); the summary's six pages (INFO,
  SKILLS with IV stars / EVs, BATTLE MOVES and CONTEST MOVES with the picked move's numbers and words, POKé BALL,
  JOURNAL; RENAME on SELECT; the title follows OPTIONS > CREATURES); and KAI's own pop-ups everywhere on the field, in
  battle and over a style's screens: menus, YES / NO, the save's box, a shop's lists (menu.c's frames, PopUps), the
  battle's own boxes (HandleBattleWindow: YES / NO with its cursor, the level up's stats), each in the style's window
  frame and letters, red / grey words kept red / grey, KAI's button icons and extra symbols (+, Lv, PP, ID) as the
  style's letters. KAI's words, menus and layouts stay; only how they look changes.
- How the summary's pages are built per style: FIRERED, RED and CRYSTAL on their games' own status screens; PLATINUM on
  its page NSCRs with the rows each page needs (pt.py summary_ground's pick: the move rows' light stripe whole and ten of
  the dark one's rows, PP in its special digits); BLACK on its INFO (screen 63), STATS (73) and lined (64) pages of
  a/0/7/8, the contest icons spelt from its type icons' own letters (bw_build.py contest_icon).
- 1.200.3: a pop-up whose words wouldn't fit the style's letters, or that holds a picture or has sprites over it (the
  CO-OP menu), keeps KAI's own inside in the style's frame; long descriptions page by themselves; the BAG's picture
  follows the pocket and the player's gender (C_FEMALE) in each style that has one; BLACK's cursor is a ring traced
  round the button's own shape (bw.py command_rings / move_rings). COOP_NOTES "UI STYLE (1.200.0)" has the details.
- Tests: scenarios ui_style_battles, ui_style_screens (the summary's pages and OPTIONS in every style; shots looked at
  by hand), ui_style_dpad, ui_style_party_battles (the party from a Triple Battle; since 1.200.4 every battle's party
  is the style's screen), check_specs.py for every style after any spec change.
