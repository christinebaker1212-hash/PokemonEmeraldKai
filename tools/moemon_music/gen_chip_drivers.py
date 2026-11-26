#!/usr/bin/env python3
# Moemon Music Studio: the sound drivers the chiptune exports carry (src/chip_drivers.h, checked in like the
# fonts; run this again after changing a driver). Each plays a register stream (src/chip.c "The stream"):
#   00-3F rr vv   write vv to sound register rr (NES: $4000 + rr; Game Boy: $FF10 + rr, the wave at $FF30-$FF3F)
#   80            jump to the loop point
#   81-FE         this frame is done; wait (byte - 0x81) more frames
#   FF            the end: silence
# The stream starts in the first bank after the driver and runs on across banks:
#   NES (NSF, 4 KB banks): the stream is read through $9000-$9FFF, bank register $5FF9; the driver is bank 0 at $8000
#   Game Boy (GBS, 16 KB banks): read through $4000-$7FFF, bank register $2000 (MBC1); the driver at $0400
# The loop point (bank, address) is patched into the driver by src/chip.c at the offsets this script reports.
# A tiny two-pass assembler for the few instructions each driver uses.
import os

here = os.path.dirname(os.path.abspath(__file__))


class Asm:
    def __init__(self, org):
        self.org = org
        self.out = []
        self.labels = {}
        self.fix = []      # (position, label, kind)
        self.marks = {}

    def pc(self):
        return self.org + len(self.out)

    def label(self, name):
        self.labels[name] = self.pc()

    def mark(self, name):          # an operand byte src/chip.c patches
        self.marks[name] = len(self.out)

    def b(self, *bs):
        self.out.extend(x & 0xFF for x in bs)

    def ref16(self, name):
        self.fix.append((len(self.out), name, 'abs'))
        self.b(0, 0)

    def rel8(self, name):
        self.fix.append((len(self.out), name, 'rel'))
        self.b(0)

    def done(self):
        for pos, name, kind in self.fix:
            t = self.labels[name]
            if kind == 'abs':
                self.out[pos] = t & 0xFF
                self.out[pos + 1] = t >> 8
            else:
                d = t - (self.org + pos + 1)
                assert -128 <= d <= 127, name
                self.out[pos] = d & 0xFF
        return bytes(self.out)


