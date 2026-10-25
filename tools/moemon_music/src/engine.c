// Moemon Music Studio: the game's music engine. Each function names the one it was ported from;
// keep them in step with the game's code (src/music_player.c, src/sound_mixer.c, src/m4a.c,
// src/platform/cgb_audio.c) so what plays here is what the game plays.
#include <string.h>
#include "engine.h"
#include "sf.h"

#define MPT_FLG_VOLSET 0x01
#define MPT_FLG_VOLCHG 0x03
#define MPT_FLG_PITSET 0x04
#define MPT_FLG_PITCHG 0x0C
#define MPT_FLG_START  0x40
#define MPT_FLG_EXIST  0x80

#define SF_START   0x80
#define SF_STOP    0x40
#define SF_LOOP    0x10
#define SF_IEC     0x04
#define SF_ENV     0x03
#define SF_ON      (SF_START | SF_STOP | SF_IEC | SF_ENV)

#define TYPE_CGB 0x07
#define TYPE_FIX 0x08
#define TYPE_SPL 0x40
#define TYPE_RHY 0x80


// GB sound registers, as offsets into e->io (GBA address - 0x04000060)
enum {
    NR10 = 0x00, NR11 = 0x02, NR12 = 0x03, NR13 = 0x04, NR14 = 0x05,
    NR21 = 0x08, NR22 = 0x09, NR23 = 0x0C, NR24 = 0x0D,
    NR30 = 0x10, NR31 = 0x12, NR32 = 0x13, NR33 = 0x14, NR34 = 0x15,
    NR41 = 0x18, NR42 = 0x19, NR43 = 0x1C, NR44 = 0x1D,
    NR50 = 0x20, NR51 = 0x21, NR52 = 0x24, BIAS_H = 0x29, WAVE_RAM = 0x30,
};

static uint32_t umul3232H32(uint32_t a, uint32_t b) { return (uint32_t)(((uint64_t)a * b) >> 32); }

// ------------------------------------------------------------------ cgb_audio.c

static void cgb_set_sweep(Engine *e, uint8_t sweep)
{
    e->gb.ch1SweepDir = (sweep & 0x08) >> 3;
    e->gb.ch1SweepCounter = e->gb.ch1SweepCounterI = (sweep & 0x70) >> 4;
    e->gb.ch1SweepShift = sweep & 0x07;
}

static void cgb_set_wavram(Engine *e)
{
    for (int i = 0; i < 0x10; i++) {
        e->gb.WAVRAM[i << 1] = ((e->io[WAVE_RAM + i] & 0xF0) >> 4) / 7.5f - 1.0f;
        e->gb.WAVRAM[(i << 1) + 1] = (e->io[WAVE_RAM + i] & 0x0F) / 7.5f - 1.0f;
    }
}

static void cgb_toggle_length(Engine *e, int ch, int state) { e->gb.LenOn[ch] = state ? 1 : 0; }
static void cgb_set_length(Engine *e, int ch, uint8_t len) { e->gb.Len[ch] = e->gb.LenI[ch] = len; }

static void cgb_set_envelope(Engine *e, int ch, uint8_t env)
{
    if (ch == 2) {
        switch (env & 0xE0) {
        case 0x00: e->gb.Vol[2] = e->gb.VolI[2] = 0; break;
        case 0x20: e->gb.Vol[2] = e->gb.VolI[2] = 4; break;
        case 0x40: e->gb.Vol[2] = e->gb.VolI[2] = 2; break;
        case 0x60: e->gb.Vol[2] = e->gb.VolI[2] = 1; break;
        case 0x80: e->gb.Vol[2] = e->gb.VolI[2] = 3; break;
        }
    } else {
        e->gb.DAC[ch] = (env & 0xF8) > 0;
        e->gb.Vol[ch] = e->gb.VolI[ch] = (env & 0xF0) >> 4;
        e->gb.EnvDir[ch] = (env & 0x08) >> 3;
        e->gb.EnvCounter[ch] = e->gb.EnvCounterI[ch] = env & 0x07;
    }
}

static void cgb_trigger_note(Engine *e, int ch)
{
    e->gb.Vol[ch] = e->gb.VolI[ch];
    e->gb.Len[ch] = e->gb.LenI[ch];
    if (ch != 2) e->gb.EnvCounter[ch] = e->gb.EnvCounterI[ch];
    if (ch == 3) { e->gb.ch4LFSR[0] = 0x8000; e->gb.ch4LFSR[1] = 0x80; }
}

static void cgb_audio_init(Engine *e)
{
    memset(&e->gb, 0, sizeof(e->gb));
    for (int i = 0; i < 4; i++) e->soundChannelPos[i] = 0;
    e->soundChannelPos[1] = 1;
    e->PU1Table = sf_PU0;
    e->PU2Table = sf_PU0;
    e->gb.ch4LFSR[0] = 0x8000;
    e->gb.ch4LFSR[1] = 0x80;
    e->ch4Samples = 0.0f;
    e->apuFrame = 0;
    e->apuCycle = 0;
}

static const short *duty_table(uint8_t reg)
{
    switch (reg & 0xC0) {
    case 0x00: return sf_PU0;
    case 0x40: return sf_PU1;
    case 0x80: return sf_PU2;
    default:   return sf_PU3;
    }
}

