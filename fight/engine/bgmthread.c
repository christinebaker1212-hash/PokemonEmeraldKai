/*
 * BGM thread (original module 0x489450-0x48AB5F): the command queue
 * (g_bgmQueue, under g_bgmMutex), the thread that runs the commands and
 * keeps the ring buffer of decoded blocks filled, the SDL_mixer music hook
 * that plays those blocks, and the settings lists given to plugins.
 *
 * Streams decoded by the engine (BGM_MODE_DECODED) are read block by block
 * from the stream loader (Bgm_StreamRead) into g_bgmRing; each block starts
 * with a sequence number, which the mixer callback publishes in
 * g_bgmPlayedSeq so that events of the decoder (end of the stream) take
 * effect when the mixer gets there. Plugins that play by themselves
 * (BGM_MODE_PLUGIN, sdlmix) are only started, paused and muted from here.
 *
 * The ring buffer and the playback counters are shared with the SDL audio
 * thread and changed under SDL_LockAudio, the ring's space and fill
 * counters with Interlocked operations, as in the original.
 */
#define MF_SYSTEM_HEAP  /* Moemon Fight: the music's memory is the music thread's, outside the fight's heap (mf_mem.h) */
#include <SDL.h>
#include <SDL_mixer.h>
#define HAVE_STRUCT_TIMESPEC 1      /* the CRT's time.h already has it */
#include <pthread.h>

#include "mugen.h"

#define BGM_LOCK()   pthread_mutex_lock((pthread_mutex_t *)&g_bgmMutex)
#define BGM_UNLOCK() pthread_mutex_unlock((pthread_mutex_t *)&g_bgmMutex)

/* Bgm_StreamRead statuses. */
enum {
    BGM_READ_ERROR = 0,
    BGM_READ_MORE = 1,
    BGM_READ_END = 2
};

/* Value of a silent sample byte in the BGM format. */
static int bgm_silence(void)
{
    return g_bgmThreadFormat.format == AUDIO_FMT_U8 ? 0x80 : 0;
}

/* Appends elem_size bytes from elem, doubling the capacity when full (inlined by the original). */
static int dynarray_push(DynArray *a, const void *elem)
{
    if (a->count >= a->capacity) {
        void *data = realloc(a->data, a->elem_size * a->capacity * 2);

        if (data == NULL) {
            return 0;
        }
        a->data = data;
        a->capacity *= 2;
    }
    memcpy((char *)a->data + a->count * a->elem_size, elem, a->elem_size);
    a->count++;
    return 1;
}

/*
 * 0x00489450: starts the BGM thread for the mixer format and chunk (in
 * frames), and waits until it runs. Returns 0, or an error code.
 *
 * Quirks kept: an unknown sample format counts chunk bytes per sample; when
 * the ring buffer cannot be created the start-up handshake leaks (and when
 * g_bgmScratch cannot be allocated, g_bgmRing is freed but not cleared); a
 * failed pthread_create would wait forever. The original passes an
 * uninitialised sched_param to attributes it never uses; here it is zeroed.
 */
