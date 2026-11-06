# BATTLE ANIMS > MODERN: the audit and the rebuild (1.120.0)

OPTIONS > BATTLE > BATTLE ANIMS: CLASSIC / MODERN. **Classic is never changed** (the user, 2026-11-06: the full audit
"applies ONLY to Modern animations"): the Classic animation script always runs with the same sprites, sounds and frame
count, so a friend on CLASSIC stays in step (tests anim_classic / anim_modern compare every move's frame count,
tools/test/check_anim_frames.sh). MODERN is a layer drawn over the finished picture by src/mp/mp_modernanim.c.

Two rules from the user (2026-11-06):
- **Nothing of MODERN over the UI.** The renderer marks the pixels that come from BG0 (the battle's text box and menus)
  or from the battle interface's sprites (HP boxes, HP bars, the party's ball bars: MP_ModernAnim_IsUiSprite) in
  gMpUiLayer (src/platform/gba_easy_draw.c, sprite.c gMpOamUi); after MODERN draws, those pixels come back as the game
  drew them, and the impact shake moves the scene but never the UI. The type icons and Triple Battle cards are drawn
  after MODERN (sdl2.c), so they're on top anyway. Nothing is drawn on an animation's first frame (the mask isn't known
  yet). Check: `python3 tools/moemon_modernanim/ui_check.py OUTDIR` (CLASSIC vs MODERN, the text box pixel for pixel).
- **The stats going up / down keep the game's own animation**: MODERN draws nothing on B_ANIM_STATS_CHANGE, and the
  BUFF / DEBUFF families have no chevrons of their own.

## The audit (tools/moemon_modernanim)

- `MOEMON_MODERN_AUDIT=1` logs one line per animation ("modern audit: ... frames F hits splat / shake / sound / quake
  ... fx PIXELS peak P first H last L"). `audit_frames.txt` is that run for all 847 moves (`mp animall 1 847 0 1`,
  ALICE attacking): each Classic animation's length and first / last hit frame.
- `anim_shares.py`: which moves play the same Classic script. 848 moves, 472 scripts: **376 moves borrow another
  move's animation** (21 play RAZOR LEAF's, 21 THUNDERBOLT's, 20 FLAMETHROWER's, 19 GROWL's, 19 CRUNCH's...), some far
  from what they are (SHELL SMASH plays GROWL, JET PUNCH plays SURF, MAKE IT RAIN plays IRON TAIL, HEAT CRASH plays
  FLAMETHROWER, BOOMBURST plays TAKE DOWN). Before 1.120.0 MODERN looked the same for all of them.
- Findings of the old MODERN (1.43.0-1.119.0): one recipe for every move (the effect sprites glowing in the type's color,
  a burst at each hit, motes into the user, stat chevrons). **232 damaging moves never got a hit** (their Classic
  script has no hit splat, shake task or targeted sound, so no impact at all); **95 moves have no effect sprites** (only
  the Pokémon move), so the glow did nothing; status moves, healing, shields, weather, sounds and buffs had nothing of
  their own; additive light vanished on the pale battle fields.
- `sheets.py OUTDIR MOVE...` lays out contact sheets of MODERN frames (the audit's eyes; `--big`, `--step`).

## The rebuild

`gen_recipes.py` gives every move a **family** (how its energy moves) from its name, type, power, target, effect and
the Gen 5-9 flags (punching, biting, sound, pulse, ballistic, slicing, wind, dance, powder, healing, multi-hit), a
**variant** (BIG at 120+ power, SPREAD, DARK, TWIN, SPIRAL, RISING) and a **color** (the type's, or the move's own:
BLUE FLARE blue, MOONGEIST BEAM violet...), plus the audit's timing, into src/data/mp_modernanim_moves.h
(`--report` lists the families and their moves). Rerun it after adding moves or rerunning the audit.

The families, drawn along the Classic animation's own timing (the audit's hit frame, followed live when a hit comes
sooner):

| family | what it draws |
|---|---|
| STRIKE / RAM | speed lines from the user, (RAM) a comet tail into the target, an eight-point impact star |
| PUNCH / KICK | a tight ring and four heavy rays / a sweeping arc into the target, then the star |
| SLASH / BITE / PIERCE | three (two when BIG) bright cuts across the target / two jaws closing / a spear of light |
| BEAM | a beam from the user to the target: core, glow, pulses running down it, light at both ends (TWIN: two) |
| BREATH | a cone of the type's particles from the mouth to the target |
| ORB | a ball of light flying in an arc with a trail, a ring where it lands |
| FROMSKY | bolts (jagged for ELECTRIC), rocks, ice coming down onto the target |
| QUAKE / BLAST / WAVE | cracks along the ground and the heaving waves / an explosion (on the user for EXPLOSION) / a band sweeping the field |
| SOUND / WIND / MIND / SHADOW | arcs from the mouth / swirling arcs / rings closing in and the air rippled / a dark miasma with wisps |
| POWDER / STATUS / DRAIN | motes drifting down / a stream of rings and a ring settling / orbs flowing back to the user |
| HEAL / BUFF / DEBUFF | soft light, stars and plus signs / a column of light and rising streaks / a dark column and sinking streaks |
| SHIELD / FIELD / SWAP / CHARGE / DANCE / MULTI | a hexagon shield / the field washed in the color with particles / two lights trading places / motes gathering / ribbons circling / a star per blow |

A damaging move whose Classic script has no hit gets MODERN's own at the family's moment (MULTI: three blows), so every
damaging move now lands. Everything stays gentle on the eyes: no full-screen flash, light local and fading, the shake
2 pixels at most and only on the scene.
