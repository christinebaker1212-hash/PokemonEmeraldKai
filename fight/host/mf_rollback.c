/*
 * Moemon Fight: saving and restoring the fight, frame by frame, for rollback (docs/FIGHT_DECISIONS.md).
 *
 * The fight's whole state is the heap region (fight/host/mf_mem.c: the engine's heap and stack) and the engine's
 * globals, which the build gathers into one section (the Makefile renames the engine objects' data sections:
 * mfstate on Linux, .data$mfB between the markers below on Windows). After every frame, the pages of the region
 * that frame changed are found (Windows: GetWriteWatch; elsewhere: compared with a copy) and their contents from
 * before the frame are kept, with a copy of the globals and the host's own values. Going back k frames writes
 * those pages back, newest first. MF_RB_FRAMES frames are kept.
 */
#define MF_SYSTEM_HEAP
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "mf_mem_internal.h"
#include "mf_rollback.h"

#define PAGE 4096

#ifdef _WIN32
__attribute__((section(".data$mfA"))) unsigned char mf_state_start[16] = { 1 };
__attribute__((section(".data$mfC"))) unsigned char mf_state_end[16] = { 1 };
#define STATE_START mf_state_start
#define STATE_END mf_state_end
#else
extern unsigned char __start_mfstate[], __stop_mfstate[];
#define STATE_START __start_mfstate
#define STATE_END __stop_mfstate
#endif

typedef struct RbFrame {
    int valid;
    unsigned int frame;
    unsigned char *globals;         /* the engine's globals after the frame */
    unsigned char host[MF_RB_HOST_SIZE];
    size_t count, capacity;         /* pages the frame changed */
    uint32_t *pages;
    unsigned char *old;             /* their contents before the frame */
} RbFrame;

static int sOn;
static RbFrame sRing[MF_RB_FRAMES];
static unsigned int sLast;          /* the newest frame kept */
static unsigned int sFirst;         /* the oldest frame that can be gone back to */
static unsigned char *sShadow;      /* the region as of the newest frame */
static size_t sShadowSize;          /* bytes of it kept up to date */
#ifdef _WIN32
static size_t sShadowCommitted;
#endif
static uint32_t *sDirty;            /* pages a frame changed */
static size_t sDirtyCap;
static MfRbStats sStats;

static size_t Rb_GlobalsSize(void)
{
    return (size_t)(STATE_END - STATE_START);
}

/* Copies the globals' section whole: with AddressSanitizer (tests) it holds the guard zones between the
 * globals too, so the copy mustn't be checked */
