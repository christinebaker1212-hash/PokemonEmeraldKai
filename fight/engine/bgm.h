#ifndef MUGEN_BGM_H
#define MUGEN_BGM_H

/*
 * Types of the background music modules: the plugin registry and BGM API
 * (bgm.c), the BGM thread with its command queue and mixer callback
 * (bgmthread.c), and what they use of the BGM stream loader (0x49BA00..,
 * not decompiled yet).
 *
 * BGM commands are queued under g_bgmMutex, stamped with the game frame
 * (g_bgmGetHandle), and run by the BGM thread. Streams decoded by the engine
 * (BGM_MODE_DECODED) go through a ring buffer of blocks that the thread fills
 * and the SDL_mixer music hook (BgmThread_MixCallback) plays.
 */

/* Growable array of elem_size-byte elements; the original inlines its code everywhere. */
typedef struct DynArray {
    void *data;                 /* 0x00 capacity * elem_size bytes */
    int elem_size;              /* 0x04 */
    int capacity;               /* 0x08 doubled when full */
    int count;                  /* 0x0C */
} DynArray;
MUGEN_ASSERT_SIZE(DynArray, 0x10);

/* Initial capacity of the arrays created by the BGM code. */
#define BGM_ARRAY_CAPACITY 8

/* ------------------------------------------------------------------ plugins */

/* Settings of a [Music] plugin: parallel lists of key and value strings (BgmSettings_New). */
typedef struct BgmSettings {
    DynArray *keys;             /* 0x00 char * */
    DynArray *values;           /* 0x04 char * */
} BgmSettings;
MUGEN_ASSERT_SIZE(BgmSettings, 0x08);

/* A "plugin = name [, file]" entry of mugen.cfg [Music] (GameVars.music_plugins holds them). */
typedef struct BgmPluginEntry {
    char *name;                 /* 0x00 */
    char *file;                 /* 0x04 plugin DLL, NULL when not given */
    char *extensions;           /* 0x08 "extensions" setting, else the plugin's default once registered */
    BgmSettings *settings;      /* 0x0C the other settings; Bgm_Init frees them after the plugin's init */
} BgmPluginEntry;
MUGEN_ASSERT_SIZE(BgmPluginEntry, 0x10);

/* A BGM and its playback parameters ("<prefix>.bgm", ".volume", ".loop"... of a .def; Bgm_ReadInfo). */
typedef struct BgmInfo {
    char path[0x208];           /* 0x000 lower-cased, at most 0x200 characters */
    double volume;              /* 0x208 percent (default 100) */
    int loop;                   /* 0x210 default 1 */
    int loop_start;             /* 0x214 -1 when not given */
    int loop_end;               /* 0x218 -1 when not given or not after loop_start */
    int unused_21c;             /* 0x21C cleared by Stage_Activate, never read */
} BgmInfo;
MUGEN_ASSERT_OFFSET(BgmInfo, volume, 0x208);
MUGEN_ASSERT_SIZE(BgmInfo, 0x220);

/* Output format and settings given to a plugin's init. */
typedef struct BgmPluginConfig {
    AudioFormat format;         /* 0x00 mixer format */
    BgmSettings *settings;      /* 0x10 read with the BgmSettingFn */
} BgmPluginConfig;
MUGEN_ASSERT_OFFSET(BgmPluginConfig, settings, 0x10);
MUGEN_ASSERT_SIZE(BgmPluginConfig, 0x18);

typedef const char *(__cdecl *BgmGetExtensionFn)(const char *path);
typedef void (__cdecl *BgmGetLoopFn)(const BgmInfo *info, int *loop);
typedef const char *(__cdecl *BgmSettingFn)(const BgmSettings *settings, const char *key);

/* Engine services given to a plugin's init. */
typedef struct BgmHostCallbacks {
    BgmGetExtensionFn get_extension;    /* 0x00 Bgm_GetExtension */
    BgmGetLoopFn get_loop;              /* 0x04 Bgm_GetLoopInfo */
    void *reserved;                     /* 0x08 NULL: a callback slot the host leaves empty */
} BgmHostCallbacks;
MUGEN_ASSERT_SIZE(BgmHostCallbacks, 0x0C);

typedef unsigned int (__cdecl *BgmVersionFn)(void);
typedef const char *(__cdecl *BgmExtensionsFn)(void);
typedef int (__cdecl *BgmPluginInitFn)(const BgmPluginConfig *config, const BgmHostCallbacks *host,
                                       BgmSettingFn get_setting);
