#!/usr/bin/env python3
"""JOHTO's phone, Crystal's own (pokecrystal): the trainers who give their numbers, what they say and do when they
call or are called, when they want a rematch and with which team, the gifts they keep for the player. Every script is
Crystal's, converted to a small bytecode src/mp/mp_qzphone.c runs (Crystal's script semantics: `end` returns from a
call, conditions test the last value): the phone scripts (engine/phone/scripts), the number-asking ones
(engine/events/std_scripts.asm) and each contact's trainer script on its map (the `.Script` after `trainer`, which runs
when the beaten trainer is talked to). Words are Crystal's (data/phone/text, data/text, maps), as paragraphs the game
wraps to the window it prints in. Rematch teams are Crystal's (data/trainers/parties.asm); a contact's first team is
Quetzal's trainer on the same map.

    python3 -I tools/quetzal_port/gen_phone.py POKECRYSTAL_DIR      (writes src/data/qz_phone.h)
"""
import glob
import json
import os
import re
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))

OPS = ['END', 'JUMP', 'CALL', 'IFTRUE', 'IFFALSE', 'IFEQ', 'IFNE', 'IFGT', 'IFLT', 'CHECKFLAG', 'SETFLAG', 'CLEARFLAG',
       'CHECKSTORY', 'TEXT', 'PROMPT', 'YESNO', 'RANDOM', 'READVAR', 'CHECKTIME', 'TRAINERNAME', 'CLASSNAME', 'LANDMARK',
       'STRING', 'SPECIAL', 'SETCALLER', 'CHECKCELL', 'ASKNUMBER', 'READMEM', 'LOADMEM', 'LOADTRAINER', 'STARTBATTLE',
       'WINLOSS', 'GIVEITEM', 'SETVAL', 'SWARM', 'REGSOUND', 'CURLANDMARK', 'MONNAME', 'CHECKMONEY', 'NOP']
OP = {n: i for i, n in enumerate(OPS)}
VARS = ['VAR_WEEKDAY', 'VAR_HOUR', 'VAR_CALLERID', 'VAR_MAPGROUP', 'VAR_KENJI_BREAK', 'VAR_ENVIRONMENT', 'VAR_BOXSPACE',
        'VAR_SPECIALPHONECALL']
SPECIALS = ['RandomPhoneMon', 'RandomPhoneWildMon', 'RandomUnseenWildMon', 'ActivateFishingSwarm',
            'SampleKenjiBreakCountdown']
TIMES = {'MORN': 1, 'DAY': 2, 'NITE': 4}
WEEKDAYS = {'SUNDAY': 0, 'MONDAY': 1, 'TUESDAY': 2, 'WEDNESDAY': 3, 'THURSDAY': 4, 'FRIDAY': 5, 'SATURDAY': 6}
NUMS = {'FISHSWARM_QWILFISH': 1, 'FISHSWARM_REMORAID': 2, 'PHONE_CONTACTS_FULL': 1, 'PHONE_CONTACT_REFUSED': 2, 'TRUE': 1, 'FALSE': 0, 'MAPGROUP_DUNGEONS': 255,
        'CAVE': 1, 'DUNGEON': 1, 'ENVIRONMENT_5': 0}
# Crystal's story marks -> what the game checks (src/mp/mp_qzphone.c Story)
STORY = ['ENGINE_FLYPOINT_GOLDENROD', 'ENGINE_FLYPOINT_ECRUTEAK', 'ENGINE_FLYPOINT_OLIVINE', 'ENGINE_FLYPOINT_CIANWOOD',
         'ENGINE_FLYPOINT_MAHOGANY', 'ENGINE_FLYPOINT_LAKE_OF_RAGE', 'ENGINE_FLYPOINT_BLACKTHORN',
         'ENGINE_FLYPOINT_VERMILION', 'ENGINE_PLAINBADGE', 'ENGINE_FOGBADGE', 'ENGINE_RISINGBADGE', 'ENGINE_MARSHBADGE',
         'ENGINE_ROCKETS_IN_RADIO_TOWER', 'EVENT_BEAT_ELITE_FOUR', 'EVENT_CLEARED_RADIO_TOWER',
         'EVENT_CLEARED_ROCKET_HIDEOUT', 'EVENT_RESTORED_POWER_TO_KANTO', 'EVENT_OPENED_MT_SILVER',
         'ENGINE_DAILY_BUG_CONTEST', 'ENGINE_GOLDENROD_DEPT_STORE_SALE_IS_ON', 'EVENT_JASMINE_RETURNED_TO_GYM',
         'EVENT_FOUGHT_SNORLAX']
