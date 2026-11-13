#include "global.h"
#include "mp_region.h"
#include "constants/kai_regions.h"
#include "mp.h"
#include "mp_life.h"   // Moemon Co-op
#include "pokemon.h"
#include "battle.h"
#include "daycare.h"
#include "string_util.h"
#include "mail.h"
#include "pokemon_storage_system.h"
#include "event_data.h"
#include "random.h"
#include "main.h"
#include "egg_hatch.h"
#include "text.h"
#include "menu.h"
#include "international_string_util.h"
#include "script.h"
#include "strings.h"
#include "task.h"
#include "window.h"
#include "party_menu.h"
#include "list_menu.h"
#include "overworld.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/region_map_sections.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

static void ClearDaycareMonMail(struct DaycareMail *mail);
static void SetInitialEggData(struct Pokemon *mon, u16 species, struct DayCare *daycare);
static u8 GetDaycareCompatibilityScore(struct DayCare *daycare);
static void DaycarePrintMonInfo(u8 windowId, u32 daycareSlotId, u8 y);

// RAM buffers used to assist with BuildEggMoveset()
EWRAM_DATA static u16 sHatchedEggLevelUpMoves[EGG_LVL_UP_MOVES_ARRAY_COUNT] = {0};
EWRAM_DATA static u16 sHatchedEggFatherMoves[MAX_MON_MOVES] = {0};
EWRAM_DATA static u16 sHatchedEggFinalMoves[MAX_MON_MOVES] = {0};
EWRAM_DATA static u16 sHatchedEggEggMoves[EGG_MOVES_ARRAY_COUNT] = {0};
EWRAM_DATA static u16 sHatchedEggMotherMoves[MAX_MON_MOVES] = {0};

#include "data/pokemon/egg_moves.h"

static const struct WindowTemplate sDaycareLevelMenuWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 15,
    .tilemapTop = 1,
    .width = 14,
    .height = 6,
    .paletteNum = 15,
    .baseBlock = 8
};

// Indices here are assigned by Task_HandleDaycareLevelMenuInput to VAR_RESULT,
// which is copied to VAR_0x8004 and used as an index for GetDaycareCostAndPrepareString
static const struct ListMenuItem sLevelMenuItems[] =
{
    {gText_ExpandedPlaceholder_Empty, 0},
    {gText_ExpandedPlaceholder_Empty, 1},
    {gText_Exit, DAYCARE_LEVEL_MENU_EXIT}
};

static const struct ListMenuTemplate sDaycareListMenuLevelTemplate =
{
    .items = sLevelMenuItems,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .itemPrintFunc = DaycarePrintMonInfo,
    .totalItems = 3,
    .maxShowed = 3,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NORMAL,
    .cursorKind = CURSOR_BLACK_ARROW
};

static const u8 *const sCompatibilityMessages[] =
{
    gDaycareText_GetAlongVeryWell,
    gDaycareText_GetAlong,
    gDaycareText_DontLikeOther,
    gDaycareText_PlayOther
};

static const u8 sJapaneseEggNickname[] = _("タマゴ"); // "tamago" ("egg" in Japanese)

u8 *GetMonNickname2(struct Pokemon *mon, u8 *dest)
{
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];
    GetMonData(mon, MON_DATA_NICKNAME, nickname);
    return StringCopy_Nickname(dest, nickname);
}

u8 *GetBoxMonNickname(struct BoxPokemon *mon, u8 *dest)
{
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];
    GetBoxMonData(mon, MON_DATA_NICKNAME, nickname);
    return StringCopy_Nickname(dest, nickname);
}

u8 CountPokemonInDaycare(struct DayCare *daycare)
{
    u8 i, count;
    count = 0;

    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        if (GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SPECIES) != 0)
            count++;
    }

    return count;
}

void InitDaycareMailRecordMixing(struct DayCare *daycare, struct RecordMixingDaycareMail *mixMail)
{
    u8 i;
    u8 numDaycareMons = 0;

    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        if (GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SPECIES) != SPECIES_NONE)
        {
            numDaycareMons++;
            if (GetBoxMonData(&daycare->mons[i].mon, MON_DATA_HELD_ITEM) == ITEM_NONE)
                mixMail->cantHoldItem[i] = FALSE;
            else
                mixMail->cantHoldItem[i] = TRUE;
        }
        else
        {
            // Daycare slot empty
            mixMail->cantHoldItem[i] = TRUE;
        }
    }

    mixMail->numDaycareMons = numDaycareMons;
}

static s8 Daycare_FindEmptySpot(struct DayCare *daycare)
{
    u8 i;

    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        if (GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SPECIES) == SPECIES_NONE)
            return i;
    }

    return -1;
}

static void StorePokemonInDaycare(struct Pokemon *mon, struct DaycareMon *daycareMon)
{
    if (MonHasMail(mon))
    {
        u8 mailId;

        StringCopy(daycareMon->mail.otName, gSaveBlock2Ptr->playerName);
        GetMonNickname2(mon, daycareMon->mail.monName);
        StripExtCtrlCodes(daycareMon->mail.monName);
        daycareMon->mail.gameLanguage = GAME_LANGUAGE;
        daycareMon->mail.monLanguage = GetMonData(mon, MON_DATA_LANGUAGE);
        mailId = GetMonData(mon, MON_DATA_MAIL);
        daycareMon->mail.message = gSaveBlock1Ptr->mail[mailId];
        TakeMailFromMon(mon);
    }

    daycareMon->mon = mon->box;
    BoxMonRestorePP(&daycareMon->mon);
    daycareMon->steps = 0;
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
}

static void StorePokemonInEmptyDaycareSlot(struct Pokemon *mon, struct DayCare *daycare)
{
    s8 slotId = Daycare_FindEmptySpot(daycare);
    if (daycare == &gSaveBlock1Ptr->daycare)
        VarSet(VAR_JOHTO_DAYCARE_OWNERS, VarGet(VAR_JOHTO_DAYCARE_OWNERS) & ~(1 << slotId));   // (Moemon Kai: JOHTO's DAY-CARE)
    StorePokemonInDaycare(mon, &daycare->mons[slotId]);
}

void StoreSelectedPokemonInDaycare(void)
{
    u8 monId = GetCursorSelectionMonId();
    StorePokemonInEmptyDaycareSlot(&gPlayerParty[monId], &gSaveBlock1Ptr->daycare);
}

// Shifts the second daycare Pokémon slot into the first slot.
static void ShiftDaycareSlots(struct DayCare *daycare)
{
    // This condition is only satisfied when the player takes out the first Pokémon from the daycare.
    if (GetBoxMonData(&daycare->mons[1].mon, MON_DATA_SPECIES) != SPECIES_NONE
        && GetBoxMonData(&daycare->mons[0].mon, MON_DATA_SPECIES) == SPECIES_NONE)
    {
        if (daycare == &gSaveBlock1Ptr->daycare)
            VarSet(VAR_JOHTO_DAYCARE_OWNERS, VarGet(VAR_JOHTO_DAYCARE_OWNERS) >> 1);   // (Moemon Kai: JOHTO's DAY-CARE)
        daycare->mons[0].mon = daycare->mons[1].mon;
        ZeroBoxMonData(&daycare->mons[1].mon);

        daycare->mons[0].mail = daycare->mons[1].mail;
        daycare->mons[0].steps = daycare->mons[1].steps;
        daycare->mons[1].steps = 0;
        ClearDaycareMonMail(&daycare->mons[1].mail);
    }
}

static void ApplyDaycareExperience(struct Pokemon *mon)
{
    s32 i;
    bool8 firstMove;
    u16 learnedMove;

    for (i = 0; i < MAX_LEVEL; i++)
    {
        // Add the mon's gained daycare experience level by level until it can't level up anymore.
        if (TryIncrementMonLevel(mon))
        {
            // Teach the mon new moves it learned while in the daycare.
            firstMove = TRUE;
            while ((learnedMove = MonTryLearningNewMove(mon, firstMove)) != 0)
            {
                firstMove = FALSE;
                if (learnedMove == MON_HAS_MAX_MOVES)
                    DeleteFirstMoveAndGiveMoveToMon(mon, gMoveToLearn);
            }
        }
        else
        {
            break;
        }
    }

    // Re-calculate the mons stats at its new level.
    CalculateMonStats(mon);
}