int BgmThread_Init(const AudioFormat *format, int chunk)
{
    BgmThreadStart *start;
    pthread_attr_t attr;
    struct sched_param param;
    pthread_t thread;
    int sample_size;
    int err;

    g_bgmThreadFormat = *format;
    if (format->format & 0x04) {
        sample_size = 1;
    } else if (format->format & 0x08) {
        sample_size = 2;
    } else if (format->format & 0x10) {
        sample_size = 4;
    } else if (format->format & 0x20) {
        sample_size = 8;
    } else {
        sample_size = chunk;
    }
    g_bgmChunkBytes = format->channels * sample_size * chunk;
    err = pthread_mutex_init((pthread_mutex_t *)&g_bgmMutex, NULL);
    if (err != 0) {
        return err;
    }
    start = calloc(1, sizeof(BgmThreadStart));
    if (start == NULL) {
        g_outOfMemory = 1;
        return 1;
    }
    err = pthread_cond_init((pthread_cond_t *)&start->cond, NULL);
    if (err != 0) {
        free(start);
        return err;
    }
    err = pthread_mutex_init((pthread_mutex_t *)&start->mutex, NULL);
    if (err != 0) {
        free(start);
        return err;
    }
    g_bgmRing = BgmRing_Create(g_bgmChunkBytes + 4);
    if (g_bgmRing == NULL) {
        return 1;
    }
    g_bgmScratch = Mem_Calloc(BGM_RING_BLOCKS, g_bgmChunkBytes);
    if (g_bgmScratch == NULL) {
        free(g_bgmRing);
        return 1;
    }
    g_bgmStream = Bgm_StreamCreate();
    g_bgmThreadRunning = 1;
    pthread_attr_init(&attr);
    memset(&param, 0, sizeof(param));
    pthread_attr_setschedparam(&attr, &param);
    g_bgmRing->paused = 1;
    g_bgmRing->stopped = 1;
    pthread_attr_destroy(&attr);
    pthread_mutex_lock((pthread_mutex_t *)&start->mutex);
    err = pthread_create(&thread, NULL, BgmThread_Main, start);
    memcpy(&g_bgmThread, &thread, sizeof(thread));   /* Moemon Fight: pthread_t is the smaller one on 64-bit */
    pthread_cond_wait((pthread_cond_t *)&start->cond, (pthread_mutex_t *)&start->mutex);
    pthread_mutex_unlock((pthread_mutex_t *)&start->mutex);
    pthread_mutex_destroy((pthread_mutex_t *)&start->mutex);
    pthread_cond_destroy((pthread_cond_t *)&start->cond);
    free(start);
    return err;
}

/* Runs one queued command (the switch of BgmThread_Main). */
static void bgm_run_command(const BgmCommand *c, int *running)
{
    BgmStream *s;

    switch (c->cmd) {
    case BGM_CMD_LOAD:
        BgmThread_Load((const BgmInfo *)c->data[0]);
        break;
    case BGM_CMD_PLAY:
        BgmThread_Play((const BgmInfo *)c->data[0]);
        break;
    case BGM_CMD_UNLOAD:
        BgmThread_Unload();
        break;
    case BGM_CMD_START:
        BgmThread_Start();
        break;
    case BGM_CMD_STOP:
        BgmThread_Stop();
        break;
    case BGM_CMD_HOLD:
        BgmThread_Hold((unsigned int)c->data[0]);
        break;
    case BGM_CMD_PREBUFFER:
        BgmThread_Prebuffer();
        break;
    case BGM_CMD_UNHOLD:
        BgmThread_Unhold((unsigned int)c->data[0]);
        break;
    case BGM_CMD_PAUSE:
        g_bgmPauseFlags |= (unsigned int)c->data[0];
        BgmThread_Hold((unsigned int)c->data[0]);
        break;
    case BGM_CMD_RESUME:
        g_bgmPauseFlags &= ~(unsigned int)c->data[0];
        BgmThread_Unhold((unsigned int)c->data[0]);
        break;
    case BGM_CMD_SET_STREAM: {
        int muted;

        g_bgmStream = (BgmStream *)c->data[0];
        BgmThread_UpdateHook();
        muted = g_bgmMuted;
        s = g_bgmStream;
        if (s != NULL) {
            s->muted = muted;
            if (s->plugin != NULL && s->mode == BGM_MODE_PLUGIN && s->playing) {
                s->plugin->set_mute(s->instance, muted);
            }
        }
        break;
    }
    case BGM_CMD_VOLUME:
        Bgm_StreamSetVolume(*(const double *)c->data);
        break;
    case BGM_CMD_MUTE:
        s = g_bgmStream;
        if (s != NULL && s->playing && s->plugin != NULL) {
            g_bgmMuted = c->data[0];
            Bgm_StreamSetMute(g_bgmStream, g_bgmMuted);
        }
        break;
    case BGM_CMD_QUIT:
        *running = 0;
        break;
    default:
        MatchLog_Printf(3, "Invalid BGM command: %d\n", c->cmd);
        break;
    }
}

/*
 * 0x00489650: the BGM thread. Each round: sleep 16 ms when there is nothing
 * to do, handle the decoder events the mixer has reached, run the queued
 * commands that share the oldest time stamp, and fill the next ring block
 * when there is room. Ends with BGM_CMD_QUIT.
 */
