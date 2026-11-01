// Moemon Kai: the Psikyo PS5V2 board for TETRIS THE GRAND MASTER 2 (ps5.h). Hardware as MAME describes it
// (psikyosh.cpp's ps5_map and psikyo5_240 machine, psikyosh_v.cpp's PS6406B, eepromser.cpp's 93C56); the code is this
// game's own.
#include "ps5.h"

#include <algorithm>
#include <cstring>

#include "arcade.h"

using namespace cps2recomp;

namespace psikyosh {

namespace {

constexpr int kVblankLine = 240;
constexpr uint32_t kCyclesPerSample = 768;     // (the YMF278B: clock / 768)
constexpr uint32_t kTiles4 = 0x3000000 / 128, kTiles8 = 0x3000000 / 256;
// the EEPROM's states and commands (MAME's eeprom_serial_base_device)
enum : uint8_t { kEepReset, kEepStart, kEepCommand, kEepReading, kEepData, kEepCompletion };
enum : uint8_t { kCmdNone, kCmdRead, kCmdWrite, kCmdErase, kCmdLock, kCmdUnlock, kCmdWriteAll, kCmdEraseAll };
constexpr uint64_t kUs = Board::kCpuClock / 1000000;
constexpr uint64_t kEepWriteTime = 1750 * kUs, kEepEraseTime = 1000 * kUs, kEepAllTime = 8000 * kUs;

inline uint32_t Pal6(uint32_t v) {
    return (v << 2) | (v >> 4);
}

// MAME's alpha_blend_r32: s over d by level (0-255)
inline uint32_t Blend(uint32_t d, uint32_t s, uint32_t level) {
    return ((((s & 0xFF00FF) * level + (d & 0xFF00FF) * (256 - level)) >> 8) & 0xFF00FF)
         | ((((s & 0x00FF00) * level + (d & 0x00FF00) * (256 - level)) >> 8) & 0x00FF00);
}

// 0x00RRGGBB (the line tables') to 0x00BBGGRR
inline uint32_t Bgr(uint32_t rgb) {
    return ((rgb >> 16) & 0xFF) | (rgb & 0xFF00) | ((rgb & 0xFF) << 16);
}

// The 16x16 pictures: 4 bits a pixel packed (the left pixel in the top half of the byte) or 8 bits
inline uint8_t Pen4(const uint8_t *g, uint32_t tile, uint32_t x, uint32_t y) {
    const uint8_t b = g[tile * 128 + y * 8 + (x >> 1)];
    return (x & 1) ? (b & 15) : (b >> 4);
}
inline uint8_t Pen8(const uint8_t *g, uint32_t tile, uint32_t x, uint32_t y) {
    return g[tile * 256 + y * 16 + x];
}

// The background zoom (MAME's, "verified against hardware"): a line's source step in .10 fixed point
struct BgZoom {
    uint16_t step[256];
    BgZoom() {
        for (int i = 0; i < 256; i++)
            step[i] = static_cast<uint16_t>((64 * 0x400) / (i + 64));
    }
};
const BgZoom kBgZoom;

// Pens 0xC0-0xFF of an 8-bit picture fade out (per-pen alpha)
struct AlphaTable {
    uint8_t level[256];
    AlphaTable() {
        for (int i = 0; i < 0xC0; i++)
            level[i] = 0xFF;
        for (int i = 0; i < 0x40; i++)
            level[0xC0 + i] = static_cast<uint8_t>(Pal6(0x3F - i));
    }
};
const AlphaTable kAlpha;

} // namespace

Board::Board(Roms roms)
    : m_roms(std::move(roms))
    , m_cpu(*this)
    , m_ym(*this)
    , m_s(std::make_unique<State>()) {
    m_roms.program.resize(0x100000, 0);
    m_roms.graphics.resize(0x3000000, 0);
    m_roms.samples.resize(0x400000, 0);
    m_roms.eeprom.resize(0x100, 0xFF);
    m_program.resize(0x40000);
    for (size_t i = 0; i < m_program.size(); i++) {
        const uint8_t *p = &m_roms.program[i * 4];
        m_program[i] = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
    }
    m_picture.assign(kWidth * kHeight, 0);
    m_z.assign(kWidth * kHeight, 0);
    Reset();
}

void Board::Reset() {
    State &s = *m_s;
    memset(&s, 0, sizeof(s));
    memcpy(s.eep, m_roms.eeprom.data(), sizeof(s.eep));
    s.eepLocked = 1;
    s.eepState = kEepReset;
    s.ymTimerAt[0] = s.ymTimerAt[1] = -1;
    s.nextSampleAt = kCyclesPerSample;
    m_cpu.Reset();
    m_ym.reset();
    audio.clear();
}

// --- the bus (big-endian 32 bits: the program, the RAM and the video chip's memories as 32-bit words) ---

uint32_t Board::IoRead(uint32_t address) {
    // 03000000: the controls (all active low); 03000004: JP4, the EEPROM's DO in bit 28
    if (address < 0x03000004)
        return m_inputs;
    if (address < 0x03000008)
        return EepromDataOut() ? 0x10000000 : 0;
    return 0;
}

uint32_t Board::Read32(uint32_t address) {
    State &s = *m_s;
    switch (address >> 24) {
    case 0x00:
        return address < 0x100000 ? m_program[address >> 2] : 0;
    case 0x06:
        return address < 0x06100000 ? s.ram[(address & 0xFFFFF) >> 2] : 0;
    case 0x04: {
        const uint32_t o = address & 0xFFFFFF;
        if (o < 0x10000)
            return s.vram[o >> 2];
        if (o >= 0x40000 && o < 0x45000)
            return s.palette[(o - 0x40000) >> 2];
        if (o >= 0x50000 && o < 0x50200)
            return s.zoom[(o - 0x50000) >> 2];
        if (o >= 0x5FFE0 && o < 0x60000)
            return s.vidregs[(o - 0x5FFE0) >> 2];
        if (o >= 0x60000 && o < 0x80000) {
            // the graphics ROMs a 128 KB bank at a time (vidregs[4]), for the ROM test; as MAME's bank on a
            // little-endian host reads them
            const uint64_t at = uint64_t(s.vidregs[4] & 0xFFF) * 0x20000 + ((o - 0x60000) & ~3u);
            if (at + 4 > m_roms.graphics.size())
                return 0;
            const uint8_t *p = &m_roms.graphics[at];
            return (uint32_t(p[3]) << 24) | (uint32_t(p[2]) << 16) | (uint32_t(p[1]) << 8) | p[0];
        }
        return 0;
    }
    case 0x03:
        if (address >= 0x03100000 && address < 0x03100008) {
            uint32_t v = 0;
            for (int i = 0; i < 4; i++)
                v = (v << 8) | m_ym.read((address & 4) + i);
            return v;
        }
        return IoRead(address & ~3u);
    default:
        return 0;
    }
}

uint16_t Board::Read16(uint32_t address) {
    if ((address >> 24) == 0x03 && address >= 0x03100000 && address < 0x03100008)
        return static_cast<uint16_t>((m_ym.read(address & 6) << 8) | m_ym.read((address & 6) + 1));
    return static_cast<uint16_t>(Read32(address & ~3u) >> ((~address & 2) * 8));
}

uint8_t Board::Read8(uint32_t address) {
    if ((address >> 24) == 0x03 && address >= 0x03100000 && address < 0x03100008)
        return m_ym.read(address & 7);
    return static_cast<uint8_t>(Read32(address & ~3u) >> ((~address & 3) * 8));
}

// A write of the lanes in `mask` of the 32-bit word at `address` (& ~3)
void Board::WriteMasked(uint32_t address, uint32_t v, uint32_t mask) {
    State &s = *m_s;
    auto merge = [&](uint32_t &word) { word = (word & ~mask) | (v & mask); };
    switch (address >> 24) {
    case 0x06:
        if (address < 0x06100000)
            merge(s.ram[(address & 0xFFFFF) >> 2]);
        return;
    case 0x04: {
        const uint32_t o = address & 0xFFFFFC;
        if (o < 0x10000)
            merge(s.vram[o >> 2]);
        else if (o >= 0x40000 && o < 0x45000)
            merge(s.palette[(o - 0x40000) >> 2]);
        else if (o >= 0x50000 && o < 0x50200)
            merge(s.zoom[(o - 0x50000) >> 2]);
        else if (o >= 0x5FFE0 && o < 0x60000)
            merge(s.vidregs[(o - 0x5FFE0) >> 2]);
        else if (o == 0x5FFDC && !(v & mask & 0x00C00000)) {
            // irqctrl: the vblank interrupt is let go by a write without bits 22-23 (the lanes not written: 0)
            s.vblankIrq = 0;
            UpdateIrq();
        }
        return;
    }
    case 0x03:
        if (address >= 0x03100000 && address < 0x03100008) {
            for (int i = 0; i < 4; i++)
                if (mask & (0xFF000000u >> (8 * i)))
                    m_ym.write((address & 4) + i, static_cast<uint8_t>(v >> (24 - 8 * i)));
        } else if ((address & ~3u) == 0x03000004 && (mask & 0xFF000000u)) {
            EepromWrite(static_cast<uint8_t>(v >> 24));
        }
        return;
    default:
        return;
    }
}

void Board::Write32(uint32_t address, uint32_t value) {
    if ((address >> 24) == 0x06) {
        if (address < 0x06100000)
            m_s->ram[(address & 0xFFFFF) >> 2] = value;
        return;
    }
    WriteMasked(address & ~3u, value, 0xFFFFFFFFu);
}

void Board::Write16(uint32_t address, uint16_t value) {
    const int shift = (~address & 2) * 8;
    WriteMasked(address & ~3u, uint32_t(value) << shift, 0xFFFFu << shift);
}

void Board::Write8(uint32_t address, uint8_t value) {
    const int shift = (~address & 3) * 8;
    WriteMasked(address & ~3u, uint32_t(value) << shift, 0xFFu << shift);
}

// --- the 93C56 (8-bit mode: 256 cells, 9 command address bits), as MAME's eeprom_serial_93cxx ---

bool Board::EepromDataOut() const {
    const State &s = *m_s;
    const bool ready = m_cpu.s.cycles >= s.eepReadyAt;
    if (s.eepState == kEepStart)
        return ready;
    if (s.eepState == kEepReading)
        return (s.eepShift & 0x80000000u) != 0;
    return true;   // (tristated, pulled up)
}

// Bit 5 data in, bit 6 clock, bit 7 chip select (in that order, as MAME's eeprom_w)
void Board::EepromWrite(uint8_t data) {
    State &s = *m_s;
    s.eepDi = (data >> 5) & 1;
    s.eepCsRose = 0;
    const uint8_t cs = (data >> 7) & 1, clk = (data >> 6) & 1;
    if (cs != s.eepCs) {
        s.eepCs = cs;
        if (cs) {
            s.eepCsRose = 1;
            if (s.eepState == kEepReset)
                s.eepState = kEepStart;
        } else {
            s.eepState = kEepReset;   // (a falling CS resets every state)
        }
    }
    if (clk != s.eepClk) {
        s.eepClk = clk;
        if (clk)
            EepromClock();
    }
}

void Board::EepromClock() {
    State &s = *m_s;
    const uint64_t now = m_cpu.s.cycles;
    switch (s.eepState) {
    case kEepStart:
        // (a clock on the same write as the rising CS is not one)
        if (s.eepDi && now >= s.eepReadyAt && !s.eepCsRose) {
            s.eepAccum = s.eepBits = 0;
            s.eepState = kEepCommand;
        }
        break;
    case kEepCommand:
        s.eepAccum = (s.eepAccum << 1) | s.eepDi;
        if (++s.eepBits == 2 + 9) {
            s.eepAddress = s.eepAccum & 0x1FF;
            s.eepBits = 0;
            switch (s.eepAccum >> 9) {
            case 0: {
                const uint32_t op = s.eepAddress >> 7;
                s.eepCommand = op == 0 ? kCmdLock : op == 1 ? kCmdWriteAll : op == 2 ? kCmdEraseAll : kCmdUnlock;
                s.eepAddress = 0;
                break;
            }
            case 1: s.eepCommand = kCmdWrite; break;
            case 2: s.eepCommand = kCmdRead; break;
            default: s.eepCommand = kCmdErase; break;
            }
            switch (s.eepCommand) {
            case kCmdRead:
                s.eepShift = 0;
                s.eepState = kEepReading;
                break;
            case kCmdWrite: case kCmdWriteAll:
                s.eepShift = 0;
                s.eepState = kEepData;
                break;
            case kCmdErase:
                if (s.eepLocked) {
                    s.eepState = kEepReset;
                    break;
                }
                s.eep[s.eepAddress & 0xFF] = 0xFF;
                s.eepReadyAt = now + kEepEraseTime;
                s.eepState = kEepCompletion;
                break;
            case kCmdEraseAll:
                if (s.eepLocked) {
                    s.eepState = kEepReset;
                    break;
                }
                memset(s.eep, 0xFF, sizeof(s.eep));
                s.eepReadyAt = now + kEepAllTime;
                s.eepState = kEepCompletion;
                break;
            case kCmdLock: s.eepLocked = 1; s.eepState = kEepReset; break;
            default: s.eepLocked = 0; s.eepState = kEepReset; break;
            }
        }
        break;
    case kEepReading: {
        const uint32_t bit = s.eepBits++;
        if (bit == 0)
            s.eepShift = uint32_t(s.eep[s.eepAddress & 0xFF]) << 24;
        else
            s.eepShift = (s.eepShift << 1) | 1;
        break;
    }
    case kEepData:
        s.eepShift = (s.eepShift << 1) | s.eepDi;
        if (++s.eepBits == 8) {
            if (s.eepLocked) {
                s.eepState = kEepReset;
                break;
            }
            if (s.eepCommand == kCmdWrite) {
                s.eep[s.eepAddress & 0xFF] = static_cast<uint8_t>(s.eepShift);
                s.eepReadyAt = now + kEepWriteTime;
            } else {
                for (uint8_t &b : s.eep)
                    b &= static_cast<uint8_t>(s.eepShift);
                s.eepReadyAt = now + kEepAllTime;
            }
            s.eepState = kEepCompletion;
        }
        break;
    default:
        break;
    }
}

// --- interrupts and the YMF278B ---

void Board::UpdateIrq() {
    const State &s = *m_s;
    m_cpu.SetIrqLevel(s.ymIrq ? 12 : s.vblankIrq ? 4 : 0);
}

void Board::ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) {
    if (tnum < 2)
        m_s->ymTimerAt[tnum] = duration_in_clocks < 0 ? -1 : int64_t(m_cpu.s.cycles) + duration_in_clocks;
}