static u16 TakeSelectedPokemonFromDaycare(struct DaycareMon *daycareMon)
{
    u16 species;
    u32 experience;
    struct Pokemon pokemon;

    GetBoxMonNickname(&daycareMon->mon, gStringVar1);
    species = GetBoxMonData(&daycareMon->mon, MON_DATA_SPECIES);
    BoxMonToMon(&daycareMon->mon, &pokemon);

    if (GetMonData(&pokemon, MON_DATA_LEVEL) != MAX_LEVEL)
    {
        experience = GetMonData(&pokemon, MON_DATA_EXP) + daycareMon->steps;
        SetMonData(&pokemon, MON_DATA_EXP, &experience);
        ApplyDaycareExperience(&pokemon);
    }

    gPlayerParty[PARTY_SIZE - 1] = pokemon;
    if (daycareMon->mail.message.itemId)
    {
        GiveMailToMon(&gPlayerParty[PARTY_SIZE - 1], &daycareMon->mail.message);
        ClearDaycareMonMail(&daycareMon->mail);
    }

    ZeroBoxMonData(&daycareMon->mon);
    daycareMon->steps = 0;
    CompactPartySlots();
    CalculatePlayerPartyCount();
    return species;
}

static u16 TakeSelectedPokemonMonFromDaycareShiftSlots(struct DayCare *daycare, u8 slotId)
{
    u16 species = TakeSelectedPokemonFromDaycare(&daycare->mons[slotId]);
    ShiftDaycareSlots(daycare);
    return species;
}

u16 TakePokemonFromDaycare(void)
{
    return TakeSelectedPokemonMonFromDaycareShiftSlots(&gSaveBlock1Ptr->daycare, gSpecialVar_0x8004);
}

static u8 GetLevelAfterDaycareSteps(struct BoxPokemon *mon, u32 steps)
{
    struct BoxPokemon tempMon = *mon;

    u32 experience = GetBoxMonData(mon, MON_DATA_EXP) + steps;
    SetBoxMonData(&tempMon, MON_DATA_EXP,  &experience);
    return GetLevelFromBoxMonExp(&tempMon);
}

static u8 GetNumLevelsGainedFromSteps(struct DaycareMon *daycareMon)
{
    u8 levelBefore;
    u8 levelAfter;

    levelBefore = GetLevelFromBoxMonExp(&daycareMon->mon);
    levelAfter = GetLevelAfterDaycareSteps(&daycareMon->mon, daycareMon->steps);
    return levelAfter - levelBefore;
}

static u8 GetNumLevelsGainedForDaycareMon(struct DaycareMon *daycareMon)
{
    u8 numLevelsGained = GetNumLevelsGainedFromSteps(daycareMon);
    ConvertIntToDecimalStringN(gStringVar2, numLevelsGained, STR_CONV_MODE_LEFT_ALIGN, 2);
    GetBoxMonNickname(&daycareMon->mon, gStringVar1);
    return numLevelsGained;
}

static u32 PrepareDaycareCostStringForSelectedMon(struct DaycareMon *daycareMon)
{
    u32 cost;

    u8 numLevelsGained = GetNumLevelsGainedFromSteps(daycareMon);
    GetBoxMonNickname(&daycareMon->mon, gStringVar1);
    cost = 100 + 100 * numLevelsGained;
    ConvertIntToDecimalStringN(gStringVar2, cost, STR_CONV_MODE_LEFT_ALIGN, 5);
    return cost;
}

static u16 PrepareDaycareCostStringForMon(struct DayCare *daycare, u8 slotId)
{
    return PrepareDaycareCostStringForSelectedMon(&daycare->mons[slotId]);
}

void GetDaycareCostAndPrepareString(void)
{
    gSpecialVar_0x8005 = PrepareDaycareCostStringForMon(&gSaveBlock1Ptr->daycare, gSpecialVar_0x8004);
}

// Moemon Kai: KANTO's ROUTE 5 DAY-CARE, FireRed's one-POKéMON day care (pokefirered src/daycare.c, audit case 47):
// its own slot, kept in "<save>.region" (mp_region.c chunk R5DC), growing by a step for each of the player's
static struct DaycareMon sRoute5DayCareMon;

void *MP_Route5Daycare_Data(u32 *size)
{
    *size = sizeof(sRoute5DayCareMon);
    return &sRoute5DayCareMon;
}

void MP_Route5Daycare_Clear(void)
{
    memset(&sRoute5DayCareMon, 0, sizeof(sRoute5DayCareMon));
}

void PutMonInRoute5Daycare(void)
{
    u8 monIdx = GetCursorSelectionMonId();
    StorePokemonInDaycare(&gPlayerParty[monIdx], &sRoute5DayCareMon);
}

void GetCostToWithdrawRoute5DaycareMon(void)
{
    gSpecialVar_0x8005 = PrepareDaycareCostStringForSelectedMon(&sRoute5DayCareMon);
}

bool8 IsThereMonInRoute5Daycare(void)
{
    return GetBoxMonData(&sRoute5DayCareMon.mon, MON_DATA_SPECIES) != SPECIES_NONE;
}

u8 GetNumLevelsGainedForRoute5DaycareMon(void)
{
    return GetNumLevelsGainedForDaycareMon(&sRoute5DayCareMon);
}

u16 TakePokemonFromRoute5Daycare(void)
{
    return TakeSelectedPokemonFromDaycare(&sRoute5DayCareMon);
}

// Moemon Kai: JOHTO's DAY-CARE, Crystal's (pokecrystal engine/events/daycare.asm, audit case 47): the MAN and the
// LADY each raise one POKéMON. They're the game's DAY CARE pair (ROUTE 117's, FOUR ISLAND's): the slot each of them
// holds is a bit in VAR_JOHTO_DAYCARE_OWNERS. Person in VAR_0x8009: 0 the LADY, 1 the MAN.
static s8 JohtoSlot(u8 man)
{
    u16 owners = VarGet(VAR_JOHTO_DAYCARE_OWNERS);
    u32 i;
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
        if (GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[i].mon, MON_DATA_SPECIES) != SPECIES_NONE
         && ((owners >> i) & 1) == man)
            return i;
    return -1;
}

// The POKéMON this person holds: VAR_RESULT its slot (0xFF none); its nickname in STR_VAR_1, the levels it grew in
// STR_VAR_2 and VAR_0x8006, the price (100 + 100 a level) in STR_VAR_3 and VAR_0x8005
u16 MP_JohtoDaycare_Slot(void)
{
    s8 slot = JohtoSlot(gSpecialVar_0x8009 != 0);
    struct DaycareMon *dm;
    u8 levels;
    if (slot < 0)
        return 0xFF;
    dm = &gSaveBlock1Ptr->daycare.mons[slot];
    levels = GetNumLevelsGainedFromSteps(dm);
    GetBoxMonNickname(&dm->mon, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, levels, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_0x8005 = 100 + 100 * levels;
    gSpecialVar_0x8006 = levels;
    ConvertIntToDecimalStringN(gStringVar3, gSpecialVar_0x8005, STR_CONV_MODE_LEFT_ALIGN, 5);
    return slot;
}

// Crystal's DayCareAskDepositPokemon checks on the POKéMON chosen (VAR_0x8004): 0 fine, 1 an EGG, 2 the last one
// that can battle, 3 holding MAIL
u16 MP_JohtoDaycare_CheckPick(void)
{
    struct Pokemon *mon = &gPlayerParty[gSpecialVar_0x8004];
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return 1;
    if (CountPartyAliveNonEggMons_IgnoreVar0x8004Slot() == 0)
        return 2;
    if (MonHasMail(mon))
        return 3;
    return 0;
}

void MP_JohtoDaycare_Deposit(void)
{
    s8 slot = Daycare_FindEmptySpot(&gSaveBlock1Ptr->daycare);
    if (slot < 0)
        return;
    StorePokemonInEmptyDaycareSlot(&gPlayerParty[gSpecialVar_0x8004], &gSaveBlock1Ptr->daycare);
    if (gSpecialVar_0x8009)
        VarSet(VAR_JOHTO_DAYCARE_OWNERS, VarGet(VAR_JOHTO_DAYCARE_OWNERS) | (1 << slot));
}

// The POKéMON back (its species in VAR_RESULT, its nickname in STR_VAR_1)
u16 MP_JohtoDaycare_Withdraw(void)
{
    s8 slot = JohtoSlot(gSpecialVar_0x8009 != 0);
    if (slot < 0)
        return SPECIES_NONE;
    return TakeSelectedPokemonMonFromDaycareShiftSlots(&gSaveBlock1Ptr->daycare, slot);
}

// Who stands where (the DAY-CARE's and ROUTE 34's ON_TRANSITION): the MAN waits outside with an EGG
void MP_JohtoDaycare_Flags(void)
{
    u16 owners = VarGet(VAR_JOHTO_DAYCARE_OWNERS);
    bool8 has[2] = {FALSE, FALSE};   // (the MAN's, the LADY's: Crystal's Route34EggCheckCallback .CheckMon1 / 2)
    u32 i;
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
        if (GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[i].mon, MON_DATA_SPECIES) != SPECIES_NONE)
            has[((owners >> i) & 1) ? 0 : 1] = TRUE;
    if (has[0])
        FlagClear(FLAG_JOHTO_HIDE_DAYCARE_MON_1);
    else
        FlagSet(FLAG_JOHTO_HIDE_DAYCARE_MON_1);
    if (has[1])
        FlagClear(FLAG_JOHTO_HIDE_DAYCARE_MON_2);
    else
        FlagSet(FLAG_JOHTO_HIDE_DAYCARE_MON_2);
    if (GetDaycareState() == DAYCARE_EGG_WAITING)
    {
        FlagSet(FLAG_JOHTO_HIDE_DAYCARE_MAN_IN);
        FlagClear(FLAG_JOHTO_HIDE_DAYCARE_MAN_OUT);
    }
    else
    {
        FlagClear(FLAG_JOHTO_HIDE_DAYCARE_MAN_IN);
        FlagSet(FLAG_JOHTO_HIDE_DAYCARE_MAN_OUT);
    }
}

