// Moemon Kai: small Platinum commands on Kai's own systems (`sinnoh SINNOH_MISC, OP, A, B`, tools/sinnoh/gen_scripts.py),
// what they keep in "<save>.region" chunk SMSC (mp_region.c):
//   0 GetJubilifeLotteryTrainerID A: the day's lottery number, the game's own (lottery_corner.c: HOENN's LOTTERY
//     CORNER draws the same one)
//   1 CheckForJubilifeLotteryWinner: A the first of three variables (Platinum's: the winning POKéMON's place, its
//     matching digits, 1 if in a box), B the number drawn; the player's party, then the save's boxes, as Platinum's
//   2 RandomizeJubilifeLottery: a new number
//   3 CheckIsDepartmentStoreRegular A: 5 buys or more in the VEILSTONE DEPARTMENT STORE (Sinnoh_StoreBought, shop.c)
//   4 BufferVarPaddingDigits: A a variable, B the text slot << 8 | 0x80 for zeroes | the digits
//   5 BufferMonNicknameFromPC: A a variable (box * 30 + place), B the text slot
//   6 CalcSizeContestResult: A the result's variable (Platinum's SIZE_CONTEST_SMALLER / SAME_SIZE / LARGER, 0-2), B the
//     party slot's: the POKéMON against ROUTE 222's REMORAID record, measured as Emerald's SEEDOT / LOTAD
//     (pokemon_size_record.c, its hash in remoraidRecord; 0: none yet, Platinum's starting record 33280)
//   7 BufferPartyPokemonSize: A the party slot's variable, B the text slots (whole inches << 8 | tenths)
//   8 UpdateSizeContestRecord: A the party slot's variable: its POKéMON is the new record
//   9 BufferSizeContestRecord: B the text slots (as 7) for the record (A: the species, REMORAID)
//   AMITY SQUARE (Platinum's scrcmd_amity_square.c; every POKéMON follows in this game, so the square's walker is the
//   player's own follower: tools/sinnoh/gen_scripts.py KAI_REWRITES):
//   10 the follower's party slot into A, its species into B (none: slot 0, SPECIES_NONE)
//   11 ClearAmitySquareStepCount; 12 GetAmitySquareStepCount A (the steps in the square since, Sinnoh_MiscStep)
//   13 CalcAmitySquareFoundAccessory: A the accessory found, B the species' variable (Platinum's pools and odds)
//   16 out of the square (its walker no more)
//   19 CheckABPress A; 20 CheckMetFurnitureRequirements: A the order form's choice (the furniture + 1), B the answer
//      (Platinum's ov5_021F6454.c on the game's records: the frontier's best streaks, HALL OF FAME entries, BERRIES
//      planted, eggs hatched, steps; the FIGHT AREA's battleground battles counted by 21)
//   22 BufferValuePaddingDigits: A a number (its low 16 bits, after 23 A: its high ones, for this one number), B as 4's
//   24 ClearHasPartner: SINNOH's walking partner's walk ends (s3d_scrcmd.c Sinnoh_PartnerUpdate): its graphics var
//      cleared, the one following the player removed (the map's own object of it, if any, takes over)
//   21 a battle in the FIGHT AREA's battleground (Platinum's RECORD_TIMES_BATTLED_AT_BATTLEGROUND)
//   17 GetCurrentBGM A: the song playing (the VILLA's music box keeps it), 18 SetFieldScene A: that song again
//   14 CheckAmitySquareManGiftIsAccessory: A the gift's number, B the answer; 15 GetAmitySquareBerryOrAccessoryIDFromMan:
//      A the gift's number, B its item or accessory
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "main.h"
#include "overworld.h"
#include "lottery_corner.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/map_groups.h"
#include "mp.h"
#include "mp_life_field.h"
#include "event_object_movement.h"
#include "mp_region.h"
#include "constants/event_object_movement.h"
#include "m4a.h"
#include "mp_kaisong.h"
#include "sound.h"
#include "constants/game_stat.h"
#include "constants/frontier_util.h"

#define MAGIC 0x43534D53   // "SMSC"

