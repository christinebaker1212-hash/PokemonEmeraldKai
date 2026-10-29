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
    uint8_t tcgSpecial;     /* the ELITE FOUR: their old Pokemon card deck (TCG_SPECIAL_*), whose promo a win hands over */
    uint8_t portraitW, portraitH;
    uint32_t portrait[64 * 64]; /* ARGB: their sprite as they faced you (up to 32x32), or 64x64: their battle picture */
};
#define YGO_SPECIAL_NONE 0xFF

/* The card screens' side (ygo_ui.c) */
void YgoUi_OpenChallenge(const struct YgoOpponent *opponent);
/* The CARD CLUB's Yu-Gi-Oh! duelist: the four Starter Decks, the lesson, the counter (a script waits) */
void YgoUi_OpenStarter(void);
void YgoUi_OpenLesson(void);
void YgoUi_OpenCounter(void);
void YgoUi_OpenGameCenter(void);   /* the GAME CENTER clerk: SINGLES and DECKS for money, then the counter */

/* Who plays what (src/ygo/ygo_field.c): 0 the Pokemon cards (or nothing), YGO_PLAYS_YGO Yu-Gi-Oh! (*line: what they
 * say; *refused: not now), YGO_PLAYS_BOTH they ask which (opponent filled for Yu-Gi-Oh!). clubStaff: the CARD CLUB's */
#define YGO_PLAYS_YGO 1
#define YGO_PLAYS_BOTH 2
struct ObjectEvent;
int Ygo_FieldPick(struct ObjectEvent *npc, const uint8_t *script, uint32_t hash, uint32_t key, int clubStaff,
                  struct YgoOpponent *opponent, const uint8_t **line, int *refused);
#endif