static void UNUSED Debug_AddDaycareSteps(u16 numSteps)
{
    gSaveBlock1Ptr->daycare.mons[0].steps += numSteps;
    gSaveBlock1Ptr->daycare.mons[1].steps += numSteps;
}

u8 GetNumLevelsGainedFromDaycare(void)
{
    if (GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[gSpecialVar_0x8004].mon, MON_DATA_SPECIES) != 0)
        return GetNumLevelsGainedForDaycareMon(&gSaveBlock1Ptr->daycare.mons[gSpecialVar_0x8004]);

    return 0;
}

static void ClearDaycareMonMail(struct DaycareMail *mail)
{
    s32 i;

    for (i = 0; i < PLAYER_NAME_LENGTH + 1; i++)
        mail->otName[i] = 0;
    for (i = 0; i < POKEMON_NAME_LENGTH + 1; i++)
        mail->monName[i] = 0;

    ClearMail(&mail->message);
}

static void ClearDaycareMon(struct DaycareMon *daycareMon)
{
    ZeroBoxMonData(&daycareMon->mon);
    daycareMon->steps = 0;
    ClearDaycareMonMail(&daycareMon->mail);
}

static void UNUSED ClearAllDaycareData(struct DayCare *daycare)
{
    u8 i;

    for (i = 0; i < DAYCARE_MON_COUNT; i++)
        ClearDaycareMon(&daycare->mons[i]);

    daycare->offspringPersonality = 0;
    daycare->stepCounter = 0;
}

// Determines what the species of an Egg would be based on the given species.
// It determines this by working backwards through the evolution chain of the
// given species.
static u16 GetEggSpecies(u16 species)
{
    int i, j, k;
    bool8 found;

    // Working backwards up to 5 times seems arbitrary, since the maximum number
    // of times would only be 3 for 3-stage evolutions.
    for (i = 0; i < EVOS_PER_MON; i++)
    {
        found = FALSE;
        for (j = 1; j < NUM_SPECIES; j++)
        {
            for (k = 0; k < EVOS_PER_MON; k++)
            {
                if (gEvolutionTable[j][k].targetSpecies == species)
                {
                    species = j;
                    found = TRUE;
                    break;
                }
            }

            if (found)
                break;
        }

        if (j == NUM_SPECIES)
            break;
    }

    return species;
}

// Moemon Kai: an EVERSTONE passes its holder's Nature every time, from either parent, as in BLACK / WHITE on (the user,
// 2026-11-06: "Be certain that Everstones and Destiny Knots function properly"; Emerald gave only the mother's or
// DITTO's, half the time). Both holding one: either at random.
static s32 GetParentToInheritNature(struct DayCare *daycare)
{
    bool8 holds[DAYCARE_MON_COUNT];
    s32 i;
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
        holds[i] = GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SANITY_HAS_SPECIES)
                && GetBoxMonData(&daycare->mons[i].mon, MON_DATA_HELD_ITEM) == ITEM_EVERSTONE;
    if (holds[0] && holds[1])
        return Random() & 1;
    if (holds[0])
        return 0;
    if (holds[1])
        return 1;
    return -1;
}

static void _TriggerPendingDaycareEgg(struct DayCare *daycare)
{
    s32 parent;
    s32 natureTries = 0;

    SeedRng2(gMain.vblankCounter2);
    parent = GetParentToInheritNature(daycare);

    // don't inherit nature
    if (parent < 0)
    {
        daycare->offspringPersonality = (Random2() << 16) | ((Random() % 0xfffe) + 1);
    }
    // inherit nature
    else
    {
        u8 wantedNature = GetNatureFromPersonality(GetBoxMonData(&daycare->mons[parent].mon, MON_DATA_PERSONALITY, NULL));
        u32 personality;

        do
        {
            personality = (Random2() << 16) | (Random());
            if (wantedNature == GetNatureFromPersonality(personality) && personality != 0)
                break; // found a personality with the same nature

            natureTries++;
        } while (natureTries <= 100000);   // Moemon Kai: (1 in 25 a try: never runs out, as 2400 could)

        daycare->offspringPersonality = personality;
    }

    FlagSet(FLAG_PENDING_DAYCARE_EGG);
}

// Functionally unused
static void _TriggerPendingDaycareMaleEgg(struct DayCare *daycare)
{
    daycare->offspringPersonality = (Random()) | (EGG_GENDER_MALE);
    FlagSet(FLAG_PENDING_DAYCARE_EGG);
}

void TriggerPendingDaycareEgg(void)
{
    _TriggerPendingDaycareEgg(&gSaveBlock1Ptr->daycare);
}

static void UNUSED TriggerPendingDaycareMaleEgg(void)
{
    _TriggerPendingDaycareMaleEgg(&gSaveBlock1Ptr->daycare);
}

// Removes the selected index from the given IV list and shifts the remaining
// elements to the left.
static void RemoveIVIndexFromList(u8 *ivs, u8 selectedIv)
{
    s32 i, j;
    u8 temp[NUM_STATS];

    ivs[selectedIv] = 0xFF;
    for (i = 0; i < NUM_STATS; i++)
    {
        temp[i] = ivs[i];
    }

    j = 0;
    for (i = 0; i < NUM_STATS; i++)
    {
        if (temp[i] != 0xFF)
            ivs[j++] = temp[i];
    }
}

// Moemon Kai: three different IVs from the parents, five when either holds a DESTINY KNOT (X / Y on; the user,
// 2026-11-06: "IVs not being passed down"). Emerald's own pick could take the same stat twice (it removed list slot
// i instead of the slot it picked), so an egg often got only one or two of its parents' IVs.
// Moemon Kai: a POWER item passes its holder's own stat (HEARTGOLD / SOULSILVER on), one of the inherited IVs; both
// parents holding one: either at random (X / Y on). -1: none.
static s8 PowerItemStat(u16 item)
{
    switch (item)
    {
    case ITEM_POWER_WEIGHT: return STAT_HP;
    case ITEM_POWER_BRACER: return STAT_ATK;
    case ITEM_POWER_BELT:   return STAT_DEF;
    case ITEM_POWER_ANKLET: return STAT_SPEED;
    case ITEM_POWER_LENS:   return STAT_SPATK;
    case ITEM_POWER_BAND:   return STAT_SPDEF;
    }
    return -1;
}

