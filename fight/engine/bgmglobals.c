/* Moemon Fight: the music's globals, taken out of globals.c. The BGM thread shares them, so they stay out of
 * the fight's saved state (fight/host/mf_rollback.c snapshots every other engine global). */

#define MF_SYSTEM_HEAP  /* Moemon Fight: the music's memory is the music thread's, outside the fight's heap (mf_mem.h) */
#include "mugen.h"

BgmSubsystem g_bgmSdlmixSubsystem = { Bgm_BuiltinVersion, Sdlmix_Extensions, Sdlmix_Init, Stub_ReturnZero, Sdlmix_Create, Sdlmix_Destroy, Sdlmix_IsPlaying, Sdlmix_Start, Sdlmix_Stop, Sdlmix_Rewind, Sdlmix_Seek, Sdlmix_Open, Sdlmix_Close, Sdlmix_GetVolume, Sdlmix_SetVolume, Sdlmix_GetLoopFlag, Sdlmix_SetLoopFlag, Sdlmix_GetLoopPoints, Sdlmix_SetLoopPoints, Sdlmix_SetEndCallback, Sdlmix_StrError, Sdlmix_Read, NULL, Sdlmix_Poll, Sdlmix_IsPaused, Sdlmix_Pause, Sdlmix_Resume, Sdlmix_SetMute, Sdlmix_GetMute, NULL }; /* 0x004f17a0 */
BgmSubsystem g_bgmMpg123Subsystem = { Bgm_BuiltinVersion, Mpg123_Extensions, Mpg123_Init, Mpg123_Deinit, Mpg123_Create, Mpg123_Destroy, Mpg123_IsPlaying, Mpg123_Start, Mpg123_Stop, Mpg123_Rewind, Mpg123_Seek, Mpg123_Open, Mpg123_Close, Mpg123_GetVolume, Mpg123_SetVolume, Mpg123_GetLoopFlag, Mpg123_SetLoopFlag, Mpg123_GetLoopPoints, Mpg123_SetLoopPoints, Mpg123_SetEndCallback, Mpg123_StrError, Mpg123_Read, Mpg123_GetFormatOf, NULL, NULL, NULL, NULL, NULL, NULL, NULL }; /* 0x004f1818 */
const char * g_mpg123Errors[7] = { "OK", "Out of memory", "mpg123 internal error", "bad output format request", "file not loaded or plugin not initiated", "I/O error", "libmpg123 library not found" }; /* 0x00500070 */
BgmHandleFn g_bgmGetHandle; /* 0x0050387c */
int g_bgmInitialized; /* 0x00503890 */
int g_bgmActive; /* 0x00503894 */
double g_bgmVolume; /* 0x00503898 */
AudioFormat g_bgmFormat; /* 0x005038a0 */
int g_bgmChunk; /* 0x005038b0 */
int g_bgmBackgroundMode; /* 0x005038b4 */
Resampler * g_bgmResampler; /* 0x005038b8 */
DynArray * g_bgmSubsystems; /* 0x005038bc */
DynArray * g_bgmExtensions; /* 0x005038c0 */
int g_bgmQueueCount; /* 0x00503cf4 */
unsigned int g_bgmChunkBytes; /* 0x00503cf8 */
int g_bgmPlayedSeq; /* 0x00503cfc */
BgmCommand g_bgmQueue[BGM_QUEUE_SIZE]; /* 0x00503d00 */
pthread_mutex_t g_bgmMutex; /* 0x00503e80 (Moemon Fight: the real type; a pointer in pthreads-win32) */
int g_bgmMuted; /* 0x00503e84 */
BgmThreadId g_bgmThread; /* 0x00503e88 */
int g_bgmThreadRunning; /* 0x00503e90 */
BgmStream * g_bgmStream; /* 0x00503e94 */
AudioFormat g_bgmThreadFormat; /* 0x00503e98 */
void * g_bgmScratch; /* 0x00503ea8 */
int g_bgmFillSeq; /* 0x00503eac */
int g_bgmBackgroundChanged; /* 0x00503eb0 */
BgmRing * g_bgmRing; /* 0x00503eb4 */
unsigned int g_bgmPauseFlags; /* 0x00503eb8 */
int g_bgmEventCount; /* 0x00503ebc */
BgmEvent g_bgmEvents[BGM_EVENT_QUEUE_SIZE]; /* 0x00503ec0 */
int g_bgmHooked; /* 0x00503f80 */
Mpg123InfoFn g_mpg123_info; /* 0x00503f84 */
Mpg123HandleIntFn g_mpg123_open_feed; /* 0x00503f88 */
void * g_mpg123_current_decoder; /* 0x00503f8c */
int g_mpg123Initialized; /* 0x00503f90 */
void * g_mpg123_getvolume; /* 0x00503f94 */
Mpg123FormatFn g_mpg123_format; /* 0x00503f98 */
Mpg123HandleIntFn g_mpg123_format_none; /* 0x00503f9c */
Mpg123ParamFn g_mpg123_param; /* 0x00503fa0 */
HMODULE g_mpg123Lib; /* 0x00503fa4 */
Mpg123ReadFn g_mpg123_read; /* 0x00503fa8 */
Mpg123VolumeFn g_mpg123_volume; /* 0x00503fac */
Mpg123NewFn g_mpg123_new; /* 0x00503fb0 */
int g_mpg123Ntom; /* 0x00503fb4 */
Mpg123PlainStrErrorFn g_mpg123_plain_strerror; /* 0x00503fb8 */
Mpg123HandleFn g_mpg123_delete; /* 0x00503fbc */
void * g_mpg123_getparam; /* 0x00503fc0 */
Mpg123FeedSeekFn g_mpg123_feedseek; /* 0x00503fc4 */
unsigned int g_mpg123ReadBufferSize; /* 0x00503fc8 */
Mpg123HandleIntFn g_mpg123_close; /* 0x00503fcc */
double g_mpg123Volume; /* 0x00503fd0 */
Mpg123InitFn g_mpg123_init; /* 0x00503fd8 */
Mpg123FeedFn g_mpg123_feed; /* 0x00503fdc */
Mpg123GetFormatFn g_mpg123_getformat; /* 0x00503fe0 */
Mpg123StrErrorFn g_mpg123_strerror; /* 0x00503fe4 */
AudioFormat g_mpg123Format; /* 0x00503fe8 */
Mpg123ExitFn g_mpg123_exit; /* 0x00503ff8 */
void * g_mpg123_getstate; /* 0x00503ffc */
char g_mpg123Decoder[0x40]; /* 0x00504000 */
int g_mpg123Rva; /* 0x00504b04 */
