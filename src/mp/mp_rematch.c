// Moemon Co-op: team rematches with gym leaders.
//
// Talk to a gym leader you've already beaten while a friend is on the same
// map, and they offer a team rematch with their strongest team (stronger
// still once you have all eight badges). Your friend joins the usual way, by
// standing next to you facing you. Both of you get a reward for winning.
// Scripts: data/scripts/mp_rematch.inc.
#include <stdio.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "event_scripts.h"
#include "item.h"
#include "money.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/opponents.h"

u16 MP_TrainerIdInScript(const u8 *s);
void MP_Gatcha_AddPulls(s32 n);

struct Leader
{
    u16 trainer;     // first battle
    u16 rematch;     // the team rematch (before all badges)
    u16 endgame;     // the team rematch with all badges
    u16 badge;
    const u8 *script;
};

static const struct Leader sLeaders[] = {
    {TRAINER_ROXANNE_1, TRAINER_ROXANNE_2, TRAINER_ROXANNE_5, FLAG_BADGE01_GET, MpRematch_EventScript_Roxanne},
    {TRAINER_BRAWLY_1, TRAINER_BRAWLY_2, TRAINER_BRAWLY_5, FLAG_BADGE02_GET, MpRematch_EventScript_Brawly},
    {TRAINER_WATTSON_1, TRAINER_WATTSON_2, TRAINER_WATTSON_5, FLAG_BADGE03_GET, MpRematch_EventScript_Wattson},
    {TRAINER_FLANNERY_1, TRAINER_FLANNERY_2, TRAINER_FLANNERY_5, FLAG_BADGE04_GET, MpRematch_EventScript_Flannery},
    {TRAINER_NORMAN_1, TRAINER_NORMAN_2, TRAINER_NORMAN_5, FLAG_BADGE05_GET, MpRematch_EventScript_Norman},
    {TRAINER_WINONA_1, TRAINER_WINONA_2, TRAINER_WINONA_5, FLAG_BADGE06_GET, MpRematch_EventScript_Winona},
    {TRAINER_TATE_AND_LIZA_1, TRAINER_TATE_AND_LIZA_2, TRAINER_TATE_AND_LIZA_5, FLAG_BADGE07_GET, MpRematch_EventScript_TateAndLiza},
    {TRAINER_JUAN_1, TRAINER_JUAN_2, TRAINER_JUAN_5, FLAG_BADGE08_GET, MpRematch_EventScript_Juan},
};

static bool8 FriendOnThisMap(void)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 x, y;
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState
         && gMpPlayers[i].state.mapGroup == gSaveBlock1Ptr->location.mapGroup
         && gMpPlayers[i].state.mapNum == gSaveBlock1Ptr->location.mapNum
         && MP_GetRemoteMapPos(i, &x, &y))
            return TRUE;
    }
    return FALSE;
}

// From field_control_avatar.c: talking to an object. Returns the script to run.
const u8 *MP_Rematch_ScriptForObject(const u8 *script)
{
    u16 trainer;
    u8 i;
    if (!MP_IsConnected() || gMpLocalId < 0 || script == NULL)
        return script;
    trainer = MP_TrainerIdInScript(script);
    if (trainer == 0)
        return script;
    for (i = 0; i < ARRAY_COUNT(sLeaders); i++)
        if (sLeaders[i].trainer == trainer)
            break;
    if (i == ARRAY_COUNT(sLeaders) || !FlagGet(sLeaders[i].badge) || !FriendOnThisMap())
        return script;
    return sLeaders[i].script;
}

bool8 MP_Rematch_IsRematchTrainer(u16 trainer)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sLeaders); i++)
        if (sLeaders[i].rematch == trainer || sLeaders[i].endgame == trainer)
            return TRUE;
    return FALSE;
}

// Special: with all eight badges the leaders bring their strongest team
void MP_Rematch_IsEndgame(void)
{
    u8 i, n = 0;
    for (i = 0; i < 8; i++)
        n += FlagGet(FLAG_BADGE01_GET + i) ? 1 : 0;
    gSpecialVar_Result = (n == 8);
}

// Special (and for the partner after a co-op win): the team rematch reward
// The day's first team rematch win gives a MOEGATCHA pull and a RARE CANDY;
// after that, prize money
void MP_Rematch_Reward(void)
{
    u8 text[96];
    char key[24];
    snprintf(key, sizeof(key), "rematch:%d", (int)MP_Today());
    if (!MP_SaveData_Get(key))
    {
        MP_SaveData_Set(key, 1);
        if (!AddBagItem(ITEM_RARE_CANDY, 1))
            AddMoney(&gSaveBlock1Ptr->money, 2000);
        MP_Gatcha_AddPulls(1);
        MP_AsciiToGame(text, "Team rematch won!\nRARE CANDY x1, +1 MOEGATCHA pull", 90);
    }
    else
    {
        AddMoney(&gSaveBlock1Ptr->money, 2000);
        MP_AsciiToGame(text, "Team rematch won!\n$2000 (today's pull is already yours)", 90);
    }
    MP_Toast(text);
    MP_Achieve_Count("rematch", 1);
}
