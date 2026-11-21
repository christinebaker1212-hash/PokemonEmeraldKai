#!/usr/bin/env python3
"""Every Yu-Gi-Oh! card's power from its own text, effects and stats (the user, 2026-11-21: "rating card power based on
actual card text, effects, and stats"; docs/YGO_DECISIONS.md "NPC decks from YGOPRODeck").

Each card (tools/ygo/catalog.json) is read into features: what its effects do (search, draw, Special Summon from each
place, negation, each kind of removal, floodgates, hand traps, recursion, burn...), what holds them back (costs, once
per turn, archetype locks, summoning conditions) and its stats against its Level. A ridge regression fits a weight to
each feature against what is known of real cards' power:
  - the duel-measured ratings of the older cards (src/ygo/ygo_card_ratings.c, AI-against-AI duels on EDOPro),
  - how hard the Forbidden & Limited lists ever hit it (tools/ygo/banlists.json, TCG and OCG since 2002),
  - how often it is played in tournament decks (YGOPRODeck's Tournament Meta Decks, ~/.cache/moemon/ygoprodeck_decks),
so the text decides every card's score, the known cards only teach the weights. Writes tools/ygo/card_power.json
({password: power}, an Elo-like scale: a plain card 1500, the strongest near 2300) and docs/audit_ygo_text_power.txt
(the weights, the top and bottom cards, how well the text explains the known cards).

  tools/ygo/text_power.py
"""
import glob, json, math, os, re, sys
import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DECKS = os.path.expanduser('~/.cache/moemon/ygoprodeck_decks')

NUM = {'a': 1, 'an': 1, 'one': 1, 'two': 2, 'three': 3, 'four': 4, 'five': 5}


def n_of(m, default=1):
    if not m:
        return 0
    g = m.group(1) if m.lastindex else None
    if not g:
        return default
    g = g.lower()
    return int(g) if g.isdigit() else NUM.get(g, default)


def split_effects(text):
    """the card's text cut into its effects (sentences and bullet lines)"""
    t = text.replace('\r', '\n')
    parts = re.split(r'\n+|(?<=[.!])\s+(?=[A-Z●"(])', t)
    return [p.strip() for p in parts if p.strip()]


