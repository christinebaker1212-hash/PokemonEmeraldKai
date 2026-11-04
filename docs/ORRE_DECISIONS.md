# ORRE (Pokémon Colosseum's region): the user's decisions

Orre as a real playable region, ported from Pokémon Colosseum (GC6E01) through the Colosseum decomp
(christinebaker1212-hash/pkmn-colosseum) and the user's own disc image.

## Decided (the user, 2026-11-04)
- **Overworld**: drawn by the SINNOH 3D engine (src/sinnoh/s3d.c) from Colosseum's own map models, with every
  Colosseum camera replaced by a fixed Platinum-style camera. Game logic stays on Emerald's grid, as SINNOH's does.
- **Sprites**: everything on the map is a 2D sprite standing in the 3D scene, never Colosseum's 3D characters.
  NPC sprites (and trainers' battle pictures) are rendered from Colosseum's own models so they look like GBA sprites.
- **Battles**: Kai's own battle system (2D, co-op, PvP), never Colosseum's 3D battle scenes.
- **Words and scripts**: Colosseum's own, converted from the game (as SINNOH's are Platinum's): never original
  dialogue or scripts for Orre.
- **Shadow Pokémon**: only in Orre (snagging, the heart gauge, Hyper Mode, Shadow moves, purification).

## Rules to keep
- The disc image and anything extracted from it (models, textures, collision, text, scripts) are never committed
  or shipped: converted at build time from the user's own disc, as the arcade cabinets read the user's ROM sets.
- Saves never change layout: Orre's state (heart gauges, Shadow flags) goes in a sidecar.

## Open
- Orre's tile scale (how many tiles a Colosseum room becomes): decided from the first map in the engine.
