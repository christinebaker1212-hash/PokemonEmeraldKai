#ifndef GUARD_MP_TCARD_H
#define GUARD_MP_TCARD_H

// Moemon Co-op: the TRAINER CARD's own records ("<save>.card", src/mp/mp_tcard.c): every region's badges (when each
// was won, the lead POKéMON, the friends in that battle, its polish), the BADGE CASE's fading, the card's title and
// team. The save itself never changes layout; the card's badges are read from each region's badge flags
// (mp_region.c MP_Region_BadgeFlag).

#include "mp.h"

#define TCARD_BADGES_PER_REGION 8
#define TCARD_PARTNERS 3

// Platinum's badge polish (pret/pokeplatinum include/trainer_case.h): a badge starts at 140, loses 10 a day, and is
// FILTHY under 100, DIRTY under 140, shines with 2 sparkles from 170 and 4 from 190, at most 199
#define TCARD_POLISH_DIRTY      100
#define TCARD_POLISH_NORMAL     140
#define TCARD_POLISH_2_SPARKLES 170
#define TCARD_POLISH_4_SPARKLES 190
#define TCARD_POLISH_MAX        199

enum {
    TCARD_POLISH_LEVEL_FILTHY,
    TCARD_POLISH_LEVEL_DIRTY,
    TCARD_POLISH_LEVEL_NORMAL,
    TCARD_POLISH_LEVEL_2_SPARKLES,
    TCARD_POLISH_LEVEL_4_SPARKLES,
};

// OPTIONS: how badges lose their shine (PLATINUM: as Platinum's do, 10 a day; SLOW: a third of that)
enum {
    TCARD_FADE_PLATINUM,
    TCARD_FADE_SLOW,
    TCARD_FADE_OFF,
    TCARD_FADE_COUNT,
};

struct MpTCardBadge
{
    u32 when;                                   // the PC's clock (seconds since 1970) it was won; 0 = before records
    u16 leadSpecies;                            // the first POKéMON then
    u8 leadLevel;
    u8 polish;                                  // 0-199 (TCARD_POLISH_*)
    u8 partnerCount;                            // friends in the battle that won it
    u8 partners[TCARD_PARTNERS][MP_NAME_LEN + 1];   // their names (game text)
};

struct MpTCardMon
{
    u16 species;                                // 0: an empty place
    u8 level;
    u8 shiny;
    u32 personality;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
};

void MP_TCard_Update(void);                     // every frame: new badges, a day's fading, co-op partners seen
void MP_TCard_Forget(void);                     // the save file changed: read its card file again
bool8 MP_TCard_HasBadge(u8 region, u8 badge);
const struct MpTCardBadge *MP_TCard_Badge(u8 region, u8 badge);
u8 MP_TCard_PolishLevel(u8 polish);             // TCARD_POLISH_LEVEL_*
bool8 MP_TCard_Rub(u8 region, u8 badge);        // one rub of the cloth: TRUE when it reached a new level
void MP_TCard_SaveNow(void);                    // after polishing (the case saves when it closes)
u8 MP_TCard_Fade(void);
void MP_TCard_SetFade(u8 fade);
bool8 MP_TCard_ShowTitle(void);
void MP_TCard_SetShowTitle(bool8 show);
u16 MP_TCard_Title(void);                       // an achievement's number + 1 (0: none chosen)
void MP_TCard_SetTitle(u16 title);
u8 MP_TCard_TeamCount(void);
const struct MpTCardMon *MP_TCard_TeamMon(u8 i);
void MP_TCard_SetTeamFromParty(const u8 *slots, u8 count);   // the card's team: these party slots, as they are now
void MP_TCard_DebugDays(s32 days);              // tests: as if this many days had passed on the PC's clock

#endif // GUARD_MP_TCARD_H
