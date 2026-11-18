"""The regional sets' attack wordings as effect scripts (src/tcg/tcg_script.inc; docs/TCG_REGIONS.md).

script(text, name, damage) -> a script, an existing effect row (kind, amount, extra[, after]), or None. The text is
taken as printed, with the card's own name as SELF; each family is one way the cards word an effect. Only exact
wordings are matched (the whole text): a text with anything more waits for its own family.
"""
import re

TYPES = ['Fire', 'Grass', 'Lightning', 'Water', 'Fighting', 'Psychic', 'Colorless', 'Darkness', 'Metal']
ANY = 15
STATUS = {'paralyzed': 'par', 'asleep': 'slp', 'confused': 'cnf', 'poisoned': 'psn', 'burned': 'brn'}
ST = r'(paralyzed|asleep|confused|poisoned|burned)'
STS = r'((?:paralyzed|asleep|confused|poisoned|burned)(?:(?:,| and|, and) (?:paralyzed|asleep|confused|poisoned|burned))*)'
TY = r'(' + '|'.join(TYPES) + r')'
N = r'(\d+)'


def statuses(words, prefix='', cond=''):
    out = []
    for w in re.findall(r'paralyzed|asleep|confused|poisoned|burned', words):
        out.append((cond + '.' if cond else '') + prefix + STATUS[w])
    return ' '.join(out)


def tnum(t):
    return TYPES.index(t) if t in TYPES else ANY


def clean(text, name):
    t = text.replace('Pokémon', 'Pokemon').replace('é', 'e').replace('’', "'")
    for n in sorted({name, name.split()[-1], name.replace('Dark ', '').replace('Light ', '')}, key=len, reverse=True):
        if n:
            t = re.sub(r"\b" + re.escape(n) + r"\b", 'SELF', t)
    t = re.sub(r"\b(this Pokemon|this card)\b", 'SELF', t, flags=re.I)
    # the reminders in brackets say nothing the rules don't
    t = re.sub(r"\s*\((?:Don't apply Weakness and Resistance for Benched Pokemon|Don't apply Weakness or Resistance for "
               r"Benched Pokemon|Any other effects that would happen after applying Weakness and Resistance still "
               r"happen|Do the damage before switching the Pokemon|after doing damage|Any other effects of attacks "
               r"still happen|If there are fewer than \d+ cards in your deck, discard all of them|Benching or evolving "
               r"either Pokemon ends this effect)\.?\)", '', t, flags=re.I)
    return ' '.join(t.split())


F = []


def fam(pattern):
    def deco(fn):
        F.append((re.compile(pattern + r'$', re.I), fn))
        return fn
    return deco


# ---------------------------------------------------------------------------------------------- Special Conditions
@fam(r"The Defending Pokemon is now " + STS + r"\.")
def _(m, d):
    return statuses(m.group(1).lower())


@fam(r"Flip a coin\. If heads, the Defending Pokemon is now " + STS + r"\.")
def _(m, d):
    return 'c ' + statuses(m.group(1).lower(), cond='h')


@fam(r"Flip a coin\. If heads, the Defending Pokemon is now " + STS + r"[.;] If tails, the Defending Pokemon is now " + STS + r"\.")
def _(m, d):
    return 'c ' + statuses(m.group(1).lower(), cond='h') + ' ' + statuses(m.group(2).lower(), cond='t')


@fam(r"Flip 2 coins\. If either of them is heads, the Defending Pokemon is now " + STS + r"\.")
def _(m, d):
    return 'c:2 ' + statuses(m.group(1).lower(), cond='h')


@fam(r"Flip 2 coins\. If both (?:of them )?are heads, the Defending Pokemon is now " + STS + r"\.")
def _(m, d):
    return 'c:2 ' + statuses(m.group(1).lower(), cond='ge2')


@fam(r"SELF is now " + STS + r"\.")
def _(m, d):
    return statuses(m.group(1).lower(), prefix='s')


@fam(r"Both the Defending Pokemon and SELF are now " + STS + r"\.")
def _(m, d):
    return statuses(m.group(1).lower()) + ' ' + statuses(m.group(1).lower(), prefix='s')


