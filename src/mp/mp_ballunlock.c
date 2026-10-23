// Moemon Co-op: which POKé BALL colors and ball effect pieces the player can pick.
//
// The original 11 colors, 12 body forms, 16 particle sets and 7 recalls are
// everyone's. The ones added after them are earned: half by an achievement
// (sByAchievement: it counts as soon as the achievement is done, so a save that
// already has it has the piece too), the other half as MOEGATCHA prizes
// (MP_BallUnlock_Give, kept with the save as "fxu:<kind>:<index>"; a prize the
// player already has turns into a MOEGATCHA TICKET, mp_gatcha.c).
//
// Only choosing is limited: a friend's Moemon (or a traded one) shows its own
// effect whatever this save has unlocked.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"

static const u8 sFirstEarned[MPFX_PIECE_KINDS] = {
    [MPFX_PIECE_COLOR] = MP_BALL_FIRST_EARNED,
    [MPFX_PIECE_FORM] = MPFX_FORM_FIRST_EARNED,
    [MPFX_PIECE_PART] = MPFX_PART_FIRST_EARNED,
    [MPFX_PIECE_RECALL] = MPFX_RECALL_FIRST_EARNED,
};

static const u8 sCounts[MPFX_PIECE_KINDS] = {
    [MPFX_PIECE_COLOR] = MP_BALL_COLORS,
    [MPFX_PIECE_FORM] = MPFX_FORM_COUNT,
    [MPFX_PIECE_PART] = MPFX_PART_COUNT,
    [MPFX_PIECE_RECALL] = MPFX_RECALL_COUNT,
};

static const char sKindLetters[MPFX_PIECE_KINDS] = {'c', 'f', 'p', 'r'};

struct AchUnlock { u8 kind, index; const char *ach; };

// The pieces an achievement unlocks (the rest are MOEGATCHA prizes)
static const struct AchUnlock sByAchievement[] = {
#include "mp_ballunlock_ach.h"
};

u8 MP_BallUnlock_Count(u8 kind)
{
    return kind < MPFX_PIECE_KINDS ? sCounts[kind] : 0;
}

u8 MP_BallUnlock_First(u8 kind)
{
    return kind < MPFX_PIECE_KINDS ? sFirstEarned[kind] : 0;
}

static const char *AchievementFor(u8 kind, u8 index)
{
    u16 i;
    for (i = 0; i < ARRAY_COUNT(sByAchievement); i++)
        if (sByAchievement[i].kind == kind && sByAchievement[i].index == index)
            return sByAchievement[i].ach;
    return NULL;
}

static void Key(char *out, u8 kind, u8 index)
{
    snprintf(out, 16, "fxu:%c:%d", sKindLetters[kind], index);
}

bool8 MP_BallUnlock_Has(u8 kind, u8 index)
{
    char key[16];
    const char *ach;
    if (kind >= MPFX_PIECE_KINDS || index >= sCounts[kind])
        return FALSE;
    if (index < sFirstEarned[kind] || getenv("MOEMON_FX_UNLOCK_ALL"))
        return TRUE;
    if ((ach = AchievementFor(kind, index)) != NULL)
        return MP_Achieve_IsDoneById(ach);
    Key(key, kind, index);
    return MP_SaveData_Get(key) != 0;
}

// A MOEGATCHA prize: FALSE if the player had it already
bool8 MP_BallUnlock_Give(u8 kind, u8 index)
{
    char key[16];
    if (kind >= MPFX_PIECE_KINDS || index >= sCounts[kind] || MP_BallUnlock_Has(kind, index))
        return FALSE;
    Key(key, kind, index);
    MP_SaveData_Set(key, 1);
    MPNet_Log("ball unlock: %c %d", sKindLetters[kind], index);
    return TRUE;
}

// The pieces given as MOEGATCHA prizes (not unlocked by an achievement), in order
u8 MP_BallUnlock_PrizeCount(void)
{
    u8 kind, n = 0;
    u16 i;
    for (kind = 0; kind < MPFX_PIECE_KINDS; kind++)
        for (i = sFirstEarned[kind]; i < sCounts[kind]; i++)
            if (!AchievementFor(kind, i))
                n++;
    return n;
}

