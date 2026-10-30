/* Moemon Rhythm: the DANCE machine on the GAME CENTER's 3F (docs/RHYTHM_DECISIONS.md). While it's on, its screen
 * is the game's picture (Rhythm_Step draws it), its sound replaces the game's (Rhythm_MixAudio) and the game waits.
 * The map side (src/mp/mp_rhythm.c) starts a credit and hears how it ended. */
#ifndef GUARD_RHYTHM_H
#define GUARD_RHYTHM_H

#include <stdint.h>

#define RHYTHM_STAGES 4

/* a credit */
struct RhythmBegin
{
    uint8_t side;               /* our pad: 0 the left (1P), 1 the right (2P) */
    int8_t peer;                /* the friend on the other pad (player id), -1 alone */
    uint8_t picker;             /* we choose the songs (1P, or whoever is left) */
    char peerName[16];          /* ASCII */
    char myName[16];
};

int Rhythm_IsActive(void);
/* A frame while it's on: draws into the game's 240x160 picture (15-bit). 0 when it isn't on */
int Rhythm_Step(uint16_t *frame, uint16_t gbaKeys);
void Rhythm_MixAudio(float *stereo, int frames);
/* every SDL event (sdl2.c ProcessEvents), so steps are timed by when they happened */
void Rhythm_OnEvent(const void *sdlEvent);
int Rhythm_Begin(const struct RhythmBegin *begin);
/* how the last credit went: stages cleared, and whether it ended early (friend gone, gave up) */
int Rhythm_LastStagesCleared(void);
/* online: relay messages for the machine's session (mp_rhythm.c passes them on) */
void Rhythm_OnNet(uint8_t from, const uint8_t *data, uint16_t len);
/* the friend left the room */
void Rhythm_PeerGone(uint8_t id);

/* provided by the map side (mp_rhythm.c) */
void MP_Rhythm_Send(int8_t to, const uint8_t *data, uint16_t len);
void MP_Rhythm_Ended(void);
void MP_Rhythm_Dancing(uint8_t arrows);     /* the arrows held now (bits 0-3), for friends watching */
void MP_Rhythm_SubmitScore(uint32_t chart, const char *title, int diff, int meter, uint32_t score, int grade,
                           int maxCombo, int fullCombo, const int *judges);
/* the server's records for a chart: name/score pairs best first (count 0: none known yet) */
int MP_Rhythm_Records(uint32_t chart, char names[][16], uint32_t *scores, uint8_t *grades, int max);
void MP_Rhythm_AskRecords(uint32_t chart);
/* a stage's end, for the achievements and the player's own bests */
struct RhythmStage
{
    const char *title;
    uint32_t chart, song;       /* the chart's records key, the song's key */
    uint8_t diff, meter, cleared, grade, fullCombo, speed, appearance, marvelous;
    uint8_t record;             /* beat the best score known for the chart */
    int8_t versus;              /* -1 alone, 0 lost to the friend, 1 beat them */
    uint16_t maxCombo;
    uint32_t score;
};
void MP_Rhythm_StageDone(const struct RhythmStage *stage);
/* the credit's end: stages played, all full combos, all AAA */
void MP_Rhythm_CreditDone(int stages, int allFullCombo, int allAaa);
/* the options kept with the save */
void MP_Rhythm_LoadOptions(uint8_t *opts, int size);
void MP_Rhythm_SaveOptions(const uint8_t *opts, int size);

#endif
