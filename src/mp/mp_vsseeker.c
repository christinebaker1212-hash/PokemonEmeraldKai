// Moemon Kai: FireRed's VS SEEKER (pokefirered src/vs_seeker.c; audit cases 17 and 24) and the original games' trainer
// teams (case 57). The data is src/data/qz_original_teams.h (tools/quetzal_port/gen_original_teams.py).
//
// The VS SEEKER charges over 100 steps in the BAG; used on the field it beeps, and every trainer in view answers: "!"
// not battled yet, "X" not now, or "!!" wants a rematch (FireRed's 70%). Those trainers then challenge the player again
// (when talked to: FireRed's raise their hand) until the player has walked 100 steps or left the map, as in FireRed; the rematch team is
// FireRed's next tier the story allows (TryGetRematchTrainerIdGivenGameState: the VS SEEKER, CELADON, FUCHSIA, the
// LEAGUE, SEVII's network machine). Each tier keeps its own "beaten" flag (KANTO_VS_TIER_FLAGS_*), as FireRed's tier
// trainers do. The trainers' scripts are FireRed's already (ShouldTryRematchBattle, trainerbattle_rematch): battle_setup.c
// asks this file for KANTO's trainers.
#include <stdio.h>
#include "global.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "item.h"
#include "item_menu.h"
#include "menu.h"
#include "random.h"
#include "script.h"
#include "script_movement.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/event_object_movement.h"
#include "constants/field_effects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/kai_regions.h"
#include "constants/qz_port.h"
#include "constants/songs.h"
#include "constants/trainer_types.h"
#include "constants/trainers.h"
#include "mp_region.h"

#include "../data/qz_original_teams.h"

#define VS_SEEKER_STEPS 100
#define VIEW_X 7
#define VIEW_Y 5

void MP_StartVsSeekerIcon(bool8 rematch);   // trainer_see.c: FireRed's "!!" / "X"

static EWRAM_DATA bool8 sReady[256] = {0};   // by local id: wants a rematch (FireRed's trainerRematches)
static EWRAM_DATA bool8 sAnyReady = FALSE;
static EWRAM_DATA u8 sChargeSteps = 0;        // steps since the last use that found rematches (FireRed's charging)
static EWRAM_DATA u16 sReadyMap = 0;          // (group << 8 | num) + 1 of the map the answers belong to
static EWRAM_DATA u16 sRematchTrainer = 0;    // the rematch being fought: its trainer and tier
static EWRAM_DATA u8 sRematchTier = 0;
static EWRAM_DATA bool8 sTestAlwaysYes = FALSE;   // (tests: every trainer with a rematch wants it)

static const u8 sText_BatteryNotCharged[] = _("The battery isn't charged enough.\pNo. of steps required to fully\ncharge the battery: {STR_VAR_1}{PAUSE_UNTIL_PRESS}");
static const u8 sText_NoTrainersWithinRange[] = _("There are no TRAINERS within range\nwho can battle…\pThe VS SEEKER was turned off.{PAUSE_UNTIL_PRESS}");
static const u8 sText_TrainersNotReady[] = _("The other TRAINERS don't appear\nto be ready for battle.\pLet's wait till later.{PAUSE_UNTIL_PRESS}");