static void InheritIVs(struct Pokemon *egg, struct DayCare *daycare)
{
    u8 i, count = INHERITED_IV_COUNT, start = 0;
    u8 availableIVs[NUM_STATS];
    s8 power[DAYCARE_MON_COUNT];
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        bool8 here = GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SANITY_HAS_SPECIES);
        u16 item = here ? GetBoxMonData(&daycare->mons[i].mon, MON_DATA_HELD_ITEM) : ITEM_NONE;
        if (item == ITEM_DESTINY_KNOT)
            count = 5;
        power[i] = here ? PowerItemStat(item) : -1;
    }
    for (i = 0; i < NUM_STATS; i++)
        availableIVs[i] = i;
    if (power[0] >= 0 || power[1] >= 0)
    {
        u8 parent = (power[0] >= 0 && power[1] >= 0) ? (Random() & 1) : (power[0] >= 0 ? 0 : 1);
        u8 stat = power[parent], iv = GetBoxMonData(&daycare->mons[parent].mon, MON_DATA_HP_IV + stat);
        SetMonData(egg, MON_DATA_HP_IV + stat, &iv);
        RemoveIVIndexFromList(availableIVs, stat);
        start = 1;
    }
    for (i = start; i < count; i++)
    {
        u8 index = Random() % (NUM_STATS - i), stat = availableIVs[index];
        u8 parent = Random() % DAYCARE_MON_COUNT, iv;
        RemoveIVIndexFromList(availableIVs, index);
        if (!GetBoxMonData(&daycare->mons[parent].mon, MON_DATA_SANITY_HAS_SPECIES))
            parent ^= 1;
        iv = GetBoxMonData(&daycare->mons[parent].mon, MON_DATA_HP_IV + stat);
        SetMonData(egg, MON_DATA_HP_IV + stat, &iv);
    }
}

// Counts the number of egg moves a Pokémon learns and stores the moves in
// the given array.
// Moemon Kai: a species' egg moves (STADIUM's CHALLENGE CUP, mp_stadium.c); out holds EGG_MOVES_ARRAY_COUNT
u8 MP_SpeciesEggMoves(u16 species, u16 *out)
{
    u16 i, idx = 0xFFFF, n = 0;
    for (i = 0; i < ARRAY_COUNT(gEggMoves) - 1; i++)
        if (gEggMoves[i] == species + EGG_MOVES_SPECIES_OFFSET)
        {
            idx = i + 1;
            break;
        }
    if (idx == 0xFFFF)
        return 0;
    for (i = 0; i < EGG_MOVES_ARRAY_COUNT && idx + i < ARRAY_COUNT(gEggMoves); i++)
    {
        if (gEggMoves[idx + i] > EGG_MOVES_SPECIES_OFFSET)
            break;
        out[n++] = gEggMoves[idx + i];
    }
    return n;
}

static u8 GetEggMoves(struct Pokemon *pokemon, u16 *eggMoves)
{
    u16 eggMoveIdx;
    u16 numEggMoves;
    u16 species;
    u16 i;

    numEggMoves = 0;
    eggMoveIdx = 0;
    species = GetMonData(pokemon, MON_DATA_SPECIES);
    for (i = 0; i < ARRAY_COUNT(gEggMoves) - 1; i++)
    {
        if (gEggMoves[i] == species + EGG_MOVES_SPECIES_OFFSET)
        {
            eggMoveIdx = i + 1;
            break;
        }
    }

    for (i = 0; i < EGG_MOVES_ARRAY_COUNT; i++)
    {
        if (gEggMoves[eggMoveIdx + i] > EGG_MOVES_SPECIES_OFFSET)
            break;

        eggMoves[i] = gEggMoves[eggMoveIdx + i];
        numEggMoves++;
    }

    return numEggMoves;
}

static void BuildEggMoveset(struct Pokemon *egg, struct BoxPokemon *father, struct BoxPokemon *mother)
{
    u16 numSharedParentMoves;
    u32 numLevelUpMoves;
    u16 numEggMoves;
    u16 i, j;

    numSharedParentMoves = 0;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        sHatchedEggMotherMoves[i] = MOVE_NONE;
        sHatchedEggFatherMoves[i] = MOVE_NONE;
        sHatchedEggFinalMoves[i] = MOVE_NONE;
    }
    for (i = 0; i < EGG_MOVES_ARRAY_COUNT; i++)
        sHatchedEggEggMoves[i] = MOVE_NONE;
    for (i = 0; i < EGG_LVL_UP_MOVES_ARRAY_COUNT; i++)
        sHatchedEggLevelUpMoves[i] = MOVE_NONE;

    numLevelUpMoves = GetLevelUpMovesBySpecies(GetMonData(egg, MON_DATA_SPECIES), sHatchedEggLevelUpMoves);
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        sHatchedEggFatherMoves[i] = GetBoxMonData(father, MON_DATA_MOVE1 + i);
        sHatchedEggMotherMoves[i] = GetBoxMonData(mother, MON_DATA_MOVE1 + i);
    }

    numEggMoves = GetEggMoves(egg, sHatchedEggEggMoves);

    // Moemon Kai: breeding's moves as in SCARLET / VIOLET: the level-up moves both parents know, then the egg moves
    // either parent knows (the father's only before X / Y), last so they stay when there are more than four; moves
    // from TMs no longer pass down (X / Y on)
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (sHatchedEggFatherMoves[i] == MOVE_NONE)
            break;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            if (sHatchedEggFatherMoves[i] == sHatchedEggMotherMoves[j] && sHatchedEggFatherMoves[i] != MOVE_NONE)
                sHatchedEggFinalMoves[numSharedParentMoves++] = sHatchedEggFatherMoves[i];
        }
    }

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (sHatchedEggFinalMoves[i] == MOVE_NONE)
            break;
        for (j = 0; j < numLevelUpMoves; j++)
        {
            if (sHatchedEggLevelUpMoves[j] != MOVE_NONE && sHatchedEggFinalMoves[i] == sHatchedEggLevelUpMoves[j])
            {
                if (MP_Gen_MoveAllowed(sHatchedEggFinalMoves[i]) && GiveMoveToMon(egg, sHatchedEggFinalMoves[i]) == MON_HAS_MAX_MOVES)   // Moemon Kai: GENERATION
                    DeleteFirstMoveAndGiveMoveToMon(egg, sHatchedEggFinalMoves[i]);
                break;
            }
        }
    }

    for (i = 0; i < 2 * MAX_MON_MOVES; i++)
    {
        u16 move = i < MAX_MON_MOVES ? sHatchedEggMotherMoves[i] : sHatchedEggFatherMoves[i - MAX_MON_MOVES];
        if (move == MOVE_NONE)
            continue;
        for (j = 0; j < numEggMoves; j++)
        {
            if (move == sHatchedEggEggMoves[j])
            {
                if (MP_Gen_MoveAllowed(move) && GiveMoveToMon(egg, move) == MON_HAS_MAX_MOVES)   // Moemon Kai: GENERATION
                    DeleteFirstMoveAndGiveMoveToMon(egg, move);
                break;
            }
        }
    }
}

static void RemoveEggFromDayCare(struct DayCare *daycare)
{
    daycare->offspringPersonality = 0;
    daycare->stepCounter = 0;
}

void RejectEggFromDayCare(void)
{
    struct Pokemon unused;
    MP_Linkx_TakePendingEgg(&unused);   // Moemon Co-op
    RemoveEggFromDayCare(&gSaveBlock1Ptr->daycare);
}

// Moemon Kai: babies hatch without their incense, as in SCARLET / VIOLET (MARILL always gives AZURILL; Emerald and
// Gen 4 wanted the incense held). The next stage only while the GENERATION setting hasn't reached the baby.
static const u16 sIncenseBabies[][2] =
{
    {SPECIES_AZURILL, SPECIES_MARILL},
    {SPECIES_WYNAUT, SPECIES_WOBBUFFET},
    {SPECIES_BUDEW, SPECIES_ROSELIA},
    {SPECIES_CHINGLING, SPECIES_CHIMECHO},
    {SPECIES_BONSLY, SPECIES_SUDOWOODO},
    {SPECIES_MIME_JR, SPECIES_MR_MIME},
    {SPECIES_HAPPINY, SPECIES_CHANSEY},
    {SPECIES_MUNCHLAX, SPECIES_SNORLAX},
    {SPECIES_MANTYKE, SPECIES_MANTINE},
};

static void AlterEggSpeciesWithIncenseItem(u16 *species, struct DayCare *daycare)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sIncenseBabies); i++)
    {
        if (*species == sIncenseBabies[i][0])
        {
            if (MP_Gen_SealedSpecies(*species))
                *species = sIncenseBabies[i][1];
            return;
        }
    }
}

