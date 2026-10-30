// Moemon Music Studio: the game's instruments, as gen_soundfont.py extracted them.
#ifndef SF_H
#define SF_H

#include <stdint.h>

typedef struct {
    const char *name;        // symbol without "DirectSoundWaveData_"
    uint32_t flags;          // wav2agb header word 0 (bit 30 = loops)
    uint32_t freq;           // 22.10 fixed point, like WaveData.freq
    uint32_t loopStart;
    uint32_t size;           // samples
    uint32_t offset;         // into sf_pcm
} SfSample;

typedef struct {
    const char *name;        // "ProgrammableWaveData_N"
    uint32_t offset;         // 16 bytes in sf_pcm
} SfWave;

// One voicegroup entry, with the raw ToneData bytes and what its pointer points at.
typedef struct {
    uint8_t type, key, length, panSweep;
    int16_t ref;             // DirectSound: sample index; square: duty; wave: wave index; noise: period
    int16_t sub;             // key-split / drum kit: group index into sf_groups
    int16_t ks;              // key-split: table index into sf_keysplits
    uint8_t attack, decay, sustain, release;
} SfVoice;

typedef struct {
    const char *name;
    int cat;
    const char *songs;       // song voicegroups that use it
    int uses;
    SfVoice voice;
} SfPreset;

typedef struct {
    const char *name;        // mus_*
    const char *group;       // its voicegroup (midi.cfg -G)
    int vol, reverb, priority, exact;
} SfSong;

// Where a song plays (tools/kaisong/gen.py): kind = an index into sf_kind_names (map music, scenes, events, battles...)
typedef struct {
    short song;              // sf_songs index
    unsigned char kind;
    const char *place;       // map key (mus_x@<place>.kaisong), "" = not one map
    const char *label;
} SfPlace;

typedef struct { const char *key, *name; } SfMap;

#include "sf_counts.h"

extern const unsigned char sf_pcm[];
extern const SfSample sf_samples[SF_NUM_SAMPLES];
extern const SfWave sf_waves[SF_NUM_WAVES];
extern const char *const sf_group_names[SF_NUM_GROUPS];
extern const SfVoice sf_groups[SF_NUM_GROUPS][128];
extern const char *const sf_keysplit_names[SF_NUM_KEYSPLITS];
extern const unsigned char sf_keysplits[SF_NUM_KEYSPLITS][128];
extern const char *const sf_cat_names[SF_NUM_CATS];
extern const SfPreset sf_presets[SF_NUM_PRESETS];
extern const char *const sf_gm_names[128];
extern const short sf_gm_preset[128];
extern const SfSong sf_songs[SF_NUM_SONGS];
extern const SfPlace sf_places[SF_NUM_PLACES];
extern const char *const sf_kind_names[SF_NUM_KINDS];   // SfPlace.kind's group names, in picker order
extern const SfMap sf_maps[SF_NUM_MAPS];
extern const char sf_table_intro[];
extern const short sf_song_num[SF_NUM_SONGS];        // the game's song number
extern const char *const sf_song_md[SF_NUM_SONGS];   // SONG_TABLE.md: the lines under the song's heading
extern const char *const sf_song_group_names[SF_NUM_SONG_GROUPS];
extern const SfVoice sf_song_groups[SF_NUM_SONG_GROUPS][128];
extern const unsigned char sf_gScaleTable[180];
extern const unsigned int sf_gFreqTable[12];
extern const unsigned char sf_gClockTable[49];
extern const unsigned char sf_gCgbScaleTable[132];
extern const short sf_gCgbFreqTable[12];
extern const unsigned char sf_gNoiseTable[60];
extern const unsigned char sf_gCgb3Vol[16];
extern const short sf_PU0[32], sf_PU1[32], sf_PU2[32], sf_PU3[32];
extern const float sf_freqTable[2048];
extern const float sf_freqTableNSE[256];
extern const unsigned char sf_noteDurationLUT[97];
extern const unsigned char sf_noteVelocityLUT[128];

#endif