@fam(r"Flip a coin\. If heads, the Defending Pokemon is now Poisoned\. It now takes 20 Poison damage instead of 10 after each player's turn \(even if it was already Poisoned\)\.")
def _(m, d):
    return 'c h.psn20'


@fam(r"The Defending Pokemon is now Poisoned\. It now takes 20 Poison damage instead of 10 after each player's turn \(even if it was already Poisoned\)\.")
def _(m, d):
    return 'psn20'


@fam(r"Flip a coin\. If heads, the Defending Pokemon is now Paralyzed\. If tails, this attack does nothing(?: \(not even damage\))?\.")
def _(m, d):
    return 'c t.none h.par'


# ------------------------------------------------------------------------------------------------------- coins
@fam(r"Flip a coin\. If heads, this attack does " + N + r" damage plus " + N + r" more damage(?:[.;] If tails, this attack does \d+ damage)?\.")
def _(m, d):
    return f'c h.add:{m.group(2)}'


@fam(r"Flip a coin\. If heads, this attack does " + N + r" damage plus " + N + r" more damage\. If tails, this attack does \d+ damage and the Defending Pokemon is now " + STS + r"\.")
def _(m, d):
    return f'c h.add:{m.group(2)} ' + statuses(m.group(3).lower(), cond='t')


@fam(r"Flip a coin\. If heads, this attack does " + N + r" damage plus " + N + r" more damage and the Defending Pokemon is now " + STS + r"[.;] If tails, this attack does \d+ damage\.")
def _(m, d):
    return f'c h.add:{m.group(2)} ' + statuses(m.group(3).lower(), cond='h')


@fam(r"Flip a coin\. If heads, this attack does " + N + r" damage plus " + N + r" more damage\. If tails, SELF does " + N + r" damage to itself\.")
def _(m, d):
    return f'c h.add:{m.group(2)} t.rec:{m.group(3)}'


@fam(r"Flip " + N + r" coins\. This attack does " + N + r"(?: damage)? times the number of heads\.")
def _(m, d):
    return f'c:{m.group(1)} mulh:{m.group(2)}'


@fam(r"Flip " + N + r" coins\. This attack does " + N + r" damage plus " + N + r" more damage for each heads\.?")
def _(m, d):
    return f'c:{m.group(1)} addh:{m.group(3)}'


@fam(r"Flip a coin until you get tails\. This attack does " + N + r" damage plus " + N + r" more damage for each heads\.")
def _(m, d):
    return f'cu addh:{m.group(2)}'


@fam(r"Flip a coin until you get tails\. This attack does " + N + r" damage times the number of heads\.")
def _(m, d):
    return f'cu mulh:{m.group(1)}'


@fam(r"Flip a number of coins equal to the (?:number|amount) of " + TY + r"? ?Energy(?: cards)? attached to SELF\. This attack does " + N + r" damage plus " + N + r" more damage for each heads\.")
def _(m, d):
    return f'ce:{tnum(m.group(1))} addh:{m.group(3)}'


@fam(r"Flip a number of coins equal to the (?:number|amount) of " + TY + r"? ?Energy(?: cards)? attached to SELF\. This attack does " + N + r" damage times the number of heads\.(?: You can't flip more than " + N + r" coins in this way\.)?")
def _(m, d):
    cap = f',{m.group(3)}' if m.group(3) else ''
    return f'ce:{tnum(m.group(1))}{cap} mulh:{m.group(2)}'


@fam(r"Flip a number of coins equal to the number of damage counters on the Defending Pokemon\. This attack does " + N + r" damage plus " + N + r" more damage for each heads\.")
def _(m, d):
    return f'cdc addh:{m.group(2)}'


@fam(r"Flip a coin\. If tails, this attack does nothing\.")
def _(m, d):
    return 'c t.none'


@fam(r"Flip 2 coins\. If both are heads, this attack does " + N + r" damage plus " + N + r" more damage\. If both are tails, this attack does nothing\. If 1 is heads and 1 is tails, this attack just does \d+ damage\.")
def _(m, d):
    return f'c:2 at.none ah.add:{m.group(2)}'


