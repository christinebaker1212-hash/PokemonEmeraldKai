#ifndef GUARD_S3D_CONTEST_H
#define GUARD_S3D_CONTEST_H

// Moemon Kai: SINNOH's SUPER CONTESTS (src/sinnoh/s3d_contest.c the rules, s3d_contest_ui.c the screens)

#define CONTEST_ENTRANTS 4
#define CONTEST_TURNS 4
#define CONTEST_MAX_WORN 20
#define CONTEST_NOTES 16

// Platinum's competition types (include/constants/contests.h)
enum {
    CONTEST_UNK0,              // VISUAL and ACTING
    CONTEST_UNK1,              // VISUAL and DANCE
    CONTEST_OFFICIAL,          // all three
    CONTEST_PRACTICE_VISUAL,
    CONTEST_VISUAL,
    CONTEST_PRACTICE_DANCE,
    CONTEST_DANCE,
    CONTEST_PRACTICE_ACTING,
    CONTEST_ACTING,
};

enum { ROUND_VISUAL, ROUND_DANCE, ROUND_ACTING, ROUND_COUNT };

// the text banks the contest reads by number (tools/sinnoh/contest_data.py BANKS)
enum { CB_TEXT, CB_EFFECTS, CB_ACTING, CB_VISUAL, CB_DANCE, CB_RESULTS, CB_JUDGE_NAMES, CB_OPPONENT_NAMES };

// Platinum's contest effects (generated/move_contest_effects.txt)
enum {
    CONTEST_EFFECT_NONE,
    CONTEST_EFFECT_FIRST_NEXT_TURN,
    CONTEST_EFFECT_LAST_NEXT_TURN,
    CONTEST_EFFECT_DOUBLED_JUDGE,
    CONTEST_EFFECT_2_HEARTS_WHEN_VOLTAGE_UP,
    CONTEST_EFFECT_BASIC,
    CONTEST_EFFECT_UNIQUE_JUDGE,
    CONTEST_EFFECT_CONSECUTIVE_USE,
    CONTEST_EFFECT_VOLTAGE,
    CONTEST_EFFECT_ALL_SAME_JUDGE,
    CONTEST_EFFECT_LOWERS_VOLTAGE,
    CONTEST_EFFECT_DOUBLE_NEXT_TURN,
    CONTEST_EFFECT_STEAL_VOLTAGE,
    CONTEST_EFFECT_SUPPRESS_VOLTAGE,
    CONTEST_EFFECT_RANDOM_ORDER,
    CONTEST_EFFECT_DOUBLE_FINAL_ACT,
    CONTEST_EFFECT_LOW_VOLTAGE_ADVANTAGE,
    CONTEST_EFFECT_FIRST_PERFORMANCE_ADVANTAGE,
    CONTEST_EFFECT_FINAL_PERFORMANCE_ADVANTAGE,
    CONTEST_EFFECT_NO_VOLTAGE_DOWN,
    CONTEST_EFFECT_TWO_VOLTAGE_IN_A_ROW_ADVANTAGE,
    CONTEST_EFFECT_HIGH_SCORE_LATER_TURN,
    CONTEST_EFFECT_MAX_VOLTAGE_ADVANTAGE,
    CONTEST_EFFECT_PITY_POINTS,
    CONTEST_EFFECT_COUNT
};

// Platinum's accessory numbers (include/constants/accessories.h) the first wins give
#define ACCESSORY_PINK_BARRETTE 72
#define ACCESSORY_RED_BARRETTE 73
#define ACCESSORY_BLUE_BARRETTE 74
#define ACCESSORY_YELLOW_BARRETTE 75
#define ACCESSORY_GREEN_BARRETTE 76
#define ACCESSORY_PINK_BALLOON 77
#define ACCESSORY_RED_BALLOONS 78
#define ACCESSORY_BLUE_BALLOONS 79
#define ACCESSORY_YELLOW_BALLOONS 80
#define ACCESSORY_GREEN_BALLOONS 81
#define ACCESSORY_LACE_HEADDRESS 82
#define ACCESSORY_TOP_HAT 83
#define ACCESSORY_SILK_VEIL 84
#define ACCESSORY_HEROIC_HEADBAND 85
#define ACCESSORY_PROFESSOR_HAT 86
#define ACCESSORY_FLOWER_STAGE 87
#define ACCESSORY_GOLD_PEDESTAL 88
#define ACCESSORY_GLASS_STAGE 89
#define ACCESSORY_AWARD_PODIUM 90
#define ACCESSORY_CUBE_STAGE 91

