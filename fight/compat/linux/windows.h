/*
 * Moemon Fight: the Windows names the engine uses, for the Linux test build (the players' build
 * is Windows). Only types and the few calls the kept modules make.
 */
#ifndef MF_LINUX_WINDOWS_H
#define MF_LINUX_WINDOWS_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define __stdcall
#define __cdecl
#define CALLBACK
#define WINAPI
#define __declspec(x)
#define __forceinline inline __attribute__((always_inline))
#define __int64 long      /* = int64_t on 64-bit Linux, as long long is on Windows */

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef uint32_t DWORD;
typedef unsigned int UINT;
typedef int32_t LONG;
typedef intptr_t INT_PTR;
typedef intptr_t LPARAM;
typedef uintptr_t WPARAM;
typedef intptr_t LRESULT;
typedef void *HANDLE;
typedef void *HWND;
typedef void *HMODULE;
typedef void *HINSTANCE;
typedef void *HICON;
typedef void *HMONITOR;
typedef void *HDC;
typedef void *HBRUSH;
typedef int64_t __time64_t;
#define _time64 time
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { DWORD cbSize; RECT rcMonitor; RECT rcWork; DWORD dwFlags; } MONITORINFO;
typedef union { struct { DWORD LowPart; LONG HighPart; } u; int64_t QuadPart; } LARGE_INTEGER;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define MAX_PATH 260

#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#define _isnan isnan

static inline char *_strlwr(char *s)
{
    char *p;
    for (p = s; *p; p++)
        if (*p >= 'A' && *p <= 'Z')
            *p += 'a' - 'A';
    return s;
}

static inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *f)
{
    f->QuadPart = 1000000000LL;
    return 1;
}

static inline BOOL QueryPerformanceCounter(LARGE_INTEGER *c)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    c->QuadPart = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    return 1;
}

static inline void Sleep(DWORD ms)
{
    usleep((useconds_t)ms * 1000);
}

#define LOCALE_USER_DEFAULT 0x0400
#define LOCALE_SISO639LANGNAME 0x59
static inline int GetLocaleInfoA(DWORD locale, DWORD type, char *data, int size)
{
    return 0;
}

#define MEM_DECOMMIT 0x4000
static inline BOOL VirtualFree(void *address, size_t size, DWORD type)
{
    return 1;
}

typedef void (*FARPROC)(void);
static inline HMODULE LoadLibraryA(const char *name)
{
    return NULL;
}
static inline FARPROC GetProcAddress(HMODULE module, const char *name)
{
    return NULL;
}
static inline BOOL FreeLibrary(HMODULE module)
{
    return 1;
}

#define InterlockedExchangeAdd(p, v) __sync_fetch_and_add((p), (v))
#define InterlockedIncrement(p) __sync_add_and_fetch((p), 1)
#define InterlockedDecrement(p) __sync_sub_and_fetch((p), 1)
#define InterlockedExchange(p, v) __sync_lock_test_and_set((p), (v))

/* Window placement (video.c's full screen switch, never reached: the engine has no window of its own) */
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define MONITOR_DEFAULTTOPRIMARY 1
#define WS_POPUP 0x80000000u
#define WS_VISIBLE 0x10000000u
#define WS_SYSMENU 0x00080000u
#define HWND_TOP ((HWND)0)
#define HWND_NOTOPMOST ((HWND)-2)
#define SWP_FRAMECHANGED 0x0020
#define SWP_SHOWWINDOW 0x0040
static inline BOOL GetClientRect(HWND w, RECT *r) { return 0; }
static inline BOOL GetWindowRect(HWND w, RECT *r) { return 0; }
static inline LONG GetWindowLongA(HWND w, int i) { return 0; }
static inline LONG SetWindowLongA(HWND w, int i, LONG v) { return 0; }
static inline int GetSystemMetrics(int i) { return 0; }
static inline HMONITOR MonitorFromWindow(HWND w, DWORD f) { return NULL; }
static inline BOOL GetMonitorInfoA(HMONITOR m, MONITORINFO *mi) { return 0; }
static inline BOOL SetWindowPos(HWND w, HWND after, int x, int y, int cx, int cy, UINT f) { return 0; }

#endif /* MF_LINUX_WINDOWS_H */