void *BgmThread_Main(void *arg)
{
    BgmThreadStart *start = arg;
    int running = g_bgmThreadRunning;
    int more = 0;               /* commands were left in the queue */

    pthread_mutex_lock((pthread_mutex_t *)&start->mutex);
    pthread_cond_signal((pthread_cond_t *)&start->cond);
    pthread_mutex_unlock((pthread_mutex_t *)&start->mutex);
    while (running) {
        BgmRing *r = g_bgmRing;
        unsigned int fill;

        if (r->stopped) {
            fill = 0;
        } else {
            long space = r->space;

            fill = space < 0 ? 0 : (unsigned int)space;
            if (fill > (unsigned int)r->block) {
                fill = r->block;
            }
        }
        if (g_bgmBackgroundChanged == 0 && !more && fill == 0 && g_bgmEventCount == 0) {
            Sleep(16);
        }
        g_bgmBackgroundChanged = 0;

        if (g_bgmEventCount > 0) {
            int played = g_bgmPlayedSeq;
            int n = g_bgmEventCount;
            int i;

            for (i = 0; i < n && g_bgmEvents[i].seq <= played; i++) {
                if (g_bgmEvents[i].type == BGM_EVENT_END) {
                    BgmThread_Stop();
                    n = g_bgmEventCount;
                }
            }
            for (i = 0; i < n && g_bgmEvents[i].seq <= played; i++) {
            }
            n -= i;
            g_bgmEventCount = n;
            if (n != 0) {
                memmove(&g_bgmEvents[0], &g_bgmEvents[i], n * sizeof(BgmEvent));
            }
        }

        BGM_LOCK();
        if (g_bgmQueueCount > 0) {
            int time = g_bgmQueue[0].time;
            int i;

            for (i = 0; i < g_bgmQueueCount && g_bgmQueue[i].time == time; i++) {
                bgm_run_command(&g_bgmQueue[i], &running);
            }
            BgmQueue_Drop(time);
            more = g_bgmQueueCount > 0;
        }
        BGM_UNLOCK();
        if (!running) {
            break;
        }

        r = g_bgmRing;
        if (r->stopped == 0 && r->space >= r->block) {
            unsigned int pos = r->write;
            unsigned int block = (unsigned int)r->block;
            unsigned int size = (pos / block + 1) * block - pos;

            r->write = (pos + size) % r->size;
            InterlockedExchangeAdd(&r->space, -r->block);
            if (size != 0) {
                BgmThread_FillBlock(r->buf + pos, size);
            }
        }
    }
    BgmThread_Cleanup();
    return NULL;
}

/*
 * 0x00489A40: end of the BGM thread: unhooks the mixer callback, drops the
 * queued commands and frees the ring buffer and the stream. Quirk: the
 * stream is freed but g_bgmStream is not cleared.
 */
void BgmThread_Cleanup(void)
{
    BgmRing *r;

    g_bgmThreadRunning = 0;
    if (g_bgmHooked) {
        MatchLog_Printf(7, "Cleaning up mixer callback...");
        MatchLog_Printf(7, "unhooking mixer callback...\n");
        g_bgmHooked = 0;
        Mix_HookMusic(NULL, NULL);
    }
    g_bgmHooked = 0;
    memset(&g_bgmThreadFormat, 0, sizeof(g_bgmThreadFormat));
    BgmQueue_Drop(0x7FFFFFFF);
    memset(g_bgmQueue, 0, sizeof(g_bgmQueue));
    pthread_mutex_destroy((pthread_mutex_t *)&g_bgmMutex);
    memset(&g_bgmMutex, 0, sizeof(g_bgmMutex));     /* Moemon Fight: the real type */
    g_bgmEventCount = 0;
    memset(g_bgmEvents, 0, sizeof(g_bgmEvents));
    r = g_bgmRing;
    free(r->buf);
    free(r);
    g_bgmRing = NULL;
    free(g_bgmScratch);
    g_bgmScratch = NULL;
    Bgm_StreamFree();
}

/*
 * 0x00489B10: queues command cmd stamped with time (call with g_bgmMutex
 * held). data (two words) may be NULL. Returns 1 when the queue is full.
 */
