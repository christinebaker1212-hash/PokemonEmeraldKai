# quetzal_rip

Kanto and Johto ripped from Moemon Quetzal (English Alpha 9 v1), in Kai's decomp formats, for a later port.
Not part of the build. Start with `docs/QUETZAL_RIP.md`.

Regenerate: `python3 tools/quetzal_rip/rip.py QUETZAL.gba --crystal PATH/pokecrystal --firered PATH/pokefirered`
(rewrites maps/, scripts/, layouts/, tilesets/, object_events/, trainer_pics/, data/, include/, previews/).
sound/ comes from tools/quetzal_rip/songs.py. Region map graphics are not ripped yet.
Check: `tools/quetzal_rip/check_build.sh QUETZAL.gba` (assembles everything with Kai's tools and compares it with the ROM).
