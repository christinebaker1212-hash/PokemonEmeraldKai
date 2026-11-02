#!/usr/bin/env python3
"""The TRAINER CARD's designs (graphics/moemon/cards/passes.bin, built in by src/mp/mp_pass_data.c): Pokemon Battle
Revolution's Battle Pass designs, as the user sent them (docs/user_images/63.zip), from graphics/moemon/cards/passes/.
A design whose file name says "recolorable" can be turned to any hue on the card (MY CARD > HUE).

  python3 tools/moemon_cards/build_passes.py

passes.bin: "MPAS", u16 version 1, u16 count; then per design: char name[32] (as the card's menu shows it), u8 flags
(bit 0: recolorable), u8 pad[3], u32 size, the PNG file itself (the game reads it with stb_image)."""
import os, struct

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SRC = os.path.join(REPO, 'graphics', 'moemon', 'cards', 'passes')
DST = os.path.join(REPO, 'graphics', 'moemon', 'cards', 'passes.bin')

# the order the card's menu goes through them: the plain passes first (RENTAL PASS BLUE is a new card's), then the
# patterns, the legends, the POKeTOPIA colors and vehicles, the colosseums
ORDER = [
    'rental_pass_blue', 'rental_pass_green', 'rental_pass_yellow', 'rental_pass_purple', 'rental_pass_gray',
    'rental_pass_recolorable', 'shiny_cobalt_recolorable', 'stream_blue_recolorable', 'star_green_recolorable',
    'light_pastel_recolorable', 'pearl_wave', 'diamond_dust', 'ball_mosaic_red', 'plus_minus',
    'groudon_red', 'kyogre_blue', 'rayquaza_green',
    'poketopia_white', 'poketopia_silver', 'poketopia_gold', 'poketopia_bronze', 'poketopia_black',
    'poketopia_train_recolorable', 'poketopia_ship_recolorable', 'poketopia_helicopter_recolorable',
    'poketopia_jet_recolorable',
    'courtyard_colosseum', 'sunny_park_colosseum', 'main_street_colosseum', 'waterfall_colosseum',
    'gateway_colosseum', 'magma_colosseum', 'sunset_colosseum', 'neon_colosseum', 'crystal_colosseum',
    'stargazer_colosseum',
]


def shown_name(stem):
    words = stem.replace('_recolorable', '').split('_')
    name = ' '.join(w.upper() for w in words if w)
    return name.replace('POKETOPIA', 'POKéTOPIA')


def main():
    have = sorted(f[:-4] for f in os.listdir(SRC) if f.endswith('.png'))
    missing = [s for s in ORDER if s not in have]
    extra = [s for s in have if s not in ORDER]
    if missing:
        raise SystemExit('not in %s: %s' % (SRC, missing))
    out = bytearray(b'MPAS' + struct.pack('<HH', 1, len(ORDER) + len(extra)))
    for stem in ORDER + extra:
        png = open(os.path.join(SRC, stem + '.png'), 'rb').read()
        name = shown_name(stem).encode('utf-8')[:31]
        out += name + bytes(32 - len(name))
        out += struct.pack('<B3xI', 1 if 'recolorable' in stem else 0, len(png))
        out += png
    with open(DST, 'wb') as f:
        f.write(out)
    print('%s: %d designs, %d bytes' % (os.path.relpath(DST, REPO), len(ORDER) + len(extra), len(out)))


if __name__ == '__main__':
    main()
