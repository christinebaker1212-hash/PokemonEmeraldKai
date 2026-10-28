/* Moemon Rhythm: one player's song: judgment, combo, score, life (docs/RHYTHM_DECISIONS.md #2-#3, from Beware's
 * DDR EXTREME build of StepMania 3.9). Times are the song's seconds (RhNote.time). Plain C, no game headers. */
#ifndef GUARD_RHYTHM_PLAY_H
#define GUARD_RHYTHM_PLAY_H

#include "rhythm/chart.h"

/* better is higher */
enum { RH_J_NONE, RH_J_MISS, RH_J_BOO, RH_J_GOOD, RH_J_GREAT, RH_J_PERFECT, RH_J_MARVELOUS, RH_J_COUNT };
enum { RH_HOLD_NONE, RH_HOLD_ACTIVE, RH_HOLD_OK, RH_HOLD_NG };
enum { RH_GRADE_AAAA, RH_GRADE_AAA, RH_GRADE_AA, RH_GRADE_A, RH_GRADE_B, RH_GRADE_C, RH_GRADE_D, RH_GRADE_E };

#define RH_WINDOW_MARVELOUS 0.016666
#define RH_WINDOW_PERFECT   0.033333
#define RH_WINDOW_GREAT     0.083
#define RH_WINDOW_GOOD      0.120
#define RH_WINDOW_BOO       0.158333
#define RH_WINDOW_OK        0.250
#define RH_WINDOW_MINE      0.083

/* the options that change the notes (the screen's are the drawing's) */
enum { RH_TURN_OFF, RH_TURN_MIRROR, RH_TURN_LEFT, RH_TURN_RIGHT, RH_TURN_SHUFFLE };
struct RhNoteOptions
{
    uint8_t turn;               /* RH_TURN_* */
    uint8_t little;             /* only the 4th notes */
    uint8_t noFreeze;           /* freeze arrows are steps */
    uint32_t shuffleSeed;
};

/* what just happened, for the screen (and the other player online) */
enum { RH_EV_TAP, RH_EV_ROW, RH_EV_HOLD, RH_EV_MINE, RH_EV_FAIL };
struct RhEvent
{
    uint8_t kind, judge, col;   /* judge: RH_J_* (ROW: the row's), or RH_HOLD_* */
    int32_t note;               /* the note's index */
    float offset;               /* seconds early (+) or late (-) */
};
#define RH_EVENTS 64

struct RhPlay
{
    const struct RhSong *song;
    struct RhNote *notes;       /* the chart after the note options (owned) */
    int count;
    uint8_t *judge;             /* each note's RH_J_* (taps, heads) */
    uint8_t *hold;              /* each freeze's RH_HOLD_* */
    float *holdLife;            /* 1 while held, down to 0 in RH_WINDOW_OK when let go */
    float *offset;              /* when it was stepped on (+ early) */
    int first;                  /* notes before this one are all done */
    int meter;
    uint8_t showMarvelous;
    /* the counts: rows by their judgment, freezes */
    int rowCount[RH_J_COUNT];
    int tapCount[RH_J_COUNT];
    int ok, ng, minesHit;
    int combo, maxCombo;
    int rowsTotal, holdsTotal, holdGroups;
    /* score (DDR MAX2 / EXTREME) */
    int32_t score;
    int32_t maxPoints, pointBonus;
    int scoreEvents, scoreEventsTotal;
    /* life */
    float life;
    int missCombo, comboToRegain;
    uint8_t failed, finished;
    double failTime;
    /* dance points (the grade) */
    int dpActual, dpPossible;
    /* timing (ms early + / late -), for the results' numbers */
    double offsetSum, offsetAbsSum;
    int offsetN;
    int fastCount, slowCount;
    /* events since the screen last took them */
    struct RhEvent events[RH_EVENTS];
    int eventCount;
    uint8_t held[RH_COLS];
};

int RhPlay_Begin(struct RhPlay *p, const struct RhSong *song, const struct RhChart *chart, int meter,
                 const struct RhNoteOptions *opt, int showMarvelous);
void RhPlay_End(struct RhPlay *p);
/* A step at time t (the music's clock when the key went down) */
void RhPlay_Press(struct RhPlay *p, int col, double t);
void RhPlay_Release(struct RhPlay *p, int col, double t);
/* Each frame: misses, freezes, mines, the end */
void RhPlay_Update(struct RhPlay *p, double t, double dt);
int RhPlay_Grade(const struct RhPlay *p);
/* full combo: 0 none, 1 GREAT or better, 2 PERFECT or better, 3 MARVELOUS */
int RhPlay_FullCombo(const struct RhPlay *p);
const char *RhPlay_JudgeName(int judge);

#endif
