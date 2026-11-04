# Moemon Maker's editor (C++)

The editor's window and UI (docs/MOEMON_MAKER.md). Built into the game by the Makefile's MAKER_* rules; it talks to the
game only through include/maker_api.h (src/maker/*.c).

- `imgui/`: Dear ImGui v1.91.9b-docking (commit 4806a19, MIT, imgui/LICENSE.txt): the core, the SDL2 and SDL_Renderer
  back ends and misc/cpp/imgui_stdlib. Unchanged from upstream.
- `fonts/`: Liberation Sans Regular / Bold (SIL Open Font License 1.1, fonts/OFL.txt), built in by
  src/maker/maker_font.c.
