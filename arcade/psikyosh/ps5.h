// Moemon Kai: a Psikyo PS5V2 board (TETRIS THE GRAND MASTER 2 and its PLUS, MAME's "tgm2" / "tgm2p"), for the GAME
// CENTER's cabinets. Its parts: an SH-2 (SH7604, 28.6 MHz: sh2.h), the PS6406B video chip (four background layers of
// 16x16 tiles, 4 or 8 bits a pixel, with per-line scroll / zoom / alpha / priority / bank; sprites zoomed through a
// table, blended by sprite or by pen; line fill and line blend layers), a YMF278B (OPL4: ymfm, arcade/ymfm) and a
// 93C56 EEPROM (256 bytes, 8-bit mode). The hardware is as MAME describes it (psikyosh.cpp, psikyosh_v.cpp: David
// Haywood, Paul Priest; BSD-3-Clause; eepromser.cpp for the EEPROM's protocol); the code is this game's own. The ROM
// set is the player's own (include/arcade.h).
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "sh2.h"
#include "state.h"
#include "../ymfm/ymfm_opl.h"

namespace psikyosh {

// The set's parts, as they load
struct Roms {
    std::vector<uint8_t> program;   // 0x100000, as the SH-2 sees it (big-endian; the u21 / u22 pair interleaved)
    std::vector<uint8_t> graphics;  // 0x3000000, MAME's "gfx" region (the l / h pairs a word each)
    std::vector<uint8_t> samples;   // 0x400000, the YMF278B's ROM
    std::vector<uint8_t> eeprom;    // 0x100, the EEPROM's first contents (the set's .nv)
};

class Board : public Sh2Bus, public ymfm::ymfm_interface {
public:
    static constexpr int kWidth = 320, kHeight = 240;
    static constexpr uint32_t kCpuClock = 57272727 / 2;   // (the YMF278B's too)
    static constexpr int kLines = 262, kCyclesPerLine = 443 * 4;   // 61.68 frames a second

    explicit Board(Roms roms);
    void Reset();
    // One frame with the players' controls (include/arcade.h ARCADE_* bits)
    void RunFrame(uint16_t p1, uint16_t p2);
    const uint32_t *Picture() const {   // kWidth * kHeight pixels, 0x00BBGGRR
        return m_picture.data();
    }
    std::vector<int16_t> audio;     // stereo pairs at SampleRate()
    uint32_t SampleRate() const {
        return kCpuClock / 768;
    }
    double FrameRate() const {
        return double(kCpuClock) / (kLines * kCyclesPerLine);
    }
    void Serialize(cps2recomp::Archive &ar);
    uint32_t Hash() const;
    // The board's TEST switch (held: the operator's test menu) and SERVICE button, as read from the next frame on
    void SetTestSwitch(bool test, bool service = false) {
        m_test = (test ? 0x20u : 0) | (service ? 0x10u : 0);
    }

    // Sh2Bus
    uint8_t Read8(uint32_t address) override;
    uint16_t Read16(uint32_t address) override;
    uint32_t Read32(uint32_t address) override;
    void Write8(uint32_t address, uint8_t value) override;
    void Write16(uint32_t address, uint16_t value) override;
    void Write32(uint32_t address, uint32_t value) override;
    // ymfm_interface
    void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) override;
    void ymfm_update_irq(bool asserted) override;
    uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override;

private:
    Roms m_roms;
    std::vector<uint32_t> m_program;   // the program as 32-bit words

    Sh2 m_cpu;
    ymfm::ymf278b m_ym;

    // what is saved (Serialize)
    struct State {
        uint32_t ram[0x40000];      // 06000000 (1 MB)
        uint32_t vram[0x4000];      // 04000000: 32 banks of 0x800 bytes (sprites, the sprite list, tiles, line tables)
        uint32_t palette[0x1400];   // 04040000: RRGGBBxx
        uint32_t zoom[0x80];        // 04050000: the sprites' zoom table (16-bit entries)
        uint32_t vidregs[8];        // 0405FFE0
        uint32_t sprites[0x1000];   // vram's 0000-3FFF as it was at the last vblank (the sprites drawn)
        // the 93C56
        uint8_t eep[0x100];
        uint8_t eepState, eepCs, eepClk, eepDi, eepLocked, eepCommand, eepCsRose, pad0;
        uint32_t eepAccum, eepShift, eepBits, eepAddress;
        uint64_t eepReadyAt;        // (CPU cycles: a write / erase keeps DO low until then)
        uint8_t vblankIrq, ymIrq, pad1[2];
        int64_t ymTimerAt[2];       // the YMF278B's timers: the CPU cycle they run out (-1: off)
        uint64_t nextSampleAt;      // the CPU cycle of the next YMF278B sample (one every 768)
        uint32_t frame;
    };
    std::unique_ptr<State> m_s;
    std::vector<uint8_t> m_ymState;
    std::vector<uint32_t> m_picture;
    uint32_t m_inputs = 0xFFFFFFFF, m_test = 0;

    // the renderer's own (rebuilt every frame)
    uint32_t m_pens[0x1400] = {};   // the palette as 0x00BBGGRR
    std::vector<uint16_t> m_z;      // the sprites' z buffer

    uint32_t IoRead(uint32_t address);
    void WriteMasked(uint32_t address, uint32_t value, uint32_t mask);
    void EepromWrite(uint8_t data);
    bool EepromDataOut() const;
    void EepromClock();
    void UpdateIrq();
    void RunCpu(uint64_t until);
    void Sample();

    void Render();
    void DrawSprites(int priority);
    void DrawSprite(uint32_t code, uint32_t color, bool flipX, bool flipY, int x, int y, int alpha, uint32_t zoomX,
                    uint32_t zoomY, int wide, int high, int z, bool deep);
    void DrawLayer(int layer, int priority);
    void PreLineBlend();
    void PostLineBlend(int priority);
};

} // namespace psikyosh