int Bgm_QueueCommand(int time, const intptr_t *data, int cmd, int owns_data)
{
    BgmCommand *c;

    if (MfHost_Replaying()) {
        /* Moemon Fight: a frame run again after going back (rollback): the music already did what it asks */
        if (owns_data && data != NULL && data[0] != 0) {
            free((void *)data[0]);
        }
        return 0;
    }
    if (g_bgmQueueCount == BGM_QUEUE_SIZE) {
        MatchLog_Printf(3, "BGM command queue capacity exceeded at time %ld", time);
        return 1;
    }
    c = &g_bgmQueue[g_bgmQueueCount];
    c->cmd = cmd;
    c->time = time;
    if (data != NULL) {
        c->data[0] = data[0];
        c->data[1] = data[1];
    }
    c->owns_data = owns_data;
    g_bgmQueueCount++;
    return 0;
}

/* 0x00489B80: removes the leading commands stamped up to time, freeing their owned data. */
void BgmQueue_Drop(int time)
{
    int n = g_bgmQueueCount;
    int i;

    for (i = 0; i < n && g_bgmQueue[i].time <= time; i++) {
        if (g_bgmQueue[i].owns_data && g_bgmQueue[i].data[0] != 0) {
            free((void *)g_bgmQueue[i].data[0]);
        }
    }
    memcpy(&g_bgmQueue[0], &g_bgmQueue[i], (n - i) * sizeof(BgmCommand));
    g_bgmQueueCount = n - i;
}

/* Queues cmd with a copy of info (BgmQueue_Load / BgmQueue_Play). */
static int bgm_queue_info(int time, const BgmInfo *info, int cmd)
{
    BgmInfo *copy = malloc(sizeof(BgmInfo));
    intptr_t data[2];
    int err;

    if (copy == NULL) {
        return 1;
    }
    memcpy(copy, info, sizeof(BgmInfo));
    data[0] = (intptr_t)copy;
    err = Bgm_QueueCommand(time, data, cmd, 1);
    if (err != 0) {
        free(copy);
    }
    return err;
}

/* 0x00489BF0: queues BGM_CMD_LOAD with a copy of info. */
int BgmQueue_Load(int time, const BgmInfo *info)
{
    return bgm_queue_info(time, info, BGM_CMD_LOAD);
}

/* 0x00489C50: queues BGM_CMD_PLAY with a copy of info. */
int BgmQueue_Play(int time, const BgmInfo *info)
{
    return bgm_queue_info(time, info, BGM_CMD_PLAY);
}

/* 0x00489CC0: queues BGM_CMD_UNLOAD (Bgm_QueueCommand inlined, the data words are left as they were). */
int BgmQueue_Unload(int time)
{
    BgmCommand *c;

    if (g_bgmQueueCount == BGM_QUEUE_SIZE) {
        MatchLog_Printf(3, "BGM command queue capacity exceeded at time %ld", time);
        return 1;
    }
    c = &g_bgmQueue[g_bgmQueueCount];
    c->cmd = BGM_CMD_UNLOAD;
    c->time = time;
    c->owns_data = 0;
    g_bgmQueueCount++;
    return 0;
}

/*
 * 0x00489D10: drops the queued commands, queues BGM_CMD_QUIT and waits for
 * the BGM thread to end. Returns pthread_join's result.
 */
int BgmThread_Shutdown(void)
{
    void *result = NULL;
    pthread_t thread;
    BgmCommand *c;

    BGM_LOCK();
    BgmQueue_Drop(0x7FFFFFFF);
    if (g_bgmQueueCount == BGM_QUEUE_SIZE) {
        MatchLog_Printf(3, "BGM command queue capacity exceeded at time %ld", -1);
    } else {
        c = &g_bgmQueue[g_bgmQueueCount];
        c->cmd = BGM_CMD_QUIT;
        c->time = -1;
        c->owns_data = 0;
        g_bgmQueueCount++;
    }
    BGM_UNLOCK();
    g_bgmBackgroundChanged = 1;
    memcpy(&thread, &g_bgmThread, sizeof(thread));
    return pthread_join(thread, &result);
}