def features(c):
    """a dict of named features for one catalog card"""
    t = (c.get('text') or '').replace('\r\n', '\n')
    low = t.lower()
    frame = c['frame']
    tl = c.get('typeline', '')
    f = {}
    monster = frame not in ('spell', 'trap', 'token')
    extra = frame in ('fusion', 'synchro', 'xyz', 'link')
    lvl = c.get('level') or 0
    atk = c.get('atk') if isinstance(c.get('atk'), int) and c.get('atk') >= 0 else 0
    df = c.get('defense') if isinstance(c.get('defense'), int) and c.get('defense') >= 0 else 0

    # what kind of card
    f['is_monster'] = monster
    f['is_extra'] = extra
    f['is_spell'] = frame == 'spell'
    f['is_trap'] = frame == 'trap'
    f['vanilla'] = frame == 'normal'
    prop = (c.get('property') or '').lower()
    f['quickplay'] = prop == 'quick-play'
    f['continuous_st'] = prop == 'continuous'
    f['field_spell'] = prop == 'field'
    f['equip'] = prop == 'equip'
    f['counter_trap'] = prop == 'counter'
    f['ritual'] = frame == 'ritual' or prop == 'ritual'
    f['pendulum'] = 'Pendulum' in tl
    f['tuner'] = 'Tuner' in tl
    f['gemini_spirit_union_toon'] = any(k in tl for k in ('Gemini', 'Spirit', 'Union', 'Toon'))
    f['flip'] = 'Flip' in tl

    # stats (monsters): ATK against what its Level asks for
    if monster:
        tribute = 2 if lvl >= 7 else 1 if lvl >= 5 else 0
        if extra:
            tribute = 0
        f['atk_k'] = atk / 1000.0
        f['def_k'] = df / 1000.0
        f['atk_per_cost'] = atk / 1000.0 / (1 + tribute) if not extra else 0
        f['low_level'] = lvl <= 4 and not extra and frame != 'ritual'
        f['high_level_main'] = lvl >= 5 and not extra and frame != 'ritual'
        f['link_rating'] = lvl if frame == 'link' else 0
    # what the effects do
    f['search_deck'] = len(re.findall(r'add (?:up to )?(\w+)?[^.;]*? from your deck to your hand', low))
    f['search_deck_or_gy'] = len(re.findall(r'add [^.;]*? from your deck or gy to your hand', low))
    f['recover_gy'] = len(re.findall(r'add [^.;]*? from your (?:gy|graveyard) to your hand', low))
    f['draw'] = sum(n_of(m) for m in re.finditer(r'draw (\w+) cards?', low))
    f['draw_cost_both'] = 1 if re.search(r'each player draws', low) else 0
    f['ss_from_deck'] = len(re.findall(r'special summon [^.;]*? from your deck', low))
    f['ss_from_gy'] = len(re.findall(r'special summon [^.;]*? from (?:your|either player\'s|your opponent\'s) (?:gy|graveyard)', low))
    f['ss_from_hand'] = len(re.findall(r'special summon [^.;]*? from your hand', low))
    f['ss_from_extra'] = len(re.findall(r'special summon [^.;]*? from your extra deck', low))
    f['ss_self'] = 1 if re.search(r'(?:you can )?special summon this card', low) else 0
    f['ss_self_from_hand'] = 1 if re.search(r'special summon this card (?:\(from your hand\)|from your hand)', low) else 0
    f['self_revive'] = 1 if re.search(r'special summon this card from (?:your )?(?:gy|graveyard)', low) else 0
    f['token_summon'] = len(re.findall(r'special summon (?:\w+ )?[^.;]*?tokens?', low))
    f['extra_normal'] = 1 if re.search(r'additional normal summon|normal summon 1 monster in addition', low) else 0
    f['send_deck_gy'] = len(re.findall(r'send [^.;]*? from your deck to the (?:gy|graveyard)', low))
    f['banish_deck'] = len(re.findall(r'banish [^.;]*? from your deck', low))
    f['excavate_mill_self'] = 1 if re.search(r'excavate|send the top \w+ cards? of your deck', low) else 0
    f['negate_activation'] = len(re.findall(r'negate the activation', low))
    f['negate_effect'] = len(re.findall(r'negate (?:that|its|the|those)? ?(?:card\'s )?effects?', low))
    f['negate_summon'] = len(re.findall(r'negate the summon', low))
    f['destroy_all'] = len(re.findall(r'destroy all (?:cards|monsters|spells?|traps?|spell/trap|face-up|set)', low))
    f['destroy_opp_all'] = len(re.findall(r'destroy all (?:cards|monsters|spells?|traps?|spell/trap)[^.;]*?your opponent controls', low))
    f['destroy_target'] = len(re.findall(r'(?:target \w+ [^.;]*?; destroy|destroy (?:that|those|it|1|up to \w+|the targeted))', low))
    f['banish_opp'] = len(re.findall(r'banish (?:that|those|it|1|up to \w+|all)[^.;]*?(?:your opponent controls|on the field|opponent\'s)', low))
    f['banish_any'] = len(re.findall(r'banish (?:that|those|it|1 card|the targeted)', low))
    f['bounce'] = len(re.findall(r'return [^.;]*? to the hand', low))
    f['spin'] = len(re.findall(r'shuffle [^.;]*? into the deck', low))
    f['send_field_gy'] = len(re.findall(r'send [^.;]*?(?:your opponent controls|on the field) to the (?:gy|graveyard)', low))
    f['take_control'] = 1 if re.search(r'take control|gain control', low) else 0
    f['change_position'] = 1 if re.search(r'change (?:it|that monster|its battle position|the battle position)', low) else 0
    f['atk_zero_or_minus'] = 1 if re.search(r'atk becomes 0|loses? \d+ atk|atk/def become 0', low) else 0
    f['floodgate_cannot'] = len(re.findall(r'(?:your opponent cannot|neither player can|your opponent can only)', low))
    f['floodgate_ss'] = 1 if re.search(r'(?:neither player|your opponent) cannot special summon', low) else 0
    f['protect_self'] = len(re.findall(r'cannot be (?:destroyed|targeted|banished)|unaffected by', low))
    f['protect_others'] = 1 if re.search(r'monsters you control cannot be|cards you control cannot be', low) else 0
    f['hand_trap'] = 1 if (monster and re.search(r'discard this card', low) and re.search(r'quick effect|during either player\'s turn|your opponent\'s turn', low)) else 0
    f['quick_effect'] = len(re.findall(r'\(quick effect\)', low))
    f['from_gy_effect'] = 1 if re.search(r'banish this card from (?:your )?(?:gy|graveyard)', low) else 0
    f['float_on_destroy'] = 1 if re.search(r'if this card (?:is|was) (?:destroyed|sent to the (?:gy|graveyard))', low) else 0
    f['on_summon_trigger'] = 1 if re.search(r'if this card is (?:normal or special |normal |special |tribute |flip )?summoned', low) else 0
    f['burn'] = sum(int(x) for x in re.findall(r'inflict (\d+) damage', low)) / 1000.0
    f['lp_gain'] = sum(int(x) for x in re.findall(r'gain (\d+) (?:lp|life points)', low)) / 1000.0
    f['piercing_multi_attack'] = 1 if re.search(r'piercing|attack twice|attack all|second attack|attack directly', low) else 0
    f['stat_boost'] = 1 if re.search(r'gains? \d+ atk', low) else 0
    f['hand_disrupt'] = 1 if re.search(r'(?:your opponent|opponent\'s hand)[^.;]*?discard|look at your opponent\'s hand', low) else 0
    f['mill_opp'] = 1 if re.search(r'top \w+ cards? of your opponent\'s deck', low) else 0
    f['wins_duel'] = 1 if re.search(r'win the duel', low) else 0
    # what holds them back
    f['once_per_turn'] = len(re.findall(r'once per turn', low))
    f['hard_once'] = len(re.findall(r'you can only (?:use|activate)[^.]*?once per turn|you can only activate 1', low))
    f['lock_after'] = 1 if re.search(r'you cannot (?:special summon|activate)[^.]*?(?:except|for the rest of this turn)', low) else 0
    f['cost_pay_lp'] = sum(int(x) for x in re.findall(r'pay (\d+) lp', low)) / 1000.0
    f['cost_half_lp'] = 1 if 'pay half your lp' in low else 0
    f['cost_discard'] = len(re.findall(r'(?:discard \w+ cards?|discard 1 [^;.]*?);', low))
    f['cost_tribute'] = len(re.findall(r'tribute [^;.]*?;', low))
    f['cost_banish_own'] = len(re.findall(r'banish [^;.]*?from your (?:hand|field|gy|graveyard)[^;.]*?;', low))
    f['must_be_ss_condition'] = 1 if re.search(r'cannot be normal summoned|must first be|must be special summoned', low) else 0
    f['self_destroy_upkeep'] = 1 if re.search(r'during (?:your|each) standby phase[^.]*?(?:destroy this card|pay|send this card)', low) else 0
    f['self_harm'] = 1 if re.search(r'you take \d+ damage|inflict \d+ damage to yourself|you lose \d+ lp', low) else 0
    f['opponent_benefit'] = 1 if re.search(r'your opponent (?:draws|gains|can special summon|adds)', low) else 0
    f['archetype_refs'] = len(re.findall(r'"[^"]+"', t)) - len(re.findall(r'"%s"' % re.escape(c['name']), t))
    f['generic_support'] = 1 if (f['search_deck'] or f['ss_from_deck']) and f['archetype_refs'] <= 0 else 0
    f['effect_count'] = len(split_effects(t)) if frame not in ('normal', 'token') else 0
    f['text_length_k'] = len(t) / 1000.0 if frame not in ('normal', 'token') else 0
    # extra deck materials: generic is easier to make
    if extra:
        first = t.split('\n')[0].lower()
        f['generic_materials'] = 1 if re.match(r'^(?:\d\+? |1 tuner \+ 1\+ non-tuner|2\+? (?:effect )?monsters|2 level \d monsters)', first) else 0
        f['named_material'] = 1 if '"' in first else 0
        f['material_count'] = sum(int(x) for x in re.findall(r'(\d)', first.split(' + ')[0][:3])) if re.match(r'^\d', first) else 2
    return {k: float(v) for k, v in f.items()}