# ------------------------------------------------------------------------------------------------ damage counts
@fam(r"(?:This attack does|Does) " + N + r" damage times the number of damage counters on SELF\.")
def _(m, d):
    return f'set:0 addsc:{m.group(1)}'


@fam(r"(?:This attack does|Does) " + N + r" damage plus " + N + r" more damage for each damage counter on SELF\.")
def _(m, d):
    return f'addsc:{m.group(2)}'


@fam(r"(?:This attack does|Does) " + N + r" damage plus " + N + r" more damage for each Energy card attached to the Defending Pokemon\.")
def _(m, d):
    return f'addde:{m.group(2)}'


@fam(r"(?:This attack does|Does) " + N + r" damage plus " + N + r" more damage for each " + TY + r"? ?Energy attached to SELF but not used to pay for this attack's Energy (?:c|C)ost\. You can't add more than " + N + r" damage in this way\.")
def _(m, d):
    per, cap = int(m.group(2)), int(m.group(4))
    return f'addpast:{tnum(m.group(3))},{per},{cap // per}'


@fam(r"If the Defending Pokemon is " + ST + r", this attack does " + N + r" damage plus " + N + r" more damage\.")
def _(m, d):
    c = {'poisoned': 'dpsn', 'burned': 'dbrn'}.get(m.group(1).lower())
    return f'{c}.add:{m.group(3)}' if c else None


@fam(r"If the Defending Pokemon already has any damage counters on it, this attack does " + N + r" damage plus " + N + r" more damage\. If not, this attack does \d+ damage\.")
def _(m, d):
    return f'ddmg.add:{m.group(2)}'


# ------------------------------------------------------------------------------------------------------ itself
@fam(r"SELF does " + N + r" damage to itself\.")
def _(m, d):
    return f'rec:{m.group(1)}'


@fam(r"Flip a coin\. If tails, SELF does " + N + r" damage to itself\.")
def _(m, d):
    return f'c t.rec:{m.group(1)}'


@fam(r"Flip a coin\. If tails, put " + N + r" damage counters? on SELF\.")
def _(m, d):
    return f'c t.ctr:{m.group(1)}'


@fam(r"(?:Remove|If there are any damage counters on SELF, remove) " + N + r" (?:damage counters?|of them) from SELF\.")
def _(m, d):
    return f'heal:{m.group(1)}'


@fam(r"Flip a coin\. If heads, remove " + N + r" damage counters? from SELF\.")
def _(m, d):
    return f'c h.heal:{m.group(1)}'


@fam(r"(?:You can't use this attack during your next turn|SELF can't use this attack during your next turn)\.")
def _(m, d):
    return 'lock'


@fam(r"SELF can't attack during your next turn\.")
def _(m, d):
    return 'selflock'


@fam(r"SELF does " + N + r" damage to itself\. SELF can't use this attack during your next turn\.")
def _(m, d):
    return f'rec:{m.group(1)} lock'


@fam(r"Switch SELF with 1 of your Benched Pokemon, if any\.")
def _(m, d):
    return 'ssw'


# ----------------------------------------------------------------------------------------------- the Defending
@fam(r"The Defending Pokemon can't retreat during your opponent's next turn\.")
def _(m, d):
    return 'nort'


@fam(r"Your opponent switches the Defending Pokemon with 1 of his or her Benched Pokemon, if any\.")
def _(m, d):
    return 'osw'


@fam(r"If the Defending Pokemon has any Energy cards attached to it, flip a coin\. If heads, choose 1 of those Energy cards and discard it\.")
def _(m, d):
    return 'c h.dde'


@fam(r"Flip a coin\. If heads, discard 1 Energy card attached to the Defending Pokemon, if any\.")
def _(m, d):
    return 'c h.dde'


@fam(r"Flip a coin\. If heads, your opponent can't play Trainer cards during his or her next turn\.")
def _(m, d):
    return 'c h.notr'


# ----------------------------------------------------------------------------------------------------- Benches
@fam(r"If your opponent has any Benched Pokemon, flip a coin\. If heads, choose 1 of them and this attack does " + N + r" damage to it\.")
def _(m, d):
    return f'c h.b1:{m.group(1)}'


