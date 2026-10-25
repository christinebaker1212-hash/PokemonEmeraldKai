// Moemon Music Studio: the game's music engine (MP2K / m4a as the PC port runs it),
// ported from src/music_player.c, src/sound_mixer.c, src/m4a.c and src/platform/cgb_audio.c
// so a song sounds here the way it will in the game.
#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

#define ENG_RATE            42048    // what the game opens SDL audio at
#define ENG_FRAME_SAMPLES   701      // samples the game mixes per frame
#define ENG_GAME_TRACKS     10       // the game's BGM player has 10 tracks
#define ENG_MAX_TRACKS      32       // what the studio can play when a song goes over that (ESong.trackLimit)
#define ENG_DS_CHANNELS     5        // m4aSoundInit: 5 DirectSound channels
#define ENG_DS_MAX          12       // MAX_DIRECTSOUND_CHANNELS: what m4aSoundMode allows (ESong.dsVoices)
#define ENG_MASTER_VOL      12       // m4aSoundInit: master volume 12
#define ENG_PREVIEW_TRACK   ENG_MAX_TRACKS   // one extra track for auditioning instruments

typedef struct {
    uint32_t freq, loopStart, size;
    int loops;
    const int8_t *data;
} EWave;

// A voicegroup entry as the engine reads it (struct MP2KInstrument).
typedef struct EInst {
    uint8_t type, key, length, panSweep;
    const EWave *wav;                // DirectSound
    const uint8_t *wave16;           // programmable wave (16 bytes)
    uint32_t cgbParam;               // square duty / noise period
    const struct EInst *group;       // key-split / drum kit: 128 entries
    const uint8_t *split;            // key-split table: 128 entries
    uint8_t attack, decay, sustain, release;
} EInst;

typedef struct {
    int ntracks;
    const uint8_t *data[ENG_MAX_TRACKS];   // track bytecode; GOTO / PATT / REPT targets are 4-byte LE offsets into the track
    int len[ENG_MAX_TRACKS];
    int trackLimit;                  // tracks that play: ENG_GAME_TRACKS like the game (0 = that), up to ENG_MAX_TRACKS
    int dsVoices;                    // sample channels: ENG_DS_CHANNELS like the game (0 = that), up to ENG_DS_MAX
    uint8_t priority, reverb;        // reverb: 0x80 | level, or 0 (keep)
    const EInst *voices;             // 128
} ESong;

struct ETrack;

typedef struct EChan {
    uint8_t status, type, rightVol, leftVol, attack, decay, sustain, release, key, envelopeVol;
    uint8_t envelopeVolR, envelopeVolL;      // DirectSound
    uint8_t envelopeGoal, envelopeCtr;       // CGB
    uint8_t echoVol, echoLen, gateTime, untransposedKey, velocity, priority;
    int8_t rhythmPan;
    // DirectSound
    int32_t ct;
    float fw;
    uint32_t freq;
    const EWave *wav;
    int32_t cur;
    // CGB
    uint8_t sustainGoal, n4, pan, panMask, modify, length, sweep;
    uintptr_t wavePointer, currentPointer;
    struct ETrack *track;
    struct EChan *prev, *next;
} EChan;

typedef struct ETrack {
    uint8_t status, wait, patternLevel, repeatCount, gateTime, key, velocity, runningStatus;
    int8_t keyShiftCalculated;
    uint8_t pitchCalculated;
    int8_t keyShift, keyShiftPublic, tune;
    uint8_t pitchPublic;
    int8_t bend;
    uint8_t bendRange, volRightCalculated, volLeftCalculated, vol, volPublic;
    int8_t pan, panPublic, modCalculated;
    uint8_t modDepth, modType, lfoSpeed, lfoSpeedCounter, lfoDelay, lfoDelayCounter, priority, echoVolume, echoLength;
    EChan *chan;
    EInst instrument;
    uint8_t voiceSlot, voiceSet;      // last VOICE (so instrument edits reach a playing track)
    const uint8_t *base;
    int len;
    int cmd;                          // offset into base
    int patternStack[3];
} ETrack;

typedef struct {
    // player
    const ESong *song;
    uint32_t status;                  // a bit per playing track
    int paused;                       // MUSICPLAYER_STATUS_PAUSE, kept apart so 32 tracks fit in status
    uint8_t trackCount, priority;
    uint32_t clock;
    uint16_t tempoRawBPM, tempoScale, tempoInterval, tempoCounter;
    ETrack tracks[ENG_MAX_TRACKS + 1];
    const EInst *voicegroup;
    int playing;
    uint32_t muteMask;                // tracks whose notes are skipped (listening only)
    uint8_t memacc[16];               // gMPlayMemAccArea
    // mixer
    EChan chans[ENG_DS_MAX];
    int numChans;                     // sample channels in use (soundInfo->maxChans)
    EChan cgb[4];
    uint8_t reverb;
    float ring[7][ENG_FRAME_SAMPLES * 2];
    int ringPos;
    uint8_t c15;
    // GB APU (registers at their GBA offsets from 0x04000060)
    uint8_t io[0x50];
    struct {
        float vol[4];
        uint8_t Vol[4], VolI[4], Len[4], LenI[4], LenOn[4], EnvCounter[4], EnvCounterI[4], EnvDir[4], DAC[4];
        float WAVRAM[32];
        uint16_t ch1Freq;
        uint8_t ch1SweepCounter, ch1SweepCounterI, ch1SweepDir, ch1SweepShift;
        uint16_t ch4LFSR[2];
    } gb;
    float soundChannelPos[4];
    const short *PU1Table, *PU2Table;
    uint32_t apuFrame;
    uint8_t apuCycle;
    float ch4Samples;
    float cgbOut[ENG_FRAME_SAMPLES * 2];
    // what the UI shows
    uint8_t trackLevel[ENG_MAX_TRACKS + 1];   // a short flash when a track plays a note
} Engine;

void eng_init(Engine *e);
void eng_start(Engine *e, const ESong *song);
void eng_stop(Engine *e);
// Mixes one game frame (ENG_FRAME_SAMPLES stereo samples, interleaved L R).
void eng_frame(Engine *e, float *out);
// Runs frames without mixing (seeking). Returns frames run.
int eng_skip(Engine *e, int frames);
int eng_finished(const Engine *e);
int eng_ds_busy(const Engine *e);
// Audition an instrument on the extra track.
void eng_preview_on(Engine *e, const EInst *inst, int key, int velocity);
void eng_preview_off(Engine *e);
// Copies an engine (its channel / track links point into the copy).
void eng_copy(Engine *dst, const Engine *src);
// Tracks pick up their voicegroup entry again (after an instrument edit).
void eng_refresh_instruments(Engine *e);

#endif
