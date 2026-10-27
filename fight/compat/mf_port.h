/*
 * Moemon Fight: included at the end of the engine's mugen.h. The engine runs inside the game:
 * its files are under the mugen/ folder (never the game's working directory, which it must not
 * change), its clock is the one the game steps, and a fatal error returns to the game instead of
 * ending the program. fight/host/mf_host.c implements these.
 */
#ifndef MF_PORT_H
#define MF_PORT_H

#include <stdio.h>

FILE *MfFile_Open(const char *path, const char *mode);
int MfFile_Remove(const char *path);
int MfFile_Chdir(const char *path);
char *MfFile_Getcwd(char *buf, int size);
#define fopen(path, mode) MfFile_Open(path, mode)
#define remove(path) MfFile_Remove(path)
#define _chdir(path) MfFile_Chdir(path)
#define _getcwd(buf, size) MfFile_Getcwd(buf, size)

/* The engine's clock: one frame of the game per step, never the wall clock. */
int MfTime_Counter(LARGE_INTEGER *count);
int MfTime_Frequency(LARGE_INTEGER *freq);
unsigned int MfTime_Ms(void);
#define QueryPerformanceCounter(count) MfTime_Counter(count)
#define QueryPerformanceFrequency(freq) MfTime_Frequency(freq)
#define timeGetTime() MfTime_Ms()

/* A fatal error (Error_Exit) or exit() goes back to the game. */
void MfHost_Exit(int code) __attribute__((noreturn));
#define exit(code) MfHost_Exit(code)
void MfHost_ErrorDialog(const char *text, const char *title);

/* The "moemon" BGM plugin (compat/mf_bgm.c) */
extern const BgmSubsystem g_mfBgmSubsystem;

/* After data/mugen.cfg is read: what the game needs from the engine (System_Init) */
void MfHost_ConfigLoaded(void);
/* After the [P1 Keys] / [P2 Keys] of the options (Options_Load) */
void MfHost_KeysLoaded(void);

/* The end of every frame of the screen loops (Screen_EndFrame). */
void MfHost_FrameEnd(void);

#endif /* MF_PORT_H */