@fam(r"Flip a coin\. If heads, the Defending Pokemon is now " + STS + r" and this attack does " + N + r" damage to each of your opponent's Benched Pokemon\.")
def _(m, d):
    return 'c ' + statuses(m.group(1).lower(), cond='h') + f' h.bo:{m.group(2)}'


@fam(r"(?:Does|This attack does) " + N + r" damage to each Benched Pokemon \(yours and your opponent's\)\.")
def _(m, d):
    return f'bb:{m.group(1)}'


@fam(r"(?:If your opponent has any Benched Pokemon, )?(?:this attack does|does) " + N + r" damage to each of (?:them|your opponent's Benched Pokemon)\.")
def _(m, d):
    return f'bo:{m.group(1)}'


# ----------------------------------------------------------------------------------------------------- Energy
@fam(r"Flip a coin\. If tails, discard (?:a|1) " + TY + r" Energy card attached to SELF\.")
def _(m, d):
    return f'c t.dsa:{tnum(m.group(1))},1'


@fam(r"Discard (?:a|1) " + TY + r" Energy card attached to SELF\.")
def _(m, d):
    return f'dsa:{tnum(m.group(1))},1'


@fam(r"Discard all " + TY + r" Energy cards attached to SELF\.")
def _(m, d):
    return f'dsa:{tnum(m.group(1))},0'


@fam(r"If there are any " + TY + r" Energy cards in your discard pile, flip a coin\. If heads, attach 1 of them to SELF\.")
def _(m, d):
    return f'c h.attd:{tnum(m.group(1))},1'


@fam(r"If there are any " + TY + r" Energy cards in your discard pile, attach 1 of them to SELF\.")
def _(m, d):
    return f'attd:{tnum(m.group(1))},1'


@fam(r"Take up to " + N + r" " + TY + r" Energy cards from your discard pile and attach them to SELF\.")
def _(m, d):
    return f'attd:{tnum(m.group(2))},{m.group(1)}'


# ------------------------------------------------------------------------------------------------- batch 2
@fam(r"Flip a coin\. If heads, during your opponent's next turn, prevent all effects of attacks, including damage, done to SELF\.")
def _(m, d):
    return 'c h.bar:2'


@fam(r"Flip a coin\. If heads, prevent all damage done (?:by attacks )?to SELF during your opponent's next turn\.")
def _(m, d):
    return 'c h.bar:1'


@fam(r"Flip a coin\. If heads, during your opponent's next turn, prevent all effects of attacks, including damage, done to SELF; if tails, this attack does nothing(?: \(not even damage\))?\.")
def _(m, d):
    return 'c t.none h.bar:2'


@fam(r"If the Defending Pokemon tries to attack during your opponent's next turn, your opponent flips a coin\. If tails, that attack does nothing\.")
def _(m, d):
    return ('WEAKEN', 3, 0, 2)


@fam(r"Don't apply Resistance\.")
def _(m, d):
    return 'nores'


@fam(r"Don't apply Weakness and Resistance(?: for this attack)?\.")
def _(m, d):
    return 'nowr'


@fam(r"This attack's damage isn't affected by Weakness, Resistance, (?:Pokemon Powers|Poke-Powers, Poke-Bodies), or any other effects on the Defending Pokemon\.")
def _(m, d):
    return ('SWIFT', 0, 0, 2)


@fam(r"If your opponent has any Benched Pokemon, choose 1 of them and this attack does " + N + r" damage to it\.")
def _(m, d):
    return f'b1:{m.group(1)}'


@fam(r"If your opponent has any Benched Pokemon, he or she chooses 1 of them and switches (?:it with the Defending Pokemon|it with his or her Active Pokemon|the Defending Pokemon with it)\.")
def _(m, d):
    return 'osw'


@fam(r"If your opponent has any Benched Pokemon, he or she chooses 1 of them and switches it with his or her Active Pokemon, then, if you have any Benched Pokemon, you switch 1 of them with your Active Pokemon\.")
def _(m, d):
    return 'osw ssw'


@fam(r"(?:Does|This attack does) " + N + r" damage to each of your own Benched Pokemon\.")
def _(m, d):
    return f'bm:{m.group(1)}'