static void cgb_audio_generate(Engine *e, int samples)
{
    const uint32_t sampleRate = ENG_RATE;
    const uint16_t lfsrMax[2] = {0x8000, 0x80};
    uint8_t *io = e->io;
    float *out = e->cgbOut;
    e->PU1Table = duty_table(io[NR11]);
    e->PU2Table = duty_table(io[NR21]);

    for (int i = 0; i < samples; i++, out += 2) {
        e->apuFrame += 512;
        if (e->apuFrame >= sampleRate) {
            e->apuFrame -= sampleRate;
            e->apuCycle++;
            if ((e->apuCycle & 1) == 0) {   // length
                for (int ch = 0; ch < 4; ch++)
                    if (e->gb.Len[ch] && --e->gb.Len[ch] == 0 && e->gb.LenOn[ch])
                        io[NR52] &= (0xFF ^ (1 << ch));
            }
            if ((e->apuCycle & 7) == 7) {   // envelope
                for (int ch = 0; ch < 4; ch++) {
                    if (ch == 2) continue;
                    if (e->gb.EnvCounter[ch] && --e->gb.EnvCounter[ch] == 0) {
                        if (e->gb.Vol[ch] && !e->gb.EnvDir[ch]) {
                            e->gb.Vol[ch]--;
                            e->gb.EnvCounter[ch] = e->gb.EnvCounterI[ch];
                        } else if (e->gb.Vol[ch] < 0x0F && e->gb.EnvDir[ch]) {
                            e->gb.Vol[ch]++;
                            e->gb.EnvCounter[ch] = e->gb.EnvCounterI[ch];
                        }
                    }
                }
            }
            if ((e->apuCycle & 3) == 2) {   // sweep
                if (e->gb.ch1SweepCounterI && e->gb.ch1SweepShift && --e->gb.ch1SweepCounter == 0) {
                    uint16_t f = (io[NR13] | (io[NR14] << 8)) & 0x7FF;
                    if (e->gb.ch1SweepDir) {
                        f -= f >> e->gb.ch1SweepShift;
                        if (f & 0xF800) f = 0;
                    } else {
                        f += f >> e->gb.ch1SweepShift;
                        if (f & 0xF800) { f = 0; e->gb.EnvCounter[0] = 0; e->gb.Vol[0] = 0; }
                    }
                    e->gb.ch1Freq = f;
                    io[NR13] = f & 0xFF;
                    io[NR14] &= 0xF8;
                    io[NR14] += (f >> 8) & 0x07;
                    e->gb.ch1SweepCounter = e->gb.ch1SweepCounterI;
                }
            }
        }
        e->soundChannelPos[0] += sf_freqTable[(io[NR13] | (io[NR14] << 8)) & 0x7FF] / (sampleRate / 32);
        e->soundChannelPos[1] += sf_freqTable[(io[NR23] | (io[NR24] << 8)) & 0x7FF] / (sampleRate / 32);
        e->soundChannelPos[2] += sf_freqTable[(io[NR33] | (io[NR34] << 8)) & 0x7FF] / (sampleRate / 32);
        while (e->soundChannelPos[0] >= 32) e->soundChannelPos[0] -= 32;
        while (e->soundChannelPos[1] >= 32) e->soundChannelPos[1] -= 32;
        while (e->soundChannelPos[2] >= 32) e->soundChannelPos[2] -= 32;
        float L = 0, R = 0;
        if (io[NR52] & 0x80) {
            if (e->gb.DAC[0] && (io[NR52] & 0x01)) {
                float s = e->gb.Vol[0] * e->PU1Table[(int)e->soundChannelPos[0]] / 15.0f;
                if (io[NR51] & 0x10) L += s;
                if (io[NR51] & 0x01) R += s;
            }
            if (e->gb.DAC[1] && (io[NR52] & 0x02)) {
                float s = e->gb.Vol[1] * e->PU2Table[(int)e->soundChannelPos[1]] / 15.0f;
                if (io[NR51] & 0x20) L += s;
                if (io[NR51] & 0x02) R += s;
            }
            if ((io[NR30] & 0x80) && (io[NR52] & 0x04)) {
                float s = e->gb.Vol[2] * e->gb.WAVRAM[(int)e->soundChannelPos[2]] / 4.0f;
                if (io[NR51] & 0x40) L += s;
                if (io[NR51] & 0x04) R += s;
            }
            if (e->gb.DAC[3] && (io[NR52] & 0x08)) {
                int mode = (io[NR43] & 0x08) == 8;
                e->ch4Samples += sf_freqTableNSE[io[NR43]] / sampleRate;
                int o = (e->gb.ch4LFSR[mode] & 1) ? 1 : -1;
                int avgDiv = 1;
                while (e->ch4Samples >= 1) {
                    avgDiv++;
                    int carry = 0;
                    if (e->gb.ch4LFSR[mode] & 2) carry ^= 1;
                    e->gb.ch4LFSR[mode] >>= 1;
                    if (e->gb.ch4LFSR[mode] & 2) carry ^= 1;
                    if (carry) e->gb.ch4LFSR[mode] |= lfsrMax[mode];
                    o += (e->gb.ch4LFSR[mode] & 1) ? 1 : -1;
                    e->ch4Samples--;
                }
                float s = (float)o;
                if (avgDiv > 1) s /= avgDiv;
                if (io[NR51] & 0x80) L += e->gb.Vol[3] * s / 15.0f;
                if (io[NR51] & 0x08) R += e->gb.Vol[3] * s / 15.0f;
            }
        }
        out[0] = L / 4.0f;
        out[1] = R / 4.0f;
    }
}

// ------------------------------------------------------------------ m4a.c CGB side

static uint32_t MidiKeyToCgbFreq(uint8_t chanNum, uint8_t key, uint8_t fineAdjust)
{
    if (chanNum == 4) {
        if (key <= 20) key = 0;
        else { key -= 21; if (key > 59) key = 59; }
        return sf_gNoiseTable[key];
    } else {
        int32_t val1, val2;
        if (key <= 35) { fineAdjust = 0; key = 0; }
        else { key -= 36; if (key > 130) { key = 130; fineAdjust = 255; } }
        val1 = sf_gCgbScaleTable[key];
        val1 = sf_gCgbFreqTable[val1 & 0xF] >> (val1 >> 4);
        val2 = sf_gCgbScaleTable[key + 1];
        val2 = sf_gCgbFreqTable[val2 & 0xF] >> (val2 >> 4);
        return (uint32_t)(val1 + ((fineAdjust * (val2 - val1)) >> 8) + 2048);
    }
}

static void CgbOscOff(Engine *e, uint8_t chanNum)
{
    switch (chanNum) {
    case 1: e->io[NR12] = 8; e->io[NR14] = 0x80; break;
    case 2: e->io[NR22] = 8; e->io[NR24] = 0x80; break;
    case 3: e->io[NR30] = 0; break;
    default: e->io[NR42] = 8; e->io[NR44] = 0x80; break;
    }
    cgb_set_envelope(e, chanNum - 1, 8);
    cgb_trigger_note(e, chanNum - 1);
}

static int CgbPan(EChan *c)
{
    uint32_t r = c->rightVol, l = c->leftVol;
    if (r >= l) {
        if (r / 2 >= l) { c->pan = 0x0F; return 1; }
    } else {
        if (l / 2 >= r) { c->pan = 0xF0; return 1; }
    }
    return 0;
}

