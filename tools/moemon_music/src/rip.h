// Moemon Music Studio: music ripped from other consoles (PSF, PSF2, USF, GBS, 2SF, GSF).
// Each file plays in an emulator of its console. While it plays, what the sound chip is told is
// written down a frame at a time (the game's frames, ENG_RATE / ENG_FRAME_SAMPLES): every voice's
// sample, pitch and volume. rip_song.c turns that into a MIDI and instruments for the studio.
#ifndef RIP_H
#define RIP_H

#include <stdint.h>

enum { RIP_PSF1, RIP_PSF2, RIP_USF, RIP_GBS, RIP_2SF, RIP_GSF, RIP_NUM_KINDS };

#define RIP_MAX_VOICES 64
#define RIP_MAX_SECONDS 900          // longest a rip plays (15 minutes)

// What a chip voice plays.
enum {
    RIPS_PCM,                        // a sample (pcm at rate, maybe looping)
    RIPS_SQUARE,                     // Game Boy / DS square wave, param = duty 0-3 (GB: 12.5/25/50/75%) or DS 0-7
    RIPS_WAVE,                       // Game Boy programmable wave: wave[32] 4-bit
    RIPS_NOISE,                      // noise, param = 1 for the short (7-bit) kind
};

typedef struct {
    int kind;
    int16_t *pcm;                    // RIPS_PCM
    int len, rate;                   // samples; the rate it plays at when the chip's pitch is 1.0
    int loopStart, loops;            // loops back to loopStart at the end
    int param;
    uint8_t wave[32];
    uint32_t id;                     // the chip's own name for it (sound RAM address...)
    uint32_t hash;
    // filled by rip_song.c
    float rootKey;                   // MIDI key of the sample played at `rate` (pitch detection); 60 unknown
    int pitched;
    float peak, rms;                 // of the PCM (0-1)
    char name[32];
} RipSample;

// One voice in one frame.
typedef struct {
    int16_t sample;                  // RipSample index, -1 = silent
    uint8_t keyon;                   // started (again) during this frame
    uint8_t flags;                   // RIPV_*
    float rate;                      // the rate the sample plays at (Hz); squares / waves: the note's frequency (Hz)
    float l, r;                      // volume with the envelope, linear: 1 = the chip's loudest
} RipVF;

#define RIPV_REVERB 1                // the voice goes to the reverb

typedef struct Rip {
    int kind;
    char path[1024];
    char title[128], artist[128], game[128], year[32], copyright[128], ripper[128];
    double length, fade;             // seconds from the tags (0 = not given)
    int track;                       // GBS: the track number played
    // the original sound (stereo int16 at `rate`), for the original-instruments player
    int rate;
    int16_t *pcm;
    int pcmFrames;                   // stereo frames
    // what the chip did, one row per game frame
    int nvoices, nframes;
    RipVF *vf;                       // [frame * nvoices + voice]
    int nsamples, capSamples;
    RipSample *samples;
    float reverb;                    // how much reverb the song uses (0-1)
    char console[32];
} Rip;

// Loads and plays a rip (and its _lib files) to the end of its length + fade (or `seconds` when the
// tags give none; 0 = 180). biosDir: where to look for a PlayStation 2 BIOS (PSF / PSF2 need one).
// gbsTrack: the GBS subsong (-1 = from the .m3u or the first). Returns 0 and sets err on failure.
int rip_load(Rip *r, const char *path, const char *biosDir, int gbsTrack, int seconds, char *err, int errlen);
void rip_free(Rip *r);
int rip_kind_of(const char *path);                 // -1 = not a rip
const char *rip_kind_name(int kind);
int rip_needs_bios(int kind);

// for the console modules
int rip_add_sample(Rip *r, const RipSample *s);    // copies it (and its pcm); returns its index, or the
                                                   // index of the same sample already added
int rip_find_sample(const Rip *r, uint32_t id, uint32_t hash);
uint32_t rip_hash(const void *data, int len);

// A console module: plays the file into stereo int16 at `rate` and reports its voices.
typedef struct RipEmu {
    int rate, nvoices;
    void *ctx;
    Rip *rip;
    // Renders exactly n stereo frames. Returns 0 on success.
    int (*render)(struct RipEmu *e, int16_t *buf, int n);
    // Every voice's state now (out[nvoices]); key-ons since the last call are reported and cleared.
    void (*snapshot)(struct RipEmu *e, RipVF *out);
    void (*close)(struct RipEmu *e);
} RipEmu;

int rip_open_psf(RipEmu *e, Rip *r, const char *biosDir, char *err, int errlen);
int rip_open_usf(RipEmu *e, Rip *r, char *err, int errlen);
int rip_open_gbs(RipEmu *e, Rip *r, int track, char *err, int errlen);
int rip_open_2sf(RipEmu *e, Rip *r, char *err, int errlen);
int rip_open_gsf(RipEmu *e, Rip *r, char *err, int errlen);

// psflib tags -> Rip fields (title, artist, game, year, copyright, length, fade...)
int rip_psf_tag(Rip *r, const char *name, const char *value);
double rip_parse_time(const char *s);

#endif
