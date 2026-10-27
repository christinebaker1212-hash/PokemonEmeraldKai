#ifndef GUARD_MF_ENGINE_H
#define GUARD_MF_ENGINE_H

/*
 * Moemon Fight: the fighting game engine (fight/, a port of mugen-unleashed) as the game sees
 * it. The engine runs on its own thread and draws a 480x320 picture; the game steps it one
 * frame at a time with both players' buttons. No SDL types: the engine's own SDL 1.2 names live
 * in fight/compat. Everything here is called from the game's main thread except
 * MfEngine_MixAudio (the audio callback).
 */

#define MF_SCREEN_W 480
#define MF_SCREEN_H 320

// A player's buttons for one frame
#define MF_BTN_UP     0x0001
#define MF_BTN_DOWN   0x0002
#define MF_BTN_LEFT   0x0004
#define MF_BTN_RIGHT  0x0008
#define MF_BTN_A      0x0010
#define MF_BTN_B      0x0020
#define MF_BTN_C      0x0040
#define MF_BTN_X      0x0080
#define MF_BTN_Y      0x0100
#define MF_BTN_Z      0x0200
#define MF_BTN_START  0x0400

enum
{
    MF_MODE_VERSUS, // two players pick, then fight each other
    MF_MODE_CPU,    // player 1 picks and fights a CPU-picked opponent
};

enum
{
    MF_RESULT_NONE,     // still running, or not started
    MF_RESULT_P1,       // the left side won
    MF_RESULT_P2,       // the right side won
    MF_RESULT_DRAW,
    MF_RESULT_CANCELLED, // ended by MfEngine_Cancel (or a player quit)
    MF_RESULT_ERROR,    // the engine stopped on an error: MfEngine_Error says why
};

struct MfSession
{
    int mode;            // MF_MODE_*
    unsigned int seed;   // the match RNG's seed (the same on both sides online)
    char cabinet[32];    // section of mugen/cabinets.ini with the rules
};

// Starts a session (loading the engine the first time). Returns 0 when the engine can't run.
int MfEngine_Begin(const struct MfSession *session);
// A session is running (select, versus, fight, victory).
int MfEngine_IsActive(void);
// Runs one frame with both players' MF_BTN_* buttons.
void MfEngine_Step(const unsigned short buttons[2]);
// The last frame drawn: MF_SCREEN_W x MF_SCREEN_H RGB565 pixels, or NULL before the first one.
const unsigned short *MfEngine_Frame(void);
// Counts the frames drawn (a new picture when it changes)
unsigned int MfEngine_FrameSerial(void);
// MF_RESULT_* of the session that ended (MF_RESULT_NONE while one runs).
int MfEngine_Result(void);
// Ends the running session as soon as the engine can (MF_RESULT_CANCELLED).
void MfEngine_Cancel(void);
// Why the engine stopped (MF_RESULT_ERROR), or "".
const char *MfEngine_Error(void);
// Adds the fight's sound to a buffer of `frames` stereo 16-bit frames at `rate` Hz.
void MfEngine_MixAudio(short *stereo, int frames, int rate);

#endif // GUARD_MF_ENGINE_H
