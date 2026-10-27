/*
 * Moemon Fight: the fight's heap (fight/host/mf_mem.c). Everything the engine allocates on its own thread goes into
 * one region (with the engine's stack), so that region and the engine's globals are the fight's whole state: what
 * rollback saves and restores (fight/host/mf_rollback.c). Other threads (the music) get the C library's heap. A
 * file defines MF_SYSTEM_HEAP before including this to keep to the C library's heap.
 */
#ifndef MF_MEM_H
#define MF_MEM_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void *MfMem_Malloc(size_t size);
void *MfMem_Calloc(size_t count, size_t size);
void *MfMem_Realloc(void *ptr, size_t size);
void MfMem_Free(void *ptr);
char *MfMem_Strdup(const char *s);
void *MfMem_ReallocSystem(void *ptr, size_t size);

#if defined(MF_SYSTEM_HEAP) && !defined(MF_MEM_IMPL)
/* new memory from the C library, but what the engine gave (in the region) is still freed by the region */
#undef realloc
#undef free
#define realloc(p, n) MfMem_ReallocSystem(p, n)
#define free(p) MfMem_Free(p)
#elif !defined(MF_SYSTEM_HEAP)
#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup
#undef _strdup
#define malloc(n) MfMem_Malloc(n)
#define calloc(n, s) MfMem_Calloc(n, s)
#define realloc(p, n) MfMem_Realloc(p, n)
#define free(p) MfMem_Free(p)
#define strdup(s) MfMem_Strdup(s)
#define _strdup(s) MfMem_Strdup(s)
#endif

#endif