// the hall's script commands (`sinnoh SINNOH_CONTEST call, a, b`: tools/sinnoh/gen_scripts.py)
enum {
    CC_NEW_RANK_TYPE,     // NewContest's rank and type variables (CC_NEW_MODE_SLOT follows)
    CC_NEW_MODE_SLOT,     // its competition type and party slot: the contest made
    CC_END,               // EndContest: the ribbon, the photo, the TV
    CC_JUDGE_NAME,        // A: the judge (0-2), B: the string slot
    CC_TRAINER_NAME,      // A: the entry number, B: the string slot
    CC_MON_NAME,          // A: the entry number, B: the string slot
    CC_NUMBER,            // A: a number, B: the string slot (BufferContestRegistrationEntryNumber)
    CC_RANK_NAME,         // A: the string slot
    CC_TYPE_NAME,         // A: the string slot
    CC_WINNER_TRAINER,    // A: the string slot
    CC_WINNER_MON,        // A: the string slot
    CC_PLACEMENT,         // A: variable <- the player's place (0 first)
    CC_WINNER_ENTRY,      // A: variable <- the winner's entry number
    CC_PLAYER_ENTRY,      // A: variable <- the player's entry number
    CC_OBJ_GFX,           // A: the entry number, B: variable <- the entrant's overworld sprite
    CC_FAME,              // A: the entry number, B: variable <- its POKéMON's fame
    CC_MODE,              // A: variable <- CONTEST_MODE_OFFICIAL / _PRACTICE
    CC_INFO_RANK_TYPE,    // A, B: variables <- the rank, the type
    CC_INFO_MODE_SLOT,    // A, B: variables <- the competition type, the party slot
    CC_HAS_RIBBON,        // A: variable <- the POKéMON has the rank's ribbon of the type
    CC_RIBBON_NAME,       // A: the string slot
    CC_FIRST_ACCESSORY,   // A: variable <- the accessory a first win gives (0xFFFF: none)
    CC_SKIP_AWARD,        // A: variable <- no award ceremony (practice, a single round, the player didn't win)
    CC_PARTY_MENU,        // A: the rank variable, B: the type's: before the game's party menu
    CC_PARTY_PICKED,      // after it: the choice checked against the rank's ribbon
    CC_PARTY_RESULT,      // A: variable <- the slot (0xFF none), B: variable <- FALSE
    CC_RUN,               // RunContestApplication: the contest's screens, the script waits
    CC_SHOW_PHOTO,        // A: the photo's place (a contest type), B: variable <- TRUE when one was shown
    CC_STUDIO,            // A: the party slot's variable, B: variable <- TRUE when a photo was taken (ScrCmd_0A6)
    CC_STUDIO_PHOTO,      // A: the gallery's frame, B: variable <- 1 when none was shown (ShowDressUpPhoto)
    CC_STUDIO_TITLE,      // A: the word's variable (SetDressUpPhotoTitle)
    CC_BUFFER_WORD,       // A: the string slot, B: the word's variable (BufferCustomMessageWord)
    CC_PICK_BEGIN,        // before the game's party menu (special ChoosePartyMon) for Platinum's SelectMoveTutorPokemon:
                          // VAR_0x8004 kept
    CC_PICK_END,          // after it: the choice (0xFF none) kept for GetSelectedPartySlot, VAR_0x8004 put back
    CC_PICK_RESULT,       // A: variable <- the choice (GetSelectedPartySlot)
    CC_FRIENDSHIP,        // A: variable <- the friendship of B's party POKéMON (GetPartyMonFriendship)
    CC_DEX_CAUGHT,        // A: variable <- the national POKéDEX's caught count as Platinum counts it (1-493, no mythicals)
    CC_DEX_DONE,          // A: variable <- that count reached Platinum's goal (482: CheckNationalDexCompleted)
    CC_PARTY_NON_EGGS,    // A: variable <- the party's POKéMON that aren't EGGS (CountPartyNonEggs)
    CC_LINK_LEAD,         // BECOME LEADER: friends in the room are gathered (s3d_contest_link.c)
    CC_LINK_JOIN,         // JOIN GROUP: the first friend leading takes this game in
    CC_LINK_START,        // A: variable <- 0 the group's contest begins (COMM_CLUB_RET_0), 3 no friend is left
    CC_LINK_END,          // the group left (cancelled by this player)
    CC_PARTY_ANY_RANK,    // A: variable <- TRUE: the next party menu takes any POKéMON (link contests: no ribbon asked)
    CC_LINK_TEXT,         // A: which of the CO-OP link desk's words (LINK_TEXT_*) into gStringVar4
};
enum { LINK_TEXT_WELCOME, LINK_TEXT_EXPLAIN, LINK_TEXT_LAUNCH, LINK_TEXT_LEAD, LINK_TEXT_JOIN, LINK_TEXT_SETUP,
       LINK_TEXT_WAITING };