@fam(r"Until the end of your next turn, if an attack damages the Defending Pokemon \(after applying Weakness and Resistance\), that attack does " + N + r" more damage to the Defending Pokemon\.")
def _(m, d):
    return f'vuln:{m.group(1)}'


@fam(r"Flip a coin\. If heads, the Defending Pokemon can't attack during your opponent's next turn\.")
def _(m, d):
    return 'c h.noat'


@fam(r"Flip " + N + r" coins\. This attack does " + N + r" damage times the number of heads\. SELF is now Confused\.")
def _(m, d):
    return f'c:{m.group(1)} mulh:{m.group(2)} scnf'


@fam(r"Remove a number of damage counters from SELF equal to half the damage done to the Defending Pokemon \(after applying Weakness and Resistance\) \(rounded up to the nearest 10\)\. If SELF has fewer damage counters than that, remove all of them\.")
def _(m, d):
    return ('DRAIN_DONE', 2, 0, 2)


@fam(r"Discard all " + TY + r" Energy cards attached to SELF or this attack does nothing\.")
def _(m, d):
    return f'dsx:{tnum(m.group(1))},0'


@fam(r"Discard (?:a|1) " + TY + r" Energy card attached to SELF or this attack does nothing\.")
def _(m, d):
    return f'dsx:{tnum(m.group(1))},1'


@fam(r"Discard (?:a|1) " + TY + r" Energy card attached to SELF in order to use this attack\.")
def _(m, d):
    return f'dse:{tnum(m.group(1))},1'


@fam(r"Discard all Energy cards attached to SELF in order to use this attack\.")
def _(m, d):
    return 'dse:15,0'


@fam(r"Discard (?:an|1) Energy card attached to SELF\.")
def _(m, d):
    return 'dsa:15,1'


@fam(r"If SELF and the Defending Pokemon don't have the same number of Energy cards attached to them, this attack's base damage is " + N + r" instead of " + N + r"\.")
def _(m, d):
    return f'neqe.set:{m.group(1)}'


@fam(r"Search your deck for a basic Energy card and attach it to SELF\. Shuffle your deck afterward\.")
def _(m, d):
    return 'srchp:15'


@fam(r"Search your deck for (?:a|an) " + TY + r" Energy card and attach it to SELF\. Shuffle your deck afterward\.")
def _(m, d):
    return f'srch:{tnum(m.group(1))},1'


@fam(r"If your opponent has any Benched Pokemon, choose up to " + N + r" of them\. This attack does " + N + r" damage to each of them\.")
def _(m, d):
    return ('GIGASHOCK', int(m.group(2)), int(m.group(1)), 2)


@fam(r"If your opponent has any Benched Pokemon, choose 1 of them and flip a coin\. If heads, this attack does " + N + r" damage to that Pokemon\.")
def _(m, d):
    return f'c h.b1:{m.group(1)}'


@fam(r"If your opponent has any Benched Pokemon, choose 1 of them and switch it with the Defending Pokemon\.")
def _(m, d):
    return 'lure'


@fam(r"Flip a coin\. If heads, the Defending Pokemon is now " + STS + r"\. If tails, SELF does " + N + r" damage to itself\.")
def _(m, d):
    return 'c ' + statuses(m.group(1).lower(), cond='h') + f' t.rec:{m.group(2)}'


@fam(r"Flip a coin\. If tails, discard all " + TY + r" Energy cards attached to SELF\.")
def _(m, d):
    return f'c t.dsa:{tnum(m.group(1))},0'


@fam(r"All damage done (?:by attacks )?to SELF during your opponent's next turn is reduced by (10|20) \(after applying Weakness and Resistance\)\.")
def _(m, d):
    return 'guard:' + ('1' if m.group(1) == '10' else '2')


@fam(r"During your opponent's next turn, any damage done to SELF by attacks is reduced by (10|20)\.")
def _(m, d):
    return 'guard:' + ('1' if m.group(1) == '10' else '2')


@fam(r"Choose 1 of the Defending Pokemon's attacks\. That Pokemon can't use that attack during your opponent's next turn\.")
def _(m, d):
    return ('AMNESIA', 0, 0, 2)


