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
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "lottery_corner.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "string_util.h"
#include "constants/map_groups.h"
#include "mp.h"

#define MAGIC 0x43534D53   // "SMSC"

static struct
{
    u32 magic;
    u16 storeBuys;
    u16 unused[15];
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
    }
}