SKIP_SCRIPTS = {'mom.asm', 'elm.asm', 'bill.asm', 'buena.asm', 'bike_shop.asm', 'unused.asm'}
# Crystal's items given by contacts -> the game's
ITEMS = {'BERRY': 'ITEM_ORAN_BERRY', 'BITTER_BERRY': 'ITEM_PERSIM_BERRY', 'PRZCUREBERRY': 'ITEM_CHERI_BERRY',
         'PSNCUREBERRY': 'ITEM_PECHA_BERRY', 'POKE_BALL': 'ITEM_POKE_BALL', 'GREAT_BALL': 'ITEM_GREAT_BALL',
         'ULTRA_BALL': 'ITEM_ULTRA_BALL', 'THUNDERSTONE': 'ITEM_THUNDER_STONE', 'PINK_BOW': 'ITEM_SILK_SCARF',
         'NO_ITEM': 'ITEM_NONE'}
SPECIES_FIX = {'NIDORAN_F': 'NIDORAN_F', 'NIDORAN_M': 'NIDORAN_M', 'MR__MIME': 'MR_MIME', 'FARFETCH_D': 'FARFETCHD'}
MOVE_FIX = {'PSYCHIC_M': 'PSYCHIC', 'HI_JUMP_KICK': 'HI_JUMP_KICK', 'HIGH_JUMP_KICK': 'HI_JUMP_KICK',
            'BUBBLEBEAM': 'BUBBLE_BEAM', 'CONVERSION2': 'CONVERSION_2', 'DOUBLESLAP': 'DOUBLE_SLAP',
            'POISONPOWDER': 'POISON_POWDER', 'SELFDESTRUCT': 'SELF_DESTRUCT', 'SOFTBOILED': 'SOFT_BOILED',
            'SOLARBEAM': 'SOLAR_BEAM', 'SONICBOOM': 'SONIC_BOOM', 'THUNDERPUNCH': 'THUNDER_PUNCH',
            'THUNDERSHOCK': 'THUNDER_SHOCK'}


def read(path):
    return open(path, encoding='utf-8').read()


# ------------------------------------------------------------------------------------------------- asm parsing
def parse_asm(text):
    """[(label or None, command, args)] with local labels made global ('Global.Local')"""
    out, glob_label = [], None
    for raw in text.split('\n'):
        line = raw.split(';', 1)[0].rstrip() if '"' not in raw else raw.rstrip()
        if not line.strip():
            continue
        m = re.match(r'^(\.?\w+):{1,2}\s*$', line) or re.match(r'^(\.\w+)\s*$', line)   # (Route38's .Script has no colon)
        if m:
            name = m.group(1)
            if name.startswith('.'):
                name = (glob_label or '') + name
            else:
                glob_label = name
            out.append((name, None, None))
            continue
        m = re.match(r'^\s+(\w+)\s*(.*)$', line)
        if m:
            args = [a.strip() for a in split_args(m.group(2))] if m.group(2).strip() else []
            args = [(glob_label + a) if a.startswith('.') else a for a in args]
            out.append((None, m.group(1), args))
    return out


def split_args(s):
    out, cur, q = [], '', False
    for c in s:
        if c == '"':
            q = not q
        if c == ',' and not q:
            out.append(cur)
            cur = ''
        else:
            cur += c
    out.append(cur)
    return out


# ------------------------------------------------------------------------------------------------- texts
def conv_chars(s):
    s = s.replace('<PLAY_G>', '{PLAYER}').replace('<PLAYER>', '{PLAYER}').replace('<RIVAL>', '{RIVAL}')
    s = s.replace('<……>', '……').replace('<PKMN>', 'MOéMON').replace('<BSP>', ' ').replace('<PK><MN>', 'MOéMON')
    s = s.replace('#MON', 'MOéMON').replace('#', 'POKé').replace("'", '’').replace('@', '')
    s = s.replace('{d:NUM_TMS}', '50')   # (Crystal's count of its TMs, in its own words)
    return s


