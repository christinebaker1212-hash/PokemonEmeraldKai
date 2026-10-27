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
#else
#include <signal.h>
#include <sys/mman.h>
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

#ifndef _WIN32
/*
 * Linux (the test builds): the pages a frame writes are found by write-protecting the region and noting each page
 * the first time it's written (the fault handler lets it be written from then on). Without it (AddressSanitizer has
 * the handler), every page is compared with the shadow, which costs a lot more.
 */
static int sProtect = -1;               /* -1 not set up yet, 0 compare instead, 1 protect */
static unsigned char *sProtLo;
static volatile size_t sProtLen;
static uint32_t *sTrap;                 /* pages written since the last look */
static size_t sTrapCap;
static volatile size_t sTrapCount;
static volatile int sTrapOverflow;
static struct sigaction sOldSegv;

static void Rb_OnSegv(int sig, siginfo_t *si, void *ctx)
{
    unsigned char *a = si->si_addr;

    if (sProtLen != 0 && a >= sProtLo && a < sProtLo + sProtLen) {
        unsigned char *page = (unsigned char *)((uintptr_t)a & ~(uintptr_t)(PAGE - 1));
        mprotect(page, PAGE, PROT_READ | PROT_WRITE);
        if (sTrapCount < sTrapCap) {
            sTrap[sTrapCount++] = (uint32_t)((size_t)(page - sProtLo) / PAGE);
        } else {
            sTrapOverflow = 1;
        }
        return;
    }
    /* not ours */
    if (sOldSegv.sa_flags & SA_SIGINFO) {
        sOldSegv.sa_sigaction(sig, si, ctx);
    } else if (sOldSegv.sa_handler != SIG_DFL && sOldSegv.sa_handler != SIG_IGN) {
        sOldSegv.sa_handler(sig);
    } else {
        signal(SIGSEGV, SIG_DFL);
        raise(SIGSEGV);
    }
}

static int Rb_ProtectSetUp(void)
{
    struct sigaction sa;

    if (sProtect >= 0) {
        return sProtect;
    }
    sProtect = 0;
#if defined(__SANITIZE_ADDRESS__)
    return 0;
#endif
    if (getenv("MOEMON_FIGHT_RB_COMPARE") != NULL) {
        return 0;
    }
    sTrapCap = MfMem_Size() / PAGE;
    sTrap = malloc(sTrapCap * sizeof(uint32_t));
    if (sTrap == NULL) {
        return 0;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = Rb_OnSegv;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, &sOldSegv) != 0) {
        return 0;
    }
    sProtect = 1;
    return 1;
}

/* Write-protects pages (or lets them be written) */
static void Rb_Protect(size_t from, size_t to, int on)
{
    if (to > from) {
        mprotect(MfMem_Base() + from, to - from, on ? PROT_READ : PROT_READ | PROT_WRITE);
    }
}

static void Rb_Unprotect(void)
{
    if (sProtect == 1 && sProtLen != 0) {
        size_t len = sProtLen;
        sProtLen = 0;
        Rb_Protect(0, len, 0);
    }
}
#endif

/* The engine thread runs the engine on the region's stack: a fault there needs a stack of its own (Linux) */
void MfRollback_ThreadInit(void)
{
#ifndef _WIN32
    static unsigned char altstack[64 * 1024];
    stack_t ss;

    memset(&ss, 0, sizeof(ss));
    ss.ss_sp = altstack;
    ss.ss_size = sizeof(altstack);
    sigaltstack(&ss, NULL);
#endif
}

/* Finds the pages changed since the last call (in sDirty), up to the shadow's reach (reach_before: how far the
 * last call's reach went: past it, pages are taken into use now) */
static size_t Rb_FindDirty(size_t reach_before)
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
    if (sProtect == 1 && sProtLen != 0 && !sTrapOverflow) {
        /* the pages written, then the pages newly taken into use (compared: they were never protected) */
        size_t trapped = sTrapCount;
        for (i = 0; i < trapped; i++) {
            sDirty[n++] = sTrap[i];
            Rb_Protect((size_t)sTrap[i] * PAGE, (size_t)sTrap[i] * PAGE + PAGE, 1);
        }
        sTrapCount = 0;
        for (i = reach_before / PAGE; i < pages; i++) {
            const uint64_t *a = (const uint64_t *)(base + i * PAGE), *b = (const uint64_t *)(sShadow + i * PAGE);
            size_t k;
            for (k = 0; k < PAGE / 8; k++) {
                if (a[k] != b[k]) {
                    sDirty[n++] = (uint32_t)i;
                    break;
                }
            }
        }
        Rb_Protect(reach_before, sShadowSize, 1);
        sProtLen = sShadowSize;
        return n;
    }
    if (sProtect == 1 && sProtLen != 0) {
        /* too many pages to note: compare them all this time */
        Rb_Unprotect();
        sTrapCount = 0;
        sTrapOverflow = 0;
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
    if (sProtect == 1) {
        Rb_Protect(0, sShadowSize, 1);
        sProtLen = sShadowSize;
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
#else
    if (Rb_ProtectSetUp()) {
        sProtLo = MfMem_Base();
        sTrapCount = 0;
        sTrapOverflow = 0;
        Rb_Protect(0, sShadowSize, 1);
        sProtLen = sShadowSize;
    }
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
#ifndef _WIN32
    Rb_Unprotect();
    sTrapCount = 0;
    sTrapOverflow = 0;
#endif
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
    {
        size_t reach = sShadowSize;
        if (!Rb_ShadowReach(MfMem_Used(), 0)) {
            MfRollback_End();
            return 0;
        }
        n = Rb_FindDirty(reach);
    }
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
#ifndef _WIN32
            if (sProtect == 1 && sProtLen != 0) {
                Rb_Protect(at, at + PAGE, 0);
            }
#endif
            memcpy(MfMem_Base() + at, f->old + i * PAGE, PAGE);
            memcpy(sShadow + at, f->old + i * PAGE, PAGE);
#ifndef _WIN32
            if (sProtect == 1 && sProtLen != 0) {
                Rb_Protect(at, at + PAGE, 1);
            }
#endif
        }
        f->valid = 0;
    }
#ifdef _WIN32
    ResetWriteWatch(MfMem_Base(), MfMem_Size());
#else
    if (sProtect == 1 && sProtLen != 0) {
        /* pages written since the last frame was kept, now as they were: watched again */
        size_t k;
        for (k = 0; k < sTrapCount; k++) {
            size_t at = (size_t)sTrap[k] * PAGE;
            memcpy(MfMem_Base() + at, sShadow + at, PAGE);
            Rb_Protect(at, at + PAGE, 1);
        }
        sTrapCount = 0;
    }
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
