// Moemon Co-op: making up for BAD EGGS.
//
// Up to protocol 23, a co-op wild Pokémon caught by someone other than the host
// became a BAD EGG when it reached its catcher: GiveMonToPlayer overwrote the
// Trainer ID that its data is encrypted with (fixed in pokemon.c,
// MP_SetBoxMonOtIdKeepData). After loading a save, every BAD EGG in the party and
// the PC is removed and its player gets MoeGatcha pulls for it, once per egg (the
// co-op data remembers which ones were paid, in case the game isn't saved after).
#include <stdio.h>
#include "global.h"
#include "mp.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"

#define PULLS_PER_EGG 10

void MP_Gatcha_AddPulls(s32 n);

static bool8 PayFor(struct BoxPokemon *boxMon)
{
    char key[24];
    snprintf(key, sizeof(key), "badegg:%08x", (unsigned)boxMon->personality);
    if (MP_SaveData_Get(key))
        return FALSE;   // paid already (the game wasn't saved after the last time)
    MP_SaveData_Set(key, 1);
    MP_Gatcha_AddPulls(PULLS_PER_EGG);
    return TRUE;
}

void MP_CompensateBadEggs(void)
{
    u8 i, box, pos, removed = 0, paid = 0;
    char msg[96];

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (MP_IsBoxMonBadEgg(&gPlayerParty[i].box))
        {
            paid += PayFor(&gPlayerParty[i].box);
            ZeroMonData(&gPlayerParty[i]);
            removed++;
        }
    }
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
        {
            struct BoxPokemon *boxMon = &MP_PersonalStorage()->boxes[box][pos];
            if (MP_IsBoxMonBadEgg(boxMon))
            {
                paid += PayFor(boxMon);
                ZeroBoxMonData(boxMon);
                removed++;
            }
        }
    }
    if (removed == 0)
        return;

    CompactPartySlots();
    CalculatePlayerPartyCount();
    // A party of only BAD EGGS: the first healthy Pokémon in the PC comes along
    if (gPlayerPartyCount == 0)
    {
        for (box = 0; box < TOTAL_BOXES_COUNT && gPlayerPartyCount == 0; box++)
        {
            for (pos = 0; pos < IN_BOX_COUNT; pos++)
            {
                struct BoxPokemon *boxMon = &MP_PersonalStorage()->boxes[box][pos];
                if (GetBoxMonData(boxMon, MON_DATA_SPECIES) != SPECIES_NONE && !GetBoxMonData(boxMon, MON_DATA_IS_EGG))
                {
                    BoxMonToMon(boxMon, &gPlayerParty[0]);
                    ZeroBoxMonData(boxMon);
                    CalculatePlayerPartyCount();
                    break;
                }
            }
        }
    }

    MPNet_Log("bad eggs: removed %d, paid %d (%d pulls each)", removed, paid, PULLS_PER_EGG);
    if (paid)
        snprintf(msg, sizeof(msg), "%d BAD EGG%s removed.\n+%d MoeGatcha pulls", removed, removed > 1 ? "S" : "",
                 paid * PULLS_PER_EGG);
    else
        snprintf(msg, sizeof(msg), "%d BAD EGG%s removed.", removed, removed > 1 ? "S" : "");
    MP_Events_QueueToast(msg);
}
