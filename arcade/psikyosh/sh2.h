// Moemon Kai: a Hitachi SH-2 (the SH7604 of Psikyo's PS5V2 board, TETRIS THE GRAND MASTER 2): an interpreter of the
// whole SH-2 instruction set with its delay slots, the external interrupt levels (autovectored, 64 + level / 2), the
// cache-through and on-chip RAM spaces and the on-chip registers the board's game uses (the divide unit; the rest
// kept as written). The instructions' fine points (DIV1, MAC.L / MAC.W's saturation, the DIVU's overflow) follow MAME's
// sh.cpp / sh7604.cpp (BSD-3-Clause, R. Belmont, Juergen Buchmueller and others); the code is this game's own.
#pragma once

#include <cstdint>

namespace psikyosh {

struct Sh2Bus {
    virtual ~Sh2Bus() = default;
    virtual uint8_t Read8(uint32_t address) = 0;
    virtual uint16_t Read16(uint32_t address) = 0;
    virtual uint32_t Read32(uint32_t address) = 0;
    virtual void Write8(uint32_t address, uint8_t value) = 0;
    virtual void Write16(uint32_t address, uint16_t value) = 0;
    virtual void Write32(uint32_t address, uint32_t value) = 0;
};

class Sh2 {
public:
    explicit Sh2(Sh2Bus &bus)
        : m_bus(bus) {}
    void Reset();
    // The external interrupt request level (IRL, 0: none); taken while above the mask in SR
    void SetIrqLevel(int level) {
        s.irl = static_cast<uint8_t>(level);
    }
    // Runs until `cycles` reaches `until`
    void Run(uint64_t until);

    // What a save state holds (plain data)
    struct State {
        uint32_t r[16], sr, gbr, vbr, mach, macl, pr, pc;
        uint32_t delayTarget;   // a delayed branch's destination (with inDelay)
        uint8_t inDelay, sleeping, irl, pad;
        uint64_t cycles;
        uint8_t onchipRam[0x1000];
        uint32_t dvsr, dvdnth, dvdntl, dvcr;
        uint8_t regs[0x200];    // FFFFFE00-FFFFFFFF as written
    } s;

private:
    Sh2Bus &m_bus;

    uint8_t Rb(uint32_t a);
    uint16_t Rw(uint32_t a);
    uint32_t Rl(uint32_t a);
    void Wb(uint32_t a, uint8_t v);
    void Ww(uint32_t a, uint16_t v);
    void Wl(uint32_t a, uint32_t v);
    uint32_t OnchipRead(uint32_t a, int size);
    void OnchipWrite(uint32_t a, uint32_t v, int size);
    void DivuStart(int64_t dividend);
    void Exception(int vector, int level);
    void Execute(uint16_t op, uint32_t pc);
    void Delay(uint32_t target);
};

} // namespace psikyosh