typedef void (__cdecl *BgmPluginDeinitFn)(void);
typedef int (__cdecl *BgmInstanceFn)(void *instance);
typedef int (__cdecl *BgmInstanceMuteFn)(void *instance, int muted);

/* File access the stream loader gives a plugin's open (Stream functions). */
typedef size_t (__cdecl *BgmIoReadFn)(void *buf, size_t size, size_t count, void *handle);
typedef int (__cdecl *BgmIoSeekFn)(void *handle, long offset, int origin);
typedef long (__cdecl *BgmIoTellFn)(void *handle);
typedef int (__cdecl *BgmIoEofFn)(void *handle);

typedef struct BgmPluginIo {
    void *handle;                       /* 0x00 Stream * */
    BgmIoReadFn read;                   /* 0x04 Stream_Read */
    BgmIoSeekFn seek;                   /* 0x08 Stream_Seek */
    BgmIoTellFn tell;                   /* 0x0C Stream_Tell */
    BgmIoEofFn eof;                     /* 0x10 Stream_Eof */
} BgmPluginIo;
MUGEN_ASSERT_SIZE(BgmPluginIo, 0x14);

/* What a plugin's open reports (BgmStream.mode onwards). */
typedef struct BgmOpenResult {
    int mode;                           /* 0x00 BGM_MODE_* */
    int pad_04;                         /* 0x04 never accessed by the game */
    AudioFormat format;                 /* 0x08 decoder output (BGM_MODE_DECODED) */
} BgmOpenResult;
MUGEN_ASSERT_SIZE(BgmOpenResult, 0x18);

/* BgmReadFn events: what happened at the end of the data read. */
enum {
    BGM_READ_EVENT_NONE = 0,
    BGM_READ_EVENT_BLOCK = 1,           /* a block is complete: give it to the resampler */
    BGM_READ_EVENT_END = 2,             /* the stream ended: flush the resampler */
    BGM_READ_EVENT_FORMAT = 3           /* the output format changes after this data */
};

typedef void *(__cdecl *BgmCreateFn)(void);
typedef void (__cdecl *BgmDestroyFn)(void *instance);
typedef int (__cdecl *BgmOpenFn)(void *instance, const BgmPluginIo *io, BgmInfo *info, BgmOpenResult *out);
typedef int (__cdecl *BgmSetVolumeFn)(void *instance, double volume);
typedef const char *(__cdecl *BgmStrErrorFn)(void *instance, int err);
typedef int (__cdecl *BgmReadFn)(void *instance, void *buf, unsigned int size, unsigned int *got, int *event);
typedef int (__cdecl *BgmGetFormatFn)(void *instance, AudioFormat *format);

/*
 * Function table of a BGM plugin ("subsystem"): getBGMSubsystem() of a
 * plugin DLL, or the built-in mpg123 (0x4F1818) and sdlmix (0x4F17A0)
 * tables. Only the entries the BGM modules call are typed; the rest are used
 * by the stream loader.
 */
typedef struct BgmSubsystem {
    BgmVersionFn version;               /* 0x00 a.b.c.d as 0xAABBCCDD; at least 1.0.0.0 */
    BgmExtensionsFn extensions;         /* 0x04 default extension list ("mp3", "ogg,vorbis"...) */
    BgmPluginInitFn init;               /* 0x08 0 on success */
    BgmPluginDeinitFn deinit;           /* 0x0C */
    BgmCreateFn create;                 /* 0x10 new decoder instance */
    BgmDestroyFn destroy;               /* 0x14 */
    void *is_playing;                   /* 0x18 not called by the game (the slots named after the built-in plugins' functions) */
    BgmInstanceFn start;                /* 0x1C */
    BgmInstanceFn stop;                 /* 0x20 */
    void *rewind;                       /* 0x24 */
    void *seek;                         /* 0x28 */
    BgmOpenFn open;                     /* 0x2C */
    BgmInstanceFn close;                /* 0x30 */
    void *get_volume;                   /* 0x34 */
    BgmSetVolumeFn set_volume;          /* 0x38 (instance, double percent) */
    void *get_loop_flag;                /* 0x3C */
    void *set_loop_flag;                /* 0x40 */
    void *get_loop_points;              /* 0x44 */
    void *set_loop_points;              /* 0x48 */
    void *set_end_callback;             /* 0x4C */
    BgmStrErrorFn strerror;             /* 0x50 */
    BgmReadFn read;                     /* 0x54 decoded data (BGM_MODE_DECODED) */
    BgmGetFormatFn get_format;          /* 0x58 output format after BGM_READ_EVENT_FORMAT */
    BgmInstanceFn start_paused;         /* 0x5C start of a held BGM_MODE_PLUGIN stream */
    void *is_paused;                    /* 0x60 */
    BgmInstanceFn pause;                /* 0x64 */
    BgmInstanceFn resume;               /* 0x68 */
    BgmInstanceMuteFn set_mute;         /* 0x6C */
    void *get_mute;                     /* 0x70 */
    void *reserved;                     /* 0x74 NULL in both built-in plugins, never called */
} BgmSubsystem;
MUGEN_ASSERT_OFFSET(BgmSubsystem, set_volume, 0x38);
MUGEN_ASSERT_OFFSET(BgmSubsystem, pause, 0x64);
MUGEN_ASSERT_SIZE(BgmSubsystem, 0x78);

