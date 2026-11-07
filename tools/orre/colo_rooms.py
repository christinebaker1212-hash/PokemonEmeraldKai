#!/usr/bin/env python3
"""Pokemon Colosseum's rooms as the game itself lays them out: every room, where a player can enter it, and where
each of its doors, stairs and elevators leads, all read from the game's own tables (the Colosseum decomp names them):

  rooms(unpacked) -> {room: Room}: the floor table's rooms (tools/orre/colo_data.py floors) with
     place   the room's place name (a common_rel message: the floor record's + 0x24, floor_data.c
             floorDataBiosGetPlaceName)
     entries [(angle, x, y, z)]: where floorLink(room, entry) puts the player (the record's + 0x18 slot,
             floorDataBiosGetPosListPtr: a FieldTable {count *, entries *} the room's module publishes in its
             _prolog; entries of 16 bytes: u16 angle in degrees, pad, f32 x y z)
     events  [Event]: the event table's rows for the room (common_rel .data + 0x146364, 0x1C each, count at
             + 0x14A704: u8 kind (1 pass, 2 touch, 3 check, 4 ...), u16 room, u32 the collision id (the trigger
             triangles' u16 id in the room's .col), u32 script, u32 args x4; floor_character.c heroDispatchEvent runs
             the script with the args when the hero passes / touches / checks that trigger)
     links   {collision id: Link}: where its triggers lead (below)
     gates   collision objects the room's scripts switch on and off (scriptSetCol, native 0x76: story gates)

  A link is one of (the shared script bank 0x0596, common_script_0596.json in the decomp):
     0596:4  link_floor_with_sound(room, entry, sound): a door, a staircase, a way out; room 910 = the world map
     0596:6  leave_elevator(elevator, room, elevator there, alternate): arriving at the other elevator
     0596:12 REALGAM TOWER's lift (the decomp's leave_elevator_for_room_123): it rides through room 123, whose own
             script carries on to the event's (room, entry): taken here as a link straight there
     0100:n  a function of the room's own script (.scd) that calls floorLink / floorLinkWithSE: those rooms choose
             between a place's versions by the story (PYRITE's colosseum lobby, M2_enter_1F or M2_enter_1F_2): the
             event's own (room, entry) arguments when they name another field room, else (they name the room
             itself: THE UNDER's lift up to PYRITE) the first room the function links to
  Doors that open (0596:5 control_door) are touch triggers in front of a door; the pass trigger behind it links.

    python3 -I tools/orre/colo_rooms.py UNPACKED_DIR [ROOM...]     (prints the rooms and their links)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import colo_data  # noqa: E402

EVENTS, EVENT_COUNT, EVENT_SIZE = 0x146364, 0x14A704, 0x1C
WORLD_MAP = 910
KIND_FIELD = 1     # the floor table's kinds: 1 a field room, 2 a stadium's battle stage, 5 / 6 menus and scenes
PASS, TOUCH, CHECK = 1, 2, 3
BANK_COMMON, BANK_ROOM = 0x0596, 0x0100
FN_LINK, FN_DOOR, FN_ELEVATOR, FN_LIFT = 4, 5, 6, 12

# the native functions the room scripts call to change rooms (main.dol's native table lbl_802E1CF0, by index)
NATIVE_FLOOR_LINK, NATIVE_FLOOR_LINK_SE, NATIVE_SET_COL = 0x5F, 0x7D, 0x76


# ---- REL modules (Dolphin's format: the decomp's docs/REL_MODULES.md)

def rel_sections(d):
    n, st = struct.unpack_from(">II", d, 0x0C)
    return [(struct.unpack_from(">I", d, st + 8 * i)[0] & ~3, struct.unpack_from(">I", d, st + 8 * i + 4)[0])
            for i in range(n)]


def rel_relocs(d):
    """{(section, offset): (module, type, target section, addend)}: every relocation the module carries."""
    imp, impsz = struct.unpack_from(">II", d, 0x28)
    out = {}
    for k in range(impsz // 8):
        mod, off = struct.unpack_from(">II", d, imp + 8 * k)
        sec, pos = None, 0
        while True:
            o, t, s, a = struct.unpack_from(">HBBI", d, off)
            off += 8
            if t == 203:        # R_DOLPHIN_END
                break
            pos += o
            if t == 202:        # R_DOLPHIN_SECTION
                sec, pos = s, 0
            elif t != 201:      # (R_DOLPHIN_NOP)
                out[(sec, pos)] = (mod, t, s, a)
    return out


def rel_prolog_stores(d):
    """{main.dol address: (section, offset)}: what the module's _prolog stores where (the floor modules publish their
    tables into main.dol's .sbss this way: lis / addi of a table, lis / stw into a slot, every address relocated)."""
    secs, rel = rel_sections(d), rel_relocs(d)
    me = struct.unpack_from(">I", d, 0)[0]
    psec, poff = d[0x30], struct.unpack_from(">I", d, 0x34)[0]
    base, regs, out = secs[psec][0], {}, {}
    for o in range(poff, poff + 0x1000, 4):
        ins = struct.unpack_from(">I", d, base + o)[0]
        if ins == 0x4E800020:   # blr
            break
        op, rd, ra = ins >> 26, (ins >> 21) & 31, (ins >> 16) & 31
        r = rel.get((psec, o + 2))
        val = None if r is None else (("self", r[2], r[3]) if r[0] == me else ("main", r[3]))
        if op in (14, 15) and val:          # addi / lis of an address
            regs[rd] = val
        elif op == 14 and ra in regs:
            regs[rd] = regs[ra]
        elif op == 31 and ((ins >> 1) & 0x3FF) == 444 and rd in regs:   # mr
            regs[ra] = regs[rd]
        elif op == 36 and val and val[0] == "main" and rd in regs:      # stw into a slot
            out[val[1]] = regs[rd][1:]
    return out


