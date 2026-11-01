"""SINNOH's gifts: the people who hand the player an item in Platinum (its scripts give it with Common_GiveItemQuantity),
found from pret/pokeplatinum's scripts; gen_words.py gives each of them their words (tools/sinnoh/words.py) and the
item, once (a flag of its own: GIFT_FLAGS + its place in the sorted list). Nothing of Platinum's text is used: the
words are this game's, then one of a few lines of this game's own as the item changes hands.

Only people shown from the start with words of their own (not story people, gym leaders or scripts of this game's own),
and items the game has (key items for systems it hasn't, as the JOURNAL, the COUPONS or the POFFIN CASE, are left out).
"""
import functools
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import items  # noqa: E402

GIFT_FLAGS = 0x3DA0   # (free: the legendaries' added objects' 0x3D80-0x3D8F; the hidden items' start at 0x3E00)
GIFT_FLAGS_END = 0x3E00
SKIP = {'ITEM_JOURNAL', 'ITEM_PARCEL', 'ITEM_TOWN_MAP', 'ITEM_COIN_CASE', 'ITEM_HM06'}   # (the TOWN MAP and COIN CASE
#   are the game's own already; HMs come from the HOENN story: no HM restrictions)


# What each says as their item changes hands (the person's own words come first: tools/sinnoh/words.py)
LINES = {
    'Cafe Person2': "Here, have a few bottles on the house. Fresh from the farm!",
    'CanalaveCitySoutheastHouse Person2': "You look like you train hard too. Take this TM. It's served me well.",
    'CelesticTownNorthwestHouse Person3': "The elder gave me these glasses, but they suit a clever MOéMON better. Here.",
    'CelesticTownPokecenter1f Person4': "Visitors to the shrine should have one of these. Take it!",
    'EternaCityCondominiums2f Person1': "Grandpa says the old stories hold old moves too. He wants you to have this TM.",
    'FightArea Person7': "Here, take my spare rod. A good one! You'll hook the big ones with it.",
    'FightAreaMart Person2': "You remind me of myself at your age, darling. Take this lens. It helps one aim true.",
    'FloaromaTown Person4': "Oh, and someone left this flower with me. It's meant for a very special MOéMON. Take it!",
    'FloaromaTownMiddleHouse Person2': "You can have this! It was in the flower basket. I don't know what it does.",
    'FuegoIronworksBuilding Person5': "Found this in the furnace ash. Pretty, isn't it? It's yours!",
    'GameCorner Person1': "Sorry for the trouble. Here's a little something while you wait.",
    'GrandLakeRoute213NorthwestHouse Person1': "Since I'm not working, you can have my act. It's on this TM!",
    'GreatMarsh6 Person3': "I dug this out of the mud. Odd red stone. Maybe you know what it's for?",
    'HearthomeCitySoutheastHouse2f Person1': "A bell that rings like the sea. CLEFAIRY doesn't need it. Here!",
    'JubilifeCityCondominiums1f Person4': "This was a housewarming gift, but I have two. Please, take one!",
    'OreburghCity Person17': "Here, a little something from the first-aid box. Miners share!",
    'OreburghCityEastHouse2f Person1': "I traded a lump of coal for this BALL. You can have it!",
    'OreburghCityNorthHouse2f Person3': "I don't collect these, though. This BALL is yours.",
    'OreburghCityNorthwestHouse2f Person3': "Want this BALL? It works best in dark places, like the mine!",
    'PastoriaCityMiddleHouse Person2': "It made me so strong, I don't need it anymore. Here!",
    'PastoriaCityNortheastHouse Person1': "My sister made scarves for every MOéMON we know. Here's a spare!",
    'PokemonMansionMaidsRoom Person2': "Here, this bell is too loud for the maids' room. Your MOéMON will love it.",
    'Route201 Person2': "Here, have a free POTION!",
    'Route204North Person11': "This TM blows like that breeze. Take it!",
    'Route209 Person29': "You don't have a good rod? Take my old one. Patience does the rest!",
    'Route209LostTower5f Person1': "Please take this. It keeps the memory of a MOéMON's spirit.",
    'Route209LostTower5f Person2': "Please take this tag. It keeps restless spirits away from you.",
    'Route210South Person22': "If you get lost in the fog, this TM will help you fly out of it!",
    'Route211East Person18': "Here's a TM for the long climb. It'll pump your MOéMON up!",
    'Route215 Person24': "You train in the rain too? Then you've earned this TM!",
    'Route217WestHouse Person1': "I found this plate frozen in the snow. Maybe it's yours now.",
    'Route218GateToJubilifeCity Person2': "Never fished before? Then start with this rod!",
    'Route221House Person1': "I was going to give this belt to the park's workers. You take it!",
    'Route222 Person26': "Since you're my guest on my beach, here's a TM. Money is no object!",
    'Route222EastHouse Person1': "This BALL is made for MOéMON of the water. Take it!",
    'Route225House Person1': "You look thirsty, climbing all the way up here. Have some water!",
    'Route229 Person3': "Here's something I found in the grass. Shiny, isn't it?",
    'SolaceonRuinsRoom2 Person1': "I found this green stone in one of the rooms. You can have it!",
    'SolaceonTownPokemonNewsPress Person1': "Thanks for listening! Here's a pretty scale a reader sent us.",
    'SurvivalAreaSouthHouse Person1': "You've got the spirit of a fighter! Take this TM.",
    'TrainersSchool Person7': "Here, I have an extra POTION from class. Take it!",
    'VeilstoneCity Person23': "A TOUGH TRAINER deserves a TOUGH TM. Here!",
}