# ---------------------------------------------------------------- NES (6502), org $8000
# zero page: $F0/$F1 the stream pointer, $F2 its bank, $F3 frames to wait, $F4 stopped
def nes():
    a = Asm(0x8000)
    PTR, PTRH, BANK, WAIT, STOP = 0xF0, 0xF1, 0xF2, 0xF3, 0xF4
    a.label('init')
    a.b(0xA9, 0x01); a.b(0x85, BANK); a.b(0x8D, 0xF9, 0x5F)      # lda #1 / sta BANK / sta $5FF9
    a.b(0xA9, 0x00); a.b(0x85, PTR)                               # lda #0 / sta PTR
    a.b(0x85, WAIT); a.b(0x85, STOP)                              # sta WAIT / sta STOP
    a.b(0xA9, 0x90); a.b(0x85, PTRH)                              # lda #$90 / sta PTRH
    a.b(0xA9, 0x0F); a.b(0x8D, 0x15, 0x40)                        # lda #$0F / sta $4015: the four channels on
    a.b(0xA9, 0x40); a.b(0x8D, 0x17, 0x40)                        # lda #$40 / sta $4017: no frame IRQ
    a.b(0xA9, 0x08); a.b(0x8D, 0x01, 0x40); a.b(0x8D, 0x05, 0x40) # the sweeps off (negate set, shift 0)
    a.b(0x60)                                                     # rts
    a.label('play')
    a.b(0xA5, STOP); a.b(0xD0); a.rel8('ret')                     # lda STOP / bne ret
    a.b(0xA5, WAIT); a.b(0xF0); a.rel8('next')                    # lda WAIT / beq next
    a.b(0xC6, WAIT); a.b(0x60)                                    # dec WAIT / rts
    a.label('next')
    a.b(0x20); a.ref16('getb')                                    # jsr getb
    a.b(0xC9, 0x80); a.b(0xB0); a.rel8('special')                 # cmp #$80 / bcs special
    a.b(0xAA)                                                     # tax
    a.b(0x20); a.ref16('getb')                                    # jsr getb
    a.b(0x9D, 0x00, 0x40)                                         # sta $4000,x
    a.b(0x4C); a.ref16('next')                                    # jmp next
    a.label('special')
    a.b(0xF0); a.rel8('loop')                                     # beq loop
    a.b(0xC9, 0xFF); a.b(0xF0); a.rel8('stop')                    # cmp #$FF / beq stop
    a.b(0x38); a.b(0xE9, 0x81); a.b(0x85, WAIT)                   # sec / sbc #$81 / sta WAIT
    a.label('ret')
    a.b(0x60)
    a.label('loop')
    a.b(0xA9); a.mark('nes_loop_bank'); a.b(0x01)                 # lda #bank
    a.b(0x85, BANK); a.b(0x8D, 0xF9, 0x5F)
    a.b(0xA9); a.mark('nes_loop_lo'); a.b(0x00); a.b(0x85, PTR)   # lda #lo / sta PTR
    a.b(0xA9); a.mark('nes_loop_hi'); a.b(0x90); a.b(0x85, PTRH)  # lda #hi / sta PTRH
    a.b(0x4C); a.ref16('next')
    a.label('stop')
    a.b(0xA9, 0x01); a.b(0x85, STOP)
    a.b(0xA9, 0x00); a.b(0x8D, 0x15, 0x40)                        # every channel off
    a.b(0x60)
    # getb: A = the next byte (X kept); the pointer moves on, into the next bank at $A000
    a.label('getb')
    a.b(0xA0, 0x00); a.b(0xB1, PTR)                               # ldy #0 / lda (PTR),y
    a.b(0xE6, PTR); a.b(0xD0); a.rel8('gb_ok')                    # inc PTR / bne ok
    a.b(0xE6, PTRH)                                               # inc PTRH
    a.b(0xA4, PTRH); a.b(0xC0, 0xA0); a.b(0xD0); a.rel8('gb_ok')  # ldy PTRH / cpy #$A0 / bne ok
    a.b(0xA0, 0x90); a.b(0x84, PTRH)                              # ldy #$90 / sty PTRH
    a.b(0xE6, BANK); a.b(0xA4, BANK); a.b(0x8C, 0xF9, 0x5F)       # inc BANK / ldy BANK / sty $5FF9
    a.label('gb_ok')
    a.b(0x60)
    code = a.done()
    return code, a.labels['init'], a.labels['play'], a.marks


