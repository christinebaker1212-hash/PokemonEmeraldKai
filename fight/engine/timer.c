/*
 * Timer and frame pacing (original module 0x476C90-0x47713F).
 *
 * Times are 64-bit counts of QueryPerformanceCounter ticks (or timeGetTime
 * milliseconds when there is no performance counter), relative to
 * g_timerStart. The game advances g_timerFrame once per logic frame; a frame
 * is due every g_timerFrameTicks ticks after g_timerFrameStart.
 *
 * Several callers in other modules inline Timer_Count instead of calling
 * these functions, so the globals are part of the interface.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "mugen.h"

/* Current time relative to g_timerStart (inlined everywhere in the original). */
static __int64 Timer_Count(void)
{
    LARGE_INTEGER now;

    if (g_timerPerformance) {
        QueryPerformanceCounter(&now);
        return now.QuadPart - g_timerStart;
    }
    return (__int64)timeGetTime() - g_timerStart;
}

/* 0x00476C90: always returns 1. */
int Timer_Init(void)
{
    LARGE_INTEGER freq;

    if (QueryPerformanceFrequency(&freq)) {
        g_timerPerformance = 1;
        g_timerFrequency = freq.QuadPart;
    } else {
        g_timerPerformance = 0;
        timeBeginPeriod(1);
        g_timerFrequency = 1000;
    }
    g_timerTicksPerSecond = 100;
    g_timerFpsInterval = 10;
    g_timerFpsFrames = 0;
    g_timerFpsStart = 0;
    g_timerStart = 0;
    g_timerStart = Timer_Count();
    g_timerInitialized = 1;
    return 1;
}

/* 0x00476D70: time since Timer_Init in 1/g_timerTicksPerSecond seconds (1/100). */
__int64 Timer_GetTicks(void)
{
    return (__int64)g_timerTicksPerSecond * Timer_Count() / g_timerFrequency;
}

/*
 * 0x00476DE0: frames per second, measured once every g_timerFpsInterval + 1
 * calls (one call per frame); the other calls return the last measurement
 * (-1 until the first one after Timer_Reset).
 */
float Timer_GetFps(void)
{
    __int64 now = Timer_Count();
    __int64 ticks;
    __int64 elapsed;

    if (g_timerFpsFrames < g_timerFpsInterval) {
        g_timerFpsFrames++;
        return (float)g_timerMeasuredFps;
    }
    ticks = (__int64)(g_timerFpsInterval + 1) * g_timerFrequency;
    elapsed = now - g_timerFpsStart;
    g_timerFpsStart = now;
    g_timerFpsFrames = 0;
    /* The original divides the exact 64-bit integers on the x87 stack; this
     * rounds them to double first, which only differs from 2^53 ticks on. */
    g_timerMeasuredFps = (double)ticks / (double)elapsed;
    return (float)g_timerMeasuredFps;
}

/* 0x00476EC0: restarts the FPS measurement, now every 31 frames. */
void Timer_Reset(void)
{
    g_timerFpsInterval = 30;
    g_timerFpsFrames = 0;
    g_timerFpsStart = Timer_Count();
    g_timerMeasuredFps = -1.0;
}

/* 0x00476F30: number of frames due since g_timerFrameStart. */
__int64 Timer_GetFrame(void)
{
    return (Timer_Count() - g_timerFrameStart) / g_timerFrameTicks;
}

/* 0x00476F90: makes the current time frame 0 (after a stall or a speed change). */
void Timer_ResetFrame(void)
{
    __int64 now = Timer_Count();

    g_timerFrame = 0;
    g_timerResyncs++;
    g_timerFrameStart = now;
}

/*
 * 0x00476FF0: sets the frame rate. The frame length is rounded to the nearest
 * tick; fps is divided as an unsigned value, like the original.
 */
void Timer_SetFps(int fps)
{
    g_timerFrameRate = fps;
    g_frameRate = fps;
    g_timerFrameTicks = (g_timerFrequency * 2 + 1) / (unsigned int)fps / 2;
    Timer_ResetFrame();
}

/* 0x00477040: ticks left until frame g_timerFrame is due (negative when late). */
__int64 Timer_GetFrameTimeLeft(void)
{
    __int64 due = g_timerFrame * g_timerFrameTicks + g_timerFrameStart;

    return due - Timer_Count();
}

/*
 * 0x004770D0: waits until frame g_timerFrame is due: sleeps while more than
 * 2 ms are left, then spins. Returns how many frames late the game is (no
 * caller uses it).
 */
int Timer_WaitFrame(void)
{
    __int64 behind;
    int ms = (int)(Timer_GetFrameTimeLeft() * 1000 / g_timerFrequency);

    if (ms > 2) {
        Sleep(ms - 1);
    }
    while (Timer_GetFrameTimeLeft() > 0) {
    }
    behind = Timer_GetFrame() - g_timerFrame;
    if (behind < 0) {
        behind = 0;
    }
    return (int)behind;
}