static void GiveVoltTackleIfLightBall(struct Pokemon *mon, struct DayCare *daycare)
{
    u32 motherItem = GetBoxMonData(&daycare->mons[0].mon, MON_DATA_HELD_ITEM);
    u32 fatherItem = GetBoxMonData(&daycare->mons[1].mon, MON_DATA_HELD_ITEM);

    if (motherItem == ITEM_LIGHT_BALL || fatherItem == ITEM_LIGHT_BALL)
    {
        if (GiveMoveToMon(mon, MOVE_VOLT_TACKLE) == MON_HAS_MAX_MOVES)
            DeleteFirstMoveAndGiveMoveToMon(mon, MOVE_VOLT_TACKLE);
    }
}

static u16 DetermineEggSpeciesAndParentSlots(struct DayCare *daycare, u8 *parentSlots)
{
    u16 i;
    u16 species[DAYCARE_MON_COUNT];
    u16 eggSpecies;

    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        species[i] = GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SPECIES);
        if (species[i] == SPECIES_DITTO)
        {
            parentSlots[0] = i ^ 1;
            parentSlots[1] = i;
        }
        else if (GetBoxMonGender(&daycare->mons[i].mon) == MON_FEMALE)
        {
            parentSlots[0] = i;
            parentSlots[1] = i ^ 1;
        }
    }

    eggSpecies = GetEggSpecies(species[parentSlots[0]]);
    if (eggSpecies == SPECIES_MANAPHY)
        eggSpecies = SPECIES_PHIONE;   // Moemon Kai: as in DIAMOND / PEARL
    // Moemon Kai: Gen 9's rule (BLACK / WHITE on): a NIDORAN or VOLBEAT / ILLUMISE EGG is either one, half and half,
    // whichever parent laid it (the user, 2026-11-07: NIDORAN M and DITTO gave only NIDORAN M)
    if (eggSpecies == SPECIES_NIDORAN_F || eggSpecies == SPECIES_NIDORAN_M)
        eggSpecies = (daycare->offspringPersonality & EGG_GENDER_MALE) ? SPECIES_NIDORAN_M : SPECIES_NIDORAN_F;
    if (eggSpecies == SPECIES_ILLUMISE || eggSpecies == SPECIES_VOLBEAT)
        eggSpecies = (daycare->offspringPersonality & EGG_GENDER_MALE) ? SPECIES_VOLBEAT : SPECIES_ILLUMISE;

    // Make Ditto the "mother" slot if the other daycare mon is male.
    if (species[parentSlots[1]] == SPECIES_DITTO && GetBoxMonGender(&daycare->mons[parentSlots[0]].mon) != MON_FEMALE)
    {
        u8 ditto = parentSlots[1];
        parentSlots[1] = parentSlots[0];
        parentSlots[0] = ditto;
    }

    return eggSpecies;
}

// Moemon Kai: the parent a SCARLET / VIOLET EGG takes its POKé BALL and Ability from: the one that isn't DITTO, else
// the mother
static u8 EggMainParent(struct DayCare *daycare)
{
    u8 i;
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
        if (GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SPECIES) == SPECIES_DITTO)
            return i ^ 1;
    return GetBoxMonGender(&daycare->mons[1].mon) == MON_FEMALE ? 1 : 0;
}

// The POKé BALL a POKéMON is in, as an item (the special balls are kept in its ball style: mp_ballstyle.c)
static u16 BoxMonBallItem(struct BoxPokemon *mon)
{
    u8 special = MP_BallStyle_Special(GetBoxMonData(mon, MON_DATA_ENCRYPT_SEPARATOR));
    return special ? MP_SpecialBall_Item(special) : GetBoxMonData(mon, MON_DATA_POKEBALL);
}

// Moemon Kai: the EGG comes in its mother's POKé BALL, or the one that isn't DITTO's, half and half between parents of
// the same species (SUN / MOON on; a MASTER BALL too, as in SCARLET / VIOLET). The ASTRAL BALL stays its catch's own
// (a POKé BALL instead), as a CHERISH BALL does.
static void InheritBall(struct Pokemon *egg, struct DayCare *daycare)
{
    u8 from = EggMainParent(daycare), special;
    u16 item, style;
    u32 ball;
    if (GetBoxMonData(&daycare->mons[0].mon, MON_DATA_SPECIES) == GetBoxMonData(&daycare->mons[1].mon, MON_DATA_SPECIES)
     && GetBoxMonData(&daycare->mons[from ^ 1].mon, MON_DATA_SANITY_HAS_SPECIES))
        from = Random() & 1;
    item = BoxMonBallItem(&daycare->mons[from].mon);
    if (item == ITEM_ASTRAL_BALL || !MP_IsBallItem(item))
        item = ITEM_POKE_BALL;
    special = MP_SpecialBall_FromItem(item);
    ball = special ? ITEM_POKE_BALL : item;
    style = (u16)special << 12;
    SetMonData(egg, MON_DATA_POKEBALL, &ball);
    SetMonData(egg, MON_DATA_ENCRYPT_SEPARATOR, &style);
}

// Moemon Kai: the Ability as in X / Y on: the mother's (or the parent that isn't DITTO's) Ability slot 4 times in 5,
// the other slot otherwise (Emerald: the personality's, either parent ignored)
static void InheritAbility(struct Pokemon *egg, struct DayCare *daycare)
{
    u16 species = GetMonData(egg, MON_DATA_SPECIES);
    u8 num;
    if (gSpeciesInfo[species].abilities[1] == ABILITY_NONE)
        return;
    num = GetBoxMonData(&daycare->mons[EggMainParent(daycare)].mon, MON_DATA_ABILITY_NUM);
    if (Random() % 5 == 0)
        num ^= 1;
    SetMonData(egg, MON_DATA_ABILITY_NUM, &num);
}

static void BuildDaycareEgg(struct DayCare *daycare, struct Pokemon *egg)
{
    u16 species;
    u8 parentSlots[DAYCARE_MON_COUNT];
    bool8 isEgg;

    species = DetermineEggSpeciesAndParentSlots(daycare, parentSlots);
    AlterEggSpeciesWithIncenseItem(&species, daycare);
    SetInitialEggData(egg, species, daycare);
    InheritBall(egg, daycare);
    InheritAbility(egg, daycare);
    InheritIVs(egg, daycare);
    BuildEggMoveset(egg, &daycare->mons[parentSlots[1]].mon, &daycare->mons[parentSlots[0]].mon);

    if (species == SPECIES_PICHU)
        GiveVoltTackleIfLightBall(egg, daycare);

    isEgg = TRUE;
    SetMonData(egg, MON_DATA_IS_EGG, &isEgg);
}

static void _GiveEggFromDaycare(struct DayCare *daycare)
{
    struct Pokemon egg;

    // Moemon Co-op: an EGG made with a friend's DAY CARE POKéMON before 1.171.5, still waiting (each player's DAY CARE is
    // their own since: the user, 2026-11-07; no new ones are made)
    if (daycare == &gSaveBlock1Ptr->daycare && MP_Linkx_TakePendingEgg(&egg))
        ;
    else if (CountPokemonInDaycare(daycare) == 1)
    {
        // The other parent was a friend's (before 1.171.5) but the EGG got lost: make it from ours alone
        struct DayCare tmp = *daycare;
        u8 s = GetBoxMonData(&tmp.mons[0].mon, MON_DATA_SANITY_HAS_SPECIES) ? 0 : 1;
        tmp.mons[s ^ 1] = tmp.mons[s];
        BuildDaycareEgg(&tmp, &egg);
    }
    else
        BuildDaycareEgg(daycare, &egg);
    // Moemon Co-op: MOéMON GARDEN, the EGG's record knows its parents (the ones in this DAY CARE)
    if (daycare == &gSaveBlock1Ptr->daycare)
        MP_Life_NoteEgg(&egg.box,
                        GetBoxMonData(&daycare->mons[0].mon, MON_DATA_SANITY_HAS_SPECIES) ? &daycare->mons[0].mon : NULL,
                        GetBoxMonData(&daycare->mons[1].mon, MON_DATA_SANITY_HAS_SPECIES) ? &daycare->mons[1].mon : NULL);
    gPlayerParty[PARTY_SIZE - 1] = egg;
    CompactPartySlots();
    CalculatePlayerPartyCount();
    RemoveEggFromDayCare(daycare);
}