static struct
{
    u32 magic;
    u16 storeBuys;
    u16 remoraidRecord;   // (ROUTE 222's size contest: the record's size hash, 0 = none yet)
    u16 amitySteps;       // (AMITY SQUARE: steps since the follower's last find)
    u16 amityWalking;     // (AMITY SQUARE: the follower is the square's walker, from a gate in to a gate out)
    u16 battleground;     // (the FIGHT AREA's battleground: battles there, for the VILLA's GUEST SET)
    u16 unused[11];
} sM;

const void *Sinnoh_MiscData(u32 *size)
{
    sM.magic = MAGIC;
    *size = sizeof(sM);
    return &sM;
}

void Sinnoh_MiscLoad(const void *data, u32 size)
{
    memset(&sM, 0, sizeof(sM));
    if (size == sizeof(sM) && ((const u32 *)data)[0] == MAGIC)
        memcpy(&sM, data, size);
}

void Sinnoh_MiscReset(void)
{
    memset(&sM, 0, sizeof(sM));
}

// (shop.c, a purchase) one more buy in the VEILSTONE DEPARTMENT STORE
void Sinnoh_StoreBought(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    if (map >= MAP_SINNOH_VEILSTONE_STORE1F && map <= MAP_SINNOH_VEILSTONE_STORE_ELEVATOR && sM.storeBuys < 0xFFFF)
        sM.storeBuys++;
}

// (Platinum's CheckTrainerIdForMatch) the digits matching from the last one
static u8 Matching(u16 drawn, u16 otId)
{
    u8 n = 0, i;
    for (i = 0; i < 5; i++)
    {
        if (drawn % 10 != otId % 10)
            break;
        drawn /= 10;
        otId /= 10;
        n++;
    }
    return n;
}

u16 MP_MonSizeHash(struct Pokemon *mon);
u32 MP_MonSizeTenthsInch(u16 species, u16 hash);
extern u8 gSinnohStrVar[3][64];

static u8 *TextSlot(u8 i)
{
    return i == 0 ? gStringVar1 : i == 1 ? gStringVar2 : i == 2 ? gStringVar3 : gSinnohStrVar[(i - 3) % 3];
}

static u16 RemoraidRecord(void)
{
    return sM.remoraidRecord != 0 ? sM.remoraidRecord : 33280;   // (Platinum's InitSizeContestRecord)
}