@fam(r"Flip a number of coins equal to the number of damage counters on SELF\. This attack does " + N + r" damage plus " + N + r" more damage for each heads\.")
def _(m, d):
    return f'csc addh:{m.group(2)}'


@fam(r"Discard 1 " + TY + r" Energy card attached to SELF in order to use this attack\.")
def _(m, d):
    return f'dse:{tnum(m.group(1))},1'


@fam(r"If there are any " + TY + r" Energy cards in your discard pile, choose 1 of them and attach it to SELF\.")
def _(m, d):
    return f'attd:{tnum(m.group(1))},1'


@fam(r"Flip 2 coins\. If both are heads, the Defending Pokemon is now " + STS + r"\. If either of them is tails, this attack does nothing(?: \(not even damage\))?\.")
def _(m, d):
    return 'c:2 t.none ' + statuses(m.group(1).lower())


@fam(r"Flip 2 coins\. If either of them is tails, this attack does nothing\.")
def _(m, d):
    return 'c:2 t.none'


@fam(r"Choose 1 of your opponent's Pokemon\. This attack does " + N + r" damage to that Pokemon\. Don't apply Weakness and Resistance for this attack\.")
def _(m, d):
    return f'any1:{m.group(1)}'


@fam(r"Flip a coin\. If heads, choose 1 of your opponent's Pokemon\. This attack does " + N + r" damage to that Pokemon\. Don't apply Weakness and Resistance\.")
def _(m, d):
    return f'c h.any1:{m.group(1)}'


@fam(r"Discard all " + TY + r" Energy cards attached to SELF\. This attack does " + N + r" damage plus " + N + r" more damage for each card discarded this way\.")
def _(m, d):
    return f'adde:{tnum(m.group(1))},{m.group(3)} dse:{tnum(m.group(1))},0'


@fam(r"Flip a coin\. If heads, this attack does " + N + r" damage plus " + N + r" more damage and does " + N + r" damage to each of your opponent's Benched Pokemon, if any\.")
def _(m, d):
    return f'c h.add:{m.group(2)} h.bo:{m.group(3)}'


@fam(r"This attack does " + N + r" damage to each of your opponent's Benched Pokemon\. Then, flip a coin\. If tails, this attack can't be used during your next turn\.")
def _(m, d):
    return f'bo:{m.group(1)} c t.lock'


@fam(r"Flip a coin\. If heads, this attack does " + N + r" damage to each of your opponent's Benched Pokemon\. If tails, this attack does " + N + r" damage to each of your Pokemon in play\.")
def _(m, d):
    return f'c h.bo:{m.group(1)} t.bm:{m.group(2)} t.rec:{m.group(2)}'


@fam(r"Flip 2 coins\. This attack does " + N + r" damage times the number of heads\. If both of them are tails, the Defending Pokemon is now " + STS + r"\.")
def _(m, d):
    return f'c:2 mulh:{m.group(1)} ' + statuses(m.group(2).lower(), cond='at')


@fam(r"Flip 2 coins\. This attack does " + N + r" damage times the number of heads\. If both coins are heads, switch SELF with 1 of your Benched Pokemon\.")
def _(m, d):
    return f'c:2 mulh:{m.group(1)} ah.ssw'


@fam(r"If SELF has " + N + r" or more damage counters on it, this attack's base damage is " + N + r"\.")
def _(m, d):
    return f'sc{m.group(1)}.set:{m.group(2)}'


@fam(r"If (?:here|there) are no damage counters on the Defending Pokemon, this attack's base damage is " + N + r" instead of " + N + r"\.")
def _(m, d):
    return f'dnodmg.set:{m.group(1)}'


@fam(r"Each player discards the top " + N + r" cards from his or her deck\.")
def _(m, d):
    return f'mill:{m.group(1)}'


@fam(r"Flip a coin\. If heads, your opponent can't play Supporter cards during his or her next turn\.")
def _(m, d):
    return 'c h.nosup'


def script(text, name, damage):
    t = clean(text, name)
    for rx, fn in F:
        m = rx.match(t)
        if m:
            out = fn(m, damage)
            if out:
                return out if isinstance(out, tuple) else ('SCRIPT', out)
    return None