static void CgbModVol(EChan *c)
{
    if (!CgbPan(c)) {
        c->pan = 0xFF;
        c->envelopeGoal = (uint8_t)((uint32_t)(c->leftVol + c->rightVol) / 16);
    } else {
        uint32_t g = (uint32_t)(c->leftVol + c->rightVol) / 16;
        if (g > 15) g = 15;
        c->envelopeGoal = (uint8_t)g;
    }
    c->sustainGoal = (uint8_t)((c->envelopeGoal * c->sustain + 15) >> 4);
    c->pan &= c->panMask;
}

static void CgbSound(Engine *e)
{
    uint8_t *io = e->io;
    int mask = 0xff;
    if (e->c15) e->c15--;
    else e->c15 = 14;

    for (int ch = 1; ch <= 4; ch++) {
        EChan *c = &e->cgb[ch - 1];
        int nrx0, nrx1, nrx2, nrx3, nrx4;
        if (!(c->status & SF_ON))
            continue;
        switch (ch) {
        case 1: nrx0 = NR10; nrx1 = NR11; nrx2 = NR12; nrx3 = NR13; nrx4 = NR14; break;
        case 2: nrx0 = NR10 + 1; nrx1 = NR21; nrx2 = NR22; nrx3 = NR23; nrx4 = NR24; break;
        case 3: nrx0 = NR30; nrx1 = NR31; nrx2 = NR32; nrx3 = NR33; nrx4 = NR34; break;
        default: nrx0 = NR30 + 1; nrx1 = NR41; nrx2 = NR42; nrx3 = NR43; nrx4 = NR44; break;
        }
        int prevC15 = e->c15;
        int envStep = io[nrx2];

        if (c->status & SF_START) {
            if (!(c->status & SF_STOP)) {
                c->status = 3;   // SOUND_CHANNEL_SF_ENV_ATTACK
                c->modify = 3;
                CgbModVol(c);
                switch (ch) {
                case 1:
                    io[nrx0] = c->sweep;
                    cgb_set_sweep(e, c->sweep);
                    // fallthrough
                case 2:
                    io[nrx1] = (uint8_t)((c->wavePointer << 6) + c->length);
                    goto init_env_step_time_dir;
                case 3:
                    if (c->wavePointer != c->currentPointer) {
                        io[nrx0] = 0x40;
                        memcpy(&io[WAVE_RAM], (const void *)c->wavePointer, 16);
                        c->currentPointer = c->wavePointer;
                        cgb_set_wavram(e);
                    }
                    io[nrx0] = 0;
                    io[nrx1] = c->length;
                    c->n4 = c->length ? 0xC0 : 0x80;
                    break;
                default:
                    io[nrx1] = c->length;
                    io[nrx3] = (uint8_t)(c->wavePointer << 3);
                init_env_step_time_dir:
                    envStep = c->attack + 0x08;
                    c->n4 = c->length ? 0x40 : 0x00;
                    break;
                }
                cgb_set_length(e, ch - 1, c->length);
                c->envelopeCtr = c->attack;
                if ((int8_t)(c->attack & mask)) {
                    c->envelopeVol = 0;
                    goto envelope_step_complete;
                } else {
                    goto envelope_decay_start;
                }
            } else {
                goto oscillator_off;
            }
        } else if (c->status & SF_IEC) {
            c->echoLen--;
            if ((int8_t)(c->echoLen & mask) <= 0) {
            oscillator_off:
                CgbOscOff(e, ch);
                c->status = 0;
                goto channel_complete;
            }
            goto envelope_complete;
        } else if ((c->status & SF_STOP) && (c->status & SF_ENV)) {
            c->status &= ~SF_ENV;
            c->envelopeCtr = c->release;
            if ((int8_t)(c->release & mask)) {
                c->modify |= 1;
                if (ch != 3) envStep = c->release | 0x00;
                goto envelope_step_complete;
            } else {
                goto envelope_pseudoecho_start;
            }
        } else {
        envelope_step_repeat:
            if (c->envelopeCtr == 0) {
                if (ch == 3) c->modify |= 1;
                CgbModVol(c);
                if ((c->status & SF_ENV) == 0) {           // release
                    c->envelopeVol--;
                    if ((int8_t)(c->envelopeVol & mask) <= 0) {
                    envelope_pseudoecho_start:
                        c->envelopeVol = (uint8_t)(((c->envelopeGoal * c->echoVol) + 0xFF) >> 8);
                        if (c->envelopeVol) {
                            c->status |= SF_IEC;
                            c->modify |= 1;
                            if (ch != 3) envStep = 0 | 0x08;
                            goto envelope_complete;
                        } else {
                            goto oscillator_off;
                        }
                    } else {
                        c->envelopeCtr = c->release;
                    }
                } else if ((c->status & SF_ENV) == 1) {    // sustain
                envelope_sustain:
                    c->envelopeVol = c->sustainGoal;
                    c->envelopeCtr = 7;
                } else if ((c->status & SF_ENV) == 2) {    // decay
                    c->envelopeVol--;
                    int ev = (int8_t)(c->envelopeVol & mask);
                    int sg = (int8_t)c->sustainGoal;
                    if (ev <= sg) {
                    envelope_sustain_start:
                        if (c->sustain == 0) {
                            c->status &= ~SF_ENV;
                            goto envelope_pseudoecho_start;
                        } else {
                            c->status--;
                            c->modify |= 1;
                            if (ch != 3) envStep = 0 | 0x08;
                            goto envelope_sustain;
                        }
                    } else {
                        c->envelopeCtr = c->decay;
                    }
                } else {                                    // attack
                    c->envelopeVol++;
                    if ((uint8_t)(c->envelopeVol & mask) >= c->envelopeGoal) {
                    envelope_decay_start:
                        c->status--;
                        c->envelopeCtr = c->decay;
                        if ((uint8_t)(c->envelopeCtr & mask)) {
                            c->modify |= 1;
                            c->envelopeVol = c->envelopeGoal;
                            if (ch != 3) envStep = c->decay | 0x00;
                        } else {
                            goto envelope_sustain_start;
                        }
                    } else {
                        c->envelopeCtr = c->attack;
                    }
                }
            }
        }

    envelope_step_complete:
        c->envelopeCtr--;
        if (prevC15 == 0) {
            prevC15--;
            goto envelope_step_repeat;
        }

    envelope_complete:
        if (c->modify & 2) {
            if (ch < 4 && (c->type & TYPE_FIX)) {
                int dac = io[BIAS_H];
                if (dac < 0x40) c->freq = (c->freq + 2) & 0x7fc;
                else if (dac < 0x80) c->freq = (c->freq + 1) & 0x7fe;
            }
            if (ch != 4) io[nrx3] = (uint8_t)c->freq;
            else io[nrx3] = (uint8_t)((io[nrx3] & 0x08) | c->freq);
            c->n4 = (uint8_t)((c->n4 & 0xC0) + ((c->freq >> 8) & 0xFF));
            io[nrx4] = (uint8_t)(c->n4 & mask);
        }
        if (c->modify & 1) {
            io[NR51] = (uint8_t)((io[NR51] & ~c->panMask) | c->pan);
            if (ch == 3) {
                // gCgb3Vol has 16 entries; the game reads on into gClockTable past them
                io[nrx2] = c->envelopeVol < 16 ? sf_gCgb3Vol[c->envelopeVol] : sf_gClockTable[(c->envelopeVol - 16) % 49];
                if (c->n4 & 0x80) {
                    io[nrx0] = 0x80;
                    io[nrx4] = c->n4;
                    c->n4 &= 0x7f;
                }
            } else {
                io[nrx2] = (uint8_t)((envStep & 0xF) + (c->envelopeVol << 4));
                io[nrx4] = c->n4 | 0x80;
                if (ch == 1 && !(io[nrx0] & 0x08))
                    io[nrx4] = c->n4 | 0x80;
            }
            cgb_set_envelope(e, ch - 1, io[nrx2]);
            cgb_toggle_length(e, ch - 1, io[nrx4] & 0x40);
            cgb_trigger_note(e, ch - 1);
        }
    channel_complete:
        c->modify = 0;
    }
}