static void BufferSize(u32 tenths, u16 slots)
{
    ConvertIntToDecimalStringN(TextSlot(slots >> 8), tenths / 10, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(TextSlot(slots & 0xFF), tenths % 10, STR_CONV_MODE_LEFT_ALIGN, 1);
}

// Platinum's ACCESSORY_* (include/constants/accessories.h), the ones AMITY SQUARE gives
enum
{
    A_WHITE_FLUFF, A_YELLOW_FLUFF, A_PINK_FLUFF, A_BROWN_FLUFF, A_BLACK_FLUFF, A_ORANGE_FLUFF, A_ROUND_PEBBLE,
    A_GLITTER_BOULDER, A_SNAGGY_PEBBLE, A_JAGGED_BOULDER, A_BLACK_PEBBLE, A_MINI_PEBBLE, A_PINK_SCALE, A_BLUE_SCALE,
    A_GREEN_SCALE, A_PURPLE_SCALE, A_BIG_SCALE, A_NARROW_SCALE, A_BLUE_FEATHER, A_RED_FEATHER, A_YELLOW_FEATHER,
    A_WHITE_FEATHER, A_BLACK_MOUSTACHE, A_WHITE_MOUSTACHE, A_BLACK_BEARD, A_WHITE_BEARD, A_SMALL_LEAF, A_BIG_LEAF,
    A_NARROW_LEAF, A_SHED_CLAW, A_SHED_HORN, A_THIN_MUSHROOM, A_THICK_MUSHROOM, A_STUMP,
};

// (Platinum's sMonFindableAccessories: by the follower's kind, ten finds from common to rare)
static const u8 sAmityFinds[6][10] = {
    {A_WHITE_FLUFF, A_ORANGE_FLUFF, A_WHITE_FEATHER, A_ROUND_PEBBLE, A_SMALL_LEAF, A_BLUE_SCALE, A_WHITE_BEARD,
     A_THIN_MUSHROOM, A_BIG_SCALE, A_STUMP},
    {A_PINK_FLUFF, A_RED_FEATHER, A_YELLOW_FEATHER, A_BLACK_BEARD, A_NARROW_SCALE, A_WHITE_FLUFF, A_WHITE_MOUSTACHE,
     A_SHED_CLAW, A_NARROW_LEAF, A_PURPLE_SCALE},
    {A_JAGGED_BOULDER, A_SNAGGY_PEBBLE, A_BROWN_FLUFF, A_ROUND_PEBBLE, A_BLACK_MOUSTACHE, A_SHED_HORN, A_NARROW_SCALE,
     A_GLITTER_BOULDER, A_GREEN_SCALE, A_THICK_MUSHROOM},
    {A_PINK_SCALE, A_BLUE_FEATHER, A_WHITE_FEATHER, A_BLUE_SCALE, A_THIN_MUSHROOM, A_SNAGGY_PEBBLE, A_WHITE_BEARD,
     A_BLACK_BEARD, A_BIG_LEAF, A_GLITTER_BOULDER},
    {A_ORANGE_FLUFF, A_BROWN_FLUFF, A_SMALL_LEAF, A_RED_FEATHER, A_YELLOW_FEATHER, A_YELLOW_FLUFF, A_MINI_PEBBLE,
     A_BIG_SCALE, A_BLACK_MOUSTACHE, A_PURPLE_SCALE},
    {A_PINK_SCALE, A_SHED_HORN, A_PINK_FLUFF, A_YELLOW_FEATHER, A_SHED_CLAW, A_BLACK_FLUFF, A_JAGGED_BOULDER,
     A_BIG_LEAF, A_GREEN_SCALE, A_BLACK_PEBBLE},
};

static u8 AmityPool(u16 species)
{
    switch (species)
    {
    case SPECIES_PIPLUP: case SPECIES_PRINPLUP: case SPECIES_EMPOLEON: return 1;
    case SPECIES_TURTWIG: case SPECIES_GROTLE: case SPECIES_TORTERRA: return 2;
    case SPECIES_PSYDUCK: case SPECIES_BUNEARY: case SPECIES_DRIFLOON: return 3;
    case SPECIES_PIKACHU: case SPECIES_CLEFAIRY: case SPECIES_PACHIRISU: case SPECIES_HAPPINY: return 4;
    case SPECIES_JIGGLYPUFF: case SPECIES_TORCHIC: case SPECIES_SKITTY: case SPECIES_SHROOMISH: return 5;
    }
    return 0;   // (CHIMCHAR's line, and any other POKéMON: Platinum's default)
}

// (Platinum's sBerryAndAccessoryManOptions: 0-8 BERRIES, 9-15 accessories)
static const u16 sAmityGifts[16] = {
    ITEM_MAGOST_BERRY, ITEM_CORNN_BERRY, ITEM_RABUTA_BERRY, ITEM_NOMEL_BERRY, ITEM_SPELON_BERRY, ITEM_PAMTRE_BERRY,
    ITEM_WATMEL_BERRY, ITEM_DURIN_BERRY, ITEM_BELUE_BERRY,
    A_MINI_PEBBLE, A_GLITTER_BOULDER, A_BLACK_PEBBLE, A_BIG_SCALE, A_BIG_LEAF, A_THICK_MUSHROOM, A_STUMP,
};

static bool8 InAmitySquare(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_SINNOH_AMITY_SQUARE)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_SINNOH_AMITY_SQUARE);
}

// (mp_life_interact.c, A in front of the follower) in AMITY SQUARE, the walker's own script: Platinum's AmitySquare_FollowerMon
// (its find every 200 steps, or a line); NULL elsewhere (the follower's usual words)
extern const u8 SinnohP_amity_square_6[] __attribute__((weak));
const u8 *Sinnoh_FollowerTalkScript(void)
{
    if (!InAmitySquare() || !sM.amityWalking)
        return NULL;
    MPNet_Log("amity: the walker's script (%u steps)", sM.amitySteps);
    return SinnohP_amity_square_6;
}

// (field_control_avatar.c, each step) AMITY SQUARE counts its walker's steps
void Sinnoh_MiscStep(void)
{
    if (InAmitySquare() && sM.amityWalking && sM.amitySteps < 0xFFFF)
        sM.amitySteps++;
}