void Board::ymfm_update_irq(bool asserted) {
    m_s->ymIrq = asserted;
    UpdateIrq();
}

uint8_t Board::ymfm_external_read(ymfm::access_class type, uint32_t address) {
    if (type != ymfm::ACCESS_PCM)
        return 0;
    return m_roms.samples[address & 0x3FFFFF];
}

void Board::Sample() {
    ymfm::ymf278b::output_data out;
    m_ym.generate(&out);
    // the board has one speaker (MAME: all six outputs into it; the game pans its voices as if it had two)
    // (x4: ymfm's OPL4 at its own level peaks near 4000, a quarter of the other cabinets' boards)
    const int32_t v = std::clamp((out.data[0] + out.data[1] + out.data[2] + out.data[3] + out.data[4] + out.data[5]) * 4,
                                 -32768, 32767);
    audio.push_back(static_cast<int16_t>(v));
    audio.push_back(static_cast<int16_t>(v));
}

// The SH-2 up to `until`, stopping at each YMF278B sample and timer
void Board::RunCpu(uint64_t until) {
    State &s = *m_s;
    while (m_cpu.s.cycles < until) {
        uint64_t stop = std::min<uint64_t>(until, s.nextSampleAt);
        for (int t = 0; t < 2; t++)
            if (s.ymTimerAt[t] >= 0)
                stop = std::min<uint64_t>(stop, uint64_t(std::max<int64_t>(s.ymTimerAt[t], 0)));
        if (stop > m_cpu.s.cycles)
            m_cpu.Run(stop);
        const uint64_t now = m_cpu.s.cycles;
        for (int t = 0; t < 2; t++)
            if (s.ymTimerAt[t] >= 0 && now >= uint64_t(s.ymTimerAt[t])) {
                s.ymTimerAt[t] = -1;
                m_engine->engine_timer_expired(t);
            }
        while (now >= s.nextSampleAt) {
            Sample();
            s.nextSampleAt += kCyclesPerSample;
        }
    }
}