/* 0x00489DC0: queues BGM_CMD_MUTE (the second data word is uninitialised, as in the original). */
int Bgm_QueueMute(int muted, int time)
{
    intptr_t data[2];

    data[0] = muted;
    return Bgm_QueueCommand(time, data, BGM_CMD_MUTE, 0);
}

/* Stops feeding and playing the ring buffer and empties it (inlined by the original). */
static void bgm_reset_playback(void)
{
    SDL_LockAudio();
    g_bgmRing->stopped = 1;
    g_bgmRing->paused = 1;
    SDL_UnlockAudio();
    BgmRing_Reset();
    if (g_bgmHooked) {
        SDL_LockAudio();
        g_bgmFillSeq = 0;
        g_bgmPlayedSeq = 0;
        SDL_UnlockAudio();
    } else {
        g_bgmFillSeq = 0;
        g_bgmPlayedSeq = 0;
    }
    g_bgmEventCount = 0;
}

/*
 * 0x00489DF0: BGM_CMD_LOAD: stops the current BGM and opens info's (an
 * empty path only resets playback). Returns 0 on success, 1 when the stream
 * loader failed (logged as "error 1!") or there is no stream.
 */
int BgmThread_Load(const BgmInfo *info)
{
    int failed;

    if (g_bgmStream == NULL) {
        return 1;
    }
    BgmThread_Stop();
    if (info->path[0] != '\0') {
        MatchLog_Printf(7, "attempting load of %s...", info->path);
    } else {
        MatchLog_Printf(7, "loading empty BGM...\n");
        bgm_reset_playback();
    }
    if (info->path[0] == '\0') {
        return 0;
    }
    failed = Bgm_StreamOpen(info) == 0;
    if (failed) {
        MatchLog_Printf(3, "error %d!\n", failed);
        return failed;
    }
    BgmThread_UpdateHook();
    Bgm_StreamSetMute(g_bgmStream, g_bgmMuted);
    MatchLog_Printf(7, "success!\n");
    return 0;
}

/*
 * 0x00489F00: BGM_CMD_PLAY: nothing when info's BGM is already playing,
 * unloads for an empty path, else loads and starts it. When that fails,
 * playback is reset.
 */
int BgmThread_Play(const BgmInfo *info)
{
    int err;

    if (g_bgmStream == NULL) {
        return 1;
    }
    MatchLog_Printf(7, "attempting load and start...");
    switch (Bgm_StreamCompare(info)) {
    case 0:
        MatchLog_Printf(7, "nothing to do!\n");
        return 0;
    case 1:
        err = BgmThread_Unload();
        MatchLog_Printf(7, "no BGM...");
        if (err != 0) {
            MatchLog_Printf(3, "error %d\n", err);
        }
        return err;
    case 2:
        err = BgmThread_Load(info);
        if (err == 0) {
            err = BgmThread_Start();
            if (err == 0 && g_bgmStream->playing) {
                g_bgmRing->stopped = 0;
                BgmThread_UpdateHook();
                Bgm_StreamSetMute(g_bgmStream, g_bgmMuted);
                return 0;
            }
        }
        break;
    default:
        MatchLog_Printf(3, "shouldn't be here!\n");
        err = 1;
        break;
    }
    bgm_reset_playback();
    return err;
}

/* 0x0048A070: BGM_CMD_UNLOAD: stops and closes the loaded BGM. Always 0. */
int BgmThread_Unload(void)
{
    BgmStream *s = g_bgmStream;

    if (s != NULL && s->plugin != NULL && s->info.path[0] != '\0') {
        BgmThread_Stop();
        Bgm_StreamClose(g_bgmStream);
    }
    return 0;
}

/*
 * 0x0048A0A0: BGM_CMD_START: starts the loaded stream (paused if pause
 * flags are kept). Always 0. Unlike prebuffering, no path is required.
 */
int BgmThread_Start(void)
{
    BgmStream *s = g_bgmStream;

    if (s != NULL && s->plugin != NULL && !s->playing) {
        MatchLog_Printf(7, "starting BGM...\n");
        Bgm_StreamSetMute(g_bgmStream, g_bgmMuted);
        Bgm_StreamStart(g_bgmStream, g_bgmPauseFlags);
        g_bgmRing->stopped = 0;
        if (g_bgmStream->flags == 0) {
            g_bgmRing->paused = 0;
        }
        BgmThread_UpdateHook();
    }
    return 0;
}