// Platinum's ScrCmd_CheckMetFurnitureRequirements: the furniture that waits on the player's records (the frontier's:
// Platinum asks one win at each of the five facilities for the bust, a silver print at any for the silver one; the
// game's facilities keep their best streaks, and a Brain stands at the 21st battle of a streak: Platinum's silver print)
u16 Sinnoh_FrontierBest(u8 facility);
u16 Sinnoh_HallBest(void);

static u16 BestTower(void)
{
    u32 i, j, best = 0;
    for (i = 0; i < 4; i++)
        for (j = 0; j < FRONTIER_LVL_MODE_COUNT; j++)
            if (gSaveBlock2Ptr->frontier.towerRecordWinStreaks[i][j] > best)
                best = gSaveBlock2Ptr->frontier.towerRecordWinStreaks[i][j];
    return best;
}

static u16 BestFactory(void)
{
    u32 i, j, best = 0;
    for (i = 0; i < 2; i++)
        for (j = 0; j < FRONTIER_LVL_MODE_COUNT; j++)
            if (gSaveBlock2Ptr->frontier.factoryRecordWinStreaks[i][j] > best)
                best = gSaveBlock2Ptr->frontier.factoryRecordWinStreaks[i][j];
    return best;
}

static bool8 VillaFurnitureAllowed(u16 choice)
{
    u16 best[5] = {BestTower(), BestFactory(), Sinnoh_HallBest(), Sinnoh_FrontierBest(0), Sinnoh_FrontierBest(1)};
    u32 i;
    switch (choice)
    {
    case 12 + 1:   // the POKéMON BUST: a win at each facility
        for (i = 0; i < ARRAY_COUNT(best); i++)
            if (best[i] < 1)
                return FALSE;
        return TRUE;
    case 13 + 1:   // the silver one: a Brain beaten at any (the 21st battle of a streak)
        for (i = 0; i < ARRAY_COUNT(best); i++)
            if (best[i] >= 21)
                return TRUE;
        return FALSE;
    case 14 + 1: return GetGameStat(GAME_STAT_ENTERED_HOF) >= 10;      // the PIANO
    case 15 + 1: return sM.battleground >= 50;                         // the GUEST SET
    case 16 + 1: return GetGameStat(GAME_STAT_PLANTED_BERRIES) >= 50;  // the WALL CLOCK
    case 17 + 1: return GetGameStat(GAME_STAT_HATCHED_EGGS) >= 30;     // the MASTERPIECE
    case 19 + 1: return GetGameStat(GAME_STAT_STEPS) >= 300000;        // the CHANDELIER
    }
    return TRUE;
}

static struct Pokemon *PartySlotMon(u16 var)
{
    return &gPlayerParty[VarGet(var) % PARTY_SIZE];
}

static u16 sHigh;   // (22's number's high half, from 23)