def old_ratings():
    path = os.path.join(ROOT, 'src/ygo/ygo_card_ratings.c')
    out = {}
    for m in re.finditer(r'\[(\d+)\] = (-?\d+),', open(path).read()):
        out[int(m.group(1))] = int(m.group(2))
    return out


def banlist_signal(cards):
    """how hard the lists ever hit each card: 0 never, up to 1 (Forbidden for long)"""
    lists = json.load(open(os.path.join(ROOT, 'tools/ygo/banlists.json')))
    worst, hits = {}, {}
    for l in lists:
        for pw, st in l['cards'].items():
            v = {0: 1.0, 1: 0.65, 2: 0.35}.get(st, 0)
            worst[pw] = max(worst.get(pw, 0), v)
            hits[pw] = hits.get(pw, 0) + v
    out = {}
    for c in cards:
        pw = str(c['password'])
        if pw in worst:
            out[c['password']] = worst[pw] * 0.7 + min(1.0, hits[pw] / 20.0) * 0.3
    return out


def usage_signal(cards):
    """how often tournament decks play it (decks with it, out of the tournament decks), on a log scale"""
    files = [p for p in glob.glob(os.path.join(DECKS, '*.jsonl')) if 'Tournament' in os.path.basename(p)
             or 'world_championship' in os.path.basename(p)]
    count, decks = {}, 0
    seen = set()
    for p in files:
        for line in open(p):
            try:
                d = json.loads(line)
            except ValueError:
                continue
            if d['deckNum'] in seen:
                continue
            seen.add(d['deckNum'])
            ids = set()
            for k in ('main_deck', 'extra_deck', 'side_deck'):
                try:
                    ids |= set(int(x) for x in json.loads(d.get(k) or '[]'))
                except (ValueError, TypeError):
                    pass
            for x in ids:
                count[x] = count.get(x, 0) + 1
            decks += 1
    if not decks:
        return {}, 0
    return {pw: math.log1p(n) / math.log1p(decks) for pw, n in count.items()}, decks