// --- the PS6406B's picture (MAME's psikyosh_v.cpp screen_update) ---

void Board::PreLineBlend() {
    const State &s = *m_s;
    const uint32_t bank = s.vidregs[7] >> 24;
    for (int y = 0; y < kHeight; y++) {
        const uint32_t c = bank > 0x1F ? 0 : Bgr(s.vram[bank * 0x200 + y] >> 8);
        std::fill(&m_picture[y * kWidth], &m_picture[(y + 1) * kWidth], c);
    }
}

void Board::PostLineBlend(int priority) {
    const State &s = *m_s;
    if (int(s.vidregs[2] & 0xF) != priority)
        return;
    const uint32_t bank = s.vidregs[7] >> 24;
    if (bank > 0x1F)
        return;
    for (int y = 0; y < kHeight; y++) {
        const uint32_t v = s.vram[bank * 0x200 + 0x100 + y], c = Bgr(v >> 8);
        uint32_t *row = &m_picture[y * kWidth];
        if (v & 0x80)
            std::fill(row, row + kWidth, c);
        else if (v & 0x7F)
            for (int x = 0; x < kWidth; x++)
                row[x] = Blend(row[x], c, (v & 0x7F) << 1);
    }
}

// One background layer's lines of this priority (registers per layer or per line, in the bank vidregs[6] names)
void Board::DrawLayer(int layer, int priority) {
    const State &s = *m_s;
    const uint32_t lineBank = ((s.vidregs[6] << (8 * layer)) & 0x7F000000) >> 24;
    if (lineBank > 0x1F)
        return;
    const uint32_t ctl = s.vidregs[7] << (4 * layer);
    const bool perLine = ((s.vidregs[6] << (8 * layer)) & 0x80000000) != 0;
    const bool deep = (ctl & 0x4000) != 0;
    const int height = (ctl & 0x1000) ? 512 : 256;
    const uint32_t *scrollReg = &s.vram[lineBank * 0x200], *pzabReg = scrollReg + 0x100;
    const uint8_t *g = m_roms.graphics.data();
    for (int y = 0; y < kHeight; y++) {
        const uint32_t reg = (perLine ? uint32_t(y) : 0xFCu + layer) & 0xFF;
        const uint32_t pzab = pzabReg[reg];
        if (int(pzab >> 24) != priority)
            continue;
        const uint32_t tileBank = pzab & 0xFF;
        if (tileBank < 0x0A || tileBank > 0x1F)
            continue;
        const uint32_t scrollX = scrollReg[reg] & 0x1FF, scrollY = (scrollReg[reg] >> 16) & 0x1FF;
        const uint32_t step = kBgZoom.step[(pzab >> 16) & 0xFF];
        const bool alphaMap = (pzab & 0x8000) != 0;
        int alpha = (pzab >> 8) & 0x3F;
        if (!alphaMap)
            alpha = int(Pal6(0x3F - alpha));
        const uint32_t srcY = (uint32_t(y) - scrollY) & (height - 1);
        const uint32_t *tiles = &s.vram[((tileBank + (srcY >> 8)) & 0x1F) * 0x200 + ((srcY & 0xFF) >> 4) * 32];
        const uint32_t ty = srcY & 15;
        uint32_t *row = &m_picture[y * kWidth];
        uint32_t xcnt = 0;
        for (int x = 0; x < kWidth; x++, xcnt += step) {
            const uint32_t srcX = ((xcnt >> 10) - scrollX) & 0x1FF;
            const uint32_t data = tiles[srcX >> 4];
            const uint32_t tile = data & 0x7FFFF;
            const uint8_t pix = deep ? Pen8(g, tile % kTiles8, srcX & 15, ty) : Pen4(g, tile % kTiles4, srcX & 15, ty);
            if (!pix)
                continue;
            const uint32_t c = m_pens[(pix + (data >> 24) * 16) & 0xFFF];
            if (alphaMap) {
                const uint8_t a = kAlpha.level[pix];
                if (a == 0xFF)
                    row[x] = c;
                else if (a)
                    row[x] = Blend(row[x], c, a);
            } else if (alpha == 0xFF) {
                row[x] = c;
            } else if (alpha > 0) {
                row[x] = Blend(row[x], c, uint32_t(alpha));
            }
        }
    }
}