/* 0x0048A130: BGM_CMD_PREBUFFER: starts the loaded stream held, so that the ring buffer fills. Always 0. */
int BgmThread_Prebuffer(void)
{
    BgmStream *s = g_bgmStream;

    if (s != NULL && s->plugin != NULL && !s->playing && s->info.path[0] != '\0') {
        MatchLog_Printf(7, "prebuffering BGM...\n");
        Bgm_StreamSetMute(g_bgmStream, g_bgmMuted);
        if (Bgm_StreamStart(g_bgmStream, g_bgmPauseFlags | BGM_STREAM_HELD) == 0) {
            g_bgmRing->stopped = 0;
        }
        BgmThread_UpdateHook();
    }
    return 0;
}

/* 0x0048A1B0: BGM_CMD_STOP: stops the stream and resets playback (also used before loads). Always 0. */
int BgmThread_Stop(void)
{
    if (g_bgmStream == NULL) {
        return 0;
    }
    Bgm_StreamStop();
    bgm_reset_playback();
    return 0;
}

/*
 * 0x0048A260: adds pause flags (and the kept ones) to the playing stream;
 * the first one pauses it. Returns the plugin's pause result, else 0.
 * Quirk: g_bgmStream is not checked for NULL.
 */
int BgmThread_Hold(unsigned int flags)
{
    BgmStream *s = g_bgmStream;
    unsigned int old = s->flags;

    if (!s->playing || s->plugin == NULL) {
        return 0;
    }
    s = g_bgmStream;
    s->flags = g_bgmPauseFlags | flags | s->flags;
    if (old != 0) {
        return 0;
    }
    if (s->mode != BGM_MODE_PLUGIN) {
        g_bgmRing->paused = 1;
        return 0;
    }
    return s->plugin->pause(s->instance);
}

/*
 * 0x0048A2E0: removes pause flags from the playing stream; removing the
 * last one resumes it. Returns the plugin's resume result, else 0.
 */
int BgmThread_Unhold(unsigned int flags)
{
    BgmStream *s = g_bgmStream;
    unsigned int old = s->flags;

    if (!s->playing || s->plugin == NULL) {
        return 0;
    }
    s->flags = ~flags & old;
    if (old == 0 || s->flags != 0) {
        return 0;
    }
    if (s->mode != BGM_MODE_PLUGIN) {
        g_bgmRing->paused = 0;
        return 0;
    }
    return s->plugin->resume(s->instance);
}

/* 0x0048A350: empties the ring buffer. */
void BgmRing_Reset(void)
{
    BgmRing *r;

    SDL_LockAudio();
    r = g_bgmRing;
    r->write = 0;
    r->read = 0;
    memset(r->buf, 0, r->size);
    r->filled = 0;
    r->space = r->size;
    SDL_UnlockAudio();
}

/* 0x0048A390: fills size bytes with silence. */
void BgmThread_Silence(void *dst, unsigned int size)
{
    memset(dst, bgm_silence(), size);
}

/*
 * 0x0048A3C0: fills the ring block at dst (size bytes): the sequence
 * number, then the stream's next samples (silence without a stream). At the
 * end of the stream an end event is posted and filling stops. Always 0.
 * Quirks kept: streams the plugin plays itself leave the block untouched
 * apart from the sequence number; silence blocks do not advance it.
 */
