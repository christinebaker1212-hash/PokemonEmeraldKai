/*
 * Moemon Fight: the fight's heap (fight/compat/mf_mem.h).
 *
 * One region of address space, reserved once: the engine's stack at its start (fight/host/mf_coro.c), then a
 * dlmalloc mspace for every allocation the engine makes while it runs (MfMem_InEngine). With the engine's
 * globals, the region is everything a fight changes, so rollback saves and restores the fight by saving and
 * restoring pages of it (fight/host/mf_rollback.c). The region is asked for at the same address on every PC, so
 * both players' heaps hold the same pointers. Anything else (other threads, or the region not being available)
 * uses the C library's heap.
 */
#define MF_SYSTEM_HEAP
#define MF_MEM_IMPL
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "mf_mem.h"
#include "mf_mem_internal.h"

void *create_mspace_with_base(void *base, size_t capacity, int locked);
void *mspace_malloc(void *msp, size_t bytes);
void mspace_free(void *msp, void *mem);
void *mspace_calloc(void *msp, size_t n_elements, size_t elem_size);
void *mspace_realloc(void *msp, void *mem, size_t newsize);
size_t mspace_usable_size(const void *mem);

/* dlmalloc's struct mallinfo, under a name of its own (it would clash with the C library's) */
#define MEM_BASE_HINT ((uintptr_t)0x6F0000000000ULL)

_Thread_local int MfMem_InEngine;
static unsigned char *sBase;
static size_t sSize;
static void *sSpace;
static size_t sLeaked;
static size_t sHigh;            /* bytes from the base anything was ever allocated in */

static void *Mem_Note(void *p, size_t size)
{
    if (p != NULL) {
        size_t end = (size_t)((unsigned char *)p - sBase) + size + 64;  /* with the next chunk's header */
        if (end > sHigh) {
            sHigh = end;
        }
    }
    return p;
}

static int Mem_InRegion(const void *p)
{
    return sBase != NULL && (const unsigned char *)p >= sBase && (const unsigned char *)p < sBase + sSize;
}

int MfMem_Init(void)
{
    static const size_t sizes[] = { (size_t)1024 << 20, (size_t)768 << 20, (size_t)512 << 20, (size_t)384 << 20 };
    const char *env = getenv("MOEMON_FIGHT_HEAP_MB");
    size_t want = env != NULL && atoi(env) >= 128 ? (size_t)atoi(env) << 20 : 0;
    int i;

    if (sBase != NULL) {
        return 1;
    }
    for (i = want ? -1 : 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])) && sBase == NULL; i++) {
        size_t size = i < 0 ? want : sizes[i];
#ifdef _WIN32
        void *p = VirtualAlloc((void *)MEM_BASE_HINT, size, MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH, PAGE_READWRITE);
        if (p == NULL) {
            p = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH, PAGE_READWRITE);
        }
#else
        int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
#ifdef MAP_FIXED_NOREPLACE
        void *p = mmap((void *)MEM_BASE_HINT, size, PROT_READ | PROT_WRITE, flags | MAP_FIXED_NOREPLACE, -1, 0);
        if (p == MAP_FAILED) {
            p = mmap(NULL, size, PROT_READ | PROT_WRITE, flags, -1, 0);
        }
#else
        void *p = mmap((void *)MEM_BASE_HINT, size, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
        if (p == MAP_FAILED) {
            p = NULL;
        }
#endif
        if (p != NULL) {
            sBase = p;
            sSize = size;
        }
    }
    if (sBase == NULL) {
        return 0;
    }
    sSpace = create_mspace_with_base(sBase + MF_MEM_STACK_SIZE, sSize - MF_MEM_STACK_SIZE, 0);
    if (sSpace == NULL) {
        return 0;
    }
    sHigh = MF_MEM_STACK_SIZE + 4096;  /* the mspace's own state */
    return 1;
}

void *MfMem_StackTop(void)
{
    return sBase != NULL ? sBase + MF_MEM_STACK_SIZE : NULL;
}

unsigned char *MfMem_Base(void)
{
    return sBase;
}

size_t MfMem_Size(void)
{
    return sSize;
}

/* The part of the region ever written: the stack and the heap up to the end of the highest block it ever gave
 * (with the top chunk's header); nothing past it has been touched */
size_t MfMem_Used(void)
{
    size_t used;

    if (sSpace == NULL) {
        return 0;
    }
    used = sHigh > MF_MEM_STACK_SIZE + 4096 ? sHigh : MF_MEM_STACK_SIZE + 4096;
    used = (used + 4096 + 4095) & ~(size_t)4095;
    return used < sSize ? used : sSize;
}

void MfMem_SetUsed(size_t used)
{
    sHigh = used > 4096 ? used - 4096 : used;
}

size_t MfMem_Leaked(void)
{
    return sLeaked;
}

void *MfMem_Malloc(size_t size)
{
    if (MfMem_InEngine && sSpace != NULL) {
        return Mem_Note(mspace_malloc(sSpace, size), size);
    }
    return malloc(size);
}

void *MfMem_Calloc(size_t count, size_t size)
{
    if (MfMem_InEngine && sSpace != NULL) {
        return Mem_Note(mspace_calloc(sSpace, count, size), count * size);
    }
    return calloc(count, size);
}

void *MfMem_Realloc(void *ptr, size_t size)
{
    if (ptr == NULL) {
        return MfMem_Malloc(size);
    }
    if (Mem_InRegion(ptr)) {
        if (!MfMem_InEngine) {
            /* never from another thread (the heap has no locks): a copy on the C library's heap */
            size_t old = mspace_usable_size(ptr);
            void *copy = malloc(size);
            if (copy != NULL) {
                memcpy(copy, ptr, old < size ? old : size);
            }
            sLeaked++;
            return copy;
        }
        return Mem_Note(mspace_realloc(sSpace, ptr, size), size);
    }
    return realloc(ptr, size);
}

void MfMem_Free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    if (Mem_InRegion(ptr)) {
        if (MfMem_InEngine) {
            mspace_free(sSpace, ptr);
        } else {
            sLeaked++;      /* another thread: left alone (the heap has no locks) */
        }
        return;
    }
    free(ptr);
}

void *MfMem_ReallocSystem(void *ptr, size_t size)
{
    if (ptr != NULL && Mem_InRegion(ptr)) {
        size_t old = mspace_usable_size(ptr);
        void *copy = malloc(size);
        if (copy != NULL) {
            memcpy(copy, ptr, old < size ? old : size);
            MfMem_Free(ptr);
        }
        return copy;
    }
    return realloc(ptr, size);
}

char *MfMem_Strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copy = MfMem_Malloc(n);

    if (copy != NULL) {
        memcpy(copy, s, n);
    }
    return copy;
}