// ------------------------------------------------------------------ m4a.c / music_player.c player

static uint32_t MidiKeyToFreq(const EWave *wav, uint8_t key, uint8_t fineAdjust)
{
    uint32_t fine = (uint32_t)fineAdjust << 24;
    if (key > 178) { key = 178; fine = 255u << 24; }
    uint32_t v1 = sf_gScaleTable[key];
    v1 = sf_gFreqTable[v1 & 0xF] >> (v1 >> 4);
    uint32_t v2 = sf_gScaleTable[key + 1];
    v2 = sf_gFreqTable[v2 & 0xF] >> (v2 >> 4);
    return umul3232H32(wav ? wav->freq : 0, v1 + umul3232H32(v2 - v1, fine));
}

static void ClearChain(EChan *c)   // MP2KClearChain
{
    ETrack *t = c->track;
    if (!t) return;
    EChan *next = c->next, *prev = c->prev;
    if (prev) prev->next = next;
    else t->chan = next;
    if (next) next->prev = prev;
    c->track = NULL;
}

static void ClearModM(ETrack *t)
{
    t->lfoSpeedCounter = 0;
    t->modCalculated = 0;
    if (t->modType == 0) t->status |= MPT_FLG_PITCHG;
    else t->status |= MPT_FLG_VOLCHG;
}

static void TrkVolPitSet(ETrack *t)
{
    if (t->status & MPT_FLG_VOLSET) {
        int32_t x = (int32_t)((uint32_t)(t->vol * t->volPublic) >> 5);
        if (t->modType == 1)
            x = (int32_t)((uint32_t)(x * (t->modCalculated + 128)) >> 7);
        int32_t y = 2 * t->pan + t->panPublic;
        if (t->modType == 2)
            y += t->modCalculated;
        if (y < -128) y = -128;
        else if (y > 127) y = 127;
        t->volRightCalculated = (uint8_t)((uint32_t)((y + 128) * x) >> 8);
        t->volLeftCalculated = (uint8_t)((uint32_t)((127 - y) * x) >> 8);
    }
    if (t->status & MPT_FLG_PITSET) {
        int32_t bend = t->bend * t->bendRange;
        int32_t x = (t->tune + bend) * 4 + (t->keyShift << 8) + (t->keyShiftPublic << 8) + t->pitchPublic;
        if (t->modType == 0)
            x += 16 * t->modCalculated;
        t->keyShiftCalculated = (int8_t)(x >> 8);
        t->pitchCalculated = (uint8_t)x;
    }
    t->status &= ~(MPT_FLG_PITSET | MPT_FLG_VOLSET);
}

static void ChnVolSet(EChan *c, ETrack *t)   // ChnVolSetAsm
{
    int8_t fp = c->rhythmPan;
    uint32_t r = (uint8_t)(fp + 128) * c->velocity * t->volRightCalculated / 128 / 128;
    if (r > 0xFF) r = 0xFF;
    c->rightVol = (uint8_t)r;
    uint32_t l = (uint8_t)(127 - fp) * c->velocity * t->volLeftCalculated / 128 / 128;
    if (l > 0xFF) l = 0xFF;
    c->leftVol = (uint8_t)l;
}

static uint8_t rd(ETrack *t) { return t->cmd < t->len ? t->base[t->cmd++] : 0xB1; }
static uint8_t peek(ETrack *t) { return t->cmd < t->len ? t->base[t->cmd] : 0xB1; }
static int rd32(ETrack *t)
{
    int v = 0;
    for (int i = 0; i < 4; i++) v |= (int)rd(t) << (8 * i);
    return v;
}

static void TrackStop(Engine *e, ETrack *t)
{
    if (t->status & 0x80) {
        for (EChan *c = t->chan; c; c = c->next) {
            if (c->status) {
                int cgbType = c->type & 7;
                if (cgbType) CgbOscOff(e, cgbType);
                c->status = 0;
            }
            c->track = NULL;
        }
        t->chan = NULL;
    }
}

static void ev_fine(Engine *e, ETrack *t)
{
    (void)e;
    for (EChan *c = t->chan; c; c = c->next) {
        if (c->status & 0xC7) c->status |= 0x40;
        ClearChain(c);
    }
    t->status = 0;
}

