# UI STYLE: the interface in six games' looks

The user's request (2026-11-15): a style selector for the whole interface, KAI (the game as it is, the default) and
RED, CRYSTAL, FIRERED, PLATINUM, BLACK, each from its own game's art, over Kai's own single, double and triple battle
layouts; a mock-up of every style on every screen it changes, shown for approval before anything is built.

**Status (2026-11-15): first mock-ups not approved ("everything other than Emerald looks extremely rough"); the
second pass is under way.** RED, CRYSTAL and FIRERED are redone (below) and pass the checker; PLATINUM and BLACK are
still the first pass and are being redone the same way. Nothing in the game yet.

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
- tools/ui_styles/check.py runs with every render and writes report.txt: anything drawn outside KAI's boxes, one-pixel
  seams, text overlapping text, text inked over other art (seen by drawing each screen again without its words), text
  off the screen. A finding that is the original game's own layout is listed with its reason, never hidden.
- PLATINUM (tools/ui_styles/ds.py, first pass, being redone): its font and, from the user's ROM, (wrongly) pl_batt_obj
  117 as the healthbox; the real ones, by the decomp's res/graphics/battle/sprites.order: enemy 188 / short_cell 187,
  player singles 191 / tall_cell 190, player doubles 200 and 203 (cells 199 / 202), enemy doubles 194 / 197, palette
  primary.NCLR 71; the battle's message bar, the BAG's screen and bag;
  its touch-screen buttons (FIGHT red, BAG yellow, RUN blue, POKéMON green) beside the message on the one screen, its
  windows, party panels, summary and OPTIONS drawn in its shapes and colours.
- BLACK (tools/ui_styles/ds.py, first pass, being redone): its own font from the ROM (a/0/2/3, NFTR); Gen 5's dark slanted HP bars, flat round
  buttons, the menu as a grid of tiles, the BAG's ring of pockets, drawn in its colours.

    python3 tools/ui_styles/mockups.py KAI_SHOTS EXPORT OUT --pokered DIR --pokecrystal DIR --pokefirered DIR \
            --platinum ROM --black ROM