#if defined(__SANITIZE_ADDRESS__)
__attribute__((no_sanitize_address))
#endif
static void Rb_CopyGlobals(unsigned char *dst, const unsigned char *src)
{
    size_t n = Rb_GlobalsSize(), i;

    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

static int Rb_Grow(void **p, size_t *cap, size_t need, size_t elem)
{
    size_t n = *cap ? *cap : 64;
    void *q;

    if (need <= *cap) {
        return 1;
    }
    while (n < need) {
        n *= 2;
    }
    q = realloc(*p, n * elem);
    if (q == NULL) {
        return 0;
    }
    *p = q;
    *cap = n;
    return 1;
}

/* The shadow copy reaches `used` bytes of the region. At the start it copies them; later, pages newly reached
 * were never written before (MfMem_Used), so they were zero, as the shadow is */
static int Rb_ShadowReach(size_t used, int copy)
{
    unsigned char *base = MfMem_Base();

    used = (used + PAGE - 1) & ~(size_t)(PAGE - 1);
    if (used > MfMem_Size()) {
        used = MfMem_Size();
    }
    if (sShadow == NULL) {
#ifdef _WIN32
        sShadow = VirtualAlloc(NULL, MfMem_Size(), MEM_RESERVE, PAGE_READWRITE);
#else
        sShadow = calloc(1, MfMem_Size());  /* untouched pages cost nothing */
#endif
        if (sShadow == NULL) {
            return 0;
        }
    }
#ifdef _WIN32
    if (used > sShadowCommitted) {
        /* committed as it's reached, 16 MB at a time */
        size_t want = (used + ((size_t)16 << 20) - 1) & ~(((size_t)16 << 20) - 1);
        if (want > MfMem_Size()) {
            want = MfMem_Size();
        }
        if (VirtualAlloc(sShadow + sShadowCommitted, want - sShadowCommitted, MEM_COMMIT, PAGE_READWRITE) == NULL) {
            return 0;
        }
        sShadowCommitted = want;
    }
#endif
    if (copy) {
        memcpy(sShadow, base, used);
        if (sShadowSize > used) {
            memset(sShadow + used, 0, sShadowSize - used);
        }
        sShadowSize = used;
    } else if (used > sShadowSize) {
        sShadowSize = used;
    }
    return 1;
}

/* Finds the pages changed since the last call (in sDirty), up to the shadow's reach */
static size_t Rb_FindDirty(void)
{
    unsigned char *base = MfMem_Base();
    size_t pages = sShadowSize / PAGE, n = 0, i;

#ifdef _WIN32
    {
        ULONG_PTR count = pages;
        DWORD granularity;
        if (!Rb_Grow((void **)&sDirty, &sDirtyCap, pages + 1, sizeof(uint32_t))) {
            return (size_t)-1;
        }
        {
            PVOID *addresses = malloc(pages * sizeof(PVOID));
            if (addresses == NULL) {
                return (size_t)-1;
            }
            if (GetWriteWatch(WRITE_WATCH_FLAG_RESET, base, pages * PAGE, addresses, &count, &granularity) != 0) {
                free(addresses);
                return (size_t)-1;
            }
            for (i = 0; i < count; i++) {
                sDirty[n++] = (uint32_t)(((unsigned char *)addresses[i] - base) / PAGE);
            }
            free(addresses);
        }
    }
#else
    if (!Rb_Grow((void **)&sDirty, &sDirtyCap, pages + 1, sizeof(uint32_t))) {
        return (size_t)-1;
    }
    for (i = 0; i < pages; i++) {
        const uint64_t *a = (const uint64_t *)(base + i * PAGE), *b = (const uint64_t *)(sShadow + i * PAGE);
        size_t k;
        for (k = 0; k < PAGE / 8; k += 8) {
            if ((a[k] ^ b[k]) | (a[k + 1] ^ b[k + 1]) | (a[k + 2] ^ b[k + 2]) | (a[k + 3] ^ b[k + 3]) |
                (a[k + 4] ^ b[k + 4]) | (a[k + 5] ^ b[k + 5]) | (a[k + 6] ^ b[k + 6]) | (a[k + 7] ^ b[k + 7])) {
                sDirty[n++] = (uint32_t)i;
                break;
            }
        }
    }
#endif
    return n;
}

static void Rb_Clear(void)
{
    int i;

    for (i = 0; i < MF_RB_FRAMES; i++) {
        sRing[i].valid = 0;
        sRing[i].count = 0;
    }
}

int MfRollback_Begin(unsigned int frame, const void *host)
{
    size_t gsize = Rb_GlobalsSize();
    int i;

    Rb_Clear();
    sOn = 0;
    if (MfMem_Base() == NULL) {
        return 0;
    }
    for (i = 0; i < MF_RB_FRAMES; i++) {
        if (sRing[i].globals == NULL) {
            sRing[i].globals = malloc(gsize);
            if (sRing[i].globals == NULL) {
                return 0;
            }
        }
    }
    /* the shadow starts as the region is now */
    if (!Rb_ShadowReach(MfMem_Used(), 1)) {
        return 0;
    }
#ifdef _WIN32
    ResetWriteWatch(MfMem_Base(), MfMem_Size());
#endif
    sOn = 1;
    sFirst = sLast = frame;
    sRing[frame % MF_RB_FRAMES].valid = 1;
    sRing[frame % MF_RB_FRAMES].frame = frame;
    Rb_CopyGlobals(sRing[frame % MF_RB_FRAMES].globals, STATE_START);
    memcpy(sRing[frame % MF_RB_FRAMES].host, host, MF_RB_HOST_SIZE);
    return 1;
}

void MfRollback_End(void)
{
    sOn = 0;
    Rb_Clear();
}

int MfRollback_On(void)
{
    return sOn;
}

unsigned int MfRollback_Oldest(void)
{
    return sFirst;
}

int MfRollback_Save(unsigned int frame, const void *host)
{
    RbFrame *f;
    size_t n, i;

    if (!sOn) {
        return 0;
    }
    if (frame != sLast + 1) {
        MfRollback_End();
        return 0;
    }
    /* the pages the frame changed, including any it took into use */
    if (!Rb_ShadowReach(MfMem_Used(), 0)) {
        MfRollback_End();
        return 0;
    }
    n = Rb_FindDirty();
    if (n == (size_t)-1) {
        MfRollback_End();
        return 0;
    }
    f = &sRing[frame % MF_RB_FRAMES];
    f->valid = 1;
    f->frame = frame;
    f->count = 0;
    if (!Rb_Grow((void **)&f->pages, &f->capacity, n, sizeof(uint32_t))) {
        MfRollback_End();
        return 0;
    }
    {
        /* old[] follows the page list's capacity */
        unsigned char *old = realloc(f->old, f->capacity * PAGE);
        if (old == NULL) {
            MfRollback_End();
            return 0;
        }
        f->old = old;
    }
    for (i = 0; i < n; i++) {
        size_t at = (size_t)sDirty[i] * PAGE;
        f->pages[f->count] = sDirty[i];
        memcpy(f->old + f->count * PAGE, sShadow + at, PAGE);
        memcpy(sShadow + at, MfMem_Base() + at, PAGE);
        f->count++;
    }
    Rb_CopyGlobals(f->globals, STATE_START);
    memcpy(f->host, host, MF_RB_HOST_SIZE);
    sLast = frame;
    if (sLast - sFirst >= MF_RB_FRAMES) {
        sFirst = sLast - (MF_RB_FRAMES - 1);
    }
    sStats.frames++;
    sStats.pages += n;
    if (n > sStats.most_pages) {
        sStats.most_pages = n;
    }
    return 1;
}

int MfRollback_Restore(unsigned int frame, void *host)
{
    unsigned int k;
    size_t i;

    if (!sOn || frame < sFirst || frame > sLast) {
        return 0;
    }
    for (k = sLast; k > frame; k--) {
        RbFrame *f = &sRing[k % MF_RB_FRAMES];
        if (!f->valid || f->frame != k) {
            MfRollback_End();
            return 0;
        }
        for (i = f->count; i-- > 0;) {
            size_t at = (size_t)f->pages[i] * PAGE;
            memcpy(MfMem_Base() + at, f->old + i * PAGE, PAGE);
            memcpy(sShadow + at, f->old + i * PAGE, PAGE);
        }
        f->valid = 0;
    }
#ifdef _WIN32
    ResetWriteWatch(MfMem_Base(), MfMem_Size());
#endif
    Rb_CopyGlobals(STATE_START, sRing[frame % MF_RB_FRAMES].globals);
    memcpy(host, sRing[frame % MF_RB_FRAMES].host, MF_RB_HOST_SIZE);
    sStats.rewinds++;
    sStats.frames_back += sLast - frame;
    sLast = frame;
    return 1;
}

/* The engine as it was when it had started up, before any session (MfRollback_KeepStart): every session begins
 * from it, so a session is the same whatever ran before it, on any PC */
static unsigned char *sStart;
static size_t sStartSize, sStartHigh;
static unsigned char *sStartGlobals;
static unsigned char sStartHost[MF_RB_HOST_SIZE];

int MfRollback_KeepStart(const void *host)
{
    size_t used = MfMem_Used();

    free(sStart);
    free(sStartGlobals);
    sStart = malloc(used);
    sStartGlobals = malloc(Rb_GlobalsSize());
    if (sStart == NULL || sStartGlobals == NULL) {
        free(sStart);
        free(sStartGlobals);
        sStart = sStartGlobals = NULL;
        return 0;
    }
    memcpy(sStart, MfMem_Base(), used);
    Rb_CopyGlobals(sStartGlobals, STATE_START);
    memcpy(sStartHost, host, MF_RB_HOST_SIZE);
    sStartSize = used;
    sStartHigh = used;
    return 1;
}

int MfRollback_RestoreStart(void *host)
{
    size_t used = MfMem_Used();

    if (sStart == NULL) {
        return 0;
    }
    MfRollback_End();
    memcpy(MfMem_Base(), sStart, sStartSize);
    if (used > sStartSize) {
        /* what sessions used past it, as it was: never written */
        memset(MfMem_Base() + sStartSize, 0, used - sStartSize);
    }
    MfMem_SetUsed(sStartHigh);
    Rb_CopyGlobals(STATE_START, sStartGlobals);
    memcpy(host, sStartHost, MF_RB_HOST_SIZE);
    return 1;
}

void MfRollback_Stats(MfRbStats *out)
{
    *out = sStats;
    out->globals = Rb_GlobalsSize();
    out->shadow = sShadowSize;
}
