#!/usr/bin/env python3
"""ORRE's story scenes, one by one (docs/ORRE_HANDOFF.md "The story runs"): for each story flag in Colosseum's order
(ORDER.txt: the game's `mp orreorder`) that a room's function sets (tools/orre/story_route.py), a test step: the story
jumped to just before it (`mp orrestory FLAG`), the player in the room (`mp warp`), the function run as a scene
(`mp orrescript 100 FUNC`, a person's talk script with (1, person, 0, 0)), A pressed until it ends (YES to every
question; a strong party wins the battles), then the flag read. Writes tools/test/scenarios/orre_scenes_NN, 25 steps
each; a step that doesn't set its flag shows as "[test] orreget 0x... 0" in its log (tools/orre/check_scenes.py).

    python3 -I tools/orre/gen_story_tests.py ORDER.txt [UNPACKED]
"""
import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import story_route  # noqa: E402

PER = 25
PASSES = 3   # (orre_story_chain: tools/orre/gen_story_tests.py chain)


def listing(unpacked, tag):
    import subprocess
    return subprocess.run([sys.executable, '-I', os.path.join(HERE, 'gsvm.py'),
                           os.path.join(unpacked, tag, tag + '.scd')], capture_output=True, text=True).stdout.splitlines()


def f32(v):
    import struct
    return struct.unpack('>f', struct.pack('>I', v))[0] if v > 0xFFFF else float(v)


def trigger_box(lines, fn):
    """The box (x0, z0, x1, z1, Colosseum's units) of the std058 test nearest before `call func FN` in a main loop, or
    in FN itself before its first message (a main loop that is its own scene)"""
    for i, l in enumerate(lines):
        if re.search(r'call func %d \(' % fn, l) or re.match(r'-- function %d \(' % fn, l):
            rng = range(i - 1, max(0, i - 60), -1) if 'call' in l else range(i + 1, min(len(lines), i + 400))
            for j in rng:
                m = re.search(r'std058_characterCheckWithinBounds\(0, 100, (\d+), (\d+), (\d+), (\d+)\)', lines[j])
                if m:
                    return tuple(f32(int(v)) for v in m.groups())
                if 'call func' in lines[j] and j != i or lines[j].startswith('-- function'):
                    break
    return None


