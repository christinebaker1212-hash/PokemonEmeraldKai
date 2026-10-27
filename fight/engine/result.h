#ifndef MUGEN_RESULT_H
#define MUGEN_RESULT_H

/* Survival result screen (result.c). */

/* The state of the result screen loop (ScreenLoop.state, 0xC bytes). */
typedef struct ResultState {
    struct Player *p1;          /* 0x00 the human team's players */
    struct Player *p2;          /* 0x04 */
    int wins;                   /* 0x08 matches won (shown by winstext) */
} ResultState;
MUGEN_ASSERT_SIZE(ResultState, 0x0C);

/* States of the players on the result screen. */
#define RESULT_STATE_LOSE   170
#define RESULT_STATE_WIN    180

#endif /* MUGEN_RESULT_H */