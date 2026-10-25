# quetzal_rip

Kanto and Johto ripped from Moemon Quetzal (English Alpha 9 v1), in Kai's decomp formats, for a later port.
Not part of the build. Start with `docs/QUETZAL_RIP.md`.

Regenerate: `python3 tools/quetzal_rip/rip.py QUETZAL.gba --crystal PATH/pokecrystal --firered PATH/pokefirered`
(rewrites maps/, scripts/, layouts/, tilesets/, object_events/, trainer_pics/, data/, include/, previews/).
sound/ and region_map/ come from tools/quetzal_rip/songs.py and region_map.py.
Check: `tools/quetzal_rip/check_build.sh QUETZAL.gba` (assembles everything with Kai's tools and compares it with the ROM).
gifts.json / names.json come from tools/quetzal_rip/gifts.py (rip.py runs it).
gen4/ is the rip capped to Gen 4: `python3 tools/quetzal_rip/gencap.py --max-gen 4` (never edit by hand).