def chain(out, steps, maps):
    """orre_story_chain: every step in the story's order from the landing, the story never jumped, so each one starts
    from what the ones before it left (as a player's game does); a step the story has already done is passed over
    (`skipif orreflag:N`). The first "TIMEOUT waiting for log orre: flag N = 1" is where the story stops."""
    name = 'orre_story_chain'
    start = os.environ.get('ORRE_CHAIN_FROM')   # (a later start: Colosseum's story jump to it, then the chain on)
    if start:
        name += '_from'
        steps = steps[[s[0] for s in steps].index(int(start)):]
    d = os.path.join(out, name)
    if os.path.isdir(d):
        shutil.rmtree(d)
    os.makedirs(d)
    lines = ['pressuntil A field 5000', 'wait 30', 'mp teach 0 94', 'mp teach 1 94']
    if start:
        lines += ['mp orrestory %d' % int(start)]
    # (Colosseum's flag order isn't the order a player meets them: PHENAC STADIUM's entry waits for PYRITE TOWN's.
    # Several passes: a step done is passed over at once; one not possible yet is tried again in the next)
    for flag, tag, fn, args, main, at in steps * PASSES:
        mi, x, y = maps[tag]
        if at:
            x, y = at
        # (already in the room: walked to, as a player would; warping in again would set the room up afresh, and a
        # room's set-up can take back what a scene left for the player: PHENAC's sack)
        block = ['skipif inmap:45,%d 2' % mi, 'mp warp 45,%d,%d,%d' % (mi, x, y), 'wait 150']
        if main:   # (the box a main loop's scene waits for: walked into; a talk's scene needs no place)
            block += ['pressuntil A field 3000', 'walkto %d %d 900' % (x, y)]
        if not main:
            block.append('mp orrescript 100 %d%s' % (fn, args))
        else:   # (a main loop's scene can wait on something looked at there: PHENAC's sack; A to each side)
            block += ['pressuntil A field 3000']
            for way in ('up', 'left', 'right', 'down'):   # (turned only: a tap the way it faces would step)
                block += ['mp face %s' % way, 'wait 6', 'press A 2', 'wait 60']
        # (a scene can run on well past its flag: its battles, its next lines; the next step waits for the field)
        # (a scene still running when the wait ends runs on: the field's wait lets it finish)
        block += ['pressuntillog A 2500 orre: flag %d = 1' % flag, 'pressuntil A field 9000', 'wait 30']
        lines += ['skipif orreflag:%d %d' % (flag, len(block))] + block
        if flag == 993:
            # (PHENAC: the kidnappers beaten, the sack is examined as a player does it: RUI out, the gate's pen open.
            # Kai's own flag 4094, RUI follows: no Colosseum flag tells it)
            sack = ['skipif inmap:45,2 0', 'walkto 48 106 600', 'mp face up', 'wait 10',
                    'pressuntillog A 9000 orre npcs: RUI follows', 'pressuntil A field 9000', 'wait 30']
            sack[0] = 'skipif orreflag:4094 %d' % (len(sack) - 1)
            lines += sack
    lines += ['mp orreget %d' % steps[-1][0], 'quit']
    open(os.path.join(d, 'a.txt'), 'w').write('\n'.join(lines) + '\n')
    open(os.path.join(d, 'conf'), 'w').write(
        '# ORRE\'s whole story chained from the landing (generated by tools/orre/gen_story_tests.py): every step in order,\n'
        '# each from what the ones before left; the first TIMEOUT is where the story stops (docs/ORRE_HANDOFF.md "The\n'
        '# story runs"). Long: run it on its own.\nMODE=1\nTMO=%d\nWARP=45,0,33,40\n'
        'ENV_A="MOEMON_DEBUG_PARTY=150:100,150:100 MOEMON_ORRE_SCRIPTS=1 MOEMON_ORRE_NO_OPENING=1 MOEMON_ORRE_FLAG_LOG=1"\nEXPECT_A=()\n'
        'UNEXPECT_A=("[crash]")\n' % (120 * PASSES * len(steps) + 60))


