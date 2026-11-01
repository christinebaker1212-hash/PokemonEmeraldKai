// Moemon Kai: the Taito B System board for PUZZLE BOBBLE (taitob.h). Hardware as MAME describes it (taito_b.cpp's
// pbobble machine, tc0180vcu.cpp, taitoio.cpp TC0640FIO, taitosnd.cpp TC0140SYT); the code is this game's own.
#include "taitob.h"

#include <algorithm>
#include <cstring>

#include "arcade.h"

using namespace cps2recomp;

namespace taitob {

namespace {

constexpr int kLines = 256;              // a frame's lines (the picture: 16-239)
constexpr int kFirstLine = 16;
constexpr uint32_t kLinesPerSecond = 60 * kLines;
constexpr uint32_t kMainClock = 12000000, kSoundClock = 4000000;
constexpr uint8_t kSytPort01Full = 0x01, kSytPort23Full = 0x02, kSytPort01FullMaster = 0x04,
                  kSytPort23FullMaster = 0x08;
// The VCU's color bases (MAME's pbobble: fb 0x80 (x16), bg 0x00, fg 0x40, tx 0xc0)
constexpr int kFbBase = 0x80 * 16, kBgBase = 0x00, kFgBase = 0x40, kTxBase = 0xc0;

uint16_t Word(const uint8_t *p) {
    return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

} // namespace

Board::Board(Roms roms)
    : m_roms(std::move(roms))
    , m_cpu(*this)
    , m_z80(*this)
    , m_ym(*this)
    , m_s(std::make_unique<State>()) {
    m_roms.program.resize(0x80000, 0xFF);
    m_roms.sound.resize(0x20000, 0xFF);
    m_roms.graphics.resize(0x100000, 0);
    m_roms.samples.resize(0x100000, 0);
    m_ym.set_fidelity(ymfm::OPN_FIDELITY_MIN);   // (one sample every 144 clocks: 55.5 kHz)
    m_z80.hasNmi = true;
    m_picture.assign(kWidth * kHeight, 0);
    Decode();
    Reset();
}

// The VCU's pictures, a pen a byte: 4 planes from the two ROM halves (MAME's charlayout / tilelayout; the first plane
// is the pen's top bit)
void Board::Decode() {
    const std::vector<uint8_t> &g = m_roms.graphics;
    const size_t half = g.size() / 2;
    m_charCount = static_cast<uint32_t>(half / 16);
    m_tileCount = static_cast<uint32_t>(half / 64);
    m_chars.assign(static_cast<size_t>(m_charCount) * 64, 0);
    m_tiles.assign(static_cast<size_t>(m_tileCount) * 256, 0);
    auto pen = [&](size_t base, int bit) {
        return static_cast<uint8_t>((((g[base] >> bit) & 1) << 3) | (((g[base + 1] >> bit) & 1) << 2)
                                    | (((g[half + base] >> bit) & 1) << 1) | ((g[half + base + 1] >> bit) & 1));
    };
    for (uint32_t c = 0; c < m_charCount; c++)
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                m_chars[c * 64 + y * 8 + x] = pen(c * 16 + y * 2, 7 - x);
    for (uint32_t t = 0; t < m_tileCount; t++)
        for (int y = 0; y < 16; y++)
            for (int x = 0; x < 16; x++)
                m_tiles[t * 256 + y * 16 + x] = pen(t * 64 + (x >= 8 ? 16 : 0) + (y >= 8 ? 32 : 0) + (y & 7) * 2, 7 - (x & 7));
}

void Board::Reset() {
    State &s = *m_s;
    memset(&s, 0, sizeof(s));
    s.ymTimer[0] = s.ymTimer[1] = -1;
    s.z80bank = 0;
    m_cpu.Reset(true);
    m_z80.Reset();
    m_ym.reset();
    audio.clear();
}

// --- the 68000's side ---

uint16_t Board::Fetch16(uint32_t address) {
    return Read16(address);
}

uint8_t Board::FioRead(int offset) {
    // TC0640FIO: 0 SERVICE, 1 COIN (bit 0: the EEPROM's data out), 2 START, 3 buttons, 7 sticks; all active low
    const uint16_t p1 = m_inputs[0], p2 = m_inputs[1];
    switch (offset) {
    case 0:
        return 0xFF;
    case 1:
        return static_cast<uint8_t>((m_eeprom.DataOut() ? 1 : 0) | 0x0E | ((p1 & ARCADE_COIN) ? 0 : 0x10)
                                    | ((p2 & ARCADE_COIN) ? 0 : 0x20) | 0xC0);
    case 2:
        return static_cast<uint8_t>(0x0F | ((p1 & ARCADE_START) ? 0 : 0x10) | ((p2 & ARCADE_START) ? 0 : 0x20) | 0xC0);
    case 3: {
        auto buttons = [](uint16_t p) {
            return ((p & (ARCADE_LP | ARCADE_LK)) ? 1 : 0) | ((p & (ARCADE_MP | ARCADE_MK)) ? 2 : 0)
                 | ((p & (ARCADE_HP | ARCADE_HK)) ? 4 : 0);
        };
        return static_cast<uint8_t>(~(buttons(p1) | (buttons(p2) << 4)));
    }
    case 4:
        return m_s->fio[4];
    case 7: {
        auto stick = [](uint16_t p) {
            return ((p & ARCADE_UP) ? 1 : 0) | ((p & ARCADE_DOWN) ? 2 : 0) | ((p & ARCADE_LEFT) ? 4 : 0)
                 | ((p & ARCADE_RIGHT) ? 8 : 0);
        };
        return static_cast<uint8_t>(~(stick(p1) | (stick(p2) << 4)));
    }
    default:
        return 0xFF;
    }
}

// The I/O at 5xxxxx and 6xxxxx / 7xxxxx, a byte at a time (even addresses are a word's top byte)
uint8_t Board::ReadIo(uint32_t address) {
    if (address >= 0x500000 && address <= 0x50000F)
        return (address & 1) ? 0 : FioRead((address - 0x500000) >> 1);   // (halfword_byteswap: the top byte)
    switch (address) {
    case 0x500024: case 0x500025: case 0x50002E: case 0x50002F:
        return 0xFF;   // (players 3 and 4)
    case 0x500026:
        return static_cast<uint8_t>(m_s->eepLatch >> 8);
    case 0x500027:
        return static_cast<uint8_t>(m_s->eepLatch);
    case 0x700002:
        return SytMasterRead();
    default:
        return 0;
    }
}

void Board::WriteIo(uint32_t address, uint8_t value) {
    State &s = *m_s;
    if (address >= 0x500000 && address <= 0x50000F) {
        if (!(address & 1)) {
            const int offset = (address - 0x500000) >> 1;
            s.fio[offset] = value;   // (0: the watchdog; 4: coin counters and lockout)
        }
        return;
    }
    switch (address) {
    case 0x500026:
        s.eepLatch = static_cast<uint16_t>((s.eepLatch & 0x00FF) | (value << 8));
        // bit 2 data, bit 3 clock, bit 4 select (MAME's EEPROMOUT)
        m_eeprom.SetLines((value & 0x10) != 0, (value & 0x08) != 0, (value & 0x04) != 0);
        break;
    case 0x500027:
        s.eepLatch = static_cast<uint16_t>((s.eepLatch & 0xFF00) | value);
        break;
    case 0x700000:
        s.mainMode = value & 0x0F;   // TC0140SYT: the master's port
        break;
    case 0x700002:
        SytMasterWrite(value);
        break;
    default:
        break;   // (5x0028 coins 3 / 4, 600000 the MB87078's volume)
    }
}

uint8_t Board::Read8(uint32_t address) {
    State &s = *m_s;
    address &= 0xFFFFFF;
    if (address < 0x80000)
        return m_roms.program[address];
    if (address >= 0x900000 && address < 0x910000)
        return s.ram[address - 0x900000];
    if (address >= 0x800000 && address < 0x802000)
        return s.palette[address - 0x800000];
    if (address >= 0x400000 && address < 0x480000) {
        const uint32_t o = address - 0x400000;
        if (o < 0x14000)
            return s.vcu[o];
        if (o >= 0x18000 && o < 0x18020)
            return static_cast<uint8_t>((o & 1) ? s.ctrl[(o - 0x18000) >> 1] : s.ctrl[(o - 0x18000) >> 1] >> 8);
        if (o >= 0x40000) {
            const uint32_t w = (o - 0x40000) >> 1, sy = w >> 8, sx = 2 * (w & 0xFF) + (o & 1);
            return static_cast<uint8_t>(s.fb[(sy >> 8) & 1][(sy & 0xFF) * 512 + sx]);
        }
        return 0;
    }
    return ReadIo(address);
}

uint16_t Board::Read16(uint32_t address) {
    address &= 0xFFFFFE;
    if (address < 0x80000)
        return Word(&m_roms.program[address]);
    if (address >= 0x900000 && address < 0x910000)
        return Word(&m_s->ram[address - 0x900000]);
    return static_cast<uint16_t>((Read8(address) << 8) | Read8(address + 1));
}

void Board::Write8(uint32_t address, uint8_t value) {
    State &s = *m_s;
    address &= 0xFFFFFF;
    if (address >= 0x900000 && address < 0x910000) {
        s.ram[address - 0x900000] = value;
        return;
    }
    if (address >= 0x800000 && address < 0x802000) {
        s.palette[address - 0x800000] = value;
        return;
    }
    if (address >= 0x400000 && address < 0x480000) {
        const uint32_t o = address - 0x400000;
        if (o < 0x14000) {
            s.vcu[o] = value;
        } else if (o >= 0x18000 && o < 0x18020) {
            const int n = (o - 0x18000) >> 1;
            const uint16_t old = s.ctrl[n];
            s.ctrl[n] = (o & 1) ? static_cast<uint16_t>((old & 0xFF00) | value) : static_cast<uint16_t>((old & 0xFF) | (value << 8));
            if (n == 7 && !(o & 1)) {
                // video control: bit 7 holds the frame buffer page bit 6 picks
                s.videoControl = value;
                if (value & 0x80)
                    s.fbPage = (value & 0x40) ? 0 : 1;
            }
        } else if (o >= 0x40000) {
            const uint32_t w = (o - 0x40000) >> 1, sy = w >> 8, sx = 2 * (w & 0xFF) + (o & 1);
            s.fb[(sy >> 8) & 1][(sy & 0xFF) * 512 + sx] = value;
        }
        return;
    }
    if (address >= 0x500000)
        WriteIo(address, value);
}

void Board::Write16(uint32_t address, uint16_t value) {
    address &= 0xFFFFFE;
    if (address >= 0x500000 && address <= 0x50000F) {
        WriteIo(address, static_cast<uint8_t>(value >> 8));   // (halfword_byteswap: the top byte)
        return;
    }
    Write8(address, static_cast<uint8_t>(value >> 8));
    if (!(address >= 0x700000 && address < 0x700004))   // (the TC0140SYT is on the top byte only)
        Write8(address + 1, static_cast<uint8_t>(value));
}

void Board::InterruptAck(unsigned level) {
    m_s->irqPending &= static_cast<uint8_t>(~(1u << level));   // (HOLD_LINE: until taken)
    UpdateIrq();
}

void Board::UpdateIrq() {
    const uint8_t p = m_s->irqPending;
    m_cpu.SetInterruptLevel(static_cast<uint8_t>((p & (1 << 5)) ? 5 : (p & (1 << 3)) ? 3 : 0));
}

// --- TC0140SYT: the main CPU and the sound CPU pass 4-bit values ---

void Board::UpdateNmi() {
    const State &s = *m_s;
    m_z80.SetNmiLine((s.status & (kSytPort01Full | kSytPort23Full)) && s.nmiEnabled);
}

void Board::SytMasterWrite(uint8_t value) {
    State &s = *m_s;
    value &= 0x0F;
    switch (s.mainMode) {
    case 0: case 2:
        s.slaveData[s.mainMode++] = value;
        break;
    case 1:
        s.slaveData[s.mainMode++] = value;
        s.status |= kSytPort01Full;
        UpdateNmi();
        break;
    case 3:
        s.slaveData[s.mainMode++] = value;
        s.status |= kSytPort23Full;
        UpdateNmi();
        break;
    case 4:
        if (value)   // (held in reset while set)
            m_z80.Reset();
        break;
    default:
        break;
    }
}

uint8_t Board::SytMasterRead() {
    State &s = *m_s;
    uint8_t r = 0;
    switch (s.mainMode) {
    case 0: case 2:
        r = s.masterData[s.mainMode++];
        break;
    case 1:
        r = s.masterData[s.mainMode++];
        s.status &= ~kSytPort01FullMaster;
        break;
    case 3:
        r = s.masterData[s.mainMode++];
        s.status &= ~kSytPort23FullMaster;
        break;
    case 4:
        r = s.status;
        break;
    default:
        break;
    }
    return r;
}

void Board::SytSlaveWrite(uint8_t value) {
    State &s = *m_s;
    value &= 0x0F;
    switch (s.subMode) {
    case 0: case 2:
        s.masterData[s.subMode++] = value;
        break;
    case 1:
        s.masterData[s.subMode++] = value;
        s.status |= kSytPort01FullMaster;
        break;
    case 3:
        s.masterData[s.subMode++] = value;
        s.status |= kSytPort23FullMaster;
        break;
    case 5:
        s.nmiEnabled = 0;
        UpdateNmi();
        break;
    case 6:
        s.nmiEnabled = 1;
        UpdateNmi();
        break;
    default:
        break;
    }
}

uint8_t Board::SytSlaveRead() {
    State &s = *m_s;
    uint8_t r = 0;
    switch (s.subMode) {
    case 0: case 2:
        r = s.slaveData[s.subMode++];
        break;
    case 1:
        r = s.slaveData[s.subMode++];
        s.status &= ~kSytPort01Full;
        UpdateNmi();
        break;
    case 3:
        r = s.slaveData[s.subMode++];
        s.status &= ~kSytPort23Full;
        UpdateNmi();
        break;
    case 4:
        r = s.status;
        break;
    default:
        break;
    }
    return r;
}

// --- the Z80's side ---

uint8_t Board::Read(uint16_t address) {
    State &s = *m_s;
    if (address < 0x4000)
        return m_roms.sound[address];
    if (address < 0x8000)
        return m_roms.sound[(s.z80bank & 3) * 0x4000 + (address - 0x4000)];
    if (address >= 0xC000 && address < 0xE000)
        return s.z80ram[address - 0xC000];
    if (address >= 0xE000 && address <= 0xE003)
        return m_ym.read(address & 3);
    if (address == 0xE201)
        return SytSlaveRead();
    return 0;
}

void Board::Write(uint16_t address, uint8_t value) {
    State &s = *m_s;
    if (address >= 0xC000 && address < 0xE000)
        s.z80ram[address - 0xC000] = value;
    else if (address >= 0xE000 && address <= 0xE003)
        m_ym.write(address & 3, value);
    else if (address == 0xE200)
        s.subMode = value & 0x0F;
    else if (address == 0xE201)
        SytSlaveWrite(value);
    else if (address == 0xF200)
        s.z80bank = value & 3;
}

// --- the YM2610B ---

void Board::ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) {
    if (tnum < 2)
        m_s->ymTimer[tnum] = duration_in_clocks < 0 ? -1 : duration_in_clocks;
}

void Board::ymfm_update_irq(bool asserted) {
    m_s->ymIrq = asserted;
}

uint8_t Board::ymfm_external_read(ymfm::access_class type, uint32_t address) {
    (void)type;   // (ADPCM-A and -B share the one sample ROM)
    return address < m_roms.samples.size() ? m_roms.samples[address] : 0;
}

// The chip's clock moves on: its timers, and a sample every 144 clocks
void Board::Ym(uint32_t clocks) {
    State &s = *m_s;
    for (int t = 0; t < 2; t++) {
        if (s.ymTimer[t] < 0)
            continue;
        s.ymTimer[t] -= clocks;
        if (s.ymTimer[t] <= 0) {
            const int64_t over = -s.ymTimer[t];
            s.ymTimer[t] = -1;
            m_engine->engine_timer_expired(t);
            if (s.ymTimer[t] > 0)
                s.ymTimer[t] = std::max<int64_t>(1, s.ymTimer[t] - over);
        }
    }
    s.sampleFrac += clocks;
    while (s.sampleFrac >= 144) {
        s.sampleFrac -= 144;
        ymfm::ym2610b::output_data out;
        m_ym.generate(&out);
        // MAME's pbobble: the FM left and right and the SSG (x0.75) into one speaker
        int32_t v = out.data[0] + out.data[1] + out.data[2] * 3 / 4;
        v = std::clamp(v, -32768, 32767);
        audio.push_back(static_cast<int16_t>(v));
        audio.push_back(static_cast<int16_t>(v));
    }
}

void Board::RunSound(uint64_t untilZ80) {
    while (m_z80.cycles < untilZ80) {
        if (m_s->ymIrq)
            m_z80.RaiseInterrupt();
        else
            m_z80.LowerInterrupt();
        const uint64_t before = m_z80.cycles, step = std::min<uint64_t>(untilZ80, before + 64);
        m_z80.Run(step, Z80::Mode::Interpreter);
        Ym(static_cast<uint32_t>((m_z80.cycles - before) * (kYmClock / kSoundClock)));
    }
}

// --- the TC0180VCU's pictures ---

// At vblank: the shown page cleared (unless held), the pages swapped (unless held), the sprites drawn into it
void Board::VblankUpdate() {
    State &s = *m_s;
    if (!(s.videoControl & 0x01))
        for (int y = kFirstLine; y < kFirstLine + kHeight; y++)
            memset(&s.fb[s.fbPage][y * 512], 0, kWidth * sizeof(uint16_t));
    if (!(s.videoControl & 0x80))
        s.fbPage ^= 1;
    DrawSprites(s.fb[s.fbPage]);
}

void Board::DrawSprites(uint16_t *fb) {
    const State &s = *m_s;
    auto spr = [&](int word) {
        return Word(&s.vcu[0x10000 + word * 2]);
    };
    int xlatch = 0, ylatch = 0, xNo = 0, yNo = 0, xNum = 0, yNum = 0;
    uint32_t zoomXLatch = 0, zoomYLatch = 0;
    bool big = false;
    for (int offs = (0x1980 - 16) / 2; offs >= 0; offs -= 8) {
        const uint32_t code = spr(offs) % std::max<uint32_t>(1, m_tileCount);
        const uint16_t attr = spr(offs + 1);
        const bool flipX = attr & 0x4000, flipY = attr & 0x8000;
        const int color = (attr & 0x3F) * 16;
        int x = spr(offs + 2) & 0x3FF, y = spr(offs + 3) & 0x3FF;
        if (x >= 0x200)
            x -= 0x400;
        if (y >= 0x200)
            y -= 0x400;
        uint32_t data = spr(offs + 5);
        if (data && !big) {
            xNum = (data >> 8) & 0xFF;
            yNum = data & 0xFF;
            xNo = yNo = 0;
            xlatch = x;
            ylatch = y;
            data = spr(offs + 4);
            zoomXLatch = (data >> 8) & 0xFF;
            zoomYLatch = data & 0xFF;
            big = true;
        }
        data = spr(offs + 4);
        uint32_t zoomX = (data >> 8) & 0xFF, zoomY = data & 0xFF;
        int zx = static_cast<int>((0x100 - zoomX) / 16), zy = static_cast<int>((0x100 - zoomY) / 16);
        if (big) {
            zoomX = zoomXLatch;
            zoomY = zoomYLatch;
            x = xlatch + static_cast<int>((xNo * (0xFF - zoomX) + 15) / 16);
            y = ylatch + static_cast<int>((yNo * (0xFF - zoomY) + 15) / 16);
            zx = xlatch + static_cast<int>(((xNo + 1) * (0xFF - zoomX) + 15) / 16) - x;
            zy = ylatch + static_cast<int>(((yNo + 1) * (0xFF - zoomY) + 15) / 16) - y;
            if (++yNo > yNum) {
                yNo = 0;
                if (++xNo > xNum)
                    big = false;
            }
        }
        if (!(zoomX || zoomY))
            zx = zy = 16;
        if (zx <= 0 || zy <= 0)
            continue;
        const uint8_t *pic = &m_tiles[code * 256];
        for (int dy = 0; dy < zy; dy++) {
            const int py = y + dy;
            if (py < kFirstLine || py >= kFirstLine + kHeight)
                continue;
            int sy = dy * 16 / zy;
            if (flipY)
                sy = 15 - sy;
            for (int dx = 0; dx < zx; dx++) {
                const int px = x + dx;
                if (px < 0 || px >= kWidth)
                    continue;
                int sx = dx * 16 / zx;
                if (flipX)
                    sx = 15 - sx;
                const uint8_t pen = pic[sy * 16 + sx];
                if (pen)
                    fb[py * 512 + px] = static_cast<uint16_t>(color + pen);
            }
        }
    }
}

// The picture as the screen showed it (MAME's taitob screen_update: bg, sprites behind, fg, sprites, text)
void Board::Render() {
    const State &s = *m_s;
    static uint16_t index[kWidth * kHeight];
    auto vram = [&](uint32_t word) {
        return Word(&s.vcu[(word & 0x7FFF) * 2]);
    };
    auto scroll = [&](uint32_t word) {
        return Word(&s.vcu[0x13800 + (word & 0x3FF) * 2]);
    };
    if (!(s.videoControl & 0x20)) {
        std::fill(std::begin(index), std::end(index), 0);
    } else {
        // a 16x16 tilemap (map 0 the background, opaque; 1 the foreground) with its scroll blocks
        auto tilemap = [&](int map) {
            const int plane = map == 0 ? 1 : 0;
            const uint16_t ctl = s.ctrl[map == 0 ? 1 : 0];
            const uint32_t codes = ((ctl >> 8) & 0x0F) << 12, attrs = ((ctl >> 12) & 0x0F) << 12;
            const int linesPerBlock = std::max(1, 256 - (s.ctrl[2 + plane] >> 8));
            const int base = map == 0 ? kBgBase : kFgBase;
            for (int y = kFirstLine; y < kFirstLine + kHeight; y++) {
                const int block = y / linesPerBlock;
                const int sx = static_cast<int16_t>(scroll(plane * 0x200 + block * 2 * linesPerBlock));
                const int sy = static_cast<int16_t>(scroll(plane * 0x200 + block * 2 * linesPerBlock + 1));
                const int ty = (y - sy) & 1023;
                uint16_t *row = &index[(y - kFirstLine) * kWidth];
                for (int x = 0; x < kWidth; x++) {
                    const int tx = (x - sx) & 1023;
                    const uint32_t cell = (ty >> 4) * 64 + (tx >> 4);
                    const uint16_t attr = vram(cell + attrs);
                    int px = tx & 15, py = ty & 15;
                    if (attr & 0x40)
                        px = 15 - px;
                    if (attr & 0x80)
                        py = 15 - py;
                    const uint8_t pen = m_tiles[(vram(cell + codes) % std::max<uint32_t>(1, m_tileCount)) * 256 + py * 16 + px];
                    if (pen || map == 0)
                        row[x] = static_cast<uint16_t>((base + (attr & 0x3F)) * 16 + pen);
                }
            }
        };
        // the sprites' frame buffer: with bit 3 of video control all over the foreground, else by each one's bit 4
        auto framebuffer = [&](int priority) {
            if ((s.videoControl & 0x08) && priority)
                return;
            const uint16_t *fb = s.fb[s.fbPage];
            for (int y = kFirstLine; y < kFirstLine + kHeight; y++)
                for (int x = 0; x < kWidth; x++) {
                    const uint16_t c = fb[y * 512 + x];
                    if (c && ((s.videoControl & 0x08) || (c & 0x10) == (priority << 4)))
                        index[(y - kFirstLine) * kWidth + x] = static_cast<uint16_t>(kFbBase + c);
                }
        };
        tilemap(0);
        framebuffer(1);
        tilemap(1);
        framebuffer(0);
        // the 8x8 text layer
        const uint32_t txBank = ((s.ctrl[6] >> 8) & 0x0F) << 11;
        for (int y = kFirstLine; y < kFirstLine + kHeight; y++)
            for (int x = 0; x < kWidth; x++) {
                const uint16_t tile = vram((y >> 3) * 64 + (x >> 3) + txBank);
                const uint32_t code = ((tile & 0x7FF) | ((s.ctrl[4 + ((tile >> 11) & 1)] >> 8) << 11)) % std::max<uint32_t>(1, m_charCount);
                const uint8_t pen = m_chars[code * 64 + (y & 7) * 8 + (x & 7)];
                if (pen)
                    index[(y - kFirstLine) * kWidth + x] = static_cast<uint16_t>((kTxBase + ((tile >> 12) & 0x0F)) * 16 + pen);
            }
    }
    // RRRRGGGGBBBBRGBx, each 5 bits (MAME's format), to 0x00BBGGRR
    for (int i = 0; i < kWidth * kHeight; i++) {
        const uint16_t w = Word(&s.palette[(index[i] & 0xFFF) * 2]);
        const uint32_t r = ((w >> 11) & 0x1E) | ((w >> 3) & 1), g = ((w >> 7) & 0x1E) | ((w >> 2) & 1),
                       b = ((w >> 3) & 0x1E) | ((w >> 1) & 1);
        m_picture[i] = ((r << 3) | (r >> 2)) | (((g << 3) | (g >> 2)) << 8) | (((b << 3) | (b >> 2)) << 16);
    }
}

// --- a frame ---

void Board::RunFrame(uint16_t p1, uint16_t p2) {
    State &s = *m_s;
    m_inputs[0] = p1;
    m_inputs[1] = p2;
    // vblank: the picture of the frame that ended, then the sprites for the next, and INTH (level 3)
    Render();
    VblankUpdate();
    s.irqPending |= 1 << 3;
    UpdateIrq();
    const uint64_t mainStart = m_cpu.cycles, soundStart = m_z80.cycles;
    for (int line = 0; line < kLines; line++) {
        if (line == 8) {
            // INTL (level 5) eight lines on; INTH let go if it wasn't taken
            s.irqPending = static_cast<uint8_t>((s.irqPending & ~(1 << 3)) | (1 << 5));
            UpdateIrq();
        }
        const uint64_t mainUntil = mainStart + static_cast<uint64_t>(kMainClock) * (line + 1) / kLinesPerSecond;
        const uint64_t soundUntil = soundStart + static_cast<uint64_t>(kSoundClock) * (line + 1) / kLinesPerSecond;
        m_cpu.Run(mainUntil, M68K::Mode::Interpreter);
        RunSound(soundUntil);
    }
    s.frame++;
}

// --- save states (online rollback) ---

void Board::Serialize(Archive &ar) {
    ar.Mark(0x54414942);   // 'TAIB'
    ar.Pod(*m_s);
    m_cpu.Serialize(ar);
    m_z80.Serialize(ar);
    m_eeprom.Serialize(ar);
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
    for (uint8_t b : m_s->ram)
        h = (h ^ b) * 16777619u;
    return h;
}

} // namespace taitob
