/* Moemon Fight: the fight's heap region, for the host (fight/host/mf_mem.c, mf_coro.c, mf_rollback.c) */
#ifndef MF_MEM_INTERNAL_H
#define MF_MEM_INTERNAL_H
#include <stddef.h>

#define MF_MEM_STACK_SIZE ((size_t)16 << 20)    /* the engine's stack, at the start of the region */

/* Set while the engine runs (its coroutine): its allocations go to the region */
extern _Thread_local int MfMem_InEngine;

/* Reserves the region (once). 0 when it can't: the engine then uses the C library's heap and can't roll back */
int MfMem_Init(void);
unsigned char *MfMem_Base(void);
size_t MfMem_Size(void);
/* Bytes from the base that may have been written: the stack and the heap up to its top chunk */
size_t MfMem_Used(void);
/* Puts back what MfMem_Used said (the region restored to an earlier state) */
void MfMem_SetUsed(size_t used);
/* The engine's stack: from the base up to here */
void *MfMem_StackTop(void);
/* Frees from other threads that were left alone */
size_t MfMem_Leaked(void);
/* Desync recovery: notes what this process has mapped (MfMem_MapLocal, before a series of checks), then whether a
 * value is a pointer into it outside the region (code, the C library's heap: different on every PC) */
void MfMem_MapLocal(void);
int MfMem_LocalPointer(unsigned long long value);

#endif