def walk_chain(out, steps, maps, ent):
    """orre_story_walk: the story as a player plays it, from the landing: no warps and no scripts started by the test.
    Each step's room is walked to (`goto`: its doors, the world map), and its scene started as a player starts it: a
    room's main loop's box walked into, a person talked to, a touch spot stepped into, a check spot looked at
    (`approach`). The first TIMEOUT is where a player would be stuck (or where a step needs what a test can't do yet:
    `[walk] cannot` lines say which)"""
    d = os.path.join(out, 'orre_story_walk')
    if os.path.isdir(d):
        shutil.rmtree(d)
    os.makedirs(d)
    lines = ['pressuntil A field 5000', 'wait 30', 'mp teach 0 94', 'mp teach 1 94']
    for flag, tag, fn, args, main, at, how in steps * PASSES:
        mi, x, y = maps[tag]
        if at:
            x, y = at
        block = ['goto 45,%d %d %d 12000' % (mi, x, y)]
        m = re.match(r'(person) (\d+) talk|(touch|check) spot (\d+)', how or '')
        if main or not m:
            if not main:   # (reached some other way: the room's own scripts, once the player is in)
                block += ['echo [walk] %s:%d starts by itself (no talk / touch / check)' % (tag, fn)]
            block += ['pressuntil A field 3000']
            for way in ('up', 'left', 'right', 'down'):
                block += ['mp face %s' % way, 'wait 6', 'press A 2', 'wait 60']
        else:
            if True:
                kind, n = (m.group(1), m.group(2)) if m.group(1) else (m.group(3), m.group(4))
                block += ['approach %s %s 3000' % (kind, n)]
        block += ['pressuntillog A 2500 orre: flag %d = 1' % flag, 'pressuntil A field 9000', 'wait 30']
        # (no way to the room yet: nothing is pressed where the player stands instead: the landing's sailor would
        # sail them back)
        # (and a scene the way there set off has the flag already: nothing left to wait for)
        block = block[:1] + ['skipif notmap:45,%d %d' % (mi, len(block)), 'skipif orreflag:%d %d' % (flag, len(block) - 1)] + block[1:]
        lines += ['skipif orreflag:%d %d' % (flag, len(block))] + block
        if flag == 993:
            # (PHENAC: the sack examined as a player does it, as the chain above)
            sack = ['', 'walkto 48 106 600', 'mp face up', 'wait 10',
                    'pressuntillog A 9000 orre npcs: RUI follows', 'pressuntil A field 9000', 'wait 30']
            sack[0] = 'skipif orreflag:4094 %d' % (len(sack) - 1)
            lines += sack
    lines += ['mp orreget %d' % steps[-1][0], 'quit']
    open(os.path.join(d, 'a.txt'), 'w').write('\n'.join(lines) + '\n')
    open(os.path.join(d, 'conf'), 'w').write(
        '# ORRE\'s story walked as a player plays it (generated by tools/orre/gen_story_tests.py walk_chain): no warps,\n'
        '# no scripts started by the test; the first TIMEOUT is where a player would be stuck. Long: run it on its own.\n'
        'MODE=1\nTMO=%d\nWARP=45,0,33,40\n'
        'ENV_A="MOEMON_DEBUG_PARTY=150:100,150:100 MOEMON_ORRE_SCRIPTS=1 MOEMON_ORRE_NO_OPENING=1 MOEMON_ORRE_FLAG_LOG=1 MOEMON_TEST_STAY=1"\n'
        'EXPECT_A=()\nUNEXPECT_A=("[crash]")\n' % (300 * PASSES * len(steps) + 60))