RAM = {'wStringBuffer3': '{STR_VAR_1}', 'wStringBuffer4': '{STR_VAR_2}', 'wStringBuffer5': '{STR_VAR_3}',
       'wStringBuffer1': '{STR_VAR_1}', 'wStringBuffer2': '{STR_VAR_2}'}


def text_blocks(paths):
    """{label: [paragraph, ...]} from text / line / cont / para / text_ram / done blocks"""
    out = {}
    for p in paths:
        cur, paras, para = None, None, ''
        for raw in read(p).split('\n'):
            m1 = re.match(r'^(\w+):{1,2}\s+db\s+"([^"]*)"', raw)
            if m1:   # (a one-line string: MomString: db "Mom@")
                out[m1.group(1)] = [conv_chars(m1.group(2))]
                continue
            m = re.match(r'^(\w+):{1,2}\s*$', raw.strip())
            if m and not raw.startswith('\t') and not raw.startswith(' '):
                cur, paras, para = m.group(1), [], ''
                continue
            if cur is None:
                continue
            m = re.match(r'^\s+(text|line|cont|para|text_start|text_ram|done|prompt|text_promptbutton|db)\b\s*(.*)$', raw)
            if not m:
                continue
            cmd, arg = m.group(1), m.group(2).strip()
            s = ''
            if arg.startswith('"'):
                s = conv_chars(re.match(r'"(.*)"', arg).group(1))
            if cmd == 'text_ram':
                para += RAM.get(arg, '')
            elif cmd in ('text', 'text_start'):
                para += s
            elif cmd in ('line', 'cont'):
                para = join_line(para, s)
            elif cmd == 'para':
                paras.append(para.strip())
                para = s
            elif cmd == 'db' and arg.startswith('"'):   # (a string label: getstring)
                out[cur] = [s]
                cur = None
            elif cmd in ('done', 'prompt'):
                paras.append(para.strip())
                out[cur] = [x for x in paras if x]
                cur = None
    return out


def join_line(a, b):
    if not a:
        return b
    if a.endswith('-') and not a.endswith(' -'):
        return a + b
    return a + ' ' + b


def c_string(s):
    return '_("%s")' % s.replace('\\', '\\\\').replace('"', '\\"')


