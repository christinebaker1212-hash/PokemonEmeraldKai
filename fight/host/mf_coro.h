/* Moemon Fight: the engine as a coroutine on the fight's heap region (fight/host/mf_coro.c) */
#ifndef MF_CORO_H
#define MF_CORO_H

/* Sets the engine up to run fn on the region's stack (MfMem_Init first). 0: no region */
int MfCoro_Create(void (*fn)(void));
/* Runs the engine until it yields: 0 when its function has returned */
int MfCoro_Resume(void);
/* From inside the engine: back to MfCoro_Resume's caller until the next resume */
void MfCoro_Yield(void);
int MfCoro_Done(void);

#endif
