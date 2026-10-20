// Moemon Co-op: lend field moves. A friend standing close by (same map, within
// LEND_RANGE tiles) whose Moemon knows CUT, SURF, STRENGTH, ROCK SMASH,
// WATERFALL or DIVE lends it: the tree, rock, boulder or water prompt works as
// if that Moemon were in your party ("ALICE's PELIPPER used SURF!"), and the
// friend hears about it. Your own badges still decide what you can use.
//
// Every client broadcasts the first Moemon in its party that knows each of
// those moves (MPR_LEND) whenever that changes or someone new shows up. The
// field scripts get MP_LEND_SLOT as the "party slot" from checkpartymove, and
// the places that look that slot up (nickname, the show-Moemon effect,
// STRENGTH) ask here instead (scrcmd.c, field_effect.c, fldeff_strength.c).
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "pokemon.h"
#include "string_util.h"
#include "party_menu.h"
#include "data.h"
#include "constants/characters.h"
#include "constants/moves.h"
#include "constants/species.h"

#define LEND_RANGE 4

enum { LEND_CUT, LEND_SURF, LEND_STRENGTH, LEND_ROCK_SMASH, LEND_WATERFALL, LEND_DIVE, LEND_COUNT };
static const u16 sLendMoves[LEND_COUNT] = {MOVE_CUT, MOVE_SURF, MOVE_STRENGTH, MOVE_ROCK_SMASH, MOVE_WATERFALL, MOVE_DIVE};

struct LendMon
{
    u16 species;
    u32 personality;
    u32 otId;
    u8 nick[POKEMON_NAME_LENGTH + 1];
};

#define LEND_ENTRY_SIZE (2 + 4 + 4 + POKEMON_NAME_LENGTH + 1)

enum { MPR_LEND_PARTY_SUB, MPR_LEND_USED_SUB };

static struct LendMon sFriend[MP_MAX_PLAYERS][LEND_COUNT];
static struct LendMon sMine[LEND_COUNT];
static bool8 sMineValid;
static u8 sKnownPlayers;      // present players last time (a newcomer gets our list)
static u16 sTimer;
static s8 sActiveFriend = -1;
static u8 sActiveMove;

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

static int LendIndex(u16 move)
{
    int k;
    for (k = 0; k < LEND_COUNT; k++)
        if (sLendMoves[k] == move)
            return k;
    return -1;
}

static void ReadMine(struct LendMon *out)
{
    int k, i;
    memset(out, 0, sizeof(struct LendMon) * LEND_COUNT);
    for (k = 0; k < LEND_COUNT; k++)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
                break;
            if (GetMonData(mon, MON_DATA_IS_EGG) || !MonKnowsMove(mon, sLendMoves[k]))
                continue;
            out[k].species = GetMonData(mon, MON_DATA_SPECIES);
            out[k].personality = GetMonData(mon, MON_DATA_PERSONALITY);
            out[k].otId = GetMonData(mon, MON_DATA_OT_ID);
            GetMonData(mon, MON_DATA_NICKNAME, out[k].nick);
            out[k].nick[POKEMON_NAME_LENGTH] = EOS;
            break;
        }
    }
}

static void PutU32(u8 *p, u32 v)
{
    p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}