bool8 MP_BallUnlock_Prize(u8 n, u8 *kind, u8 *index)
{
    u8 k;
    u16 i;
    for (k = 0; k < MPFX_PIECE_KINDS; k++)
        for (i = sFirstEarned[k]; i < sCounts[k]; i++)
            if (!AchievementFor(k, i) && n-- == 0)
            {
                *kind = k;
                *index = i;
                return TRUE;
            }
    return FALSE;
}

// From cur, the next (dir 1) or previous (dir -1) piece the player has, wrapping around
u8 MP_BallUnlock_Step(u8 kind, u8 cur, s8 dir)
{
    u8 count = MP_BallUnlock_Count(kind), i, v = cur;
    if (count == 0)
        return 0;
    for (i = 0; i < count; i++)
    {
        v = (v + count + dir) % count;
        if (MP_BallUnlock_Has(kind, v))
            return v;
    }
    return cur;
}

// How many of a kind are still to earn
u8 MP_BallUnlock_Missing(u8 kind)
{
    u8 i, n = 0;
    for (i = MP_BallUnlock_First(kind); i < MP_BallUnlock_Count(kind); i++)
        n += !MP_BallUnlock_Has(kind, i);
    return n;
}

// Where a piece comes from: the achievement's name, or MOEGATCHA
void MP_BallUnlock_HowTo(u8 kind, u8 index, char *out, u32 size)
{
    const char *ach = AchievementFor(kind, index);
    if (ach)
        snprintf(out, size, "Achievement: %s", MP_Achieve_NameById(ach));
    else
        snprintf(out, size, "a MOEGATCHA prize");
}

// "BALL COLOR: MINT", "SEND-OUT: SAKURA STORM", "PARTICLES: STARS", "RETURN: WARP OUT"
void MP_BallUnlock_PieceName(u8 kind, u8 index, char *out, u32 size)
{
    char name[24];
    switch (kind)
    {
    case MPFX_PIECE_COLOR:
        MP_GameToAscii(name, MP_BallStyle_ColorName(index), sizeof(name) - 1);
        snprintf(out, size, "BALL COLOR: %s", name);
        break;
    case MPFX_PIECE_FORM:
        snprintf(out, size, "SEND-OUT: %s", MP_BallFx_FormName(index));
        break;
    case MPFX_PIECE_PART:
        snprintf(out, size, "PARTICLES: %s", MP_BallFx_PartName(index));
        break;
    default:
        snprintf(out, size, "RETURN: %s", MP_BallFx_RecallName(index));
        break;
    }
}

// The pieces an achievement unlocks, for its reward text ("BALL COLOR: MINT")
u8 MP_BallUnlock_ForAchievement(const char *ach, u8 *kinds, u8 *indexes, u8 max)
{
    u16 i;
    u8 n = 0;
    for (i = 0; i < ARRAY_COUNT(sByAchievement) && n < max; i++)
        if (strcmp(sByAchievement[i].ach, ach) == 0)
        {
            kinds[n] = sByAchievement[i].kind;
            indexes[n] = sByAchievement[i].index;
            n++;
        }
    return n;
}

// Test harness: "mp fxunlock KIND INDEX" / "mp fxunlock all"
void MP_BallUnlock_Debug(const char *arg)
{
    u8 k;
    u16 i;
    if (strcmp(arg, "all") == 0)
    {
        for (k = 0; k < MPFX_PIECE_KINDS; k++)
            for (i = sFirstEarned[k]; i < sCounts[k]; i++)
                if (!AchievementFor(k, i))
                    MP_BallUnlock_Give(k, i);
    }
    else
    {
        int kind = 0, index = 0;
        sscanf(arg, "%d %d", &kind, &index);
        MP_BallUnlock_Give(kind, index);
    }
    for (k = 0; k < MPFX_PIECE_KINDS; k++)
        printf("[test] ball unlocks %c: %d of %d still to earn\n", sKindLetters[k], MP_BallUnlock_Missing(k),
               MP_BallUnlock_Count(k) - MP_BallUnlock_First(k));
}
