#!/usr/bin/env python3
"""Moemon Co-op trainer creator: matches the parts kit's files across views.

The kit ("pokemon png") has the same clothes drawn for 7 views (front, back,
overworld walk/run/bike, per gender), but folder names, file names and even
color names differ between views. This module builds one table:
    items[gender][category] = [(style, [(color, {view: path})...])...]
Colors are matched between views by their actual average color.
"""
import glob
import os
import re

import numpy as np
from PIL import Image

VIEWS = ('front', 'back', 'walk', 'run', 'bike')
TOPS = ['t-shirt', 'tank top', 'v-neck', 'collared shirt', 'shirt combo', 'hoodie', 'open jacket', 'vest',
        'scarf top', 'turtleneck', 'trench coat', 'classy top', 'formal top', 'school uniform', 'jumpsuit top',
        'alt jumpsuit top', 'beach top', 'lord suit top']
BOTTOMS = ['jeans', 'shorts', 'capri pants', 'pants and boots', 'classy bottoms', 'formal bottoms', 'long skirt',
           'pencil skirt', 'ribbon skirt', 'school uniform bottom', 'jumpsuit pants', 'jumpsuit pants alt',
           'beach bottom', 'lord suit pants']
HATS = ['beanie', 'youngster hat', 'beret', 'fedora', 'straw hat', 'sun hat', 'feather hat', 'bonnet', 'crown',
        'flower', 'ribbon', 'hair band', 'headband', 'glasses', 'cat ears', 'devil', 'mining', 'poop']
BAGS = ['bag', 'rucksack', 'sporty backpack']
SKINS = ['pale', 'light', 'medium', 'dark']
HAIR_COLORS = ['black', 'brown', 'blonde', 'ginger', 'red', 'pink', 'purple', 'blue', 'cyan', 'green', 'white']

TOP_ALIASES = {'trenchcoat': 'trench coat', 'scarf shirt': 'scarf top', 'school uniform top': 'school uniform',
               'jumpsuit alt top': 'alt jumpsuit top', 'jumpsuit alt': 'alt jumpsuit top', 'jumpsuit': 'jumpsuit top',
               'lord shirt': 'lord suit top', 'lord suit': 'lord suit top', 'beach': 'beach top',
               'classy': 'classy top', 'formal': 'formal top'}
BOTTOM_ALIASES = {'capri': 'capri pants', 'capris': 'capri pants', 'classy pants': 'classy bottoms',
                  'classy bottom': 'classy bottoms', 'formal pants': 'formal bottoms', 'school girl': 'school uniform bottom',
                  'lords pants': 'lord suit pants'}
HAT_ALIASES = {'cat': 'cat ears', 'cate': 'cat ears', 'baguette': 'beret', 'miner': 'mining', 'hairband': 'hair band', 'youngster cap': 'youngster hat'}


def norm(s):
    s = s.lower().replace('.png', '')
    s = re.sub(r'\s+', ' ', s).strip()
    return s


def strip_gender(s):
    return re.sub(r' (m|f|fm|w)$', '', s)


def avg_color(path):
    a = np.array(Image.open(path).convert('RGBA')).astype(int)
    vis = a[:, :, 3] > 128
    rgb = a[:, :, :3][vis]
    if len(rgb) == 0:
        return np.zeros(3)
    v = rgb.max(1)
    keep = rgb[v > 50]   # not the outline
    return (keep if len(keep) else rgb).mean(0)


def files_in(d):
    out = {}
    if os.path.isdir(d):
        for f in sorted(glob.glob(os.path.join(d, '*.png'))):
            out[norm(os.path.basename(f))] = f
    return out


def color_name(fname, style_words):
    """'jeans  light blue' -> 'light blue'."""
    n = fname
    for w in sorted(style_words, key=len, reverse=True):
        n = n.replace(w, ' ')
    n = re.sub(r'\b(m|f|w|fm|top|tops|bottom|bottoms|alt|skirt|shirt|pants|lords|open|short|school|girl|boy|schoolgirl|schoolboy|#1|copy)\b', ' ', n)
    n = re.sub(r'^\d+\b', ' ', n.strip())
    return re.sub(r'\s+', ' ', n).strip() or 'default'