// A sprite of wide x high tiles (numbered across, then down), zoomed by the table's slopes (.10 fixed point), with
// one alpha (0xFF: none) or the pens' own (-1), drawn where z (its place in the list) is not below what is there
void Board::DrawSprite(uint32_t code, uint32_t color, bool flipX, bool flipY, int x, int y, int alpha, uint32_t zoomX,
                       uint32_t zoomY, int wide, int high, int z, bool deep) {
    if (!zoomX || !zoomY)
        return;
    const int sw = (zoomX == 0x400) ? wide * 16 : int(((uint32_t(wide) * 16 * 0x100000u) / zoomX + 0x200) >> 10);
    const int sh = (zoomY == 0x400) ? high * 16 : int(((uint32_t(high) * 16 * 0x100000u) / zoomY + 0x200) >> 10);
    if (!sw || !sh)
        return;
    int sx = x, sy = y, ex = x + sw, ey = y + sh;
    int dx = int(zoomX), dy = int(zoomY);
    int xBase = 0, yIndex = 0;
    if (flipX) {
        xBase = (sw - 1) * int(zoomX);
        dx = -dx;
    }
    if (flipY) {
        yIndex = (sh - 1) * int(zoomY);
        dy = -dy;
    }
    if (sx < 0) {
        xBase += -sx * dx;
        sx = 0;
    }
    if (sy < 0) {
        yIndex += -sy * dy;
        sy = 0;
    }
    ex = std::min(ex, kWidth);
    ey = std::min(ey, kHeight);
    if (ex <= sx)
        return;
    const uint8_t *g = m_roms.graphics.data();
    const uint32_t *pal = &m_pens[color * 16];
    for (int py = sy; py < ey; py++, yIndex += dy) {
        const uint32_t srcY = uint32_t(yIndex) >> 10;
        const uint32_t rowTile = code + (srcY >> 4) * uint32_t(wide), ty = srcY & 15;
        uint32_t *dst = &m_picture[py * kWidth];
        uint16_t *pri = &m_z[py * kWidth];
        int xIndex = xBase;
        for (int px = sx; px < ex; px++, xIndex += dx) {
            const uint32_t srcX = uint32_t(xIndex) >> 10;
            const uint32_t tile = rowTile + (srcX >> 4);
            if (z > 0 && z < pri[px])
                continue;
            const uint8_t pen = deep ? Pen8(g, tile % kTiles8, srcX & 15, ty) : Pen4(g, tile % kTiles4, srcX & 15, ty);
            if (!pen)
                continue;
            const uint32_t c = pal[pen];
            if (alpha == 0xFF)
                dst[px] = c;
            else if (alpha >= 0)
                dst[px] = Blend(dst[px], c, uint32_t(alpha));
            else
                dst[px] = kAlpha.level[pen] == 0xFF ? c : Blend(dst[px], c, kAlpha.level[pen]);
            if (z > 0)
                pri[px] = static_cast<uint16_t>(z);
        }
    }
}