# ---- room scripts (.scd, the GS VM: the decomp's src/game/gs_vm.c)

OP_LEN = {0: 0, 1: 0, 2: 0, 3: 3, 4: 8, 5: 0, 6: 4, 7: 4, 8: 5, 13: 2, 14: 2, 15: 1, 16: 3, 17: 1, 18: 1, 19: 1,
          31: 2, 32: 4, 33: 2, 34: 2, 35: 4, 36: 4, 37: 4}
OP_LEN.update({k: 2 for k in range(20, 31)})


def scd_native_calls(d):
    """[(function, native, args)]: every native call of a script bank whose arguments are pushed as constants (None
    where an argument is computed). A script: u16 functions at + 4, their offsets at + 0x18; code: an opcode byte and
    its operands (OP_LEN); op 8 pushes a constant, op 37 calls the native whose index is its first argument."""
    nf = struct.unpack_from(">H", d, 4)[0]
    offs = list(struct.unpack_from(">%dI" % nf, d, 0x18))
    ends = sorted(set(offs + [len(d)]))
    out = []
    for f, o in enumerate(offs):
        end = min(e for e in ends if e > o)
        ip, st = o, []
        while ip < end:
            op = d[ip]
            ip += 1
            if op not in OP_LEN:
                break
            if op == 8:
                st.append(struct.unpack_from(">I", d, ip + 1)[0])
            elif op == 37:
                argc = struct.unpack_from(">H", d, ip + 2)[0]
                args = st[-argc:] if argc and len(st) >= argc else [None] * argc
                if args and args[0] is not None:
                    out.append((f, args[0], args[1:]))
                del st[len(st) - argc:]
                st.append(None)
            elif op == 16:
                if st:
                    st.pop()
                st.append(None)
            elif op == 3:
                n = struct.unpack_from(">H", d, ip + 1)[0]
                del st[max(0, len(st) - n):]
            ip += OP_LEN[op]
    return out


# ---- the rooms

class Room:
    def __init__(self, room, file, area, kind, place):
        self.room, self.file, self.area, self.kind, self.place = room, file, area, kind, place
        self.entries, self.events, self.links, self.gates = [], [], {}, set()

    def __repr__(self):
        return "Room(%d %s)" % (self.room, self.file)


def _member(unpacked, file, ext):
    p = os.path.join(unpacked, file, file + "." + ext)
    return open(p, "rb").read() if os.path.exists(p) else None