/* Oldest plugin version accepted by Bgm_RegisterSubsystem (1.0.0.0). */
#define BGM_MIN_VERSION 0x01000000u

/* ------------------------------------------------------------------ streams */

/* BgmStream.mode */
enum {
    BGM_MODE_PLUGIN = 0,        /* the plugin plays it (sdlmix) */
    BGM_MODE_DECODED = 1        /* decoded into the ring buffer, played by BgmThread_MixCallback */
};

/* BgmStream.flags: reasons the stream is paused. */
#define BGM_STREAM_HELD   0x01  /* Bgm_Pause; streams are also prebuffered with it */
#define BGM_STREAM_PAUSED 0x04  /* window focus lost with BGMBackgroundMode = Pause */

/* State of the current BGM (created by the stream loader, 0x49C070: calloc 0x278). */
typedef struct BgmStream {
    BgmInfo info;                       /* 0x000 path[0] = 0: no BGM */
    Stream *file;                       /* 0x220 */
    const BgmSubsystem *plugin;         /* 0x224 */
    void *instance;                     /* 0x228 plugin decoder instance */
    int pad_22c;                        /* 0x22C never accessed by the game */
    int mode;                           /* 0x230 BGM_MODE_* */
    int pad_234;                        /* 0x234 never accessed by the game */
    AudioFormat format;                 /* 0x238 decoder output format */
    unsigned int flags;                 /* 0x248 BGM_STREAM_* */
    int pad_24c;                        /* 0x24C never accessed by the game */
    double volume;                      /* 0x250 info.volume */
    int playing;                        /* 0x258 started */
    int muted;                          /* 0x25C */
    ResamplerSession *resampler;        /* 0x260 */
    unsigned char *decode_buf;          /* 0x264 */
    unsigned int decode_capacity;       /* 0x268 */
    unsigned int decode_size;           /* 0x26C */
    unsigned int block_bytes;           /* 0x270 bytes decoded per ring block */
    int format_changed;                 /* 0x274 the decoder reported a new output format (BGM_READ_EVENT_FORMAT): the
                                           resampler is set up again once it has drained */
} BgmStream;
MUGEN_ASSERT_OFFSET(BgmStream, plugin, 0x224);
MUGEN_ASSERT_OFFSET(BgmStream, mode, 0x230);
MUGEN_ASSERT_OFFSET(BgmStream, flags, 0x248);
MUGEN_ASSERT_OFFSET(BgmStream, playing, 0x258);
MUGEN_ASSERT_SIZE(BgmStream, 0x278);

/*
 * Ring buffer between the BGM thread and the mixer callback (0x49BA00):
 * blocks of a 4-byte sequence number followed by one mixer chunk of
 * samples. The callback adds a block to "space" when it has played it, the
 * thread fills a block whenever space allows.
 */
typedef struct BgmRing {
    unsigned char *buf;                 /* 0x00 count blocks */
    int block;                          /* 0x04 bytes per block */
    int count;                          /* 0x08 32 */
    unsigned int size;                  /* 0x0C block * count */
    long filled;                        /* 0x10 sample bytes written since the last reset (atomic) */
    unsigned int read;                  /* 0x14 next block the callback plays */
    int paused;                         /* 0x18 the callback plays silence */
    long space;                         /* 0x1C bytes the thread may fill (atomic) */
    unsigned int write;                 /* 0x20 next block the thread fills */
    int stopped;                        /* 0x24 the thread does not fill */
} BgmRing;
MUGEN_ASSERT_SIZE(BgmRing, 0x28);

