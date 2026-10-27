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

/*
 * The screenpack's space in the picture (docs/FIGHT_DECISIONS.md #5, #7): scaled to fit (by height for 4:3 packs in
 * the 3:2 picture) and centred, g_mfMotifOffsetX pixels from the left. The fight HUD is fitted by
 * MfFit_Hud (fight/host/mf_fit.c): each side's part goes to its real edge (g_mfHudSideX, chosen by MF_HUD_SIDE),
 * the top part to the cabinet's hud.top and anything past the safe margin back inside (g_mfHudTopY / BottomY,
 * picked per element while g_mfHudVertical is set). A sprite across the middle of the screen is shared by both
 * sides and keeps its centred place.
 */
struct SysDrawItem;
extern int g_mfMotifOffsetX;
extern int g_mfHudShift;
extern int g_mfHudSideX[2];
extern int g_mfHudShiftY;
extern int g_mfHudTopY, g_mfHudBottomY;
extern int g_mfHudVertical;
#define MF_HUD_SIDE(side) (g_mfHudShift = (side) < 0 ? 0 : g_mfHudSideX[(side) & 1])
#define MF_MOTIF_X() (g_mfMotifOffsetX + g_mfHudShift)
#define MF_MOTIF_Y() (g_mfHudShiftY)
/* GameVars.motif_scale and g_mfMotifOffsetX for the game resolution and the motif's localcoord */
void MfHost_FitMotif(void);
/* Fits the fight HUD (after fight.def and the cabinet's settings): top_px is where its top goes, -1 the pack's place */
void MfFit_Hud(int top_px);
/* An element about to be drawn (before SysElem_SetScale): picks its shifts */
void MfFit_PlaceItem(const struct SysDrawItem *e);

/* The fight is on screen (Fight_DrawHud) */
void MfHost_MatchShown(void);
/* A character one of the online players lacks (greyed out, never picked at random) */
int MfHost_CharOff(int char_index);
/* The stage's name for the VS screen ("" none), where (motif coordinates) and in which font (font, bank, align;
 * font -1: the pack's first name font) */
const char *MfHost_VsStage(int pos[2], int font[3]);

/*
 * The engine's math is bundled (fight/third_party/musl_math, musl's): every player's build computes the same
 * results, whatever the C library or the CPU (x87 fsin differs between makers), so online fights stay in step.
 */
#include <math.h>
double mfm_sin(double x);
double mfm_cos(double x);
double mfm_tan(double x);
double mfm_log(double x);
double mfm_exp(double x);
double mfm_pow(double x, double y);
double mfm_atan(double x);
double mfm_asin(double x);
double mfm_acos(double x);
#define sin mfm_sin
#define cos mfm_cos
#define tan mfm_tan
#define log mfm_log
#define exp mfm_exp
#define pow mfm_pow
#define atan mfm_atan
#define asin mfm_asin
#define acos mfm_acos

/* The engine's memory: the fight's heap (mf_mem.h, fight/host/mf_mem.c) */
#include "mf_mem.h"

/* Rollback: a frame being run again (MfEngine_StepQuiet) plays only the sounds and music its first run didn't */
int MfHost_Replaying(void);
unsigned int MfHost_FrameRunning(void);
/* The caller is the engine (its coroutine), not another thread such as the music's */
int MfHost_OnEngine(void);

/* The end of every frame of the screen loops (Screen_EndFrame). */
void MfHost_FrameEnd(void);

#endif /* MF_PORT_H */