static void play_note(Engine *e, ETrack *t, int ti)   // the second half of MP2K_event_nxx
{
    int8_t forcedPan = 0;
    const EInst *inst = &t->instrument;
    uint8_t key = t->key;
    uint8_t type = inst->type;

    if (type & (TYPE_RHY | TYPE_SPL)) {
        uint8_t idx;
        if (inst->type & TYPE_SPL) idx = inst->split ? inst->split[t->key & 0x7F] : 0;
        else idx = t->key;
        if (!inst->group) return;
        inst = inst->group + (idx & 0x7F);
        if (inst->type & (TYPE_RHY | TYPE_SPL)) return;
        if (type & TYPE_RHY) {
            if (inst->panSweep & 0x80) forcedPan = (int8_t)(((int8_t)(inst->panSweep & 0x7F) - 0x40) * 2);
            key = inst->key;
        }
    }

    unsigned priority = e->priority + t->priority;
    if (priority > 0xFF) priority = 0xFF;

    uint8_t cgbType = inst->type & TYPE_CGB;
    EChan *c;
    if (cgbType) {
        c = &e->cgb[cgbType - 1];
        if ((c->status & SF_ON) && !(c->status & SF_STOP)) {
            if (c->priority > priority || (c->priority == priority && c->track < t))
                return;
        }
    } else {
        unsigned p = priority;
        ETrack *tt = t;
        int foundStopping = 0;
        c = NULL;
        for (int i = 0; i < e->numChans; i++) {
            EChan *cc = &e->chans[i];
            if (!(cc->status & SF_ON)) { c = cc; break; }
            if ((cc->status & SF_STOP) && !foundStopping) {
                foundStopping = 1;
                p = cc->priority; tt = cc->track; c = cc;
            } else if (((cc->status & SF_STOP) && foundStopping) || (!(cc->status & SF_STOP) && !foundStopping)) {
                if (cc->priority < p) { p = cc->priority; tt = cc->track; c = cc; }
                else if (cc->priority == p && cc->track > tt) { tt = cc->track; c = cc; }
                else if (cc->priority == p && cc->track == tt) { c = cc; }
            }
        }
    }
    if (!c) return;
    ClearChain(c);
    c->prev = NULL;
    c->next = t->chan;
    if (t->chan) t->chan->prev = c;
    t->chan = c;
    c->track = t;

    t->lfoDelayCounter = t->lfoDelay;
    if (t->lfoDelay) ClearModM(t);
    TrkVolPitSet(t);

    c->gateTime = t->gateTime;
    c->untransposedKey = t->key;
    c->velocity = t->velocity;
    c->priority = (uint8_t)priority;
    c->key = key;
    c->rhythmPan = forcedPan;
    c->type = inst->type;
    if (cgbType == 3) c->wavePointer = (uintptr_t)inst->wave16;
    else if (cgbType) c->wavePointer = inst->cgbParam;
    c->wav = inst->wav;
    c->attack = inst->attack;
    c->decay = inst->decay;
    c->sustain = inst->sustain;
    c->release = inst->release;
    c->echoVol = t->echoVolume;
    c->echoLen = t->echoLength;
    ChnVolSet(c, t);

    int tk = c->key + t->keyShiftCalculated;
    if (tk < 0) tk = 0;
    if (cgbType) {
        c->length = inst->length;
        if ((inst->panSweep & 0x80) || (inst->panSweep & 0x70) == 0) c->sweep = 8;
        else c->sweep = inst->panSweep;
        c->freq = MidiKeyToCgbFreq(cgbType, (uint8_t)tk, t->pitchCalculated);
    } else {
        c->freq = MidiKeyToFreq(c->wav, (uint8_t)tk, t->pitchCalculated);
    }
    c->status = SF_START;
    t->status &= ~0xF;
    if (ti <= ENG_MAX_TRACKS) e->trackLevel[ti] = 255;
}

static void ev_nxx(Engine *e, ETrack *t, int ti, uint8_t clock)   // MP2K_event_nxx
{
    t->gateTime = sf_gClockTable[clock];
    if (peek(t) < 0x80) {
        t->key = rd(t);
        if (peek(t) < 0x80) {
            t->velocity = rd(t);
            if (peek(t) < 0x80)
                t->gateTime += rd(t);
        }
    }
    if (e->muteMask & (1u << ti))
        return;
    play_note(e, t, ti);
}

static void ev_endtie(ETrack *t)
{
    uint8_t key = peek(t);
    if (key < 0x80) { t->key = key; t->cmd++; }
    else key = t->key;
    for (EChan *c = t->chan; c; c = c->next) {
        if ((c->status & 0x83) && !(c->status & 0x40) && c->untransposedKey == key) {
            c->status |= 0x40;
            return;
        }
    }
}

static void ev_goto(ETrack *t)
{
    int target = rd32(t);
    t->cmd = (target >= 0 && target < t->len) ? target : t->len;
}