def main():
    order = [l.split() for l in open(sys.argv[1]) if 'orre order' in l]
    unpacked = sys.argv[2] if len(sys.argv) > 2 else os.path.expanduser('~/.cache/moemon/orre/unpacked')
    maps = {}
    for i, m in enumerate(re.finditer(r'\{MAP_ORRE_\w+, "(\w+)", \d+, \d+, (\d+), (\d+), (\d+)\}',
                                      open(os.path.join(ROOT, 'src', 'orre', 'orre_maps.h')).read())):
        maps[m.group(1)] = (i, int(m.group(2)), int(m.group(3)))
    b, ent = story_route.banks(unpacked), story_route.entries(unpacked)
    setters = {}
    for tag, funcs in b.items():
        for fn, (calls, sets) in funcs.items():
            for fl in sets:
                setters.setdefault(fl, (tag, fn))
    lists, grids = {}, {}
    import maps as orre_maps
    mconf = {m['file']: m for m in orre_maps.load(unpacked)}
    steps = []
    for l in order:
        flag = int(l[5])
        if flag not in setters:
            continue
        tag, fn = setters[flag]
        if tag not in maps:
            continue
        # (how it's reached: a function the room runs by itself (its main loop or set-ups, or one they call) isn't
        # started again; a person's talk script gets (1, person, 0, 0))
        callers = {}
        for t2, funcs in b.items():
            for f2, (calls, _) in funcs.items():
                for c in calls:
                    callers.setdefault((t2, c), set()).add((t2, f2))
        args, main, seen, todo = '', False, set(), [(tag, fn)]
        while todo:
            k = todo.pop()
            if k in seen:
                continue
            seen.add(k)
            for how in ent.get(k, []):
                m = re.match(r'person (\d+) talk', how)
                if m and k == (tag, fn):
                    args = ' 1 %s 0 0' % m.group(1)
                main = main or how in ('main loop', 'set-up', 'set-up 2')
            todo += list(callers.get(k, ()))
        if (tag, fn) in ent and any(h.startswith(('person', 'check', 'touch', 'on leaving')) for h in ent[(tag, fn)]):
            main = False
        at = None
        if main:   # (the player where the main loop's test for the scene looks: its box's middle)
            if tag not in lists:
                lists[tag] = listing(unpacked, tag)
            box = None
            for k in seen:
                if k[0] == tag:
                    box = box or trigger_box(lists[tag], k[1])
            if box:
                if tag not in grids:
                    grids[tag] = orre_maps.build_grid(unpacked, mconf[tag])
                g = grids[tag]
                cx, cz = (box[0] + box[2]) / 2, (box[1] + box[3]) / 2
                tx, ty = int((cx - g['x0']) // g['tile']), int((cz - g['z0']) // g['tile'])
                walk = [(y, x) for y in range(g['h']) for x in range(g['w']) if g['walk'][y, x]]
                lo, hi = (min(box[0], box[2]), min(box[1], box[3])), (max(box[0], box[2]), max(box[1], box[3]))
                inside = [(y, x) for y, x in walk if lo[0] <= g['x0'] + (x + .5) * g['tile'] <= hi[0]
                          and lo[1] <= g['z0'] + (y + .5) * g['tile'] <= hi[1]]
                y, x = min(inside or walk, key=lambda p: (abs(p[0] - ty) + abs(p[1] - tx), p))
                at = (x, y)
        how = None
        for k in sorted(seen, key=lambda k: k != (tag, fn)):
            for h in ent.get(k, []):
                if h.startswith(('person', 'touch', 'check')) and (not h.startswith('person') or h.endswith('talk')):
                    how = how or h
        steps.append((flag, tag, fn, args, main, at, how))
    out = os.path.join(ROOT, 'tools', 'test', 'scenarios')
    walk_chain(out, steps, maps, ent)
    steps = [st[:6] for st in steps]
    chain(out, steps, maps)
    for d in os.listdir(out):
        if d.startswith('orre_scenes_'):
            shutil.rmtree(os.path.join(out, d))
    for k in range(0, len(steps), PER):
        d = os.path.join(out, 'orre_scenes_%02d' % (k // PER))
        os.makedirs(d)
        lines = ['pressuntil A field 5000', 'wait 30', 'mp teach 0 94', 'mp teach 1 94']
        for flag, tag, fn, args, main, at in steps[k:k + PER]:
            mi, x, y = maps[tag]
            if at:
                x, y = at
            lines += ['mp orrestory %d' % flag, 'mp warp 45,%d,%d,%d' % (mi, x, y), 'wait 150']
            if not main:   # (a main loop runs by itself: the room's, once the player is in)
                lines.append('mp orrescript 100 %d%s' % (fn, args))
            lines += ['pressuntillog A 3000 orre: flag %d = 1' % flag, 'pressuntil A field 3000', 'wait 30',
                      'mp orreget %d' % flag]
        lines.append('quit')
        open(os.path.join(d, 'a.txt'), 'w').write('\n'.join(lines) + '\n')
        open(os.path.join(d, 'conf'), 'w').write(
            '# ORRE story scenes %d-%d (generated by tools/orre/gen_story_tests.py: each run on its own; '
            'tools/orre/check_scenes.py)\nMODE=1\nTMO=%d\nWARP=45,0,33,40\n'
            'ENV_A="MOEMON_DEBUG_PARTY=150:100,150:100 MOEMON_ORRE_SCRIPTS=1 MOEMON_ORRE_NO_OPENING=1 MOEMON_ORRE_FLAG_LOG=1"\nEXPECT_A=()\n'
            'UNEXPECT_A=("[crash]")\n' % (k, k + PER - 1, 100 * len(steps[k:k + PER]) + 60))
    print('%d steps in %d scenarios' % (len(steps), (len(steps) + PER - 1) // PER))


if __name__ == '__main__':
    main()
