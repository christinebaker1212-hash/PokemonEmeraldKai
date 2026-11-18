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
    "Flip 3 coins. If exactly 1 is heads, draw a card. If exactly 2 are heads, this attack does 20 damage. If all 3 are heads, choose 1 of the Defending Pokemon's attacks. SELF copies that attack except for its Energy costs. (No matter what type the Defending Pokemon is, SELF's type is still Water.)": 'eq1.draw:1 eq2.set:20',
    "Flip a coin. If heads, choose an attack on 1 of your opponent's Pokemon. Super Metronome copies that attack except for its Energy cost. (You must still do anything else in order to use that attack.) (No matter what type the Defending Pokemon is, SELF's type is still Colorless.) SELF performs that attack. (SELF can make that attack even if it does not have the appropriate number or type of Energy attached to it necessary to make the attack.)": ('METRONOME', 0, 17),
    "Look at a number of cards on top of your opponent's deck equal to the number of Energy cards attached to the Defending Pokemon. Put those cards in any order, and then put them back on top of your opponent's deck.": 'shufatk',
    "Choose 1 Basic Pokemon in any player's discard pile. Put it onto that player's Bench. Put a number of damage counters on that Pokemon equal to half its HP (rounded down to the nearest 10). (You can't put a Pokemon on a Bench that's full.)": 'askch:41,1',
    'Choose any number of your Hoppips and Skiplooms. Then, for each Pokemon you chose in this way, you may search your deck for a card that evolves from that Pokemon and attach it to that Pokemon. (This counts as evolving those Pokemon.) Shuffle your deck afterward.': 'ask0:47',
    'Flip a coin. If heads, you may return any number of your Pokemon in play and all cards attached to them to your hand.': 'c h.askch:48,1',
    "Flip a coin. If heads, choose a basic Energy card attached to 1 of your opponent's Pokemon and attach it to another of your opponent's Pokemon of your choice.": 'c h.askch:21,1',
    "Flip a coin. If heads, choose 1 of either player's Evolved Pokemon, remove the highest stage Evolution card from that Pokemon, and put it into that player's hand.": ('DEVOLUTION_BEAM', 1, 0),
    'Move 2 Water Energy cards attached to SELF to 1 or 2 of your Benched Pokemon. (You may put both on the same Pokemon or 1 each on 2 different Pokemon.) If you have no Benched Pokemon, ignore this effect.': 'askn:23,2',
    "You may discard all Lightning Energy cards attached to SELF when you use this attack. If you do, put damage counters equal to the amount of Energy cards removed in this way on any number of your opponent's Benched Pokemon in the way you like. (For example, if you discard 3 Lightning Energy cards, you can put 1 damage counter on 1 of your opponent's Benched Pokemon and 2 on another.)": 'opt:0,2 o1.blast',
    'Flip 3 coins. For each heads, you may search your deck for a Lightning Energy card and attach it to 1 of your Lightning Pokemon. Shuffle your deck afterward.': 'c:3 askn:25,0',
    "Choose 1 of your opponent's Pokemon. Remove any number of damage counters from that Pokemon, then draw that many cards.": 'opt:12 helph',
    "Flip a coin. If heads and if any of your Pokemon have any damage counters on them, then remove 2 damage counters from 1 of them (or 1 if it only has 1). If tails and if any of your opponent's Pokemon have any damage counters on them, choose 1 of them and remove 2 damage counters from it (or 1 if it only has 1).": 'c h.askch:19,1 t.askch:20,1',
    "Choose 1 of your opponent's Pokemon and put a Lightning Rod marker on it. (A Pokemon can have only 1 Lightning Rod marker on it at a time.)": 'rodset',
    'This attack does 20 damage to any Pokemon with a Lightning Rod marker on it. Apply Weakness and Resistance.': 'rodhit:20',
    "If the Defending Pokemon doesn't have a Char counter on it, flip a coin. If heads, put a Char counter on it. A Char counter requires your opponent to flip a coin after every turn. If tails, put 2 damage counters on the Pokemon with that Char counter. (Char counters stay on the Pokemon as long as it's in play.)": 'char',
    'Flip a coin. If heads, choose a type (other than Colorless) and put a Coloring counter on the Defending Pokemon. That Pokemon is now the type you chose. If it already had a Coloring counter, remove the old one. If tails, this attack does nothing.': 'opt:10 c h.color',
    'Your opponent looks at the top 3 cards of his or her deck. If any of them are basic Energy cards, he or she may show any number of them to you and put them into his or her hand. You do the same. Either way, each player shuffles his or her deck.': 'askch:7',
    'Your opponent may choose 1 of his or her Pokemon and search his or her deck for a card that evolves from that Pokemon. Your opponent attaches that card to that Pokemon. This counts as evolving that Pokemon. Either way, you may do the same, and then each player who searched shuffles his or her deck.': 'askch:8',
    'Flip a coin. If heads, this attack does 20 damage. If tails and if your opponent has any Benched Pokemon, he or she chooses 1 of them and this attack does 20 damage to it.': 'c h.add:20 t.askch:11',
    'Flip a coin. If heads, choose an Energy card attached to the Defending Pokemon. Your opponent moves that card to 1 of his or her other Pokemon. (If your opponent has no Benched Pokemon, this attack does nothing.)': 'c h.ekick',
    "Look at your opponent's hand. If he or she has any Baby Pokemon and/or Basic Pokemon there, you may put any number of them onto your opponent's Bench (as long as there's room). Then, your opponent looks at your hand. If you have any Baby Pokemon and/or Basic Pokemon there, your opponent may put any number of them onto your Bench (as long as there's room).": 'obench',
    'Your opponent may choose a Baby Pokemon, Basic Pokemon, or Evolution card from his or her discard pile and put it into his or her hand. Either way, you may do the same.': 'askch:1',
    'Your opponent may choose up to 3 Baby Pokemon, Basic Pokemon, and/or Evolution cards from his or her discard pile and shuffle them into his or her deck. Either way, you may do the same.': 'askch:2',
    'Flip a coin. If heads, each player may choose a card from his or her discard pile and put it into his or her hand.': 'c h.askch:3',
    "During your next turn, treat any tails flipped when using SELF's Electric Bolt attack as if they were heads.": 'lockon',
    "Use any attack from SELF's Basic Pokemon card or Evolution card. (SELF doesn't have to pay for that attack's Energy cost.)": ('METRONOME', 0, 4),
    'If the Defending Pokemon attacked last turn, and SELF was in play during that attack, SELF copies that attack except for its Energy costs and anything else required in order to use that attack.': ('METRONOME', 0, 8),
    'If you have any Grass Pokemon on your Bench, remove all Grass Energy cards from SELF and attach them to 1 of those Pokemon, then switch SELF with that Pokemon.': 'gmove:1',
    "Flip 3 coins. For each heads, discard 1 Fire Energy card attached to SELF. If you can't discard Energy cards, this attack does nothing. This attack does 10 damage times the number of heads to each of your opponent's Pokemon. Don't apply Weakness and Resistance for this attack.": 'c:3 dsxh:0 mulh:10 nowr boh:10',
    'Discard all Fire Energy cards attached to SELF. If all Energy cards attached to SELF provide 2 Fire Energy, discard all of them. This attack does 20 damage plus 20 more damage for each Fire Energy discarded in this way.': 'adde:0,20 dse:0,0',
    'Put SELF and all cards attached to it on top of your deck. Then search your deck for any Basic Pokemon or Evolution card not named SELF. Show that card to your opponent, then put it into your hand. Shuffle your deck afterward.': 'srchk:4 selfdeck',
    'Choose up to 3 cards from your hand and put them on top of your deck. Then search your deck for that many basic Energy cards. Show those cards to your opponent, then put them into your hand. Shuffle your deck afterward.': 'mime',
    'Flip a coin for each Fire Energy card attached to SELF. For each heads, discard a Fire Energy card attached to SELF or this attack does nothing. Then, for each heads, choose an Energy card attached to the Defending Pokemon and discard it. If it has fewer Energy cards than that, remove all of them.': 'ce:0 dsxh:0 ddeh:6',
    "If the Defending Pokemon has any Energy cards attached to it, flip a coin. If heads, choose 1 of those Energy cards and move it to 1 of your opponent's Benched Pokemon. If your opponent has no Benched Pokemon, ignore this effect.": ('MAGNETIC_LINES', 0, 7),
    "Flip a coin. If heads, choose 1 Energy card attached to the Defending Pokemon and 1 of your opponent's Benched Pokemon. Attach that Energy card to that Pokemon.": ('MAGNETIC_LINES', 0, 3),
    "Flip a coin. If heads, SELF does 40 damage to itself and 10 damage to each Pokemon on each player's Bench. Flip a coin. If tails, this attack does nothing (not even damage).": 'c:2 t2.none ah.rec:40 ah.bb:10',
    "If the Defending Pokemon is a Basic Pokemon, choose 1 of its attacks. That Pokemon can't use that attack during your opponent's next turn.": 'opt:11 amn',
    "If an attack does damage to SELF during your opponent's next turn (even if SELF is Knocked Out), flip a coin. If heads, prevent all damage done to SELF from that attack and do 20 damage to the attacking Pokemon.": 'guard:10',
    'Whenever SELF is attacked, your opponent flips a coin. If tails, that attack does no damage to SELF. (Any other effects of the attack still happen.) This effect lasts until SELF takes damage (or is Benched or is evolved).': 'guardk:11',
    "Flip a coin. If heads, SELF's maximum HP is now 80 and SELF's Pound attack's base damage is 30 instead of 10. (Benching SELF ends this effect.)": 'c h.hpset:80,1,30',
    "If an attack does damage to SELF during your opponent's next turn (even if SELF is Knocked Out), SELF attacks your opponent's Active Pokemon for 10 damage. (Apply Weakness and Resistance.)": 'react:1',
    "If an attack does damage to SELF during your opponent's next turn (even if SELF is Knocked Out), flip a coin. If heads, SELF attacks your opponent's Active Pokemon for double that amount of damage. (If SELF takes 20 damage, it does 40 damage to that Pokemon.)": 'react:2',
    "If an attack damages SELF during your opponent's next turn (even if SELF is Knocked Out), flip a coin. If heads, SELF attacks the Defending Pokemon for an equal amount of damage.": 'react:3',
    "During your opponent's next turn, whenever your opponent's attack damages SELF (even if SELF is Knocked Out), your opponent's Active Pokemon is now Poisoned and SELF does 10 damage to each of your opponent's Benched Pokemon.": 'react:4',
    "Flip a coin. If heads, then if, during your opponent's next turn, SELF would be Knocked Out by an attack, SELF isn't Knocked Out and its remaining HP become 10 instead.": 'c h.react:5',
    'Until the end of your next turn, if SELF would be Knocked Out by damage from an attack, flip a coin. If heads, SELF is not Knocked Out and its remaining HP become 10 instead.': 'react:6,2',
    "During your opponent's next turn, if the Defending Pokemon tries to retreat, do 10 damage to it. (Don't apply Weakness and Resistance.)": 'pursuit',
    'If your opponent attaches an Energy card to the Defending Pokemon during his or her next turn, that Pokemon becomes Poisoned.': 'attpsn',
    "Flip a coin. If tails, this attack does nothing. Either way, you can't use this attack again as long as SELF stays in play (even putting SELF on the Bench won't let you use it again).": ('FAIL_COIN', 0, 1),
    "Flip a coin. If tails, this attack does nothing. Either way, you can't use this attack again as long as SELF stays in play (even putting SELF on the Bench won't let you use it again).": ('FAIL_COIN', 0, 1),
    "During your next turn, SELF's Double-edge attack's damage (base damage and damage to itself) is doubled.": 'boost:1,0',
    "This attack can't be used unless SELF used its Lie Low attack last turn.": 'needlast:0',
    'If the Defending Pokemon is Asleep and was attacked with Night Eyes during your last turn, it is Knocked Out.': 'koslp:0',
    'Flip 3 coins. If exactly 1 is heads, this attack does 40 damage. If exactly 2 are heads, remove 3 damage counters from the Defending Pokemon. If the Defending Pokemon has fewer damage counters than that, remove all of them. If all 3 are heads, this attack does 60 damage. If all 3 are tails, remove all damage counters from the Defending Pokemon.': 'c:3 eq1.add:40 eq2.dheal:3 ah.add:60 at.dheal:0',
    "Choose an Energy type other than Colorless. This attack does 20 damage to each of your opponent's Pokemon with any Energy cards of that type attached to it. Don't apply Weakness and Resistance.": 'opt:10 oetype:20',
    'Flip a number of coins equal to the total number of Pokemon in play. For each heads, you may search your deck for a basic Energy card, show it to your opponent, and put it into your hand. Shuffle your deck afterward.': 'cpk srche:0,12',
    'Shuffle SELF and all cards attached to it into your deck. Flip a coin. If heads, shuffle a card from your discard pile into your deck.': 'selfdeck c h.distop:0 h.shuf',
    'You may discard a Fire Energy card attached to SELF when you use this attack. If you do and if your opponent has any Benched Pokemon, choose 1 of them and this attack does 20 damage to it.': 'opt:0,0 o1.dsa:0,1 o1.b1:20',
    "The Defending Pokemon is now Poisoned. Flip a coin. If heads, the Defending Pokemon can't retreat until the end of your opponent's next turn and if the effect of an attack, Pokemon Power, or Trainer card would change that player's Active Pokemon, that part of the effect does nothing.": 'psn c h.anchor:0',
    "Until the end of your opponent's next turn, as long as SELF is your Active Pokemon, the Defending Pokemon can't retreat, and if the effect of an attack, Pokemon Power, or Trainer card would change that player's Active Pokemon, that part of the effect does nothing.": 'anchor:1',
    "Flip a coin. If heads, this attack does 20 damage plus 10 more damage and, until the end of your next turn, as long as SELF is your Active Pokemon, the Defending Pokemon can't retreat, and if the effect of an attack, Pokemon Power, or Trainer card would change that player's Active Pokemon, that part of the effect does nothing. If tails, this attack does 20 damage.": 'c h.add:10 h.anchor:2',
    "During your opponent's next turn, whenever 30 or more damage is done to SELF (after applying Weakness and Resistance), prevent that damage.": 'guard:9',
    "If you have any Benched Pokemon, switch 1 of them with SELF. As long as that Pokemon is your Active Pokemon, it can't become Asleep, Confused, Paralyzed, or Poisoned. (All other effects of attacks, Pokemon Powers, and Trainer cards still happen.)": 'mb.ssw mb.noconds:1',
    'You may discard any number of Fire Energy cards attached to your Pokemon. Flip a coin for each Fire Energy card discarded in this way. This attack does 40 damage times the number of heads.': 'opt:9,0 dfld:0,1 mulh:40',
    'Discard any number of Energy cards attached to any of your Pokemon. Flip a number of coins equal to the number of Energy cards discarded this way. This attack does 40 damage times the number of heads.': 'opt:9,15 dfld:15,1 mulh:40',
    'Discard any number of Energy cards attached to any of your Pokemon. Flip a number of coins equal to the number of Energy cards discarded this way. This attack does 40 damage times the number of heads.': 'opt:9,15 dfld:15,1 mulh:40',
    "Search your deck for a Basic Pokemon card named SELF or Nidoran M and put it onto your Bench. Shuffle your deck afterward. (You can't use this attack if your Bench is full.)": ('CALL_FAMILY', 0, 0),
    "Choose an evolved Pokemon (your own or your opponent's). Return the highest Stage Evolution card on that Pokemon to its player's hand. That Pokemon is no longer Asleep, Confused, Paralyzed, or Poisoned, or anything else that might be the result of an attack (just as if you had evolved it).": ('DEVOLUTION_BEAM', 0, 0),
    'When you use this attack, you may discard any number of Fire Energy cards attached to your Pokemon in play. This attack does 10 damage plus 10 more damage for each Fire Energy card you discarded in this way.': ('BURNING_FIRE', 10, 0),
    "If the Defending Pokemon has any basic Energy cards attached to it, flip a coin. If heads, choose 1 of those Energy cards and move it to 1 of your opponent's Benched Pokemon. If your opponent has no Benched Pokemon, ignore this effect.": ('MAGNETIC_LINES', 0, 5),
    'Your opponent switches each Defending Pokemon with his or her Benched Pokemon. If your opponent has only 1 Benched Pokemon, you choose the Defending Pokemon to switch.': 'osw',
    "If it's not your birthday, this attack does 30 damage. If it is your birthday, flip a coin. If heads, this attack does 30 damage plus 50 more damage; if tails, this attack does 30 damage.": 'nop',
    "Flip a coin. If heads, discard 1 Fire Energy card attached to SELF. If tails, discard all Energy cards attached to SELF. If you can't discard Energy cards, this attack does nothing.": 'c h.dsa:0,1 t.dsa:15,0',
    "If SELF and the Defending Pokemon don't have the same number of Energy cards attached to them, the player controlling the Active Pokemon with the fewest number of Energy cards attached to it switches 1 of his or her Benched Pokemon with his or her Active Pokemon.": 'dfew.osw dmoree.ssw',
    "Flip a coin. If heads, this attack does 10 damage to each of your opponent's Pokemon. Don't apply Weakness and Resistance. Then, search your deck for an Evolution card named Dark Tyranitar and put it on SELF. (This counts as evolving SELF.) Shuffle your deck afterward.": 'c h.oall:10 evdeck',
    "Put a Baby Pokemon or Basic Pokemon card from your opponent's discard pile onto his or her Bench. Put 1 damage counter on that Pokemon. (You can't use this attack if your Bench is full.)": 'odisbench',
    'If your opponent has any evolved Pokemon in play, choose 1 of them and flip a coin. If heads, your opponent takes the highest Stage Evolution card on that Pokemon and shuffles it into his or her deck.': 'oevo.c h.odevb:1',
    "Flip a number of coins equal to the number of Fighting Energy cards attached to SELF. This attack does 20 damage times the number of heads. Then, for each heads, discard the top card from your opponent's deck.": 'ce:4 mulh:20 omill:0',
    "If there are any Darkness Energy cards attached to SELF, discard 1 of them and this attack does 30 damage plus 20 more damage (plus 10 more damage for the Darkness Energy you discarded). If there aren't any, this attack does 30 damage.": 'en7.add:30 en7.dsx:7,1',
    "If your opponent's Bench isn't full, look at his or her hand. If your opponent has any Baby Pokemon or Basic Pokemon there, choose 1 of them and put it on his or her Bench. Then, switch it with the Defending Pokemon.": 'ohbench',
    'If your opponent has any Benched Pokemon, choose 1 of them and switch it with the Defending Pokemon, then flip a coin. If heads, the new Defending Pokemon is now Paralyzed.': 'drag ob.c h.par',
    "If your opponent has any Benched Pokemon, flip a coin. If heads, return the Defending Pokemon and all cards attached to it to your opponent's hand. If tails, your opponent chooses 1 of his or her Benched Pokemon and switches it with the Defending Pokemon.": 'ob.c h.ohand t.osw',
    'If you have any Benched Pokemon, flip a coin. If heads, take 2 Energy cards attached to SELF and attach them to 1 of your Benched Pokemon. Then return SELF and all cards attached to it to your hand.': 'mb.c h.eto:16 h.eto:16 h.selfhand',
    "Discard 2 Fire Energy cards attached to SELF or this attack does nothing. This attack does 10 damage to each of your opponent's Benched Pokemon. Then, flip a coin. If heads, choose 1 Energy card attached to each of your opponent's Pokemon that has any Energy cards attached to it and discard those Energy cards.": 'dsx:0,2 bo:10 c h.deall',
    "Flip a coin. If heads, choose a Special Energy card attached to 1 of your opponent's Pokemon. Your opponent shuffles that card into his or her deck.": 'c h.dte:1',
    "Choose a Trainer card attached to 1 of your opponent's Pokemon. Your opponent shuffles that card into his or her deck.": 'dte:2',
    'If SELF does damage to the Defending Pokemon (after applying Weakness and Resistance), remove a number of damage counters from SELF equal to half the damage done to the Defending Pokemon (rounded up to the nearest 10). If SELF has fewer damage counters than that, remove all of them.': 'drain:2',
    'If this attack damages the Defending Pokemon, the Defending Pokemon is now Poisoned and you remove a number of damage counters from SELF equal to half that damage (rounded up to the nearest 10). If SELF has fewer damage counters than that, remove all of them.': 'dlt.psn drain:2',
    'If this attack damages the Defending Pokemon, the Defending Pokemon is now Poisoned and you remove a number of damage counters from SELF equal to half that damage (rounded up to the nearest 10). If SELF has fewer damage counters than that, remove all of them. Either way, the Defending Pokemon is now Poisoned.': 'psn drain:2',
    'Does 10 damage times the number of Energy attached to the Defending Pokemon. After doing damage, remove a number of damage counters from SELF equal to the amount of damage done to the Defending Pokemon (after applying Weakness and Resistance).': 'addde:10 drain:1',
    'Flip a coin for each Energy attached to the Defending Pokemon. This attack does 10 damage plus 10 more damage for each heads. Remove a number of damage counters from SELF equal to the damage done to the Defending Pokemon (after applying Weakness and Resistance). If SELF has fewer damage counters than that, remove all of them.': 'cde addh:10 drain:1',
    "Flip a coin for each of your opponent's Pokemon. For each heads, this attack does 10 damage to that Pokemon. Don't apply Weakness and Resistance. Remove a number of damage counters from SELF equal to the damage dealt. If SELF has fewer damage counters than that, remove all of them.": 'ocoinh:10',
    "Flip a coin. If heads, the Defending Pokemon is now Poisoned. Your opponent now puts 3 damage counters on it instead of 1 after each player's turn (even if it was already Poisoned).": 'c h.psn30',
    "Search your deck for any number of Pokemon named SELF, Tentacruel, SELF, and/or Misty's Tentacruel. Show those cards to your opponent, then put them into your hand. Shuffle your deck afterward.": 'nsrch:1,0',
    "Flip a coin. If heads, you may search your deck for a Basic Pokemon with Misty in its name and put it onto your Bench. (You can't use this attack if your Bench is full.) Shuffle your deck afterward.": 'needroom c h.dbown:1',
    "Flip a number of coins equal to the total number of Sabrina's Gastlys, Sabrina's Haunters, and Sabrina's Gengars you have in play. This attack does 30 damage times the number of heads.": 'cnp:2 mulh:30',
    "Flip a coin. If heads, you may search your deck for a Basic Pokemon card with Brock in its name and put it onto your Bench. Shuffle your deck afterward. (You can't use this attack if your Bench is full.)": 'needroom c h.dbown:0',
    "Flip a coin. If heads, this attack does 50 damage plus 50 more damage if you have at least 1 Giovanni's Nidoking on your Bench. If tails, this attack does nothing.": 'c t.none h.addnb:50,4,1',
    "Does 10 damage times the number of Koga's Zubats you have in play. Before doing damage, you may search your deck for any number of Basic Pokemon named SELF and put them onto your Bench. (You can't get more cards with this attack than you have room on your Bench.) If you do, shuffle your deck afterward.": 'dbsame mulname:10',
    'Flip a coin. If heads, this attack does 10 damage plus 10 more damage for each Omanyte, Omastar, SELF, and Kabutops on your Bench.': 'c t.none h.addnb:10,3,0',
    'Flip a number of coins equal to the number of Murkrows on your Bench. This attack does 10 damage plus 10 more damage for each heads.': 'cnb:5 addh:10',
    'Search your deck for a card named Gyarados, Dark Gyarados, or Shining Gyarados. Show it to your opponent and put it into your hand. Shuffle your deck afterward.': 'nsrch:0,1',
    "Flip a coin. If heads, your opponent can't attach Energy cards from his or her hand to his or her Active Pokemon during his or her next turn.": 'c h.noatt',
    "Flip a coin. If heads, your opponent can't attach Energy cards to the Defending Pokemon during his or her next turn.": 'c h.noatt',
    "If the Defending Pokemon's maximum HP is 50 or less, it can't attack SELF during your opponent's next turn.": 'dhp50.noat',
    "During your opponent's next turn, the Defending Pokemon can't evolve except from effects of attacks or Pokemon Powers. (Benching that Pokemon ends this effect.)": 'noevo',
    "Flip a coin. If heads, choose 1 Energy card attached to each of your opponent's Pokemon that has any Energy cards and discard those Energy cards.": 'c h.deall',
    # (Metronome's own effect row: extra 1 a coin first, 2 the copied attack's Energy paid by its own)
    "Choose 1 of the Defending Pokemon's attacks. Copy copies that attack. This attack does nothing if SELF doesn't have the Energy necessary to use that attack. (You must still do anything else required in order to use that attack.)": ('METRONOME', 0, 2),
    "Flip a coin. If heads, choose 1 of the Defending Pokemon's attacks. Mini-Metronome copies that attack except for its Energy cost. (You must still do anything else required in order to use that attack.) (No matter what type the Defending Pokemon is, SELF is still Colorless.) SELF performs that attack.": ('METRONOME', 0, 1),
    "Flip a coin. If heads, choose 1 of the Defending Pokemon's attacks. Mini-Metronome copies that attack except for its Energy cost. (You must still do anything else in order to use that attack.) (No matter what type the Defending Pokemon is, SELF's type is still Colorless.) SELF performs that attack.": ('METRONOME', 0, 1),
    'Flip a coin. If heads, search your deck for a card that evolves from 1 of your Benched Pokemon and put that card on that Pokemon. (This counts as evolving that Pokemon.) Shuffle your deck afterward.': 'c h.evbench',
    'For each SELF in play, you may search your deck for a Lightning Energy card and attach it to SELF. Shuffle your deck afterward.': 'srchnm:2',
    'You may search your deck for a Fire Energy card and attach it to Ho-Oh. Shuffle your deck afterward.': 'srch:0,1',
    'If you have any Benched Pokemon, flip a coin. If heads, remove all damage counters from 1 of your Benched Pokemon and discard all Energy cards attached to it.': 'c h.healbd',
    'Flip a coin. If heads, choose a Trainer card from your discard pile, show it to your opponent, and then shuffle it into your deck.': 'c h.distrdeck',
    'Put an Omastar from your hand onto SELF. This counts as evolving SELF.': 'evhand',
    'Shuffle a basic Energy card from your discard pile into your deck.': 'dised',
    'Choose 1 of your Pokemon with an Energy card attached to it. Remove 1 damage counter from that Pokemon.': 'healte:1',
    'Before doing damage, discard any Trainer cards attached to the Defending Pokemon.': 'dtrdisc',
    'Flip a number of coins equal to the number of damage counters on SELF. This attack does 10 damage times the number of heads.': 'csc mulh:10',
    'Discard all Lightning Energy cards attached to SELF. Then, flip a number of coins equal to the number of Energy cards discarded that way. This attack does 40 damage times the number of heads.': 'ce:2 dse:2,0 mulh:40',
    "Choose 1 of your opponent's Pokemon. This attack does 20 damage to that Pokemon. Don't apply Weakness or Resistance.": 'zero any1:20',
    'If the Pokemon using this attack has Fire and Lightning basic Energy cards attached to it, the Defending Pokemon is now Confused. If the Pokemon using this attack has Fire and Fighting basic Energy cards attached to it, this attack does 30 damage plus 10 more damage and remove 3 damage counters from the Pokemon that Miracle Plate SELF is attached to.': 'e02.cnf e04.add:10 e04.heal:3',
    'If the Pokemon using this attack has Fire and Water basic Energy cards attached to it, this attack does 30 damage plus 10 more damage and the Defending Pokemon is now Burned. If the Pokemon using this attack has Water and Psychic basic Energy cards attached to it, choose an Energy card attached to the Defending Pokemon, if any, and your opponent shuffles that card into his or her deck.': 'e03.add:10 e03.brn e35.dedeck:1',
    "If the Pokemon using this attack has Grass and Lightning basic Energy cards attached to it, the Defending Pokemon is now Asleep and Poisoned. If the Pokemon using this attack has Water and Lightning basic Energy cards attached to it, this attack does 30 damage plus 10 more damage and lets you put 1 damage counter on each of your opponent's Benched Pokemon.": 'e12.slp e12.psn e32.add:10 e32.bco:1',
    'If your opponent has 5 or more Prizes, search your deck for a Trainer card, show it to your opponent, and put it into your hand. If your opponent has only 1 Prize, the Defending Pokemon is now Burned, Paralyzed, and Poisoned.': 'pg5.srchk:2 pz1.brn pz1.par pz1.psn',
    'If your opponent has 5 or more Prizes, draw 3 cards. If your opponent has only 1 Prize, choose 2 Energy cards attached to the Defending Pokemon (1 if there is only 1). Your opponent shuffles those cards into his or her deck.': 'pg5.draw:3 pz1.dedeck:2',
    "If your opponent has 5 or more Prizes, shuffle your hand into your deck and then draw 6 cards. If your opponent has exactly 2 Prizes, choose 1 of your opponent's Evolved Pokemon. Your opponent puts the top card on that Evolved Pokemon on the bottom of his or her deck. (This counts as devolving that Pokemon.)": 'pg5.handdeck pg5.draw:6 pz2.odevb',
    'If your opponent has 5 or more Prizes, search your deck for up to 3 basic Energy cards, show them to your opponent, and put them into your hand. Shuffle your deck afterward. If your opponent has exactly 2 Prizes, remove all damage counters from 1 of your Pokemon.': 'pg5.srche:3 pz2.healt',
    'Flip a coin. If tails, shuffle SELF and all cards attached to it into your deck.': 'c t.selfdeck',
    'Flip a coin. If heads, shuffle SELF and all cards attached to it into your deck.': 'c h.selfdeck',
    'Shuffle SELF and all cards attached to it into your deck. Then, your opponent shuffles his or her Active Pokemon and all cards attached to it into his or her deck.': 'odeck selfdeck',
    'Unless this attack Knocks Out the Defending Pokemon, flip 2 coins. If both of them are heads, your opponent shuffles his or her Active Pokemon and all cards attached to it into his or her deck.': 'c:2 ah.odeck',
    'Flip a coin. If heads, return SELF and all cards attached to it to your hand. (Either way, this attack does its damage.)': 'c h.selfhand',
    'Return SELF and all Energy cards attached to it to your hand. (Discard all other cards attached to SELF.)': 'selfhand:1',
    'Return SELF and all basic Energy cards attached to it to your hand. If you have no Benched Pokemon, this attack does nothing.': 'needmb selfhand:2',
    "Flip a coin. If heads, look at your opponent's hand. If he or she has any Trainer cards there, choose 1 of them. Your opponent shuffles that card into his or her deck.": 'c h.ohdeck:1',
    "Flip a coin. If heads, look at your opponent's hand. If he or she has any Trainer cards there, choose 1 of them. You opponent shuffles that card into his or her deck.": 'c h.ohdeck:1',
    "Flip a coin. If heads, look at your opponent's hand. If he or she has any Energy cards there, choose 1 of them. Your opponent shuffles that card into his or her deck.": 'c h.ohdeck:2',
    "Look at your opponent's hand. Choose all Technical Machine and Pokemon Tool cards there and put them into his or her deck. Your opponent shuffles the deck afterward.": 'ohtmtool',
    'Search your discard pile for a Metal Energy card and attach it to SELF.': 'attdn:0',
    'Discard 1 Fire Energy card attached to SELF or this attack does nothing. If there are any Darkness Energy cards in your discard pile, attach 1 of them to SELF.': 'dsx:0,1 attdn:1',
    "Flip a coin. If heads, the Defending Pokemon can't retreat. (Benching or evolving that Pokemon ends this effect.)": 'c h.stuck',
    "The Defending Pokemon can't retreat as long as SELF remains your Active Pokemon.": 'stuck:2',
    "Flip a coin. If heads, the Defending Pokemon can't attack. (Benching or evolving the Defending Pokemon ends this effect.)": 'c h.noatall',
    'Flip a coin. If heads, whenever the Defending Pokemon tries to attack, your opponent flips a coin. If tails, that attack does nothing. (This effect lasts until the Defending Pokemon evolves or is Benched.)': 'c h.blindall',
    'Flip a coin. If heads, whenever the Defending Pokemon attacks, your opponent flips a coin. If tails, that attack does nothing. (Benching or evolving that Pokemon ends this effect.)': 'c h.blindall',
    "The Defending Pokemon can't retreat during your opponent's next turn. If the Defending Pokemon tries to attack during your opponent's next turn, your opponent flips a coin. If tails, that attack does nothing. (Benching either Pokemon ends this effect.)": 'nort weak:3',
    "During your next turn, SELF's Gnaw attack's base damage is doubled.": 'boost:1,0',
    "During your next turn, SELF's Quick Attack's base damage is doubled.": 'boost:1,0',
    "During your next turn, SELF's Mega Punch attack's base damage is 80.": 'boost:1,80',
    "During your opponent's next turn, SELF can't become Asleep, Confused, Paralyzed, or Poisoned. (All other effects of attacks, Pokemon Powers and Trainer cards still happen.)": 'noconds',
    "During your opponent's next turn, SELF can't become affected by a Special Condition. (Any other effects of attacks, Poke-Powers, Poke-Bodies, and Trainer cards still happen.)": 'noconds',
    "Prevent all effects of attacks, other than damage, done to SELF during your opponent's next turn.": 'bar:3',
    "If the Defending Pokemon attacks during your opponent's next turn, any damage it does is reduced by 10 (before applying Weakness and Resistance).": 'weak:1',
    "Flip a coin. If heads, the Defending Pokemon is now Paralyzed. If tails, during your opponent's next turn, your opponent pays Colorless more to retreat the Defending Pokemon.": 'c h.par t.rt1:1',
    'Flip a coin. If heads, discard a Special Energy card attached to the Defending Pokemon. If tails, this attack does nothing.': 'c t.none h.dspe',
    'Flip 2 coins. For each heads, choose 1 Energy card attached to the Defending Pokemon, if any, and discard it. If both are tails, this attack does nothing (not even damage).': 'c:2 at.none ddeh',
    'Flip 2 coins. If both are tails, this attack does nothing. For each heads, discard an Energy attached to the Defending Pokemon.': 'c:2 at.none ddeh',
    'Look at the top 3 cards of your deck, and then choose 1 of those cards and put it into your hand. Shuffle the rest into your deck afterward.': 'top3h',
    'Search your deck for a card that evolves from SELF. Attach it to SELF. This counts as evolving SELF. Shuffle your deck afterward.': 'evdeck',
    'Flip a coin. If heads, remove all damage counters from SELF. Then, search your deck for a card that evolves from SELF and attach that card to SELF. This counts as evolving SELF. Shuffle your deck afterward.': 'c h.heal:99 evdeck',
    'Flip a coin. If heads, remove all damage counters from SELF. Search your deck for a card that evolves from SELF and attach that card to SELF. This counts as evolving SELF. Shuffle your deck afterward.': 'c h.heal:99 evdeck',
    'Flip a coin. If heads, search your deck for a card that evolves from SELF, show it to your opponent, and put it into your hand. Shuffle your deck afterward.': 'c h.evdeck:1',
    'Flip 2 coins. For each heads, search your deck for a basic Energy card. Show those cards to your opponent, and then put them into your hand. Shuffle your deck afterward.': 'c:2 srche:0,2',
    'You may discard up to 2 cards from your hand. If you do, draw that many cards.': 'hdraw:2',
    'You may move a Lightning Energy card attached to SELF to 1 of your Pokemon.': 'opt:0,2 o1.eto:2,1',
    'Flip a coin. If heads, you may attach up to 2 Grass Energy cards from your hand to SELF.': 'c h.hatt:2,2,0',
    'Attach any number of basic Energy cards from your hand to SELF.': 'hatt:1,60,0',
    'Attach up to 2 Grass Energy cards from your hand to SELF.': 'hatt:2,2,0',
    'Attach an Energy card from your hand to SELF.': 'hatt:0,1,0',
    'You may attach a basic Energy card from your hand to 1 of your Pokemon.': 'hatt:1,1,1',
    'Attach up to 2 Grass Energy cards from your hand to 1 of your Grass Pokemon.': 'hatt:2,2,1',
    "Before doing damage, you may choose 1 of your opponent's Benched Pokemon with no damage counters on it and switch the Defending Pokemon with it.": 'dragmay:1',
    "Before doing damage, you may choose 1 of your opponent's Benched Pokemon and switch it with the Defending Pokemon.": 'dragmay',
    'If your opponent has any Benched Pokemon, you may choose 1 of them and switch it with the Defending Pokemon (before doing damage or other effects of this attack). Either way, the Defending Pokemon is now Asleep.': 'dragmay slp',
    "If your opponent has any Benched Pokemon, flip a coin. If heads, choose 1 of your opponent's Benched Pokemon and switch it with the Defending Pokemon. The new Defending Pokemon is now Poisoned.": 'c h.luremay h.psn',
    'Flip 3 coins. If you get 1 heads, this attack does 10 damage plus 10 more damage. If you get 2 heads, this attack does 10 damage plus 30 more damage. If you get 3 heads, this attack does 10 damage plus 60 more damage.': 'c:3 eq1.add:10 eq2.add:30 eq3.add:60',
    "Flip a coin. If heads, this attack does 20 damage to the Defending Pokemon (don't apply Weakness or Resistance). If tails and your opponent has any Benched Pokemon, choose 1 of them and this attack does 20 damage to it.": 'c h.set:20 h.nowr t.zero t.b1:20',
    'Flip a coin. If heads, this attack does 30 damage plus 10 more damage. If tails, this attack does 30 damage and, if your opponent has any Benched Pokemon, he or she chooses 1 of them and switches it with the Defending Pokemon.': 'c h.add:10 t.osw',
    "If the Defending Pokemon attacks SELF during your opponent's next turn, any damage done by the attack is reduced by 10 (after applying Weakness and Resistance). (Benching either Pokemon ends this effect.)": 'guard:8',
    "If the Defending Pokemon attacks SELF during your opponent's next turn, any damage done to SELF is reduced by 20 (before applying Weakness and Resistance). (Benching either Pokemon ends this effect.)": 'weak:2',
    "Choose 1 of your opponent's Pokemon that has a damage counter on it. This attack does 20 damage to that Pokemon. Don't apply Weakness and Resistance.": 'zero any1:20,1',
    'This attack does 30 damage plus 10 more damage for each Zubat, SELF, and Crobat on your Bench.': 'addfam:10',
    'Search your deck for a Supporter card, show it to your opponent, and put it into your hand. Shuffle your deck afterward.': 'srchk:0',
    'Search your deck for a Technical Machine or Pokemon Tool card, show it to your opponent, and put it into your hand. Shuffle your deck afterward.': 'srchk:1',
    'Flip a coin. If heads, you may search your deck for a Trainer card. Show it to your opponent and put it into your hand. Shuffle your deck afterward.': 'c h.srchk:2',
    "Search your deck for a Baby Pokemon or Basic Pokemon card and put it onto your Bench. Shuffle your deck afterward. (You can't use this attack if your Bench is full.)": 'needroom dbench:0',
    "Search your deck for a Baby Pokemon card and put it onto your Bench. Shuffle your deck afterward. (You can't use this attack if your Bench is full.)": 'needroom dbench:1',
    "Put a Baby Pokemon or a Basic Pokemon card from your discard pile onto your Bench. (You can't use this attack if your Bench is full.)": 'needroom disbench',
    'Search your discard pile for a Baby Pokemon, Basic Pokemon, Evolution card, or basic Energy card, show it to your opponent, and then put it into your hand.': 'dishand',
    'You may move a Water Energy attached to SELF to 1 of your Pokemon.': 'opt:0,3 o1.eto:3,1',
    'You may move a Fire Energy card attached to SELF to 1 of your Pokemon.': 'opt:0,0 o1.eto:0,1',
    "During your opponent's next turn, whenever SELF takes damage, divide that damage in half (rounded down to the nearest 10). (Any other effects still happen.)": 'guard:4',
    "Whenever an attack does damage to SELF (after applying Weakness and Resistance) during your opponent's next turn, that attack only does half the damage to SELF (rounded down to the nearest 10).": 'guard:4',
    "If SELF would be damaged by an attack during your opponent's next turn, flip a coin. If heads, prevent that attack's damage done to SELF.": 'guard:7',
    "If an attack would do damage to SELF during your opponent's next turn, your opponent flips a coin. If tails, prevent all damage to SELF from that attack. (Any other effects of that attack happen.)": 'guard:7',
    "During your opponent's next turn, whenever 20 or less damage is done to SELF (after applying Weakness and Resistance), prevent that damage..": 'guard:5',
    "Your opponent can't play Trainer cards during his or her next turn.": 'notr',
    "SELF can't use this attack during your next turn. (Benching or evolving SELF ends this effect.)": 'lock',
    "The Defending Pokemon can't retreat until the end of your opponent's next turn.": 'nort',
    "Flip a coin. If heads, until the end of your opponent's next turn the Defending Pokemon can't attack or retreat.": 'c h.noat h.nort',
    'Discard all Energy cards attached to SELF.': 'dsa:15,0',
    'Discard all Fire Energy attached to SELF.': 'dsa:0,0',
    'Draw 2 cards.': 'draw:2',
    'Put 1 damage counter on SELF.': 'ctr:1',
    'Remove a damage counter from SELF.': 'heal:1',
    'Does 10 damage to each of your Benched Pokemon.': 'bm:10',
    'Does 10 damage to 1 of your Benched Pokemon.': 'm1:10',
    'Flip a coin. If heads, your opponent discards the top card from his or her deck.': 'c h.omill:1',
    'Flip 2 coins. This attack does 10 damage times the number of heads': 'c:2 mulh:10',
    'Flip 3 coins. This attack does 20 damage times the number of heads. If you get at least 1 heads, the Defending Pokemon is now Poisoned.': 'c:3 mulh:20 h.psn',
    'Flip a coin. If heads, switch SELF with 1 of your Benched Pokemon.': 'c h.ssw',
    'Discard a Fire Energy card attached to SELF in order to use this attack. The Defending Pokemon is now Burned.': 'dse:0,1 brn',
    'Discard an Energy card attached to SELF and flip a coin. If heads, the Defending Pokemon is now Burned.': 'dse:15,1 c h.brn',
    'Discard an Energy card attached to SELF and draw 3 cards.': 'dse:15,1 draw:3',
    'Flip a coin. If heads, discard an Energy card attached to the Defending Pokemon.': 'c h.dde',
    'SELF is now Asleep. Flip a coin. If heads, the Defending Pokemon is now Asleep.': 'sslp c h.slp',
    'If your opponent has any Benched Pokemon, choose 1 of them. Flip a coin. If heads, this attack does 20 damage to that Pokemon.': 'c h.b1:20',
    "Flip a coin. If tails, this attack's base damage is 20 instead of 60.": 'c t.set:20',
    "Discard a Water Energy card attached to SELF. Does 10 damage to each Benched Pokemon (both yours and your opponent's).": 'dsa:3,1 bb:10',
    'Discard a Fighting Energy card attached to SELF. Does 10 damage to your Benched Pokemon.': 'dsa:4,1 bm:10',
    "Flip a coin. If heads, prevent all effects of an attack, including damage, done to SELF during your opponent's next turn.": 'c h.bar:2',
    'Before doing damage, discard all Trainer cards attached to the Defending Pokemon (before they affect the damage.).': 'dtrdisc',
    "If your opponent has any Benched Pokemon, flip a coin. If heads, choose 1 of your opponent's Benched Pokemon and switch it with the Defending Pokemon.": 'c h.luremay',
    'Your opponent switches the Defending Pokemon with 1 of his or her Benched Pokemon.': 'osw',
    'Put a damage counter on the Defending Pokemon.': 'dcn:1',
    "Choose 1 of your opponent's Pokemon. Put a damage counter on that Pokemon.": 'zero any1c:1',
    "Choose 1 of your opponent's Pokemon. This attack does 30 damage to that Pokemon. Don't apply Weakness or Resistance.": 'zero any1:30',
    'Put 1 damage counter on the Defending Pokemon for each damage counter on SELF.': 'dcself',
    'Count the number of your Pokemon in play with damage counters on them. Put that many damage counters on the Defending Pokemon.': 'dcmine',
    'This attack does 10 damage times the number of Energy cards attached to the Defending Pokemon.': 'zero addde:10',
    "Flip a coin, If heads, this attack does 10 damage to each of your opponent's Benched Pokemon. If tails, this attack does 10 damage to each of your Benched Pokemon.": 'c h.bo:10 t.bm:10',
    'Flip a coin. If heads, this attack does 10 damage times the number of Pokemon you have in play.': 'c t.zero h.mulpk:10',
    "This attack does 20 damage to each Pokemon on each player's Bench. SELF does 100 damage to itself.": 'bb:20 rec:100',
    'Flip 4 coins. This attack does 30 damage plus 20 more damage for each heads. Put a damage counter on SELF for each heads.': 'c:4 addh:20 ctrh:1',
    'Flip 2 coins. If either is heads, this attack does 10 damage times the number of damage counters on SELF.': 'c:2 zero h.addsc:10',
    "If there are 4 or more damage counters on SELF, this attack's base damage is 50 instead of 30.": 'sc4.set:50',
    'Discard all Lightning Energy cards attached to SELF. This attack does 20 damage plus 10 more damage for each Energy card discarded in this way.': 'adde:2,10 dsa:2,0',
    'Remove 2 damage counters from SELF for each Energy attached to it. If it has fewer damage counters than that, remove all of them.': 'healen:2',
    'Flip 2 coins. For each heads, remove 1 damage counter from each of your Pokemon. If a Pokemon has fewer damage counters than the number of heads, remove all of them.': 'c:2 h.healall:1 ah.healall:1',
    "Remove 2 damage counters from SELF and each Pokemon that's the same type (color) as SELF.": 'healtyp:2',
    "SELF does 20 damage to itself. This attack also does 10 damage to each Benched Pokemon (yours and your opponent's).": 'rec:20 bb:10',
    'Discard 2 Fire Energy cards attached to SELF in order to use this attack. This attack does 40 damage plus 10 more damage for each damage counter on SELF.': 'dse:0,2 addsc:10',
    'Flip a coin. If heads, and if your opponent has any Benched Pokemon, choose 1 of them. This attack does 30 damage to that Pokemon. If tails, SELF does 30 damage to itself.': 'c h.b1:30 t.rec:30',
    'Flip a number of coins equal to the number of Pokemon in play. This attack does 10 damage times the number of heads. Then SELF does 10 damage times the number of tails to itself.': 'cpk mulh:10 rect:10',
    "During your opponent's next turn, prevent all effects of attacks, including damage, done to your Benched Pokemon.": 'bshield',
    "Until the end of your opponent's next turn, as long as SELF is your Active Pokemon, prevent all damage done by attacks to your Benched Pokemon.": 'bshield',
    'Flip 2 coins. For each tails, discard 1 Energy card attached to SELF.': 'c:2 dsat:15',
    'Remove 3 damage counters from each of your Active Pokemon. Each Defending Pokemon is now Asleep.': 'heal:3 slp',
    "Flip a coin. If heads, the Defending Pokemon can't attack SELF during your opponent's next turn. (Benching either Pokemon ends this effect.)": 'c h.noat',
    'If there are any Lightning Energy card in your discard pile, flip a coin. If heads, attach 1 of them to SELF.': 'c h.attd:2,1',
    'Flip a coin. If heads, search your deck for an Energy card and attach it to SELF. Shuffle your deck afterward.': 'c h.srchp:15',
    'Search your deck for a basic Energy card and attach it to 1 of your Pokemon. Shuffle your deck afterward.': 'spike',
    'You may flip a coin. If heads, this attack does 80 damage. If tails, this attack does nothing.': 'opt:0 o1.c h.set:80 t.none',
    'You may discard all Lightning Energy cards attached to SELF. If you do, this attack does 80 damage.': 'opt:0,2 o1.set:80 o1.dsa:2,0',
    'If you have any Benched Pokemon, flip a coin. If tails, this attack does 10 damage to 1 of them.': 'c t.m1:10',
    'Your opponent may draw a card.': 'odraw:1',
    "Don't apply Weakness or Resistance for this attack.": 'nowr',
    "For each Pokemon in play (yours and your opponent's), flip a coin. For each heads, this attack does 20 damage to that Pokemon. Don't apply Weakness and Resistance for this attack.": 'zero allc:20',
    'Remove 1 damage counter from each of your Pokemon that has any damage counters on it.': 'healall:1',
    'If the Defending Pokemon has Dark in its name or is a Darkness Pokemon, flip a coin. If heads, this attack does 100 damage instead of 50.': 'ddark.c h.set:100',
    'If there is a Stadium card in play, you may discard it.': 'nostad',
    "Flip a number of coins equal to the number of Energy cards attached to your opponent's Pokemon. This attack does 20 damage times the number of heads.": 'coe mulh:20',
    'If the Defending Pokemon has a Pokemon Power, that power stops working until the end of your next turn.': 'dpowoff',
    "Flip a coin. If heads, the Defending Pokemon is now Asleep. If tails, the Defending Pokemon can't retreat during your opponent's next turn.": 'c h.slp t.nort',
    "Choose 1 of your opponent's Pokemon. This attack does 10 damage for each Water Energy card attached to SELF. Don't apply Weakness and Resistance. You can't do more than 30 damage in this way.": 'zero any1e:3,10,30',
    "Show the top card of your opponent's deck to all players. If it's a Trainer card, discard it.": 'otop:1',
    'If the Defending Pokemon is Asleep or Confused, this attack does 20 damage plus 20 more damage. Then, the Defending Pokemon is no longer Asleep or Confused.': 'dslpcnf.add:20 dcure',
    'If you have any benched Pokemon, search your deck for a Fire Energy card and attach it to 1 of them. Then shuffle your deck.': 'srchb:0',
    'Flip 2 coins. For each heads, discard a Fire Energy card attached to SELF or this attack does nothing. This attack does 30 damage plus 20 damage for each heads.': 'c:2 dsxh:0 addh:20',
    'Flip 3 coins. This attack does 20 damage times the number of heads. If you get 2 or more heads, the Defending Pokemon is now Paralyzed.': 'c:3 mulh:20 ge2.par',
    'If you have any Benched Pokemon, shuffle 1 of them and all cards attached to it into your deck.': 'benchdeck',
    'If the Defending Pokemon has no damage counters on it, this attack does 40 damage. If it has any, this attack does 20 damage.': 'dnodmg.set:40 ddmg.set:20',
    'If there are more Energy attached to the Defending Pokemon than to SELF, this attack does 20 damage plus 20 more damage. If not, this attack does 20 damage.': 'dmoree.add:20',
    "The Defending Pokemon is now Asleep. Remove 1 damage counter from each Benched Pokemon (yours and your opponent's) with any damage counters on it.": 'slp healbb:1',
    'If SELF and the Defending Pokemon have a different number of Energy cards attached to them, this attack does nothing.': 'neqe.none',
    'Flip a coin. If heads, the Defending Pokemon is now Confused. If tails, SELF is now Confused.': 'c h.cnf t.scnf',
    'Remove a number of damage counters from 1 of your Benched Pokemon equal to the number of Water Energy cards attached to SELF. If the Pokemon has fewer damage counters than that, remove all of them.': 'healbe:3',
    'Flip a number of coins equal to the number of Energy attached to the Defending Pokemon. If you get 1 or more heads, the Defending Pokemon is now Asleep, Confused, or Poisoned (your choice).': 'opt:7 cde h.chosc:7',
    'Discard 1 Fire Energy card and 1 Lightning Energy card attached to SELF or this attack does nothing. Flip a coin. If tails, SELF does 30 damage to itself.': 'dsx:0,1 dsx:2,1 c t.rec:30',
    "Flip a coin. If heads, this attack does 30 damage plus 10 more damage and does 10 damage to each of your opponent's Benched Pokemon. If tails, this attack does 30 damage and SELF does 10 damage to itself.": 'c h.add:10 h.bo:10 t.rec:10',
    "Does 50 damage plus 10 more damage for each Water Energy attached to SELF but not used to pay for this attack's Energy cost. Don't apply Resistance.": 'addpast:3,10,99 nores',
    'Discard a Fire Energy card attached to SELF or this attack does nothing. This attack does 40 damage plus 10 damage for each Energy attached to the Defending Pokemon.': 'dsx:0,1 addde:10',
    'Flip 3 coins. If exactly 1 is heads, the Defending Pokemon is now Asleep. If exactly 2 are heads, the Defending Pokemon is now Confused. If all 3 are heads, the Defending Pokemon is now Paralyzed.': 'c:3 eq1.slp eq2.cnf eq3.par',
    'If your opponent has any Benched Pokemon, choose 1 of them and this attack does 10 damage to that Pokemon for each Water Energy attached to SELF.': 'b1e:3,10',
    "Flip a coin. If heads, this attack does 10 damage to each Benched Pokemon (yours and your opponent's). If tails, this attack does nothing. Either way, SELF can't attack during your next turn.": 'c t.zero h.bb:10 selflock',
    'Flip a coin until you get tails. For each heads, your opponent discards the top card from his or her deck.': 'cu omill:0',
    "Flip a coin. If heads, this attack does 10 damage to each of your opponent's Benched Pokemon, if any.": 'c h.bo:10',
    'Flip a coin. If heads, remove 2 damage counters from 1 of your Pokemon (1 if it has only 1).': 'c h.healt:2',
    'Count the number of Pokemon on your Bench. This attack does 10 times that number of damage to the Defending Pokemon, and SELF does 10 times that number of damage to itself.': 'mulmb:10 recmb:10',
    "Choose 2 of your opponent's Benched Pokemon (1 if he or she has only 1). This attack does 10 damage to each of those Pokemon.": 'bpick:10',
    "If the Defending Pokemon isn't Asleep, this attack does nothing.": 'dnslp.none',
    'Attach 2 basic Energy cards from your discard pile to SELF (1 if you have only 1).': 'attd:15,2',
    'Flip a coin. If heads this attack does 30 damage plus 10 more damage. If tails, SELF does 10 damage to itself.': 'c h.add:10 t.rec:10',
    'Flip a coin. If heads, draw a card.': 'c h.draw:1',
    'Flip a number of coins equal to the amount of Energy attached to all of of your Benched Pokemon. This attack does 30 damage plus 10 more damage for each heads.': 'cmbe addh:10',
    'This attack does 10 damage plus 10 more damage for each damage counter on the Defending Pokemon.': 'adddc:10',
    'Discard all Lightning Energy cards attached to SELF. Flip a number of coins equal to the number of Lightning Energy cards you discarded. This attack does 30 damage plus 40 more damage for each heads.': 'ce:2 addh:40 dsa:2,0',
    "Choose 1 of your opponent's Benched Pokemon and switch the Defending Pokemon with it. The new Defending Pokemon is now Asleep.": 'lure slp',
    'Flip a coin. If heads, the Defending Pokemon is now Asleep. If tails, remove 2 damage counters from SELF.': 'c h.slp t.heal:2',
    'Damage from this attack is reduced by 10 for each damage counter on SELF.': 'subsc:10',
    'SELF does 10 damage to itself. Then, your opponent switches the Defending Pokemon with 1 of his or her Benched Pokemon, if any.': 'rec:10 osw',
    'Flip a coin. If heads, discard a Fire Energy card attached to SELF and this attack does 30 damage plus 20 more damage.': 'c h.add:20 h.dsa:0,1',
    "This attack does 30 damage plus 10 damage for each Energy attached to SELF but not used to pay for this attack's Energy cost. You can't add more than 20 damage in this way.": 'addpast:15,10,2',
    'Flip 2 coins. If 1 of them is heads, this attack does 20 damage. If both are heads, this attack does 50 damage. If both are tails, the Defending Pokemon is now Confused.': 'c:2 eq1.set:20 eq2.set:50 at.zero at.cnf',
    "Choose 1 of your opponent's Benched Pokemon and switch the Defending Pokemon with it.": 'lure',
    "Choose 1 of your opponent's Benched Pokemon. Flip 3 coins. This attack does 10 damage times the number of heads to that Pokemon. Don't apply Weakness and Resistance.": 'zero c:3 b1h:10',
    'If the Defending Pokemon is an Evolved Pokemon, this attack does 20 damage plus 20 more damage.': 'devo.add:20',
    'Flip a number of coins equal to the number of Energy cards attached to the Defending Pokemon. This attack does 10 damage times the number of heads.': 'cde mulh:10',
    'Flip 3 coins. For each heads, remove 1 damage counter on SELF.': 'c:3 healh:1',
    "Flip a coin. If heads, choose 1 of your opponent's Benched Pokemon and this attack does 10 damage to it.": 'c h.b1:10',
    'Flip a coin. If heads, search your deck for a Psychic Energy card and attach it to 1 of your Benched Pokemon. Shuffle your deck afterward.': 'c h.srchb:5',
    "Flip 5 coins. This attack does 10 damage times the number of heads. SELF can't attack during your next turn.": 'c:5 mulh:10 selflock',
    'Flip 3 coins. This attack does 20 damage times the number of heads. If you get 2 or more heads, switch SELF with 1 of your Benched Pokemon, if any.': 'c:3 mulh:20 ge2.ssw',
    "Choose 1 of your opponent's Pokemon. This attack does 20 damage to that Pokemon. This attack's damage isn't affected by Weakness, Resistance, Poke-Powers, Poke-Bodies, or any other effects on that Pokemon.": 'zero any1:20',
    'Your opponent discards the top card from his or her deck. Then you remove a damage counter from SELF.': 'omill:1 heal:1',
    "Choose 1 of your opponent's Pokemon in play. This attack does 10 damage to that Pokemon. Don't apply Weakness and Resistance.": 'zero any1:10',
    "Choose 1 of your opponent's Pokemon is play. This attack does 10 damage to that Pokemon. Don't apply Weakness or Resistance for this attack.": 'zero any1:10',
    'Before doing damage, discard all Trainer cards attached to the Defending Pokemon.': 'dtrdisc',
    'Attach 1 Lightning Energy card from your discard pile to SELF.': 'attd:2,1',
    "Look at the top 3 cards of your opponent's deck. Put them back in the same order.": 'nop',
    'Flip a coin. If tails this attack does nothing.': 'c t.none',
    'Discard 1 Water Energy card attached to SELF in order to use this attack. Remove all damage counters from SELF.': 'dse:3,1 heal:99',
    "SELF does 10 damage to itself. (Don't apply Weakness or Resistance when SELF damages itself with this attack.)": 'rec:10',
    'Flip a coin. If heads, the Defending Pokemon is now Confused. Then, choose 1 of your Benched Pokemon and switch SELF with it.': 'c h.cnf ssw',
    'Flip a coin. If heads, choose whether the Defending Pokemon becomes Asleep, Confused, or Paralyzed.': 'opt:8 c h.chosc:8',
    'This attack does 30 damage plus 10 more damage for each type of Basic Energy card, if any, attached to Ho-Oh.': 'addtypes:10',
    "Flip a coin. If heads, choose 1 of your opponent's Benched Pokemon and switch the Defending Pokemon with it. The new Defending Pokemon is now Asleep.": 'c h.lure h.slp',
    "Flip a coin. If heads, discard 1 Energy card attached to 1 of your opponent's Pokemon.": 'c h.dte',
    'Flip a coin. If heads, choose a card from your discard pile and put it on top of your deck.': 'c h.distop:0',
    "Flip a coin. If heads, choose a card from your opponent's discard pile and put it on top of his or her deck.": 'c h.distop:1',
    "Put a damage counter on each of your opponent's Pokemon has already has any damage counters on it.": 'octrd',
    "If your opponent has any Benched Pokemon, choose up to 2 of them. This attack does 20 damage to each of them. SELF can't attack during your next turn.": 'bpick:20 selflock',
    'If your opponent has any Benched Pokemon, choose 2 of them (or 1 if he or she only has 1). This attack does 20 damage to each of them.': 'bpick:20',
    "Unless this attack Knocks Out the Defending Pokemon, choose 1 of your opponent's Benched Pokemon and switch it with the Defending Pokemon.": 'luremay',
    "If this attack doesn't Knock Out the Defending Pokemon, and if there are any Pokemon on your opponent's Bench, choose 1 of them and switch it with the Defending Pokemon.": 'luremay',
    "Flip a coin. If heads, switch the number of damage counters on SELF with the number of damage counters on the Defending Pokemon (even if it would Knock Out either Pokemon). (It's okay if 1 of the Pokemon has no damage counters on it.)": 'c h.swapdmg',
    'If you have any Benched Pokemon, flip a coin. If tails, choose 1 of your Benched Pokemon. This attack does 20 damage to that Pokemon.': 'c t.m1:20',
    "Flip a coin. If heads, this attack does 10 damage plus 30 more damage, and, during your opponent's next turn, prevent all effects of attacks, including damage, done to SELF. If tails, this attack does 10 damage.": 'c h.add:30 h.bar:2',
    "Move 1 damage counter from each of your Pokemon that has any on it to the Defending Pokemon. (Don't apply Weakness and Resistance.)": 'zero mvctr',
    "Flip a coin. If heads, this attack does 30 damage plus 10 more damage (to the Defending Pokemon) and 10 damage to each of your opponent's Benched Pokemon. If tails, this attack does 30 damage (to the Defending Pokemon).": 'c h.add:10 h.bo:10',
    "Flip a coin. If heads, look at the top card of your opponent's deck. If it's a Trainer card, put it in your opponent's discard pile; otherwise, put it into his or her hand.": 'c h.otop',
    'Flip a coin. If heads, remove all damage counters from 1 of your Pokemon.': 'c h.healt',
    'Does 30 damage times the number of cards with SELF in their names that you have in play (including this one).': 'mulname:30',
    'Does 10 damage plus 10 damage times the number of Water Energy cards in your discard pile. Then, shuffle all Water Energy cards from your discard pile into your deck.': 'adddis:3,10 disdeck:3',
    "Choose 1 of your opponent's Pokemon. This attack does 20 damage to that Pokemon. This attack's damage isn't affected by Weakness, Resistance, Pokemon Powers, or any other effects on the Defending Pokemon.": 'zero any1:20',
    "If an attack damaged SELF during your opponent's last turn, this attack does 20 damage plus 10 more damage. If not, this attack does 20 damage.": 'shurt.add:10',
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
        return EXACT[t] if isinstance(EXACT[t], tuple) else ('SCRIPT', EXACT[t])
    for rx, fn in F:
        m = rx.match(t)
        if m:
            out = fn(m, damage)
            if out:
                return out if isinstance(out, tuple) else ('SCRIPT', out)
    return None
