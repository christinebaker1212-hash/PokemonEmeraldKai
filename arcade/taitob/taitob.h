// Moemon Kai: a Taito B System board (PUZZLE BOBBLE, MAME's "pbobble"), for the GAME CENTER's Astro City cabinet.
// Its parts: cps2recomp's 68000 (12 MHz) and Z80 (4 MHz, the sound board), the TC0180VCU (two 16x16 tilemaps, an 8x8
// text layer, sprites drawn into a frame buffer at vblank), the TC0640FIO (inputs), the TC0140SYT (the main / sound
// link), a 93C46 EEPROM (cps2recomp's) and a YM2610B (ymfm, arcade/ymfm). Made from MAME's descriptions of the
// hardware (taito_b.cpp, tc0180vcu.cpp, taitoio.cpp, taitosnd.cpp: Nicola Salmoria, Jarek Burczynski and others; BSD-3
// / GPL-2.0+), written again here for this game. The ROM set is the player's own (include/arcade.h).
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "m68k.h"
#include "eeprom.h"
#include "state.h"
#include "../cps2recomp/sound/z80.h"
#include "../ymfm/ymfm_opn.h"

namespace taitob {

// The set's parts, as they load (68000 program big-endian; graphics as the board's two ROM halves)
struct Roms {
    std::vector<uint8_t> program;   // 0x80000
    std::vector<uint8_t> sound;     // 0x20000
    std::vector<uint8_t> graphics;  // 0x100000
    std::vector<uint8_t> samples;   // 0x100000 (ADPCM)
};

class Board : public cps2recomp::M68KBus, public cps2recomp::Z80Bus, public ymfm::ymfm_interface {
public:
    static constexpr int kWidth = 320, kHeight = 224;   // the picture (lines 16-239 of 256)
    static constexpr uint32_t kYmClock = 8000000;

    explicit Board(Roms roms);
    void Reset();
    // One frame (60 a second) with the players' controls (include/arcade.h ARCADE_* bits)
    void RunFrame(uint16_t p1, uint16_t p2);
    const uint32_t *Picture() const {
        return m_picture.data();
    }
    std::vector<int16_t> audio;     // stereo pairs at SampleRate()
    uint32_t SampleRate() const {
        return kYmClock / 144;
    }
    void Serialize(cps2recomp::Archive &ar);
    uint32_t Hash() const;

    // M68KBus
    uint8_t Read8(uint32_t address) override;
    uint16_t Read16(uint32_t address) override;
    void Write8(uint32_t address, uint8_t value) override;
    void Write16(uint32_t address, uint16_t value) override;
    uint16_t Fetch16(uint32_t address) override;
    void InterruptAck(unsigned level) override;
    // Z80Bus
    uint8_t Read(uint16_t address) override;
    void Write(uint16_t address, uint8_t value) override;
    // ymfm_interface
    void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) override;
    void ymfm_update_irq(bool asserted) override;
    uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override;

private:
    Roms m_roms;
    std::vector<uint8_t> m_chars, m_tiles;   // decoded 8x8 and 16x16 pictures, a pen (0-15) a byte
    uint32_t m_charCount = 0, m_tileCount = 0;

    cps2recomp::M68K m_cpu;
    cps2recomp::Z80 m_z80;
    ymfm::ym2610b m_ym;
    cps2recomp::Eeprom m_eeprom;

    // what is saved (Serialize)
    struct State {
        uint8_t ram[0x10000];       // 900000
        uint8_t palette[0x2000];    // 800000
        uint8_t vcu[0x18000];       // 400000: tile RAM, sprite RAM, scroll RAM
        uint16_t ctrl[16];          // 418000: the VCU's control words
        uint16_t fb[2][512 * 256];  // the frame buffer pages (a pixel: sprite color * 16 + pen)
        uint8_t fbPage, videoControl;
        uint8_t z80ram[0x2000], z80bank;
        uint8_t fio[8];
        uint16_t eepLatch;
        // TC0140SYT
        uint8_t mainMode, subMode, status, nmiEnabled, slaveData[4], masterData[4];
        uint8_t irqPending;         // 68000 levels waiting (bits 3 and 5)
        uint8_t ymIrq;
        int64_t ymTimer[2];         // YM2610 clocks left (-1: off)
        uint32_t frame;
        uint32_t sampleFrac;
    };
    std::unique_ptr<State> m_s;
    std::vector<uint8_t> m_ymState;
    std::vector<uint32_t> m_picture;
    uint16_t m_inputs[2] = {};

    void Decode();
    uint8_t ReadIo(uint32_t address);
    void WriteIo(uint32_t address, uint8_t value);
    uint8_t FioRead(int offset);
    void SytMasterWrite(uint8_t value);
    uint8_t SytMasterRead();
    void SytSlaveWrite(uint8_t value);
    uint8_t SytSlaveRead();
    void UpdateNmi();
    void UpdateIrq();
    void VblankUpdate();
    void DrawSprites(uint16_t *fb);
    void Render();
    void RunSound(uint64_t untilZ80);
    void Ym(uint32_t clocks);
};

} // namespace taitob