# ------------------------------------------------------------------------------------------------- main
def main():
    pc = sys.argv[1]
    contacts_src = read(os.path.join(pc, 'data/phone/phone_contacts.asm'))
    phone_consts = re.findall(r'^\s+(const_skip|const)\b[ \t]*(\w*)', read(os.path.join(pc, 'constants/phone_constants.asm')).split('DEF NUM_PHONE_CONTACTS')[0], re.M)
    phone_index, n = {}, 0
    for kind, name in phone_consts:
        if kind == 'const':
            phone_index[name] = n
        n += 1
    rows = re.findall(r'^\s+phone (\w+),\s+(\w+),\s+(\w+),\s+(\w+),\s+(\w+),\s+(\w+),\s+(\w+)', contacts_src, re.M)
    contacts = []
    for i, (cls, tid, mp, ctime, callee, wtime, caller) in enumerate(rows):
        if cls == 'TRAINER_NONE':
            continue
        const = [k for k, v in phone_index.items() if v == i][0]
        contacts.append({'index': i, 'const': const, 'class': cls, 'id': tid, 'map': mp, 'callee': callee, 'caller': caller})
    contact_by_const = {c['const']: c for c in contacts}

    # trainer classes, ids, names
    tconst = read(os.path.join(pc, 'constants/trainer_constants.asm'))
    classes, class_ids = [], {}
    cur = None
    for m in re.finditer(r'^\s+(trainerclass|const)\s+(\w+)', tconst, re.M):
        if m.group(1) == 'trainerclass':
            cur = m.group(2)
            classes.append(cur)
            class_ids[cur] = []
        elif cur:
            class_ids[cur].append(m.group(2))
    class_names = [conv_chars(x) for x in re.findall(r'^\s+li "([^"]*)"', read(os.path.join(pc, 'data/trainers/class_names.asm')), re.M)]
    parties_src = read(os.path.join(pc, 'data/trainers/parties.asm'))
    groups = re.split(r'^(\w+)Group:\s*$', parties_src, flags=re.M)
    group_entries = {}
    for k in range(1, len(groups), 2):
        entries = re.split(r'^\s*; [^\n]*\(\d+\)\s*$', groups[k + 1], flags=re.M)
        group_entries[groups[k]] = [e for e in entries if 'db "' in e]
    class_group = {}
    tptr = read(os.path.join(pc, 'data/trainers/party_pointers.asm'))
    gnames = re.findall(r'^\s+dw (\w+)Group', tptr, re.M)
    for cls, g in zip(classes[1:], gnames):   # (class 0 is TRAINER_NONE)
        class_group[cls] = g

    def party(cls, tid):
        idx = class_ids[cls].index(tid)
        e = group_entries[class_group[cls]][idx]
        name = re.search(r'db "([^@"]*)@?"?,\s*(\w+)', e)
        kind = name.group(2)
        mons = []
        for line in re.findall(r'^\s+db\s+(-?\d+),\s*(.+)$', e, re.M):
            lvl = int(line[0])
            if lvl < 0:
                break
            parts = [p.strip() for p in line[1].split(',')]
            sp = parts[0]
            item, moves = None, []
            if kind == 'TRAINERTYPE_ITEM':
                item = parts[1]
            elif kind == 'TRAINERTYPE_MOVES':
                moves = parts[1:5]
            elif kind == 'TRAINERTYPE_ITEM_MOVES':
                item, moves = parts[1], parts[2:6]
            mons.append((lvl, sp, item, moves))
        return conv_chars(name.group(1)), kind, mons

    # landmark names
    lm_consts = re.findall(r'^\s+const (LANDMARK_\w+)', read(os.path.join(pc, 'constants/landmark_constants.asm')), re.M)
    lm_src = read(os.path.join(pc, 'data/maps/landmarks.asm'))
    lm_labels = re.findall(r'^\s+landmark\s+-?\d+,\s*-?\d+,\s*(\w+)', lm_src, re.M)
    lm_names = dict(re.findall(r'^(\w+):\s+db "([^"]*)"', lm_src, re.M))
    landmark = {c: conv_chars(lm_names[l]) for c, l in zip(lm_consts, lm_labels)}

    # code sources
    code = []
    for p in sorted(glob.glob(os.path.join(pc, 'engine/phone/scripts/*.asm'))):
        if os.path.basename(p) not in SKIP_SCRIPTS:
            code += parse_asm(read(p))
    std = parse_asm(read(os.path.join(pc, 'engine/events/std_scripts.asm')))
    maps_src = {}
    for c in contacts:
        for p in glob.glob(os.path.join(pc, 'maps/*.asm')):
            s = read(p)
            m = re.search(r'^(\w+):\n\ttrainer %s, (%s\d*),' % (c['class'], c['id'].rstrip('0123456789')), s, re.M)
            if m:
                c['trainer_label'] = m.group(1)
                c['id'] = m.group(2)   # (PARRY's first fight is PARRY3)
                c['map_file'] = os.path.basename(p)[:-4]
                maps_src[p] = s
                break
        else:
            sys.exit('no trainer script for %s %s' % (c['class'], c['id']))
    for p, s in maps_src.items():
        code += parse_asm(s)
    labels_code = {}
    for name, cmd, args in code + std:
        pass
    allcode = code + std

    # text labels from every text source
    text_files = glob.glob(os.path.join(pc, 'data/phone/text/*.asm')) + glob.glob(os.path.join(pc, 'data/text/*.asm')) + \
        list(maps_src.keys()) + glob.glob(os.path.join(pc, 'engine/phone/scripts/*.asm')) + \
        [os.path.join(pc, 'data/phone/non_trainer_names.asm')]
    texts_all = text_blocks(text_files)

    # flatten code into instructions, keep label positions
    label_pos, instrs = {}, []
    for name, cmd, args in allcode:
        if name:
            label_pos[name] = len(instrs)
        else:
            instrs.append((cmd, args))

    # what to emit: reachable from entries
    entries = set()
    for c in contacts:
        entries.add(c['callee'])
        entries.add(c['caller'])
        entries.add(c['trainer_label'] + '.Script')
    flags, texts, story_used, strings_used = {}, {}, set(), {}
    out_code, emitted, fixups, todo = [], {}, [], list(entries)
    warn = set()

    def flag_id(n):
        if n not in flags:
            flags[n] = len(flags)
        return flags[n]

    def text_id(label):
        if label not in texts_all:
            warn.add('text ' + label)
            return 0xFFFF
        if label not in texts:
            texts[label] = len(texts)
        return texts[label]

    def contact_of(const):
        return contact_by_const[const]['index'] if const in contact_by_const else 0

    def contact_by_trainer(cls, tid):
        for c in contacts:   # (JOEY1-JOEY5 are all JOEY: the contact of the class with that name)
            if c['class'] == cls and c['id'].rstrip('0123456789') == tid.rstrip('0123456789'):
                return c
        return None

    def num(a):
        if a in NUMS:
            return NUMS[a]
        if a in WEEKDAYS:
            return WEEKDAYS[a]
        if a in phone_index:
            return phone_index[a]
        if re.match(r'^-?\d+$', a):
            return int(a)
        if re.match(r'^\$[0-9a-fA-F]+$', a):
            return int(a[1:], 16)
        warn.add('number ' + a)
        return 0

    def target(lbl):
        if lbl not in label_pos:
            warn.add('label ' + lbl)
            return 0
        if lbl not in emitted:
            todo.append(lbl)
        fixups.append((len(out_code), lbl))
        return 0

    def emit_block(lbl):
        if lbl in emitted:
            return
        emitted[lbl] = len(out_code)
        k = label_pos[lbl]
        while k < len(instrs):
            # other labels that point here share this code
            for name, pos in label_pos.items():
                if pos == k and name not in emitted and name != lbl:
                    emitted[name] = len(out_code)
            cmd, args = instrs[k]
            k += 1
            stop = conv(cmd, args)
            if stop:
                return
        out_code.append(OP['END'])

    last_trainer = {}

    def conv(cmd, args):
        A = out_code.append
        if cmd in ('end', 'endcallback'):
            A(OP['END'])
            return True
        if cmd in ('sjump', 'farsjump', 'jumpstd'):
            A(OP['JUMP'])
            A(target(args[0]))
            return True
        if cmd in ('scall', 'farscall'):
            A(OP['CALL'])
            A(target(args[0]))
            return False
        if cmd in ('iftrue', 'iffalse'):
            A(OP['IFTRUE' if cmd == 'iftrue' else 'IFFALSE'])
            A(target(args[0]))
            return False
        if cmd in ('ifequal', 'ifnotequal', 'ifgreater', 'ifless'):
            A(OP[{'ifequal': 'IFEQ', 'ifnotequal': 'IFNE', 'ifgreater': 'IFGT', 'ifless': 'IFLT'}[cmd]])
            A(num(args[0]))
            A(target(args[1]))
            return False
        if cmd in ('checkflag', 'checkevent'):
            if args[0] in STORY:
                story_used.add(args[0])
                A(OP['CHECKSTORY'])
                A(STORY.index(args[0]))
            else:
                A(OP['CHECKFLAG'])
                A(flag_id(args[0]))
            return False
        if cmd in ('setflag', 'setevent', 'clearflag', 'clearevent'):
            if args[0] in STORY:
                warn.add('story set ' + args[0])
                return False
            A(OP['SETFLAG' if cmd.startswith('set') else 'CLEARFLAG'])
            A(flag_id(args[0]))
            return False
        if cmd in ('writetext', 'farwritetext'):
            A(OP['TEXT'])
            A(text_id(args[0]))
            return False
        if cmd in ('promptbutton', 'waitbutton'):
            A(OP['PROMPT'])
            return False
        if cmd == 'yesorno':
            A(OP['YESNO'])
            return False
        if cmd == 'random':
            A(OP['RANDOM'])
            A(num(args[0]))
            return False
        if cmd == 'readvar':
            A(OP['READVAR'])
            A(VARS.index(args[0]) if args[0] in VARS else 0)
            if args[0] not in VARS:
                warn.add('var ' + args[0])
            return False
        if cmd == 'checktime':
            A(OP['CHECKTIME'])
            A(TIMES[args[0]])
            return False
        if cmd == 'gettrainerclassname' and len(args) == 2:   # (a class alone: the gossip scripts)
            A(OP['STRING'])
            A(int(args[0][-1]))
            A(text_for_string(class_names[classes.index(args[1]) - 1]))
            return False
        if cmd in ('gettrainername', 'gettrainerclassname'):
            c = contact_by_trainer(args[1], args[2])
            A(OP['TRAINERNAME' if cmd == 'gettrainername' else 'CLASSNAME'])
            A(int(args[0][-1]))
            A(c['index'] if c else 0)
            if not c:
                warn.add('trainer ' + ','.join(args[1:]))
            return False
        if cmd == 'getlandmarkname':
            A(OP['LANDMARK'])
            A(int(args[0][-1]))
            A(text_for_string(landmark[args[1]]))
            return False
        if cmd == 'getcurlandmarkname':
            A(OP['CURLANDMARK'])
            A(int(args[0][-1]))
            return False
        if cmd == 'getstring':
            A(OP['STRING'])
            A(int(args[0][-1]))
            A(text_id(args[1]))
            return False
        if cmd == 'getmonname':
            A(OP['MONNAME'])
            A(int(args[0][-1]))
            A(SPECIES_INDEX.setdefault(args[1], len(SPECIES_INDEX)))
            return False
        if cmd == 'special':
            if args[0] in SPECIALS:
                A(OP['SPECIAL'])
                A(SPECIALS.index(args[0]))
            else:
                warn.add('special ' + args[0])
            return False
        if cmd == 'loadvar' and args[0] == 'VAR_CALLERID':
            A(OP['SETCALLER'])
            A(contact_of(args[1]))
            return False
        if cmd == 'checkcellnum':
            A(OP['CHECKCELL'])
            A(contact_of(args[0]))
            return False
        if cmd == 'askforphonenumber':
            A(OP['ASKNUMBER'])
            A(contact_of(args[0]))
            return False
        if cmd in ('readmem', 'loadmem'):
            who = re.match(r'w(\w+?)FightCount', args[0])
            c = next((x for x in contacts if x['id'].rstrip('0123456789').upper() == who.group(1).upper()), None) if who else None
            if not c:
                warn.add('mem ' + args[0])
                return False
            A(OP['READMEM' if cmd == 'readmem' else 'LOADMEM'])
            A(c['index'])
            if cmd == 'loadmem':
                A(num(args[1]))
            return False
        if cmd == 'loadtrainer':
            c = contact_by_trainer(args[0], args[1])
            if not c:
                warn.add('loadtrainer ' + ','.join(args))
                return False
            c.setdefault('fights', [])
            if args[1] not in c['fights']:
                c['fights'].append(args[1])
            A(OP['LOADTRAINER'])
            A(c['index'])
            A(c['fights'].index(args[1]))
            return False
        if cmd == 'startbattle':
            A(OP['STARTBATTLE'])
            return False
        if cmd == 'winlosstext':
            A(OP['WINLOSS'])
            A(text_id(args[0]))
            return False
        if cmd == 'verbosegiveitem':
            A(OP['GIVEITEM'])
            ITEM_USED.add(args[0])
            A(len(ITEM_LIST) if args[0] not in ITEM_LIST else ITEM_LIST.index(args[0]))
            if args[0] not in ITEM_LIST:
                ITEM_LIST.append(args[0])
            return False
        if cmd == 'setval':
            A(OP['SETVAL'])
            A(num(args[0]))
            return False
        if cmd == 'swarm':
            A(OP['SWARM'])
            A(0)
            return False
        if cmd == 'playsound' and 'REGISTER' in args[0]:
            A(OP['REGSOUND'])
            return False
        if cmd in ('checkmoney',):
            A(OP['CHECKMONEY'])
            return False
        if cmd in ('opentext', 'closetext', 'endifjustbattled', 'waitsfx', 'specialphonecall', 'reloadmapafterbattle',
                   'playsound', 'getmoney', 'getnum', 'checkpoke', 'faceplayer'):
            return False
        warn.add('command ' + cmd)
        return False

    STRINGS = {}

    def text_for_string(s):
        key = '__str_' + s
        if key not in texts_all:
            texts_all[key] = [s]
        return text_id(key)

    SPECIES_INDEX = {}
    ITEM_USED, ITEM_LIST = set(), []
    # (contacts' first fights: the id the contact table names)
    for c in contacts:
        c['fights'] = [c['id']]
    while todo:
        emit_block(todo.pop(0))
    for pos, lbl in fixups:
        out_code[pos] = emitted.get(lbl, 0)

    # Quetzal's trainer for each contact: its map's trainerbattle whose trainer has the contact's name
    tq = read(os.path.join(REPO, 'src/data/trainers_qz.h'))
    qz_names = dict(re.findall(r'\[(TRAINER_QZ_\d+)\] =\s*\{[^}]*?\.trainerName = _\("([^"]*)"\)', tq, re.S))
    for c in contacts:
        cname, _, _ = party(c['class'], c['id'])
        c['name'] = cname
        c['class_name'] = class_names[classes.index(c['class']) - 1]
        c['qz'] = None
        base = re.sub(r'\d+F$', '', c['map_file'])   # (Quetzal's LIGHTHOUSE is one map; ROUTE 26 two)
        for d in sorted(set(glob.glob(os.path.join(REPO, 'data/maps', 'Johto_' + c['map_file'] + '*')) +
                            glob.glob(os.path.join(REPO, 'data/maps', 'Johto_' + base + '*')))):
            p = os.path.join(d, 'scripts.inc')
            for t in re.findall(r'trainerbattle \w+, (TRAINER_QZ_\d+)', read(p)) if os.path.exists(p) else []:
                if qz_names.get(t) == cname and c['qz'] is None:
                    c['qz'] = t
                    c['qz_map'] = os.path.basename(d)
        johto = 'Johto_' + c['map_file']
        if c['qz'] is None:
            warn.add('no Quetzal trainer for %s (%s)' % (cname, johto))

    # ---- write
    sp_consts = set(re.findall(r'#define (SPECIES_\w+)', read(os.path.join(REPO, 'include/constants/species.h'))))
    mv_consts = set(re.findall(r'#define (MOVE_\w+)', read(os.path.join(REPO, 'include/constants/moves.h'))))
    it_consts = set(re.findall(r'#define (ITEM_\w+)', read(os.path.join(REPO, 'include/constants/items.h'))))

    def species(n):
        s = 'SPECIES_' + SPECIES_FIX.get(n, n)
        if s not in sp_consts:
            warn.add('species ' + n)
            return 'SPECIES_NONE'
        return s

    def move(n):
        if n in ('NO_MOVE', ''):
            return 'MOVE_NONE'
        s = 'MOVE_' + MOVE_FIX.get(n, n)
        if s not in mv_consts:
            warn.add('move ' + n)
            return 'MOVE_NONE'
        return s

    def item(n):
        s = ITEMS.get(n, 'ITEM_' + n)
        if s == 'ITEM_NONE':
            return s
        if s not in it_consts:
            warn.add('item ' + n)
            return 'ITEM_NONE'
        return s

    out = ['// Generated by tools/quetzal_port/gen_phone.py from pret/pokecrystal. Do not edit.',
           '// JOHTO\'s phone (src/mp/mp_qzphone.c): Crystal\'s contacts, their scripts as bytecode, their words and rematch teams']
    out.append('enum {%s};' % ', '.join('QZOP_' + o for o in OPS))
    out.append('#define QZPHONE_FLAGS %d' % len(flags))
    out.append('enum {%s};' % ', '.join('QZFLAG_%s = %d' % (n, i) for n, i in sorted(flags.items(), key=lambda kv: kv[1])))
    out.append('#define QZPHONE_CONTACT_SLOTS %d' % (max(c['index'] for c in contacts) + 1))
    out.append('enum {%s, QZSTORY_COUNT};' % ', '.join('QZSTORY_' + s for s in STORY))
    for label, idx in sorted(texts.items(), key=lambda kv: kv[1]):
        paras = texts_all[label]
        # (Crystal's `para` mostly turns the page of an 18-letter box, mid-sentence too: the words flow on and the
        # game pages them to the window they print in)
        out.append('static const u8 sQzPhoneText%d[] = %s;   // %s' % (idx, c_string(' '.join(paras)), label))
    out.append('static const u8 *const sQzPhoneTexts[] = {%s};' % ', '.join('sQzPhoneText%d' % i for i in range(len(texts))))
    out.append('static const u16 sQzPhoneCode[] = {')
    for k in range(0, len(out_code), 16):
        out.append('    ' + ', '.join(str(v) for v in out_code[k:k + 16]) + ',')
    out.append('};')
    out.append('static const u16 sQzPhoneItems[] = {%s};' % ', '.join(item(i) for i in ITEM_LIST) if ITEM_LIST else
               'static const u16 sQzPhoneItems[] = {ITEM_NONE};')
    out.append('static const u16 sQzPhoneMonNames[] = {%s};' % (', '.join(species(s) for s in SPECIES_INDEX) or 'SPECIES_NONE'))
    # rematch teams
    for c in contacts:
        for f, tid in enumerate(c['fights']):
            if f == 0:
                continue
            _, kind, mons = party(c['class'], tid)
            out.append('static const struct TrainerMonItemCustomMoves sQzPhoneParty_%s_%d[] = {' % (c['const'], f))
            for lvl, sp, it, mv in mons:
                mv = (mv + ['NO_MOVE'] * 4)[:4]
                custom = kind in ('TRAINERTYPE_MOVES', 'TRAINERTYPE_ITEM_MOVES')
                out.append('    {.iv = %d, .lvl = %d, .species = %s, .heldItem = %s, .moves = {%s}},' % (
                    min(255, 32 * f), lvl, species(sp), item(it) if it else 'ITEM_NONE',
                    ', '.join(move(m) for m in mv) if custom else 'MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE'))
            out.append('};')
    out.append('struct QzPhoneFight { const struct TrainerMonItemCustomMoves *party; u8 count, customMoves; };')
    out.append('struct QzPhoneContact { u8 name[12], className[16]; u16 trainer, callee, caller, script, map; '
               'u8 female, fightCount, rematchFlag; struct QzPhoneFight fights[5]; };')
    out.append('static const struct QzPhoneContact sQzPhoneContacts[QZPHONE_CONTACT_SLOTS] = {')
    for c in contacts:
        fights = []
        for f, tid in enumerate(c['fights']):
            if f == 0:
                fights.append('{NULL, 0, 0}')
            else:
                _, kind, mons = party(c['class'], tid)
                fights.append('{sQzPhoneParty_%s_%d, %d, %d}' % (c['const'], f, len(mons),
                                                                  kind in ('TRAINERTYPE_MOVES', 'TRAINERTYPE_ITEM_MOVES')))
        female = 'Female' in ''.join(code_text(code, c['callee']))
        mapid = json.load(open(os.path.join(REPO, 'data/maps', c['qz_map'], 'map.json')))['id'] if c.get('qz_map') else 'MAP_UNDEFINED'
        who = c['id'].rstrip('0123456789')
        rematch = flags.get('ENGINE_%s_READY_FOR_REMATCH' % who, 0xFF)
        out.append('    [%d] = {%s, %s, %s, %d, %d, %d, %s, %d, %d, %d, {%s}},   // %s' % (
            c['index'], c_string(c['name']), c_string(c['class_name']), c['qz'] or 'TRAINER_NONE',
            emitted[c['callee']], emitted[c['caller']], emitted[c['trainer_label'] + '.Script'], mapid, female,
            len(c['fights']), rematch, ', '.join(fights), c['const']))
    out.append('};')
    open(os.path.join(REPO, 'src/data/qz_phone.h'), 'w', encoding='utf-8').write('\n'.join(out) + '\n')
    print('%d contacts, %d words of code, %d texts, %d flags; stories %s' % (
        len(contacts), len(out_code), len(texts), len(flags), sorted(story_used)))
    for w in sorted(warn):
        print('  warning:', w)


def code_text(code, label):
    """the commands of a label's block, as text (to tell a male contact from a female one)"""
    out, on = [], False
    for name, cmd, args in code:
        if name:
            on = name == label or (on and name.startswith(label + '.'))
            continue
        if on:
            out.append(' '.join([cmd] + (args or [])))
    return out


if __name__ == '__main__':
    main()
