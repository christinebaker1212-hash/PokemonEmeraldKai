// Moemon Kai: the SH-2 interpreter (sh2.h).
#include "sh2.h"

#include <cstring>
#include <limits>

namespace psikyosh {

namespace {
constexpr uint32_t T = 0x001, S = 0x002, I = 0x0F0, Q = 0x100, M = 0x200, kSrMask = 0x3F3;
inline uint32_t Get32(const uint8_t *p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
}
inline void Put32(uint8_t *p, uint32_t v) {
    p[0] = uint8_t(v >> 24), p[1] = uint8_t(v >> 16), p[2] = uint8_t(v >> 8), p[3] = uint8_t(v);
}
} // namespace

void Sh2::Reset() {
    memset(&s, 0, sizeof(s));
    s.sr = I;
    s.pc = Rl(0);
    s.r[15] = Rl(4);
}

// --- memory: 0x0 / 0x2 the outside (cached / cache-through), 0x4 cache purges, 0xC0000000 the on-chip RAM (the
// cache's data array), 0xFFFFFE00 the on-chip registers ---

uint32_t Sh2::OnchipRead(uint32_t a, int size) {
    if ((a & 0xFFFFFFC0) == 0xFFFFFF00) {
        switch (a & 0x1C) {   // the divide unit (and its mirror at FF20)
        case 0x00: return s.dvsr;
        case 0x04: case 0x14: case 0x1C: return s.dvdntl;
        case 0x08: return s.dvcr;
        case 0x10: case 0x18: return s.dvdnth;
        default: break;
        }
    }
    const uint32_t o = a - 0xFFFFFE00;
    if (size == 1)
        return s.regs[o];
    if (size == 2)
        return uint32_t(s.regs[o] << 8) | s.regs[o + 1];
    return Get32(&s.regs[o & ~3u]);
}

void Sh2::OnchipWrite(uint32_t a, uint32_t v, int size) {
    const uint32_t o = a - 0xFFFFFE00;
    if ((a & 0xFFFFFFC0) == 0xFFFFFF00 && size == 4) {
        switch (a & 0x1C) {
        case 0x00: s.dvsr = v; return;
        case 0x04: s.dvdntl = v; DivuStart(int32_t(v)); return;   // (32 bits: sign-extended)
        case 0x08: s.dvcr = v & 3; return;
        case 0x10: s.dvdnth = v; return;
        case 0x14: s.dvdntl = v; DivuStart(int64_t((uint64_t(s.dvdnth) << 32) | v)); return;
        default: break;
        }
    }
    if (size == 1)
        s.regs[o] = uint8_t(v);
    else if (size == 2)
        s.regs[o] = uint8_t(v >> 8), s.regs[o + 1] = uint8_t(v);
    else
        Put32(&s.regs[o & ~3u], v);
}

// The divide unit (SH7604 manual 10.3.3, as MAME's divu_start): an overflow leaves the partial remainder and the
// quotient at its limit
void Sh2::DivuStart(int64_t dividend) {
    const int32_t divisor = int32_t(s.dvsr);
    if (divisor && (dividend != std::numeric_limits<int64_t>::min() || divisor != -1)) {
        const int64_t quotient = dividend / divisor;
        if (quotient == int32_t(quotient)) {
            s.dvdntl = uint32_t(quotient);
            s.dvdnth = uint32_t(dividend % divisor);
            return;
        }
    }
    int64_t remainder = dividend >> 32;
    uint32_t quotient = uint32_t(dividend);
    for (int i = 0; i < 3; i++) {
        const bool subtract = (remainder < 0) == (divisor < 0);
        remainder = (remainder << 1) | (quotient >> 31);
        quotient <<= 1;
        remainder += subtract ? -int64_t(divisor) : int64_t(divisor);
        quotient |= ((remainder < 0) == (divisor < 0)) ? 1 : 0;
    }
    const bool negative = (dividend < 0) != (divisor < 0);
    s.dvdnth = uint32_t(remainder);
    s.dvdntl = (s.dvcr & 2) ? quotient : negative ? 0x80000000u : 0x7FFFFFFFu;
    s.dvcr |= 1;
}

uint8_t Sh2::Rb(uint32_t a) {
    switch (a >> 29) {
    case 0: case 1: return m_bus.Read8(a & 0x1FFFFFFF);
    case 6: return s.onchipRam[a & 0xFFF];
    case 7: if (a >= 0xFFFFFE00) return uint8_t(OnchipRead(a, 1)); break;
    default: break;
    }
    return 0;
}

uint16_t Sh2::Rw(uint32_t a) {
    a &= ~1u;
    switch (a >> 29) {
    case 0: case 1: return m_bus.Read16(a & 0x1FFFFFFF);
    case 6: return uint16_t((s.onchipRam[a & 0xFFF] << 8) | s.onchipRam[(a + 1) & 0xFFF]);
    case 7: if (a >= 0xFFFFFE00) return uint16_t(OnchipRead(a, 2)); break;
    default: break;
    }
    return 0;
}

uint32_t Sh2::Rl(uint32_t a) {
    a &= ~3u;
    switch (a >> 29) {
    case 0: case 1: return m_bus.Read32(a & 0x1FFFFFFF);
    case 6: return Get32(&s.onchipRam[a & 0xFFC]);
    case 7: if (a >= 0xFFFFFE00) return OnchipRead(a, 4); break;
    default: break;
    }
    return 0;
}

void Sh2::Wb(uint32_t a, uint8_t v) {
    switch (a >> 29) {
    case 0: case 1: m_bus.Write8(a & 0x1FFFFFFF, v); break;
    case 6: s.onchipRam[a & 0xFFF] = v; break;
    case 7: if (a >= 0xFFFFFE00) OnchipWrite(a, v, 1); break;
    default: break;   // (cache purges, the address array)
    }
}

void Sh2::Ww(uint32_t a, uint16_t v) {
    a &= ~1u;
    switch (a >> 29) {
    case 0: case 1: m_bus.Write16(a & 0x1FFFFFFF, v); break;
    case 6: s.onchipRam[a & 0xFFF] = uint8_t(v >> 8), s.onchipRam[(a + 1) & 0xFFF] = uint8_t(v); break;
    case 7: if (a >= 0xFFFFFE00) OnchipWrite(a, v, 2); break;
    default: break;
    }
}

void Sh2::Wl(uint32_t a, uint32_t v) {
    a &= ~3u;
    switch (a >> 29) {
    case 0: case 1: m_bus.Write32(a & 0x1FFFFFFF, v); break;
    case 6: Put32(&s.onchipRam[a & 0xFFC], v); break;
    case 7: if (a >= 0xFFFFFE00) OnchipWrite(a, v, 4); break;
    default: break;
    }
}

// --- running ---

void Sh2::Exception(int vector, int level) {
    if (s.sleeping) {   // (back to the instruction after the SLEEP)
        s.sleeping = 0;
        s.pc += 2;
    }
    s.r[15] -= 4;
    Wl(s.r[15], s.sr);
    s.r[15] -= 4;
    Wl(s.r[15], s.pc);
    if (level >= 0)
        s.sr = (s.sr & ~I) | (uint32_t(level) << 4);
    s.pc = Rl(s.vbr + uint32_t(vector) * 4);
    s.cycles += 13;
}

void Sh2::Run(uint64_t until) {
    while (s.cycles < until) {
        if (s.irl && int(s.irl) > int((s.sr >> 4) & 15))
            Exception(64 + s.irl / 2, s.irl);   // (autovectored: the board's IRL lines)
        if (s.sleeping) {
            s.cycles = until;
            break;
        }
        const uint32_t pc = s.pc;
        Execute(Rw(pc), pc);
        s.cycles++;
    }
}

// The instruction after a delayed branch, then the branch
void Sh2::Delay(uint32_t target) {
    const uint32_t slot = s.pc;   // (the branch's address + 2)
    s.inDelay = 1;
    Execute(Rw(slot), slot);
    s.cycles++;
    s.inDelay = 0;
    s.pc = target;
}

void Sh2::Execute(uint16_t op, uint32_t pc) {
    uint32_t *r = s.r;
    const int n = (op >> 8) & 15, m = (op >> 4) & 15;
    const int32_t d8 = int8_t(op & 0xFF);
    const uint32_t i8 = op & 0xFF;
    s.pc = pc + 2;
    auto setT = [&](bool b) { s.sr = b ? (s.sr | T) : (s.sr & ~T); };
    switch (op >> 12) {
    case 0x0:
        switch (op & 0xF) {
        case 0x2:
            if (m == 0) r[n] = s.sr; else if (m == 1) r[n] = s.gbr; else if (m == 2) r[n] = s.vbr;
            return;
        case 0x3:
            if (m == 0) { s.pr = pc + 4; Delay(pc + 4 + r[n]); }       // BSRF
            else if (m == 2) Delay(pc + 4 + r[n]);                        // BRAF
            return;
        case 0x4: Wb(r[n] + r[0], uint8_t(r[m])); return;
        case 0x5: Ww(r[n] + r[0], uint16_t(r[m])); return;
        case 0x6: Wl(r[n] + r[0], r[m]); return;
        case 0x7: s.macl = r[n] * r[m]; s.cycles++; return;               // MUL.L
        case 0x8:
            if (m == 0) s.sr &= ~T; else if (m == 1) s.sr |= T; else if (m == 2) s.mach = s.macl = 0;
            return;
        case 0x9:
            if (m == 1) s.sr &= ~(M | Q | T);                             // DIV0U
            else if (m == 2) r[n] = s.sr & T;                             // MOVT
            return;                                                       // (NOP)
        case 0xA:
            if (m == 0) r[n] = s.mach; else if (m == 1) r[n] = s.macl; else if (m == 2) r[n] = s.pr;
            return;
        case 0xB:
            if (m == 0) { Delay(s.pr); }                                  // RTS
            else if (m == 1) { s.sleeping = 1; s.pc = pc; }               // SLEEP
            else if (m == 2) {                                            // RTE
                const uint32_t target = Rl(r[15]);
                r[15] += 4;
                s.sr = Rl(r[15]) & kSrMask;
                r[15] += 4;
                Delay(target);
            }
            return;
        case 0xC: r[n] = uint32_t(int8_t(Rb(r[m] + r[0]))); return;
        case 0xD: r[n] = uint32_t(int16_t(Rw(r[m] + r[0]))); return;
        case 0xE: r[n] = Rl(r[m] + r[0]); return;
        case 0xF: {                                                       // MAC.L @Rm+,@Rn+
            int32_t tn = int32_t(Rl(r[n]));
            r[n] += 4;
            int32_t tm = int32_t(Rl(r[m]));
            r[m] += 4;
            const int64_t product = int64_t(tn) * int64_t(tm);
            int64_t sum = int64_t((uint64_t(s.mach) << 32) | s.macl) + product;
            if (s.sr & S) {
                const uint64_t u = uint64_t(sum);
                if (u > 0x00007FFFFFFFFFFFULL && u < 0xFFFF800000000000ULL)
                    sum = product < 0 ? int64_t(0xFFFF800000000000ULL) : int64_t(0x00007FFFFFFFFFFFULL);
            }
            s.mach = uint32_t(uint64_t(sum) >> 32);
            s.macl = uint32_t(sum);
            s.cycles += 2;
            return;
        }
        default: break;
        }
        break;
    case 0x1: Wl(r[n] + (op & 0xF) * 4, r[m]); return;
    case 0x2:
        switch (op & 0xF) {
        case 0x0: Wb(r[n], uint8_t(r[m])); return;
        case 0x1: Ww(r[n], uint16_t(r[m])); return;
        case 0x2: Wl(r[n], r[m]); return;
        case 0x4: { const uint32_t v = r[m]; r[n] -= 1; Wb(r[n], uint8_t(v)); return; }
        case 0x5: { const uint32_t v = r[m]; r[n] -= 2; Ww(r[n], uint16_t(v)); return; }
        case 0x6: { const uint32_t v = r[m]; r[n] -= 4; Wl(r[n], v); return; }
        case 0x7:                                                         // DIV0S
            s.sr = (s.sr & ~(Q | M | T)) | ((r[n] >> 31) ? Q : 0) | ((r[m] >> 31) ? M : 0);
            setT(((r[n] ^ r[m]) >> 31) != 0);
            return;
        case 0x8: setT((r[n] & r[m]) == 0); return;
        case 0x9: r[n] &= r[m]; return;
        case 0xA: r[n] ^= r[m]; return;
        case 0xB: r[n] |= r[m]; return;
        case 0xC: {                                                       // CMP/STR
            const uint32_t x = r[n] ^ r[m];
            setT(!(x & 0xFF000000) || !(x & 0x00FF0000) || !(x & 0x0000FF00) || !(x & 0x000000FF));
            return;
        }
        case 0xD: r[n] = (r[n] >> 16) | (r[m] << 16); return;              // XTRCT
        case 0xE: s.macl = uint32_t(uint16_t(r[n])) * uint16_t(r[m]); return;
        case 0xF: s.macl = uint32_t(int32_t(int16_t(r[n])) * int16_t(r[m])); return;
        default: break;
        }
        break;
    case 0x3:
        switch (op & 0xF) {
        case 0x0: setT(r[n] == r[m]); return;
        case 0x2: setT(r[n] >= r[m]); return;
        case 0x3: setT(int32_t(r[n]) >= int32_t(r[m])); return;
        case 0x4: {                                                       // DIV1 (as MAME's)
            const uint32_t oldQ = s.sr & Q;
            s.sr = (r[n] & 0x80000000) ? (s.sr | Q) : (s.sr & ~Q);
            r[n] = (r[n] << 1) | (s.sr & T);
            const uint32_t tmp = r[n];
            const bool add = (oldQ != 0) != ((s.sr & M) != 0);
            if (!add) r[n] -= r[m]; else r[n] += r[m];
            const bool carry = add ? (r[n] < tmp) : (r[n] > tmp);
            // the new Q from the old Q, M and the carry / borrow
            bool q = (s.sr & Q) != 0;
            if (!oldQ) {
                if (!(s.sr & M)) q = q ? !carry : carry;
                else q = q ? carry : !carry;
            } else {
                if (!(s.sr & M)) q = q ? !carry : carry;
                else q = q ? carry : !carry;
            }
            s.sr = q ? (s.sr | Q) : (s.sr & ~Q);
            setT(((s.sr & Q) != 0) == ((s.sr & M) != 0));
            return;
        }
        case 0x5: { const uint64_t p = uint64_t(r[n]) * r[m]; s.mach = uint32_t(p >> 32); s.macl = uint32_t(p); s.cycles++; return; }
        case 0x6: setT(r[n] > r[m]); return;
        case 0x7: setT(int32_t(r[n]) > int32_t(r[m])); return;
        case 0x8: r[n] -= r[m]; return;
        case 0xA: {                                                       // SUBC
            const uint32_t t0 = r[n], t1 = t0 - r[m], t2 = t1 - (s.sr & T);
            setT(t0 < t1 || t1 < t2);
            r[n] = t2;
            return;
        }
        case 0xB: {                                                       // SUBV
            const int32_t a = int32_t(r[n]), b = int32_t(r[m]), res = int32_t(uint32_t(a) - uint32_t(b));
            setT(((a ^ b) & (a ^ res)) < 0);
            r[n] = uint32_t(res);
            return;
        }
        case 0xC: r[n] += r[m]; return;
        case 0xD: { const int64_t p = int64_t(int32_t(r[n])) * int32_t(r[m]); s.mach = uint32_t(uint64_t(p) >> 32); s.macl = uint32_t(p); s.cycles++; return; }
        case 0xE: {                                                       // ADDC
            const uint32_t t1 = r[n] + r[m], t0 = r[n], res = t1 + (s.sr & T);
            setT(t0 > t1 || t1 > res);
            r[n] = res;
            return;
        }
        case 0xF: {                                                       // ADDV
            const int32_t a = int32_t(r[n]), b = int32_t(r[m]), res = int32_t(uint32_t(a) + uint32_t(b));
            setT(((~(a ^ b)) & (a ^ res)) < 0);
            r[n] = uint32_t(res);
            return;
        }
        default: break;
        }
        break;
    case 0x4:
        if ((op & 0xF) == 0xF) {                                          // MAC.W @Rm+,@Rn+ (as MAME's)
            int32_t tn = int16_t(Rw(r[n]));
            r[n] += 2;
            int32_t tm = int16_t(Rw(r[m]));
            r[m] += 2;
            const uint32_t oldL = s.macl;
            tm *= tn;
            const int dest = int(s.macl >> 31), src = int(uint32_t(tm) >> 31) + dest;
            s.macl += uint32_t(tm);
            const int ans = int(s.macl >> 31) + dest;
            if (s.sr & S) {
                if (ans == 1) {
                    if (src == 0) { s.macl = 0x7FFFFFFF; s.mach |= 1; }
                    else if (src == 2) { s.macl = 0x80000000; s.mach |= 1; }
                }
            } else {
                s.mach += tm < 0 ? 0xFFFFFFFFu : 0;
                if (oldL > s.macl)
                    s.mach += 1;
            }
            s.cycles += 2;
            return;
        }
        switch (op & 0xFF) {
        case 0x00: setT(r[n] >> 31); r[n] <<= 1; return;                 // SHLL
        case 0x01: setT(r[n] & 1); r[n] >>= 1; return;                   // SHLR
        case 0x02: r[n] -= 4; Wl(r[n], s.mach); return;
        case 0x03: r[n] -= 4; Wl(r[n], s.sr); return;
        case 0x04: setT(r[n] >> 31); r[n] = (r[n] << 1) | (r[n] >> 31); return;
        case 0x05: setT(r[n] & 1); r[n] = (r[n] >> 1) | (r[n] << 31); return;
        case 0x06: s.mach = Rl(r[n]); r[n] += 4; return;
        case 0x07: s.sr = Rl(r[n]) & kSrMask; r[n] += 4; return;
        case 0x08: r[n] <<= 2; return;
        case 0x09: r[n] >>= 2; return;
        case 0x0A: s.mach = r[n]; return;
        case 0x0B: { const uint32_t target = r[n]; s.pr = pc + 4; Delay(target); return; }   // JSR
        case 0x0E: s.sr = r[n] & kSrMask; return;
        case 0x10: r[n]--; setT(r[n] == 0); return;                       // DT
        case 0x11: setT(int32_t(r[n]) >= 0); return;
        case 0x12: r[n] -= 4; Wl(r[n], s.macl); return;
        case 0x13: r[n] -= 4; Wl(r[n], s.gbr); return;
        case 0x15: setT(int32_t(r[n]) > 0); return;
        case 0x16: s.macl = Rl(r[n]); r[n] += 4; return;
        case 0x17: s.gbr = Rl(r[n]); r[n] += 4; return;
        case 0x18: r[n] <<= 8; return;
        case 0x19: r[n] >>= 8; return;
        case 0x1A: s.macl = r[n]; return;
        case 0x1B: { const uint8_t v = Rb(r[n]); setT(v == 0); Wb(r[n], v | 0x80); s.cycles += 3; return; }   // TAS.B
        case 0x1E: s.gbr = r[n]; return;
        case 0x20: setT(r[n] >> 31); r[n] <<= 1; return;                 // SHAL
        case 0x21: setT(r[n] & 1); r[n] = uint32_t(int32_t(r[n]) >> 1); return;
        case 0x22: r[n] -= 4; Wl(r[n], s.pr); return;
        case 0x23: r[n] -= 4; Wl(r[n], s.vbr); return;
        case 0x24: { const uint32_t t = s.sr & T; setT(r[n] >> 31); r[n] = (r[n] << 1) | t; return; }   // ROTCL
        case 0x25: { const uint32_t t = s.sr & T; setT(r[n] & 1); r[n] = (r[n] >> 1) | (t << 31); return; }
        case 0x26: s.pr = Rl(r[n]); r[n] += 4; return;
        case 0x27: s.vbr = Rl(r[n]); r[n] += 4; return;
        case 0x28: r[n] <<= 16; return;
        case 0x29: r[n] >>= 16; return;
        case 0x2A: s.pr = r[n]; return;
        case 0x2B: Delay(r[n]); return;                                   // JMP
        case 0x2E: s.vbr = r[n]; return;
        default: break;
        }
        break;
    case 0x5: r[n] = Rl(r[m] + (op & 0xF) * 4); return;
    case 0x6:
        switch (op & 0xF) {
        case 0x0: r[n] = uint32_t(int8_t(Rb(r[m]))); return;
        case 0x1: r[n] = uint32_t(int16_t(Rw(r[m]))); return;
        case 0x2: r[n] = Rl(r[m]); return;
        case 0x3: r[n] = r[m]; return;
        case 0x4: { const uint32_t v = uint32_t(int8_t(Rb(r[m]))); if (n != m) r[m] += 1; r[n] = v; return; }
        case 0x5: { const uint32_t v = uint32_t(int16_t(Rw(r[m]))); if (n != m) r[m] += 2; r[n] = v; return; }
        case 0x6: { const uint32_t v = Rl(r[m]); if (n != m) r[m] += 4; r[n] = v; return; }
        case 0x7: r[n] = ~r[m]; return;
        case 0x8: r[n] = (r[m] & 0xFFFF0000) | ((r[m] & 0xFF) << 8) | ((r[m] >> 8) & 0xFF); return;
        case 0x9: r[n] = (r[m] << 16) | (r[m] >> 16); return;
        case 0xA: {                                                       // NEGC
            const uint32_t t = 0 - r[m], res = t - (s.sr & T);
            setT(t != 0 || res > t);
            r[n] = res;
            return;
        }
        case 0xB: r[n] = 0 - r[m]; return;
        case 0xC: r[n] = r[m] & 0xFF; return;
        case 0xD: r[n] = r[m] & 0xFFFF; return;
        case 0xE: r[n] = uint32_t(int8_t(r[m])); return;
        case 0xF: r[n] = uint32_t(int16_t(r[m])); return;
        default: break;
        }
        break;
    case 0x7: r[n] += uint32_t(d8); return;
    case 0x8:
        switch (n) {
        case 0x0: Wb(r[m] + (op & 0xF), uint8_t(r[0])); return;
        case 0x1: Ww(r[m] + (op & 0xF) * 2, uint16_t(r[0])); return;
        case 0x4: r[0] = uint32_t(int8_t(Rb(r[m] + (op & 0xF)))); return;
        case 0x5: r[0] = uint32_t(int16_t(Rw(r[m] + (op & 0xF) * 2))); return;
        case 0x8: setT(r[0] == uint32_t(d8)); return;
        case 0x9: if (s.sr & T) { s.pc = pc + 4 + uint32_t(d8 * 2); s.cycles += 2; } return;   // BT
        case 0xB: if (!(s.sr & T)) { s.pc = pc + 4 + uint32_t(d8 * 2); s.cycles += 2; } return; // BF
        case 0xD: if (s.sr & T) Delay(pc + 4 + uint32_t(d8 * 2)); return;   // BT/S
        case 0xF: if (!(s.sr & T)) Delay(pc + 4 + uint32_t(d8 * 2)); return; // BF/S
        default: break;
        }
        break;
    case 0x9: r[n] = uint32_t(int16_t(Rw(pc + 4 + i8 * 2))); return;
    case 0xA: case 0xB: {
        int32_t d12 = op & 0xFFF;
        if (d12 & 0x800)
            d12 -= 0x1000;
        if (op >> 12 == 0xB)
            s.pr = pc + 4;
        Delay(pc + 4 + uint32_t(d12 * 2));
        return;
    }
    case 0xC:
        switch (n) {
        case 0x0: Wb(s.gbr + i8, uint8_t(r[0])); return;
        case 0x1: Ww(s.gbr + i8 * 2, uint16_t(r[0])); return;
        case 0x2: Wl(s.gbr + i8 * 4, r[0]); return;
        case 0x3: {                                                       // TRAPA
            r[15] -= 4;
            Wl(r[15], s.sr);
            r[15] -= 4;
            Wl(r[15], pc + 2);
            s.pc = Rl(s.vbr + i8 * 4);
            s.cycles += 7;
            return;
        }
        case 0x4: r[0] = uint32_t(int8_t(Rb(s.gbr + i8))); return;
        case 0x5: r[0] = uint32_t(int16_t(Rw(s.gbr + i8 * 2))); return;
        case 0x6: r[0] = Rl(s.gbr + i8 * 4); return;
        case 0x7: r[0] = ((pc + 4) & ~3u) + i8 * 4; return;               // MOVA
        case 0x8: setT((r[0] & i8) == 0); return;
        case 0x9: r[0] &= i8; return;
        case 0xA: r[0] ^= i8; return;
        case 0xB: r[0] |= i8; return;
        case 0xC: setT((Rb(s.gbr + r[0]) & i8) == 0); return;
        case 0xD: Wb(s.gbr + r[0], uint8_t(Rb(s.gbr + r[0]) & i8)); return;
        case 0xE: Wb(s.gbr + r[0], uint8_t(Rb(s.gbr + r[0]) ^ i8)); return;
        case 0xF: Wb(s.gbr + r[0], uint8_t(Rb(s.gbr + r[0]) | i8)); return;
        default: break;
        }
        break;
    case 0xD: r[n] = Rl(((pc + 4) & ~3u) + i8 * 4); return;
    case 0xE: r[n] = uint32_t(d8); return;
    default: break;
    }
    // an illegal instruction (vector 4; 6 in a delay slot)
    s.pc = pc;
    Exception(s.inDelay ? 6 : 4, -1);
}

} // namespace psikyosh
