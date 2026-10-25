// Moemon Co-op: the MOéMON GARDEN with friends (docs/POKEMON_GARDEN.md 11.1).
//
// Everyone's game runs its own Pokémon there; each sends where they are and what they're doing
// (MPR_GARDEN_ACTORS, about six times a second while something changes) and its decorations
// (MPR_GARDEN_DECOR), and shows the others'. A friend's Pokémon walks to its tile here by itself, so it
// moves smoothly at any lag. Friends' decorations show while they're here.
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "mp_life.h"
#include "mp_life_emote.h"
#include "main.h"
#include "fieldmap.h"
#include "constants/characters.h"
#include "data.h"
#include "pokemon.h"
#include "string_util.h"
#define GX(x) ((x) + MAP_OFFSET)
#define GY(y) ((y) + MAP_OFFSET)

#define MPR_GARDEN_ACTORS (MPR_GARDEN_FIRST + 0)   // u8 count, then 15 bytes each
#define MPR_GARDEN_DECOR  (MPR_GARDEN_FIRST + 1)   // u8 count, then decor, x, y each
#define ACTOR_BYTES 18

bool8 MP_GardenLife_Local(u8 n, u32 *key, u16 *dex, u8 *female, s16 *hue, s16 *x, s16 *y, s16 *fx, s16 *fy, u8 *speed, u8 *dir,
                          u8 *pose, u8 *act, u8 *actor);
s8 MP_GardenLife_SetRemote(s8 owner, u8 index, u32 key, u16 dex, bool8 female, s16 hue, s16 x, s16 y, s16 fx, s16 fy,
                           u8 speed, u8 dir, u8 pose, u8 act);
void MP_GardenLife_DropMissing(s8 owner, u8 present);
void MP_GardenDecor_SetRemote(u8 owner, const u8 *list, u8 count);
u8 MP_GardenDecor_PackLocal(u8 *out);

static struct
{
    u8 last[2 + PARTY_SIZE * ACTOR_BYTES];
    u16 lastLen;
    u16 timer, heartbeat, decorTimer;
    u8 remoteSeq[MP_MAX_PLAYERS][PARTY_SIZE];
    bool8 remoteSeen[MP_MAX_PLAYERS][PARTY_SIZE];
    bool8 wasHere[MP_MAX_PLAYERS];
} sN;

static bool8 OnGardenMap(u8 id)
{
    return gMpPlayers[id].present && gMpPlayers[id].hasState && gMpPlayers[id].state.mapGroup == gSaveBlock1Ptr->location.mapGroup
        && gMpPlayers[id].state.mapNum == gSaveBlock1Ptr->location.mapNum;
}

static void PutU16(u8 *p, u16 v) { p[0] = v; p[1] = v >> 8; }
static u16 GetU16(const u8 *p) { return p[0] | (p[1] << 8); }