#define BGM_RING_BLOCKS 32

/* ------------------------------------------------------------------ thread */

/* Returns the time stamp of queued BGM commands: the game frame (g_bgmGetHandle). */
typedef int (__cdecl *BgmHandleFn)(void);

/* BgmCommand.cmd */
enum {
    BGM_CMD_LOAD = 0,           /* data: BgmInfo * (owned) */
    BGM_CMD_PLAY = 1,           /* load if needed and start; data: BgmInfo * (owned) */
    BGM_CMD_UNLOAD = 2,
    BGM_CMD_START = 3,
    BGM_CMD_STOP = 4,           /* stop and flush the ring buffer */
    BGM_CMD_HOLD = 5,           /* data[0]: BGM_STREAM_* flags to set */
    BGM_CMD_PREBUFFER = 6,
    BGM_CMD_UNHOLD = 7,         /* data[0]: flags to clear */
    BGM_CMD_PAUSE = 8,          /* data[0]: flags to set and keep for later streams */
    BGM_CMD_RESUME = 9,         /* data[0]: flags to clear (and forget) */
    BGM_CMD_SET_STREAM = 10,    /* data: BgmStream * */
    BGM_CMD_VOLUME = 11,        /* data: double percent, -1 = keep the stream's own */
    BGM_CMD_MUTE = 12,          /* data[0]: 1 to mute, 0 to unmute */
    BGM_CMD_QUIT = 13           /* ends the thread */
};

/* Entry of g_bgmQueue. */
typedef struct BgmCommand {
    int cmd;                    /* 0x00 BGM_CMD_* */
    int time;                   /* 0x04 g_bgmGetHandle() when queued; -1 for BGM_CMD_QUIT */
    intptr_t data[2];           /* 0x08 flags, a pointer or a double (Moemon Fight: pointer-sized words) */
    int owns_data;              /* 0x10 data[0] is a malloc'ed block freed with the command */
    int pad_14;                 /* 0x14 never accessed by the game */
} BgmCommand;
MUGEN_ASSERT_SIZE(BgmCommand, 0x18);

#define BGM_QUEUE_SIZE 16

/* Events of the decoder, handled when the mixer reaches the block they refer to. */
enum {
    BGM_EVENT_END = 1           /* the stream ended: stop and flush */
};

typedef struct BgmEvent {
    int type;                   /* 0x00 BGM_EVENT_* */
    int seq;                    /* 0x04 sequence number of the block */
    BgmStream *stream;          /* 0x08 */
} BgmEvent;
MUGEN_ASSERT_SIZE(BgmEvent, 0x0C);

#define BGM_EVENT_QUEUE_SIZE 16

/* pthreads-win32's pthread_t (g_bgmThread). */
typedef struct BgmThreadId {
    void *p;                    /* 0x00 */
    unsigned int x;             /* 0x04 */
} BgmThreadId;
MUGEN_ASSERT_SIZE(BgmThreadId, 0x08);

/* Start-up handshake of the BGM thread (BgmThread_Init): pthread_cond_t and pthread_mutex_t, both pointers. */
typedef struct BgmThreadStart {
    pthread_cond_t cond;        /* 0x00 (Moemon Fight: the real types, pointers in pthreads-win32) */
    pthread_mutex_t mutex;      /* 0x04 */
} BgmThreadStart;
MUGEN_ASSERT_SIZE(BgmThreadStart, 0x08);

/* ------------------------------------------------------------------ built-in plugins */

/* Called by a plugin when its stream ends by itself (plugins' set end callback). */
typedef void (__cdecl *BgmEndFn)(void *arg);

/* libmpg123 entry points (resolved by Mpg123_LoadSymbols); handles are mpg123_handle *. */
typedef struct Mpg123FrameInfo {
    int version;                /* 0x00 */
    int layer;                  /* 0x04 */
    long rate;                  /* 0x08 sample rate of the source */
    int mode;                   /* 0x0C */
    int mode_ext;               /* 0x10 */
    int framesize;              /* 0x14 */
    int flags;                  /* 0x18 */
    int emphasis;               /* 0x1C */
    int bitrate;                /* 0x20 */
    int abr_rate;               /* 0x24 */
    int vbr;                    /* 0x28 */
} Mpg123FrameInfo;
MUGEN_ASSERT_SIZE(Mpg123FrameInfo, 0x2C);

