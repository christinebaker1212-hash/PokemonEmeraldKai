"""SINNOH's items on the ground (tools/sinnoh/gen_maps.py): which item each of Platinum's item balls and hidden items is,
as the game's own item constants. Read from pret/pokeplatinum: the visible items' script list (an item ball's script
is 7000 + its place in that list) and the hidden items' table (a hidden item's is 8000 + its place); only the item and
how many reach the repository.

TM51-TM92 are the game's own (items 618-659, tools/moemon_tms/gen_tm2.py). Story keys (the GALACTIC KEY, the SUITE
KEY, the LUNAR WING) wait for their scenes.
"""
import functools
import os
import re

VISIBLE_BASE, HIDDEN_BASE = 7000, 8000
HIDDEN_FLAGS = 0x3E00          # a hidden item's flag: HIDDEN_FLAGS + its place in Platinum's table (ids.py stays below)

RENAME = {'ITEM_PARLYZ_HEAL': 'ITEM_PARALYZE_HEAL', 'ITEM_ENERGYPOWDER': 'ITEM_ENERGY_POWDER',
          'ITEM_SILVERPOWDER': 'ITEM_SILVER_POWDER', 'ITEM_THUNDERSTONE': 'ITEM_THUNDER_STONE',
          'ITEM_TINYMUSHROOM': 'ITEM_TINY_MUSHROOM', 'ITEM_NEVERMELTICE': 'ITEM_NEVER_MELT_ICE',
          'ITEM_X_DEFENSE': 'ITEM_X_DEFEND',}
STORY = {'ITEM_GALACTIC_KEY', 'ITEM_SUITE_KEY', 'ITEM_LUNAR_WING'}

_own = None


def own():
    global _own
    if _own is None:
        here = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
        _own = set(re.findall(r'#define (ITEM_\w+)\s', open(os.path.join(here, 'include/constants/items.h')).read()))
    return _own


def ours(item):
    """The game's item for a Platinum item name, or None (a story item)"""
    if item in STORY:
        return None
    item = RENAME.get(item, item)
    if item not in own():
        raise SystemExit('items.py: no item %s in the game' % item)
    return item


@functools.lru_cache(None)
def visible(root):
    """[(item, count)] in script order"""
    text = open(os.path.join(root, 'res/field/scripts/scripts_visible_items.s'), encoding='utf-8').read()
    names = re.findall(r'^\s*ScriptEntry (\w+)', text, re.M)
    out = []
    for n in names:
        m = re.search(r'^%s:\s*\n\s*SetVar VAR_0x8008, (ITEM_\w+)\s*\n\s*SetVar VAR_0x8009, (\d+)' % n, text, re.M)
        out.append((m.group(1), int(m.group(2))) if m else (None, 0))
    return out


@functools.lru_cache(None)
def hidden(root):
    text = open(os.path.join(root, 'include/data/field/hidden_items.h'), encoding='utf-8').read()
    return [(i, int(q)) for i, q in re.findall(r'HIDDEN_ITEM_ENTRY\((ITEM_\w+),\s*(\d+),\s*\d+,\s*\w+\)', text)]


def ball(root, script):
    """An item ball's (our item, count), or None"""
    if not isinstance(script, int) or not VISIBLE_BASE <= script < HIDDEN_BASE:
        return None
    v = visible(root)
    k = script - VISIBLE_BASE
    if k >= len(v) or v[k][0] is None or ours(v[k][0]) is None:
        return None
    return ours(v[k][0]), v[k][1]


def buried(root, script):
    """A hidden item's (our item, count, flag), or None"""
    if not isinstance(script, int) or not HIDDEN_BASE <= script < HIDDEN_BASE + 800:
        return None
    h = hidden(root)
    k = script - HIDDEN_BASE
    if k >= len(h) or ours(h[k][0]) is None:
        return None
    return ours(h[k][0]), h[k][1], HIDDEN_FLAGS + k


def script_label(item, count):
    return 'Sinnoh_EventScript_' + item[len('ITEM_'):].title().replace('_', '') + ('' if count == 1 else 'x%d' % count)
