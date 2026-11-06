#ifndef GUARD_MP_STADIUM_TEAMS_H
#define GUARD_MP_STADIUM_TEAMS_H

// POKéMON STADIUM's saved teams: 99 for the whole server online, the save's own 99 offline (src/mp/mp_stadium_teams.c)

#include "mp_stadium_cup.h"

#define STADIUM_SAVED_TEAMS 99

struct StadiumSavedTeam
{
    u8 cup;                              // the cup it was registered for (STADIUM_CUP_*)
    u8 count;
    u8 owner[PLAYER_NAME_LENGTH + 1];    // who saved it (game text)
    u16 ownerId;                         // their Trainer ID (low half)
    struct StadiumTeamSlot slot[STADIUM_TEAM_SIZE];
};

const struct StadiumSavedTeam *StadiumTeams_Get(u8 slot);
u16 StadiumTeams_Version(void);
bool8 StadiumTeams_IsServer(void);
u8 StadiumTeams_Count(void);
u8 StadiumTeams_FirstFree(void);
bool8 StadiumTeams_Put(u8 slot, const struct StadiumSavedTeam *team);
void StadiumTeams_Erase(u8 slot);
void StadiumTeams_OnConnected(void);
void StadiumTeams_OnStore(const u8 *data, u16 len);
bool8 StadiumTeams_TakeFull(void);

#endif // GUARD_MP_STADIUM_TEAMS_H