static void run_event(Engine *e, ETrack *t, uint8_t ev)
{
    switch (ev) {
    case 0xB2: ev_goto(t); break;                                   // GOTO
    case 0xB3:                                                      // PATT
        if (t->patternLevel < 3) {
            t->patternStack[t->patternLevel++] = t->cmd + 4;
            ev_goto(t);
        } else {
            ev_fine(e, t);
        }
        break;
    case 0xB4:                                                      // PEND
        if (t->patternLevel) t->cmd = t->patternStack[--t->patternLevel];
        break;
    case 0xB5:                                                      // REPT
        if (peek(t) == 0) { t->cmd++; ev_goto(t); }
        else {
            uint8_t n = ++t->repeatCount;
            if (n < rd(t)) ev_goto(t);
            else { t->repeatCount = 0; t->cmd += 4; }
        }
        break;
    case 0xB9: {                                                    // MEMACC (ply_memacc)
        uint8_t op = rd(t);
        uint8_t *addr = &e->memacc[rd(t) & 15];
        uint8_t data = rd(t);
        int cond = -1;
        switch (op) {
        case 0: *addr = data; break;
        case 1: *addr += data; break;
        case 2: *addr -= data; break;
        case 3: *addr = e->memacc[data & 15]; break;
        case 4: *addr += e->memacc[data & 15]; break;
        case 5: *addr -= e->memacc[data & 15]; break;
        case 6: cond = *addr == data; break;
        case 7: cond = *addr != data; break;
        case 8: cond = *addr > data; break;
        case 9: cond = *addr >= data; break;
        case 10: cond = *addr <= data; break;
        case 11: cond = *addr < data; break;
        case 12: cond = *addr == e->memacc[data & 15]; break;
        case 13: cond = *addr != e->memacc[data & 15]; break;
        case 14: cond = *addr > e->memacc[data & 15]; break;
        case 15: cond = *addr >= e->memacc[data & 15]; break;
        case 16: cond = *addr <= e->memacc[data & 15]; break;
        case 17: cond = *addr < e->memacc[data & 15]; break;
        }
        if (cond == 1) ev_goto(t);
        else if (cond == 0) t->cmd += 4;
        break;
    }
    case 0xBA: t->priority = rd(t); break;                          // PRIO
    case 0xBB: {                                                    // TEMPO
        uint16_t bpm = rd(t);
        bpm *= 2;
        e->tempoRawBPM = bpm;
        e->tempoInterval = (uint16_t)((bpm * e->tempoScale) / 256);
        break;
    }
    case 0xBC: t->keyShift = (int8_t)rd(t); t->status |= 0xC; break; // KEYSH
    case 0xBD: {                                                    // VOICE
        uint8_t v = rd(t);
        if (e->voicegroup) t->instrument = e->voicegroup[v & 0x7F];
        t->voiceSlot = v & 0x7F;
        t->voiceSet = 1;
        break;
    }
    case 0xBE: t->vol = rd(t); t->status |= 0x3; break;             // VOL
    case 0xBF: t->pan = (int8_t)(rd(t) - 0x40); t->status |= 0x3; break;
    case 0xC0: t->bend = (int8_t)(rd(t) - 0x40); t->status |= 0xC; break;
    case 0xC1: t->bendRange = rd(t); t->status |= 0xC; break;
    case 0xC2: t->lfoSpeed = rd(t); if (!t->lfoSpeed) ClearModM(t); break;
    case 0xC3: t->lfoDelay = rd(t); break;
    case 0xC4: t->modDepth = rd(t); if (!t->modDepth) ClearModM(t); break;
    case 0xC5: {
        uint8_t ty = rd(t);
        if (ty != t->modType) { t->modType = ty; t->status |= 0xF; }
        break;
    }
    case 0xC8: t->tune = (int8_t)(rd(t) - 0x40); t->status |= 0xC; break;
    case 0xCC: t->cmd += 2; break;                                  // PORT
    case 0xCD: {                                                    // XCMD
        uint8_t x = rd(t);
        if (x == 0x08) t->echoVolume = rd(t);
        else if (x == 0x09) t->echoLength = rd(t);
        break;
    }
    case 0xCE: ev_endtie(t); break;                                 // EOT
    default: ev_fine(e, t); break;                                  // FINE and unused commands
    }
}

static void player_main(Engine *e)   // MP2KPlayerMain
{
    if (e->paused && !(e->tracks[ENG_PREVIEW_TRACK].status & MPT_FLG_EXIST))
        return;
    e->tempoCounter += e->tempoInterval;
    while (e->tempoCounter >= 150) {
        uint32_t trackBits = 0;
        for (int i = 0; i <= ENG_MAX_TRACKS; i++) {
            ETrack *t = &e->tracks[i];
            if (!(t->status & MPT_FLG_EXIST))
                continue;
            if (i < ENG_MAX_TRACKS) trackBits |= 1u << i;
            for (EChan *c = t->chan; c; c = c->next) {
                if (!(c->status & SF_ON)) ClearChain(c);
                else if (c->gateTime && --c->gateTime == 0) c->status |= SF_STOP;
            }
            if (i == ENG_PREVIEW_TRACK)
                continue;
            if (t->status & MPT_FLG_START) {
                const uint8_t *base = t->base; int len = t->len, cmd = t->cmd;
                memset(t, 0, sizeof(*t));
                t->base = base; t->len = len; t->cmd = cmd;
                t->status = MPT_FLG_EXIST;
                t->bendRange = 2;
                t->volPublic = 64;
                t->lfoSpeed = 22;
                t->instrument.type = 1;
            }
            while (t->wait == 0) {
                uint8_t ev = peek(t);
                if (ev < 0x80) {
                    ev = t->runningStatus;
                } else {
                    t->cmd++;
                    if (ev >= 0xBD) t->runningStatus = ev;
                }
                if (ev >= 0xCF) {
                    ev_nxx(e, t, i, ev - 0xCF);
                } else if (ev >= 0xB1) {
                    run_event(e, t, ev);
                    if (t->status == 0) goto nextTrack;
                } else {
                    t->wait = sf_gClockTable[ev - 0x80];
                }
            }
            t->wait--;
            if (t->lfoSpeed && t->modDepth) {
                if (t->lfoDelayCounter) {
                    t->lfoDelayCounter--;
                    goto nextTrack;
                }
                t->lfoSpeedCounter += t->lfoSpeed;
                int8_t r;
                if (t->lfoSpeedCounter >= 0x40 && t->lfoSpeedCounter < 0xC0) r = (int8_t)(128 - t->lfoSpeedCounter);
                else if (t->lfoSpeedCounter >= 0xC0) r = (int8_t)(t->lfoSpeedCounter - 256);
                else r = (int8_t)t->lfoSpeedCounter;
                r = (int8_t)((t->modDepth * r) >> 6);
                if (r != t->modCalculated) {
                    t->modCalculated = r;
                    if (t->modType == 0) t->status |= MPT_FLG_PITCHG;
                    else t->status |= MPT_FLG_VOLCHG;
                }
            }
        nextTrack:;
        }
        e->clock++;
        if (trackBits == 0) {
            e->status = 0;
            e->paused = 1;
            if (!(e->tracks[ENG_PREVIEW_TRACK].status & MPT_FLG_EXIST))
                return;
        } else {
            e->status = trackBits;
            e->paused = 0;
        }
        e->tempoCounter -= 150;
    }

    for (int i = 0; i <= ENG_MAX_TRACKS; i++) {
        ETrack *t = &e->tracks[i];
        if (!(t->status & MPT_FLG_EXIST) || !(t->status & 0xF))
            continue;
        TrkVolPitSet(t);
        for (EChan *c = t->chan; c; c = c->next) {
            if (!(c->status & 0xC7)) { ClearChain(c); continue; }
            uint8_t cgbType = c->type & 7;
            if (t->status & MPT_FLG_VOLCHG) {
                ChnVolSet(c, t);
                if (cgbType) c->modify |= 1;
            }
            if (t->status & MPT_FLG_PITCHG) {
                int key = c->key + t->keyShiftCalculated;
                if (key < 0) key = 0;
                if (cgbType) {
                    c->freq = MidiKeyToCgbFreq(cgbType, (uint8_t)key, t->pitchCalculated);
                    c->modify |= 2;
                } else {
                    c->freq = MidiKeyToFreq(c->wav, (uint8_t)key, t->pitchCalculated);
                }
            }
        }
        t->status &= ~0xF;
    }
}

