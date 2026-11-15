"""Shared pieces of the UI style mock-ups (tools/ui_styles/mockups.py): the scenes every style is drawn over (the game's
own pictures with its interface hidden: tools/test/scenarios/uicap_*), what each screen says, and small drawing helpers.
Everything is drawn at the GBA's 240 x 160, never scaled."""
import json
import os

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
W, H = 240, 160

# ------------------------------------------------------------------------------------------------- what's on screen
FOE = dict(name='POOCHYENA', sex='m', level=7, hp=1.0, species=286)
MINE = dict(name='TORCHIC', sex='m', level=24, hp=56, maxhp=56, exp=0.35, species=280)
DOUBLE_FOES = [dict(name='ZIGZAGOON', sex='m', level=6, hp=1.0, species=288),
               dict(name='POOCHYENA', sex='m', level=7, hp=1.0, species=286)]
DOUBLE_MINE = [dict(name='TORCHIC', sex='m', level=24, hp=56, maxhp=56, exp=0.35, species=280),
               dict(name='MUDKIP', sex='m', level=22, hp=55, maxhp=55, exp=0.6, species=283)]
TRIPLE_FOES = [dict(name='ZIGZAGOON', sex='m', level=6, hp=1.0, species=288),
               dict(name='WURMPLE', sex='m', level=7, hp=1.0, species=290),
               dict(name='POOCHYENA', sex='m', level=7, hp=1.0, species=286)]
TRIPLE_MINE = [dict(name='TORCHIC', sex='m', level=24, hp=56, maxhp=56, exp=0.35, species=280),
               dict(name='POOCHYENA', sex='f', level=20, hp=47, maxhp=47, exp=0.2, species=286),
               dict(name='MUDKIP', sex='m', level=22, hp=55, maxhp=55, exp=0.6, species=283)]
PARTY = [dict(name='TORCHIC', sex='m', level=24, hp=56, maxhp=56, species=280),
         dict(name='MUDKIP', sex='m', level=22, hp=55, maxhp=55, species=283),
         dict(name='POOCHYENA', sex='f', level=20, hp=47, maxhp=47, species=286),
         dict(name='ZIGZAGOON', sex='f', level=18, hp=42, maxhp=42, species=288)]
MOVES = [('FOCUS ENERGY', 30, 30, 'NORMAL'), ('EMBER', 25, 25, 'FIRE'), ('PECK', 35, 35, 'FLYING'),
         ('SAND-ATTACK', 15, 15, 'GROUND')]
ACTIONS = ['FIGHT', 'BAG', 'MOéMON', 'RUN']
PROMPT = ('What will', 'TORCHIC do?')
START_MENU = ['MOéDEX', 'MOéMON', 'BAG', 'TEST', 'SAVE', 'OPTIONS', 'EXIT']
MESSAGE = 'Welcome!'
BAG_ITEMS = [('POTION', 10, 13), ('ANTIDOTE', 2, 14), ('AWAKENING', 1, 17), ('FRESH WATER', 3, 26)]
BAG_POCKET = 'MEDICINE'
BAG_DESC = ['A spray-type medicine for wounds.', 'It restores the HP of one MOéMON by', 'just 20 points.']
POCKETS = ['ITEMS', 'MEDICINE', 'POKé BALLS', 'TMs & HMs', 'BERRIES', 'BATTLE ITEMS', 'KEY ITEMS', 'FAVORITES']
SUMMARY = dict(no='004', name='TORCHIC', species='TORCHIC', level=24, sex='m', ot='TEST', idno='02016',
               type='FIRE', ability='BLAZE', ability_text='Ups FIRE moves in a pinch.', nature='TIMID',
               met='met at Lv24,', place='INSIDE OF TRUCK.', item='NONE')
OPTION_TABS = ['GAME', 'BATTLE', 'DISPLAY', 'CONTROLS', 'CO-OP']
# the DISPLAY tab with the new row: UI STYLE (the style the mock-up shows chosen)
OPTION_ROWS = [('UI STYLE', None), ('THEME', ['LIGHT', 'DARK']), ('FRAME', ['TYPE 1']),
               ('SHADOWS', ['OFF', 'ON']), ('BATTLE ANIMS', ['CLASSIC', 'MODERN']), ('TYPE ICONS', ['OFF', 'ON'])]
STYLES = ['KAI', 'RED', 'CRYSTAL', 'FIRERED', 'PLATINUM', 'BLACK']


# ------------------------------------------------------------------------------------------------- the game's pictures
def shot(kai, name):
    """one of the game's own pictures (rendershot, 6x) at 240 x 160"""
    return Image.open(os.path.join(kai, name + '.png')).convert('RGBA').resize((W, H), Image.NEAREST)


def creature(export, species, kind='front'):
    """a MOéMON's front picture or icon from the game's export (MOEMON_EXPORT_APP, src/mp/mp_export.c)"""
    if kind == 'icon':
        return Image.open(os.path.join(export, 'icons', '%d.png' % species)).convert('RGBA').crop((0, 0, 32, 32))
    p = Image.open(os.path.join(export, 'front', '%d.png' % species)).convert('RGBA')
    if 'pal' not in json.load(open(os.path.join(export, 'data.json')))['species'][species]:
        return p   # (an export with its pictures in colour)
    pals = {i: sp.get('pal', '') for i, sp in enumerate(json.load(open(os.path.join(export, 'data.json')))['species'])}
    pal = pals.get(species, '')
    cols = []
    for k in range(16):
        v = int(pal[4 * k:4 * k + 4] or '0', 16)
        cols.append(((v & 31) * 255 // 31, (v >> 5 & 31) * 255 // 31, (v >> 10 & 31) * 255 // 31, 255))
    px = p.load()
    for y in range(p.height):
        for x in range(p.width):
            r, g, b, a = px[x, y]
            if a:
                px[x, y] = cols[r // 17]
    return p


def item_icon(export, item):
    p = os.path.join(export, 'items', '%d.png' % item)
    return Image.open(p).convert('RGBA') if os.path.exists(p) else None


def paste(im, pic, x, y):
    layer = Image.new('RGBA', im.size, (0, 0, 0, 0))
    layer.paste(pic, (x, y), pic)
    im.alpha_composite(layer)


def rect(im, box, fill=None, outline=None):
    ImageDraw.Draw(im).rectangle(box, fill=fill, outline=outline)


def hp_colour(frac, green=(88, 208, 128), yellow=(248, 224, 56), red=(248, 88, 56)):
    return green if frac > 0.5 else yellow if frac > 0.2 else red
