#!/usr/bin/env python3
"""Fetches every Final Fantasy XIV Triple Triad card from xivapi (https://v2.xivapi.com/): names, the four
numbers, stars, type, description, sale value, and both pictures (the 40x40 icon and the 104x128 card face).

    python3 tools/tt/fetch_cards.py            # writes tools/tt/cards.json, pictures into tools/tt/metadata/
    python3 tools/tt/fetch_cards.py --no-art   # only the data

The pictures are gitignored; tools/tt/build_art.py packs them into graphics/tt/cards.bin."""
import json, os, sys, time, urllib.request

API = "https://v2.xivapi.com/api"
HERE = os.path.dirname(os.path.abspath(__file__))
META = os.path.join(HERE, "metadata")

def get(url, binary=False):
    for attempt in range(5):
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "moemon-tt-fetch/1.0"})
            with urllib.request.urlopen(req, timeout=60) as r:
                data = r.read()
            return data if binary else json.loads(data)
        except Exception as e:  # network hiccups: try again a little later
            if attempt == 4:
                raise
            print("retry", url, e, file=sys.stderr)
            time.sleep(2 << attempt)

def sheet(name, fields):
    rows, after = [], None
    while True:
        url = f"{API}/sheet/{name}?limit=500&fields={fields}"
        if after is not None:
            url += f"&after={after}"
        page = get(url)["rows"]
        if not page:
            return rows
        rows += page
        after = page[-1]["row_id"]
        if len(page) < 500:
            return rows

def main():
    names = {r["row_id"]: r["fields"] for r in sheet("TripleTriadCard", "Name,Description")}
    res = {r["row_id"]: r["fields"] for r in sheet(
        "TripleTriadCardResident",
        "Top,Bottom,Left,Right,TripleTriadCardRarity.Stars,TripleTriadCardType.Name,SaleValue,SortKey,Order,"
        "UIPriority,AcquisitionType.Text.Text")}
    cards = []
    for rid in sorted(names):
        n, r = names[rid], res.get(rid)
        if not r or not n["Name"] or n["Name"] == "0" or not r["Top"]:
            continue
        cards.append({
            "id": rid,
            "name": n["Name"],
            "description": n["Description"],
            "top": r["Top"], "right": r["Right"], "bottom": r["Bottom"], "left": r["Left"],
            "stars": r["TripleTriadCardRarity"]["fields"]["Stars"],
            "type": r["TripleTriadCardType"]["fields"].get("Name", ""),
            "sale": r["SaleValue"],
            "order": r["Order"],
            "sort": r["SortKey"],
            "ui": r["UIPriority"],
            "obtain": ((r.get("AcquisitionType") or {}).get("fields", {}).get("Text") or {}).get("fields", {}).get("Text", ""),
        })
    with open(os.path.join(HERE, "cards.json"), "w") as f:
        json.dump(cards, f, indent=1, ensure_ascii=False)
    print(len(cards), "cards")
    if "--no-art" in sys.argv:
        return
    os.makedirs(META, exist_ok=True)
    for c in cards:
        for kind, base in (("small", 88000), ("big", 87000)):
            path = os.path.join(META, f"{kind}_{c['id']}.png")
            if os.path.exists(path):
                continue
            icon = base + c["id"]
            url = f"{API}/asset?path=ui/icon/{icon // 1000 * 1000:06d}/{icon:06d}.tex&format=png"
            data = get(url, binary=True)
            with open(path, "wb") as f:
                f.write(data)
    # the card back, and the card frame with the star and type icons
    for name, path in (("back", "ui/icon/087000/087000.tex"), ("frame", "ui/uld/CardTripleTriad_hr1.tex")):
        out = os.path.join(META, name + ".png")
        if not os.path.exists(out):
            with open(out, "wb") as f:
                f.write(get(f"{API}/asset?path={path}&format=png", binary=True))
    print("pictures in", META)

if __name__ == "__main__":
    main()
