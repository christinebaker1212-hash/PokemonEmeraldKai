#!/usr/bin/env python3
"""JOHTO's people drawn as someone else (2026-11-15, the sprite audit): each object matched to its Crystal person by its
words (pokecrystal maps/*.asm, their texts against the object's script's), then drawn as the sprite Crystal gives that
person where Quetzal's choice is the wrong sex, age or person (a FISHER as a woman, a female GRUNT as a man, TUSCANY of
TUESDAY as a fat man...). Quetzal's other choices stand: Crystal had few sprites and drew a CAMPER or a PSYCHIC as a
YOUNGSTER, and Quetzal's more exact ones keep the battle pictures they go with (MAHOGANY's EXPERTS).

KANTO matches FireRed object for object; its two stand-ins (the TOWN MAP in BLUE's house, the docked S.S. ANNE) were
sizes the import couldn't take: tools/quetzal_port/gen_objects.py PADDED / BLANK.

    python3 tools/quetzal_port/fix_sprites.py     (idempotent: an object already changed is left alone)
"""
import os
import re

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))

# (map, x, y, Quetzal's sprite, Crystal's person's): why
FIXES = [
    ('EarlsPokemonAcademy', 6, 10, 'QZ_16', 'QZ_35'),                 # the BERRY student: Crystal's YOUNGSTER, not a woman
    ('GoldenrodCity', 20, 30, 'QZ_55', 'QZ_16015'),                   # "the RADIO TOWER is a landmark": COOLTRAINER_F, not a hiker
    ('GoldenrodCity', 28, 30, 'QZ_16047', 'QZ_16048'),                # Crystal's ROCKET (a man), not the female GRUNT
    ('GoldenrodUndergroundSwitchRoomEntrances', 28, 13, 'QZ_16048', 'QZ_16047'),   # GRUNTF_3: a woman
    ('OlivinePunishmentSpeechHouse', 8, 5, 'QZ_11', 'QZ_47'),         # "big bro always scares me": Crystal's LASS, a girl
    ('RadioTower2F', 26, 14, 'QZ_16007', 'QZ_16002'),                 # BUENA's prize RECEPTIONIST, not a boy
    ('Route29', 25, 12, 'QZ_17', 'QZ_20'),                            # TUSCANY of TUESDAY: Crystal's TEACHER
    ('Route29', 6, 22, 'QZ_16', 'QZ_16021'),                          # "I saved to record my progress": the FISHER
    ('Route43Gate', 7, 3, 'QZ_16047', 'QZ_16048'),                    # "We're making a bundle": Crystal's ROCKET (a man)
    ('Route40', 19, 13, 'QZ_47', 'QZ_45'),                            # MONICA of MONDAY: Crystal's BEAUTY
    ('LakeOfRage', 55, 15, 'QZ_44', 'QZ_16055'),                      # WESLEY of WEDNESDAY: Crystal's SUPER_NERD
    ('IndigoPlateauPokecenter1F', 30, 14, 'QZ_37', 'QZ_16038'),       # the old man whose ABRA teleports: Crystal's GRAMPS
    ('DanceTheater', 3, 9, 'QZ_16038', 'QZ_16016'),                   # "That man's always with his RHYDON": COOLTRAINER_M
]


def main():
    for m, x, y, old, new in FIXES:
        p = os.path.join(REPO, 'data/maps/Johto_%s/map.json' % m)
        s = open(p, encoding='utf-8').read()
        pat = re.compile(r'("graphics_id": "OBJ_EVENT_GFX_)' + old + r'(",\s*"x": %d,\s*"y": %d,)' % (x, y))
        s, n = pat.subn(r'\g<1>' + new + r'\2', s)
        if n:
            open(p, 'w', encoding='utf-8').write(s)
        print('%-45s %2d,%2d %s' % (m, x, y, 'changed' if n else 'already'))


if __name__ == '__main__':
    main()
