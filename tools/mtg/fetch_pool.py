#!/usr/bin/env python3
"""Writes tools/mtg/pool.json: the Magic cards the game knows, from Scryfall (https://api.scryfall.com).

The pool is Premodern's (docs/MTG_DECISIONS.md): every card legal in it, and its banned cards kept apart
("banned": true) so the game's own list can follow Premodern's. One entry per card name, sorted by name:
  {"name", "type", "cmc", "colors", "rarity", "banned"}   (rarity: common / uncommon / rare, of its first printing)
Scryfall asks for a named User-Agent, an Accept header and a short pause between requests.

usage: fetch_pool.py [OUT]   (default tools/mtg/pool.json)"""
import json, os, sys, time, urllib.parse, urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
HEADERS = {"User-Agent": "MoemonKaiCardTool/1.0", "Accept": "application/json"}


def search(q):
    url = "https://api.scryfall.com/cards/search?" + urllib.parse.urlencode({"q": q, "unique": "cards", "order": "name"})
    while url:
        with urllib.request.urlopen(urllib.request.Request(url, headers=HEADERS)) as r:
            d = json.load(r)
        yield from d["data"]
        url = d.get("next_page")
        time.sleep(0.12)


def entry(c, banned):
    return {"name": c["name"], "type": c.get("type_line", ""), "cmc": c.get("cmc", 0),
            "colors": "".join(c.get("colors", c.get("card_faces", [{}])[0].get("colors", []))),
            "rarity": first.get(c["name"]) or ("rare" if c.get("rarity") == "mythic" else c.get("rarity", "common")),
            "banned": banned}


first = {}   # name -> the rarity of its earliest printing in the pool's years (a land "special" / "bonus" -> common)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "pool.json")
    for c in search("format:premodern date<=2003-07-01 unique:prints order:released direction:asc"):
        r = c.get("rarity", "common")
        first.setdefault(c["name"], r if r in ("common", "uncommon", "rare") else "common")
    cards = {c["name"]: entry(c, False) for c in search("format:premodern")}
    for c in search("banned:premodern"):
        cards[c["name"]] = entry(c, True)
    pool = [cards[n] for n in sorted(cards)]
    with open(out, "w") as f:
        json.dump(pool, f, indent=0, ensure_ascii=False)
        f.write("\n")
    print("%d cards (%d banned) -> %s" % (len(pool), sum(c["banned"] for c in pool), out))


if __name__ == "__main__":
    main()
