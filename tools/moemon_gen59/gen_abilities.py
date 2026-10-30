#!/usr/bin/env python3
"""The Gen 5-9 abilities for Kai (after Gen 4's: 125-311 = pokeemerald-expansion's 124-310, Pickpocket .. Poison
Puppeteer): constants, names (12 letters), descriptions.

    python3 tools/moemon_gen59/gen_abilities.py [EXPANSION_DIR]

The expansion's src/data/abilities.h is read into tools/moemon_gen59/abilities.json, so later runs need no checkout.
Their effects are written by hand in the battle code (look for "Gen 5-9 abilities"); until then an ability does
nothing. Generated: the include/constants/abilities.h block and src/data/text/abilities_gen59.h.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'tools', 'moemon_gen4'))
from kaitext import kai_text  # noqa: E402

FIRST_KAI, FIRST_EXP, LAST_EXP = 125, 124, 310
SHORT = {   # names still over 12 letters
    'ELECTROMORPHOSIS': 'ELECTROMORPH', 'EMBODY ASPECT': 'EMBODYASPECT', 'SUPERSWEET SYRUP': 'SUPERSWEET',
    'POISON PUPPETEER': 'POISONPUPPET', 'PROTOSYNTHESIS': 'PROTOSYNTHES', 'SUPREME OVERLORD': 'SUPREME LORD',
    'TERAFORM ZERO': 'TERAFORMZERO', 'POWER OF ALCHEMY': 'PWR ALCHEMY', 'QUEENLY MAJESTY': 'QUEENLY MAJ.',
    'ICE SCALES': 'ICE SCALES', 'GORILLA TACTICS': 'GORILLA TACT', 'MIRROR ARMOR': 'MIRROR ARMOR',
    'GRASSY SURGE': 'GRASSY SURGE', 'PROPELLER TAIL': 'PROPELLRTAIL', 'NEUTRALIZING GAS': 'NEUTRAL. GAS',
    'STEELY SPIRIT': 'STEELYSPIRIT', 'PERISH BODY': 'PERISH BODY', 'WANDERING SPIRIT': 'WANDERSPIRIT',
    'QUICK DRAW': 'QUICK DRAW', 'UNSEEN FIST': 'UNSEEN FIST', 'CURIOUS MEDICINE': 'CURIOUS MED.',
    'TRANSISTOR': 'TRANSISTOR', "DRAGON'S MAW": "DRAGON'S MAW", 'CHILLING NEIGH': 'CHILLINGNEIGH'[:12],
    'GRIM NEIGH': 'GRIM NEIGH', 'AS ONE': 'AS ONE', 'LINGERING AROMA': 'LINGERAROMA', 'SEED SOWER': 'SEED SOWER',
    'THERMAL EXCHANGE': 'THERMAL EXCH', 'ANGER SHELL': 'ANGER SHELL', 'PURIFYING SALT': 'PURIFY SALT',
    'WELL-BAKED BODY': 'WELLBAKEDBDY', 'WIND RIDER': 'WIND RIDER', 'GUARD DOG': 'GUARD DOG', 'ROCKY PAYLOAD': 'ROCKYPAYLOAD',
    'WIND POWER': 'WIND POWER', 'ZERO TO HERO': 'ZERO TO HERO', 'COMMANDER': 'COMMANDER', 'QUARK DRIVE': 'QUARK DRIVE',
    'GOOD AS GOLD': 'GOOD AS GOLD', 'VESSEL OF RUIN': 'VESSELOFRUIN', 'SWORD OF RUIN': 'SWORD OF RUIN'[:12],
    'TABLETS OF RUIN': 'TABLETSOFRUIN'[:12], 'BEADS OF RUIN': 'BEADSOFRUIN', 'ORICHALCUM PULSE': 'ORICHALCUM',
    'HADRON ENGINE': 'HADRONENGINE', 'OPPORTUNIST': 'OPPORTUNIST', 'CUD CHEW': 'CUD CHEW', 'SHARPNESS': 'SHARPNESS',
    'COSTAR': 'COSTAR', 'TOXIC DEBRIS': 'TOXIC DEBRIS', 'ARMOR TAIL': 'ARMOR TAIL', 'EARTH EATER': 'EARTH EATER',
    'MYCELIUM MIGHT': 'MYCELIUM MGT', "MIND'S EYE": "MIND'S EYE", 'HOSPITALITY': 'HOSPITALITY', 'TOXIC CHAIN': 'TOXIC CHAIN',
    'TERA SHIFT': 'TERA SHIFT', 'TERA SHELL': 'TERA SHELL', 'MULTISCALE': 'MULTISCALE', 'WIMP OUT': 'WIMP OUT',
    'BULLETPROOF': 'BULLETPROOF', 'STAKEOUT': 'STAKEOUT', 'BEAST BOOST': 'BEAST BOOST', 'SURGE SURFER': 'SURGE SURFER',
    'FULL METAL BODY': 'FULLMETALBDY', 'SHADOW SHIELD': 'SHADOWSHIELD', 'PRISM ARMOR': 'PRISM ARMOR',
    'BATTLE BOND': 'BATTLE BOND', 'POWER CONSTRUCT': 'PWRCONSTRUCT', 'RKS SYSTEM': 'RKS SYSTEM',
    'ELECTRIC SURGE': 'ELEC. SURGE', 'PSYCHIC SURGE': 'PSYCH. SURGE', 'MISTY SURGE': 'MISTY SURGE',
    'INTREPID SWORD': 'INTREPIDSWRD', 'DAUNTLESS SHIELD': 'DAUNTLESSSHD', 'SCREEN CLEANER': 'SCREENCLEANR',
    'PASTEL VEIL': 'PASTEL VEIL', 'HUNGER SWITCH': 'HUNGERSWITCH', 'ICE FACE': 'ICE FACE', 'POWER SPOT': 'POWER SPOT',
    'SAND SPIT': 'SAND SPIT', 'COTTON DOWN': 'COTTON DOWN', 'STALWART': 'STALWART', 'GULP MISSILE': 'GULP MISSILE',
    'LIBERO': 'LIBERO', 'BALL FETCH': 'BALL FETCH', 'PUNK ROCK': 'PUNK ROCK', 'RIPEN': 'RIPEN',
    'COMPETITIVE': 'COMPETITIVE', 'STRONG JAW': 'STRONG JAW', 'REFRIGERATE': 'REFRIGERATE', 'SWEET VEIL': 'SWEET VEIL',
    'STANCE CHANGE': 'STANCECHANGE', 'GRASS PELT': 'GRASS PELT', 'SYMBIOSIS': 'SYMBIOSIS', 'TOUGH CLAWS': 'TOUGH CLAWS',
    'PIXILATE': 'PIXILATE', 'AERILATE': 'AERILATE', 'PARENTAL BOND': 'PARENTALBOND', 'DARK AURA': 'DARK AURA',
    'FAIRY AURA': 'FAIRY AURA', 'AURA BREAK': 'AURA BREAK', 'PRIMORDIAL SEA': 'PRIMORDIALSEA'[:12],
    'DESOLATE LAND': 'DESOLATELAND', 'DELTA STREAM': 'DELTA STREAM', 'STEAM ENGINE': 'STEAM ENGINE',
    'EMERGENCY EXIT': 'EMERGENCYEXT', 'WATER COMPACTION': 'WATERCOMPACT', 'SHIELDS DOWN': 'SHIELDS DOWN',
    'DAZZLING': 'DAZZLING', 'SOUL-HEART': 'SOUL-HEART', 'TANGLING HAIR': 'TANGLINGHAIR', 'RECEIVER': 'RECEIVER',
    'TRIAGE': 'TRIAGE', 'CORROSION': 'CORROSION', 'COMATOSE': 'COMATOSE', 'INNARDS OUT': 'INNARDS OUT',
    'DANCER': 'DANCER', 'BATTERY': 'BATTERY', 'FLUFFY': 'FLUFFY', 'MERCILESS': 'MERCILESS', 'GALVANIZE': 'GALVANIZE',
    'SLUSH RUSH': 'SLUSH RUSH', 'LONG REACH': 'LONG REACH', 'LIQUID VOICE': 'LIQUID VOICE', 'SCHOOLING': 'SCHOOLING',
    'DISGUISE': 'DISGUISE', 'SURGE': 'SURGE', 'FOREWARN': 'FOREWARN', 'MAGIC BOUNCE': 'MAGIC BOUNCE',
    'SAP SIPPER': 'SAP SIPPER', 'PRANKSTER': 'PRANKSTER', 'SAND FORCE': 'SAND FORCE', 'IRON BARBS': 'IRON BARBS',
    'ZEN MODE': 'ZEN MODE', 'VICTORY STAR': 'VICTORY STAR', 'TURBOBLAZE': 'TURBOBLAZE', 'TERAVOLT': 'TERAVOLT',
    'AROMA VEIL': 'AROMA VEIL', 'FLOWER VEIL': 'FLOWER VEIL', 'CHEEK POUCH': 'CHEEK POUCH', 'PROTEAN': 'PROTEAN',
    'FUR COAT': 'FUR COAT', 'MAGICIAN': 'MAGICIAN', 'GOOEY': 'GOOEY', 'GALE WINGS': 'GALE WINGS',
    'MEGA LAUNCHER': 'MEGALAUNCHER', 'POISON TOUCH': 'POISON TOUCH', 'REGENERATOR': 'REGENERATOR',
    'BIG PECKS': 'BIG PECKS', 'SAND RUSH': 'SAND RUSH', 'WONDER SKIN': 'WONDER SKIN', 'ANALYTIC': 'ANALYTIC',
    'ILLUSION': 'ILLUSION', 'IMPOSTER': 'IMPOSTER', 'INFILTRATOR': 'INFILTRATOR', 'MUMMY': 'MUMMY', 'MOXIE': 'MOXIE',
    'JUSTIFIED': 'JUSTIFIED', 'RATTLED': 'RATTLED', 'TELEPATHY': 'TELEPATHY', 'MOODY': 'MOODY', 'OVERCOAT': 'OVERCOAT',
    'POISON HEAL': 'POISON HEAL', 'FRIEND GUARD': 'FRIEND GUARD', 'WEAK ARMOR': 'WEAK ARMOR', 'HEAVY METAL': 'HEAVY METAL',
    'LIGHT METAL': 'LIGHT METAL', 'HARVEST': 'HARVEST', 'TOXIC BOOST': 'TOXIC BOOST', 'FLARE BOOST': 'FLARE BOOST',
    'HEALER': 'HEALER', 'IRON FIST': 'IRON FIST', 'PICKPOCKET': 'PICKPOCKET', 'SHEER FORCE': 'SHEER FORCE',
    'CONTRARY': 'CONTRARY', 'UNNERVE': 'UNNERVE', 'DEFIANT': 'DEFIANT', 'DEFEATIST': 'DEFEATIST', 'CURSED BODY': 'CURSED BODY',
}


def enum_ids(text):
    ids, n = {}, 0
    for m in re.finditer(r'^\s+(ABILITY_\w+|ABILITIES_\w+)(?:\s*=\s*(\w+))?,', text, re.M):
        name, val = m.group(1), m.group(2)
        if val:
            n = int(val) if val.isdigit() else ids.get(val, n)
        ids[name] = n
        n += 1
    return ids


def main():
    cache_p = os.path.join(HERE, 'abilities.json')
    if len(sys.argv) > 1:
        exp = sys.argv[1]
        ids = enum_ids(open(os.path.join(exp, 'include/constants/abilities.h'), encoding='utf-8').read())
        src = open(os.path.join(exp, 'src/data/abilities.h'), encoding='utf-8').read()
        out = []
        for a, n in sorted(ids.items(), key=lambda x: x[1]):
            if not a.startswith('ABILITY_') or n < FIRST_EXP or n > LAST_EXP:
                continue
            blk = re.search(r'\[%s\] =\s*\{(.*?)\n    \},' % a, src, re.S)
            body = re.sub(r'[ \t]*#if[^\n]*\n(.*?)[ \t]*#else[^\n]*\n.*?[ \t]*#endif[^\n]*\n', r'\1', blk.group(1), flags=re.S)
            name = re.search(r'\.name = _\("([^"]*)"\)', body).group(1)
            desc = re.search(r'\.description = (?:COMPOUND_STRING|_)\((.*?)\),\n', body, re.S)
            out.append({'const': a, 'expansion_id': n, 'name': name,
                        'description': ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', desc.group(1))) if desc else ''})
        json.dump(out, open(cache_p, 'w', encoding='utf-8'), indent=1, ensure_ascii=False)
    abil = json.load(open(cache_p, encoding='utf-8'))
    p = os.path.join(REPO, 'include', 'constants', 'abilities.h')
    s = open(p).read()
    s = s.replace('\n#define ABILITIES_COUNT 125\n// end of Gen 4 abilities\n', '\n// end of Gen 4 abilities\n')
    body = ''.join('#define %s %d\n' % (a['const'], FIRST_KAI + i) for i, a in enumerate(abil))
    block = ('// Moemon Kai: Gen 5-9 abilities (tools/moemon_gen59/gen_abilities.py)\n' + body +
             '\n#define FIRST_GEN59_ABILITY %d\n#define ABILITIES_COUNT %d\n// end of Gen 5-9 abilities\n' % (
                 FIRST_KAI, FIRST_KAI + len(abil)))
    if '// Moemon Kai: Gen 5-9 abilities' in s:
        s = re.sub(r'// Moemon Kai: Gen 5-9 abilities.*?// end of Gen 5-9 abilities\n', lambda m: block, s, flags=re.S)
    else:
        s = s.replace('// end of Gen 4 abilities\n', '// end of Gen 4 abilities\n' + block)
    open(p, 'w').write(s)
    out = ['// Generated by tools/moemon_gen59/gen_abilities.py. Do not edit.\n']
    cam = lambda c: ''.join(w.capitalize() for w in c[8:].split('_'))
    names = {}
    for a in abil:
        n = a['name'].upper().replace('É', 'é')
        n = SHORT.get(n, n)
        if len(n) > 12:
            n = n.replace(' ', '')[:12]
        names[a['const']] = n
        out.append('static const u8 s%sDescription[] = _("%s");\n' % (cam(a['const']), kai_text(a['description']).replace('\\n', ' ')))
    out.append('#define ABILITY_NAMES_GEN59 \\\n' + ' \\\n'.join('    [%s] = _("%s"),' % (a['const'], names[a['const']]) for a in abil) + '\n')
    out.append('#define ABILITY_DESCRIPTIONS_GEN59 \\\n' + ' \\\n'.join(
        '    [%s] = s%sDescription,' % (a['const'], cam(a['const'])) for a in abil) + '\n')
    open(os.path.join(REPO, 'src', 'data', 'text', 'abilities_gen59.h'), 'w', encoding='utf-8').write(''.join(out))
    print('%d abilities (%d-%d)' % (len(abil), FIRST_KAI, FIRST_KAI + len(abil) - 1))


if __name__ == '__main__':
    main()
