#!/usr/bin/env python3
"""Pokemon Colosseum's Shadow Pokemon as Kai's data (docs/SHADOW_POKEMON.md), read from the game's own tables in
common_rel (the decomp names them: christinebaker1212-hash/pkmn-colosseum), never typed in:

  the Shadow table      .data + 0x1439D8, records of 0x38 (count + 0x14A6F8; battle_range_801ED640.c BattleRangeDef,
                        ids 1-0x60): + 0 the catch rate, + 2 the species (Gen 3's internal numbers: Kai's), + 4 / + 6
                        the trainers that hold it (the first meeting and the second chance; 0 none), + 8 the heart
                        gauge's size (battle_main.c copies it into the save as the Pokemon's starting DP), + 0xC .. + 0x1C
                        the story flags of its states (met, snagged, purified: darkPokemonCheckFlag0C / 12 / 14 / 16)
  the trainers          .data + 0x91684, 0x34 each (count + 0x14A6AC; fightTrainerDataBiosGetPtr): + 4 their first
                        Pokemon in the trainer Pokemon table
  the trainer Pokemon   .data + 0x9E5DC, 0x50 each (count + 0x14A6B4; fightTrainerPokemonDataBios*): + 0 the Ability slot,
                        + 1 gender, + 2 nature (255 any), + 3 the Shadow id it is, + 4 level, + 8 friendship,
                        + 0xA species, + 0xC ball, + 0x10 item, + 0x1C IVs x6, + 0x22 EVs x6 (u16), + 0x30 four moves
                        (8 bytes each: u8 PP ups, the move at + 4)
  the DP sources        .data + 0x1219FC, s8 each (count + 0x14A6DC; pokemonDpFilterDataBios*): 0 battling, 1 walking
                        (every 256 steps: field_exact_801ED3B8.c), 2 CALL, 3 the day care, 4 a scent (times its
                        strength), 5 a share of what's left
  the natures           .data + 0x121614, 0x28 each (count + 0x14A6D8; PokemonSeikakuData): + 0 .. + 4 the rate (an index
                        into the rates) for sources 0-4, + 0x0B .. + 0x10 the chances (%) of Hyper Mode's ways of
                        ignoring orders (fight_range_8022A504.c: a move of its own choosing, attacking the opposing
                        trainer, attacking its own, attacking itself, too confused to act, trying its item), + 0x18
                        Hyper Mode's chance by gauge level (0 full .. 6 empty), + 0x1F CALL's chance by gauge level
  the rates             .data + 0x13E430, (kake, waru) pairs (count + 0x14A6E4): a source's drop times kake / waru

A drop (pokemon_range_8011F5FC.c pokemonAddDpFormPokemonDpFilterId): DP += source * kake / waru, never below 0. The
gauge's level (pokemon_range_8011F77C.c pokemonGetDarkPokemonLevel): DP / gauge >= 100% 0, >= 80% 1, >= 60% 2,
>= 40% 3, >= 20% 4, > 0 5, else 6 (ready to purify); 7 not Shadow.

    python3 -I tools/orre/shadow_data.py UNPACKED_DIR [OUT.h]    (default src/data/orre_shadow.h)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import colo_data  # noqa: E402

SHADOW, SHADOW_COUNT, SHADOW_SIZE = 0x1439D8, 0x14A6F8, 0x38
TRAINER, TRAINER_COUNT, TRAINER_SIZE = 0x91684, 0x14A6AC, 0x34
TMON, TMON_COUNT, TMON_SIZE = 0x9E5DC, 0x14A6B4, 0x50
DPF, DPF_COUNT = 0x1219FC, 0x14A6DC
NATURE, NATURE_COUNT, NATURE_SIZE = 0x121614, 0x14A6D8, 0x28
RATE, RATE_COUNT = 0x13E430, 0x14A6E4


def _u32(d, o):
    return struct.unpack_from(">I", d, o)[0]


def read(unpacked):
    d = colo_data._data(unpacked)
    n_tr, n_tm = _u32(d, TRAINER_COUNT), _u32(d, TMON_COUNT)

    def trainer_mon(tr, sid):
        if tr == 0 or tr >= n_tr:
            return None
        first = struct.unpack_from(">H", d, TRAINER + tr * TRAINER_SIZE + 4)[0]
        for k in range(first, min(first + 6, n_tm)):
            o = TMON + k * TMON_SIZE
            if d[o + 3] == sid:
                return o
        return None

    shadows = []
    for sid in range(1, _u32(d, SHADOW_COUNT)):
        r = SHADOW + sid * SHADOW_SIZE
        species, tr1, tr2, gauge = struct.unpack_from(">HHHH", d, r + 2)
        if species == 0:
            continue   # (the table's spare rows)
        o = trainer_mon(tr1, sid) or trainer_mon(tr2, sid)
        if o is None:
            raise SystemExit("Shadow %d (species %d): no trainer holds it" % (sid, species))
        flags = [struct.unpack_from(">H", d, r + k)[0] for k in (0xC, 0xE, 0x12, 0x14, 0x16, 0x1C)]
        shadows.append({
            "id": sid, "catch": d[r], "species": species, "gauge": gauge,
            "trainers": (tr1 if trainer_mon(tr1, sid) else 0, tr2 if trainer_mon(tr2, sid) else 0),
            "level": d[o + 4], "ability": d[o], "gender": d[o + 1], "nature": d[o + 2],
            "friendship": struct.unpack_from(">h", d, o + 8)[0], "item": struct.unpack_from(">i", d, o + 0x10)[0],
            "ivs": list(d[o + 0x1C:o + 0x22]), "evs": list(struct.unpack_from(">6H", d, o + 0x22)),
            "moves": [_u32(d, o + 0x30 + 8 * i + 4) for i in range(4)], "flags": flags})
    dpf = [struct.unpack_from(">b", d, DPF + i)[0] for i in range(_u32(d, DPF_COUNT))]
    rates = [tuple(d[RATE + 2 * i:RATE + 2 * i + 2]) for i in range(_u32(d, RATE_COUNT))]
    natures = []
    for i in range(_u32(d, NATURE_COUNT)):
        o = NATURE + i * NATURE_SIZE
        natures.append({"rate": list(d[o:o + 5]), "hyper": list(d[o + 0x18:o + 0x1F]), "call": list(d[o + 0x1F:o + 0x26]),
                        "disobey": list(d[o + 0x0B:o + 0x11])})
    return shadows, dpf, rates, natures


def write(out, shadows, dpf, rates, natures):
    L = ["// Generated by tools/orre/shadow_data.py from Pokemon Colosseum's own tables (common_rel): its Shadow Pokemon,",
         "// the heart gauge's sources and each nature's rates (docs/SHADOW_POKEMON.md). Don't edit: rerun the tool.",
         "// Included once, by src/orre/orre_shadow.c.", ""]
    L.append("static const struct OrreShadowMon sOrreShadows[] = {")
    for s in shadows:
        mv = [m if 0 < m < 0x10000 else 0 for m in s["moves"]]
        L.append("    {%d, %d, %d, %d, %d, {%d, %d}, %d, %d, %d, %d, %d, {%s}, {%s}, {%s}, {%s}},   // %s" % (
            s["id"], s["species"], s["catch"], s["gauge"], s["level"], s["trainers"][0], s["trainers"][1],
            s["nature"], s["gender"] & 0xFF, s["ability"], s["friendship"], max(0, s["item"]),
            ", ".join(map(str, mv)), ", ".join(map(str, s["ivs"])), ", ".join(map(str, s["evs"])),
            ", ".join("0x%X" % f for f in s["flags"]), "Shadow %d" % s["id"]))
    L.append("};")
    L.append("")
    L.append("// the heart gauge's sources (ORRE_DP_*): each one's drop")
    L.append("static const s8 sOrreDpSource[%d] = {%s};" % (len(dpf), ", ".join(map(str, dpf))))
    L.append("// a drop's scale by rate: kake / waru")
    L.append("static const u8 sOrreDpRate[%d][2] = {%s};" % (len(rates), ", ".join("{%d, %d}" % r for r in rates)))
    L.append("// per nature (Kai's nature order is the same): the rate for sources 0-4, Hyper Mode's chance and CALL's by")
    L.append("// the gauge's level (0 full .. 6 empty)")
    L.append("static const struct OrreShadowNature sOrreShadowNatures[%d] = {" % len(natures))
    for i, n in enumerate(natures):
        L.append("    {{%s}, {%s}, {%s}, {%s}},   // nature %d" % (", ".join(map(str, n["rate"])), ", ".join(map(str, n["hyper"])),
                                                               ", ".join(map(str, n["call"])), ", ".join(map(str, n["disobey"])), i))
    L.append("};")
    open(out, "w").write("\n".join(L) + "\n")


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        print(__doc__)
        sys.exit(1)
    sh, dpf, rates, nat = read(sys.argv[1])
    out = sys.argv[2] if len(sys.argv) == 3 else "src/data/orre_shadow.h"
    write(out, sh, dpf, rates, nat)
    print("%d Shadow Pokemon (%d species), %d sources, %d rates, %d natures -> %s" % (
        len(sh), len({s["species"] for s in sh}), len(dpf), len(rates), len(nat), out))
