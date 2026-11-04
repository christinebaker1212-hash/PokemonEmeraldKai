#!/usr/bin/env python3
"""Pokemon Colosseum's message tables (.msg): every string with its key, controls named.

Layout: u16 count at 0x04, then (key u32, offset u32) entries from 0x10; strings are UTF-16BE ending in 0, and
0xFFFF starts a control: one id byte, then the control's parameters. The ids, names and parameter sizes are the
game's own (the Colosseum decomp: src/game/data/data_80363774.c msgctrlcode, the handlers in src/game/msgctrl*.c).

  python3 tools/orre/msg.py FILE.msg     (prints key: text)
"""
import struct
import sys

# id: (name, parameter bytes)
CONTROLS = {
    0: ("br", 0), 1: ("br", 0), 2: ("end", 0), 3: ("wait_key", 0), 4: ("ruby", 0), 5: ("ruby_top", 0),
    6: ("ruby_end", 0), 7: ("font", 1), 8: ("color", 4), 9: ("wait", 1),
    13: ("str0", 0), 14: ("str1", 0), 15: ("attack_mon", 0), 16: ("defence_mon", 0), 17: ("client_mon", 0),
    18: ("added_mon", 0), 19: ("my_name", 0), 20: ("my_mon", 0), 21: ("my_mon2", 0), 22: ("enemy_mon", 0),
    23: ("enemy_mon2", 0), 24: ("enemy_tmon", 0), 25: ("enemy_tmon2", 0), 26: ("ability_a", 0),
    27: ("ability_d", 0), 28: ("ability_c", 0), 29: ("ability_t", 0), 30: ("client_no_work", 0),
    31: ("side_attack_ha", 0), 32: ("side_attack_wo", 0), 33: ("side_attack_no", 0), 34: ("trainer_type", 0),
    35: ("trainer_name", 0), 36: ("trainer_lose", 0), 37: ("trainer_enemy", 0), 38: ("trainer_enemy2", 0),
    39: ("trainer_client_no", 0), 40: ("move_name", 0), 41: ("item_name", 0), 42: ("paso_name", 0),
    43: ("hero", 0), 44: ("rui", 0), 45: ("item", 0), 46: ("item2", 0), 47: ("digit", 0), 48: ("digit2", 0),
    49: ("msg_id", 0), 50: ("pokemon", 0), 51: ("pokemon2", 0), 52: ("menu_digit", 0), 53: ("menu_digit2", 0),
    54: ("menu_pokemon", 0), 55: ("menu_msg", 0), 56: ("palette", 1), 57: ("move", 0), 61: ("se_off", 0),
    62: ("se_on", 0), 65: ("str2", 0), 66: ("side_defence_ha", 0), 67: ("side_defence_wo", 0),
    68: ("side_defence_no", 0), 69: ("menu_udigit", 0), 70: ("menu_udigit2", 0), 71: ("menu_hex", 0),
    72: ("menu_hex2", 0), 73: ("menu_zdigit", 0), 74: ("menu_zdigit2", 0), 75: ("money", 0), 76: ("time", 0),
    77: ("string", 0), 78: ("pokemon_id", 0), 79: ("menu_full_digit", 0), 80: ("menu_money", 0),
    81: ("menu_msg2", 0), 82: ("shadow", 1), 83: ("align", 1), 84: ("talk_se", 1), 85: ("menu_msg_id", 0),
    # ids with no handler in the game's table (10-12, 58-60, 63, 64): read as controls without parameters
    10: ("ctl10", 0), 11: ("ctl11", 0), 12: ("ctl12", 0), 58: ("ctl58", 0), 59: ("ctl59", 0), 60: ("ctl60", 0),
    63: ("ctl63", 0), 64: ("ctl64", 0),
    86: ("menu_msg_id2", 0), 87: ("string2", 0), 88: ("tribe", 0), 89: ("npc", 0), 90: ("indent_off", 0),
    91: ("line_space", 1), 92: ("baseline", 1), 93: ("snd_play", 0), 94: ("snd_wait", 0),
}


class MsgError(ValueError):
    pass


def parse_string(d, o):
    """-> list of parts: str runs and (control name, params bytes) tuples; and the end offset."""
    parts = []
    run = []
    while True:
        if o + 2 > len(d):
            raise MsgError("string runs off the end")
        c = struct.unpack_from(">H", d, o)[0]
        o += 2
        if c == 0:
            break
        if c != 0xFFFF:
            run.append(chr(c))
            continue
        cid = d[o]
        o += 1
        if cid not in CONTROLS:
            raise MsgError("unknown control %d at 0x%x" % (cid, o - 1))
        name, n = CONTROLS[cid]
        if run:
            parts.append("".join(run))
            run = []
        parts.append((name, bytes(d[o:o + n])))
        o += n
    if run:
        parts.append("".join(run))
    return parts, o


def parse(d):
    """{key: parts} in table order."""
    count = struct.unpack_from(">H", d, 4)[0]
    out = {}
    for i in range(count):
        key, off = struct.unpack_from(">II", d, 0x10 + i * 8)
        out[key] = parse_string(d, off)[0]
    return out


def show(parts):
    s = []
    for p in parts:
        if isinstance(p, str):
            s.append(p)
        else:
            name, prm = p
            s.append("\n" if name == "br" else "{%s%s}" % (name, (":" + prm.hex()) if prm else ""))
    return "".join(s)


if __name__ == "__main__":
    for k, v in parse(open(sys.argv[1], "rb").read()).items():
        print("%d: %s" % (k, show(v).replace("\n", "\\n")))
