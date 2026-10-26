// Moemon Music Studio: turning the game's voice entries into engine instruments, and naming them.
#ifndef INST_H
#define INST_H

#include "sf.h"
#include "engine.h"

void inst_init(void);
void inst_make(const SfVoice *v, EInst *out);
const EWave *inst_wave(int sample);
int inst_find_preset(const SfVoice *v);        // -1 when no preset is exactly this voice
int voice_same(const SfVoice *a, const SfVoice *b);
const char *voice_kind(const SfVoice *v);      // "DirectSound", "Square 1", ...
void voice_name(const SfVoice *v, char *buf, int n);       // the preset's name, or a description
void voice_detail(const SfVoice *v, char *buf, int n);     // sample / duty / kit and the envelope
int voice_is_cgb(const SfVoice *v);
int voice_is_group(const SfVoice *v);
// The key a drum kit should play for MIDI drum note `key`: the key itself when the kit has a sound
// there, else the closest sound it has (same kind of drum first, then the nearest note). Not a kit: key.
int drum_resolve(const SfVoice *kit, int key);
// How loud a voice plays in the game engine (a note at full volume and velocity; the loudest
// 1/60 s of its first half second). Kits: the average of kick, snare and hi-hat.
float voice_loudness(const SfVoice *v);
// The factor that brings a voice to the loudness of the game's piano (what GM instruments are
// balanced against), kept within 1/4 and 4.
float voice_gm_gain(const SfVoice *v);
// Voice <-> the text the project file and exports use, e.g.
// "type=0x00 key=60 length=0 pansweep=0x00 ref=sample:sc88pro_flute adsr=255,0,255,165"
void voice_to_text(const SfVoice *v, char *buf, int n);
int voice_from_text(const char *s, SfVoice *v);
extern const SfVoice VOICE_PLACEHOLDER;        // what empty voicegroup slots hold in the game

// Samples and Game Boy waves a project brings with it (music ripped from other games). A voice's
// ref of INST_CUSTOM + i names custom sample / wave i.
#define INST_CUSTOM 10000
#define INST_MAX_CUSTOM 1024
typedef struct {
    char name[40];           // [a-z0-9_], unique in the project
    int8_t *data;            // 8-bit signed PCM, like the game's samples
    uint32_t size, loopStart, freq;   // freq: 22.10 fixed point, the rate at key 60 (WaveData.freq)
    int loops;
    int isWave;              // a Game Boy programmable wave: 16 bytes (32 4-bit steps), no rate
} CustomSample;
// The open project's custom samples (the project owns the list; this only points at it).
void inst_custom_bind(CustomSample *list, int n);
// Appends a copy of s (its data copied, with zero padding past the end) to a list. Returns its ref
// (INST_CUSTOM + index), or -1.
int custom_list_add(CustomSample **list, int *n, const CustomSample *s);
void custom_list_free(CustomSample **list, int *n);
int inst_custom_count(void);
const CustomSample *inst_custom_get(int ref);     // NULL when ref isn't a custom one
int inst_is_custom_ref(int ref);
int custom_by_name(const char *name);             // its ref, or -1

// Drum kits / key-split instrument groups and key-split tables a project brings (from GBA rips).
// A voice's sub of INST_CUSTOM + i is custom group i; its ks of INST_CUSTOM + i is custom table i.
typedef struct { char name[40]; SfVoice v[128]; } CustomGroup;
typedef struct { char name[40]; uint8_t t[128]; } CustomSplit;
// Binds the project's groups and tables (after its samples: the groups' voices can play them).
void inst_custom_groups_bind(CustomGroup *g, int ng, CustomSplit *k, int nk);
const SfVoice *group_voices(int sub);            // the 128 voices of a game or custom group (NULL)
const uint8_t *keysplit_table(int ks);           // a game or custom key-split table (NULL)
const char *group_name(int sub);                 // "" when unknown
const char *keysplit_name(int ks);
int inst_is_custom_group(int sub);
int inst_is_custom_split(int ks);

int sample_by_name(const char *name);
int wave_by_name(const char *name);
int group_by_name(const char *name);
int keysplit_by_name(const char *name);
int song_by_name(const char *name);
int song_group_by_name(const char *name);

#endif