typedef int (__cdecl *Mpg123InitFn)(void);
typedef void (__cdecl *Mpg123ExitFn)(void);
typedef void *(__cdecl *Mpg123NewFn)(const char *decoder, int *error);
typedef void (__cdecl *Mpg123HandleFn)(void *mh);
typedef int (__cdecl *Mpg123HandleIntFn)(void *mh);
typedef int (__cdecl *Mpg123FormatFn)(void *mh, long rate, int channels, int encodings);
typedef int (__cdecl *Mpg123ParamFn)(void *mh, int type, long value, double fvalue);
typedef int (__cdecl *Mpg123GetFormatFn)(void *mh, long *rate, int *channels, int *encoding);
typedef int (__cdecl *Mpg123ReadFn)(void *mh, void *out, size_t size, size_t *done);
typedef int (__cdecl *Mpg123FeedFn)(void *mh, const void *in, size_t size);
typedef long (__cdecl *Mpg123FeedSeekFn)(void *mh, long sample, int whence, long *input_offset);
typedef const char *(__cdecl *Mpg123PlainStrErrorFn)(int error);
typedef const char *(__cdecl *Mpg123StrErrorFn)(void *mh);
typedef int (__cdecl *Mpg123VolumeFn)(void *mh, double volume);
typedef int (__cdecl *Mpg123InfoFn)(void *mh, Mpg123FrameInfo *info);

/* Instance of the mpg123 plugin (Mpg123_Create: 0x68 bytes plus the read buffer). */
typedef struct Mpg123Instance {
    void *mh;                   /* 0x00 mpg123_handle */
    int loop;                   /* 0x04 BgmInfo.loop */
    int loop_start_raw;         /* 0x08 loop points as given */
    int loop_end_raw;           /* 0x0C -1: the end */
    double volume;              /* 0x10 percent */
    BgmEndFn end_callback;      /* 0x18 */
    void *end_arg;              /* 0x1C */
    BgmPluginIo io;             /* 0x20 the open file */
    int pad_34;                 /* 0x34 never accessed by the game */
    AudioFormat format;         /* 0x38 decoder output */
    int frame_bytes;            /* 0x48 */
    int src_rate;               /* 0x4C sample rate of the source */
    int loop_start;             /* 0x50 output frames */
    int loop_end;               /* 0x54 */
    int pos;                    /* 0x58 output frames decoded */
    int last_error;             /* 0x5C mpg123 error code */
    int playing;                /* 0x60 */
    unsigned char readbuf[4];   /* 0x64 g_mpg123ReadBufferSize bytes */
} Mpg123Instance;
MUGEN_ASSERT_OFFSET(Mpg123Instance, io, 0x20);
MUGEN_ASSERT_OFFSET(Mpg123Instance, format, 0x38);
MUGEN_ASSERT_OFFSET(Mpg123Instance, readbuf, 0x64);
MUGEN_ASSERT_SIZE(Mpg123Instance, 0x68);

/* SdlmixInstance.type: the kind of music (Sdlmix_Classify), index of g_sdlmixVolumes. */
enum {
    SDLMIX_TYPE_NONE = 0,
    SDLMIX_TYPE_MOD = 1,        /* tracker files */
    SDLMIX_TYPE_MIDI = 2,
    SDLMIX_TYPE_WAV = 3,        /* refused by Sdlmix_Open */
    SDLMIX_TYPE_MP3 = 4,        /* refused */
    SDLMIX_TYPE_OGG = 5         /* refused */
};

/* The instance of the sdlmix plugin (only one at a time: g_sdlmixInstance). */
typedef struct SdlmixInstance {
    struct _Mix_Music *music;   /* 0x00 */
    int loop;                   /* 0x04 */
    int loop_start;             /* 0x08 kept, not used */
    int loop_end;               /* 0x0C */
    int type;                   /* 0x10 SDLMIX_TYPE_* */
    int pad_14;                 /* 0x14 never accessed by the game */
    double volume;              /* 0x18 percent */
    int muted;                  /* 0x20 */
    int finished;               /* 0x24 the music stopped by itself (Sdlmix_Poll) */
    int ended;                  /* 0x28 SDL_mixer's finished hook ran (the read event) */
    BgmEndFn end_callback;      /* 0x2C */
    void *end_arg;              /* 0x30 */
    int pad_34;                 /* 0x34 never accessed by the game */
} SdlmixInstance;
MUGEN_ASSERT_OFFSET(SdlmixInstance, volume, 0x18);
MUGEN_ASSERT_SIZE(SdlmixInstance, 0x38);

#endif /* MUGEN_BGM_H */