// The sprites (as latched at the last vblank) whose priority (vidregs[2]'s nibble they pick) is this one
void Board::DrawSprites(int priority) {
    const State &s = *m_s;
    const uint32_t *spr = s.sprites;
    auto listWord = [&](int i) {
        const uint32_t w = spr[0x3800 / 4 + i / 2];
        return static_cast<uint16_t>((i & 1) ? w : w >> 16);
    };
    auto zoomEntry = [&](uint32_t i) {
        const uint32_t w = s.zoom[i >> 1];
        return (i & 1) ? (w & 0xFFFF) : (w >> 16);
    };
    for (int i = 0; i < 0x400; i++) {
        const uint16_t entry = listWord(i);
        const uint32_t n = uint32_t(entry & 0x3FF) << 2;
        const uint32_t w0 = spr[n], w1 = spr[n + 1], w2 = spr[n + 2];
        const uint32_t bgPri = (w1 >> 12) & 3;
        const int pri = int(((s.vidregs[2] << (4 * bgPri)) & 0xF0000000) >> 28);
        if (pri == priority) {
            const uint32_t zoomY = zoomEntry((w1 >> 16) & 0xFF), zoomX = zoomEntry(w1 & 0xFF);
            if (zoomX && zoomY) {
                const uint32_t which = (w2 >> 20) & 7;
                const uint8_t a = static_cast<uint8_t>(s.vidregs[which >> 2] >> (24 - 8 * (which & 3)));
                const int alpha = (a & 0x80) ? -1 : int(Pal6(0x3F - (a & 0x3F)));
                int x = int(w0 & 0x3FF), y = int((w0 >> 16) & 0x3FF);
                if (x & 0x200)
                    x -= 0x400;
                if (y & 0x200)
                    y -= 0x400;
                DrawSprite(w2 & 0x7FFFF, w2 >> 24, (w1 & 0x8000) != 0, (w1 & 0x80000000u) != 0, x, y, alpha, zoomX,
                           zoomY, int((w1 >> 8) & 15) + 1, int((w1 >> 24) & 15) + 1, i, (w2 & 0x800000) != 0);
            }
        }
        if (entry & 0x4000)
            break;
    }
}

