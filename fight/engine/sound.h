#ifndef MUGEN_SOUND_H
#define MUGEN_SOUND_H

/*
 * Types of the sound effects module (sound.c) and of the code it uses from
 * other, not yet decompiled modules: the id map container (0x466550..) and
 * the WAV reader / resampler (0x4886E0..).
 */

/* --------------------------------------------------- WAV reader / resampler */

/* AudioFormat.format: sample format flags of the WAV reader and resampler. */
#define AUDIO_FMT_U8    0x06    /* unsigned 8-bit */
#define AUDIO_FMT_S16   0x49    /* signed 16-bit, native endian */

typedef struct AudioFormat {
    double rate;                /* 0x00 samples per second */
    int format;                 /* 0x08 AUDIO_FMT_* */
    int channels;               /* 0x0C */
} AudioFormat;
MUGEN_ASSERT_SIZE(AudioFormat, 0x10);

/* WAV status codes (Wav_StrError). */
enum {
    WAV_OK = 0,
    WAV_ERR_FORMAT = 1,         /* not PCM */
    WAV_ERR_CORRUPT = 2,
    WAV_ERR_CHANNELS = 3,
    WAV_ERR_SAMPLE_SIZE = 4,
    WAV_ERR_NOMEM = 5,
    WAV_ERR_IO = 6,
    WAV_ERR_RATE = 7,
    WAV_ERR_UNKNOWN = 8
};

#define WAV_TAG_PCM 1

/* What Wav_ReadInfo found in a RIFF/WAVE stream. */
typedef struct WavInfo {
    AudioFormat format;         /* 0x00 */
    int format_tag;             /* 0x10 WAV_TAG_PCM */
    long data_offset;           /* 0x14 stream offset of the sample data */
    unsigned int data_size;     /* 0x18 bytes */
    unsigned int frames;        /* 0x1C data_size / frame size */
} WavInfo;
MUGEN_ASSERT_SIZE(WavInfo, 0x20);

/* Sample reader over a WAV stream (WavReader_Open). */
typedef struct WavReader {
    Stream *stream;             /* 0x00 */
    int pad_04;                 /* 0x04 never accessed by the game */
    WavInfo info;               /* 0x08 */
    unsigned int frames_read;   /* 0x28 */
    int unused_2c;              /* 0x2C cleared, never read */
} WavReader;
MUGEN_ASSERT_OFFSET(WavReader, frames_read, 0x28);
MUGEN_ASSERT_SIZE(WavReader, 0x30);

/* ResamplerVtbl.state results. */
enum {
    RESAMPLER_OUTPUT = 0,       /* converted frames can be read */
    RESAMPLER_INPUT = 1,        /* needs more input frames */
    RESAMPLER_DONE = 2
};

typedef struct ResamplerVtbl ResamplerVtbl;
struct Resampler;

/* AudioFormat.format of the float format the resampler works in. */
#define AUDIO_FMT_F32   0x50

/*
 * Sample format converter (Audio_GetConverter, 0x49AB10): frames from src
 * (src_channels interleaved) to dst (dst_channels).
 */
typedef void (__cdecl *AudioConvertFn)(void *dst, const void *src, unsigned int frames, int dst_channels,
                                       int src_channels);

/* State of a conversion (Resampler_Open). */
typedef struct ResamplerState {
    void *rs;                   /* 0x00 libresample handle (resampling only) */
    float *out_float;           /* 0x04 resampled samples: buf_frames frames of mid.channels */
    float *in_float;            /* 0x08 input converted to float: in_block frames of mid.channels */
    int buf_frames;             /* 0x0C */
    unsigned char *out_buf;     /* 0x10 converted frames in the out format, out_capacity frames */
    unsigned int out_count;     /* 0x14 frames waiting in out_buf */
    unsigned int out_capacity;  /* 0x18 */
    int resampling;             /* 0x1C in.rate != out.rate */
    unsigned int chunk;         /* 0x20 frames per output block */
    unsigned int in_block;      /* 0x24 input frames per block: ceil(chunk / ratio) */
    AudioFormat out;            /* 0x28 */
    AudioFormat in;             /* 0x38 */
    AudioFormat mid;            /* 0x48 float at the input rate, fewest channels (resampling only) */
    AudioConvertFn convert_in;  /* 0x58 in -> mid, or in -> out without resampling */
    AudioConvertFn convert_out; /* 0x5C mid -> out */
    int last;                   /* 0x60 the final input block was written */
    int pad_64;                 /* 0x64 never accessed by the game */
} ResamplerState;
MUGEN_ASSERT_OFFSET(ResamplerState, out, 0x28);
MUGEN_ASSERT_OFFSET(ResamplerState, convert_in, 0x58);
MUGEN_ASSERT_SIZE(ResamplerState, 0x68);

/* One conversion in progress. */
typedef struct ResamplerSession {
    ResamplerState *state;      /* 0x00 */
    const ResamplerVtbl *vtbl;  /* 0x04 */
} ResamplerSession;
MUGEN_ASSERT_SIZE(ResamplerSession, 0x08);

typedef ResamplerSession *(__cdecl *ResamplerOpenFn)(const AudioFormat *out, const AudioFormat *in, int chunk,
                                                     const int *quality);
typedef struct Resampler *(__cdecl *ResamplerCreateFn)(const AudioFormat *out, int chunk, const int *quality);
typedef void (__cdecl *ResamplerCloseFn)(ResamplerSession *s);
typedef unsigned int (__cdecl *ResamplerCountFn)(ResamplerSession *s);
typedef int (__cdecl *ResamplerStateFn)(ResamplerSession *s);
typedef unsigned int (__cdecl *ResamplerWriteFn)(ResamplerSession *s, const void *in, unsigned int frames, int last);
typedef unsigned int (__cdecl *ResamplerReadFn)(ResamplerSession *s, void *out, unsigned int frames);
typedef const AudioFormat *(__cdecl *ResamplerGetFn)(ResamplerSession *s);
typedef void (__cdecl *ResamplerResetFn)(ResamplerSession *s);