// ------------------------------------------------------------------ sound_mixer.c

static int TickEnvelope(EChan *c)
{
    uint8_t status = c->status;
    if ((status & 0xC7) == 0)
        return 0;
    uint8_t env = 0;
    if ((status & 0x80) == 0) {
        env = c->envelopeVol;
        if (status & 4) {
            --c->echoVol;
            if (c->echoVol == 0) { c->status = 0; return 0; }
            return 1;
        } else if (status & 0x40) {
            c->envelopeVol = (uint8_t)(env * c->release / 256u);
            uint8_t echoVol = c->echoVol;
            if (c->envelopeVol > echoVol) return 1;
            else if (echoVol == 0) { c->status = 0; return 0; }
            else { c->status |= 4; return 1; }
        }
        switch (status & 3) {
        case 2: {
            c->envelopeVol = (uint8_t)(env * c->decay / 256u);
            uint8_t sustain = c->sustain;
            if (c->envelopeVol <= sustain && sustain == 0) {
                if (c->echoVol == 0) { c->status = 0; return 0; }
                c->status |= 4;
                return 1;
            } else if (c->envelopeVol <= sustain) {
                c->envelopeVol = sustain;
                --c->status;
            }
            break;
        }
        case 3:
        attack: {
            unsigned ne = env + c->attack;
            if (ne > 0xFF) { c->envelopeVol = 0xFF; --c->status; }
            else c->envelopeVol = (uint8_t)ne;
            break;
        }
        default:
            break;
        }
        return 1;
    } else if (status & 0x40) {
        c->status = 0;
        return 0;
    } else {
        if (!c->wav) { c->status = 0; return 0; }
        c->status = 3;
        c->cur = 0;
        c->ct = (int32_t)c->wav->size;
        c->fw = 0;
        c->envelopeVol = 0;
        if (c->wav->loops) c->status |= 0x10;
        goto attack;
    }
}

static void GenerateAudio(EChan *c, float *out, int n)
{
    const EWave *wav = c->wav;
    uint8_t v = (uint8_t)(c->envelopeVol * (ENG_MASTER_VOL + 1) / 16u);
    c->envelopeVolR = (uint8_t)(c->rightVol * v / 256u);
    c->envelopeVolL = (uint8_t)(c->leftVol * v / 256u);
    int32_t loopLen = 0, loopStart = 0;
    if (c->status & 0x10) {
        loopStart = (int32_t)wav->loopStart;
        loopLen = (int32_t)(wav->size - wav->loopStart);
    }
    int32_t left = c->ct;
    const int8_t *d = wav->data;
    int32_t cur = c->cur;
    int envR = c->envelopeVolR, envL = c->envelopeVolL;

    if (c->type & 8) {
        for (int i = 0; i < n; i++, out += 2) {
            int8_t s = d[cur++];
            out[1] += (s * envR) / 32768.0f;
            out[0] += (s * envL) / 32768.0f;
            if (--left == 0) {
                left = loopLen;
                if (loopLen) cur = loopStart;
                else { c->status = 0; return; }
            }
        }
        c->ct = left;
        c->cur = cur;
    } else {
        float fine = c->fw;
        float step = c->freq * (1.0f / (60.0f * ENG_FRAME_SAMPLES));
        int b = d[cur];
        int m = d[cur + 1] - b;
        cur += 1;
        for (int i = 0; i < n; i++, out += 2) {
            float s = fine * m + b;
            out[1] += (s * envR) / 32768.0f;
            out[0] += (s * envL) / 32768.0f;
            fine += step;
            uint32_t coarse = (uint32_t)fine;
            if (coarse) {
                fine -= (int)fine;
                left -= (int32_t)coarse;
                if (left <= 0) {
                    if (loopLen) {
                        cur = loopStart;
                        int32_t nc = -left;
                        left += loopLen;
                        while (left <= 0) { nc -= loopLen; left += loopLen; }
                        b = d[cur + nc];
                        m = d[cur + nc + 1] - b;
                        cur += nc + 1;
                    } else {
                        c->status = 0;
                        return;
                    }
                } else {
                    b = d[cur + coarse - 1];
                    m = d[cur + coarse] - b;
                    cur += (int32_t)coarse;
                }
            }
        }
        c->fw = fine;
        c->ct = left;
        c->cur = cur - 1;
    }
}

static void SampleMixer(Engine *e, float *out)
{
    int n = ENG_FRAME_SAMPLES;
    float *buf = e->ring[e->ringPos];
    float *next = e->ring[(e->ringPos + 1) % 7];
    if (e->reverb) {
        for (int i = 0; i < n; i++) {
            float s = buf[i * 2] + buf[i * 2 + 1] + next[i * 2] + next[i * 2 + 1];
            s *= (float)e->reverb / 512.0f;
            buf[i * 2] = buf[i * 2 + 1] = s;
        }
    } else {
        memset(buf, 0, sizeof(float) * n * 2);
    }
    for (int i = 0; i < e->numChans; i++) {
        EChan *c = &e->chans[i];
        if (TickEnvelope(c))
            GenerateAudio(c, buf, n);
    }
    memcpy(out, buf, sizeof(float) * n * 2);
    e->ringPos = (e->ringPos + 1) % 7;
}

// ------------------------------------------------------------------ API

