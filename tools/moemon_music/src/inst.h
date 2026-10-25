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

int sample_by_name(const char *name);
int wave_by_name(const char *name);
int group_by_name(const char *name);
int keysplit_by_name(const char *name);
int song_by_name(const char *name);
int song_group_by_name(const char *name);

#endif