// ---------------------------------------------------------------------------------------------------- the data
static const struct QzOriginalTeam *OriginalTeam(u16 trainerNum)
{
    s32 lo = 0, hi = ARRAY_COUNT(sQzOriginalTeams) - 1;
    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        if (sQzOriginalTeams[mid].trainerNum == trainerNum)
            return &sQzOriginalTeams[mid];
        if (sQzOriginalTeams[mid].trainerNum < trainerNum)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

static const struct KantoRematch *Rematch(u16 trainerNum)
{
    s32 lo = 0, hi = ARRAY_COUNT(sKantoRematches) - 1;
    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        if (sKantoRematches[mid].trainerNum == trainerNum)
            return &sKantoRematches[mid];
        if (sKantoRematches[mid].trainerNum < trainerNum)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

static bool8 TierBeaten(const struct KantoRematch *r, u8 tier)
{
    u16 flag = r->tier[tier - 1].beatenFlag;
    return FlagGet(flag == 0xFFFF ? TRAINER_FLAG(r->trainerNum) : flag);
}

// FireRed's GetNextAvailableRematchTrainer: the first tier not beaten yet (SKIPs passed over), else the last
static u8 NextTier(const struct KantoRematch *r)
{
    u8 j;
    for (j = 1; j <= r->count; j++)
    {
        if (r->tier[j - 1].party == NULL || TierBeaten(r, j))
            continue;
        return j;
    }
    return r->count;
}

// FireRed's TryGetRematchTrainerIdGivenGameState: a tier waits for its point in the story; before it, the tier below
static u8 GateTier(const struct KantoRematch *r, u8 tier)
{
    bool8 ok;
    switch (tier)
    {
    case 1:  ok = CheckBagHasItem(ITEM_VS_SEEKER, 1); break;
    case 2:  ok = FlagGet(FLAG_QZ_0x2234); break;   // (CELADON's fly point: FireRed's FLAG_WORLD_MAP_CELADON_CITY)
    case 3:  ok = FlagGet(FLAG_QZ_0x223E); break;   // (FUCHSIA's)
    case 4:  ok = MP_Region_Cleared(REGION_KANTO); break;
    case 5:  ok = FlagGet(FLAG_QZ_0x220A); break;   // (SEVII's network machine: FLAG_SYS_CAN_LINK_WITH_RS)
    default: ok = TRUE; break;
    }
    if (ok)
        return tier;
    while (--tier != 0)
        if (r->tier[tier - 1].party != NULL)
            return tier;
    return 0;
}

// mp_region.c TrainerDataFor: the team a JOHTO / KANTO trainer fights (FALSE: Quetzal's / the GENERATION's)
bool8 MP_OriginalTeam(u16 trainerNum, bool8 modern, u8 *partyFlags, u8 *partySize, const void **party)
{
    const struct QzOriginalTeam *o;
    if (sRematchTier != 0 && sRematchTrainer == trainerNum)
    {
        const struct KantoRematch *r = Rematch(trainerNum);
        if (r != NULL && r->tier[sRematchTier - 1].party != NULL)
        {
            *partyFlags = r->tier[sRematchTier - 1].partyFlags;
            *partySize = r->tier[sRematchTier - 1].partySize;
            *party = r->tier[sRematchTier - 1].party;
            return TRUE;
        }
    }
    if (modern || (o = OriginalTeam(trainerNum)) == NULL)
        return FALSE;
    *partyFlags = o->partyFlags;
    *partySize = o->partySize;
    *party = o->party;
    return TRUE;
}

// ---------------------------------------------------------------------------------------------------- the answers
static u16 MapKey(void)
{
    return ((gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum) + 1;
}

static void ClearAnswers(void)
{
    memset(sReady, 0, sizeof(sReady));
    sAnyReady = FALSE;
    sChargeSteps = 0;
}

static void CheckMap(void)
{
    if (sAnyReady && sReadyMap != MapKey())
        ClearAnswers();   // (FireRed's MapResetTrainerRematches)
}

// battle_setup.c IncrementRematchStepCounter, every step: the battery charges in the BAG, and the answers last 100 steps
void MP_VsSeeker_Step(void)
{
    u16 v = VarGet(VAR_KANTO_VS_SEEKER);
    if (v < VS_SEEKER_STEPS && CheckBagHasItem(ITEM_VS_SEEKER, 1))
        VarSet(VAR_KANTO_VS_SEEKER, v + 1);
    CheckMap();
    if (sAnyReady && ++sChargeSteps >= VS_SEEKER_STEPS)
        ClearAnswers();
}

bool8 MP_VsSeeker_IsKantoTrainer(u16 trainerNum)
{
    return Rematch(trainerNum) != NULL;
}

// FireRed's ShouldTryRematchBattle: a beaten trainer of the table takes the rematch path (which asks IsTrainerReadyForRematch)
bool8 MP_VsSeeker_ShouldTry(u16 trainerNum)
{
    return FlagGet(TRAINER_FLAG(trainerNum));
}

bool8 MP_VsSeeker_Ready(void)
{
    CheckMap();
    return sAnyReady && sReady[(u8)gSpecialVar_LastTalked];
}

// TRAINER_BATTLE_REMATCH: the tier this rematch fights (FireRed's GetRematchTrainerId)
void MP_VsSeeker_PrepareRematch(u16 trainerNum)
{
    const struct KantoRematch *r = Rematch(trainerNum);
    sRematchTrainer = trainerNum;
    sRematchTier = r != NULL ? GateTier(r, NextTier(r)) : 0;
}

// The rematch was won: its tier counts as beaten, and the trainer is done until the next VS SEEKER
void MP_VsSeeker_RematchWon(void)
{
    const struct KantoRematch *r = Rematch(sRematchTrainer);
    if (r != NULL && sRematchTier != 0 && r->tier[sRematchTier - 1].beatenFlag != 0xFFFF)
        FlagSet(r->tier[sRematchTier - 1].beatenFlag);
    sReady[(u8)gSpecialVar_LastTalked] = FALSE;
    sRematchTier = 0;
}

void MP_VsSeeker_BattleEnded(void)
{
    sRematchTier = 0;
}

// ---------------------------------------------------------------------------------------------------- using it
struct Seen { u8 objectEventId; u8 localId; u16 trainerNum; };
static EWRAM_DATA struct Seen sSeen[OBJECT_EVENTS_COUNT] = {0};
static EWRAM_DATA u8 sSeenCount = 0;

static const u8 sMovement_WantsRematch[] = {MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_DOWN, MOVEMENT_ACTION_STEP_END};

// GatherNearbyTrainerInfo + IsTrainerVisibleOnScreen: the trainers in view (their script's trainerbattle names them)
static void Gather(void)
{
    s16 px, py;
    u32 i;
    PlayerGetDestCoords(&px, &py);
    sSeenCount = 0;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *o = &gObjectEvents[i];
        const u8 *script;
        if (!o->active || o->isPlayer || (o->trainerType != TRAINER_TYPE_NORMAL && o->trainerType != TRAINER_TYPE_BURIED))
            continue;
        if (o->currentCoords.x < px - VIEW_X || o->currentCoords.x > px + VIEW_X
         || o->currentCoords.y < py - VIEW_Y || o->currentCoords.y > py + VIEW_Y)
            continue;
        script = GetObjectEventScriptPointerByObjectEventId(i);
        if (script == NULL || script[0] != 0x5C)   // (SCR_OP_TRAINERBATTLE)
            continue;
        sSeen[sSeenCount].objectEventId = i;
        sSeen[sSeenCount].localId = o->localId;
        sSeen[sSeenCount].trainerNum = script[2] | (script[3] << 8);
        sSeenCount++;
    }
}

// FireRed's GetRematchableTrainerLocalId: someone in view not battled yet, or with a rematch to give
static bool8 AnyoneToAnswer(void)
{
    u32 i;
    for (i = 0; i < sSeenCount; i++)
    {
        const struct KantoRematch *r = Rematch(sSeen[i].trainerNum);
        if (!FlagGet(TRAINER_FLAG(sSeen[i].trainerNum)) || (r != NULL && NextTier(r) != 0))
            return TRUE;
    }
    return FALSE;
}

static void Icon(const struct Seen *s, u8 kind)   // 0 "!", 1 "X", 2 "!!"
{
    gFieldEffectArguments[0] = s->localId;
    gFieldEffectArguments[1] = gSaveBlock1Ptr->location.mapNum;
    gFieldEffectArguments[2] = gSaveBlock1Ptr->location.mapGroup;
    if (kind == 0)
        FieldEffectStart(FLDEFF_EXCLAMATION_MARK_ICON);
    else
        MP_StartVsSeekerIcon(kind == 2);
}

// FireRed's GetVsSeekerResponseInArea: 0 none, 1 unbattled trainers only, 2 rematches found
static u8 Answer(void)
{
    u32 i, j;
    bool8 unfought = FALSE, rematch = FALSE;
    for (i = 0; i < sSeenCount; i++)
    {
        const struct Seen *s = &sSeen[i];
        const struct KantoRematch *r = Rematch(s->trainerNum);
        u8 tier;
        if (!FlagGet(TRAINER_FLAG(s->trainerNum)))
        {
            Icon(s, 0);
            unfought = TRUE;
            continue;
        }
        tier = r != NULL ? NextTier(r) : 0;
        if (tier == 0)
        {
            Icon(s, 1);
            continue;
        }
        {
            // (GetCurVsSeekerResponse: a second object of the same trainer answers as the first did)
            s8 same = -1;
            for (j = 0; j < i; j++)
                if (sSeen[j].trainerNum == s->trainerNum)
                    same = sReady[sSeen[j].localId];
            if (same == 0 || (same < 0 && !sTestAlwaysYes && Random() % 100 < 30))
            {
                Icon(s, 1);
                continue;
            }
        }
        sReady[s->localId] = TRUE;
        UnfreezeObjectEvent(&gObjectEvents[s->objectEventId]);
        ScriptMovement_StartObjectMovementScript(s->localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sMovement_WantsRematch);
        Icon(s, 2);
        rematch = TRUE;
    }
    if (rematch)
    {
        PlaySE(SE_PIN);
        sAnyReady = TRUE;
        sChargeSteps = 0;
        sReadyMap = MapKey();
        return 2;
    }
    return unfought ? 1 : 0;
}

static void Task_CloseMessage(u8 taskId)
{
    ClearDialogWindowAndFrame(0, TRUE);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
}

#define tTimer   data[0]
#define tAnswer  data[1]

static void Task_VsSeeker(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    tTimer++;
    if (tTimer == 15 || tTimer == 31 || tTimer == 42)
        PlaySE(SE_CONTEST_MONS_TURN);   // ("beep-beep-beep")
    if (tTimer == 64)
    {
        VarSet(VAR_KANTO_VS_SEEKER, 0);   // (VsSeekerResetInBagStepCounter)
        tAnswer = Answer();
    }
    if (tTimer == 64 + 48)
    {
        if (tAnswer == 0)
        {
            DisplayItemMessageOnField(taskId, sText_TrainersNotReady, Task_CloseMessage);
            return;
        }
        Task_CloseMessage(taskId);
    }
}

// item_use.c: the VS SEEKER used on the field (from the BAG or registered)
void MP_VsSeeker_Use(u8 taskId)
{
    u16 v = VarGet(VAR_KANTO_VS_SEEKER);
    CheckMap();
    Gather();
    if (v < VS_SEEKER_STEPS)
    {
        ConvertIntToDecimalStringN(gStringVar1, VS_SEEKER_STEPS - v, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, sText_BatteryNotCharged);
        DisplayItemMessageOnField(taskId, gStringVar4, Task_CloseMessage);
        return;
    }
    if (!AnyoneToAnswer())
    {
        DisplayItemMessageOnField(taskId, sText_NoTrainersWithinRange, Task_CloseMessage);
        return;
    }
    gTasks[taskId].tTimer = 0;
    gTasks[taskId].tAnswer = 0;
    gTasks[taskId].func = Task_VsSeeker;
}

// Tests: the battery's steps and who is ready (mp_debug.c "vsseeker")
void MP_VsSeeker_Debug(bool8 alwaysYes)
{
    if (alwaysYes)
        sTestAlwaysYes = TRUE;
    u32 i, n = 0;
    for (i = 0; i < ARRAY_COUNT(sReady); i++)
        n += sReady[i];
    printf("[test] vsseeker battery %u ready %u charge %u\n", (unsigned)VarGet(VAR_KANTO_VS_SEEKER), (unsigned)n, (unsigned)sChargeSteps);
}
