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


# One wording, one script (the cards' words with SELF for the card's name; tcg_script.inc's ops)
EXACT = {
    'Take a basic Energy card attached to 1 of your Pokemon and attach it to another of your Pokemon.': 'emove:3',
    'If you have any Benched Pokemon and if there are any basic Energy cards attached to SELF, take 1 of those Energy cards and attach it to 1 of those Pokemon.': 'eto:15,1',
    'Flip a coin. If heads, choose a Special Condition (Asleep, Burned, Confused, Paralyzed, or Poisoned). The Defending Pokemon is now affected by that Special Condition.': 'opt:1 c h.chosc:1',
    'Flip a coin. If heads, the Defending Pokemon is now Asleep, Confused, Paralyzed, or Poisoned (your choice).': 'opt:2 c h.chosc:2',
    'Flip a coin. If heads, the Defending Pokemon is now either Asleep, Confused, Paralyzed, or Poisoned (your choice).': 'opt:2 c h.chosc:2',
    "Look at the top 3 cards of either player's deck and rearrange them as you like.": 'opt:3 order3',
    "For each Benched Pokemon in play (yours and your opponent's), flip a coin. If heads, this attack does 30 damage to that Pokemon.": 'bbc:30',
    "For each Benched Pokemon (yours and your opponent's), flip a coin. If heads, this attack does 20 damage to that Pokemon.": 'bbc:20',
    "You may discard all Lightning Energy cards attached to SELF. If you do, this attack's base damage is 80 instead of 40.": 'opt:0,2 o1.set:80 o1.dsa:2,0',
    "Choose 1 of your opponent's Pokemon. Count the number of Energy cards attached to that Pokemon. Put that many damage counters on the Pokemon.": 'zero ectr',
    'You may discard any number of Fire Energy cards attached to SELF when you use this attack. If you do, flip a number of coins equal to the number of Fire Energy cards you discarded. This attack does 30 damage plus 30 more damage for each heads.': 'opt:6,0 cop addh:30 dso:0',
    'You may discard any number of Fire Energy cards attached to SELF when you use this attack. If you do, this attack does 40 damage plus 20 more damage for each Fire Energy card you discarded in this way.': 'opt:6,0 addop:20 dso:0',
    'You may flip a coin. If heads, discard a Fire Energy card attached to SELF and the Defending Pokemon is now Burned.': 'opt:0,0 o1.c h.dsa:0,1 h.brn',
    "Choose 1 of your opponent's Benched Pokemon. This attack does 10 damage times the amount of Grass Energy attached to SELF.": 'zero b1e:1,10',
    "If the Defending Pokemon isn't Knocked Out by the damage from this attack, you may choose 1 of your opponent's Benched Pokemon and switch the Defending Pokemon with it.": 'luremay',
    "This attack does 10 damage to each of your opponent's Benched Pokemon that are the same type (color) as the Defending Pokemon.": 'botd:10',
    "This attack does 40 damage plus 10 more damage for each Colorless Energy in the Defending Pokemon's Retreat Cost. Damage is calculated using the Defending Pokemon's Retreat Cost after applying effects to the Retreat Cost.": 'addrc:10',
    'Discard an Energy card attached to SELF in order to use this attack. If the discarded card is a Fire Energy card, this attack does 40 damage plus 10 more damage.': 'addpk:0,10 dse:15,1',
    "Flip 2 coins. For each heads, do 10 damage to each of your opponent's Benched Pokemon. For each tails, do 10 damage to each of your own Benched Pokemon.": 'c:2 h.bo:10 ah.bo:10 t.bm:10 at.bm:10',
    "This attack does 30 damage plus 20 damage times the number of your opponent's Benched Pokemon minus the number of your Benched Pokemon. (For example, if your opponent has 3 Benched Pokemon and you have 1, this attack will do 30 damage plus 40 more damage.)": 'addbd:20,0',
    "This attack does 30 damage plus 10 damage times the number of your Benched Pokemon minus the number of your opponent's Benched Pokemon. (For example, if your opponent has 1 Benched Pokemon and you have 3, this attack will do 30 damage plus 20 more damage.)": 'addbd:10,1',
    'Each player discard the top card of his or her deck. This attack does 20 damage plus 20 more damage for each Fire Energy card discarded in this way.': 'milladdb:1,0,20',
    'You may discard a Fire or Fighting basic Energy card attached to SELF. If you discard a Fire Energy card in this way, the Defending Pokemon is now Burned. If you discard a Fighting Energy card in this way, this attack does 40 damage plus 20 more damage.': 'opt:5 o1.dsa:0,1 o1.brn o2.add:20 o2.dsa:4,1',
    'You may flip a coin. If heads, discard all Lightning Energy cards attached to SELF. This attack does 40 damage plus 20 more damage for each Energy card discarded in this way.': 'opt:0 o1.c h.adde:2,20 h.dsa:2,0',
    'Before doing damage, you may flip a coin. If heads, this attack does 80 damage. If tails, this attack does nothing.': 'opt:0 o1.c h.set:80 t.none',
    'Discard all Lightning Energy cards attached to SELF in order to use this attack. Flip a number of coins equal to the number of Lightning Energy cards you discarded. This attack does 30 damage times the number of heads.': 'ce:2 mulh:30 dse:2,0',
    'Discard all Lightning Energy cards attached to SELF in order to use this attack. Flip a number of coins equal to the number of Lightning Energy cards you discarded. This attack does 30 damage times then number of heads.': 'ce:2 mulh:30 dse:2,0',
    "Shuffle your hand into your deck, then draw 5 cards.": 'handdeck draw:5',
    "If there are any damage counters on SELF, remove 1 of them.": 'heal:1',
    "Flip a coin. If heads, put a basic Energy card from your discard pile into your hand.": 'c h.dish:15,1',
    "Look at your opponent's hand.": 'nop',
    "If this attack damages the Defending Pokemon (after applying Weakness and Resistance), remove 1 damage counter from SELF, if it has any.": 'ddmg.heal:1',
    "Choose 1 of your opponent's Pokemon. This attack does 10 damage to that Pokemon. Don't apply Weakness and Resistance.": 'zero any1:10',
    "Choose up to 2 Energy cards from your discard pile and attach them to SELF.": 'attd:15,2',
    "Does 10 damage times the number of Energy cards attached to the Defending Pokemon.": 'zero addde:10',
    "Does 10 damage times the amount of Energy attached to the Defending Pokemon.": 'zero addde:10',
    "Flip a coin. If heads, this attack does 10 damage to each of your opponent's Benched Pokemon. If tails, this attack does 10 damage to each of your own Benched Pokemon.": 'c h.bo:10 t.bm:10',
    "Flip a coin. If heads, this attack does 10 damage to each of your opponent's Benched Pokemon; if tails, this attack does 10 damage to each of your own Benched Pokemon.": 'c h.bo:10 t.bm:10',
    "Flip 2 coins. If both of them are heads, this attack does 30 damage plus 60 more damage. If 1 or both of them are tails, this attack does 30 damage.": 'c:2 ah.add:60',
    "Flip 2 coins. If 1 or both of them are heads, the Defending Pokemon is now Confused.": 'c:2 h.cnf',
    "Choose 1 of your opponent's Benched Pokemon and flip a coin. If heads, this attack does 30 damage to that Pokemon. This attack can't be used if your opponent has no Benched Pokemon.": 'needob c h.b1:30',
    "Does 10 damage to each of your opponent's Pokemon. Don't apply Weakness and Resistance.": 'zero oall:10',
    "Flip a coin. If tails, this attack does nothing to the Defending Pokemon and SELF does 20 damage to itself.": 'c t.zero t.rec:20',
    "Flip a coin. If heads, remove 4 damage counters from SELF. If SELF has fewer damage counters than that, remove all of them.": 'c h.heal:4',
    "Before doing damage, choose 1 of your opponent's Benched Pokemon and switch it with his or her Active Pokemon. This attack can't be used if your opponent has no Benched Pokemon.": 'needob drag',
    "Does damage to the Defending Pokemon equal to half the Defending Pokemon's remaining HP (rounded up to the nearest 10).": 'halfhp:1',
    "Does damage equal to half the Defending Pokemon's remaining HP (rounded down to the nearest 10).": 'halfhp:0',
    "Flip a number of coins equal to the number of Water Energy attached to SELF. This attack does 30 damage plus 10 damage for each heads.": 'ce:3 addh:10',
    "If the Defending Pokemon was Asleep, it is no longer Asleep.": 'wake',
    "Flip a coin. If heads, remove 3 damage counters from SELF and SELF is now Asleep. If SELF has fewer damage counters than that, remove all of them.": 'c h.heal:3 h.sslp',
    "Discard 1 Energy card attached to SELF in order to use this attack. If the Defending Pokemon has any Energy cards attached to it, choose 1 of them and discard it.": 'dse:15,1 dde',
    "Flip 4 coins. This attack does 20 damage times the number of heads to the Defending Pokemon and 20 damage times the number of tails to SELF.": 'c:4 mulh:20 rect:20',
    "Don't apply Resistance for this attack.": 'nores',
    "Shuffle your deck.": 'shuf',
    "Does 40 damage minus 10 damage for each damage counter on SELF.": 'subsc:10',
    "Flip 2 coins. If 1 or both of them are tails, this attack does nothing.": 'c:2 nah.none',
    "If SELF has any damage counters on it, flip a coin. If heads, remove 1 of those damage counters.": 'c h.heal:1',
    "Take 1 Lightning Energy card from your discard pile and attach it to SELF.": 'attd:2,1',
    "Flip 1 coin. This attack does 20 damage times the number of heads.": 'c mulh:20',
    "Discard 3 Fire Energy cards attached to SELF in order to use this attack.": 'dse:0,3',
    "Flip a coin. If heads, choose 1 of your opponent's Pokemon. This attack does 40 damage to that Pokemon. Don't apply Weakness and Resistance for this attack.": 'c zero h.any1:40',
    "Flip a coin. If tails, discard all Energy cards attached to SELF.": 'c t.dsa:15,0',
    "SELF does damage to itself equal to 10 times the number of Lightning Energy cards attached to it.": 'rece:2,10',
    "All Pokemon Powers stop working until the end of your opponent's next turn.": 'nopow:1',
    "All Pokemon Powers stop working until the end of your next turn.": 'nopow:2',
    "Flip a coin. If tails, this attack does no damage to the Defending Pokemon and SELF does 100 damage to itself.": 'c t.zero t.rec:100',
    "SELF does 20 damage to itself. Flip a coin. If heads, the Defending Pokemon is now Poisoned.": 'rec:20 c h.psn',
    "Flip a coin. If heads, this attack does 10 damage plus 30 more damage and SELF does 30 damage to itself; if tails, this attack does 10 damage.": 'c h.add:30 h.rec:30',
    "If the Defending Pokemon is Fighting, this attack's base damage is doubled.": 'dt4.dbl',
    "Flip a coin. If heads, and if your opponent has any Benched Pokemon, choose 1 of them and this attack does 30 damage to it.": 'c h.b1:30',
    "Flip a coin. If heads and if your opponent has any Benched Pokemon, choose 1 of them and this attack does 20 damage to it.": 'c h.b1:20',
    "Flip a number of coins equal to the number of damage counters on the Defending Pokemon. This attack does 20 damage times the number of heads.": 'cdc mulh:20',
    "Flip a coin. If heads, the Defending Pokemon can't retreat during your opponent's next turn.": 'c h.nort',
    "Choose 1 of your opponent's Pokemon and flip 3 coins. This attack does 10 damage times the number of heads to that Pokemon. Don't apply Weakness and Resistance for this attack.": 'zero c:3 any1h:10',
    "You and your opponent remove 1 damage counter from each of your Pokemon with damage counters on them.": 'healall:1 ohealall:1',
    "If the Defending Pokemon has no damage counters on it, this attack does 40 damage. If it has any damage counters on it, this attack does 10 damage.": 'dnodmg.set:40 ddmg.set:10',
    "Flip a coin. If heads, this attack does 20 damage times the number of damage counters on SELF.": 'c zero h.addsc:20',
    "You can't use this attack unless SELF has 2 or more damage counters on it.": 'needsc:2',
    "Flip a coin. If heads, this attack does 10 damage to each Benched Pokemon (including your own).": 'c h.bb:10',
    "Flip a coin. If heads, draw 2 cards.": 'c h.draw:2',
    "Flip a coin. If tails, you can't use this attack during your next turn.": 'c t.lock',
    "Search your deck for a Psychic Energy card. Show that card to your opponent, then put it into your hand. Shuffle your deck afterward.": 'srchh:5,1',
    "Flip a coin. If heads, the Defending Pokemon is now Confused. If tails, this attack does no damage and SELF is now Confused.": 'c h.cnf t.zero t.scnf',
    "Discard a Fire Energy card, a Water Energy card, and a Lightning Energy card attached to SELF in order to use this attack.": 'dse:0,1 dse:3,1 dse:2,1',
    "Does 20 damage to each Pokemon in play that has a Pokemon Power. Don't apply Weakness and Resistance.": 'zero powdmg:20',
    "This attack does 20 damage to each Pokemon with a Poke-Body or Poke-Power (yours and your opponent's). (Don't apply Weakness or Resistance.)": 'zero powdmg:20',
    "Flip a coin. If heads, all damage done by attacks to SELF during your opponent's next turn is reduced by 20 (after applying Weakness and Resistance).": 'c h.guard:2',
    "Flip a coin. If heads, this attack does 60 damage plus 20 more damage and does 20 damage to SELF. If tails, this attack does 60 damage.": 'c h.add:20 h.rec:20',
    "Discard 3 Fire Energy cards attached to SELF in order to use this attack. Do 20 damage to each Benched Pokemon (yours and your opponent's).": 'dse:0,3 bb:20',
    "Flip a coin for each of your Pokemon in play (including this one). This attack does 20 damage times the number of heads.": 'cmine mulh:20',
    "Discard the top 3 cards from your deck.": 'smill:3',
    "This attack does 10 damage plus 10 more damage for each damage counter on SELF. Then, SELF does 20 damage to itself.": 'addsc:10 rec:20',
    "Flip 2 coins. Remove 2 damage counters times the number of heads from SELF. If it has fewer damage counters than that, remove all of them.": 'c:2 healh:2',
    "Flip 3 coins. For each heads, remove 1 damage counter from each of your Pokemon. If a Pokemon has fewer damage counters than the number of heads, remove all of them.": 'c:3 healallh:1',
    "Flip a coin. If tails, discard 1 Energy card attached to SELF.": 'c t.dsa:15,1',
    "Flip a coin. If heads, until the end of your opponent's next turn, the Defending Pokemon can't attack or retreat.": 'c h.noat h.nort',
    "Flip 4 coins. This attack does 10 damage plus 10 more damage if exactly 1 is heads, or 20 more damage if exactly 2 are heads, or 40 more damage if exactly 3 are heads, or 80 more damage if all 4 are heads.": 'c:4 eq1.add:10 eq2.add:20 eq3.add:40 eq4.add:80',
    "Flip 4 coins. If you get 1 heads, this attack does 10 damage plus 10 more damage. If you get 2 heads, this attack does 10 damage plus 20 more damage. If you get 3 heads, this attack does 10 damage plus 50 more damage. If you get 4 heads, this attack does 10 damage plus 90 more damage.": 'c:4 eq1.add:10 eq2.add:20 eq3.add:50 eq4.add:90',
    "Flip a coin. If heads, the Defending Pokemon is now Confused": 'c h.cnf',
    "Remove all damage counters from SELF. SELF is now Asleep.": 'heal:30 sslp',
    "Flip a coin. If heads, the Defending Pokemon is now Poisoned. If tails, this attack does nothing (not even damage).": 'c t.none psn',
    "Flip 3 coins. For each heads, put 1 damage counter on the Defending Pokemon.": 'c:3 dctr:1',
    "Flip 4 coins. This attack does 20 damage times the number of heads. If you get 2 or more heads, the Defending Pokemon is now Confused.": 'c:4 mulh:20 ge2.cnf',
    "Discard 2 Fire Energy cards attached to SELF or this attack does nothing.": 'dsx:0,2',
    "Flip 2 coins. If both are heads, the Defending Pokemon is now Paralyzed. If only 1 is heads, the Defending Pokemon is now Asleep.": 'c:2 ah.par eq1.slp',
    "Flip a coin. If heads, during your opponent's next turn, prevent all effects of attacks, including damage, done to SELF. If tails, this attack does nothing (not even damage).": 'c t.none bar:2',
    "Flip 2 coins. For each tails, discard 1 Fire Energy card from SELF, if it has any.": 'c:2 dsat:0',
    "Flip a coin. If heads, discard all Trainer cards attached to your opponent's Pokemon.": 'c h.otrdisc',
    "Flip a coin. If heads, this attack does 10 damage to each of your opponent's Benched Pokemon.": 'c h.bo:10',
    "Flip 2 coins. If 1 of them is tails, discard 2 Energy cards attached to SELF. If both are tails, discard all Energy cards attached to SELF.": 'c:2 eq1.dsa:15,2 at.dsa:15,0',
    "Flip a coin. If heads, this attack does 20 damage plus 10 more damage and the Defending Pokemon is now Paralyzed.": 'c h.add:10 h.par',
    "Attach a basic Energy card from your discard pile to 1 of your Pokemon.": 'attd:15,1',
    "Search your deck for a basic Energy card. Show that card to your opponent, then put it into your hand. Shuffle your deck afterward.": 'srchh:15,1',
    "Flip a coin. If heads, this attack does 10 damage to each of your opponent's Pokemon. Don't apply Weakness and Resistance. Then, if your opponent has any Benched Pokemon, he or she chooses 1 of them and switches it with the Defending Pokemon.": 'c zero h.oall:10 osw',
    "Count the number of Pokemon you have in play with damage counters on them. Flip a coin. If heads, this attack does 50 damage plus 10 more damage for each of those Pokemon.": 'c t.none h.addmd:10',
    "Does 10 damage to each Pokemon that isn't Water on each player's Bench.": 'zero bnt:3,10',
    "Does 10 damage to each Grass Pokemon on your opponent's Bench.": 'zero bot:1,10',
    "Discard the top 5 cards of your deck. This attack does 20 damage for each Fire Energy card you discarded in this way.": 'zero milladd:5,0,20',
    "Flip a coin. If heads, the Defending Pokemon can't attack SELF during your opponent's next turn.": 'c h.noat',
    "This attack can't be used unless SELF and the Defending Pokemon have the same number of Energy cards attached to them.": 'needeqe',
    "If the Defending Pokemon attacks SELF during your opponent's next turn, any damage done to SELF is reduced by 10 (before applying Weakness and Resistance).": 'guard:1',
    "This attack does 30 damage plus 20 more damage for each Special Energy card attached to the Defending Pokemon.": 'adds:20',
    "The Defending Pokemon can't use any Poke-Powers until the end of your opponent's next turn.": 'dpowoff',
    "This attack does 20 damage plus 10 damage for each Lightning Energy attached to SELF. Flip a coin. If tails, SELF does 10 damage to itself for each Lightning Energy attached to it.": 'adde:2,10 c t.rece:2,10',
    "Flip a number of coins equal to the amount of Grass Energy attached to SELF. If any of them are heads, the Defending Pokemon is now Poisoned and Burned.": 'ce:1 h.psn h.brn',
    "Look at your opponent's hand. This attack does 10 damage times the number of Energy cards there.": 'zero addoh:10',
    "This attack does 10 damage to each Benched Pokemon with at least 1 Energy card attached to it (yours and your opponent's).": 'bbe:10',
    "Before doing damage, if your opponent has any Benched Pokemon, choose 1 of them and switch it with the Defending Pokemon. If your opponent has no Benched Pokemon, ignore this effect.": 'drag',
    "Discard a Fire Energy card attached to SELF in order to use this attack. If your opponent has any Benched Pokemon, this attack does 10 damage to each of them.": 'dse:0,1 bo:10',
    "Flip a coin. If heads, this attack does 10 damage times the number of Energy attached to SELF and the Defending Pokemon.": 'c zero h.adde:15,10 h.addde:10',
    "Draw a card. Then, switch SELF with 1 of your Benched Pokemon. You can't use this attack if your Bench is empty.": 'needmb draw:1 ssw',
    "Don't apply Resistance for this attack. (Any other effects that would happen after applying Resistance still happen.)": 'nores',
    "Return a Psychic Energy card attached to SELF to your hand in order to use this attack.": 'eret:5',
    "Flip 2 coins. If both of them are heads, this attack does 20 damage to each other Pokemon (even your own). Don't apply Weakness and Resistance for this attack.": 'c:2 zero ah.oall:20 ah.bm:20',
    "Flip a coin. If heads, this attack does 20 damage plus 30 more damage to the Defending Pokemon and discard all Lightning Energy cards attached to SELF. If tails, this attack does 20 damage.": 'c h.add:30 h.dsa:2,0',
    "If SELF is Poisoned, this attack's base damage is 40 instead of 20 and the Defending Pokemon is now Poisoned.": 'spsn.set:40 spsn.psn',
    "Does 30 damage plus 10 damage times the number of damage counters on SELF, then flip a coin. If tails, SELF does 30 damage to itself.": 'addsc:10 c t.rec:30',
    "Does 10 damage to each Pokemon on each player's bench. SELF does 50 damage to itself. If there is a Stadium card in play, discard it.": 'bb:10 rec:50 nostad',
    "SELF does 20 damage to itself. If there is a Stadium card in play, discard it.": 'rec:20 nostad',
    "Choose 1 of your opponent's Benched Pokemon, and this attack does 10 damage to it.": 'b1:10',
    "Look at a random card from your opponent's hand. Your opponent shuffles that card into his or her deck.": 'ohrand',
    "Flip a coin. If heads, put a number of damage counters on the Defending Pokemon so that its remaining HP are 10.": 'c h.to10',
    "Use this attack only if there are any Fire Energy cards attached to SELF. Flip a coin. If tails, discard 1 of those cards.": 'neede:0 c t.dsa:0,1',
    "If you have any Benched Pokemon, flip a coin. If heads, switch SELF with 1 of your Benched Pokemon.": 'c h.ssw',
    "Flip 3 coins. For each heads, if you have a Fire Energy card in your discard pile, put it into your hand.": 'c:3 dishh:0',
    "Shuffle your hand into your deck, then draw 7 cards.": 'handdeck draw:7',
    "Until the end of your next turn, if an attack damages the Defending Pokemon (after applying Weakness and Resistance), that attack does 20 more damage to the Defending Pokemon": 'vuln:20',
    "Take 1 Lightning Energy card attached to SELF and attach it to 1 of your Benched Pokemon. If you have no Benched Pokemon, discard that Energy card.": 'eto:2',
    "Take 1 Lightning Energy cards attached to SELF and attach it to 1 of your Benched Pokemon. If you have no Benched Pokemon, discard that Energy card.": 'eto:2',
    "If your opponent has any Benched Pokemon, choose 1 of them. Flip 2 coins. For each heads, this attack does 10 damage to that Pokemon. (Don't apply Weakness and Resistance.)": 'c:2 b1h:10',
    "Search your deck for a Basic Pokemon named SELF and put it onto your Bench. Shuffle your deck afterward. (You can't use this attack if your Bench is full.)": 'needroom benchself',
    "Search your deck for a Basic Pokemon card named SELF and put it onto your Bench. Shuffle your deck afterward. (You can't use this attack if your Bench is full.)": 'needroom benchself',
    "Flip a coin. If heads and if your opponent has any Benched Pokemon, he of she chooses 1 of them and switches it with the Defending Pokemon.": 'c h.osw',
    "Discard 1 Water Energy card attached to SELF or this attack does nothing. Remove all damage counters from SELF.": 'dsx:3,1 heal:30',
    "Before doing damage, discard all Trainer cards attached to the Defending Pokemon (before they affect the damage).": 'dtrdisc',
    "Put 3 damage counters on SELF. If this doesn't Knock Out SELF, search your deck for 2 Basic Energy cards and attach them to SELF. Shuffle your deck afterward.": 'ctr:3 srch:15,2',
    "Does 10 damage to each non-Fighting Pokemon in play. Don't apply Weakness and Resistance.": 'zero allnt:4,10',
    "Remove 3 damage counters from SELF. If it has fewer damage counters than that, remove all of them.": 'heal:3',
    "Flip 2 coins. If both are heads, this attack does 20 damage plus 40 more damage. If 1 or both of them are tails, this attack does 20 damage.": 'c:2 ah.add:40',
    "If the Defending Pokemon has any Energy cards attached to it, choose 1 of them and discard it.": 'dde',
    "Flip a coin until you get tails. This attack does 20 damage times the number of heads you get.": 'cu mulh:20',
    "Flip a coin. If heads, this attack does 10 damage plus 10 damage times the number of damage counters on SELF. If tails, this attack does 10 damage.": 'c h.addsc:10',
    "Flip a coin. If heads, this attack does 30 damage plus 10 more damage for each damage counter on SELF. If tails, this attack does nothing.": 'c t.none h.addsc:10',
    "If the Defending Pokemon has any Energy cards attached to it, flip a coin. If heads, choose 1 of those cards and discard it.": 'c h.dde',
    "If your opponent has any Benched Pokemon, this attack does 30 damage instead of 50 and choose 1 of those Benched Pokemon. This attack does 30 damage to that Pokemon.": 'ob.set:30 b1:30',
    "If the Defending Pokemon is a Baby Pokemon, this attack does 10 damage instead of 50. If the Defending Pokemon is a Basic Pokemon, this attack does 30 damage instead of 50.": 'dbasic.set:30 dbaby.set:10',
    "Does 10 damage to each of your opponent's Pokemon for each Energy card attached to that Pokemon. Don't apply Weakness and Resistance.": 'zero oalle:10',
    "Your opponent may draw 2 cards. Either way, you may draw 2 cards.": 'odraw:2 draw:2',
    "This attack can't be used during your next turn. (Benching SELF ends this effect.)": 'lock',
}


def script(text, name, damage):
    t = clean(text, name)
    if t in EXACT:
        return ('SCRIPT', EXACT[t])
    for rx, fn in F:
        m = rx.match(t)
        if m:
            out = fn(m, damage)
            if out:
                return out if isinstance(out, tuple) else ('SCRIPT', out)
    return None