# ---------------------------------------------------------------- Game Boy (LR35902), org $0400
# work RAM: $C000/$C001 the stream pointer, $C002 its bank, $C003 frames to wait, $C004 stopped
def gb():
    a = Asm(0x0400)
    PTR, PTRH, BANK, WAIT, STOP = 0xC000, 0xC001, 0xC002, 0xC003, 0xC004

    def ld_mem_a(addr): a.b(0xEA, addr & 0xFF, addr >> 8)       # ld [nn],a
    def ld_a_mem(addr): a.b(0xFA, addr & 0xFF, addr >> 8)       # ld a,[nn]
    a.label('init')
    a.b(0x3E, 0x80); a.b(0xE0, 0x26)                              # ld a,$80 / ldh [$26],a: sound on
    a.b(0x3E, 0x77); a.b(0xE0, 0x24)                              # ld a,$77 / ldh [$24],a: full volume both sides
    a.b(0x3E, 0xFF); a.b(0xE0, 0x25)                              # ld a,$FF / ldh [$25],a: every channel both sides
    a.b(0x3E, 0x01); ld_mem_a(BANK); ld_mem_a(0x2000)             # ld a,1 / ld [BANK],a / ld [$2000],a
    a.b(0xAF); ld_mem_a(PTR); ld_mem_a(WAIT); ld_mem_a(STOP)      # xor a / ld [PTR],a / [WAIT] / [STOP]
    a.b(0x3E, 0x40); ld_mem_a(PTRH)                               # ld a,$40 / ld [PTRH],a
    a.b(0xC9)                                                     # ret
    a.label('play')
    ld_a_mem(STOP); a.b(0xB7); a.b(0xC0)                          # ld a,[STOP] / or a / ret nz
    ld_a_mem(WAIT); a.b(0xB7); a.b(0x28); a.rel8('next')          # ld a,[WAIT] / or a / jr z,next
    a.b(0x3D); ld_mem_a(WAIT); a.b(0xC9)                          # dec a / ld [WAIT],a / ret
    a.label('next')
    a.b(0xCD); a.ref16('getb')                                    # call getb
    a.b(0xFE, 0x80); a.b(0x30); a.rel8('special')                 # cp $80 / jr nc,special
    a.b(0xC6, 0x10); a.b(0x4F)                                    # add $10 / ld c,a
    a.b(0xCD); a.ref16('getb')                                    # call getb
    a.b(0xE2)                                                     # ld [$FF00+c],a
    a.b(0x18); a.rel8('next')                                     # jr next
    a.label('special')
    a.b(0x28); a.rel8('loop')                                     # jr z,loop
    a.b(0xFE, 0xFF); a.b(0x28); a.rel8('stop')                    # cp $FF / jr z,stop
    a.b(0xD6, 0x81); ld_mem_a(WAIT); a.b(0xC9)                    # sub $81 / ld [WAIT],a / ret
    a.label('loop')
    a.b(0x3E); a.mark('gb_loop_bank'); a.b(0x01)                  # ld a,bank
    ld_mem_a(BANK); ld_mem_a(0x2000)
    a.b(0x3E); a.mark('gb_loop_lo'); a.b(0x00); ld_mem_a(PTR)     # ld a,lo / ld [PTR],a
    a.b(0x3E); a.mark('gb_loop_hi'); a.b(0x40); ld_mem_a(PTRH)    # ld a,hi / ld [PTRH],a
    a.b(0x18); a.rel8('next')                                     # jr next
    a.label('stop')
    a.b(0x3E, 0x01); ld_mem_a(STOP)
    a.b(0xAF); a.b(0xE0, 0x25)                                    # xor a / ldh [$25],a: every channel to neither side
    a.b(0xC9)
    # getb: A = the next byte (C kept); the pointer moves on, into the next bank at $8000
    a.label('getb')
    ld_a_mem(PTR); a.b(0x6F); ld_a_mem(PTRH); a.b(0x67)           # ld a,[PTR] / ld l,a / ld a,[PTRH] / ld h,a
    a.b(0x2A); a.b(0x47)                                          # ld a,[hl+] / ld b,a
    a.b(0x7C); a.b(0xFE, 0x80); a.b(0x20); a.rel8('g_ok')         # ld a,h / cp $80 / jr nz,ok
    a.b(0x26, 0x40)                                               # ld h,$40
    ld_a_mem(BANK); a.b(0x3C); ld_mem_a(BANK); ld_mem_a(0x2000)   # ld a,[BANK] / inc a / ld [BANK],a / ld [$2000],a
    a.label('g_ok')
    a.b(0x7D); ld_mem_a(PTR); a.b(0x7C); ld_mem_a(PTRH)           # ld a,l / ld [PTR],a / ld a,h / ld [PTRH],a
    a.b(0x78); a.b(0xC9)                                          # ld a,b / ret
    code = a.done()
    return code, a.labels['init'], a.labels['play'], a.marks


def carr(name, data):
    s = "static const unsigned char %s[%d] = {\n" % (name, len(data))
    for i in range(0, len(data), 16):
        s += "    " + ", ".join("0x%02X" % x for x in data[i:i + 16]) + ",\n"
    return s + "};\n"


nc, ni, np_, nm = nes()
gc, gi, gp, gm = gb()
with open(os.path.join(here, "src", "chip_drivers.h"), "w") as f:
    f.write("// Moemon Music Studio: the chiptune exports' sound drivers (gen_chip_drivers.py; the stream they play: its top)\n")
    f.write("#define NES_DRV_ORG 0x8000\n#define NES_DRV_INIT 0x%04X\n#define NES_DRV_PLAY 0x%04X\n" % (ni, np_))
    for k, v in nm.items():
        f.write("#define %s %d\n" % (k.upper(), v))
    f.write(carr("k_nesDriver", nc))
    f.write("#define GB_DRV_ORG 0x0400\n#define GB_DRV_INIT 0x%04X\n#define GB_DRV_PLAY 0x%04X\n" % (gi, gp))
    for k, v in gm.items():
        f.write("#define %s %d\n" % (k.upper(), v))
    f.write(carr("k_gbDriver", gc))
print("src/chip_drivers.h: NES %d bytes, Game Boy %d bytes" % (len(nc), len(gc)))
