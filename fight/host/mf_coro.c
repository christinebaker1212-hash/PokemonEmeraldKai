/*
 * Moemon Fight: the engine as a coroutine on a stack inside the fight's heap region (fight/host/mf_mem.c).
 *
 * The engine thread switches into the engine, which runs until a frame ends (MfCoro_Yield) and switches back. The
 * engine's stack and the place it stopped at (saved on that stack, its pointer at the region's base) are inside
 * the region, so restoring the region also puts the engine back where it was at that frame. The switch saves
 * the registers a call must keep (and on Windows the thread's stack limits, as fibers do); x86-64 only.
 */
#define MF_SYSTEM_HEAP
#include <stdint.h>
#include <string.h>

#include "mf_mem_internal.h"
#include "mf_coro.h"

/* At the region's base, under the stack: the engine's saved stack pointer and what it runs */
typedef struct CoroHeader {
    void *sp;               /* where the engine stopped (MfCoro_Yield), or its first frame */
    int done;               /* the engine's function returned */
} CoroHeader;

#define STACK_GUARD 4096    /* the header's page */

void mf_coro_switch(void **save_sp, void *load_sp);

#if defined(_WIN32)
/* Windows x64: rbx rbp rdi rsi r12-r15 xmm6-xmm15, MXCSR and the x87 control word, and the thread's stack
 * limits (TEB StackBase, StackLimit, DeallocationStack) */
__asm__(
    ".text\n"
    ".globl mf_coro_switch\n"
    ".def mf_coro_switch; .scl 2; .type 32; .endef\n"
    "mf_coro_switch:\n"
    ".intel_syntax noprefix\n"
    "    push rbp\n"
    "    push rbx\n"
    "    push rdi\n"
    "    push rsi\n"
    "    push r12\n"
    "    push r13\n"
    "    push r14\n"
    "    push r15\n"
    "    push qword ptr gs:[0x08]\n"
    "    push qword ptr gs:[0x10]\n"
    "    push qword ptr gs:[0x1478]\n"
    "    sub rsp, 168\n"
    "    movups [rsp], xmm6\n"
    "    movups [rsp+16], xmm7\n"
    "    movups [rsp+32], xmm8\n"
    "    movups [rsp+48], xmm9\n"
    "    movups [rsp+64], xmm10\n"
    "    movups [rsp+80], xmm11\n"
    "    movups [rsp+96], xmm12\n"
    "    movups [rsp+112], xmm13\n"
    "    movups [rsp+128], xmm14\n"
    "    movups [rsp+144], xmm15\n"
    "    stmxcsr [rsp+160]\n"
    "    fnstcw [rsp+164]\n"
    "    mov [rcx], rsp\n"
    "    mov rsp, rdx\n"
    "    movups xmm6, [rsp]\n"
    "    movups xmm7, [rsp+16]\n"
    "    movups xmm8, [rsp+32]\n"
    "    movups xmm9, [rsp+48]\n"
    "    movups xmm10, [rsp+64]\n"
    "    movups xmm11, [rsp+80]\n"
    "    movups xmm12, [rsp+96]\n"
    "    movups xmm13, [rsp+112]\n"
    "    movups xmm14, [rsp+128]\n"
    "    movups xmm15, [rsp+144]\n"
    "    ldmxcsr [rsp+160]\n"
    "    fldcw [rsp+164]\n"
    "    add rsp, 168\n"
    "    pop qword ptr gs:[0x1478]\n"
    "    pop qword ptr gs:[0x10]\n"
    "    pop qword ptr gs:[0x08]\n"
    "    pop r15\n"
    "    pop r14\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rsi\n"
    "    pop rdi\n"
    "    pop rbx\n"
    "    pop rbp\n"
    "    ret\n"
    ".att_syntax prefix\n");
#define FRAME_FLOATS 168
#define FRAME_WORDS 11      /* 3 stack limits + 8 registers */
#else
/* System V x86-64: rbx rbp r12-r15, MXCSR and the x87 control word */
__asm__(
    ".text\n"
    ".globl mf_coro_switch\n"
    ".type mf_coro_switch, @function\n"
    "mf_coro_switch:\n"
    ".intel_syntax noprefix\n"
    "    push rbp\n"
    "    push rbx\n"
    "    push r12\n"
    "    push r13\n"
    "    push r14\n"
    "    push r15\n"
    "    sub rsp, 8\n"
    "    stmxcsr [rsp]\n"
    "    fnstcw [rsp+4]\n"
    "    mov [rdi], rsp\n"
    "    mov rsp, rsi\n"
    "    ldmxcsr [rsp]\n"
    "    fldcw [rsp+4]\n"
    "    add rsp, 8\n"
    "    pop r15\n"
    "    pop r14\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbx\n"
    "    pop rbp\n"
    "    ret\n"
    ".att_syntax prefix\n"
    ".size mf_coro_switch, .-mf_coro_switch\n");
#define FRAME_FLOATS 8
#define FRAME_WORDS 6
#endif

static void *sHostSp;           /* the engine thread's own stack while the engine runs */
static void (*sFn)(void);

static CoroHeader *Coro_Header(void)
{
    return (CoroHeader *)MfMem_Base();
}

/* The engine's first frame returns here */
static void Coro_Entry(void)
{
    sFn();
    Coro_Header()->done = 1;
    for (;;) {
        MfCoro_Yield();
    }
}

int MfCoro_Create(void (*fn)(void))
{
    unsigned char *top = MfMem_StackTop();
    uint64_t *sp;
    unsigned char *frame;
    int i;

    if (top == NULL) {
        return 0;
    }
    sFn = fn;
    memset(Coro_Header(), 0, sizeof(CoroHeader));
    /* the return address 16-aligned (as a call leaves it), with room above it for the callee's home space */
    sp = (uint64_t *)((uintptr_t)(top - 64) & ~(uintptr_t)15);
    *sp = (uint64_t)(uintptr_t)Coro_Entry;
    sp -= FRAME_WORDS;
    for (i = 0; i < FRAME_WORDS; i++) {
        sp[i] = 0;
    }
#if defined(_WIN32)
    sp[0] = (uint64_t)(uintptr_t)MfMem_Base();                  /* DeallocationStack */
    sp[1] = (uint64_t)(uintptr_t)(MfMem_Base() + STACK_GUARD);  /* StackLimit */
    sp[2] = (uint64_t)(uintptr_t)top;                           /* StackBase */
#endif
    frame = (unsigned char *)sp - FRAME_FLOATS;
    memset(frame, 0, FRAME_FLOATS);
    *(uint32_t *)(frame + FRAME_FLOATS - 8) = 0x1F80;   /* MXCSR: round to nearest, exceptions masked */
    *(uint16_t *)(frame + FRAME_FLOATS - 4) = 0x037F;   /* x87: double extended, exceptions masked */
    Coro_Header()->sp = frame;
    return 1;
}

int MfCoro_Resume(void)
{
    CoroHeader *h = Coro_Header();

    if (h == NULL || h->done) {
        return 0;
    }
    MfMem_InEngine = 1;
    mf_coro_switch(&sHostSp, h->sp);
    MfMem_InEngine = 0;
    return !h->done;
}

void MfCoro_Yield(void)
{
    MfMem_InEngine = 0;
    mf_coro_switch(&Coro_Header()->sp, sHostSp);
    MfMem_InEngine = 1;
}

int MfCoro_Done(void)
{
    return Coro_Header() == NULL || Coro_Header()->done;
}