void CreateEgg(struct Pokemon *mon, u16 species, bool8 setHotSpringsLocation)
{
    u8 metLevel;
    u16 ball;
    u8 language;
    metloc_u8_t metLocation;
    u8 isEgg;

    CreateMon(mon, species, EGG_HATCH_LEVEL, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    metLevel = 0;
    ball = ITEM_POKE_BALL;
    language = LANGUAGE_JAPANESE;
    SetMonData(mon, MON_DATA_POKEBALL, &ball);
    SetMonData(mon, MON_DATA_NICKNAME, sJapaneseEggNickname);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &gSpeciesInfo[species].eggCycles);
    SetMonData(mon, MON_DATA_MET_LEVEL, &metLevel);
    SetMonData(mon, MON_DATA_LANGUAGE, &language);
    if (setHotSpringsLocation)
    {
        metLocation = METLOC_SPECIAL_EGG;
        SetMonData(mon, MON_DATA_MET_LOCATION, &metLocation);
    }

    isEgg = TRUE;
    SetMonData(mon, MON_DATA_IS_EGG, &isEgg);
}

static void SetInitialEggData(struct Pokemon *mon, u16 species, struct DayCare *daycare)
{
    u32 personality;
    u16 ball;
    u8 metLevel;
    u8 language;

    personality = daycare->offspringPersonality;
    CreateMon(mon, species, EGG_HATCH_LEVEL, USE_RANDOM_IVS, TRUE, personality, OT_ID_PLAYER_ID, 0);
    metLevel = 0;
    ball = ITEM_POKE_BALL;
    language = LANGUAGE_JAPANESE;
    SetMonData(mon, MON_DATA_POKEBALL, &ball);
    SetMonData(mon, MON_DATA_NICKNAME, sJapaneseEggNickname);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &gSpeciesInfo[species].eggCycles);
    SetMonData(mon, MON_DATA_MET_LEVEL, &metLevel);
    SetMonData(mon, MON_DATA_LANGUAGE, &language);
}

void GiveEggFromDaycare(void)
{
    _GiveEggFromDaycare(&gSaveBlock1Ptr->daycare);
}

static bool8 TryProduceOrHatchEgg(struct DayCare *daycare)
{
    u32 i, validEggs = 0;

    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        if (GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SANITY_HAS_SPECIES))
            daycare->mons[i].steps++, validEggs++;
    }

    // Check if an egg should be produced
    // Moemon Kai: the DAY CARE MAN's EGG check every 32 steps, not every 256 (the user, 2026-11-07: 64 in 1.171.5, 32 in
    // 1.171.6): the same chance each check (the parents' compatibility), eight times as often
    if (daycare->offspringPersonality == 0 && validEggs == DAYCARE_MON_COUNT && (daycare->mons[1].steps & 0x1F) == 0x1F)
    {
        u8 compatibility = GetDaycareCompatibilityScore(daycare);
        bool8 egg = compatibility > (Random() * 100u) / USHRT_MAX;
        MPNet_Log("daycare: EGG check at step %u, %u%% chance: %s", (unsigned)daycare->mons[1].steps,
                  (unsigned)compatibility, egg ? "an EGG" : "none");
        if (egg)
            TriggerPendingDaycareEgg();
    }

    // Try to hatch Egg
    // Moemon Kai: an egg cycle every 32 steps, not every 256: every EGG hatches in an eighth of the time (half in
    // 1.115.0, half again in 1.132.0, the user 2026-11-06; half again in 1.171.6, the user 2026-11-07); FLAME BODY and
    // MAGMA ARMOR still take two cycles at once
    if ((++daycare->stepCounter & 0x1F) == 0x1F)
    {
        u32 eggCycles;
        u8 toSub = GetEggCyclesToSubtract();

        for (i = 0; i < gPlayerPartyCount; i++)
        {
            if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                continue;
            if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_IS_BAD_EGG))
                continue;

            eggCycles = GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP);
            if (eggCycles != 0)
            {
                if (eggCycles >= toSub)
                    eggCycles -= toSub;
                else
                    eggCycles -= 1;

                SetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP, &eggCycles);
            }
            else
            {
                gSpecialVar_0x8004 = i;
                return TRUE;
            }
        }
    }

    return FALSE;
}

bool8 ShouldEggHatch(void)
{
    if (GetBoxMonData(&sRoute5DayCareMon.mon, MON_DATA_SANITY_HAS_SPECIES))
        sRoute5DayCareMon.steps++;   // (Moemon Kai: KANTO's ROUTE 5 DAY-CARE, as FireRed's)
    return TryProduceOrHatchEgg(&gSaveBlock1Ptr->daycare);
}

static bool8 IsEggPending(struct DayCare *daycare)
{
    return (daycare->offspringPersonality != 0);
}

// gStringVar1 = first mon's nickname
// gStringVar2 = second mon's nickname
// gStringVar3 = first mon trainer's name
static void _GetDaycareMonNicknames(struct DayCare *daycare)
{
    u8 otName[max(12, PLAYER_NAME_LENGTH + 1)];
    if (GetBoxMonData(&daycare->mons[0].mon, MON_DATA_SPECIES) != 0)
    {
        GetBoxMonNickname(&daycare->mons[0].mon, gStringVar1);
        GetBoxMonData(&daycare->mons[0].mon, MON_DATA_OT_NAME, otName);
        StringCopy(gStringVar3, otName);
    }

    if (GetBoxMonData(&daycare->mons[1].mon, MON_DATA_SPECIES) != 0)
    {
        GetBoxMonNickname(&daycare->mons[1].mon, gStringVar2);
    }
}

u16 GetSelectedMonNicknameAndSpecies(void)
{
    GetBoxMonNickname(&gPlayerParty[GetCursorSelectionMonId()].box, gStringVar1);
    return GetBoxMonData(&gPlayerParty[GetCursorSelectionMonId()].box, MON_DATA_SPECIES);
}

void GetDaycareMonNicknames(void)
{
    _GetDaycareMonNicknames(&gSaveBlock1Ptr->daycare);
}

u8 GetDaycareState(void)
{
    u8 numMons;
    if (IsEggPending(&gSaveBlock1Ptr->daycare))
    {
        return DAYCARE_EGG_WAITING;
    }

    numMons = CountPokemonInDaycare(&gSaveBlock1Ptr->daycare);
    if (numMons != 0)
    {
        return numMons + 1; // DAYCARE_ONE_MON or DAYCARE_TWO_MONS
    }

    return DAYCARE_NO_MONS;
}

static u8 UNUSED GetDaycarePokemonCount(void)
{
    u8 ret = CountPokemonInDaycare(&gSaveBlock1Ptr->daycare);
    if (ret)
        return ret;

    return 0;
}

// Determine if the two given egg group lists contain any of the
// same egg groups.
static bool8 EggGroupsOverlap(u16 *eggGroups1, u16 *eggGroups2)
{
    s32 i, j;

    for (i = 0; i < EGG_GROUPS_PER_MON; i++)
    {
        for (j = 0; j < EGG_GROUPS_PER_MON; j++)
        {
            if (eggGroups1[i] == eggGroups2[j])
                return TRUE;
        }
    }

    return FALSE;
}

