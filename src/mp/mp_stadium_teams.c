// Moemon Kai: POKéMON STADIUM's saved teams (docs/STADIUM.md; the user, 2026-11-06: "players must be able to register
// up to 99 teams per Server, and erase teams as well without prior authorization, copy teams as well, and be able to
// use each others"). Online the server keeps them for everyone on it (mp_scores.c, store ops MP_ST_TEAM_*: anyone saves
// over, copies or erases any slot, and the room hears of it at once); offline the save keeps its own 99 in
// "<save>.stadiumteams". A team is the registration screen's six (copies: the PCs' own stay where they are).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_stadium.h"
#include "mp_stadium_cup.h"
#include "mp_stadium_teams.h"
#include "../mp/mp_net.h"

extern char gPlatformSavePath[512];

#define TEAMS_MAGIC 0x314D5453   // "STM1"

static struct
{
    bool8 used[STADIUM_SAVED_TEAMS];
    struct StadiumSavedTeam team[STADIUM_SAVED_TEAMS];
    bool8 online;          // the server's (else the save's own)
    bool8 loaded;          // the save's own read in
    bool8 full;            // the server said no room
    u16 version;           // changes whenever a slot does (the screen redraws)
    char loadedFor[512];
} sT;

static void LocalPath(char *out, size_t size)
{
    snprintf(out, size, "%s.stadiumteams", gPlatformSavePath);
}

static void LocalLoad(void)
{
    char path[600];
    FILE *f;
    u32 magic = 0;
    if (sT.online || (sT.loaded && strcmp(sT.loadedFor, gPlatformSavePath) == 0))
        return;
    memset(sT.used, 0, sizeof(sT.used));
    sT.loaded = TRUE;
    snprintf(sT.loadedFor, sizeof(sT.loadedFor), "%s", gPlatformSavePath);
    LocalPath(path, sizeof(path));
    if (!(f = fopen(path, "rb")))
        return;
    if (fread(&magic, 4, 1, f) != 1 || magic != TEAMS_MAGIC || fread(sT.used, sizeof(sT.used), 1, f) != 1
     || fread(sT.team, sizeof(sT.team), 1, f) != 1)
        memset(sT.used, 0, sizeof(sT.used));
    fclose(f);
    sT.version++;
}

static void LocalSave(void)
{
    char path[600], tmp[620];
    FILE *f;
    u32 magic = TEAMS_MAGIC;
    LocalPath(path, sizeof(path));
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    if (!(f = fopen(tmp, "wb")))
        return;
    fwrite(&magic, 4, 1, f);
    fwrite(sT.used, sizeof(sT.used), 1, f);
    fwrite(sT.team, sizeof(sT.team), 1, f);
    fclose(f);
    remove(path);
    rename(tmp, path);
}

static bool8 Online(void)
{
    return MP_IsConnected() && gMpLocalId >= 0;
}

// The list as it stands (the server's online, else the save's own)
const struct StadiumSavedTeam *StadiumTeams_Get(u8 slot)
{
    if (!Online())
    {
        if (sT.online)   // (left the server: the save's own again)
            sT.online = FALSE, sT.loaded = FALSE;
        LocalLoad();
    }
    return slot < STADIUM_SAVED_TEAMS && sT.used[slot] ? &sT.team[slot] : NULL;
}

u16 StadiumTeams_Version(void)
{
    return sT.version;
}

bool8 StadiumTeams_IsServer(void)
{
    return Online();
}

u8 StadiumTeams_Count(void)
{
    u8 i, n = 0;
    for (i = 0; i < STADIUM_SAVED_TEAMS; i++)
        n += StadiumTeams_Get(i) != NULL;
    return n;
}

u8 StadiumTeams_FirstFree(void)
{
    u8 i;
    for (i = 0; i < STADIUM_SAVED_TEAMS; i++)
        if (!StadiumTeams_Get(i))
            return i;
    return 0xFF;
}

// Saves a team into a slot (0xFF: the first free one); online the server decides and tells everyone
bool8 StadiumTeams_Put(u8 slot, const struct StadiumSavedTeam *team)
{
    if (slot == 0xFF)
        slot = StadiumTeams_FirstFree();
    if (slot == 0xFF)
        return FALSE;
    if (Online())
    {
        static u8 buf[2 + sizeof(struct StadiumSavedTeam)];
        buf[0] = MP_ST_TEAM_PUT;
        buf[1] = slot;
        memcpy(buf + 2, team, sizeof(*team));
        MPNet_ClientSend(MPMSG_C_STORE, buf, sizeof(buf));
    }
    sT.used[slot] = TRUE;   // (shown at once; the server's word follows)
    sT.team[slot] = *team;
    if (!Online())
        LocalSave();
    sT.version++;
    MPNet_Log("stadium teams: saved slot %d (%s)", slot + 1, Online() ? "server" : "local");
    return TRUE;
}

void StadiumTeams_Erase(u8 slot)
{
    if (slot >= STADIUM_SAVED_TEAMS)
        return;
    if (Online())
    {
        u8 buf[2] = {MP_ST_TEAM_ERASE, slot};
        MPNet_ClientSend(MPMSG_C_STORE, buf, 2);
    }
    sT.used[slot] = FALSE;
    if (!Online())
        LocalSave();
    sT.version++;
    MPNet_Log("stadium teams: erased slot %d", slot + 1);
}

// mp_main.c: in a room: the server's list
void StadiumTeams_OnConnected(void)
{
    u8 op = MP_ST_TEAM_GET;
    memset(sT.used, 0, sizeof(sT.used));
    sT.online = TRUE;
    sT.version++;
    MPNet_ClientSend(MPMSG_C_STORE, &op, 1);
}

// mp_sharedpc.c: MP_ST_TEAM_LIST (u8 part, u8 parts, u8 count, (u8 slot, u16 len, record)*)
void StadiumTeams_OnStore(const u8 *data, u16 len)
{
    u16 at = 4;
    u8 i, count;
    if (len < 4)
        return;
    sT.online = TRUE;
    if (data[1] == 0xFE)   // no room left on the server
    {
        sT.full = TRUE;
        sT.version++;
        return;
    }
    if (data[1] == 0)
        memset(sT.used, 0, sizeof(sT.used));
    count = data[3];
    for (i = 0; i < count && at + 3 <= len; i++)
    {
        u8 slot = data[at];
        u16 l = data[at + 1] | (data[at + 2] << 8);
        at += 3;
        if (at + l > len || slot >= STADIUM_SAVED_TEAMS)
            break;
        if (data[1] == 0xFF)
            MPNet_Log("stadium teams: slot %d %s by the server", slot + 1, l ? "saved" : "erased");
        if (l == 0)
            sT.used[slot] = FALSE;
        else
        {
            memset(&sT.team[slot], 0, sizeof(sT.team[slot]));
            memcpy(&sT.team[slot], data + at, l < sizeof(sT.team[slot]) ? l : sizeof(sT.team[slot]));
            sT.used[slot] = TRUE;
        }
        at += l;
    }
    if (data[1] != 0xFF)
        MPNet_Log("stadium teams: the server's list, part %d of %d", data[1] + 1, data[2]);
    sT.version++;
}

bool8 StadiumTeams_TakeFull(void)
{
    bool8 f = sT.full;
    sT.full = FALSE;
    return f;
}
