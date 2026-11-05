// Moemon Kai: SAFFRON's POKéMON TRAINER FAN CLUB, FireRed's (pret/pokefirered src/trainer_fan_club.c), with its own
// record in KANTO's vars (audit case 88): eight members, three of them fans of the player once KANTO's League is beaten,
// fans won by winning battles against friends and lost by losing them, and with play time once there are five or more.
// The names the members talk about are the friends this player battled most (FireRed's link battle records: the
// TRAINER CARD's records here), or the rival, LT. SURGE and KOGA when there are none. Emerald's own LILYCOVE club
// (field_specials.c) keeps its own record.
#include "global.h"
#include "event_data.h"
#include "random.h"
#include "string_util.h"
#include "battle.h"
#include "mp_region.h"
#include "mp_tcard.h"
#include "constants/kai_regions.h"

const u8 *MP_KantoRivalOwnName(void);   // (qz_specials.c)

#define MEMBERS 8
enum { MEMBER1, MEMBER2, MEMBER3, MEMBER4, MEMBER5, MEMBER6, MEMBER7, MEMBER8 };

// FireRed's struct TrainerFanClub in VAR_FANCLUB_FAN_COUNTER: timer:7, gotInitialFans:1, then a byte of fans
#define FANS() (VarGet(VAR_KANTO_FANCLUB_FANS) >> 8)
#define GOT_FIRST() (VarGet(VAR_KANTO_FANCLUB_FANS) & 0x80)

static void SetFans(u8 fans)
{
    VarSet(VAR_KANTO_FANCLUB_FANS, (VarGet(VAR_KANTO_FANCLUB_FANS) & 0xFF) | (fans << 8));
}

static u8 NumFans(void)
{
    u8 i, n = 0, fans = FANS();
    for (i = 0; i < MEMBERS; i++)
        n += (fans >> i) & 1;
    return n;
}

static void GainRandomFan(void)
{
    static const u8 sOrder[] = {MEMBER2, MEMBER4, MEMBER6, MEMBER1, MEMBER8, MEMBER7, MEMBER5, MEMBER3};
    u8 i, idx = 0, fans = FANS();
    for (i = 0; i < MEMBERS; i++)
    {
        if (!((fans >> sOrder[i]) & 1))
        {
            idx = i;
            if (Random() % 2)
            {
                SetFans(fans | (1 << sOrder[i]));
                return;
            }
        }
    }
    SetFans(fans | (1 << sOrder[idx]));
}

static void LoseRandomFan(void)
{
    static const u8 sOrder[] = {MEMBER6, MEMBER7, MEMBER4, MEMBER8, MEMBER5, MEMBER2, MEMBER1, MEMBER3};
    u8 i, idx = 0, fans = FANS();
    if (NumFans() == 1)
        return;
    for (i = 0; i < MEMBERS; i++)
    {
        if ((fans >> sOrder[i]) & 1)
        {
            idx = i;
            if (Random() % 2)
            {
                SetFans(fans & ~(1 << sOrder[i]));
                return;
            }
        }
    }
    SetFans(fans & ~(1 << sOrder[idx]));
}

// FireRed's TryLoseFansFromPlayTime: one fan every 12 hours of play while there are five or more
void MP_FanClub_TryLoseFansFromPlayTime(void)
{
    u8 i;
    u16 hours = gSaveBlock2Ptr->playTimeHours;
    if (hours >= 999)
        return;
    for (i = 0; ; i++)
    {
        if (NumFans() < 5)
        {
            VarSet(VAR_KANTO_FANCLUB_TIMER, hours);
            break;
        }
        if (i == MEMBERS || hours - VarGet(VAR_KANTO_FANCLUB_TIMER) < 12)
            break;
        LoseRandomFan();
        VarSet(VAR_KANTO_FANCLUB_TIMER, VarGet(VAR_KANTO_FANCLUB_TIMER) + 12);
    }
}

// The club's map on entering (its ON_TRANSITION): before KANTO's League the four who join later stay away; once it's
// beaten, FireRed's game clear (Script_UpdateTrainerFanClubGameClear at the HALL OF FAME): three fans and the first
// meeting (scene 1). Saves that beat it before this club came get theirs on their first visit.
void MP_FanClub_Enter(void)
{
    static const u16 sLater[] = {FLAG_KANTO_HIDE_FANCLUB_BLACK_BELT, FLAG_KANTO_HIDE_FANCLUB_ROCKER,
                                 FLAG_KANTO_HIDE_FANCLUB_WOMAN, FLAG_KANTO_HIDE_FANCLUB_BEAUTY};
    u32 i;
    if (GOT_FIRST())
        return;
    if (!MP_Region_Cleared(REGION_KANTO))
    {
        for (i = 0; i < ARRAY_COUNT(sLater); i++)
            FlagSet(sLater[i]);
        return;
    }
    VarSet(VAR_KANTO_FANCLUB_FANS, 0x80 | ((1 << MEMBER1 | 1 << MEMBER2 | 1 << MEMBER3) << 8));
    VarSet(VAR_KANTO_FANCLUB_TIMER, gSaveBlock2Ptr->playTimeHours);
    for (i = 0; i < ARRAY_COUNT(sLater); i++)
        FlagClear(sLater[i]);
    VarSet(VAR_KANTO_FANCLUB_SCENE, 1);
}

// FireRed's UpdateTrainerFansAfterLinkBattle, after a battle against a friend (mp_battle.c)
void MP_FanClub_AfterPvp(u8 outcome)
{
    if (VarGet(VAR_KANTO_FANCLUB_SCENE) != 2)
        return;
    if (GOT_FIRST())
    {
        MP_FanClub_TryLoseFansFromPlayTime();
        VarSet(VAR_KANTO_FANCLUB_TIMER, gSaveBlock2Ptr->playTimeHours);
    }
    if (outcome == B_OUTCOME_WON)
        GainRandomFan();
    else
        LoseRandomFan();
}

u16 MP_FanClub_NumFans(void)
{
    return NumFans();
}

bool16 MP_FanClub_IsFan(void)
{
    return (FANS() >> gSpecialVar_0x8004) & 1;
}

// FireRed's Script_BufferFanClubTrainerName: the trainer member VAR_0x8004 talks about into STR_VAR_1
void MP_FanClub_BufferTrainerName(void)
{
    static const u8 sLtSurge[] = _("LT. SURGE");
    static const u8 sKoga[] = _("KOGA");
    u8 link = 0, npc = 0;
    switch (gSpecialVar_0x8004)
    {
    case MEMBER5: npc = 1; break;
    case MEMBER6: link = 1; break;
    case MEMBER7: npc = 2; link = 1; break;
    }
    if (MP_TCard_PvpRival(link, gStringVar1))
        return;
    if (npc == 1)
        StringCopy(gStringVar1, sLtSurge);
    else if (npc == 2)
        StringCopy(gStringVar1, sKoga);
    else
        StringCopy(gStringVar1, MP_KantoRivalOwnName());
}