static u8 GetDaycareCompatibilityScore(struct DayCare *daycare)
{
    u32 i;
    u16 eggGroups[DAYCARE_MON_COUNT][EGG_GROUPS_PER_MON];
    u16 species[DAYCARE_MON_COUNT];
    u32 trainerIds[DAYCARE_MON_COUNT];
    u32 genders[DAYCARE_MON_COUNT];

    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        u32 personality;

        species[i] = GetBoxMonData(&daycare->mons[i].mon, MON_DATA_SPECIES);
        trainerIds[i] = GetBoxMonData(&daycare->mons[i].mon, MON_DATA_OT_ID);
        personality = GetBoxMonData(&daycare->mons[i].mon, MON_DATA_PERSONALITY);
        genders[i] = GetGenderFromSpeciesAndPersonality(species[i], personality);
        eggGroups[i][0] = gSpeciesInfo[species[i]].eggGroups[0];
        eggGroups[i][1] = gSpeciesInfo[species[i]].eggGroups[1];
    }

    // check unbreedable egg group
    if (eggGroups[0][0] == EGG_GROUP_NO_EGGS_DISCOVERED || eggGroups[1][0] == EGG_GROUP_NO_EGGS_DISCOVERED)
        return PARENTS_INCOMPATIBLE;
    // two Ditto can't breed
    if (eggGroups[0][0] == EGG_GROUP_DITTO && eggGroups[1][0] == EGG_GROUP_DITTO)
        return PARENTS_INCOMPATIBLE;

    // one parent is Ditto
    if (eggGroups[0][0] == EGG_GROUP_DITTO || eggGroups[1][0] == EGG_GROUP_DITTO)
    {
        if (trainerIds[0] == trainerIds[1])
            return PARENTS_LOW_COMPATIBILITY;

        return PARENTS_MED_COMPATIBILITY;
    }
    // neither parent is Ditto
    else
    {
        if (genders[0] == genders[1])
            return PARENTS_INCOMPATIBLE;
        if (genders[0] == MON_GENDERLESS || genders[1] == MON_GENDERLESS)
            return PARENTS_INCOMPATIBLE;
        if (!EggGroupsOverlap(eggGroups[0], eggGroups[1]))
            return PARENTS_INCOMPATIBLE;

        if (species[0] == species[1])
        {
            if (trainerIds[0] == trainerIds[1])
                return PARENTS_MED_COMPATIBILITY; // same species, same trainer

            return PARENTS_MAX_COMPATIBILITY; // same species, different trainers
        }
        else
        {
            if (trainerIds[0] != trainerIds[1])
                return PARENTS_MED_COMPATIBILITY; // different species, different trainers

            return PARENTS_LOW_COMPATIBILITY; // different species, same trainer
        }
    }
}

static u8 GetDaycareCompatibilityScoreFromSave(void)
{
    return GetDaycareCompatibilityScore(&gSaveBlock1Ptr->daycare);
}

void SetDaycareCompatibilityString(void)
{
    u8 whichString;
    u8 relationshipScore;

    relationshipScore = GetDaycareCompatibilityScoreFromSave();
    whichString = 0;
    if (relationshipScore == PARENTS_INCOMPATIBLE)
        whichString = 3;
    if (relationshipScore == PARENTS_LOW_COMPATIBILITY)
        whichString = 2;
    if (relationshipScore == PARENTS_MED_COMPATIBILITY)
        whichString = 1;
    if (relationshipScore == PARENTS_MAX_COMPATIBILITY)
        whichString = 0;

    StringCopy(gStringVar4, sCompatibilityMessages[whichString]);
}

bool8 NameHasGenderSymbol(const u8 *name, u8 genderRatio)
{
    u8 i;
    u8 symbolsCount[GENDER_COUNT];
    symbolsCount[MALE] = symbolsCount[FEMALE] = 0;

    for (i = 0; name[i] != EOS; i++)
    {
        if (name[i] == CHAR_MALE)
            symbolsCount[MALE]++;
        if (name[i] == CHAR_FEMALE)
            symbolsCount[FEMALE]++;
    }

    if (genderRatio == MON_MALE   && symbolsCount[MALE] != 0 && symbolsCount[FEMALE] == 0)
        return TRUE;
    if (genderRatio == MON_FEMALE && symbolsCount[FEMALE] != 0 && symbolsCount[MALE] == 0)
        return TRUE;

    return FALSE;
}

static u8 *AppendGenderSymbol(u8 *name, u8 gender)
{
    if (gender == MON_MALE)
    {
        if (!NameHasGenderSymbol(name, MON_MALE))
            return StringAppend(name, gText_MaleSymbol4);
    }
    else if (gender == MON_FEMALE)
    {
        if (!NameHasGenderSymbol(name, MON_FEMALE))
            return StringAppend(name, gText_FemaleSymbol4);
    }

    return StringAppend(name, gText_GenderlessSymbol);
}

static u8 *AppendMonGenderSymbol(u8 *name, struct BoxPokemon *boxMon)
{
    return AppendGenderSymbol(name, GetBoxMonGender(boxMon));
}

static void UNUSED GetDaycareLevelMenuText(struct DayCare *daycare, u8 *dest)
{
    u8 monNames[DAYCARE_MON_COUNT][POKEMON_NAME_BUFFER_SIZE];
    u8 i;

    *dest = EOS;
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        GetBoxMonNickname(&daycare->mons[i].mon, monNames[i]);
        AppendMonGenderSymbol(monNames[i], &daycare->mons[i].mon);
    }

    StringCopy(dest, monNames[0]);
    StringAppend(dest, gText_NewLine2);
    StringAppend(dest, monNames[1]);
    StringAppend(dest, gText_NewLine2);
    StringAppend(dest, gText_Exit4);
}

static void UNUSED GetDaycareLevelMenuLevelText(struct DayCare *daycare, u8 *dest)
{
    u8 i;
    u8 level;
    u8 text[20];

    *dest = EOS;
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        StringAppend(dest, gText_Lv);
        level = GetLevelAfterDaycareSteps(&daycare->mons[i].mon, daycare->mons[i].steps);
        ConvertIntToDecimalStringN(text, level, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(dest, text);
        StringAppend(dest, gText_NewLine2);
    }
}

static void DaycareAddTextPrinter(u8 windowId, const u8 *text, u32 x, u32 y)
{
    struct TextPrinterTemplate printer;

    printer.currentChar = text;
    printer.windowId = windowId;
    printer.fontId = FONT_NORMAL;
    printer.x = x;
    printer.y = y;
    printer.currentX = x;
    printer.currentY = y;
    printer.unk = 0;
    gTextFlags.useAlternateDownArrow = 0;
    printer.letterSpacing = 0;
    printer.lineSpacing = 1;
    printer.fgColor = 2;
    printer.bgColor = 1;
    printer.shadowColor = 3;

    AddTextPrinter(&printer, TEXT_SKIP_DRAW, NULL);
}

static void DaycarePrintMonNickname(struct DayCare *daycare, u8 windowId, u32 daycareSlotId, u32 y)
{
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];
    GetBoxMonNickname(&daycare->mons[daycareSlotId].mon, nickname);
    AppendMonGenderSymbol(nickname, &daycare->mons[daycareSlotId].mon);
    DaycareAddTextPrinter(windowId, nickname, 8, y);
}

