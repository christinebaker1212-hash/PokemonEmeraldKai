// Moemon Co-op: when the game crashes it writes crash_log.txt (next to the
// game) with where it happened, the functions on the stack and what the game
// was doing, then tells the player. Addresses are printed as offsets into the
// exe ("exe+1a2b3c"), which `nm` on the same build turns into function names.
#ifdef PLATFORM_SDL2
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "main.h"

#define CRASH_LOG "crash_log.txt"
#define BUILD_ID __DATE__ " " __TIME__

static void WriteGameState(FILE *f, uintptr_t base)
{
    fprintf(f, "callback1 exe+%llx callback2 exe+%llx\n",
            (unsigned long long)((uintptr_t)gMain.callback1 - base), (unsigned long long)((uintptr_t)gMain.callback2 - base));
    fprintf(f, "in battle %d\n", gMain.inBattle);
    if (gSaveBlock1Ptr)
        fprintf(f, "map %d.%d at %d,%d\n", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum,
                gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
}

static void WriteHeader(FILE *f)
{
    time_t now = time(NULL);
    char when[64];
    strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(f, "\n=== Moemon Emerald Kai Co-op crash, %s (build %s)\n", when, BUILD_ID);
}

#ifdef _WIN32
#include <windows.h>

static LONG WINAPI CrashFilter(EXCEPTION_POINTERS *ep)
{
    static volatile LONG sOnce;
    uintptr_t base = (uintptr_t)GetModuleHandleA(NULL), textLo = base, textHi = base;
    CONTEXT *c = ep->ContextRecord;
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    FILE *f;

    if (InterlockedExchange(&sOnce, 1))
        return EXCEPTION_EXECUTE_HANDLER;
    // The exe's code section, to pick return addresses out of the stack
    {
        IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)base;
        IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
        IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
        int i;
        for (i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++)
        {
            if (memcmp(sec->Name, ".text", 5) == 0)
            {
                textLo = base + sec->VirtualAddress;
                textHi = textLo + sec->Misc.VirtualSize;
            }
        }
    }
    f = fopen(CRASH_LOG, "a");
    if (f)
    {
        uintptr_t sp = (uintptr_t)c->Rsp, top = (uintptr_t)((NT_TIB *)NtCurrentTeb())->StackBase;
        int found = 0;
        WriteHeader(f);
        fprintf(f, "exception %08lx at exe+%llx", (unsigned long)er->ExceptionCode, (unsigned long long)(c->Rip - base));
        if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2)
            fprintf(f, " (%s address %llx)", er->ExceptionInformation[0] ? "writing" : "reading",
                    (unsigned long long)er->ExceptionInformation[1]);
        fprintf(f, "\n");
        WriteGameState(f, base);
        fprintf(f, "stack:");
        for (; sp + 8 <= top && sp < (uintptr_t)c->Rsp + 256 * 1024 && found < 48; sp += 8)
        {
            uintptr_t v = *(uintptr_t *)sp;
            if (v >= textLo && v < textHi)
            {
                fprintf(f, " exe+%llx", (unsigned long long)(v - base));
                found++;
            }
        }
        fprintf(f, "\n");
        fclose(f);
    }
    MessageBoxA(NULL, "Moemon Emerald Kai Co-op crashed, sorry!\n\n"
                      "A report was saved to crash_log.txt next to the game.\n"
                      "Please send it along with what you were doing.",
                "Moemon Emerald Kai Co-op", MB_OK | MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void AbortHandler(int sig)
{
    FILE *f = fopen(CRASH_LOG, "a");
    if (f)
    {
        WriteHeader(f);
        fprintf(f, "signal %d (abort)\n", sig);
        WriteGameState(f, (uintptr_t)GetModuleHandleA(NULL));
        fclose(f);
    }
    _exit(3);
}

void Platform_InstallCrashLog(void)
{
    SetUnhandledExceptionFilter(CrashFilter);
    signal(SIGABRT, AbortHandler);
}

#else
#include <execinfo.h>
#include <unistd.h>

static void CrashSignal(int sig)
{
    void *frames[48];
    int n = backtrace(frames, 48);
    FILE *f = fopen(CRASH_LOG, "a");
    if (f)
    {
        WriteHeader(f);
        fprintf(f, "signal %d\n", sig);
        WriteGameState(f, 0);
        fflush(f);
        backtrace_symbols_fd(frames, n, fileno(f));
        fclose(f);
    }
    fprintf(stderr, "[crash] signal %d, see " CRASH_LOG "\n", sig);
    backtrace_symbols_fd(frames, n, 2);
    signal(sig, SIG_DFL);
    raise(sig);
}

void Platform_InstallCrashLog(void)
{
    signal(SIGSEGV, CrashSignal);
    signal(SIGBUS, CrashSignal);
    signal(SIGFPE, CrashSignal);
    signal(SIGILL, CrashSignal);
    signal(SIGABRT, CrashSignal);
}
#endif
#endif // PLATFORM_SDL2