void Sinnoh_MiscCall(u16 op, u16 a, u16 b)
{
    switch (op)
    {
    case 0:
        VarSet(a, (u16)GetLotteryNumber());
        break;
    case 1:
    {
        u16 drawn = VarGet(b), bestParty = 0, partySlot = 0, bestBox = 0, boxSlot = 0;
        u32 i, j;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            u8 n;
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
                continue;
            n = Matching(drawn, GetMonData(mon, MON_DATA_OT_ID) & 0xFFFF);
            if (n > bestParty)
                bestParty = n, partySlot = i;
        }
        for (i = 0; i < TOTAL_BOXES_COUNT; i++)
            for (j = 0; j < IN_BOX_COUNT; j++)
            {
                struct BoxPokemon *mon = &gPokemonStoragePtr->boxes[i][j];
                u8 n;
                if (GetBoxMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetBoxMonData(mon, MON_DATA_IS_EGG))
                    continue;
                n = Matching(drawn, GetBoxMonData(mon, MON_DATA_OT_ID) & 0xFFFF);
                if (n > bestBox)
                    bestBox = n, boxSlot = i * IN_BOX_COUNT + j;
            }
        if (bestParty == 0 && bestBox == 0)
            VarSet(a, 0), VarSet(a + 1, 0), VarSet(a + 2, 0);
        else if (bestParty >= bestBox)
            VarSet(a, partySlot), VarSet(a + 1, bestParty), VarSet(a + 2, 0);
        else
            VarSet(a, boxSlot), VarSet(a + 1, bestBox), VarSet(a + 2, 1);
        MPNet_Log("sinnoh lottery: %u drawn, %u digits", drawn, VarGet(a + 1));
        break;
    }
    case 2:
        SetLotteryNumber((Random() << 16) | Random());
        break;
    case 3:
        VarSet(a, sM.storeBuys >= 5);
        break;
    case 4:
    {
        static u8 *const sSlots[] = {gStringVar1, gStringVar2, gStringVar3};
        ConvertIntToDecimalStringN(sSlots[(b >> 8) % 3], VarGet(a),
                                   (b & 0x80) ? STR_CONV_MODE_LEADING_ZEROS : STR_CONV_MODE_RIGHT_ALIGN, b & 0x7F);
        break;
    }
    case 5:
    {
        static u8 *const sSlots[] = {gStringVar1, gStringVar2, gStringVar3};
        u16 n = VarGet(a) % (TOTAL_BOXES_COUNT * IN_BOX_COUNT);
        GetBoxMonData(&gPokemonStoragePtr->boxes[n / IN_BOX_COUNT][n % IN_BOX_COUNT], MON_DATA_NICKNAME, sSlots[b % 3]);
        StringGet_Nickname(sSlots[b % 3]);
        break;
    }
    case 6:
    {
        u32 mine = MP_MonSizeTenthsInch(SPECIES_REMORAID, MP_MonSizeHash(PartySlotMon(b)));
        u32 record = MP_MonSizeTenthsInch(SPECIES_REMORAID, RemoraidRecord());
        VarSet(a, mine < record ? 0 : mine == record ? 1 : 2);
        MPNet_Log("sinnoh size contest: %u.%u in against the record %u.%u", mine / 10, mine % 10, record / 10, record % 10);
        break;
    }
    case 7:
        BufferSize(MP_MonSizeTenthsInch(SPECIES_REMORAID, MP_MonSizeHash(PartySlotMon(a))), b);
        break;
    case 8:
        sM.remoraidRecord = MP_MonSizeHash(PartySlotMon(a));
        if (sM.remoraidRecord == 0)
            sM.remoraidRecord = 1;   // (0 stands for none)
        break;
    case 9:
        BufferSize(MP_MonSizeTenthsInch(SPECIES_REMORAID, RemoraidRecord()), b);
        break;
    case 10:
    {
        u8 slot = MP_Follower_LocalPartySlot();
        VarSet(a, slot < PARTY_SIZE ? slot : 0);
        VarSet(b, slot < PARTY_SIZE ? GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES) : SPECIES_NONE);
        sM.amityWalking = slot < PARTY_SIZE;
        break;
    }
    case 16:
        sM.amityWalking = FALSE;
        break;
    case 17:
        VarSet(a, MP_Kaisong_SongOf(gMPlayInfo_BGM.songHeader));
        break;
    case 18:
        if (VarGet(a) != 0)
            PlayBGM(VarGet(a));
        break;
    case 19:
        VarSet(a, JOY_NEW(A_BUTTON | B_BUTTON) ? TRUE : FALSE);
        break;
    case 20:
        VarSet(b, VillaFurnitureAllowed(VarGet(a)));
        break;
    case 22:
    {
        static u8 *const sSlots[] = {gStringVar1, gStringVar2, gStringVar3};
        ConvertIntToDecimalStringN(sSlots[(b >> 8) % 3], ((u32)sHigh << 16) | a,
                                   (b & 0x80) ? STR_CONV_MODE_LEADING_ZEROS : STR_CONV_MODE_RIGHT_ALIGN, b & 0x7F);
        sHigh = 0;
        break;
    }
    case 23:
        sHigh = a;
        break;
    case 24:
    {
        // ClearHasPartner: the walk ends (Platinum's scripts then set the partner's own movement, walk them off or
        // remove them by their own local id): the one following stops where they are, facing the way they face
        u8 obj = GetFollowPlayerObjectId();
        VarSet(VAR_SINNOH_PARTNER_GFX, 0);
        if (obj < OBJECT_EVENTS_COUNT)
        {
            static const u8 sFace[] = {MOVEMENT_TYPE_FACE_DOWN, MOVEMENT_TYPE_FACE_DOWN, MOVEMENT_TYPE_FACE_UP,
                                       MOVEMENT_TYPE_FACE_LEFT, MOVEMENT_TYPE_FACE_RIGHT};
            u8 d = gObjectEvents[obj].facingDirection;
            SetTrainerMovementType(&gObjectEvents[obj], sFace[d <= DIR_EAST ? d : 0]);
        }
        MPNet_Log("sinnoh partner: the walk ends");
        break;
    }
    case 25:
        // (the BATTLE TOWER's BP, Platinum's line: the player's name, then the points frontier_givepoints wrote)
        StringCopy(gStringVar2, gStringVar1);
        StringCopy(gStringVar1, gSaveBlock2Ptr->playerName);
        break;
    case 21:
        if (sM.battleground < 0xFFFF)
            sM.battleground++;
        break;
    case 11:
        sM.amitySteps = 0;
        break;
    case 12:
        VarSet(a, sM.amitySteps);
        break;
    case 13:
    {
        static const u8 sOdds[10] = {15, 30, 45, 60, 70, 80, 88, 93, 98, 100};   // (Platinum's: 15% .. 2%)
        u16 r = Random() % 100;
        u8 i = 0;
        while (r >= sOdds[i])
            i++;
        VarSet(a, sAmityFinds[AmityPool(VarGet(b))][i]);
        break;
    }
    case 14:
        VarSet(b, VarGet(a) >= 9);
        break;
    case 15:
        VarSet(b, sAmityGifts[VarGet(a) % ARRAY_COUNT(sAmityGifts)]);
        break;
    }
}

