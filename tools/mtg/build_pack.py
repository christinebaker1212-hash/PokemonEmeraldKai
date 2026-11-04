#!/usr/bin/env python3
"""Builds graphics/mtg/forge.pak: Forge's card script for every card of the pool (tools/mtg/pool.json), the text the
C engine (src/mtg/mtg_script.c) reads. Forge at the pinned commit: tools/mtg/fetch_forge.sh.

Pack format (little-endian): "MTGF", u32 version (1), u32 count, then count entries sorted by name (bytes)
{u32 name, u32 script, u32 size} (offsets into the text block, each string 0-terminated), then the text block.

usage: build_pack.py FORGE_DIR [OUT]   (default graphics/mtg/forge.pak)"""
import json, os, re, struct, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))


def main():
    forge = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, "graphics", "mtg", "forge.pak")
    want = open(os.path.join(HERE, "forge_commit.txt")).read().strip()
    have = subprocess.run(["git", "-C", forge, "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
    if have != want:
        sys.exit("Forge is at %s, the pin is %s (tools/mtg/fetch_forge.sh)" % (have, want))
    scripts = {}
    for dp, _, fs in os.walk(os.path.join(forge, "forge-gui", "res", "cardsfolder")):
        for f in fs:
            if f.endswith(".txt"):
                t = open(os.path.join(dp, f), encoding="utf-8").read()
                m = re.search(r"^Name:(.*)$", t, re.M)
                if m:
                    scripts.setdefault(m.group(1).strip(), t)
    pool = json.load(open(os.path.join(HERE, "pool.json")))
    cards, missing = [], []
    for c in pool:
        name = c["name"]
        # a split card's file is named by its faces ("Fire // Ice" is one file with both)
        t = scripts.get(name) or next((scripts[n] for n in name.split(" // ") if n in scripts), None)
        if t is None:
            missing.append(name)
            continue
        t = "\n".join(l.rstrip() for l in t.replace("\r", "").splitlines() if l.strip()) + "\n"
        cards.append((name.encode("utf-8"), t.encode("utf-8")))
    if missing:
        sys.exit("no Forge script for %d cards: %s" % (len(missing), ", ".join(missing[:20])))
    cards.sort()
    text, index = bytearray(), []
    for name, t in cards:
        n = len(text); text += name + b"\0"
        s = len(text); text += t + b"\0"
        index.append((n, s, len(t)))
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "wb") as f:
        f.write(b"MTGF" + struct.pack("<II", 1, len(cards)))
        for e in index:
            f.write(struct.pack("<III", *e))
        f.write(text)
    print("%d cards, %d bytes -> %s" % (len(cards), os.path.getsize(out), out))


if __name__ == "__main__":
    main()
