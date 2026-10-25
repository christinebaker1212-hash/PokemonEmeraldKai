"""Kai's text conventions for descriptions taken from other games."""
import re

STATS = re.compile(r'(?<![.!?]\s)(?<!^)\b(Sp\. ?Atk|Sp\. ?Def|Sp\. Attack|Sp\. Defense|Attack|Defense|Speed|'
                   r'Evasion|Evasiveness|Accuracy)\b')


def kai_text(s):
    """MOéMON for Pokémon, stat names in capitals (SP. ATK, SPEED) like Kai's own descriptions."""
    s = s.replace('Pokémon', 'MOéMON')
    return STATS.sub(lambda m: m.group(1).upper().replace('SP.ATK', 'SP. ATK').replace('SP.DEF', 'SP. DEF'), s)