// Tests (mp amity [STEPS]): AMITY SQUARE's walker and its steps (STEPS: set them)
void Sinnoh_AmityDebug(const char *arg)
{
    if (arg != NULL && *arg >= '0' && *arg <= '9')
        sM.amitySteps = atoi(arg);
    MPNet_Log("amity: walking %u steps %u slot %u", sM.amityWalking, sM.amitySteps, MP_Follower_LocalPartySlot());
}

// ---------------------------------------------------------------------------------------------------- ROCK CLIMB
// SINNOH's rock walls (Platinum's ROCK_CLIMB_N_S / E_W behaviors kept as the game's unused 0x4B / 0x4C, solid in the
// pack: tools/sinnoh/build_pack.py): A in front of one asks Platinum's question (data/scripts/sinnoh_rock_climb.inc)
// when a party POKeMON knows ROCK CLIMB or holds HM ROCK CLIMB (or a friend lends it: MP_PartyCanUseFieldMove) and the
// ICICLE BADGE is the player's (Platinum's FieldMoves_RockyWall); the player then goes up or down the wall the way they
// face, one step past it.
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "constants/metatile_behaviors.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "s3d.h"
#include "party_menu.h"
#include "mp.h"
#include "mp_region.h"

bool8 MP_PartyCanUseFieldMove(u16 move);   // (mp_region.c)

// (special) VAR_RESULT: the party slot of the POKeMON that climbs (PARTY_SIZE: none, or no ICICLE BADGE); its
// nickname in gStringVar1
void S3D_RockClimbSlot(void)
{
    u32 i, slot = PARTY_SIZE;
    if (FlagGet(MP_Region_BadgeFlag(REGION_SINNOH, 6)) && MP_PartyCanUseFieldMove(MOVE_ROCK_CLIMB))
    {
        for (i = 0; i < PARTY_SIZE && slot == PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
             && (MonKnowsMove(&gPlayerParty[i], MOVE_ROCK_CLIMB) || MP_HeldHmMove(&gPlayerParty[i]) == MOVE_ROCK_CLIMB))
                slot = i;
        if (slot == PARTY_SIZE)
            slot = 0;   // (a friend's, lent: the first POKeMON climbs with it)
        GetMonData(&gPlayerParty[slot], MON_DATA_NICKNAME, gStringVar1);
        StringGet_Nickname(gStringVar1);
    }
    gSpecialVar_Result = slot;
}

static bool8 IsRockWall(s16 x, s16 y)
{
    u8 b = MapGridGetMetatileBehaviorAt(x, y);
    return b == MB_UNUSED_4B || b == MB_UNUSED_4C;
}