def _body(text, label, seen):
    """A script entry's lines and those of every label it goes to (one file)"""
    if label in seen:
        return ''
    seen.add(label)
    m = re.search(r'^%s:\s*$' % re.escape(label), text, re.M)
    if not m:
        return ''
    rest = text[m.end():]
    n = re.search(r'^\w+:\s*$', rest, re.M)
    out = rest[:n.start()] if n else rest
    for t in re.findall(r'^\s*(?:GoTo|Call|GoToIf\w*|CallIf\w*)\b[^\n]*?(\w+)\s*$', out, re.M):
        out += _body(text, t, seen)
    return out


def given(root, low, entry):
    """(item, count) a script entry of Platinum's map LOW hands over, or None"""
    path = os.path.join(root, 'res/field/scripts/scripts_%s.s' % low)
    if not os.path.exists(path):
        return None
    body = _body(open(path, encoding='utf-8', errors='replace').read(), entry, set())
    if 'Common_GiveItemQuantity' not in body:
        return None
    m = re.search(r'SetVar VAR_0x8004, (ITEM_\w+)', body)
    if not m:
        return None
    c = re.search(r'SetVar VAR_0x8005, (\d+)', body[m.end():])
    return m.group(1), int(c.group(1)) if c else 1


@functools.lru_cache(None)
def gifts(root):
    """{(map name, 'Person<n>'): (the game's item, count, flag)}"""
    from source import open_source
    from gen_maps import places
    from places import events
    import who
    found = []
    for p in places(open_source(root), root):
        low = p['place']['header'][len('MAP_HEADER_'):].lower()
        names = who.entries(root, low)
        for i, o in enumerate(events(root, p['place'])['object_events']):
            s = o['script']
            if not isinstance(s, int) or not 1 <= s <= len(names):
                continue
            g = given(root, low, names[s - 1])
            if not g or g[0] in SKIP:
                continue
            try:
                item = items.ours(g[0])
            except SystemExit:
                continue   # (not an item of this game's)
            if item:
                found.append((p['name'], 'Person%d' % (i + 1), item, g[1]))
    found.sort()
    assert GIFT_FLAGS + len(found) <= GIFT_FLAGS_END, 'too many SINNOH gifts'
    return {(m, k): (item, n, GIFT_FLAGS + i) for i, (m, k, item, n) in enumerate(found)}


def line(name, key):
    """The words as it changes hands (None: not written yet)"""
    return LINES.get('%s %s' % (name[len('Sinnoh_'):], key))