static u32 GetU32(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

static void SendMine(void)
{
    u8 buf[2 + LEND_COUNT * LEND_ENTRY_SIZE];
    int k;
    buf[0] = MPR_LEND;
    buf[1] = MPR_LEND_PARTY_SUB;
    for (k = 0; k < LEND_COUNT; k++)
    {
        u8 *p = &buf[2 + k * LEND_ENTRY_SIZE];
        p[0] = sMine[k].species & 0xFF;
        p[1] = sMine[k].species >> 8;
        PutU32(p + 2, sMine[k].personality);
        PutU32(p + 6, sMine[k].otId);
        memcpy(p + 10, sMine[k].nick, POKEMON_NAME_LENGTH + 1);
    }
    MP_SendRelay(0xFF, buf, sizeof(buf));
}

// Every overworld frame
void MP_Lend_Update(void)
{
    struct LendMon now[LEND_COUNT];
    u8 present = 0, i;
    if (!MP_IsConnected() || gMpLocalId < 0)
    {
        sMineValid = FALSE;
        sKnownPlayers = 0;
        return;
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present)
            present |= 1 << i;
    if (++sTimer < 30 && (present & ~sKnownPlayers) == 0)
        return;
    sTimer = 0;
    ReadMine(now);
    if (!sMineValid || memcmp(now, sMine, sizeof(now)) != 0 || (present & ~sKnownPlayers))
    {
        memcpy(sMine, now, sizeof(now));
        sMineValid = TRUE;
        SendMine();
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (!(present & (1 << i)))
            memset(sFriend[i], 0, sizeof(sFriend[i]));
    sKnownPlayers = present;
}

void MP_Lend_OnRelay(u8 from, const u8 *data, u16 len)
{
    int k;
    if (from >= MP_MAX_PLAYERS || len < 2)
        return;
    if (data[1] == MPR_LEND_PARTY_SUB && len >= 2 + LEND_COUNT * LEND_ENTRY_SIZE)
    {
        for (k = 0; k < LEND_COUNT; k++)
        {
            const u8 *p = &data[2 + k * LEND_ENTRY_SIZE];
            struct LendMon *m = &sFriend[from][k];
            m->species = p[0] | (p[1] << 8);
            if (m->species >= NUM_SPECIES)
                m->species = SPECIES_NONE;
            m->personality = GetU32(p + 2);
            m->otId = GetU32(p + 6);
            memcpy(m->nick, p + 10, POKEMON_NAME_LENGTH + 1);
            m->nick[POKEMON_NAME_LENGTH] = EOS;
        }
    }
    else if (data[1] == MPR_LEND_USED_SUB && len >= 3 && data[2] < LEND_COUNT && sMine[data[2]].species)
    {
        // A friend just used one of ours
        u8 text[80], name[PLAYER_NAME_LENGTH + 1], tmp[24];
        static const u8 sUsedYour[] = _(" used your ");
        static const u8 sS[] = _("'s ");
        StringCopyN(name, gMpPlayers[from].name, PLAYER_NAME_LENGTH);
        name[PLAYER_NAME_LENGTH] = EOS;
        StringCopy(text, name);
        StringAppend(text, sUsedYour);
        StringAppend(text, sMine[data[2]].nick);
        StringAppend(text, sS);
        StringAppend(text, gMoveNames[sLendMoves[data[2]]]);
        tmp[0] = CHAR_EXCL_MARK;
        tmp[1] = EOS;
        StringAppend(text, tmp);
        MP_Toast(text);
    }
}

// The closest friend in range who can lend this move (-1 = nobody)
static s8 FindLender(int k)
{
    struct ObjectEvent *me;
    s8 best = -1;
    int bestD = LEND_RANGE + 1, i;
    if (k < 0 || !MP_IsConnected() || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return -1;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        struct MpPlayer *p = &gMpPlayers[i];
        int d;
        if (i == gMpLocalId || !p->present || !sFriend[i][k].species)
            continue;
        if (p->state.mapGroup != gSaveBlock1Ptr->location.mapGroup || p->state.mapNum != gSaveBlock1Ptr->location.mapNum)
            continue;
        d = max(abs(p->state.x - me->currentCoords.x), abs(p->state.y - me->currentCoords.y));
        if (d < bestD)
        {
            bestD = d;
            best = i;
        }
    }
    return best;
}

bool8 MP_Lend_Available(u16 move)
{
    return FindLender(LendIndex(move)) >= 0;
}

// checkpartymove found nothing in your party: borrow it (species, or -1)
s32 MP_Lend_Borrow(u16 move)
{
    int k = LendIndex(move);
    s8 id = FindLender(k);
    if (id < 0)
        return -1;
    sActiveFriend = id;
    sActiveMove = k;
    return sFriend[id][k].species;
}

static const struct LendMon *Active(void)
{
    if (sActiveFriend < 0 || sActiveFriend >= MP_MAX_PLAYERS)
        return NULL;
    return &sFriend[sActiveFriend][sActiveMove];
}

// "ALICE's PELIPPER"
void MP_Lend_Name(u8 *dst)
{
    const struct LendMon *m = Active();
    static const u8 sS[] = _("'s ");
    if (!m)
    {
        dst[0] = EOS;
        return;
    }
    StringCopyN(dst, gMpPlayers[sActiveFriend].name, PLAYER_NAME_LENGTH);
    dst[PLAYER_NAME_LENGTH] = EOS;
    StringAppend(dst, sS);
    StringAppend(dst, m->nick);
}

// The show-Moemon effect, and the friend hears it was used
void MP_Lend_ShowMonArgs(u32 *species, u32 *otId, u32 *personality)
{
    const struct LendMon *m = Active();
    u8 buf[3] = {MPR_LEND, MPR_LEND_USED_SUB, sActiveMove};
    if (!m)
    {
        *species = SPECIES_NONE;
        return;
    }
    *species = m->species;
    *otId = m->otId;
    *personality = m->personality;
    MP_SendRelay(sActiveFriend, buf, sizeof(buf));
}

// STRENGTH doesn't show the Moemon: just tell the friend
void MP_Lend_NotifyUsed(void)
{
    u8 buf[3] = {MPR_LEND, MPR_LEND_USED_SUB, sActiveMove};
    if (Active())
        MP_SendRelay(sActiveFriend, buf, sizeof(buf));
}

// Debug "mp lend": what each friend can lend
void MP_Lend_Debug(void)
{
    int i, k;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        for (k = 0; k < LEND_COUNT; k++)
            if (sFriend[i][k].species)
                printf("[test] lend: player %d move %d species %d%s\n", i, sLendMoves[k], sFriend[i][k].species,
                       FindLender(k) == i ? " (in range)" : "");
}