static void Task_RockClimb(u8 taskId)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 *data = gTasks[taskId].data;   // 0: direction, 1: steps taken, 2: the step past the wall taken
    s16 x, y;
    if (!ObjectEventClearHeldMovementIfFinished(player) && data[1] > 0)
        return;
    if (data[2])
    {
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    x = player->currentCoords.x, y = player->currentCoords.y;
    MoveCoords(data[0], &x, &y);
    if (!IsRockWall(x, y))
        data[2] = 1;   // (off the wall: this is the last step)
    else if (data[1] % 2 == 0)
        PlaySE(SE_M_ROCK_THROW);
    ObjectEventSetHeldMovement(player, GetWalkNormalMovementAction(data[0]));
    data[1]++;
    if (data[1] > 64)   // (never stuck on a wall that doesn't end)
        data[2] = 1;
}

// (special, waitstate) climbs the wall in front of the player, the way they face
void S3D_RockClimbStart(void)
{
    u8 taskId = CreateTask(Task_RockClimb, 80);
    gTasks[taskId].data[0] = GetPlayerFacingDirection();
    gTasks[taskId].data[1] = 0;
    gTasks[taskId].data[2] = 0;
}

// A step onto Platinum's bike slopes (0xD9 its top, 0xDA its bottom: the pack keeps the numbers, the game's unused ones)
// uphill: only on a bike (Platinum: walking up slides back down). Downhill and across are free.
bool8 S3D_BikeSlopeBlocked(s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    u8 b = MapGridGetMetatileBehaviorAt(toX, toY);
    float h, f;
    if (b != MB_UNUSED_D9 && b != MB_UNUSED_DA)
        return FALSE;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE))
        return FALSE;
    h = S3D_PlayerHeight();
    f = S3D_TileFloor(toX - MAP_OFFSET, toY - MAP_OFFSET, isnan(h) ? 0.0f : h);
    if (getenv("MOEMON_GYM_TRACE"))
        printf("[gym] bike slope: avatar 0x%X, player at %.1f, slope at %.1f\n", gPlayerAvatar.flags, h, f);
    if (isnan(h) || isnan(f))
        return TRUE;
    return f > h + 2.0f;
}

// ROCK CLIMB and DEFOG from the party menu (Platinum's FieldMoves_UseRockClimbFromMenu / UseDefogFromMenu): ROCK CLIMB
// facing one of SINNOH's rock walls; DEFOG where a deep fog lies (the map's fog weather; it clears until the map changes)
#include "field_weather.h"
#include "constants/weather.h"
extern const u8 EventScript_SinnohRockClimbFromMenu[];
extern const u8 EventScript_SinnohDefog[];

static void BufferMenuMon(void)
{
    gSpecialVar_0x8004 = GetCursorSelectionMonId();
    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
}

static void FieldCallback_RockClimb(void)
{
    BufferMenuMon();
    ScriptContext_SetupScript(EventScript_SinnohRockClimbFromMenu);
}

bool8 SetUpFieldMove_RockClimb(void)
{
    s16 x, y;
    if (!S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) || !FlagGet(MP_Region_BadgeFlag(REGION_SINNOH, 6)))
        return FALSE;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    if (!IsRockWall(x, y))
        return FALSE;
    gFieldCallback2 = FieldCallback_PrepareFadeInFromMenu;
    gPostMenuFieldCallback = FieldCallback_RockClimb;
    return TRUE;
}

static void FieldCallback_Defog(void)
{
    BufferMenuMon();
    ScriptContext_SetupScript(EventScript_SinnohDefog);
}

bool8 SetUpFieldMove_Defog(void)
{
    u8 w = GetCurrentWeather();
    if (w != WEATHER_FOG_HORIZONTAL && w != WEATHER_FOG_DIAGONAL)
        return FALSE;
    gFieldCallback2 = FieldCallback_PrepareFadeInFromMenu;
    gPostMenuFieldCallback = FieldCallback_Defog;
    return TRUE;
}

// (special) DEFOG blows the fog away: clear skies until the map changes
void S3D_DefogClear(void)
{
    SetWeather(WEATHER_NONE);
}