def first_year(c, setdates):
    ds = [setdates[x] for x in c.get('sets', []) if setdates.get(x)]
    return int(min(ds)[:4]) if ds else 2010


def main():
    from scipy import sparse
    from sklearn.feature_extraction.text import TfidfVectorizer
    from sklearn.linear_model import Ridge
    from sklearn.model_selection import KFold
    catalog = json.load(open(os.path.join(ROOT, 'tools/ygo/catalog.json')))
    setdates = {x['code']: x.get('date') for x in catalog['sets']}
    cards = [c for c in catalog['cards'] if c['frame'] != 'token']
    feats = [features(c) for c in cards]
    names = sorted(set(k for f in feats for k in f)) + ['first_year']
    X = np.array([[f.get(k, 0.0) for k in names[:-1]] + [first_year(c, setdates)] for f, c in zip(feats, cards)])
    mu, sd = X.mean(0), X.std(0) + 1e-9
    Xn = (X - mu) / sd
    texts = [(c.get('text') or '') + ' ' + c.get('typeline', '') + ' ' + (c.get('property') or '') for c in cards]

    # what is known of each card, each signal as z-scores, a weighted mean over the signals it has: the lists and
    # tournament play count fully; the older cards' duel ratings a quarter (they agree with neither: r < 0.1)
    old = old_ratings()
    ban = banlist_signal(cards)
    use, ndecks = usage_signal(cards)
    sig = []
    for name, vals, wt in (('duels', np.array([old.get(c['id'], np.nan) for c in cards], float), 0.25),
                           ('lists', np.array([ban.get(c['password'], 0.0) for c in cards], float), 1.0),
                           ('tournaments', np.array([use.get(c['password'], 0.0) for c in cards], float), 1.0 if ndecks else 0)):
        ok = ~np.isnan(vals)
        if ok.sum() > 50 and wt:
            sig.append((name, (vals - np.nanmean(vals[ok])) / (np.nanstd(vals[ok]) + 1e-9), wt))
    Z = np.array([s for _, s, _ in sig])
    W = np.array([[wt] for _, _, wt in sig]) * ~np.isnan(Z)
    label = np.nansum(np.nan_to_num(Z) * W, 0) / np.maximum(W.sum(0), 1e-9)

    # the text's words and phrases (1-3 grams) with the features above, one ridge regression; scored on cards held out
    def design(tr, te, vec=None):
        vec = vec or TfidfVectorizer(ngram_range=(1, 3), min_df=3, sublinear_tf=True)
        A = vec.fit_transform([texts[i] for i in tr])
        B = vec.transform([texts[i] for i in te])
        return sparse.hstack([A, sparse.csr_matrix(Xn[tr] * 0.35)]).tocsr(), sparse.hstack([B, sparse.csr_matrix(Xn[te] * 0.35)]).tocsr(), vec
    oof = np.zeros(len(cards))
    for tr, te in KFold(5, shuffle=True, random_state=1).split(Xn):
        A, B, _ = design(tr, te)
        oof[te] = Ridge(alpha=3.0).fit(A, label[tr]).predict(B)
    r_oof = np.corrcoef(oof, label)[0, 1]
    allidx = np.arange(len(cards))
    A, _, vec = design(allidx, allidx[:1])
    model = Ridge(alpha=3.0).fit(A, label)
    pred = model.predict(A)
    # the Elo-like scale of the game's card ratings (gYgoCardRating: a plain card about 1500)
    p = (pred - np.median(pred)) / (np.std(pred) + 1e-9)
    power = {c['password']: int(round(1500 + 220 * v)) for c, v in zip(cards, p)}
    json.dump(power, open(os.path.join(ROOT, 'tools/ygo/card_power.json'), 'w'))

    coef = model.coef_
    vocab = vec.get_feature_names_out()
    nt = len(vocab)
    rep = ['Yu-Gi-Oh! card power from text, effects and stats (tools/ygo/text_power.py)', '',
           'signals: ' + ', '.join('%s (%d cards, weight %.2f)' % (n, int((~np.isnan(z)).sum()), wt) for n, z, wt in sig) +
           '; tournament decks read: %d' % ndecks,
           'the text explains cards it never saw with r = %.3f (5-fold, held out)' % r_oof, '']
    for k, (n, z, wt) in enumerate(sig):
        ok = ~np.isnan(z)
        rep.append('  against %s alone: r = %.3f' % (n, np.corrcoef(oof[ok], z[ok])[0, 1]))
    rep += ['', 'the features (weight per standard deviation):']
    for k, v in sorted(zip(names, coef[nt:]), key=lambda x: -abs(x[1]))[:40]:
        rep.append('  %+.3f  %s' % (v * 0.35, k))
    rep += ['', 'the words and phrases that count most:']
    top = np.argsort(-coef[:nt])[:40]
    bot = np.argsort(coef[:nt])[:25]
    rep += ['  %+.3f  %s' % (coef[i], vocab[i]) for i in top] + ['  ...'] + ['  %+.3f  %s' % (coef[i], vocab[i]) for i in bot]
    order = sorted(range(len(cards)), key=lambda i: -pred[i])
    rep += ['', 'strongest 80:'] + ['  %5d  %s' % (power[cards[i]['password']], cards[i]['name']) for i in order[:80]]
    rep += ['', 'weakest 30:'] + ['  %5d  %s' % (power[cards[i]['password']], cards[i]['name']) for i in order[-30:]]
    open(os.path.join(ROOT, 'docs/audit_ygo_text_power.txt'), 'w').write('\n'.join(rep) + '\n')
    print('\n'.join(rep[:9]))


if __name__ == '__main__':
    main()
