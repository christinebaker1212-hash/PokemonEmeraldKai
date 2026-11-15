# UI STYLE: the interface in six games' looks

The user's request (2026-11-15): a style selector for the whole interface, KAI (the game as it is, the default) and
RED, CRYSTAL, FIRERED, PLATINUM, BLACK, each from its own game's art, over Kai's own single, double and triple battle
layouts; a mock-up of every style on every screen it changes, shown for approval before anything is built.

**Status (2026-11-15): first mock-ups not approved ("everything other than Emerald looks extremely rough"); the
second pass is under way.** RED, CRYSTAL, FIRERED and PLATINUM are redone (below) and pass the checker, the worst case
too; BLACK is still the first pass and is being redone the same way, from Black's own ROM. Nothing in the game yet.

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
ten screens, `<style>/<screen>.png` each at 2x.

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
- tools/ui_styles/check.py runs with every render (and again on the battles with ten-letter names at level 100 and
  999 HP: `worst/<style>.png`; RED / CRYSTAL draw an eleventh name tile at a 7-pixel pitch as their games never needed,
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
- BLACK (tools/ui_styles/ds.py, first pass, being redone): its own font from the ROM (a/0/2/3, NFTR); Gen 5's dark slanted HP bars, flat round
  buttons, the menu as a grid of tiles, the BAG's ring of pockets, drawn in its colours.

    python3 tools/ui_styles/mockups.py KAI_SHOTS EXPORT OUT --pokered DIR --pokecrystal DIR --pokefirered DIR \
            --platinum ROM --black ROM
