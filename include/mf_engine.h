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

#define MF_PACK_MAX 64

struct MfSession
{
    int mode;            // MF_MODE_*
    unsigned int seed;   // the match RNG's seed (the same on both sides online)
    char cabinet[32];    // section of mugen/cabinets.ini with the rules
    int rollback;        // keep the fight's frames to go back to (online: MfEngine_RollBack)
    // Online: the characters (select list order) and stages (struct MfPack order) one of the players lacks:
    // greyed out on the select screen, never picked at random
    unsigned char charOff[MF_PACK_MAX];
    unsigned char stageOff[MF_PACK_MAX];
    char cabinetText[2048];   // the cabinet's rules as text (MfEngine_CabinetText: online, the answering player's), "" the file
};

// What a player's fight files are, to compare before an online fight: hashes of every file each part uses.
// core: the engine's data, the screenpack (with its select list) and the fonts, which have to match for a fight
// at all; chars in the select list's order; stages in the order the cabinet could pick them (its list, or
// [ExtraStages] and then each character's own). Reads the whole pack (~80 MB): a fraction of a second.
struct MfPack
{
    unsigned int core;
    unsigned char chars, stages;
    unsigned int charHash[MF_PACK_MAX];
    unsigned int stageHash[MF_PACK_MAX];
};
int MfEngine_PackSummary(const char *cabinet, struct MfPack *out);
// The cabinet's section of mugen/cabinets.ini as text, with its [name] line; 0 when there's none
int MfEngine_CabinetText(const char *cabinet, char *out, int size);

// Starts a session (loading the engine the first time). Returns 0 when the engine can't run.
int MfEngine_Begin(const struct MfSession *session);
// A session is running (select, versus, fight, victory).
int MfEngine_IsActive(void);
// Runs one frame with both players' MF_BTN_* buttons. 0 when the engine is still busy with the last one
// (loading): the frame wasn't started, ask again.
int MfEngine_Step(const unsigned short buttons[2]);
// The engine can take a step now
int MfEngine_Ready(void);
// Frames done in this session (the frame numbers rollback uses)
unsigned int MfEngine_FrameCount(void);

// Rollback (sessions with rollback set). While the match is on screen every frame is kept, up to 9 back:
// MfEngine_Kept says whether the last frame was (and the oldest one there is). MfEngine_RollBack puts the fight
// back as it was after that frame; MfEngine_StepQuiet then runs frames again without drawing them, their
// sounds and music only where the first run didn't have them. The select and VS screens aren't kept: steps
// there have to wait for both players' buttons.
int MfEngine_Kept(unsigned int *oldest);
int MfEngine_RollBack(unsigned int frame);
void MfEngine_StepQuiet(const unsigned short buttons[2]);
// A hash of what both players' games must agree on after the last frame (players, timer, RNGs)
unsigned int MfEngine_Hash(void);
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
