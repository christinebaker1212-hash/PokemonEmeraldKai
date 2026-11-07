#!/usr/bin/env python3
"""Checks the scenario shadow_gauge's log: every `[test] shadow N:` line's DP and level, recomputed in Python from the
generated table (src/data/orre_shadow.h: Colosseum's rule, tools/orre/shadow_data.py), in the scenario's order.

    python3 -I tools/orre/check_shadow.py RESULT_DIR
"""
import re
import sys

H = open("src/data/orre_shadow.h").read()
SRC = [int(v) for v in re.search(r"sOrreDpSource\[\d+\] = \{([^}]*)\}", H).group(1).split(",")]
RATE = [tuple(int(x) for x in p) for p in re.findall(r"\{(\d+), (\d+)\}", re.search(r"sOrreDpRate\[\d+\]\[2\] = \{(.*)\};", H).group(1))]
NAT = [[int(x) for x in m.split(",")] for m in re.findall(r"    \{\{([^}]*)\}, \{", H)]
GAUGE = {int(m[0]): int(m[1]) for m in re.findall(r"    \{(\d+), \d+, \d+, (\d+), \d+, \{", H)}


def drop(dp, nature, source, strength=0):
    if source == 5:
        return 0
    d = float(SRC[source]) * (strength if source == 4 else 1)
    k, w = RATE[NAT[nature][source]]
    d = d * k / w
    now = max(0.0, dp / 100.0 + d)
    return int(100.0 * now)


def level(dp, gauge):
    if dp <= 0:
        return 6
    p = 100.0 * (dp / 100.0) / gauge
    return 0 if p >= 100 else 1 if p >= 80 else 2 if p >= 60 else 3 if p >= 40 else 4 if p >= 20 else 5


# replay the scenario's commands (its a.txt) against the log's lines, in order
cmds = [l.split() for l in open(sys.argv[1] + "/a.txt") if l.startswith("mp shadow ") or l.startswith("press ")]
states = [tuple(int(v) for v in m.groups()) for m in
          re.finditer(r"\[test\] shadow (\d+): id (\d+) dp (-?\d+) gauge (\d+) level (\d+) hyper \d+ call \d+ nature (\d+)",
                      open(sys.argv[1] + "/a.log").read())]
counts = [int(m.group(1)) for m in re.finditer(r"\[test\] shadow steps \d+: (-?\d+)", open(sys.argv[1] + "/a.log").read())]
exp, nature, steps, walked, bad, k, c = {}, {}, 0, False, 0, 0, 0


def take(slot):
    global k, bad
    if k >= len(states):
        print("missing a state line for slot", slot)
        bad += 1
        return None
    st = states[k]
    k += 1
    nature[st[0]] = st[5]
    return st


for cmd in cmds:
    if cmd[0] == "press":
        walked = True
        continue
    op = cmd[2]
    a = [int(v) for v in cmd[3:]]
    if op == "table":
        continue
    if op == "stepcount":
        got = counts[c]
        c += 1
        if walked and got == 0 and steps == 256:
            exp[0] = drop(exp[0], nature[0], 1)   # (the 257th step, a real one: the walking drop)
        steps = got
        walked = False
        continue
    slot = a[0] if a else 0
    if op == "make":
        exp[slot] = GAUGE[a[1]] * 100
    elif op == "drop" and a[1] <= 5 and slot in exp:
        st_nat = nature.get(slot)
        exp[slot] = drop(exp[slot], st_nat, a[1], a[2] if len(a) > 2 else 0)
    elif op == "steps":
        steps = min(256, steps + a[0])
        if a[0] > 256:
            exp[0] = drop(exp[0], nature[0], 1)
        continue   # (a count, not a slot: no state line)
    elif op == "purify":
        exp.pop(slot, None)
    st = take(0 if op == "reload" else slot)
    if "-v" in sys.argv: print(cmd, st, exp)
    if st is None:
        continue
    s_, sid, dp, gauge, lv, nat = st
    want = exp.get(s_, -1)
    if dp != want or (sid and lv != level(dp, gauge)):
        print("after", " ".join(cmd), ": got dp %d level %d, want dp %d level %d" % (dp, lv, want, level(want, gauge) if want >= 0 else 7))
        bad += 1
print("%d gauge lines, %d wrong" % (k, bad))
sys.exit(1 if bad or k == 0 else 0)