void MP_GardenNet_Update(void)
{
    u8 buf[2 + PARTY_SIZE * ACTOR_BYTES], i, n = 0;
    u16 len;
    if (!MP_IsConnected() || gMpLocalId < 0 || !MP_Garden_IsHere())
        return;
    // friends who left the Garden: their Pokémon go with them
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        bool8 here = i != gMpLocalId && OnGardenMap(i);
        if (sN.wasHere[i] && !here)
        {
            MP_GardenLife_DropMissing(i, 0);
            MP_GardenDecor_SetRemote(i, NULL, 0);
        }
        if (here && !sN.wasHere[i])
            sN.decorTimer = 9999;   // someone came: our decorations at once
        sN.wasHere[i] = here;
    }
    if (++sN.timer < 4)
        return;   // (every 4 frames when something changed: a step never goes unsent, at a Garden Pokémon's pace)
    sN.timer = 0;
    buf[0] = MPR_GARDEN_ACTORS;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 key;
        u16 dex;
        u8 female, dir, pose, act, actor, speed, *p = &buf[2 + n * ACTOR_BYTES];
        s16 hue, x, y, fx, fy;
        if (!MP_GardenLife_Local(i, &key, &dex, &female, &hue, &x, &y, &fx, &fy, &speed, &dir, &pose, &act, &actor))
            break;
        p[0] = key; p[1] = key >> 8; p[2] = key >> 16; p[3] = key >> 24;
        PutU16(p + 4, dex);
        p[6] = female;
        PutU16(p + 7, (u16)hue);
        p[9] = x - MAP_OFFSET;
        p[10] = y - MAP_OFFSET;
        p[11] = dir | (pose << 4);
        p[12] = act;
        p[13] = MP_LifeEmote_Current(LIFE_EMOTE_SLOT_GARDEN(actor));
        p[14] = MP_LifeEmote_Seq(LIFE_EMOTE_SLOT_GARDEN(actor));
        p[15] = fx - MAP_OFFSET;
        p[16] = fy - MAP_OFFSET;
        p[17] = speed;
        n++;
    }
    buf[1] = n;
    len = 2 + n * ACTOR_BYTES;
    if (len != sN.lastLen || memcmp(buf, sN.last, len) != 0 || ++sN.heartbeat >= 30)
    {
        sN.heartbeat = 0;
        memcpy(sN.last, buf, len);
        sN.lastLen = len;
        MP_SendRelay(0xFF, buf, len);
    }
    if (++sN.decorTimer >= 18)
    {
        u8 d[2 + GARDEN_DECOR_MAX * 3];
        sN.decorTimer = 0;
        d[0] = MPR_GARDEN_DECOR;
        d[1] = MP_GardenDecor_PackLocal(d + 2);
        MP_SendRelay(0xFF, d, 2 + d[1] * 3);
    }
}

void MP_GardenNet_OnRelay(u8 from, const u8 *data, u16 len)
{
    u8 i, count, present = 0;
    if (from >= MP_MAX_PLAYERS || from == gMpLocalId || len < 2 || !MP_Garden_IsHere() || !OnGardenMap(from))
        return;
    count = data[1];
    if (data[0] == MPR_GARDEN_DECOR)
    {
        if (len >= 2 + count * 3 && count <= GARDEN_DECOR_MAX)
            MP_GardenDecor_SetRemote(from, data + 2, count);
        return;
    }
    if (count > PARTY_SIZE || len < 2 + count * ACTOR_BYTES)
        return;
    for (i = 0; i < count; i++)
    {
        const u8 *p = &data[2 + i * ACTOR_BYTES];
        u32 key = p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
        s8 actor = MP_GardenLife_SetRemote(from, i, key, GetU16(p + 4), p[6], (s16)GetU16(p + 7), GX(p[9]), GY(p[10]),
                                           GX(p[15]), GY(p[16]), p[17], p[11] & 0xF, p[11] >> 4, p[12]);
        present |= 1 << i;
        if (actor < 0)
            continue;
        if (!sN.remoteSeen[from][i])
        {
            sN.remoteSeen[from][i] = TRUE;
            sN.remoteSeq[from][i] = p[14];
        }
        else if (sN.remoteSeq[from][i] != p[14])
        {
            sN.remoteSeq[from][i] = p[14];
            if (p[13] < LIFE_EMOTE_COUNT)
                MP_LifeEmote_Show(LIFE_EMOTE_SLOT_GARDEN(actor), p[13]);
        }
        // noted by name for the journal (their species; nicknames stay theirs)
        if (MP_Life_FindKey(key) == NULL)
        {
            u8 name[32], *end = MP_CopyPlayerName(name, from);
            static const u8 sS_[] = _("'s ");
            end = StringCopy(end, sS_);
            StringCopy(end, gSpeciesNames[NationalPokedexNumToSpecies(GetU16(p + 4))]);
            name[23] = EOS;
            MP_Life_NoteName(key, name);
        }
    }
    for (i = count; i < PARTY_SIZE; i++)
        sN.remoteSeen[from][i] = FALSE;
    MP_GardenLife_DropMissing(from, present);
}