/* Function table of the resampler (0x4F1890 in .rdata, g_resamplerVtbl). */
struct ResamplerVtbl {
    ResamplerOpenFn open;           /* 0x00 0x488CD0 Resampler_Open */
    ResamplerCreateFn create;       /* 0x04 0x489330 Resampler_Create */
    ResamplerCloseFn close;         /* 0x08 0x489020 */
    ResamplerCountFn input_needed;  /* 0x0C 0x489080 frames to write next */
    ResamplerStateFn state;         /* 0x10 0x4890A0 RESAMPLER_* */
    ResamplerWriteFn write;         /* 0x14 0x4890D0 */
    ResamplerReadFn read;           /* 0x18 0x489280 */
    ResamplerGetFn in_format;       /* 0x1C 0x4893F0 */
    ResamplerGetFn out_format;      /* 0x20 0x489400 */
    ResamplerResetFn reset;         /* 0x24 0x489410 */
    ResamplerCountFn in_block;      /* 0x28 0x489440 input block size in frames */
    ResamplerCountFn out_block;     /* 0x2C 0x489430 output block size in frames */
};
MUGEN_ASSERT_SIZE(ResamplerVtbl, 0x30);

typedef ResamplerSession *(__cdecl *ResamplerBeginFn)(struct Resampler *r, const WavInfo *in);

/* Converter to the mixer format (Resampler_Create), used by the libresample SFX path. */
typedef struct Resampler {
    ResamplerBeginFn begin;     /* 0x00 0x4893D0: vtbl->open(&out, in, chunk, quality) */
    const ResamplerVtbl *vtbl;  /* 0x04 */
    AudioFormat out;            /* 0x08 */
    int chunk;                  /* 0x18 mixer buffer size in frames */
    int *quality;               /* 0x1C malloc'ed copy of the quality */
} Resampler;
MUGEN_ASSERT_OFFSET(Resampler, out, 0x08);
MUGEN_ASSERT_SIZE(Resampler, 0x20);

/* ---------------------------------------------------------------- sounds */

/* GameVars.sfx_resample_method / g_sndResampleMethod. */
enum {
    SND_RESAMPLE_SDL = 0,       /* Mix_LoadWAV_RW converts */
    SND_RESAMPLE_LIBRESAMPLE = 1
};

/* Status codes of Snd_Parse. */
enum {
    SND_OK = 0,
    SND_ERR_FAILED = 1,         /* nothing returned */
    SND_ERR_IGNORED = 2         /* some sounds could not be decoded and were skipped */
};

#define SND_ENTRY_CHUNK 1

/* Element of Snd.map. */
typedef struct SndEntry {
    int unused_00;              /* 0x00 1, never read */
    int type;                   /* 0x04 SND_ENTRY_CHUNK */
    int unused_08;              /* 0x08 0, never read */
    struct Mix_Chunk *chunk;    /* 0x0C abuf is ours to free when !allocated */
} SndEntry;
MUGEN_ASSERT_SIZE(SndEntry, 0x10);

/* A sound bank: a .snd file, or a single .wav as sound 0,0. */
typedef struct Snd {
    KeyedArray *map;                 /* 0x00 SndEntry by (group, number) */
} Snd;
MUGEN_ASSERT_SIZE(Snd, 0x04);

/* .snd file layout. */
#define SND_SIGNATURE "ElecbyteSnd"

typedef struct SndFileHeader {
    char signature[12];         /* 0x00 SND_SIGNATURE */
    unsigned char version[4];   /* 0x0C */
    int count;                  /* 0x10 number of sounds */
    int first;                  /* 0x14 offset of the first SndSubHeader */
    char comment[0x1E8];        /* 0x18 */
} SndFileHeader;
MUGEN_ASSERT_SIZE(SndFileHeader, 0x200);

typedef struct SndSubHeader {
    int next;                   /* 0x00 offset of the next subheader */
    int size;                   /* 0x04 bytes of WAV data that follow */
    int group;                  /* 0x08 */
    int number;                 /* 0x0C */
} SndSubHeader;
MUGEN_ASSERT_SIZE(SndSubHeader, 0x10);

/* Number of entries in g_sndSlots. */
#define SOUND_SLOT_COUNT 0x100

/*
 * A playing (or prepared) sound effect. owner identifies who started it
 * (Sound_Play id; -1 = nobody); channel is the SDL_mixer channel or -1.
 */
typedef struct SoundSlot {
    int owner;                  /* 0x00 */
    int channel;                /* 0x04 */
    int reserved;               /* 0x08 never set; nonzero keeps the slot from being reused */
    struct Mix_Chunk *chunk;    /* 0x0C */
    double volume;              /* 0x10 percent, 0..100 */
    int mix_volume;             /* 0x18 volume * 1.28 (0..128) */
    int pad_1c;                 /* 0x1C never accessed by the game */
    int pad_20;                 /* 0x20 never accessed by the game */
    int loop;                   /* 0x24 play looped */
} SoundSlot;
MUGEN_ASSERT_OFFSET(SoundSlot, volume, 0x10);
MUGEN_ASSERT_OFFSET(SoundSlot, loop, 0x24);
MUGEN_ASSERT_SIZE(SoundSlot, 0x28);

#endif /* MUGEN_SOUND_H */