int BgmThread_FillBlock(void *dst, unsigned int size)
{
    unsigned char *out = (unsigned char *)dst + 4;
    unsigned int bytes = size - 4;
    BgmStream *s;
    int status = BGM_READ_MORE;
    int ended = 1;
    unsigned int got;

    *(int *)dst = g_bgmFillSeq;
    s = g_bgmStream;
    if (s == NULL || !s->playing || s->plugin == NULL) {
        memset(out, bgm_silence(), bytes);
        InterlockedExchangeAdd(&g_bgmRing->filled, bytes);
        return 0;
    }
    if (s->mode != BGM_MODE_DECODED) {
        return 0;
    }
    got = Bgm_StreamRead(out, &status);
    if (status == BGM_READ_END || status == BGM_READ_ERROR) {
        BgmThread_PostEnd();
    } else {
        ended = 0;
    }
    if (got < bytes) {
        BgmThread_Silence(out + got, bytes - got);
        if (!ended) {
            MatchLog_Printf(3, "unexpected decoder underrun!\n");
        }
    }
    g_bgmFillSeq++;
    InterlockedExchangeAdd(&g_bgmRing->filled, bytes);
    if (ended) {
        SDL_LockAudio();
        g_bgmRing->stopped = 1;
        g_bgmRing->paused = 0;
        SDL_UnlockAudio();
    }
    return 0;
}

/* 0x0048A4E0: posts the end of the stream at the block being filled. */
void BgmThread_PostEnd(void)
{
    BgmEvent *e;

    if (g_bgmEventCount == BGM_EVENT_QUEUE_SIZE) {
        MatchLog_Printf(3, "BGM event queue full!\n");
        return;
    }
    e = &g_bgmEvents[g_bgmEventCount];
    e->type = BGM_EVENT_END;
    e->seq = g_bgmFillSeq;
    e->stream = g_bgmStream;
    g_bgmEventCount++;
}

/* 0x0048A530: removes the mixer callback. */
void BgmThread_Unhook(void)
{
    MatchLog_Printf(7, "unhooking mixer callback...\n");
    g_bgmHooked = 0;
    Mix_HookMusic(NULL, NULL);
}

/* 0x0048A560: hooks the mixer callback for decoded streams and removes it for the others. */
void BgmThread_UpdateHook(void)
{
    BgmStream *s = g_bgmStream;

    if (s == NULL || s->plugin == NULL || s->info.path[0] == '\0') {
        return;
    }
    if (s->mode != BGM_MODE_PLUGIN) {
        if (!g_bgmHooked) {
            MatchLog_Printf(7, "hooking mixer callback...\n");
            g_bgmHooked = 1;
            Mix_HookMusic(BgmThread_MixCallback, NULL);
        }
        return;
    }
    if (g_bgmHooked) {
        BgmThread_Unhook();
    }
}

/*
 * 0x0048A5C0: SDL_mixer music hook (SDL audio thread): plays the next ring
 * block, or silence while paused, muted or starved.
 *
 * Quirks kept: muted, a block's space is given back even when none was
 * played; while paused the length is used as the source pointer of a
 * 0-byte copy; the length check is skipped while muted.
 */
void BgmThread_MixCallback(void *udata, Uint8 *stream, int len)
{
    BgmRing *r = g_bgmRing;
    int was_paused = r->paused;
    const unsigned char *src;
    unsigned int avail;
    unsigned int n;

    if (was_paused) {
        src = (const unsigned char *)(intptr_t)len;
        avail = 0;
    } else if (r->filled >= r->block) {
        src = r->buf + r->read;
        r->read += r->block;
        if (r->read >= r->size) {
            r->read = 0;
        }
        avail = r->block;
    } else {
        src = NULL;
        avail = 0;
    }
    if (avail < (unsigned int)(len + 4) && !was_paused && g_bgmPlayedSeq != 0) {
        MatchLog_Printf(5, "possible read starvation detected (%ld/%ld)!\n", avail, len + 4);
    }
    if (avail >= 4) {
        g_bgmPlayedSeq = *(const int *)src;
        src += 4;
    }
    if (g_bgmMuted) {
        if (!g_bgmRing->paused) {
            InterlockedExchangeAdd(&g_bgmRing->space, g_bgmRing->block);
        }
        memset(stream, bgm_silence(), len);
        return;
    }
    if (src != NULL) {
        n = avail >= 4 ? avail - 4 : 0;
        if (n > (unsigned int)len) {
            n = len;
        }
        memcpy(stream, src, n);
        if (!g_bgmRing->paused) {
            InterlockedExchangeAdd(&g_bgmRing->space, g_bgmRing->block);
        }
    } else {
        n = 0;
    }
    if (n < (unsigned int)len) {
        memset(stream + n, bgm_silence(), len - n);
    }
    if ((unsigned int)len != g_bgmChunkBytes) {
        MatchLog_Printf(3, "unexpected buffer length: %d instead of %u\n", len, g_bgmChunkBytes);
    }
}