def rooms(unpacked):
    data = colo_data._data(unpacked)
    common = open(os.path.join(unpacked, "common", "common_rel.rel"), "rb").read()
    crel = rel_relocs(common)
    out = {}
    for f in colo_data.floors(unpacked):
        rec = colo_data.FLOOR_TABLE + f["index"] * colo_data.FLOOR_SIZE
        r = Room(f["room"], f["file"], f["area"], f["kind"], struct.unpack_from(">I", data, rec + 0x24)[0])
        out[r.room] = r
        slot = crel.get((colo_data.DATA_SECTION, rec + 0x18))
        d = _member(unpacked, r.file, "rel") if r.file else None
        if slot is None or slot[0] != 0 or d is None:
            continue
        stores = rel_prolog_stores(d)
        if slot[3] not in stores:
            continue
        sec, desc = stores[slot[3]]
        secs, rr = rel_sections(d), rel_relocs(d)
        cnt, ent = rr[(sec, desc)], rr[(sec, desc + 4)]
        n = struct.unpack_from(">I", d, secs[cnt[2]][0] + cnt[3])[0]
        base = secs[ent[2]][0] + ent[3]
        for k in range(n):
            ang, x, y, z = struct.unpack_from(">hxxfff", d, base + 16 * k)
            r.entries.append((ang, x, y, z))
    n = struct.unpack_from(">I", data, EVENT_COUNT)[0]
    for i in range(n):
        kind, room, col, script, a, b, c, e = struct.unpack_from(">BxHIIIIII", data, EVENTS + i * EVENT_SIZE)
        if room in out:
            out[room].events.append((kind, col, script, (a, b, c, e)))
    for r in out.values():
        _links(unpacked, r, out)
    return out


def _links(unpacked, r, all_rooms):
    d = _member(unpacked, r.file, "scd") if r.file else None
    calls = scd_native_calls(d) if d else []
    r.gates = {a[0] for f, nat, a in calls if nat == NATIVE_SET_COL and a and a[0] is not None}
    for kind, col, script, args in r.events:
        bank, fn = script >> 16, script & 0xFFFF
        if bank == BANK_COMMON and fn == FN_LINK and kind == PASS:
            r.links.setdefault(col, ("room", args[0], args[1]))
        elif bank == BANK_COMMON and fn == FN_LIFT:
            r.links.setdefault(col, ("room", args[0], args[1]))
        elif bank == BANK_COMMON and fn == FN_ELEVATOR:
            r.links.setdefault(col, ("elevator", args[1], args[2], args[0]))
        elif bank == BANK_ROOM and kind in (PASS, TOUCH):
            linked = [a for f, nat, a in calls if f == fn and nat in (NATIVE_FLOOR_LINK, NATIVE_FLOOR_LINK_SE)]
            rooms_called = [a[0] for a in linked if a and a[0] is not None]
            if args[0] in all_rooms and all_rooms[args[0]].kind == KIND_FIELD and args[0] != r.room:
                r.links.setdefault(col, ("room", args[0], args[1]))
            elif rooms_called:
                a = [a for a in linked if a and a[0] is not None][0]
                r.links.setdefault(col, ("room", a[0], a[1] if len(a) > 1 and a[1] is not None else 0))


def link_target(all_rooms, link):
    """(room, entry) or (room, ("elevator", n)) or (WORLD_MAP, 0): where a link puts the player."""
    if link[0] == "room":
        return link[1], link[2]
    return link[1], ("elevator", link[2])


def field_rooms(all_rooms):
    """The rooms a player walks in: field rooms with a map and somewhere to enter (not the event copies)."""
    return [r for r in sorted(all_rooms.values(), key=lambda r: r.room)
            if r.kind == KIND_FIELD and r.file and r.entries]


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    rs = rooms(sys.argv[1])
    only = {int(a, 0) for a in sys.argv[2:]}
    text = colo_data.common_text(sys.argv[1])
    for r in sorted(rs.values(), key=lambda r: r.room):
        if (only and r.room not in only) or (not only and r.kind != KIND_FIELD):
            continue
        print("%3d %-20s area %2d kind %d %s, %d entries" % (r.room, r.file, r.area, r.kind,
              colo_data.plain(text.get(r.place, [])) if r.place else "-", len(r.entries)))
        for col, l in sorted(r.links.items()):
            print("      collision %2d -> %s" % (col, l))
