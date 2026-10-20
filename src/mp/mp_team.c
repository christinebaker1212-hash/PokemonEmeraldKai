// Moemon Co-op: the co-op team. Up to 3 Pokémon marked 1-2-3 in the party menu
// (TEAM). Co-op Double Battles bring all three, Triple Battles the first two.
// Stored in the save's .coop file as personality values ("team1".."team3"),
// so the marks follow the Pokémon when the party is reordered. Members that
// left the party or can't battle are skipped; the rest of the battle team is
// filled with the first usable Pokémon in party order (with no team set that
// is simply the first 3 usable, the old behavior).
#include "global.h"
#include "pokemon.h"
#include "mp.h"

static const char *const sKeys[MP_TEAM_SIZE] = {"team1", "team2", "team3"};

static u32 Personality(u8 slot)
{
    return GetMonData(&gPlayerParty[slot], MON_DATA_PERSONALITY);
}

static bool8 IsTeamCandidate(u8 slot)
{
    return slot < PARTY_SIZE
        && GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES) != SPECIES_NONE
        && !GetMonData(&gPlayerParty[slot], MON_DATA_IS_EGG);
}

// The marked party slots in team order (0xFF where a member isn't in the party).
static void MarkedSlots(u8 out[MP_TEAM_SIZE])
{
    u8 i, j, k;
    bool8 used[PARTY_SIZE] = {0};

    for (i = 0; i < MP_TEAM_SIZE; i++)
    {
        u32 p = (u32)MP_SaveData_Get(sKeys[i]);
        out[i] = 0xFF;
        if (p == 0)
            continue;
        for (j = 0; j < PARTY_SIZE; j++)
        {
            if (!used[j] && IsTeamCandidate(j) && Personality(j) == p)
            {
                out[i] = j;
                used[j] = TRUE;
                break;
            }
        }
        // the same Pokémon marked twice (a stale entry): keep the first
        for (k = 0; k < i; k++)
            if (out[k] == out[i])
                out[i] = 0xFF;
    }
}

// 1-3 = this party slot's number in the co-op team, 0 = not in it
u8 MP_Team_Number(u8 slot)
{
    u8 marked[MP_TEAM_SIZE], i, n = 0;

    if (!IsTeamCandidate(slot))
        return 0;
    MarkedSlots(marked);
    // numbers count only members still in the party, so they stay 1, 2, 3
    for (i = 0; i < MP_TEAM_SIZE; i++)
    {
        if (marked[i] == 0xFF)
            continue;
        n++;
        if (marked[i] == slot)
            return n;
    }
    return 0;
}

u8 MP_Team_Count(void)
{
    u8 marked[MP_TEAM_SIZE], i, n = 0;

    MarkedSlots(marked);
    for (i = 0; i < MP_TEAM_SIZE; i++)
        if (marked[i] != 0xFF)
            n++;
    return n;
}

static void Store(const u8 marked[MP_TEAM_SIZE])
{
    u8 i, n = 0;

    for (i = 0; i < MP_TEAM_SIZE; i++)
        if (marked[i] != 0xFF)
            MP_SaveData_Set(sKeys[n++], (s32)Personality(marked[i]));
    for (; n < MP_TEAM_SIZE; n++)
        MP_SaveData_Set(sKeys[n], 0);
}

// TEAM in the party menu. Returns the new number (1-3), 0 if it was taken off
// the team, or MP_TEAM_FULL.
u8 MP_Team_Toggle(u8 slot)
{
    u8 marked[MP_TEAM_SIZE], i;

    if (!IsTeamCandidate(slot))
        return 0;
    MarkedSlots(marked);
    for (i = 0; i < MP_TEAM_SIZE; i++)
    {
        if (marked[i] == slot)
        {
            marked[i] = 0xFF;
            Store(marked);
            return 0;
        }
    }
    for (i = 0; i < MP_TEAM_SIZE; i++)
    {
        if (marked[i] == 0xFF)
        {
            // compact first so the new member gets the next number
            u8 j, n = 0, tmp[MP_TEAM_SIZE];
            for (j = 0; j < MP_TEAM_SIZE; j++)
                if (marked[j] != 0xFF)
                    tmp[n++] = marked[j];
            tmp[n] = slot;
            for (j = n + 1; j < MP_TEAM_SIZE; j++)
                tmp[j] = 0xFF;
            Store(tmp);
            return n + 1;
        }
    }
    return MP_TEAM_FULL;
}

// The party slots to bring into a co-op battle, in order: usable team members
// first, then the first usable others. Returns how many (at most max).
u8 MP_Team_BattleSlots(u8 *out, u8 max, bool8 (*usable)(struct Pokemon *))
{
    u8 marked[MP_TEAM_SIZE], i, n = 0;
    bool8 taken[PARTY_SIZE] = {0};

    MarkedSlots(marked);
    for (i = 0; i < MP_TEAM_SIZE && n < max; i++)
    {
        if (marked[i] != 0xFF && usable(&gPlayerParty[marked[i]]))
        {
            out[n++] = marked[i];
            taken[marked[i]] = TRUE;
        }
    }
    for (i = 0; i < PARTY_SIZE && n < max; i++)
    {
        if (!taken[i] && usable(&gPlayerParty[i]))
        {
            out[n++] = i;
            taken[i] = TRUE;
        }
    }
    return n;
}