void Board::Render() {
    const State &s = *m_s;
    for (int i = 0; i < 0x1400; i++) {
        const uint32_t p = s.palette[i];   // RRGGBBxx
        m_pens[i] = (p >> 24) | ((p >> 8) & 0xFF00) | ((p << 8) & 0xFF0000);
    }
    std::fill(m_z.begin(), m_z.end(), 0);
    PreLineBlend();
    for (int priority = 0; priority <= 7; priority++) {
        DrawSprites(priority);   // (backgrounds over sprites of the same priority)
        for (int layer = 0; layer < 4; layer++)
            if ((s.vidregs[7] << (4 * layer)) & 0x8000)
                DrawLayer(layer, priority);
        PostLineBlend(priority);
    }
}

// --- a frame ---

void Board::RunFrame(uint16_t p1, uint16_t p2) {
    State &s = *m_s;
    // the controls, active low: coins 0 / 1; player 2 in bits 16-23, player 1 in 24-31 (START, buttons 3 2 1,
    // LEFT RIGHT DOWN UP); 4 SERVICE, 5 TEST; bit 6 the tilt / debug switch, off
    auto player = [](uint16_t p) {
        return uint32_t(((p & ARCADE_START) ? 0x01 : 0) | ((p & (ARCADE_HP | ARCADE_HK)) ? 0x02 : 0)
                        | ((p & (ARCADE_MP | ARCADE_MK)) ? 0x04 : 0) | ((p & (ARCADE_LP | ARCADE_LK)) ? 0x08 : 0)
                        | ((p & ARCADE_LEFT) ? 0x10 : 0) | ((p & ARCADE_RIGHT) ? 0x20 : 0)
                        | ((p & ARCADE_DOWN) ? 0x40 : 0) | ((p & ARCADE_UP) ? 0x80 : 0));
    };
    const uint32_t active = ((p1 & ARCADE_COIN) ? 1u : 0) | ((p2 & ARCADE_COIN) ? 2u : 0) | m_test
                          | (player(p2) << 16) | (player(p1) << 24);
    m_inputs = ~active;
    const uint64_t frameCycles = uint64_t(kLines) * kCyclesPerLine;
    const uint64_t base = uint64_t(s.frame) * frameCycles;
    RunCpu(base + uint64_t(kVblankLine) * kCyclesPerLine);
    // vblank: the picture, then the sprite list latched for the next, then the interrupt (held until irqctrl)
    Render();
    memcpy(s.sprites, s.vram, sizeof(s.sprites));
    s.vblankIrq = 1;
    UpdateIrq();
    RunCpu(base + frameCycles);
    s.frame++;
}

// --- save states (online rollback) ---

void Board::Serialize(Archive &ar) {
    ar.Mark(0x50533556);   // 'PS5V'
    ar.Pod(*m_s);
    ar.Pod(m_cpu.s);
    if (ar.Saving()) {
        ymfm::ymfm_saved_state ys(m_ymState, true);
        m_ym.save_restore(ys);
    }
    ar.Vec(m_ymState);
    if (!ar.Saving() && ar.Ok()) {
        ymfm::ymfm_saved_state ys(m_ymState, false);
        m_ym.save_restore(ys);
    }
}

uint32_t Board::Hash() const {
    uint32_t h = 2166136261u;
    for (uint32_t w : m_s->ram)
        h = (h ^ w) * 16777619u;
    for (uint32_t w : m_s->vram)
        h = (h ^ w) * 16777619u;
    return h;
}

} // namespace psikyosh