class Kit:
    def __init__(self, root):
        self.root = root

    def d(self, view, gender):
        g = 'male' if gender == 'm' else 'female'
        return {'front': 'trainer front ' + g, 'back': 'trainer back ' + g, 'walk': 'overworld walk',
                'run': 'overworld run', 'bike': 'overworld bike'}[view]

    # ---- per view: style -> list of (colorname, path)
    def styles(self, view, gender, cat):
        base = os.path.join(self.root, self.d(view, gender))
        if view == 'front':
            sub = {'tops': 'tops', 'bottoms': 'bottoms', 'hats': 'hats', 'hair': 'hair'}[cat]
        elif view == 'back':
            sub = {'tops': 'clothing', 'hats': 'hat', 'hair': 'hair', 'bottoms': None}[cat]
        else:
            sub = {'tops': 'tops', 'bottoms': 'bottoms', 'hats': 'hats', 'hair': 'hair', 'bags': 'carrying stuff'}[cat]
        if sub is None or not os.path.isdir(os.path.join(base, sub)):
            return {}
        root = os.path.join(base, sub)
        out = {}
        for entry in sorted(os.listdir(root)):
            p = os.path.join(root, entry)
            n = norm(entry)
            if os.path.isdir(p):
                gsuffix = re.search(r' (m|f|fm|w)$', n)
                if view in ('walk', 'run', 'bike') and gsuffix and gsuffix.group(1) in ('m', 'f') and gsuffix.group(1) != gender:
                    continue
                files = files_in(p)
                if view in ('walk', 'run', 'bike') and gsuffix and gsuffix.group(1) == 'fm':
                    want = ('f', 'girl') if gender == 'f' else ('m', 'boy')
                    files = {k: v for k, v in files.items() if re.search(r'\b(%s)\b|%s' % (want[0], want[1]), k)}
                style = strip_gender(n)
                out[style] = [(k, v) for k, v in files.items()]
            elif n.endswith('.png') or entry.lower().endswith('.png'):
                out[strip_gender(n)] = [('default', p)]
        return out

    def canon(self, cat, style, gender, view):
        if cat == 'tops':
            return TOP_ALIASES.get(style, style)
        if cat == 'bottoms':
            if gender == 'f' and view == 'front':
                # the girl's front folder swaps these two names
                if style == 'classy bottom':
                    return 'formal bottoms'
                if style == 'evening bottom':
                    return 'classy bottoms'
            if style in ('lords pants', 'lord suit pants'):
                return 'lord suit pants'
            return BOTTOM_ALIASES.get(style, style)
        if cat == 'hats':
            return HAT_ALIASES.get(style, style)
        if cat == 'hair':
            m = re.search(r'(\d)', style)
            return 'hair %s' % m.group(1)
        return style

    def build(self):
        items = {}
        for g in 'mf':
            items[g] = {}
            for cat, names, views in (('tops', TOPS, ('front', 'back', 'walk', 'run', 'bike')),
                                      ('bottoms', BOTTOMS, ('front', 'walk', 'run', 'bike')),
                                      ('hats', HATS, ('front', 'back', 'walk', 'run', 'bike')),
                                      ('hair', ['hair 1', 'hair 2', 'hair 3', 'hair 4'], ('front', 'back', 'walk', 'run', 'bike')),
                                      ('bags', BAGS, ('walk', 'run', 'bike'))):
                per_view = {}
                for v in views:
                    st = self.styles(v, g, cat)
                    if cat == 'hats':
                        # hats: single files at top level + color folders
                        flat = {}
                        for s, lst in st.items():
                            flat[self.canon(cat, s, g, v)] = lst
                        per_view[v] = flat
                    elif cat == 'hair' and v in ('walk', 'run', 'bike'):
                        want = 'boy' if g == 'm' else 'girl'
                        per_view[v] = {self.canon(cat, s, g, v): lst for s, lst in st.items() if s.startswith(want)}
                    else:
                        per_view[v] = {self.canon(cat, s, g, v): lst for s, lst in st.items()}
                result = []
                for name in names:
                    if not all(name in per_view[v] for v in views):
                        missing = [v for v in views if name not in per_view[v]]
                        print('  %s %s %-22s missing in %s' % (g, cat, name, missing))
                        continue
                    ref_view = views[0]
                    ref = per_view[ref_view][name]
                    colors = []
                    used = {v: set() for v in views}
                    for cname, cpath in ref:
                        c0 = avg_color(cpath)
                        paths = {ref_view: cpath}
                        for v in views[1:]:
                            cand = [(np.abs(avg_color(p) - c0).sum(), i, p) for i, (n2, p) in enumerate(per_view[v][name]) if i not in used[v]]
                            if not cand:
                                break
                            d, i, p = min(cand)
                            used[v].add(i)
                            paths[v] = p
                        if len(paths) == len(views):
                            colors.append((color_name(cname, name.split() + [name]), paths))
                    result.append((name, colors))
                items[g][cat] = result
        return items


if __name__ == '__main__':
    import sys
    kit = Kit(sys.argv[1])
    items = kit.build()
    for g in 'mf':
        for cat, lst in items[g].items():
            print(g, cat, len(lst), ', '.join('%s(%d)' % (s, len(c)) for s, c in lst))
