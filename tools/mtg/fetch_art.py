#!/usr/bin/env python3
"""Fetches the printed Magic cards from Scryfall (https://scryfall.com) for graphics/mtg/printed.bin
(tools/mtg/build_printed.py): for each card the engine plays (tools/mtg/scope_dump.c says which), its earliest
printing in the old frames (1993 / 1997) from the Premodern years (4th Edition, 1995, to Scourge, 2003), the "normal" scan (488x680), and its art crop for
measuring where the illustration sits. The card back too.

Scryfall asks for a named User-Agent, an Accept header and a short pause between API requests; the scans come from
its image server. Everything goes into a cache folder outside the repository; tools/mtg/art.json lists what was
picked (name, set, collector number, artist, frame), committed.

usage: fetch_art.py [--all] [CACHE_DIR]   (default ~/.cache/moemon/mtg_art; --all: every card of the pool)"""
import json, os, subprocess, sys, time, urllib.parse, urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
HEADERS = {"User-Agent": "MoemonKaiCardTool/1.0", "Accept": "application/json"}
BACK = "https://backs.scryfall.io/normal/0/a/0aeebaf5-8c7d-4636-9e82-8c27447861f7.jpg"


def get(url, binary=False):
    for attempt in range(5):
        try:
            with urllib.request.urlopen(urllib.request.Request(url, headers=HEADERS), timeout=60) as r:
                data = r.read()
            return data if binary else json.loads(data)
        except Exception as e:   # (a hiccup: wait and again)
            if attempt == 4:
                raise
            time.sleep(2 ** attempt)


def playable():
    exe = "/tmp/mtg-tests/scope_dump"
    os.makedirs(os.path.dirname(exe), exist_ok=True)
    src = ["src/mtg/%s.c" % f for f in ("mtg_script", "mtg_static", "mtg_valid", "mtg_mana", "mtg_game", "mtg_effects", "mtg_trigger",
                                                   "mtg_ai")]
    subprocess.run(["gcc", "-std=gnu99", "-O1", "-iquote", "include", "-iquote", "src/mtg", "tools/mtg/scope_dump.c"]
                   + src + ["-o", exe], cwd=ROOT, check=True)
    out = subprocess.run([exe, os.path.join(ROOT, "graphics/mtg/forge.pak")], capture_output=True, text=True,
                         check=True).stdout
    return {l.split("\t")[0] for l in out.splitlines() if l.endswith("\t")}


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    cache = args[0] if args else os.path.join(os.path.expanduser("~"), ".cache", "moemon", "mtg_art")
    os.makedirs(cache, exist_ok=True)
    pool = {c["name"] for c in json.load(open(os.path.join(HERE, "pool.json")))}
    want = pool if "--all" in sys.argv else playable() & pool
    # every old-frame printing of the pool, earliest first: the first one seen of a name is its pick
    picks = {}
    url = "https://api.scryfall.com/cards/search?" + urllib.parse.urlencode(
        {"q": "format:premodern unique:prints (frame:1993 or frame:1997) date>=1995-04-01 date<=2003-05-26", "order": "released", "dir": "asc"})
    while url:
        d = get(url)
        for c in d["data"]:
            if c["name"] in want and c["name"] not in picks and "image_uris" in c and not c.get("digital"):
                picks[c["name"]] = c
        url = d.get("next_page")
        time.sleep(0.12)
    missing = sorted(want - set(picks))
    if missing:
        print("no old-frame printing for %d (left to the game's own frame): %s" % (len(missing), ", ".join(missing[:10])))
    listing = {}
    for i, name in enumerate(sorted(picks)):
        c = picks[name]
        base = "%s_%s" % (c["set"], c["collector_number"].replace("/", "_"))
        for kind in ("normal", "art_crop"):
            path = os.path.join(cache, "%s.%s.jpg" % (base, kind))
            if not os.path.exists(path):
                data = get(c["image_uris"][kind], binary=True)
                with open(path + ".tmp", "wb") as f:
                    f.write(data)
                os.replace(path + ".tmp", path)
                time.sleep(0.05)
        listing[name] = {"set": c["set"], "number": c["collector_number"], "artist": c.get("artist", ""),
                         "frame": c["frame"], "file": base}
        if i % 100 == 0:
            print("%d/%d" % (i, len(picks)))
    back = os.path.join(cache, "back.jpg")
    if not os.path.exists(back):
        with open(back, "wb") as f:
            f.write(get(BACK, binary=True))
    with open(os.path.join(HERE, "art.json"), "w") as f:
        json.dump(listing, f, indent=0, sort_keys=True, ensure_ascii=False)
        f.write("\n")
    print("%d cards -> %s (tools/mtg/art.json)" % (len(listing), cache))


if __name__ == "__main__":
    main()
