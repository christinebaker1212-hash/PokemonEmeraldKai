/* Yu-Gi-Oh! duelists on the map (src/ygo/ygo_field.c): who they are, the deck they bring, what a win over them is
 * worth, handed to the card screens (src/ygo/ygo_ui.c). */
#ifndef MOEMON_YGO_NPC_H
#define MOEMON_YGO_NPC_H
#include <stdint.h>
#include "ygo/ygo.h"

struct YgoOpponent
{
    char name[28];          /* "PSYCHIC EDWARD", "SIDNEY" */
    char title[24];         /* "ELITE FOUR"... ("" for most) */
    uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];   /* 0-terminated */
    uint32_t key;           /* the daily reward's (map + person, or the special duelist) */
    uint16_t music, signature;   /* signature: the card a first win hands over (0 none) */
    uint8_t special, set, online, lesson;   /* set: the booster a win gives (YGO_SET_COUNT: any) */
    uint8_t portraitW, portraitH;
    uint32_t portrait[32 * 32];  /* their sprite as they faced you, ARGB */
};
#define YGO_SPECIAL_NONE 0xFF

/* The card screens' side (ygo_ui.c) */
void YgoUi_OpenChallenge(const struct YgoOpponent *opponent);
/* The CARD CLUB's Yu-Gi-Oh! duelist: the four Starter Decks, the lesson, the counter (a script waits) */
void YgoUi_OpenStarter(void);
void YgoUi_OpenLesson(void);
void YgoUi_OpenCounter(void);
#endif