const u8 *Contest_LinkText(u8 which);

#define PHOTO_STUDIO 5   // (the photos' sixth place: JUBILIFE TV's dress-up studio)

enum { NOTE_PERFORMED, NOTE_SAME_JUDGE, NOTE_APPEAL, NOTE_EFFECT, NOTE_VOLTAGE_UP, NOTE_VOLTAGE_DOWN, NOTE_EXCITED,
       NOTE_WILD, NOTE_IMPRESSED };

struct ContestWorn
{
    u8 accessory, x, y;
};

struct ContestEntrant
{
    u16 species, item, gfx;
    u32 personality, otId;
    u16 moves[4];
    u8 nick[12], trainer[12];
    u8 cond[6], gender, ai, fame, opponent;
    struct ContestWorn worn[CONTEST_MAX_WORN];
    u8 wornCount;
    s8 backdrop;
    u16 condScore, dressPoints;
    u8 condHearts, dressHearts;
    s16 visual, dance, acting;
    s16 round[ROUND_COUNT], bar[ROUND_COUNT], total;
    u8 place;
};

struct ContestPerformer
{
    u16 move;
    u8 effect;
    s16 points, bonus, voltageBonus;
    bool8 suppressed, noVoltageDown;
    u8 orderKind, orderPlace;   // (1 first, 2 last, 3 random: the next turn's place)
};

struct ContestNote
{
    u8 kind, who, other;
    u16 msg, format;
    s16 number;
};

struct ContestActing
{
    u8 turn;
    u8 order[CONTEST_ENTRANTS], judgeOf[CONTEST_ENTRANTS];
    struct ContestPerformer p[CONTEST_ENTRANTS];
    s8 voltage[3], voltageBefore[3];
    s16 total[CONTEST_ENTRANTS], lastPoints[CONTEST_ENTRANTS], before[CONTEST_ENTRANTS];
    u16 lastMove[CONTEST_ENTRANTS], repeatOk[CONTEST_ENTRANTS], nextRepeatOk[CONTEST_ENTRANTS];
    u8 doubled[CONTEST_ENTRANTS], doubleNext[CONTEST_ENTRANTS];
    struct ContestNote note[CONTEST_NOTES];
    u8 notes;
};

struct ContestSong
{
    u8 bpm, steps, near[4];
};

struct SinnohContest
{
    bool8 made, scored, conceded;   // (conceded: the player gave up: last place, Moemon Co-op)
    u8 rank, type, mode, slot, theme, newRank, newType;
    u8 judge[3];
    u8 order[CONTEST_ENTRANTS], ranking[CONTEST_ENTRANTS];
    u32 rng;
    struct ContestEntrant e[CONTEST_ENTRANTS];
    struct ContestActing act;
    bool8 link;    // a contest with friends (CONTEST_MODE_LINK: s3d_contest_link.c)
    u8 me;         // this game's entrant (0 unless with friends)
    u8 humans;     // a bit per entrant played by a person
};

extern struct SinnohContest gSinnohContest;

