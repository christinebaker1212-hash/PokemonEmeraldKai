# UI STYLE: the interface in six games' looks

The user's request (2026-11-15): a style selector for the whole interface, KAI (the game as it is, the default) and
RED, CRYSTAL, FIRERED, PLATINUM, BLACK, each from its own game's art, over Kai's own single, double and triple battle
layouts; a mock-up of every style on every screen it changes, shown for approval before anything is built.

**Status: mock-ups shown 2026-11-15, waiting for the user's approval.** Nothing in the game yet.

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
- RED / CRYSTAL (tools/ui_styles/gb.py): tile for tile from pret/pokered and pret/pokecrystal (gfx/font: the font, the
  box borders, the HUD's HP bar, :L and brackets; Crystal's frames, PACK and STATS pages), on the GBA's 30 x 20 tiles,
  in the Super Game Boy's greys with a coloured HP bar (RED) and the Game Boy Color's palettes (CRYSTAL).
- FIRERED (tools/ui_styles/firered.py): pret/pokefirered's own graphics: the font and its widths, the window frame,
  the battle textbox and its menus, the healthboxes put together as their OAMs are, the party background with its slots
  and box palettes, the BAG and its picture, the summary page.
- PLATINUM (tools/ui_styles/ds.py): its font and, from the user's ROM, the player's healthbox (pl_batt_obj 117 / cell
  116 / palette 0; the foe's and the doubles' its shorter form), the battle's message bar, the BAG's screen and bag;
  its touch-screen buttons (FIGHT red, BAG yellow, RUN blue, POKéMON green) beside the message on the one screen, its
  windows, party panels, summary and OPTIONS drawn in its shapes and colours.
- BLACK (tools/ui_styles/ds.py): its own font from the ROM (a/0/2/3, NFTR); Gen 5's dark slanted HP bars, flat round
  buttons, the menu as a grid of tiles, the BAG's ring of pockets, drawn in its colours.

    python3 tools/ui_styles/mockups.py KAI_SHOTS EXPORT OUT --pokered DIR --pokecrystal DIR --pokefirered DIR \
            --platinum ROM --black ROM