/*
 * 0x0048A730: empty plugin settings. NULL when out of memory. Quirk: some
 * partial allocations leak on failure.
 */
BgmSettings *BgmSettings_New(void)
{
    DynArray *keys = calloc(1, sizeof(DynArray));
    DynArray *values;
    BgmSettings *s;
    void *data;

    if (keys == NULL) {
        g_outOfMemory = 1;
    }
    data = calloc(BGM_ARRAY_CAPACITY, sizeof(char *));
    if (data == NULL) {
        g_outOfMemory = 1;
    }
    if (keys == NULL || data == NULL) {
        return NULL;
    }
    keys->data = data;
    keys->capacity = BGM_ARRAY_CAPACITY;
    keys->elem_size = sizeof(char *);
    values = calloc(1, sizeof(DynArray));
    if (values == NULL) {
        g_outOfMemory = 1;
    }
    data = calloc(BGM_ARRAY_CAPACITY, sizeof(char *));
    if (data == NULL) {
        g_outOfMemory = 1;
    }
    if (values == NULL || data == NULL) {
        free(keys);
        return NULL;
    }
    values->data = data;
    values->capacity = BGM_ARRAY_CAPACITY;
    values->elem_size = sizeof(char *);
    s = calloc(1, sizeof(BgmSettings));
    if (s == NULL) {
        g_outOfMemory = 1;
        free(keys->data);
        free(keys);
        free(values->data);
        free(values);
        return NULL;
    }
    s->keys = keys;
    s->values = values;
    return s;
}

/* 0x0048A820: plugin callback; the value of key (case-sensitive), or NULL. */
const char *BgmSettings_Get(const BgmSettings *s, const char *key)
{
    const DynArray *keys = s->keys;
    int count = keys->count;
    int i;

    for (i = 0; i < count; i++) {
        if (strcmp(((char **)keys->data)[i], key) == 0) {
            return ((char **)s->values->data)[i];
        }
    }
    return NULL;
}

/*
 * 0x0048A890: sets key to a copy of value. 0 when out of memory. Quirk: the
 * value replaced is leaked.
 */
int BgmSettings_Set(BgmSettings *s, const char *key, const char *value)
{
    DynArray *keys = s->keys;
    int count = keys->count;
    char *k, *v;
    int i;

    for (i = 0; i < count; i++) {
        if (strcmp(((char **)keys->data)[i], key) == 0) {
            DynArray *values;

            v = calloc(1, strlen(value) + 1);
            if (v == NULL) {
                g_outOfMemory = 1;
                return 0;
            }
            strcpy(v, value);
            values = s->values;
            if (i >= 0 && i < values->count) {
                memcpy((char *)values->data + values->elem_size * i, &v, values->elem_size);
            }
            return 1;
        }
    }
    k = calloc(1, strlen(key) + 1);
    if (k == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    v = calloc(1, strlen(value) + 1);
    if (v == NULL) {
        g_outOfMemory = 1;
        free(k);
        return 0;
    }
    strcpy(k, key);
    strcpy(v, value);
    if (!dynarray_push(s->keys, &k)) {
        free(k);
        free(v);
        return 0;
    }
    if (!dynarray_push(s->values, &v)) {
        /* Take the key back out. */
        DynArray *a = s->keys;
        int last = a->count - 1;

        if (last >= 0 && last < a->count) {
            char *at = (char *)a->data + a->elem_size * last;
            memmove(at, at + a->elem_size, (a->count - last - 1) * a->elem_size);
            a->count--;
        }
        free(k);
        free(v);
        return 0;
    }
    return 1;
}

/* 0x0048AB10: frees the keys and values of plugin settings and empties them. */
void BgmSettings_Clear(BgmSettings *s)
{
    int i;

    for (i = 0; i < s->keys->count; i++) {
        free(((char **)s->keys->data)[i]);
        free(((char **)s->values->data)[i]);
    }
    s->keys->count = 0;
    s->values->count = 0;
}