void eng_init(Engine *e)
{
    memset(e, 0, sizeof(*e));
    e->status = 0;
    e->paused = 1;
    e->numChans = ENG_DS_CHANNELS;
    e->tempoScale = 0x100;
    cgb_audio_init(e);
    // SoundInit / m4aSoundMode(DA_BIT_8) / MPlayExtender
    e->io[BIAS_H] = 0x40;
    e->io[NR52] = 0x8F;
    e->io[NR50] = 0x77;
    e->io[NR51] = 0;
    e->io[NR12] = 8; e->io[NR22] = 8; e->io[NR42] = 8;
    e->io[NR14] = 0x80; e->io[NR24] = 0x80; e->io[NR44] = 0x80;
    e->io[NR30] = 0;
    for (int i = 0; i < 4; i++) { cgb_set_envelope(e, i, 8); cgb_trigger_note(e, i); }
    for (int i = 0; i < 4; i++) e->cgb[i].type = (uint8_t)(i + 1);
    e->cgb[0].panMask = 0x11;
    e->cgb[1].panMask = 0x22;
    e->cgb[2].panMask = 0x44;
    e->cgb[3].panMask = 0x88;
}

void eng_stop(Engine *e)
{
    for (int i = 0; i < ENG_MAX_TRACKS; i++) {
        TrackStop(e, &e->tracks[i]);
        e->tracks[i].status = 0;
    }
    e->status = 0;
    e->paused = 1;
    e->playing = 0;
}

void eng_start(Engine *e, const ESong *song)   // MPlayStart
{
    eng_stop(e);
    e->song = song;
    e->voicegroup = song->voices;
    e->priority = song->priority;
    e->clock = 0;
    e->tempoRawBPM = 150;
    e->tempoInterval = 150;
    e->tempoScale = 0x100;
    e->tempoCounter = 0;
    e->status = 0;
    e->paused = 0;
    // m4aSoundMode with a channel count: every sample channel stops, then that many mix
    int voices = song->dsVoices > 0 ? song->dsVoices : ENG_DS_CHANNELS;
    if (voices > ENG_DS_MAX) voices = ENG_DS_MAX;
    if (voices != e->numChans) {
        for (int i = 0; i < ENG_DS_MAX; i++) { ClearChain(&e->chans[i]); e->chans[i].status = 0; }
        e->numChans = voices;
    }
    int limit = song->trackLimit > 0 ? song->trackLimit : ENG_GAME_TRACKS;
    if (limit > ENG_MAX_TRACKS) limit = ENG_MAX_TRACKS;
    for (int i = 0; i < song->ntracks && i < limit; i++) {
        ETrack *t = &e->tracks[i];
        t->status = MPT_FLG_EXIST | MPT_FLG_START;
        t->chan = NULL;
        t->base = song->data[i];
        t->len = song->len[i];
        t->cmd = 0;
    }
    if (song->reverb & 0x80)
        e->reverb = song->reverb & 0x7F;
    e->playing = 1;
}

void eng_frame(Engine *e, float *out)   // RunMixerFrame + m4aSoundVSync
{
    player_main(e);
    CgbSound(e);
    SampleMixer(e, out);
    cgb_audio_generate(e, ENG_FRAME_SAMPLES);
    for (int i = 0; i < ENG_FRAME_SAMPLES * 2; i++)
        out[i] += e->cgbOut[i];
    for (int i = 0; i <= ENG_MAX_TRACKS; i++)
        e->trackLevel[i] = e->trackLevel[i] > 24 ? e->trackLevel[i] - 24 : 0;
    if (e->playing && eng_finished(e))
        e->playing = 0;
}

int eng_skip(Engine *e, int frames)
{
    static float scratch[ENG_FRAME_SAMPLES * 2];
    int n = 0;
    for (; n < frames && e->playing; n++)
        eng_frame(e, scratch);
    return n;
}

int eng_finished(const Engine *e)
{
    for (int i = 0; i < ENG_MAX_TRACKS; i++)
        if (e->tracks[i].status & MPT_FLG_EXIST)
            return 0;
    return 1;
}

int eng_ds_busy(const Engine *e)
{
    int n = 0;
    for (int i = 0; i < e->numChans; i++)
        if (e->chans[i].status & SF_ON) n++;
    return n;
}

void eng_preview_on(Engine *e, const EInst *inst, int key, int velocity)
{
    ETrack *t = &e->tracks[ENG_PREVIEW_TRACK];
    eng_preview_off(e);
    for (EChan *c = t->chan; c; c = c->next) c->status |= SF_STOP;
    if (!(t->status & MPT_FLG_EXIST)) {
        memset(t, 0, sizeof(*t));
        t->status = MPT_FLG_EXIST;
        t->bendRange = 2;
        t->volPublic = 64;
        t->vol = 100;
        t->priority = 0xFF;
    }
    t->instrument = *inst;
    t->key = (uint8_t)key;
    t->velocity = (uint8_t)velocity;
    t->gateTime = 0;
    t->status |= MPT_FLG_VOLCHG | MPT_FLG_PITCHG;
    TrkVolPitSet(t);
    play_note(e, t, ENG_PREVIEW_TRACK);
}

void eng_preview_off(Engine *e)
{
    ETrack *t = &e->tracks[ENG_PREVIEW_TRACK];
    for (EChan *c = t->chan; c; c = c->next)
        if ((c->status & 0x83) && !(c->status & 0x40))
            c->status |= 0x40;
}

void eng_copy(Engine *dst, const Engine *src)
{
    memcpy(dst, src, sizeof(*dst));
#define FIXC(p) do { if (p) (p) = (EChan *)((const char *)(p) - (const char *)src + (char *)dst); } while (0)
#define FIXT(p) do { if (p) (p) = (ETrack *)((const char *)(p) - (const char *)src + (char *)dst); } while (0)
    for (int i = 0; i <= ENG_MAX_TRACKS; i++) FIXC(dst->tracks[i].chan);
    for (int i = 0; i < ENG_DS_MAX; i++) { FIXT(dst->chans[i].track); FIXC(dst->chans[i].prev); FIXC(dst->chans[i].next); }
    for (int i = 0; i < 4; i++) { FIXT(dst->cgb[i].track); FIXC(dst->cgb[i].prev); FIXC(dst->cgb[i].next); }
#undef FIXC
#undef FIXT
}

void eng_refresh_instruments(Engine *e)
{
    if (!e->voicegroup) return;
    for (int i = 0; i < ENG_MAX_TRACKS; i++)
        if ((e->tracks[i].status & MPT_FLG_EXIST) && e->tracks[i].voiceSet)
            e->tracks[i].instrument = e->voicegroup[e->tracks[i].voiceSlot];
}