// SUPER CONTESTS with friends (s3d_contest_link.c): Platinum's link contests over the co-op relay. One game leads and
// sends a seed and everyone's entrant; every game then runs the same contest, the friends' dress-ups, dance steps and
// moves coming over the relay, and waits for them where the rounds meet.
#define CONTEST_ENTRY_BYTES 56
void Contest_PackEntrant(u8 *p, const struct ContestEntrant *e);
void Contest_UnpackEntrant(const u8 *p, struct ContestEntrant *e);
void Contest_FillPlayerEntrant(struct ContestEntrant *e, u8 slot);
bool8 Contest_LinkStarted(void);
void Contest_LinkSetup(void);                 // (Contest_New with friends: their entrants in their seats)
bool8 Contest_IsHuman(u8 who);
void Contest_LinkSendDress(void);
bool8 Contest_LinkDressReady(void);
void Contest_LinkSendStep(u8 round, u8 part, u8 at, u8 dir, u8 grade);
bool8 Contest_LinkTakeStep(u8 *who, u8 *round, u8 *part, u8 *at, u8 *dir, u8 *grade);
void Contest_LinkSendPart(u8 round, u8 part);
bool8 Contest_LinkPartReady(u8 round, u8 part);
void Contest_LinkSendMove(u8 turn, u16 move, u8 judge);
bool8 Contest_LinkMovesReady(u8 turn);
bool8 Contest_LinkTakeMove(u8 who, u8 turn, u16 *move, u8 *judge);
void Contest_LinkLeave(void);
void Contest_LinkCall(u16 call, u16 a, u16 b);
void Contest_LinkOnRelay(u8 from, const u8 *data, u16 len);

void Sinnoh_ContestCall(u16 call, u16 a, u16 b);
bool8 Contest_IsMade(void);
void Contest_New(u8 rank, u8 type, u8 mode, u8 slot);
void Contest_StudioEntrant(u8 slot);
void Contest_StudioDone(bool8 taken);
bool8 Contest_IsPractice(void);
bool8 Contest_HasRound(u8 round);
u16 Contest_Random(void);
u8 *Contest_Slot(u16 slot);
u8 Contest_MoveType(u16 move);
u8 Contest_MoveEffect(u16 move);
s8 Contest_EffectAppeal(u8 effect);
bool8 Contest_EffectMessage(u8 effect, u8 variant, u16 *msg, u16 *format);
void Contest_EffectDescription(u8 effect, u16 *line1, u16 *line2);
u16 Contest_Bank(u8 which);
const u8 *Contest_Text(u8 bank, u16 msg);
u8 Contest_ThemePoints(u8 theme, u16 accessory);
u16 Contest_JudgeName(u8 judge);
u8 Contest_Fame(struct Pokemon *mon, u8 type);
void Contest_ScoreVisual(void);
const struct ContestSong *Contest_Song(u8 *pattern);
u8 Contest_JudgeStep(u16 distance, const u8 near[4], bool8 backup, bool8 leadThere, u8 dir, u8 leadDir);
s8 Contest_StepPoints(u8 grade);
s8 Contest_VoltageChangeFor(u16 move);
bool8 Contest_MoveUsable(u8 id, u16 move);
void Contest_BeginActing(void);
void Contest_Choose(u8 who, u16 move, u8 judge);
void Contest_AiChoose(u8 who, u16 *move, u8 *judge);
void Contest_Perform(u8 place);
void Contest_EndTurn(void);
void Contest_NextTurn(void);
void Contest_ScoreAll(void);

// the screens (s3d_contest_ui.c)
void Contest_RunScenes(void);
bool8 Contest_RunStudio(u8 partySlot);
bool8 Contest_ShowPhoto(u8 type);
bool8 Sinnoh_Contest_IsActive(void);
bool8 Sinnoh_Contest_Step(u16 keys);

// the FASHION CASE and its photos (s3d_fashion.c)
bool8 Sinnoh_CanFitAccessory(u16 accessory, u16 count);
u8 Sinnoh_AccessoryCount(u16 accessory);
bool8 Sinnoh_HasBackdrop(u16 backdrop);
void Sinnoh_ContestKeepPhoto(u8 type, const struct ContestEntrant *e);
const struct ContestEntrant *Sinnoh_ContestPhoto(u8 type);
u16 *Sinnoh_PhotoTitle(u8 type);

// the TV (s3d_tv.c)
void Sinnoh_TvContestResult(struct Pokemon *mon, u8 type, u8 rank, u8 place);

#endif // GUARD_S3D_CONTEST_H