static void DaycarePrintMonLvl(struct DayCare *daycare, u8 windowId, u32 daycareSlotId, u32 y)
{
    u8 level;
    u32 x;
    u8 lvlText[12];
    u8 intText[8];

    StringCopy(lvlText, gText_Lv);
    level = GetLevelAfterDaycareSteps(&daycare->mons[daycareSlotId].mon, daycare->mons[daycareSlotId].steps);
    ConvertIntToDecimalStringN(intText, level, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(lvlText, intText);
    x = GetStringRightAlignXOffset(FONT_NORMAL, lvlText, 112);
    DaycareAddTextPrinter(windowId, lvlText, x, y);
}

static void DaycarePrintMonInfo(u8 windowId, u32 daycareSlotId, u8 y)
{
    if (daycareSlotId < (unsigned) DAYCARE_MON_COUNT)
    {
        DaycarePrintMonNickname(&gSaveBlock1Ptr->daycare, windowId, daycareSlotId, y);
        DaycarePrintMonLvl(&gSaveBlock1Ptr->daycare, windowId, daycareSlotId, y);
    }
}

#define tMenuListTaskId     data[0]
#define tWindowId           data[1]

static void Task_HandleDaycareLevelMenuInput(u8 taskId)
{
    u32 input = ListMenu_ProcessInput(gTasks[taskId].tMenuListTaskId);

    if (JOY_NEW(A_BUTTON))
    {
        switch (input)
        {
        case 0:
        case 1:
            gSpecialVar_Result = input;
            break;
        case DAYCARE_LEVEL_MENU_EXIT:
            gSpecialVar_Result = DAYCARE_EXITED_LEVEL_MENU;
            break;
        }
        DestroyListMenuTask(gTasks[taskId].tMenuListTaskId, NULL, NULL);
        ClearStdWindowAndFrame(gTasks[taskId].tWindowId, TRUE);
        RemoveWindow(gTasks[taskId].tWindowId);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        gSpecialVar_Result = DAYCARE_EXITED_LEVEL_MENU;
        DestroyListMenuTask(gTasks[taskId].tMenuListTaskId, NULL, NULL);
        ClearStdWindowAndFrame(gTasks[taskId].tWindowId, TRUE);
        RemoveWindow(gTasks[taskId].tWindowId);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

void ShowDaycareLevelMenu(void)
{
    struct ListMenuTemplate menuTemplate;
    u8 windowId;
    u8 listMenuTaskId;
    u8 daycareMenuTaskId;

    windowId = AddWindow(&sDaycareLevelMenuWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    menuTemplate = sDaycareListMenuLevelTemplate;
    menuTemplate.windowId = windowId;
    listMenuTaskId = ListMenuInit(&menuTemplate, 0, 0);

    CopyWindowToVram(windowId, COPYWIN_FULL);

    daycareMenuTaskId = CreateTask(Task_HandleDaycareLevelMenuInput, 3);
    gTasks[daycareMenuTaskId].tMenuListTaskId = listMenuTaskId;
    gTasks[daycareMenuTaskId].tWindowId = windowId;
}

#undef tMenuListTaskId
#undef tWindowId

void ChooseSendDaycareMon(void)
{
    ChooseMonForDaycare();
    gMain.savedCallback = CB2_ReturnToField;
}

// ---------------------------------------------------------------------------
// Moemon Co-op: DAY CARE POKéMON of friends who are online (mp_linkx.c)

// How well two POKéMON get along (0, 20, 50 or 70), as the DAY CARE MAN says
u8 MP_Daycare_Compatibility(const struct BoxPokemon *a, const struct BoxPokemon *b)
{
    struct DayCare tmp;
    memset(&tmp, 0, sizeof(tmp));
    tmp.mons[0].mon = *a;
    tmp.mons[1].mon = *b;
    return GetDaycareCompatibilityScore(&tmp);
}

// Moemon Co-op: the phone's walk and LEVEL training gain EXP as the day care does: levels one by one, new moves as the
// day care teaches them (the oldest forgotten when four are known), stats worked out again
void MP_ApplyDaycareExperience(struct Pokemon *mon)
{
    ApplyDaycareExperience(mon);
}

// Moemon Kai (tests: `mp breedtest N ITEM_A ITEM_B`): party POKéMON 1 and 2 as the parents (A's IVs all 31, B's all 0,
// holding ITEM_A / ITEM_B), N eggs made as the DAY CARE makes them: how many took a parent's Nature, and how many IVs
// each egg took from them (an IV of 31 or 0; a random one is that 1 time in 16)
// (SLOT_A / SLOT_B: other party POKéMON as the parents.) Then (Gen 9's rules) a second line: the eggs' species, how many came in A's / B's POKé BALL (ballA / ballB, when
// given, put them in those), took the main parent's Ability slot, got a POWER item's stat, and know A's / B's first move
static bool8 EggKnows(struct Pokemon *egg, u16 move)
{
    u8 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
        if (move != MOVE_NONE && GetMonData(egg, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    return FALSE;
}

static void SetBoxMonBall(struct BoxPokemon *mon, u16 item)
{
    u8 special = MP_SpecialBall_FromItem(item);
    u32 ball = special ? ITEM_POKE_BALL : item;
    u16 style = (u16)special << 12;
    SetBoxMonData(mon, MON_DATA_POKEBALL, &ball);
    SetBoxMonData(mon, MON_DATA_ENCRYPT_SEPARATOR, &style);
}

void MP_Daycare_BreedTest(int n, u16 itemA, u16 itemB, u16 ballA, u16 ballB, u8 slotA, u8 slotB)
{
    struct DayCare tmp;
    struct Pokemon egg;
    u8 iv, natA, natB;
    int i, k, natureA = 0, natureB = 0, ivTotal = 0, ivMin = 99, ivMax = 0;
    u16 sp[2] = {0, 0};
    int spCount[3] = {0, 0, 0}, inBallA = 0, inBallB = 0, sameAbility = 0, twoAbilities = 0, power = 0;
    int knowsA = 0, knowsB = 0;
    s8 powerA = PowerItemStat(itemA), powerB = PowerItemStat(itemB);
    u16 moveA = GetMonData(&gPlayerParty[slotA], MON_DATA_MOVE1), moveB = GetMonData(&gPlayerParty[slotB], MON_DATA_MOVE1);
    memset(&tmp, 0, sizeof(tmp));
    tmp.mons[0].mon = gPlayerParty[slotA].box;
    tmp.mons[1].mon = gPlayerParty[slotB].box;
    if (ballA)
        SetBoxMonBall(&tmp.mons[0].mon, ballA);
    if (ballB)
        SetBoxMonBall(&tmp.mons[1].mon, ballB);
    ballA = BoxMonBallItem(&tmp.mons[0].mon);
    ballB = BoxMonBallItem(&tmp.mons[1].mon);
    for (k = 0; k < NUM_STATS; k++)
    {
        iv = 31;
        SetBoxMonData(&tmp.mons[0].mon, MON_DATA_HP_IV + k, &iv);
        iv = 0;
        SetBoxMonData(&tmp.mons[1].mon, MON_DATA_HP_IV + k, &iv);
    }
    SetBoxMonData(&tmp.mons[0].mon, MON_DATA_HELD_ITEM, &itemA);
    SetBoxMonData(&tmp.mons[1].mon, MON_DATA_HELD_ITEM, &itemB);
    natA = GetNatureFromPersonality(GetBoxMonData(&tmp.mons[0].mon, MON_DATA_PERSONALITY));
    natB = GetNatureFromPersonality(GetBoxMonData(&tmp.mons[1].mon, MON_DATA_PERSONALITY));
    for (i = 0; i < n; i++)
    {
        int got = 0;
        u8 nat;
        _TriggerPendingDaycareEgg(&tmp);
        BuildDaycareEgg(&tmp, &egg);
        nat = GetNatureFromPersonality(GetMonData(&egg, MON_DATA_PERSONALITY));
        natureA += nat == natA;
        natureB += nat == natB;
        for (k = 0; k < NUM_STATS; k++)
        {
            u8 v = GetMonData(&egg, MON_DATA_HP_IV + k);
            got += v == 31 || v == 0;
        }
        ivTotal += got;
        if (got < ivMin) ivMin = got;
        if (got > ivMax) ivMax = got;
        {
            u16 species = GetMonData(&egg, MON_DATA_SPECIES), ball = BoxMonBallItem(&egg.box);
            if (sp[0] == 0 || sp[0] == species)
                sp[0] = species, spCount[0]++;
            else if (sp[1] == 0 || sp[1] == species)
                sp[1] = species, spCount[1]++;
            else
                spCount[2]++;
            inBallA += ball == ballA;
            inBallB += ball == ballB;
            if (gSpeciesInfo[species].abilities[1] != ABILITY_NONE)
            {
                twoAbilities++;
                sameAbility += GetMonData(&egg, MON_DATA_ABILITY_NUM)
                            == GetBoxMonData(&tmp.mons[EggMainParent(&tmp)].mon, MON_DATA_ABILITY_NUM);
            }
            if (powerA >= 0 && powerB < 0)
                power += GetMonData(&egg, MON_DATA_HP_IV + powerA) == 31;
            else if (powerB >= 0 && powerA < 0)
                power += GetMonData(&egg, MON_DATA_HP_IV + powerB) == 0;
            knowsA += EggKnows(&egg, moveA);
            knowsB += EggKnows(&egg, moveB);
        }
    }
    FlagClear(FLAG_PENDING_DAYCARE_EGG);
    printf("[test] breed %d eggs: natureA %d natureB %d (natures %d %d) ivs avg %d.%02d min %d max %d\n", n, natureA,
           natureB, natA, natB, ivTotal / n, (ivTotal * 100 / n) % 100, ivMin, ivMax);
    printf("[test] breed2 species %d:%d %d:%d other %d ballA %d ballB %d ability %d/%d power %d knowsA %d knowsB %d\n",
           sp[0], spCount[0], sp[1], spCount[1], spCount[2], inBallA, inBallB, sameAbility, twoAbilities, power,
           knowsA, knowsB);
}

// Moemon Kai: UNOVA's DAY CARE (src/unova/u3d_vm.c): Black's scripts on the game's own day care, by slot
void MP_Daycare_Deposit(u8 partySlot)
{
    if (partySlot < PARTY_SIZE)
        StorePokemonInEmptyDaycareSlot(&gPlayerParty[partySlot], &gSaveBlock1Ptr->daycare);
}

u8 MP_Daycare_LevelNow(u8 slot)
{
    if (slot >= DAYCARE_MON_COUNT)
        return 0;
    return GetLevelAfterDaycareSteps(&gSaveBlock1Ptr->daycare.mons[slot].mon, gSaveBlock1Ptr->daycare.mons[slot].steps);
}
