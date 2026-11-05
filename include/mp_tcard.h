#ifndef GUARD_MP_TCARD_H
#define GUARD_MP_TCARD_H

// Moemon Co-op: the TRAINER CARD's own records ("<save>.card", src/mp/mp_tcard.c): every region's badges (when each
// was won, the lead POKéMON, the friends in that battle, its polish), the BADGE CASE's fading, the card's title and
// team. The save itself never changes layout; the card's badges are read from each region's badge flags
// (mp_region.c MP_Region_BadgeFlag).

#include "mp.h"
#include "mp_region.h"

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

// The card's look (MY CARD): one of Battle Revolution's Battle Pass designs (graphics/moemon/cards/passes.bin), a hue
// for the recolorable ones, an accent color, a frame, the panels behind the text, the holo shine
enum { TCARD_FRAME_NONE, TCARD_FRAME_THIN, TCARD_FRAME_THICK, TCARD_FRAME_GLOW, TCARD_FRAME_DOUBLE, TCARD_FRAME_COUNT };
enum { TCARD_PANEL_LIGHT, TCARD_PANEL_DARK, TCARD_PANEL_GLASS, TCARD_PANEL_NONE, TCARD_PANEL_COUNT };
#define TCARD_ACCENTS 12
struct MpCardStyle
{
    u8 design, accent, frame, panel;
    u16 hue;                                    // 0-359 (recolorable designs)
    u8 holo, tint;                              // tint: the GAME CORNER photo's print (MON_ICON_TINT_*, trainer_card.h)
};

// The POKéMON shown big on the card
struct MpCardShowcase
{
    u16 species;                                // 0: none
    u8 shiny, pad;                              // pad (the card file's): chosen on MY CARD (else the party's first)
    u32 personality, otId;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
};

// The card's signature, drawn as in the DS games: 192x64, a bit a pixel
#define TCARD_SIG_W 192
#define TCARD_SIG_H 64
#define TCARD_SIG_BYTES (TCARD_SIG_W * TCARD_SIG_H / 8)

// The card's records (its STATS page)
enum {
    TCARD_STAT_CATCHES, TCARD_STAT_HATCHED, TCARD_STAT_EVOLVED, TCARD_STAT_BATTLES, TCARD_STAT_TRAINERS,
    TCARD_STAT_WILD, TCARD_STAT_STEPS, TCARD_STAT_FISHED, TCARD_STAT_HOF, TCARD_STAT_CONTESTS, TCARD_STAT_ACHIEVEMENTS,
    TCARD_STAT_TOGETHER, TCARD_STAT_COUNT
};

// A card as the TRAINER CARD and the BADGE CASE show it: the player's own (live records) or a friend's (from their
// card and its co-op add-on, mp_linkx.c)
#define TCARD_TITLE_LEN 24
#define TCARD_STICKERS 3
#define TCARD_STICKER(bits, kind) (((bits) >> ((kind) * 3)) & 7)
struct MpCardView
{
    bool8 own;
    u8 gender;
    char name[MP_NAME_LEN * 2 + 1];             // ASCII
    u16 trainerId;
    u32 money;
    u16 dex;
    u16 hours;
    u8 minutes;
    u32 badges;                                 // bit region * 8 + badge
    u8 polish[REGION_COUNT * TCARD_BADGES_PER_REGION];
    u8 cleared;                                 // bit region: its CHAMPION beaten
    u16 linkWins, linkLosses, trades;
    u8 teamCount;
    u16 team[PARTY_SIZE];
    u8 teamShiny;                               // bit per place
    char title[TCARD_TITLE_LEN];                // "" = none (or not shown)
    struct MpCardStyle style;
    struct MpCardShowcase showcase;
    u32 stats[TCARD_STAT_COUNT];
    u32 regionSeconds[REGION_COUNT];            // play time in each region
    u8 profile[4][16];                          // the card's catchphrase (the profile's words, ASCII)
    bool8 hasSig;
    u8 sig[TCARD_SIG_BYTES];
    bool8 hasPic;
    u16 stickers;                               // FOUR ISLAND's STICKER MAN: 3 bits each, HALL OF FAME, EGGS, VICTORIES
                                                // (0-4; in what was padding: the album's records keep their size)
    u32 pic[64 * 64];                           // the card's picture of them (0xAARRGGBB)
};

void MP_CardView_FillOwn(struct MpCardView *v); // (the picture is taken from the card on screen: trainer_card.c)
u8 MP_CardView_Stars(const struct MpCardView *v, u8 region);   // 0-3: the region's CHAMPION, all 8 badges, all 8 shining
bool8 MP_CardView_HasBadge(const struct MpCardView *v, u8 region, u8 badge);
const char *MP_TCard_TitleText(void);
void MP_TCard_GameToText(char *dst, const u8 *src, int size);   // ASCII, é as UTF-8 (the card screens' font)           // the chosen title as shown (NULL: none or not shown)

// Friends: what you did together (counted on your own game) and the last card you saw of each (the album)
#define TCARD_FRIENDS 16
struct MpTCardFriend
{
    u8 name[MP_NAME_LEN + 1];                   // game text
    u32 otId;
    u32 firstMet;                               // the PC's clock (seconds since 1970)
    u16 wonTogether, vsWins, vsLosses, trades;
    bool8 hasCard;
    struct MpCardView card;                     // the last one seen (the album)
};
void MP_TCard_NoteCoopWin(const u8 *seatPlayers, u8 count, u8 ownSeat);
void MP_TCard_NotePvp(const u8 *oppName, u8 outcome);
void MP_TCard_NoteTrade(u8 playerId);
bool8 MP_TCard_PvpRival(u8 which, u8 *out);   // the friends battled most (0, 1): KANTO's TRAINER FAN CLUB
void MP_TCard_KeepFriendCard(const struct MpCardView *v);
const struct MpTCardFriend *MP_TCard_FriendByName(const char *asciiName);
u16 MP_TCard_PackAddon(u8 *out);               // the card's co-op add-on (mp_linkx.c), at most TCARD_ADDON_MAX bytes
bool8 MP_TCard_UnpackAddon(const u8 *in, u16 len, struct MpCardView *v);
#define TCARD_ADDON_MAX 1810
const struct MpCardStyle *MP_TCard_Style(void);
void MP_TCard_SetStyle(const struct MpCardStyle *style);
const struct MpCardShowcase *MP_TCard_Showcase(void);
void MP_TCard_SetShowcaseFromParty(s8 slot);   // -1: none
const u8 *MP_TCard_Signature(bool8 *has);
void MP_TCard_SetSignature(const u8 *sig, bool8 has);
u8 MP_TCard_AlbumCount(void);
const struct MpTCardFriend *MP_TCard_AlbumFriend(u8 i);   // the friends with a card, the latest first

// FireRed's GAME CORNER photo printer and FOUR ISLAND STICKER MAN on the card (docs/REGIONS_AUDIT.md 124, 126; specials)
u16 MP_TCard_PrintPhoto(void);          // VAR_0x8004 the print (MON_ICON_TINT_*): the party on the card's TEAM page
u16 MP_TCard_StickerBragFlags(void);    // bit 0 HALL OF FAME, 1 EGGS, 2 VICTORIES to brag about; VAR_0x8004-6 the counts
u16 MP_TCard_StickerManMet(void);      // FALSE the first time (FireRed's FLAG_MET_STICKER_MAN)
u16 MP_TCard_StickerBrag(void);         // VAR_0x8009 the kind: its level 1-4 newly stuck on, 0 the same as before, 5 the top one again

#endif // GUARD_MP_TCARD_H
