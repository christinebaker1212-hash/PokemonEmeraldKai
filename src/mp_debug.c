// Developer quick-start used by the automated multiplayer tests.
// Only active when MOEMON_DEBUG_QUICKSTART is set in the environment.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform/gamepad.h"
#include "global.h"
#include "orre.h"
#include "o3d.h"
#include "orre_people.h"
#include "orre_shadow.h"   // Moemon Kai: Shadow Pokémon
#include "region_map.h"   // (mapname: GetMapName)
#include "metatile_behavior.h"
#include "roamer.h"
#include "s3d_underground.h"
unsigned int SDL_GetTicks(void);
#include "arcade.h"   // Moemon Kai: the GAME CENTER's arcade boards
#include "s3d.h"   // Moemon Kai: SINNOH's 3D engine (tests)
#include "field_message_box.h"
#include "mp_region.h"
#include "field_control_avatar.h"
#include "battle_gen4.h"
#include "mp_kaisong.h"
#include "gba/m4a_internal.h"
#include "data.h"
#include "platform.h"
#include "siirtc.h"
#include "rtc.h"
#include "easy_chat.h"
#include "field_screen_effect.h"
#include "field_player_avatar.h"
#include "event_object_movement.h"
#include "text_window.h"
#include "daycare.h"
#include "pokemon_summary_screen.h"
#include "item.h"
#include "pokemon_storage_system.h"
#include "constants/battle.h"
#include "script.h"
#include "pokedex.h"
#include "mp.h"
#include "mp_tcard.h"
#include "mp_region.h"
#include "tcg/integration.h"
#include "ygo/integration.h"
#include "tt/integration.h"
#include "map_name_popup.h"
#include "mp_farm.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "field_effect.h"
#include "fieldmap.h"
#include "constants/field_effects.h"
#include "save.h"
#include "palette.h"
#include "constants/characters.h"
#include "main.h"
#include "battle_setup.h"
#include "pokemon.h"
#include "constants/moves.h"
#include "event_data.h"
#include "coins.h"
#include "overworld.h"
#include "sound.h"
#include "m4a.h"
#include "platform/gamepad.h"
#include "money.h"
#include "item.h"
#include "string_util.h"
#include "constants/species.h"
#include "gpu_regs.h"
#include "bg.h"
#include "party_menu.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

bool8 MP_DebugQuickStartEnabled(void)
{
    bool8 Maker_PlayNewGamePending(void);
    return getenv("MOEMON_DEBUG_QUICKSTART") != NULL || Maker_PlayNewGamePending();   // Moemon Maker: F5's story points
}

// animall FIRST LAST ATK TGT [TURN]: every move animation from FIRST to LAST in a row, ATK attacking TGT (in a battle,
// at the action menu); with MOEMON_ANIMCHECK the log lists every sprite animation started (tools/test/check_anims.py)
#include "task.h"
extern const u8 *const gBattleAnims_Moves[];
static u8 sUgFace = DIR_NORTH;   // (mp ugspot / ugface: the way to a dig spot)

static void Task_MpAnimAll(u8 taskId)
{
    static struct DisableStruct sDisable;
    s16 *d = gTasks[taskId].data;   // 0 move, 1 last, 2 atk, 3 tgt, 4 turn, 5 phase, 6 timer
    if (d[5] == 0)
    {
        if (gAnimScriptActive)
            return;
        if (d[0] > d[1])
        {
            printf("[test] animall done\n");
            MPNet_Log("animall done");   // (what the harness's waitlog sees)
            fflush(stdout);
            DestroyTask(taskId);
            return;
        }
        gBattleAnimAttacker = d[2];
        gBattleAnimTarget = d[3];
        gAnimMoveTurn = d[4];
        gAnimMovePower = 60;
        gAnimMoveDmg = 30;
        gAnimFriendship = 255;
        gWeatherMoveAnim = 0;
        memset(&sDisable, 0, sizeof(sDisable));
        gAnimDisableStructPtr = &sDisable;
        LaunchBattleAnimation(gBattleAnims_Moves, d[0], TRUE);
        d[5] = 1;
        d[6] = 0;
        return;
    }
    gAnimScriptCallback();
    if (!gAnimScriptActive || ++d[6] > 1500)
    {
        if (d[6] > 1500)
            printf("[test] animall: move %d didn't end\n", d[0]);
        else if (getenv("MOEMON_ANIM_FRAMES"))   // each animation's length (BATTLE ANIMS: CLASSIC and MODERN match)
            printf("[test] animall: move %d took %d frames\n", d[0], d[6]);
        gSprites[gBattlerSpriteIds[d[2]]].invisible = FALSE;
        gSprites[gBattlerSpriteIds[d[3]]].invisible = FALSE;
        d[0]++;
        d[5] = 0;
    }
}

static void SetAsciiName(u8 *dst, const char *src, int maxLen)
{
    int i;
    for (i = 0; i < maxLen && src[i]; i++)
    {
        char c = src[i];
        if (c >= 'A' && c <= 'Z') dst[i] = 0xBB + (c - 'A');
        else if (c >= 'a' && c <= 'z') dst[i] = 0xD5 + (c - 'a');
        else if (c >= '0' && c <= '9') dst[i] = 0xA1 + (c - '0');
        else dst[i] = 0x00;
    }
    dst[i] = EOS;
}

static int sPinFoeHp, sPinMyHp;   // mp pinhp

// mp pinhp, each frame (mp_main.c MP_FrameUpdate)
void MP_Debug_PinHpFrame(void)
{
    int i;
    if (!gMain.inBattle || (!sPinFoeHp && !sPinMyHp))
        return;
    for (i = 0; i < gBattlersCount; i++)
    {
        if (GetBattlerSide(i) == B_SIDE_OPPONENT && sPinFoeHp && gBattleMons[i].hp > sPinFoeHp)
            gBattleMons[i].hp = sPinFoeHp;
        else if (GetBattlerSide(i) == B_SIDE_PLAYER && sPinMyHp && gBattleMons[i].hp && gBattleMons[i].hp < sPinMyHp)
        {
            // (past its max HP too: a one-hit Knock Out can't get through)
            if (gBattleMons[i].maxHP < sPinMyHp)
                gBattleMons[i].maxHP = sPinMyHp;
            gBattleMons[i].hp = sPinMyHp;
        }
    }
}

void MP_DebugSetupNameAndGender(void)
{
    const char *name = getenv("MOEMON_DEBUG_NAME");
    const char *gender = getenv("MOEMON_DEBUG_GENDER");
    SetAsciiName(gSaveBlock2Ptr->playerName, name ? name : "TEST", PLAYER_NAME_LENGTH);
    gSaveBlock2Ptr->playerGender = (gender && gender[0] == 'F') ? FEMALE : MALE;
}

// Called at the end of NewGameInitData in quick-start mode
void MP_DebugQuickStartNewGame(void)
{
    const char *partyEnv = getenv("MOEMON_DEBUG_PARTY"); // e.g. "258:20,280:18+" (+ = about to level up, * = shiny, F / M = female / male)
    const char *mapEnv = getenv("MOEMON_DEBUG_WARP");     // "group,num,x,y"
    int group = MAP_GROUP(MAP_OLDALE_TOWN), num = MAP_NUM(MAP_OLDALE_TOWN), x = 10, y = 10;
    char buf[256];
    int slot = 0;

    // MOEMON_DEBUG_TID: another Trainer ID (things kept per save, e.g. the card collection)
    if (getenv("MOEMON_DEBUG_TID"))
    {
        u32 tid = (u32)strtoul(getenv("MOEMON_DEBUG_TID"), NULL, 0);
        gSaveBlock2Ptr->playerTrainerId[0] = tid;
        gSaveBlock2Ptr->playerTrainerId[1] = tid >> 8;
        gSaveBlock2Ptr->playerTrainerId[2] = tid >> 16;
        gSaveBlock2Ptr->playerTrainerId[3] = tid >> 24;
    }
    {
        bool8 Maker_PlayNewGame(void);
        if (Maker_PlayNewGame())
            return;   // Moemon Maker: F5's story point, set up by the editor (src/maker/maker_play.c)
    }
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    FlagSet(FLAG_ADVENTURE_STARTED);
    // MOEMON_DEBUG_FLAGS: extra flags to set, e.g. "0x870,0x876" or "visited" (every town)
    {
        const char *flags = getenv("MOEMON_DEBUG_FLAGS");
        if (flags && strcmp(flags, "visited") == 0)
        {
            u16 f;
            for (f = FLAG_VISITED_LITTLEROOT_TOWN; f <= FLAG_VISITED_EVER_GRANDE_CITY; f++)
                FlagSet(f);
        }
        else if (flags)
        {
            char fb[256], *tok;
            snprintf(fb, sizeof(fb), "%s", flags);
            for (tok = strtok(fb, ","); tok; tok = strtok(NULL, ","))
                FlagSet((u16)strtoul(tok, NULL, 0));
        }
    }
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 7);
    VarSet(VAR_LITTLEROOT_TOWN_STATE, 4);
    VarSet(VAR_BIRCH_LAB_STATE, 5);
    VarSet(VAR_ROUTE101_STATE, 3);
    SetMoney(&gSaveBlock1Ptr->money, 5000);
    AddBagItem(ITEM_POKE_BALL, 5);
    AddBagItem(ITEM_POTION, 5);
    AddBagItem(ITEM_BATTLE_FLAG, 1);
    gSaveBlock1Ptr->registeredItem = ITEM_BATTLE_FLAG;
    if (getenv("MOEMON_DEBUG_MONEY"))
        SetMoney(&gSaveBlock1Ptr->money, atoi(getenv("MOEMON_DEBUG_MONEY")));

    ZeroPlayerPartyMons();
    snprintf(buf, sizeof(buf), "%s", partyEnv ? partyEnv : "280:12,283:10");
    for (char *tok = strtok(buf, ","); tok && slot < PARTY_SIZE; tok = strtok(NULL, ","))
    {
        int species = 0, level = 5;
        if (sscanf(tok, "%d:%d", &species, &level) >= 1 && species > 0 && species < NUM_SPECIES)
        {
            if (strchr(tok, '*'))   // "280:15*" = shiny (a personality that's shiny with the player's Trainer ID)
            {
                u32 otId = T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
                CreateMon(&gPlayerParty[slot], species, level, USE_RANDOM_IVS, TRUE, ((otId >> 16) ^ (otId & 0xFFFF)) << 16,
                          OT_ID_PLAYER_ID, 0);
            }
            else if (getenv("MOEMON_DEBUG_PID_SALT"))   // tests with two players: Pokémon of their own (MOéMON GARDEN keys)
                CreateMon(&gPlayerParty[slot], species, level, USE_RANDOM_IVS, TRUE,
                          (u32)strtoul(getenv("MOEMON_DEBUG_PID_SALT"), NULL, 0) * 2654435761u + slot, OT_ID_PLAYER_ID, 0);
            else if (strchr(tok, 'F') || strchr(tok, 'M'))   // "702:24F" = a female, "702:24M" = a male (gender tests)
                CreateMonWithGenderNatureLetter(&gPlayerParty[slot], species, level, USE_RANDOM_IVS,
                                                strchr(tok, 'F') ? MON_FEMALE : MON_MALE, slot * 7 % NUM_NATURES, 0);
            else
                CreateMon(&gPlayerParty[slot], species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
            if (strchr(tok, '+') && level < MAX_LEVEL) // "280:15+" = one exp point short of the next level
            {
                u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1] - 1;
                SetMonData(&gPlayerParty[slot], MON_DATA_EXP, &exp);
            }
            slot++;
        }
    }
    CalculatePlayerPartyCount();

    if (mapEnv)
        sscanf(mapEnv, "%d,%d,%d,%d", &group, &num, &x, &y);
    SetLastHealLocationWarp(HEAL_LOCATION_OLDALE_TOWN);
    SetWarpDestination(group, num, WARP_ID_NONE, x, y);
    WarpIntoMap();
}

// Test harness conditions (src/platform/sdl2.c: waitfor / pressuntil)
bool8 MP_Setup_IsReady(void);
bool8 MP_Setup_ScreenIs(int screen);
// The harness's "walkto X Y": the D-pad key toward map tile (X, Y) by the shortest walkable way (collision and people
// in the way), 0 once there, 0xFFFF when no way leads there or the player can't walk now
u16 MP_TestWalkKey(s32 tx, s32 ty)
{
    static s16 sFrom[160 * 160];
    static u16 sQueue[160 * 160];
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 w = gMapHeader.mapLayout->width, h = gMapHeader.mapLayout->height, px, py, head = 0, tail = 0, i, at;
    static const s8 dx[4] = {0, 0, -1, 1}, dy[4] = {1, -1, 0, 0};
    static const u16 keys[4] = {DPAD_DOWN, DPAD_UP, DPAD_LEFT, DPAD_RIGHT};
    if (!MP_IsInOverworld() || w * h > (s32)ARRAY_COUNT(sFrom) || ScriptContext_IsEnabled())
        return 0xFFFF;
    px = p->currentCoords.x - MAP_OFFSET, py = p->currentCoords.y - MAP_OFFSET;
    if (px == tx && py == ty)
        return 0;
    if (tx < 0 || ty < 0 || tx >= w || ty >= h)
        return 0xFFFF;
    for (i = 0; i < w * h; i++)
        sFrom[i] = -1;
    // (from the target back: the first step out of the player's tile toward it)
    sFrom[ty * w + tx] = ty * w + tx;
    sQueue[tail++] = ty * w + tx;
    while (head < tail)
    {
        s32 c = sQueue[head++], cx = c % w, cy = c / w, d;
        for (d = 0; d < 4; d++)
        {
            s32 nx = cx + dx[d], ny = cy + dy[d], n = ny * w + nx;
            if (nx < 0 || ny < 0 || nx >= w || ny >= h || sFrom[n] >= 0)
                continue;
            if (nx == px && ny == py)
            {
                // the player steps from (nx, ny) to (cx, cy): the opposite of d
                return keys[d ^ 1];
            }
            if (MapGridGetCollisionAt(nx + MAP_OFFSET, ny + MAP_OFFSET)
             || GetObjectEventIdByXY(nx + MAP_OFFSET, ny + MAP_OFFSET) != OBJECT_EVENTS_COUNT)
                continue;
            sFrom[n] = c;
            sQueue[tail++] = n;
        }
    }
    (void)at;
    return 0xFFFF;
}

// The harness's "goto G,N X Y": the key toward tile (X, Y) of ORRE map G.N as a player gets there: through the rooms'
// doors (their warps) and ORRE's world map (its ways out; the place picked as the world map opens:
// worldmap.c Orre_WorldMapTestPick), by the fewest rooms. 0 once there, 0xFFFF when it can't go on now
bool8 Orre_WorldMapShows(u16 map);
void Orre_WorldMapTestPick(u16 map);
#define ORRE_TEST_MAPS ((MAP_ORRE_PYRITE_COLOSSEUM_ENTER_1F_2 & 0xFF) + 1)
static bool8 OrreHasWayOut(const struct MapHeader *h)
{
    extern const u8 Orre_EventScript_WorldMap[];
    u32 i;
    for (i = 0; h && h->events && i < h->events->coordEventCount; i++)
        if (h->events->coordEvents[i].script == Orre_EventScript_WorldMap)
            return TRUE;
    return FALSE;
}

// The harness's goto anywhere but ORRE: the fewest maps through warps and map connections (KANTO, JOHTO, SINNOH,
// HOENN's walk-only runs). Next hop: one of this map's warps into it (walked onto; an arrow warp is pressed into), else
// the edge it joins at (walked to, then stepped over). 0 once there, 0xFFFF when it can't go on now
#define GOTO_MAX_MAPS 256
static u16 GotoKeyAnywhere(u8 group, u8 num, s32 tx, s32 ty)
{
    static u16 sPrev[MAP_GROUPS_COUNT][GOTO_MAX_MAPS];   // (the map it was reached from + 1; 0 none)
    static u16 sQ[MAP_GROUPS_COUNT * GOTO_MAX_MAPS];
    u8 hg = gSaveBlock1Ptr->location.mapGroup, hn = gSaveBlock1Ptr->location.mapNum;
    u16 here = (hg << 8) | hn, target = (group << 8) | num, hop;
    s32 head = 0, tail = 0, i;
    const struct MapHeader *h;
    if (hg == group && hn == num)
        return MP_TestWalkKey(tx, ty);
    if (group >= MAP_GROUPS_COUNT || num >= GOTO_MAX_MAPS || hg >= MAP_GROUPS_COUNT || hn >= GOTO_MAX_MAPS)
        return 0xFFFF;
    memset(sPrev, 0, sizeof(sPrev));
    sPrev[hg][hn] = here + 1;
    sQ[tail++] = here;
    while (head < tail && !sPrev[group][num])
    {
        u16 c = sQ[head++];
        h = Overworld_GetMapHeaderByGroupAndId(c >> 8, c & 0xFF);
        for (i = 0; h && h->events && i < h->events->warpCount; i++)
        {
            const struct WarpEvent *w = &h->events->warps[i];
            if (w->mapGroup >= MAP_GROUPS_COUNT || w->mapNum >= GOTO_MAX_MAPS || sPrev[w->mapGroup][w->mapNum])
                continue;
            sPrev[w->mapGroup][w->mapNum] = c + 1;
            sQ[tail++] = (w->mapGroup << 8) | w->mapNum;
        }
        for (i = 0; h && h->connections && i < h->connections->count; i++)
        {
            const struct MapConnection *m = &h->connections->connections[i];
            if (m->direction < CONNECTION_SOUTH || m->direction > CONNECTION_EAST || m->mapGroup >= MAP_GROUPS_COUNT
             || m->mapNum >= GOTO_MAX_MAPS || sPrev[m->mapGroup][m->mapNum])
                continue;
            sPrev[m->mapGroup][m->mapNum] = c + 1;
            sQ[tail++] = (m->mapGroup << 8) | m->mapNum;
        }
    }
    if (!sPrev[group][num])
        return 0xFFFF;
    for (hop = target; sPrev[hop >> 8][hop & 0xFF] - 1 != here; hop = sPrev[hop >> 8][hop & 0xFF] - 1)
        ;
    // a warp into the next map
    h = &gMapHeader;
    for (i = 0; h->events && i < h->events->warpCount; i++)
    {
        const struct WarpEvent *w = &h->events->warps[i];
        struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
        u16 k;
        if (((w->mapGroup << 8) | w->mapNum) != hop)
            continue;
        if (p->currentCoords.x - MAP_OFFSET == w->x && p->currentCoords.y - MAP_OFFSET == w->y)
        {
            u8 b = MapGridGetMetatileBehaviorAt(w->x + MAP_OFFSET, w->y + MAP_OFFSET);
            if (MetatileBehavior_IsNorthArrowWarp(b)) return DPAD_UP;
            if (MetatileBehavior_IsSouthArrowWarp(b)) return DPAD_DOWN;
            if (MetatileBehavior_IsEastArrowWarp(b)) return DPAD_RIGHT;
            if (MetatileBehavior_IsWestArrowWarp(b)) return DPAD_LEFT;
            return 0xFFFF;   // (on it: its warp on its way)
        }
        if ((k = MP_TestWalkKey(w->x, w->y)) != 0xFFFF)
            return k;
    }
    // the edge joining the next map: a tile on it whose neighbour across is open, then the step over
    for (i = 0; h->connections && i < h->connections->count; i++)
    {
        const struct MapConnection *m = &h->connections->connections[i];
        s32 w = gMapHeader.mapLayout->width, hh = gMapHeader.mapLayout->height, k2, best = -1;
        struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
        s32 px = p->currentCoords.x - MAP_OFFSET, py = p->currentCoords.y - MAP_OFFSET;
        u16 key = 0xFFFF, step;
        if (((m->mapGroup << 8) | m->mapNum) != hop)
            continue;
        step = m->direction == CONNECTION_NORTH ? DPAD_UP : m->direction == CONNECTION_SOUTH ? DPAD_DOWN
             : m->direction == CONNECTION_WEST ? DPAD_LEFT : DPAD_RIGHT;
        for (k2 = 0; k2 < ((m->direction == CONNECTION_NORTH || m->direction == CONNECTION_SOUTH) ? w : hh); k2++)
        {
            s32 ex, ey, ox, oy;
            u16 kk;
            switch (m->direction)
            {
            case CONNECTION_NORTH: ex = k2, ey = 0, ox = k2, oy = -1; break;
            case CONNECTION_SOUTH: ex = k2, ey = hh - 1, ox = k2, oy = hh; break;
            case CONNECTION_WEST:  ex = 0, ey = k2, ox = -1, oy = k2; break;
            default:               ex = w - 1, ey = k2, ox = w, oy = k2; break;
            }
            if (MapGridGetCollisionAt(ox + MAP_OFFSET, oy + MAP_OFFSET))
                continue;
            if (px == ex && py == ey)
                return step;   // (on the edge: over it)
            kk = MP_TestWalkKey(ex, ey);
            if (kk != 0xFFFF && (best < 0 || abs(ex - px) + abs(ey - py) < best))
                best = abs(ex - px) + abs(ey - py), key = kk;
        }
        if (key != 0xFFFF)
            return key;
    }
    return 0xFFFF;
}

u16 MP_TestGotoKey(u8 group, u8 num, s32 tx, s32 ty)
{
    static s16 sPrev[ORRE_TEST_MAPS];
    static u8 sQ[ORRE_TEST_MAPS];
    u8 here = gSaveBlock1Ptr->location.mapNum, hop;
    s32 head = 0, tail = 0, m, i;
    const struct MapHeader *h;
    if (gSaveBlock1Ptr->location.mapGroup == group && here == num)
        return MP_TestWalkKey(tx, ty);
    if (group != 45 && gSaveBlock1Ptr->location.mapGroup != 45)
        return GotoKeyAnywhere(group, num, tx, ty);
    if (group != 45 || gSaveBlock1Ptr->location.mapGroup != 45 || num >= ORRE_TEST_MAPS || here >= ORRE_TEST_MAPS)
        return 0xFFFF;
    for (i = 0; i < ORRE_TEST_MAPS; i++)
        sPrev[i] = -1;
    sPrev[here] = here;
    sQ[tail++] = here;
    while (head < tail && sPrev[num] < 0)
    {
        u8 c = sQ[head++];
        h = Overworld_GetMapHeaderByGroupAndId(45, c);
        for (i = 0; h && h->events && i < h->events->warpCount; i++)
        {
            const struct WarpEvent *w = &h->events->warps[i];
            if (w->mapGroup == 45 && w->mapNum < ORRE_TEST_MAPS && sPrev[w->mapNum] < 0)
                sPrev[w->mapNum] = c, sQ[tail++] = w->mapNum;
        }
        if (OrreHasWayOut(h))
            for (m = 0; m < ORRE_TEST_MAPS; m++)
                if (sPrev[m] < 0 && Orre_WorldMapShows((45 << 8) | m))
                    sPrev[m] = c, sQ[tail++] = m;
    }
    if (sPrev[num] < 0)
        return 0xFFFF;
    for (hop = num; sPrev[hop] != here; hop = sPrev[hop])
        ;
    // (the way to the next room from here: one of its warps' tiles a step can reach, else a way out to the world map)
    h = &gMapHeader;
    for (i = 0; h->events && i < h->events->warpCount; i++)
    {
        const struct WarpEvent *w = &h->events->warps[i];
        u16 k;
        if (w->mapGroup != 45 || w->mapNum != hop || (k = MP_TestWalkKey(w->x, w->y)) == 0xFFFF)
            continue;
        return k == 0 ? 0xFFFF : k;   // (on it already: its warp on its way)
    }
    for (i = 0; h->events && i < h->events->coordEventCount; i++)
    {
        extern const u8 Orre_EventScript_WorldMap[];
        const struct CoordEvent *c = &h->events->coordEvents[i];
        u16 k;
        if (c->script != Orre_EventScript_WorldMap || (k = MP_TestWalkKey(c->x, c->y)) == 0xFFFF)
            continue;
        Orre_WorldMapTestPick((45 << 8) | hop);
        return k == 0 ? 0xFFFF : k;
    }
    return 0xFFFF;
}

// The harness's "approach KIND N": up to ORRE person N (KIND 0), into touch spot N (1) or up to check spot N (2) of the
// current room (N: a spot's collision id), as a player walks; for a person or a check, facing it. *DONE: there (a
// person's / a check's A is the harness's to press)
bool8 Orre_TestPersonTile(u32 index, s16 *x, s16 *y);
bool8 Orre_TestSpotTiles(bool8 touch, u32 col, s16 *x0, s16 *y0, s16 *x1, s16 *y1);
u16 MP_TestApproachKey(u8 kind, u32 n, bool8 *done)
{
    static const s8 dx[4] = {0, 0, -1, 1}, dy[4] = {1, -1, 0, 0};
    static const u16 toward[4] = {DPAD_UP, DPAD_DOWN, DPAD_RIGHT, DPAD_LEFT};   // (from the side, facing the thing)
    static const u8 facing[4] = {DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST};
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x0, y0, x1, y1, x, y;
    s32 d;
    u16 best = 0xFFFF;
    *done = FALSE;
    if (kind == 0)
    {
        if (!Orre_TestPersonTile(n, &x0, &y0))
            return 0xFFFF;
        x1 = x0, y1 = y0;
    }
    else if (!Orre_TestSpotTiles(kind == 1, n, &x0, &y0, &x1, &y1))
        return 0xFFFF;
    if (kind == 1)
    {
        for (y = y0; y <= y1; y++)
            for (x = x0; x <= x1; x++)
            {
                u16 k = MP_TestWalkKey(x, y);
                if (k == 0)
                    return *done = TRUE, 0;
                if (k != 0xFFFF && best == 0xFFFF)
                    best = k;
            }
        return best;
    }
    // (beside it, a side a step reaches, facing it)
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
            for (d = 0; d < 4; d++)
            {
                s16 sx = x + dx[d], sy = y + dy[d];
                u16 k;
                if (sx >= x0 && sx <= x1 && sy >= y0 && sy <= y1)
                    continue;
                k = MP_TestWalkKey(sx, sy);
                if (k == 0)
                {
                    if (p->facingDirection == facing[d])
                        return *done = TRUE, 0;
                    return toward[d];   // (a tap turns the player to face it)
                }
                if (k != 0xFFFF && best == 0xFFFF)
                    best = k;
            }
    return best;
}

int MP_TestCondition(const char *name)
{
    if (strcmp(name, "cards") == 0)
        return Tcg_IsActive();
    if (strcmp(name, "log") == 0)   // harness waitlog
    {
        extern int gMpLogSeen;
        return gMpLogSeen;
    }
    if (strcmp(name, "setup") == 0)
        return MP_Setup_IsReady();
    if (strncmp(name, "orreflag:", 9) == 0)   // ORRE's story flag N is set (tools/orre/gen_story_tests.py's chain)
        return Orre_Flag(atoi(name + 9));
    if (strncmp(name, "inmap:", 6) == 0)   // the player is on map GROUP,NUM
    {
        int g = -1, m = -1;
        sscanf(name + 6, "%d,%d", &g, &m);
        return gSaveBlock1Ptr->location.mapGroup == g && gSaveBlock1Ptr->location.mapNum == m;
    }
    if (strcmp(name, "field") == 0)
        return MP_IsLocalFree();
    if (strcmp(name, "down") == 0)
        return MP_Rescue_Down();
    if (strcmp(name, "helpask") == 0)
        return MP_Rescue_Asked();
    if (strcmp(name, "photo") == 0)
        return MP_Photo_Done();
    if (strcmp(name, "notice") == 0)
    {
        bool8 MP_Notice_Active(void);
        return MP_Notice_Active();
    }
    if (strcmp(name, "naming") == 0)
    {
        bool8 NamingScreen_IsActive(void);
        return NamingScreen_IsActive() && !gPaletteFade.active;
    }
    if (strcmp(name, "mainmenu") == 0)
    {
        bool8 MP_MainMenu_Active(void);   // (main_menu.c: CONTINUE / NEW GAME / OPTION / SAVE FILE waits for a key)
        return MP_MainMenu_Active();
    }
    if (strcmp(name, "regionmenu") == 0)
    {
        bool8 MP_IntroRegionMenu_Active(void);
        return MP_IntroRegionMenu_Active();
    }
    if (strcmp(name, "startertable") == 0)   // the KANTO / JOHTO starter table, choosing
    {
        bool8 MP_StarterTable_Active(void);
        return MP_StarterTable_Active();
    }
    if (strcmp(name, "pokegear") == 0)   // JOHTO's POKéGEAR, up and settled (mp_pokegear.c)
    {
        bool8 MP_Pokegear_Active(void);
        return MP_Pokegear_Active();
    }
    if (strcmp(name, "introlook") == 0)
    {
        bool8 MP_IntroLook_Active(void);
        return MP_IntroLook_Active();
    }
    if (strcmp(name, "online") == 0)
        return MP_IsConnected() && MP_ConnectedCount() >= 2;
    if (strcmp(name, "inroom") == 0)
        return MP_Lobby_InRoom();
    if (strcmp(name, "joinlist") == 0)
        return MP_Setup_ScreenIs(2);
    if (strcmp(name, "message") == 0)
        return MP_Setup_ScreenIs(4);
    if (strcmp(name, "patchlist") == 0)   // the CO-OP setup screen's PATCH NOTES list
        return MP_Setup_ScreenIs(5);
    if (strcmp(name, "patchtext") == 0)   // ... and one version line's notes
        return MP_Setup_ScreenIs(6);
    if (strcmp(name, "easychat") == 0)   // the easy chat word screen is up
    {
        bool8 MP_EasyChatOpen(void);
        return MP_EasyChatOpen();
    }
    if (strcmp(name, "partymenu") == 0)   // the party menu is up (party_menu.c)
    {
        bool8 MP_PartyMenuOpen(void);
        return MP_PartyMenuOpen();
    }
        if (strcmp(name, "yesno") == 0)   // a script's YES / NO box is up, or a menu's (menu_helpers.c)
    {
        bool8 MP_ScriptYesNoOpen(void);
        bool8 MP_MenuYesNoOpen(void);
        return MP_ScriptYesNoOpen() || MP_MenuYesNoOpen();
    }
    if (strcmp(name, "battle") == 0)
        return gMain.inBattle;
    if (strcmp(name, "free") == 0)   // on the field and free to move (no script, menu or fade)
        return MP_IsLocalFree();
    if (strcmp(name, "surfing") == 0)   // surfing and free to move
        return (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING) && MP_IsLocalFree();
    if (strcmp(name, "scrollmenu") == 0)   // a scrollable multichoice is up (field_specials.c)
    {
        bool8 MP_ScrollMultichoiceOpen(void);
        return MP_ScrollMultichoiceOpen();
    }
    if (strcmp(name, "multichoice") == 0)   // a script's multichoice menu is up
    {
        bool8 MP_ScriptMultichoiceOpen(void);
        bool8 Sinnoh_MenuOpen(void);   // (SINNOH's Platinum menus: s3d_scrcmd.c)
        return MP_ScriptMultichoiceOpen() || Sinnoh_MenuOpen();
    }
    if (strcmp(name, "partymenu") == 0)   // the party screen is up (e.g. picking the next Moemon in battle)
    {
        bool8 MP_PartyMenu_IsOpen(void);
        return MP_PartyMenu_IsOpen() && !gPaletteFade.active;
    }
    if (strcmp(name, "crushask") == 0)
    {
        bool8 MP_Crush_DebugAsking(void);
        return MP_Crush_DebugAsking();
    }
    if (strcmp(name, "summary") == 0)   // the summary screen is up (e.g. picking a move to forget)
    {
        bool8 MP_SummaryScreen_IsOpen(void);
        return MP_SummaryScreen_IsOpen();
    }
    if (strcmp(name, "nofade") == 0)
        return !gPaletteFade.active;
    if (strcmp(name, "action") == 0)
    {
        bool8 MP_Test_IsChoosingAction(void);
        return gMain.inBattle && MP_Test_IsChoosingAction();
    }
    if (strcmp(name, "joinprompt") == 0)   // "X wants to join your battle! Is this okay?" with its YES / NO up
    {
        bool8 MP_Join_PromptUp(void);
        return gMain.inBattle && MP_Join_PromptUp();
    }
    if (strncmp(name, "friendbattle", 12) == 0)   // friendbattle[N]: another player (N others) in a battle
    {
        int i, n = 0, want = name[12] ? atoi(name + 12) : 1;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (i != gMpLocalId && gMpPlayers[i].present && (gMpPlayers[i].state.flags & MPF_IN_BATTLE))
                n++;
        return n >= want;
    }
    if (strncmp(name, "joinpending", 11) == 0)   // joinpendingN: N friends asked to join our battle (next turn's prompt)
    {
        u8 MP_Join_PendingCount(void);
        return MP_Join_PendingCount() >= atoi(name + 11);
    }
    if (strncmp(name, "slot:", 5) == 0)    // slot:BOX:POS:SPECIES - the shared PC's slot (as we see it) has it (0: empty)
    {
        struct PokemonStorage *MP_SharedBox_Mirror(void);
        int box = 1, pos = 0, species = 0;
        struct BoxPokemon *b;
        sscanf(name + 5, "%d:%d:%d", &box, &pos, &species);
        if (box < 1 || box > TOTAL_BOXES_COUNT || pos < 0 || pos >= IN_BOX_COUNT)
            return FALSE;
        b = &MP_SharedBox_Mirror()->boxes[box - 1][pos];
        if (!GetBoxMonData(b, MON_DATA_SANITY_HAS_SPECIES))
            return species == 0;
        return (int)GetBoxMonData(b, MON_DATA_SPECIES) == species;
    }
    if (strncmp(name, "boxname:", 8) == 0) // boxname:BOX:NAME - the shared PC's box (as we see it) is called NAME
    {
        struct PokemonStorage *MP_SharedBox_Mirror(void);
        void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
        int box = atoi(name + 8);
        const char *want = strchr(name + 8, ':');
        char have[16];
        if (box < 1 || box > TOTAL_BOXES_COUNT || want == NULL)
            return FALSE;
        MP_GameToAscii(have, MP_SharedBox_Mirror()->boxNames[box - 1], BOX_NAME_LENGTH);
        return strcmp(have, want + 1) == 0;
    }
    if (strcmp(name, "sharedsteady") == 0) // the shared PC's box screen, nothing waiting on the server
        return MP_SharedBox_Loaded() && !MP_SharedBox_Busy() && MP_PcStorage_Steady();
    if (strcmp(name, "sharedclosed") == 0) // the shared PC's Pokémon all sent and done with
        return !MP_SharedBox_IsOpen();
    if (strcmp(name, "pcstorage") == 0)    // a PC box screen
        return MP_PcStorage_IsOpen();
    if (strcmp(name, "jaridle") == 0)      // the CANDY JAR's menu, done pouring or converting
        return MP_Candy_JarOpen() && !MP_Candy_JarBusy();
    if (strcmp(name, "jar") == 0)          // the CANDY JAR's menu (it may still be pouring)
        return MP_Candy_JarOpen();
    if (strcmp(name, "watchmenu") == 0)    // SPECTATE / BET / JOIN / CANCEL in front of a battling friend
    {
        bool8 MP_Spectate_ChoiceUp(void);
        return MP_Spectate_ChoiceUp();
    }
    return 0;
}

// Test harness: "mp COMMAND ARG"
void MP_OnFileDropped(const char *path);
void MP_TestCommand(const char *cmd, const char *arg)
{
    if (strcmp(cmd, "cards") == 0)
    {
        Tcg_Debug(arg);
        return;
    }
    if (strcmp(cmd, "ygo") == 0)
    {
        Ygo_Debug(arg);
        return;
    }
    if (strcmp(cmd, "tt") == 0)
    {
        Tt_Debug(arg);
        return;
    }
    if (strcmp(cmd, "journal") == 0)   // SINNOH's JOURNAL's page (src/sinnoh/s3d_journal.c); `mp journal open`
    {
        void Sinnoh_JournalDebug(void);
        void Sinnoh_JournalOpen(MainCallback back, bool8 continuing);
        Sinnoh_JournalDebug();
        if (arg && strcmp(arg, "open") == 0)
        {
            SetMainCallback1(NULL);
            Sinnoh_JournalOpen(CB2_ReturnToField, FALSE);
        }
        return;
    }
    if (strcmp(cmd, "mtg") == 0)
    {
        void Mtg_Debug(const char *command);
        Mtg_Debug(arg);
        return;
    }
    if (strcmp(cmd, "export") == 0)
    {
        char path[200];
        bool8 ok = MP_Avatar_ExportLocal(path, sizeof(path));
        printf("[test] export %s: %s\n", path, ok ? "ok" : "FAILED");
    }
    else if (strcmp(cmd, "wild") == 0)
    {
        // wild SPECIES:LEVEL [M1 M2 M3 M4] - start a wild battle right now (with only these moves, if given: a test
        // that needs the foe to hit doesn't leave it to the moves it picks)
        int species = 263, level = 3, m[MAX_MON_MOVES] = {0}, n, i;
        n = sscanf(arg, "%d:%d %d %d %d %d", &species, &level, &m[0], &m[1], &m[2], &m[3]);
        ZeroEnemyPartyMons();
        CreateMon(&gEnemyParty[0], species, level, 32, 0, 0, OT_ID_PLAYER_ID, 0);
        if (n >= 3)
            for (i = 0; i < MAX_MON_MOVES; i++)
                SetMonMoveSlot(&gEnemyParty[0], m[i], i);
        BattleSetup_StartWildBattle();
    }
    else if (strcmp(cmd, "battlestyle") == 0)
    {
        // battlestyle 0|1 - OPTIONS > BATTLE STYLE (0 SHIFT, 1 SET)
        gSaveBlock2Ptr->optionsBattleStyle = atoi(arg) ? OPTIONS_BATTLE_STYLE_SET : OPTIONS_BATTLE_STYLE_SHIFT;
    }
    else if (strcmp(cmd, "trainer") == 0)
    {
        // trainer ID - a battle against that trainer right now (tests: tools/test/trainer_sweep.sh)
        void MP_Debug_TrainerBattle(u16 id);
        printf("[test] trainer battle %d\n", atoi(arg));
        MP_Debug_TrainerBattle(atoi(arg));
    }
    else if (strcmp(cmd, "stadium") == 0)   // mp stadium ROUND DIVISION TRAINER: a STADIUM trainer's team as built (mp_stadium.c)
    {
        void MP_Stadium_Debug(u8 round, u8 division, u8 index);
        int r = 0, d = 0, t = 0;
        sscanf(arg, "%d %d %d", &r, &d, &t);
        MP_Stadium_Debug(r, d, t);
    }
    else if (strcmp(cmd, "stadiumai") == 0)   // mp stadiumai: STADIUM trainers' switching on fixed matchups (mp_stadium_ai.c)
    {
        void MP_StadiumAi_SelfTest(void);
        void MP_StadiumAi_Force(bool8 on);
        if (strncmp(arg, "force", 5) == 0)
            MP_StadiumAi_Force(arg[5] != ' ' || atoi(arg + 6) != 0);   // mp stadiumai force [0]: take every switch
        else
            MP_StadiumAi_SelfTest();
    }
    else if (strcmp(cmd, "stadiumwin") == 0)   // mp stadiumwin ROUND CUP CLASS HARDNESS: the party wins it (mp_stadium_rewards.c)
    {
        void MP_StadiumRewards_Debug(u8 round, u8 cup, u8 tier, u8 hardness);
        void MP_StadiumUi_DebugResult(u8 round, u8 cup, u8 tier);
        int r = 0, c = 0, t = 0, h = 0, show = 0;
        sscanf(arg, "%d %d %d %d %d", &r, &c, &t, &h, &show);
        MP_StadiumRewards_Debug(r, c, t, h);
        if (show)   // (then the result screen with the prizes)
            MP_StadiumUi_DebugResult(r, c, t);
    }
    else if (strcmp(cmd, "stadiumskip") == 0)   // mp stadiumskip N: the next STADIUM run starts at its trainer N (mp_stadium_cup.c)
    {
        void MP_StadiumCup_DebugSkip(u8 n);
        MP_StadiumCup_DebugSkip(atoi(arg));
    }
    else if (strcmp(cmd, "stadiumbadges") == 0)   // mp stadiumbadges ROUND MASK: the GYM LEADER CASTLE's badges (mp_stadium_cup.c)
    {
        void MP_StadiumCup_DebugBadges(u8 round, u8 mask);
        int r = 0, m = 0;
        sscanf(arg, "%d %d", &r, &m);
        MP_StadiumCup_DebugBadges(r, m);
    }
    else if (strcmp(cmd, "ccteam") == 0)   // mp ccteam CLASS ROUND TRAINER (255: the player): a CHALLENGE CUP team (mp_stadium.c)
    {
        void MP_StadiumCc_Debug(u8 cls, u8 round, u8 trainer);
        int c = 0, r = 0, t = 255;
        sscanf(arg, "%d %d %d", &c, &r, &t);
        MP_StadiumCc_Debug(c, r, t);
    }
    else if (strcmp(cmd, "sumstate") == 0)   // the summary screen's page and input mode (mp summary opens it)
    {
        void MP_Summary_Debug(void);
        MP_Summary_Debug();
    }
    else if (strcmp(cmd, "pmd") == 0)   // mp pmd [SLOT]: a follower's PMD Collab picture (mp_pmd.c)
    {
        void MP_Follower_PmdDebug(u8 slot);
        MP_Follower_PmdDebug(arg[0] ? atoi(arg) : 0);
    }
    else if (strcmp(cmd, "landwild") == 0)
    {
        // landwild - a wild battle with a Pokémon from this map's grass table, as an encounter makes it (its level
        // through ChooseWildMonLevel: level scaling, mp_levelscale.c)
        bool8 MP_RollExtraLandMon(struct Pokemon *out);   // wild_encounter.c
        struct Pokemon m;
        if (MP_RollExtraLandMon(&m))
        {
            ZeroEnemyPartyMons();
            gEnemyParty[0] = m;
            printf("[test] landwild: %d Lv%d\n", (int)GetMonData(&m, MON_DATA_SPECIES), (int)GetMonData(&m, MON_DATA_LEVEL));
            BattleSetup_StartWildBattle();
        }
    }
    else if (strcmp(cmd, "triple") == 0)
    {
        // triple S:L S:L S:L [S:L...] - a local wild Triple Battle (you command party
        // slots 0-2); up to six foes, the 4th-6th come in when one faints
        void BattleSetup_StartLocalTripleWildBattle(void);
        int sp[PARTY_SIZE] = {288, 286, 290}, lv[PARTY_SIZE] = {5, 5, 5}, i, n = 0, used;
        const char *p = arg;
        // (S:Lf / S:Lm: that one female / male, for tests that check a card's gender icon)
        char gender[PARTY_SIZE] = {0};
        while (n < PARTY_SIZE && sscanf(p, "%d:%d%n", &sp[n], &lv[n], &used) == 2)
        {
            p += used;
            if (*p == 'f' || *p == 'm')
                gender[n] = *p++;
            n++;
        }
        if (n < 3)
            n = 3;
        ZeroEnemyPartyMons();
        for (i = 0; i < n; i++)
        {
            if (gender[i])
                CreateMonWithGenderNatureLetter(&gEnemyParty[i], sp[i], lv[i], 32, gender[i] == 'f' ? MON_FEMALE : MON_MALE,
                                                0, 0);
            else
                CreateMon(&gEnemyParty[i], sp[i], lv[i], 32, 0, 0, OT_ID_PLAYER_ID, 0);
        }
        gMpDebugTriple = getenv("MOEMON_TRIPLEDEBUG") != NULL;
        BattleSetup_StartLocalTripleWildBattle();
    }
    else if (strcmp(cmd, "foehold") == 0)
    {
        // foehold ITEM - in a battle: the first foe holds ITEM (tests of held items on the other side)
        u8 b = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        gBattleMons[b].item = atoi(arg);
        SetMonData(&gEnemyParty[gBattlerPartyIndexes[b]], MON_DATA_HELD_ITEM, &gBattleMons[b].item);
        printf("[test] foehold: battler %d item %d\n", b, gBattleMons[b].item);
    }
    else if (strcmp(cmd, "g4side") == 0)
    {
        // g4side SIDE ROCK TSPIKES WISH - Gen 4 hazards on a battle side (0 = the player's), for tests
        int side = 0, rock = 0, tsp = 0, wish = 0;
        sscanf(arg, "%d %d %d %d", &side, &rock, &tsp, &wish);
        gGen4Sides[side & 1].stealthRock = rock;
        gGen4Sides[side & 1].toxicSpikes = tsp;
        gGen4Sides[side & 1].healingWish = wish != 0;
        gGen4Sides[side & 1].lunarDance = wish == 2;
    }
    else if (strcmp(cmd, "double") == 0)
    {
        // double S:L S:L - a local wild Double Battle (you command party slots 0-1)
        void BattleSetup_StartLocalDoubleWildBattle(void);
        int sp[2] = {288, 286}, lv[2] = {5, 5};
        sscanf(arg, "%d:%d %d:%d", &sp[0], &lv[0], &sp[1], &lv[1]);
        ZeroEnemyPartyMons();
        CreateMon(&gEnemyParty[0], sp[0], lv[0], 32, 0, 0, OT_ID_PLAYER_ID, 0);
        CreateMon(&gEnemyParty[1], sp[1], lv[1], 32, 0, 0, OT_ID_PLAYER_ID, 0);
        BattleSetup_StartLocalDoubleWildBattle();
    }
    else if (strcmp(cmd, "dumpcolors") == 0)
    {
        // dumpcolors DIR - sprites and pictures for tools/moemon_frlg/colormaps.py
        void MP_Avatar_DumpColorAssets(const char *dir);
        MP_Avatar_DumpColorAssets(arg);
    }
    else if (strcmp(cmd, "card") == 0)
    {
        // card - the player's trainer card (front of it)
        void ShowPlayerTrainerCard(void (*callback)(void));
        void CB2_ReturnToFieldContinueScript(void);
        ShowPlayerTrainerCard(CB2_ReturnToFieldContinueScript);
    }
    else if (strcmp(cmd, "finder") == 0)
    {
        // finder STYLE: OPTIONS > CO-OP > FRIEND FINDER (0 both, 1 arrows, 2 radar)
        gMpConfig.finderStyle = atoi(arg) % MP_FINDER_COUNT;
        MPNet_Log("friend finder: style %d", gMpConfig.finderStyle);
    }
    else if (strcmp(cmd, "townmap") == 0 || strcmp(cmd, "flymap") == 0)
    {
        extern void FieldInitRegionMap(MainCallback callback);
        extern void CB2_OpenFlyMap(void);
        extern void CB2_ReturnToField(void);
        if (cmd[0] == 't')
            FieldInitRegionMap(CB2_ReturnToField);
        else
            SetMainCallback2(CB2_OpenFlyMap);
    }
    else if (strcmp(cmd, "dexcatch") == 0 || strcmp(cmd, "dexsee") == 0)
    {
        // dexcatch NATIONAL_NUMBER [LAST]: as if you caught (or saw) it (or each up to LAST)
        int first = 0, last = 0, k;
        u16 nat;
        if (sscanf(arg, "%d %d", &first, &last) < 2)
            last = first;
        for (k = first; k <= last; k++)
        {
            nat = (u16)k;
            GetSetPokedexFlag(nat, FLAG_SET_SEEN);
            if (cmd[3] == 'c')
                GetSetPokedexFlag(nat, FLAG_SET_CAUGHT);
        }
        nat = (u16)first;
        printf("[test] dex %d seen %d caught %d\n", nat, GetSetPokedexFlag(nat, FLAG_GET_SEEN), GetSetPokedexFlag(nat, FLAG_GET_CAUGHT));
    }
    else if (strcmp(cmd, "shadows") == 0)   // shadows [0|1] - OPTIONS > DISPLAY > SHADOWS; prints how many are up (mp_shadow.c)
    {
        u32 i, n = 0;
        if (arg[0])
        {
            gMpConfig.noShadows = atoi(arg) == 0;
            printf("[test] shadows set %s\n", gMpConfig.noShadows ? "off" : "on");
            return;
        }
        for (i = 0; i < MAX_SPRITES; i++)
            if (gSprites[i].inUse && gSprites[i].mpShadow)
            {
                n++;
            }
        {
            extern unsigned int MP_ShadowKeep(void);   // gba_easy_draw.c: how much of the darkening shows (48 full, 64 none)
            void MP_Shadow_Debug(void);
            MP_Shadow_Debug();
            printf("[test] shadows %s: %u (keep %u)\n", gMpConfig.noShadows ? "off" : "on", (unsigned)n, MP_ShadowKeep());
        }
    }
    else if (strcmp(cmd, "s3ddoor") == 0)   // s3ddoor KIND - SINNOH's door prop in front of the player: its animation KIND
    {
        s16 x, y;
        PlayerGetDestCoords(&x, &y);
        printf("[test] s3ddoor %d\n", S3D_DoorAnimate(x, y - 1, atoi(arg)));
    }
    else if (strcmp(cmd, "s3dtex") == 0)   // s3dtex - SINNOH's animated map textures: how many, the first's frame now
    {
        void S3D_TexAnimInfo(u32 *count, u32 *firstShown);
        u32 n, shown;
        S3D_TexAnimInfo(&n, &shown);
        printf("[test] s3dtex %u animated, first shows %u\n", (unsigned)n, (unsigned)shown);
    }
    else if (strcmp(cmd, "healballs") == 0)   // healballs - the balls SINNOH's healing machine has set so far (s3d_heal.c)
    {
        u8 S3D_Heal_Shown(void);
        printf("[test] healballs %d\n", S3D_Heal_Shown());
    }
    else if (strcmp(cmd, "shadowwatch") == 0)   // shadowwatch N - how far the player's shadow moves against them (mp_shadow.c)
    {
        void MP_Shadow_Watch(u16 frames);
        MP_Shadow_Watch(atoi(arg));
    }
    else if (strcmp(cmd, "followsprites") == 0)
    {
        // followsprites N - OPTIONS > GAME > FOLLOWERS: 0 default, 1 pmd, 2 original
        gMpConfig.followerSprites = atoi(arg) % MP_FOLLOW_SPRITES_COUNT;
        printf("[test] follower sprites %d\n", gMpConfig.followerSprites);
    }
    else if (strcmp(cmd, "farm") == 0)
    {
        // farm state|plant PLOT BERRY [STAGE]|grow MINUTES|ripe PLOT: the MOéMON GARDEN's BERRY farm (mp_farm.c)
        MP_Farm_Debug(arg);
    }
    else if (strcmp(cmd, "life") == 0)
    {
        // life show|remember|minutes|trait|count|save|load|reveal|journal ... (mp_life.c)
        MP_Life_Debug(arg);
    }
    else if (strcmp(cmd, "sinnohsprites") == 0)
    {
        // sinnohsprites N: OPTIONS > DISPLAY > SPRITE SCALING > SINNOH PEOPLE (0 GEN 4, 1 MIXED, 2 GEN 3; the map's people at the next map)
        gMpConfig.sinnohSprites = atoi(arg) % 3;
        printf("[test] sinnoh sprites %d\n", gMpConfig.sinnohSprites);
    }
    else if (strcmp(cmd, "objpal") == 0)
    {
        // objpal MINGFX: each object with a graphics id from MINGFX on: its graphics id, its sprite's palette slot and colors (shown, unfaded)
        u8 id;
        for (id = 0; id < OBJECT_EVENTS_COUNT; id++)
        if (gObjectEvents[id].active && OBJ_GFX_ID(&gObjectEvents[id]) >= atoi(arg))
        {
            struct Sprite *sp = &gSprites[gObjectEvents[id].spriteId];
            int k, slot = sp->oam.paletteNum;
            printf("[test] objpal %d: gfx %#x %dx%d slot %d tag %#x:", id, (int)OBJ_GFX_ID(&gObjectEvents[id]), GetObjectEventGraphicsInfo(OBJ_GFX_ID(&gObjectEvents[id]))->width,
                   GetObjectEventGraphicsInfo(OBJ_GFX_ID(&gObjectEvents[id]))->height, slot, GetSpritePaletteTagByPaletteNum(slot));
            for (k = 0; k < 16; k++)
                printf(" %04x/%04x", gPlttBufferFaded[OBJ_PLTT_ID(slot) + k], gPlttBufferUnfaded[OBJ_PLTT_ID(slot) + k]);
            printf("\n");
        }
    }
    else if (strcmp(cmd, "spritescale") == 0)
    {
        // spritescale FOLLOWERS GARDEN: OPTIONS > DISPLAY > SPRITE SCALING, % of the way to their POKeDEX height
        // (0 off; 1 on at its percent, more: on at that percent)
        int f = 1, g = 1;
        sscanf(arg, "%d %d", &f, &g);
        gMpConfig.scaleFollowers = f > 0;
        gMpConfig.scaleGarden = g > 0;
        if (f > 1)
            gMpConfig.scaleFollowersPct = f > 100 ? 100 : f;
        if (g > 1)
            gMpConfig.scaleGardenPct = g > 100 ? 100 : g;
        printf("[test] sprite scaling: followers %d garden %d\n", MP_SCALE_FOLLOWERS_NOW(), MP_SCALE_GARDEN_NOW());
    }
    else if (strcmp(cmd, "phone") == 0)
    {
        // phone [pair|forget]: the phone link's state (mp_phone.c); pair: pairing open, its code and secret printed
        void MP_Phone_Debug(const char *arg);
        MP_Phone_Debug(arg);
    }
    else if (strcmp(cmd, "unownsee") == 0)
    {
        // unownsee LETTER [LETTER...]: UNOWN letters 0-27 seen; formviewer: ROUTE 218's POKeDEX upgrade (s3d_legends.c)
        void Sinnoh_UnownSeen(u32 personality);
        const char *p = arg;
        int l, used;
        while (sscanf(p, "%d%n", &l, &used) == 1)
        {
            Sinnoh_UnownSeen((l & 3) | (((l >> 2) & 3) << 8) | (((l >> 4) & 3) << 16) | (((l >> 6) & 3) << 24));
            p += used;
        }
    }
    else if (strcmp(cmd, "formviewer") == 0)
    {
        void Sinnoh_FormViewerEnable(void);
        Sinnoh_FormViewerEnable();
    }
    else if (strcmp(cmd, "dexorder") == 0)
    {
        // dexorder MODE ORDER: the POKéDEX list's mode (0 HOENN, 1 NATIONAL) and order (1 A-Z: only seen ones)
        int mode = 0, order = 0;
        sscanf(arg, "%d %d", &mode, &order);
        if (mode == 1 && !IsNationalPokedexEnabled())
            EnableNationalPokedex();
        gSaveBlock2Ptr->pokedex.mode = mode;
        gSaveBlock2Ptr->pokedex.order = order;
        printf("[test] dex mode %d order %d\n", mode, order);
    }
    else if (strcmp(cmd, "dexcount") == 0)
    {
        // dexcount: the NATIONAL DEX's size at the GENERATION and the counts in it (pokedex.c)
        printf("[test] dexcount size %u seen %u caught %u all %d\n", MP_Gen_DexCount(), GetNationalPokedexCount(FLAG_GET_SEEN),
               GetNationalPokedexCount(FLAG_GET_CAUGHT), HasAllMons());
    }
    else if (strcmp(cmd, "dexcheck") == 0)
    {
        // dexcheck [NATIONAL_NUMBER]: every species' long Pokedex text built and checked (pokedex.c)
        MP_Dex_DebugTexts((u16)atoi(arg));
    }
    else if (strcmp(cmd, "dexget") == 0)
    {
        u16 nat = (u16)atoi(arg);
        printf("[test] dex %d caught by %s\n", nat, MP_Dex_CaughtBy(nat) ? "a friend" : "-");
        printf("[test] dex %d seen %d caught %d\n", nat, GetSetPokedexFlag(nat, FLAG_GET_SEEN), GetSetPokedexFlag(nat, FLAG_GET_CAUGHT));
    }
    else if (strcmp(cmd, "mkbase") == 0)
    {
        // mkbase ID - make our own Secret Base there (181 = a tree on Route 110), with a few decorations
        struct SecretBase *b = &gSaveBlock1Ptr->secretBases[0];
        int id = 181;
        sscanf(arg, "%d", &id);
        memset(b, 0, sizeof(*b));
        b->secretBaseId = id;
        b->gender = gSaveBlock2Ptr->playerGender;
        memcpy(b->trainerName, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
        memcpy(b->trainerId, gSaveBlock2Ptr->playerTrainerId, TRAINER_ID_LENGTH);
        b->language = GAME_LANGUAGE;
        b->decorations[0] = 1;  b->decorationPositions[0] = 0x34;   // a desk
        b->decorations[1] = 85; b->decorationPositions[1] = 0x52;   // a doll
        FlagSet(FLAG_RECEIVED_SECRET_POWER);
        printf("[test] base %d\n", id);
    }
    else if (strcmp(cmd, "daycare") == 0)
    {
        // daycare N - leave party POKéMON N at the DAY CARE
        void StoreSelectedPokemonInDaycare(void);
        int n = 0;
        sscanf(arg, "%d", &n);
        gSpecialVar_0x8004 = n;
        StoreSelectedPokemonInDaycare();
        printf("[test] daycare: %d in it\n", CountPokemonInDaycare(&gSaveBlock1Ptr->daycare));
    }
    else if (strcmp(cmd, "breedtest") == 0)
    {
        // breedtest N ITEM_A ITEM_B [BALL_A BALL_B [SLOT_A SLOT_B]] - N eggs from party 1 and 2 (or slots SLOT_A /
        // SLOT_B, 0-5) holding those items, in those balls (daycare.c MP_Daycare_BreedTest)
        void MP_Daycare_BreedTest(int n, u16 itemA, u16 itemB, u16 ballA, u16 ballB, u8 slotA, u8 slotB);
        int n = 200, a = 0, b = 0, ba = 0, bb = 0, sa = 0, sb = 1;
        sscanf(arg, "%d %d %d %d %d %d %d", &n, &a, &b, &ba, &bb, &sa, &sb);
        if (sa < 0 || sa >= PARTY_SIZE || sb < 0 || sb >= PARTY_SIZE)
            sa = 0, sb = 1;
        MP_Daycare_BreedTest(n, a, b, ba, bb, sa, sb);
    }
    else if (strcmp(cmd, "daycarestate") == 0)
    {
        // daycarestate [STEPS] - the DAY CARE pair (species, steps), JOHTO's owners (bit: the MAN's slot), KANTO's
        // ROUTE 5 one; STEPS adds steps to all three first (daycare.c)
        void *MP_Route5Daycare_Data(u32 *size);
        u32 size, add = 0;
        struct DaycareMon *r5 = MP_Route5Daycare_Data(&size);
        int i;
        if (strncmp(arg, "egg", 3) == 0)
            TriggerPendingDaycareEgg();   // (daycarestate egg: an EGG waiting now)
        else
            sscanf(arg, "%u", &add);
        for (i = 0; i < DAYCARE_MON_COUNT; i++)
            if (GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[i].mon, MON_DATA_SPECIES))
                gSaveBlock1Ptr->daycare.mons[i].steps += add;
        if (GetBoxMonData(&r5->mon, MON_DATA_SPECIES))
            r5->steps += add;
        printf("[test] daycare: %d/%u %d/%u owners %d state %d route5 %d/%u\n",
               (int)GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[0].mon, MON_DATA_SPECIES), (unsigned)gSaveBlock1Ptr->daycare.mons[0].steps,
               (int)GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[1].mon, MON_DATA_SPECIES), (unsigned)gSaveBlock1Ptr->daycare.mons[1].steps,
               VarGet(0x5041), GetDaycareState(), (int)GetBoxMonData(&r5->mon, MON_DATA_SPECIES), (unsigned)r5->steps);
    }
    else if (strcmp(cmd, "hmrescue") == 0)
    {
        // hmrescue - the HMs a friend's progress made impossible to get, owed now (mp_hmrescue.c)
        void MP_HmRescue_Debug(void);
        MP_HmRescue_Debug();
    }
    else if (strcmp(cmd, "cardscreen") == 0)
    {
        // cardscreen [DESIGN [PAGE [PANEL [ACCENT [FRAME]]]]] - the TRAINER CARD's screen as it is (page, design,
        // stars: mp_cardscreen.c), or shown so
        void MP_CardScreen_Debug(void);
        void MP_CardScreen_DebugSet(const char *arg);
        if (arg[0])
            MP_CardScreen_DebugSet(arg);
        MP_CardScreen_Debug();
    }
    else if (strcmp(cmd, "tcard") == 0)
    {
        // tcard days N | tcard rub REGION BADGE COUNT | tcard style D A F [P H] | tcard info: the TRAINER CARD's badge records (mp_tcard.c)
        char sub[16] = "";
        int a = 0, b = 0, c = 0;
        sscanf(arg, "%15s %d %d %d", sub, &a, &b, &c);
        if (strcmp(sub, "days") == 0)
            MP_TCard_DebugDays(a);
        else if (strcmp(sub, "rub") == 0)
            while (c-- > 0)
                MP_TCard_Rub(a, b);
        else if (strcmp(sub, "style") == 0)
        {
            // tcard style DESIGN ACCENT FRAME [PANEL HUE]: the card's look (MY CARD)
            struct MpCardStyle style = *MP_TCard_Style();
            int panel = style.panel, hue = style.hue;
            sscanf(arg, "%*s %*d %*d %*d %d %d", &panel, &hue);
            style.design = a, style.accent = b, style.frame = c, style.panel = panel, style.hue = hue;
            MP_TCard_SetStyle(&style);
        }
        {
            int r, k;
            for (r = 0; r < REGION_COUNT; r++)
            {
                printf("[test] tcard region %d:", r);
                for (k = 0; k < 8; k++)
                {
                    const struct MpTCardBadge *t = MP_TCard_Badge(r, k);
                    printf(" %c%d/%d", MP_TCard_HasBadge(r, k) ? '+' : '-', t->polish, t->when != 0);
                }
                printf("\n");
            }
            {
                struct MpCardView *v = malloc(sizeof(*v));
                if (v)
                {
                    MP_CardView_FillOwn(v);
                    printf("[test] tcard photo: tint %u team %u stickers %u %u %u\n", v->style.tint, v->teamCount,
                           TCARD_STICKER(v->stickers, 0), TCARD_STICKER(v->stickers, 1), TCARD_STICKER(v->stickers, 2));
                    free(v);
                }
            }
        }
    }
    else if (strcmp(cmd, "tcgvariety") == 0)
    {
        // tcgvariety - how many different decks card players bring (tcg_field.c Tcg_DebugVariety)
        void Tcg_DebugVariety(void);
        Tcg_DebugVariety();
    }
    else if (strcmp(cmd, "giveegg") == 0)
    {
        // giveegg SPECIES - an EGG of it in the party (as a gift EGG is: its species' egg cycles left)
        u8 ScriptGiveEgg(u16 species);
        u8 r = ScriptGiveEgg((u16)atoi(arg));
        printf("[test] giveegg %d: %d\n", atoi(arg), r);
    }
    else if (strcmp(cmd, "eggsteps") == 0)
    {
        // eggsteps N - N steps of the day care's step counter (daycare.c ShouldEggHatch), then each party EGG's cycles
        bool8 ShouldEggHatch(void);
        int n = atoi(arg), i, hatch = 0;
        for (i = 0; i < n && !hatch; i++)
            hatch = ShouldEggHatch();
        for (i = 0; i < gPlayerPartyCount; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                printf("[test] egg slot %d: %u cycles left after %d steps%s\n", i,
                       (unsigned)GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP), n, hatch ? " (hatching)" : "");
    }
    else if (strcmp(cmd, "credits") == 0)
    {
        // credits - straight into the credits (as after the HALL OF FAME)
        void CB2_StartCreditsSequence(void);
        SetMainCallback2(CB2_StartCreditsSequence);
        printf("[test] credits\n");
    }
    else if (strcmp(cmd, "box") == 0)
    {
        // box - the PC's box in use
        u8 StorageGetCurrentBox(void);
        printf("[test] box %d\n", StorageGetCurrentBox() + 1);
    }
    else if (strcmp(cmd, "boxicons") == 0)
    {
        // boxicons - the PC box's POKéMON icon sprites (pokemon_storage_system.c)
        void MP_Storage_DebugIcons(void);
        MP_Storage_DebugIcons();
    }
    else if (strcmp(cmd, "hatch") == 0)
    {
        // hatch SLOT - the party EGG in SLOT hatches now, as the hatching scene's end does (egg_hatch.c); before and
        // after: species, POKé BALL, ball style, Ability slot
        void ScriptHatchMon(void);
        int slot = atoi(arg) % PARTY_SIZE;
        struct Pokemon *mon = &gPlayerParty[slot];
        printf("[test] hatch: egg %d species %d ball %d style %04x ability %d\n", (int)GetMonData(mon, MON_DATA_IS_EGG),
               (int)GetMonData(mon, MON_DATA_SPECIES), (int)GetMonData(mon, MON_DATA_POKEBALL),
               (int)GetMonData(mon, MON_DATA_ENCRYPT_SEPARATOR), (int)GetMonData(mon, MON_DATA_ABILITY_NUM));
        gSpecialVar_0x8004 = slot;
        ScriptHatchMon();
        printf("[test] hatched: egg %d species %d ball %d style %04x ability %d level %d\n", (int)GetMonData(mon, MON_DATA_IS_EGG),
               (int)GetMonData(mon, MON_DATA_SPECIES), (int)GetMonData(mon, MON_DATA_POKEBALL),
               (int)GetMonData(mon, MON_DATA_ENCRYPT_SEPARATOR), (int)GetMonData(mon, MON_DATA_ABILITY_NUM),
               (int)GetMonData(mon, MON_DATA_LEVEL));
    }
    else if (strcmp(cmd, "takeegg") == 0)
    {
        void GiveEggFromDaycare(void);
        GiveEggFromDaycare();
        printf("[test] took the egg\n");
    }
    else if (strcmp(cmd, "hof") == 0)
    {
        // hof - straight into the Hall of Fame (it saves a record)
        void CB2_DoHallOfFameScreen(void);
        SetMainCallback2(CB2_DoHallOfFameScreen);
    }
    else if (strcmp(cmd, "roamer") == 0)
    {
        void InitRoamer(void);
        gSpecialVar_0x8004 = 0;
        InitRoamer();
        printf("[test] roamer started\n");
    }
    else if (strcmp(cmd, "world") == 0)
    {
        bool8 MP_Roamer_Get(u8 *mapGroup, u8 *mapNum);
        u8 g = 0, n = 0;
        bool8 r = MP_Roamer_Get(&g, &n);
        printf("[test] world: stage %d abnormal %d roamer %d at %d.%d bases:", gSaveBlock1Ptr->weatherCycleStage,
               VarGet(VAR_ABNORMAL_WEATHER_LOCATION), r, g, n);
        for (n = 0; n < 20; n++)
            printf(" %d", gSaveBlock1Ptr->secretBases[n].secretBaseId);
        printf(" tv0kind %d\n", gSaveBlock1Ptr->tvShows[5].common.kind);
    }
    else if (strcmp(cmd, "coopask") == 0)   // coopask N - ask player N (1-based) to team up (as walking up to them does)
    {
        void MP_Coop_DebugAsk(u8 i);
        int n = 0;
        sscanf(arg, "%d", &n);
        if (n >= 1)
            MP_Coop_DebugAsk(n - 1);
    }
    else if (strcmp(cmd, "joinask") == 0)   // joinask N - ask player N (1-based) to let us join their battle (as JOIN does)
    {
        void MP_Join_Ask(u8 target);
        int n = 0;
        sscanf(arg, "%d", &n);
        if (n >= 1)
            MP_Join_Ask(n - 1);
    }
    else if (strcmp(cmd, "animall") == 0)
    {
        int first = 1, last = MOVES_COUNT - 1, atk = 1, tgt = 0, turn = 0;
        u8 t;
        sscanf(arg, "%d %d %d %d %d", &first, &last, &atk, &tgt, &turn);
        t = CreateTask(Task_MpAnimAll, 1);
        gTasks[t].data[0] = first;
        gTasks[t].data[1] = last;
        gTasks[t].data[2] = atk;
        gTasks[t].data[3] = tgt;
        gTasks[t].data[4] = turn;
        printf("[test] animall %d-%d, %d -> %d, turn %d\n", first, last, atk, tgt, turn);
    }
    else if (strcmp(cmd, "fieldweather") == 0)   // fieldweather N - this game's field weather now (WEATHER_*), not the map's
    {
        void SetCurrentAndNextWeatherNoDelay(u8 weather);
        int v = 0;
        sscanf(arg, "%d", &v);
        SetCurrentAndNextWeatherNoDelay(v);
        printf("[test] field weather %d\n", v);
    }
    else if (strcmp(cmd, "setstage") == 0)
    {
        int v = 0;
        sscanf(arg, "%d", &v);
        gSaveBlock1Ptr->weatherCycleStage = v;
    }
    else if (strcmp(cmd, "berrytree") == 0)
    {
        // berrytree TREE BERRY STAGE - plant BERRY (berry number: 47+ are the e-Reader Berries) in tree TREE
        void PlantBerryTree(u8 id, u8 berry, u8 stage, bool8 allowGrowth);
        int tree = 0, berry = 1, stage = 5;
        sscanf(arg, "%d %d %d", &tree, &berry, &stage);
        PlantBerryTree(tree, berry, stage, TRUE);
        printf("[test] berrytree %d = berry %d stage %d\n", tree, berry, stage);
    }
    else if (strcmp(cmd, "berryinfo") == 0)
    {
        // berryinfo TREE - a tree's berry, stage and minutes to the next stage
        struct BerryTree *GetBerryTreeInfo(u8 id);
        int tree = 0;
        struct BerryTree *t;
        sscanf(arg, "%d", &tree);
        t = GetBerryTreeInfo(tree);
        printf("[test] berryinfo %d: berry %d stage %d next %d min yield %d regrow %d (time events %s, wall clock %s, last %d.%02d:%02d, days var %d)\n",
               tree, t->berry, t->stage, t->minutesUntilNextStage, t->berryYield, t->regrowthCount,
               FlagGet(FLAG_SYS_CLOCK_SET) ? "on" : "off", FlagGet(FLAG_SET_WALL_CLOCK) ? "set" : "not set",
               gSaveBlock2Ptr->lastBerryTreeUpdate.days, gSaveBlock2Ptr->lastBerryTreeUpdate.hours,
               gSaveBlock2Ptr->lastBerryTreeUpdate.minutes, VarGet(VAR_DAYS));
    }
    else if (strcmp(cmd, "panel") == 0)
    {
        // panel ON SIZE: the SPECTATE panel (friends' screens under ours), SIZE 1-4
        int on = 1, size = 1;
        sscanf(arg, "%d %d", &on, &size);
        gMpConfig.spectatePanel = on != 0;
        gMpConfig.spectateScale = size < 1 ? 1 : (size > 4 ? 4 : size);
        {
            int i;   // friends with their own settings take the size too
            for (i = 0; i < MP_SPEC_FRIENDS; i++)
                if (gMpConfig.specFriends[i].name[0])
                    gMpConfig.specFriends[i].size = gMpConfig.spectateScale;
        }
        printf("[test] spectate panel %s, size %dx\n", on ? "on" : "off", gMpConfig.spectateScale);
    }
    else if (strcmp(cmd, "optionstab") == 0)
    {
        // optionstab N: OPTIONS (open) shows tab N (0 GAME, 1 BATTLE, 2 DISPLAY, 3 CONTROLS, 4 CO-OP)
        void MP_Options_DebugTab(u8 tab);
        MP_Options_DebugTab(atoi(arg));
    }
    else if (strcmp(cmd, "panelfriend") == 0)
    {
        // panelfriend PLAYER SIDE SIZE POPOUT SHOW: one friend's screen (PLAYER 1-4, SIDE 0 below 1 above 2 left 3 right)
        int pl = 0, side = 0, size = 1, pop = 0, show = 1, i, n;
        char name[MP_NAME_LEN + 1];
        sscanf(arg, "%d %d %d %d %d", &pl, &side, &size, &pop, &show);
        if (pl >= 1 && pl <= MP_MAX_PLAYERS && gMpPlayers[pl - 1].present)
        {
            MP_GameToAscii(name, gMpPlayers[pl - 1].name, MP_NAME_LEN);
            name[MP_NAME_LEN] = 0;
            for (i = 0; i < MP_SPEC_FRIENDS && gMpConfig.specFriends[i].name[0] && strcmp(gMpConfig.specFriends[i].name, name); i++)
                ;
            if (i == MP_SPEC_FRIENDS)
                i = MP_SPEC_FRIENDS - 1;
            n = i;
            snprintf(gMpConfig.specFriends[n].name, sizeof(gMpConfig.specFriends[n].name), "%s", name);
            gMpConfig.specFriends[n].side = side & 3;
            gMpConfig.specFriends[n].size = size < 1 ? 1 : (size > 4 ? 4 : size);
            gMpConfig.specFriends[n].popout = pop != 0;
            gMpConfig.specFriends[n].show = show != 0;
            printf("[test] panelfriend %s side %d size %d popout %d show %d\n", name, side & 3, size, pop, show);
        }
        else
            printf("[test] panelfriend: no player %d\n", pl);
    }
    else if (strcmp(cmd, "namelines") == 0)
    {
        // namelines: the CO-OP menu's players line, the shared PC's "from NAME" and the POKEDEX's "CAUGHT BY NAME"
        // for every player, each built into a buffer full of junk (0xF7 = "insert a stored string", which crashed
        // GetStringWidth): a name without its end marker shows the junk. "|" marks where the text ends.
        u8 buf[128], i;
        char ascii[160];
        memset(buf, 0xF7, sizeof(buf));
        MP_Menu_BuildPlayersLine(buf);
        MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
        printf("[test] namelines: players %s| (%d)\n", ascii, StringLength(buf));
        for (i = 0; i <= MP_MAX_PLAYERS; i++)
        {
            const u8 *name = i == MP_MAX_PLAYERS ? gSaveBlock2Ptr->playerName : gMpPlayers[i].name;
            if (i < MP_MAX_PLAYERS && (i == gMpLocalId || !gMpPlayers[i].present))
                continue;
            memset(buf, 0xF7, sizeof(buf));
            MP_SharedPc_FromLabel(buf, name);
            MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
            printf("[test] namelines: pc %s| (%d)\n", ascii, StringLength(buf));
            memset(buf, 0xF7, sizeof(buf));
            MP_Dex_CaughtByText(buf, name);
            MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
            printf("[test] namelines: dex %s| (%d)\n", ascii, StringLength(buf));
        }
    }
    else if (strcmp(cmd, "gatchaday") == 0)
    {
        // gatchaday N - STANDARD shows day N's prizes (-1: today)
        void MP_Gatcha_SetDay(s32 day);
        int n = -1;
        sscanf(arg, "%d", &n);
        MP_Gatcha_SetDay(n);
    }
    else if (strcmp(cmd, "gatchagive") == 0)
    {
        // gatchagive EV [SHINY [SPECIES]] - an event Moemon from the MoeGatcha list (EV_* order in mp_gatcha.c; the
        // EV_SPECIES* kinds of the later GENERATIONs take the species)
        void MP_Gatcha_DebugGive(u8 ev, bool8 shiny, u16 species);
        int ev = 0, sh = 0, sp = 0;
        sscanf(arg, "%d %d %d", &ev, &sh, &sp);
        MP_Gatcha_DebugGive(ev, sh, sp);
    }
    else if (strcmp(cmd, "gatchareel") == 0)
    {
        void MP_Gatcha_DebugReel(void);
        MP_Gatcha_DebugReel();
    }
    else if (strcmp(cmd, "pickup") == 0)
    {
        // pickup N - show PICKUP number N (0-18) instead of this week's
        void MP_Gatcha_SetPickup(s8 n);
        int n = -1;
        sscanf(arg, "%d", &n);
        MP_Gatcha_SetPickup(n);
    }
    else if (strcmp(cmd, "giveitemscript") == 0)
    {
        // giveitemscript ID - an NPC-style giveitem (the obtain message, a MOEGATCHA TICKET for important items)
        extern const u8 EventScript_MpDebugGiveItem[];
        int id = 0;
        sscanf(arg, "%d", &id);
        VarSet(VAR_0x8004, id);
        ScriptContext_SetupScript(EventScript_MpDebugGiveItem);
    }
    else if (strcmp(cmd, "qzputaway") == 0)
    {
        // qzputaway ID N - KANTO's "put away ... Pocket:" message (Quetzal's script, tests)
        extern const u8 EventScript_MpDebugQzPutAway[];
        int id = 0, n = 1;
        sscanf(arg, "%d %d", &id, &n);
        VarSet(VAR_0x8000, id);
        VarSet(VAR_0x8001, n);
        ScriptContext_SetupScript(EventScript_MpDebugQzPutAway);
    }
    else if (strcmp(cmd, "additem") == 0)
    {
        // additem ID N - straight into the bag
        int id = 0, n = 1;
        sscanf(arg, "%d %d", &id, &n);
        AddBagItem(id, n);
    }
    else if (strcmp(cmd, "checkitem") == 0)
    {
        // checkitem ID - how many the bag holds
        int id = atoi(arg);
        printf("[test] item %d: %d\n", id, CountTotalItemQuantityInBag(id));
    }
    else if (strcmp(cmd, "makerproof") == 0)   // "mp makerproof PATH": M4b's round trip (src/maker/maker_events.c)
    {
        int Maker_RawProof(const char *path);
        Maker_RawProof(arg);
    }
    else if (strcmp(cmd, "makergamescript") == 0)   // "mp makergamescript LABEL": the event "Start from the game's script"
    {                                                // makes of it (each line "[mp] maker script: LINE")
        const char *MakerApi_GameScript(const char *scriptName, int kind);
        const char *body = MakerApi_GameScript(arg, 1), *l;
        if (!body)
            printf("[mp] maker script: none\n");
        for (l = body; l && *l;)
        {
            const char *nl = strchr(l, '\n');
            size_t len = nl ? (size_t)(nl - l) : strlen(l);
            printf("[mp] maker script: %.*s\n", (int)len, l);
            l += len + (nl ? 1 : 0);
        }
    }
    else if (strcmp(cmd, "makerpackcheck") == 0)   // "mp makerpackcheck": every script of the editor's "Start from the
    {                                               // game's script" compiles (src/maker/maker_scripts.c)
        int Maker_GameScriptCount(void);
        const char *Maker_GameScriptLabel(int i);
        const char *MakerApi_GameScript(const char *scriptName, int kind);
        const char *MakerApi_CheckEvent(const char *map, const char *event, int kind, const char *source);
        int i, n = Maker_GameScriptCount(), bad = 0;
        for (i = 0; i < n; i++)
        {
            const char *label = Maker_GameScriptLabel(i), *body = MakerApi_GameScript(label, 0), *l, *err;
            char *src = malloc(strlen(body) * 2 + 16), *d = src;
            for (l = body; *l;)   // (as the editor hands it over: every line indented by two)
            {
                const char *nl = strchr(l, '\n');
                size_t len = nl ? (size_t)(nl - l) : strlen(l);
                d += sprintf(d, "  %.*s\n", (int)len, l);
                l += len + (nl ? 1 : 0);
            }
            err = MakerApi_CheckEvent("PACKCHECK", label, 0, src);
            if (err && err[0] && bad++ < 40)
                printf("[mp] maker pack: %s: %s\n", label, err);
            free(src);
        }
        printf("[mp] maker pack: %d scripts, %d don't compile\n", n, bad);
    }
    else if (strcmp(cmd, "dbinfo") == 0)
    {
        // dbinfo SPECIES MOVE ITEM - what the game reads for them now (Moemon Maker M3's database edits included)
        int sp = 0, mv = 0, it = 0;
        char a[32], b[32], c[32];
        sscanf(arg, "%d %d %d", &sp, &mv, &it);
        MP_GameToAscii(a, gSpeciesNames[sp], POKEMON_NAME_LENGTH);
        MP_GameToAscii(b, gMoveNames[mv], MOVE_NAME_LENGTH);
        MP_GameToAscii(c, gItems[it].name, ITEM_NAME_LENGTH - 1);
        printf("[test] db species %d %s stats %d %d %d %d %d %d types %d %d learn0 %u\n", sp, a, gSpeciesInfo[sp].baseHP,
               gSpeciesInfo[sp].baseAttack, gSpeciesInfo[sp].baseDefense, gSpeciesInfo[sp].baseSpeed,
               gSpeciesInfo[sp].baseSpAttack, gSpeciesInfo[sp].baseSpDefense, gSpeciesInfo[sp].types[0],
               gSpeciesInfo[sp].types[1], gLevelUpLearnsets[sp][0] & LEVEL_UP_MOVE_ID);
        printf("[test] db move %d %s power %d accuracy %d\n", mv, b, gBattleMoves[mv].power, gBattleMoves[mv].accuracy);
        printf("[test] db item %d %s price %d\n", it, c, gItems[it].price);
    }
    else if (strcmp(cmd, "sealed") == 0)
    {
        // sealed SPECIES - whether it's SEALED here (Moemon Maker M3b: a new POKéMON no pack fills is) and its base
        int sp = 0;
        bool8 MP_Gen_SealedSpecies(u16 species);
        sscanf(arg, "%d", &sp);
        printf("[test] sealed %d %d base %d\n", sp, MP_Gen_SealedSpecies(sp), Maker_SpeciesBase(sp));
    }
    else if (strcmp(cmd, "coins") == 0)
    {
        // coins N - set the COINS; coins - print them
        int n = 0;
        if (sscanf(arg, "%d", &n) == 1)
            SetCoins(n);
        else
            printf("[test] coins %d\n", GetCoins());
    }
    else if (strcmp(cmd, "cuttree") == 0 || strcmp(cmd, "smashrock") == 0)
    {
        // cuttree LOCALID / smashrock LOCALID - as if we used CUT / ROCK SMASH on it (mp_world.c tells friends)
        extern const u8 EventScript_CutTreeDown[];
        gSpecialVar_LastTalked = atoi(arg);
        if (cmd[0] == 'c')
        {
            ScriptContext_SetupScript(EventScript_CutTreeDown);
        }
        else
        {
            RemoveObjectEventByLocalIdAndMap(gSpecialVar_LastTalked, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            MP_World_RockGone();
        }
    }
    else if (strcmp(cmd, "cutgrass") == 0)
    {
        // cutgrass - CUT on the grass around where we face (SetUpFieldMove_Cut picks the tiles, as the menu does)
        bool8 SetUpFieldMove_Cut(void);
        if (SetUpFieldMove_Cut())
        {
            gFieldCallback2 = NULL;
            gPostMenuFieldCallback = NULL;
            FieldEffectStart(FLDEFF_CUT_GRASS);
        }
        else
        {
            printf("[test] cutgrass: nothing to cut here\n");
        }
    }
    else if (strcmp(cmd, "worldobj") == 0)
    {
        // worldobj LOCALID - is that object on this map there, and where
        u8 objId;
        int localId = atoi(arg);
        if (!TryGetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objId))
            printf("[test] worldobj %d at %d,%d\n", localId, gObjectEvents[objId].currentCoords.x - MAP_OFFSET, gObjectEvents[objId].currentCoords.y - MAP_OFFSET);
        else
            printf("[test] worldobj %d gone\n", localId);
    }
    else if (strcmp(cmd, "ashcount") == 0)
    {
        printf("[test] ash %d\n", VarGet(VAR_ASH_GATHER_COUNT));
    }
    else if (strcmp(cmd, "tileat") == 0)
    {
        // tileat X,Y - the metatile there (map coords without the offset)
        int x = 0, y = 0;
        sscanf(arg, "%d,%d", &x, &y);
        printf("[test] tile %d,%d = %d behavior 0x%X\n", x, y, MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET),
               MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET));
    }
    else if (strcmp(cmd, "objid") == 0)
    {
        // objid LOCALID - a map object's tile (map coords without the offset), its sprite's screen place, movement type
        u8 id;
        if (TryGetObjectEventIdByLocalIdAndMap(atoi(arg), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &id))
            printf("[test] objid %d: none\n", atoi(arg));
        else
            printf("[test] objid %d: at %d,%d facing %d type %d sprite %d,%d invisible %d\n", atoi(arg),
                   gObjectEvents[id].currentCoords.x - MAP_OFFSET, gObjectEvents[id].currentCoords.y - MAP_OFFSET,
                   gObjectEvents[id].facingDirection, gObjectEvents[id].movementType,
                   gSprites[gObjectEvents[id].spriteId].x, gSprites[gObjectEvents[id].spriteId].y,
                   gObjectEvents[id].invisible);
    }
    else if (strcmp(cmd, "findmb") == 0)
    {
        // findmb BEHAVIOR - the first tiles of the current map with that behavior (map coords without the offset)
        int b = (int)strtol(arg, NULL, 0), x, y, n = 0;
        for (y = 0; y < gBackupMapLayout.height - 2 * MAP_OFFSET && n < 8; y++)
            for (x = 0; x < gBackupMapLayout.width - 2 * MAP_OFFSET && n < 8; x++)
                if (MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET) == b)
                    printf("[test] findmb 0x%X at %d,%d\n", b, x, y), n++;
        if (n == 0)
            printf("[test] findmb 0x%X: none\n", b);
    }
    else if (strcmp(cmd, "lights") == 0)
    {
        // lights - friends' lights in a dark cave, as last drawn (mp_world.c)
        MP_World_DebugLights();
    }
    else if (strcmp(cmd, "gendersample") == 0)
    {
        // gendersample SPECIES - make 1000 wild-style Pokemon of SPECIES and count the genders
        static struct Pokemon mon;
        int species = atoi(arg), i, f = 0, m = 0, n = 0;
        for (i = 0; i < 1000; i++)
        {
            CreateMon(&mon, species, 5, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
            switch (GetMonGender(&mon))
            {
            case MON_FEMALE: f++; break;
            case MON_MALE: m++; break;
            default: n++; break;
            }
        }
        printf("[test] gendersample %d: female %d male %d none %d (ratio %d)\n", species, f, m, n, gSpeciesInfo[species].genderRatio);
    }
    else if (strcmp(cmd, "challenge") == 0)
    {
        // challenge N - the frontier / tent challenge status (3 = won), e.g. before walking into a tent lobby
        gSaveBlock2Ptr->frontier.challengeStatus = atoi(arg);
        printf("[test] challenge status %d\n", gSaveBlock2Ptr->frontier.challengeStatus);
    }
    else if (strcmp(cmd, "platinumstory") == 0)
    {
        // platinumstory - Platinum's own story begins in this save (src/sinnoh/s3d_scrcmd.c)
        void Sinnoh_PlatinumStoryBegin(void);
        Sinnoh_PlatinumStoryBegin();
    }
    else if (strcmp(cmd, "setflag") == 0)
    {
        // setflag N - an event flag (include/constants/flags.h; 0x.. hex too)
        FlagSet(strtol(arg, NULL, 0));
    }
    else if (strcmp(cmd, "setvar") == 0)
    {
        // setvar ID VALUE - an event var (0x.. hex too)
        char *end;
        long id = strtol(arg, &end, 0);
        VarSet(id, strtol(end, NULL, 0));
    }
    else if (strcmp(cmd, "kaisong") == 0)
    {
        // kaisong - list music/*.kaisong (custom songs) in the log
        MP_Kaisong_Log();
    }
    else if (strcmp(cmd, "playsong") == 0 || strcmp(cmd, "playkai") == 0)
    {
        // playsong N - song N from a clean sound state (no reverb, no voices left), for audio checks against the studio;
        // playkai FILE - the same with a music/ .kaisong file, wherever it is meant to play (tools/kaisong/loudness.py);
        // (either clears musiclock)
        extern u8 gMpTestMusicLock;
        int i;
        gMpTestMusicLock = 0;
        m4aMPlayAllStop();
        for (i = 0; i < MAX_DIRECTSOUND_CHANNELS; i++)
            gSoundInfo.chans[i].statusFlags = 0;
        for (i = 0; i < 4; i++)
            gCgbChans[i].statusFlags = 0;
        m4aSoundMode(SOUND_MODE_REVERB_SET);
        memset(gSoundInfo.pcmBuffer, 0, sizeof(gSoundInfo.pcmBuffer));
        {
            // the Game Boy channels' phase and frame clock carry on from earlier sounds in the game; the studio starts at 0
            extern u32 apuFrame, sampleRate;
            extern u8 apuCycle;
            void cgb_audio_init(u32 rate);
            cgb_audio_init(sampleRate);
            apuFrame = 0;
            apuCycle = 0;
            REG_NR13 = REG_NR14 = REG_NR23 = REG_NR24 = REG_NR33 = REG_NR34 = 0;
        }
        if (cmd[4] == 'k')
            printf("[test] playkai %s %s\n", arg, MP_Kaisong_PlayFile(arg) ? "ok" : "FAILED");
        else
        {
            m4aSongNumStart(atoi(arg));
            printf("[test] playsong %d\n", atoi(arg));
        }
    }
    else if (strcmp(cmd, "musiclock") == 0)
    {
        // musiclock 1 - the song playing keeps the music player: the map's own music changes, stops and fades are
        // ignored (tools/kaisong/loudness.py, so a long measure isn't taken over by the map's song); 0 = off
        extern u8 gMpTestMusicLock;
        gMpTestMusicLock = atoi(arg) != 0;
        printf("[test] musiclock %d\n", gMpTestMusicLock);
    }
    else if (strcmp(cmd, "arcade") == 0)
    {
        // arcade GAME FRAMES - Moemon Kai: open the GAME CENTER's arcade board GAME (include/arcade.h) from the player's
        // set (MOEMON_ARCADE_DIR in tests), run it FRAMES frames (a coin and START along the way) and say what it did;
        // with a FILE.bmp, its last picture there (1.64.0)
        int game = 0, frames = 0, f;
        long pairs = 0;
        static int16_t sAudio[8192];
        char picPath[256] = "";
        sscanf(arg, "%d %d %255s", &game, &frames, picPath);
        if (!Arcade_Open(game))
        {
            printf("[test] arcade %d: no board\n", game);
            return;
        }
        {
            static uint8_t sState[8 << 20];
            u32 t0 = SDL_GetTicks(), t1, t2;
            size_t size = 0;
            for (f = 1; f <= frames; f++)
            {
                Arcade_Frame(f > 600 && f < 620 ? ARCADE_COIN : f > 700 && f < 710 ? ARCADE_START : 0, 0);
                pairs += Arcade_TakeAudio(sAudio, 4096);
            }
            t1 = SDL_GetTicks();
            for (f = 0; f < 60; f++)
                size = Arcade_SaveState(sState, sizeof(sState));
            t2 = SDL_GetTicks();
            for (f = 0; f < 60; f++)
                Arcade_LoadState(sState, size);
            printf("[test] arcade timing: %.2f ms a frame, state %u bytes, save %.2f ms, load %.2f ms\n",
                   frames ? (t1 - t0) / (float)frames : 0.0f, (unsigned)size, (t2 - t1) / 60.0f, (SDL_GetTicks() - t2) / 60.0f);
        }
        {
            const uint32_t *pic = Arcade_Picture();
            uint32_t lit = 0, i;
            int pw, ph;
            Arcade_PictureSize(&pw, &ph);
            for (i = 0; i < (uint32_t)(pw * ph); i++)
                lit += (pic[i] & 0xFFFFFF) != 0;
            printf("[test] arcade %d (%s): %d frames, %ld sound pairs at %d Hz, %u lit pixels, ram hash %08x\n", game,
                   Arcade_Title(game), frames, pairs, Arcade_SampleRate(), lit, Arcade_Hash());
            if (picPath[0])
            {
                // a 24-bit BMP of the picture (0x00BBGGRR), bottom row first
                FILE *bf = fopen(picPath, "wb");
                if (bf)
                {
                    u8 hd[54] = {'B', 'M'};
                    int row = pw * 3, pad = (4 - row % 4) % 4, x, y, k;
                    u32 size = 54 + (u32)(row + pad) * ph;
                    u32 vals[6] = {size, 54, 40, (u32)pw, (u32)ph, 0};
                    int at[6] = {2, 10, 14, 18, 22, 0};
                    for (k = 0; k < 5; k++)
                        hd[at[k]] = vals[k], hd[at[k] + 1] = vals[k] >> 8, hd[at[k] + 2] = vals[k] >> 16, hd[at[k] + 3] = vals[k] >> 24;
                    hd[26] = 1, hd[28] = 24;
                    fwrite(hd, 1, 54, bf);
                    for (y = ph - 1; y >= 0; y--)
                    {
                        for (x = 0; x < pw; x++)
                        {
                            u32 c = pic[y * pw + x];
                            u8 bgr[3] = {(u8)(c >> 16), (u8)(c >> 8), (u8)c};
                            fwrite(bgr, 1, 3, bf);
                        }
                        fwrite("\0\0\0", 1, pad, bf);
                    }
                    fclose(bf);
                    printf("[test] arcade picture %dx%d: %s\n", pw, ph, picPath);
                }
            }
        }
        Arcade_Close();
    }
    else if (strcmp(cmd, "arcadelag") == 0)
    {
        // arcadelag GAME AT BUTTONS: run the board to frame AT (a coin and START along the way), then from there with
        // and without BUTTONS (ARCADE_* bits) held 4 frames: the first frame whose picture differs (input to picture)
        static uint8_t sState[8 << 20];
        static uint32_t sHash[2][40];
        int game = 0, at = 0, buttons = 0, f, run, lag = -1;
        size_t size;
        sscanf(arg, "%d %d %d", &game, &at, &buttons);
        if (!Arcade_Open(game))
            return;
        for (f = 1; f <= at; f++)
            Arcade_Frame(f > 600 && f < 606 ? ARCADE_COIN : f > 640 && f < 646 ? ARCADE_START : 0, 0);
        size = Arcade_SaveState(sState, sizeof(sState));
        for (run = 0; run < 2; run++)
        {
            Arcade_LoadState(sState, size);
            for (f = 0; f < 40; f++)
            {
                const uint32_t *pic;
                uint32_t h = 2166136261u, i;
                int pw, ph;
                Arcade_Frame(run && f < 4 ? buttons : 0, 0);
                pic = Arcade_Picture();
                Arcade_PictureSize(&pw, &ph);
                for (i = 0; i < (uint32_t)(pw * ph); i++)
                    h = (h ^ pic[i]) * 16777619u;
                sHash[run][f] = h;
            }
        }
        for (f = 0; f < 40 && lag < 0; f++)
            if (sHash[0][f] != sHash[1][f])
                lag = f;
        printf("[test] arcadelag %d (%s): pressed on frame 0, the picture first differs on frame %d\n", game,
               Arcade_Title(game), lag);
        Arcade_Close();
    }
    else if (strcmp(cmd, "arcadedet") == 0)
    {
        // arcadedet GAME AT: whether a save state loaded replays exactly: frames AT+1.. run straight on, then again
        // from the state saved at AT (the same buttons); the first frame whose RAM hash differs (-1: none)
        static uint8_t sState[8 << 20];
        static uint32_t sH[2][120];
        int game = 0, at = 0, f, run, bad = -1;
        size_t size;
        sscanf(arg, "%d %d", &game, &at);
        if (!Arcade_Open(game))
            return;
        for (f = 1; f <= at; f++)
            Arcade_Frame(f > 600 && f < 606 ? ARCADE_COIN : f > 640 && f < 646 ? ARCADE_START : 0, 0);
        size = Arcade_SaveState(sState, sizeof(sState));
        for (run = 0; run < 2; run++)
        {
            if (run)
                Arcade_LoadState(sState, size);
            for (f = 0; f < 120; f++)
            {
                Arcade_Frame((f / 7) % 3 == 0 ? ARCADE_RIGHT | ARCADE_LP : (f / 5) % 4 == 1 ? ARCADE_LEFT : 0, 0);
                sH[run][f] = Arcade_Hash();
            }
        }
        for (f = 0; f < 120 && bad < 0; f++)
            if (sH[0][f] != sH[1][f])
                bad = f;
        printf("[test] arcadedet %d (%s): from a loaded state, the first frame that differs: %d\n", game,
               Arcade_Title(game), bad);
        Arcade_Close();
    }
    else if (strcmp(cmd, "bgm") == 0)
    {
        // bgm - which song the music player plays now, and how many tracks / voices it has
        u32 k, busy = 0, cgb = 0;
        for (k = 0; k < MAX_DIRECTSOUND_CHANNELS; k++)
            busy += SOUND_INFO_PTR->chans[k].statusFlags != 0;
        for (k = 0; k < 4; k++)
            cgb += SOUND_INFO_PTR->cgbChans != NULL && SOUND_INFO_PTR->cgbChans[k].statusFlags != 0;
        printf("[test] bgm song %d tracks %d voices %d status %x player tracks %d clock %u busy %u cgb %u\n",
               MP_Kaisong_SongOf(gMPlayInfo_BGM.songHeader),
               gMPlayInfo_BGM.songHeader ? gMPlayInfo_BGM.songHeader->trackCount : 0, SOUND_INFO_PTR->maxChans,
               (unsigned)gMPlayInfo_BGM.status, gMPlayInfo_BGM.trackCount, (unsigned)gMPlayInfo_BGM.clock,
               (unsigned)busy, (unsigned)cgb);
    }
    else if (strcmp(cmd, "dw") == 0)
    {
        void S3D_Dw_Print(void);
        S3D_Dw_Print();   // dw - where the player is in the DISTORTION WORLD (src/sinnoh/s3d_distortion.c)
    }
    else if (strcmp(cmd, "dwput") == 0)
    {
        // dwput X Y Z [DIR] - the player onto that DISTORTION WORLD tile of this floor (its surface found), facing DIR
        // (Platinum's: 0 up, 1 down, 2 left, 3 right)
        void S3D_Dw_Put(s32 x, s32 y, s32 z, u8 dir);
        int x = 0, y = 0, z = 0, d = 1;
        sscanf(arg, "%d %d %d %d", &x, &y, &z, &d);
        S3D_Dw_Put(x, y, z, d);
    }
    else if (strcmp(cmd, "getvar") == 0)
        printf("[test] var 0x%X = %d\n", (unsigned)strtol(arg, NULL, 0), VarGet((u16)strtol(arg, NULL, 0)));   // getvar ID
    else if (strcmp(cmd, "legendko") == 0)
    {
        // legendko FLAG - as if the legendary with that done flag was just beaten (mp_legends.c)
        MP_Legend_Beaten((u16)strtol(arg, NULL, 0));
    }
    else if (strcmp(cmd, "getflag") == 0)
    {
        // getflag N - whether an event flag is set (2151 = FLAG_BADGE01_GET)
        int f = (int)strtol(arg, NULL, 0);   // (decimal or 0x hex, as setflag)
        printf("[test] flag %d = %d\n", f, FlagGet(f) ? 1 : 0);
    }
    else if (strcmp(cmd, "clearflag") == 0)
    {
        // clearflag N - an event flag off (2144 = FLAG_SYS_POKEMON_GET)
        int f = (int)strtol(arg, NULL, 0);   // (decimal or 0x hex, as setflag)
        FlagClear(f);
        printf("[test] clearflag %d\n", f);
    }
    else if (strcmp(cmd, "ticketbackfill") == 0)
    {
        // ticketbackfill [reset] - the once-per-save MOEGATCHA TICKETS for story items already got (reset: as if never run)
        if (strcmp(arg, "reset") == 0)
            MP_SaveData_Set("gatcha:ticketbackfill", 0);
        MP_Gatcha_BackfillTickets();
    }
    else if (strcmp(cmd, "gatchafx") == 0)
    {
        // gatchafx N - the next MoeGatcha pull gives ball effect prize number N (-1: a random one)
        void MP_Gatcha_RigFx(s16 n);
        MP_Gatcha_RigFx(atoi(arg));
    }
    else if (strcmp(cmd, "gatchastock") == 0)
    {
        printf("[test] gatcha tickets %d free %d\n", (int)MP_Gatcha_Tickets(), (int)MP_Gatcha_FreeWaiting());
    }
    else if (strcmp(cmd, "gatcharig") == 0)
    {
        // gatcharig N - the next MoeGatcha pull is this rarity (0 N, 1 U, 2 R, 3 SR, 4 SSR, 5 UR, 6 EX); gatcharig N P also gives P pulls
        void MP_Gatcha_Rig(s8 rarity);
        void MP_Gatcha_AddPulls(s32 n);
        int r = 0, p = 0;
        sscanf(arg, "%d %d", &r, &p);
        MP_Gatcha_Rig(r);
        if (p)
            MP_Gatcha_AddPulls(p);
    }
    else if (strcmp(cmd, "sprites") == 0)
    {
        // sprites - every sprite in use: position, size, tile and palette
        int i;
        for (i = 0; i < MAX_SPRITES; i++)
        {
            struct Sprite *sp = &gSprites[i];
            if (!sp->inUse)
                continue;
            printf("[test] spr %2d x %d y %d x2 %d y2 %d shape %d size %d tile %d pal %d tag %04x ptag %04x inv %d prio %d sub %d cb %p\n", i, sp->x, sp->y, sp->x2, sp->y2,
                   sp->oam.shape, sp->oam.size, sp->oam.tileNum, sp->oam.paletteNum, sp->template ? sp->template->tileTag : 0,
                   sp->template ? sp->template->paletteTag : 0, sp->invisible, sp->oam.priority, sp->subpriority, (void *)sp->callback);
        }
    }
    else if (strcmp(cmd, "objtiles") == 0)
    {
        // objtiles TILE WIDTH_TILES HEIGHT_TILES - print sprite tiles from VRAM as hex digits
        int t = 0, w = 4, h = 4, x, y;
        sscanf(arg, "%d %d %d", &t, &w, &h);
        for (y = 0; y < h * 8; y++)
        {
            char line[80];
            for (x = 0; x < w * 8 && x < 64; x++)
            {
                const u8 *tile = (const u8 *)(OBJ_VRAM0) + (t + (y / 8) * w + x / 8) * 32;
                u8 c = (tile[(y % 8) * 4 + (x % 8) / 2] >> ((x & 1) * 4)) & 15;
                line[x] = "0123456789ABCDEF"[c];
            }
            line[x] = 0;
            printf("[test] %s\n", line);
        }
    }
    else if (strcmp(cmd, "palvacate") == 0)
    {
        // palvacate SLOT - the game writes an NPC reflection palette there
        void MP_VacatePaletteSlot(u8 slot);
        int n = 0;
        sscanf(arg, "%d", &n);
        MP_VacatePaletteSlot(n);
    }
    else if (strcmp(cmd, "palcoop") == 0)
    {
        unsigned t = 0;
        sscanf(arg, "%x", &t);
        printf("[test] palcoop %04x -> %d\n", t, MP_AllocCoopPalette(t, NULL));
    }
        else if (strcmp(cmd, "palfree") == 0)
    {
        unsigned t = 0;
        sscanf(arg, "%x", &t);
        FreeSpritePaletteByTag(t);
    }
        else if (strcmp(cmd, "palfill") == 0)
    {
        // palfill N - the game grabs N more sprite palettes (0 = give them back)
        static const u16 sJunk[16] = {0};
        int n = 0, i;
        sscanf(arg, "%d", &n);
        if (n == 0)
            for (i = 0; i < 8; i++)
                FreeSpritePaletteByTag(0x7700 + i);
        for (i = 0; i < n; i++)
        {
            struct SpritePalette pal = {sJunk, 0x7700 + i};
            printf("[test] palfill %04x -> %d\n", 0x7700 + i, LoadSpritePalette(&pal));
        }
    }
        else if (strcmp(cmd, "crash") == 0)
    {
        // crash - test crash_log.txt
        volatile int *p = NULL;
        *p = 1;
    }
        else if (strcmp(cmd, "dumpvram") == 0)
    {
        // dumpvram FILE - the BG / OBJ video memory (binary, VRAM_SIZE bytes)
        FILE *f = fopen(arg, "wb");
        if (f)
        {
            fwrite((void *)VRAM, 1, VRAM_SIZE, f);
            fclose(f);
        }
        printf("[test] vram dumped\n");
    }
        else if (strcmp(cmd, "dumppltt") == 0)
    {
        // dumppltt FILE - palette RAM, both palette buffers and the display registers (binary); a relative FILE
        // goes in the test's picture folder (MOEMON_SHOTDIR)
        char path[512];
        const char *dir = getenv("MOEMON_SHOTDIR");
        FILE *f;
        snprintf(path, sizeof(path), "%s%s%s", arg[0] != '/' && dir ? dir : "", arg[0] != '/' && dir ? "/" : "", arg);
        f = fopen(path, "wb");
        if (f)
        {
            fwrite((void *)PLTT, 1, PLTT_SIZE, f);
            fwrite(gPlttBufferUnfaded, 1, PLTT_SIZE, f);
            fwrite(gPlttBufferFaded, 1, PLTT_SIZE, f);
            fwrite((void *)REG_BASE, 1, 0x60, f);
            fclose(f);
        }
        printf("[test] pltt dumped: fade active %d y %d blend %04x sel %08x transfer off %d\n", gPaletteFade.active,
               gPaletteFade.y, gPaletteFade.blendColor, gPaletteFade.multipurpose1, gPaletteFade.bufferTransferDisabled);
    }
        else if (strcmp(cmd, "pals") == 0)
    {
        // pals - which tag holds each sprite palette slot
        extern u8 gReservedSpritePaletteCount;
        u16 GetSpritePaletteTagByPaletteNum(u8 paletteNum);
        int i;
        printf("[test] pals reserved=%d:", gReservedSpritePaletteCount);
        for (i = 0; i < OBJ_PAL_SLOTS; i++)
            if (i < 16 || GetSpritePaletteTagByPaletteNum(i) != 0xFFFF)
                printf(" %d=%04x", i, GetSpritePaletteTagByPaletteNum(i));
        printf("\n");
    }
        else if (strcmp(cmd, "follow") == 0)
    {
        // follow N - party Pokémon N walks with you (like SELECT in the party menu)
        int n = 0;
        sscanf(arg, "%d", &n);
        printf("[test] follow %d -> %d\n", n, MP_Follower_Toggle(&gPlayerParty[n % PARTY_SIZE]));
    }
    else if (strcmp(cmd, "emote") == 0)
    {
        // emote N - like the number keys (0 !, 1 ?, 2 heart, 3 note, 4-7 phrases)
        int n = 0;
        sscanf(arg, "%d", &n);
        MP_Emote_Hotkey(n);
    }
    else if (strcmp(cmd, "emotekind") == 0)
    {
        // emotekind K - any MP_EMOTE_* (5-16: the extra emotes)
        int k = 0;
        sscanf(arg, "%d", &k);
        printf("[test] emote %d %s\n", k, MP_Emote_Name(k));
        MP_Emote_Send(k);
    }
    else if (strcmp(cmd, "battleicon") == 0)
    {
        // battleicon K - a battle bubble with emote K (5-16) over our Moemon
        int k = 0;
        sscanf(arg, "%d", &k);
        if (k >= MP_EMOTE_FIRST_EXTRA && k < MP_EMOTE_COUNT)
            MP_Emote_BattleSay(NULL, MP_BUBBLE_ICON_EXTRA + k - MP_EMOTE_FIRST_EXTRA);
    }
    else if (strcmp(cmd, "quick") == 0)
    {
        // quick PAGE - the emote / phrase quick pick, open (EMOTES 0-3, MINE 4-11, stock 12-15)
        int page = 0;
        sscanf(arg, "%d", &page);
        MP_OpenQuickPickAt(page, -1, -1);
    }
    else if (strcmp(cmd, "social") == 0)
    {
        MP_Social_Open();
    }
    else if (strcmp(cmd, "bhp") == 0)
    {
        int b;
        for (b = 0; b < gBattlersCount; b++)
            printf("[test] battler %d hp %d/%d species %d level %d (party %d)\n", b, gBattleMons[b].hp, gBattleMons[b].maxHP,
                   gBattleMons[b].species, gBattleMons[b].level,
                   GetMonData(GetBattlerSide(b) == B_SIDE_PLAYER ? &gPlayerParty[gBattlerPartyIndexes[b]] : &gEnemyParty[gBattlerPartyIndexes[b]], MON_DATA_LEVEL));
    }
    else if (strcmp(cmd, "bstatus") == 0)
    {
        // bstatus BATTLER STATUS1 [HP] - sets a battler's status (and HP) and redraws its healthbox / card
        int b = 0, st = 0, hp = -1;
        sscanf(arg, "%d %d %d", &b, &st, &hp);
        if (b >= 0 && b < gBattlersCount)
        {
            struct Pokemon *mon = GetBattlerSide(b) == B_SIDE_PLAYER ? &gPlayerParty[gBattlerPartyIndexes[b]] : &gEnemyParty[gBattlerPartyIndexes[b]];
            u32 status = st;
            SetMonData(mon, MON_DATA_STATUS, &status);
            gBattleMons[b].status1 = status;
            if (hp >= 0)
            {
                u16 h = hp;
                SetMonData(mon, MON_DATA_HP, &h);
                gBattleMons[b].hp = h;
            }
            UpdateHealthboxAttribute(gHealthboxSpriteIds[b], mon, HEALTHBOX_ALL);
        }
    }
    else if (strcmp(cmd, "tourney") == 0)
    {
        // tourney - CO-OP > TOURNAMENT
        MP_Tourney_OpenMenu();
    }
    else if (strcmp(cmd, "vsseeker") == 0)
    {
        // vsseeker [yes]: FireRed's VS SEEKER battery and ready trainers (yes: every rematch is wanted, no 30% refusal)
        void MP_VsSeeker_Debug(bool8 alwaysYes);
        MP_VsSeeker_Debug(arg != NULL && strcmp(arg, "yes") == 0);
    }
    else if (strcmp(cmd, "ttower") == 0)
    {
        // ttower: KANTO's TRAINER TOWER state (src/mp/mp_trainer_tower.c): challenge, floors cleared, clock, best times
        void MP_TrainerTower_Log(void);
        MP_TrainerTower_Log();
    }
    else if (strcmp(cmd, "jtower") == 0)
    {
        // jtower: JOHTO's BATTLE TOWER state (src/mp/mp_johto_tower.c)
        void MP_JohtoTower_Log(void);
        MP_JohtoTower_Log();
    }
    else if (strcmp(cmd, "register") == 0)
    {
        // mp register ITEM: adds it and registers it to SELECT
        u16 item = atoi(arg);
        AddBagItem(item, 1);
        gSaveBlock1Ptr->registeredItem = item;
    }
    else if (strcmp(cmd, "fam") == 0)
    {
        int Gamepad_IconFamily(void);
        printf("[test] icon family %d\n", Gamepad_IconFamily());
    }
    else if (strcmp(cmd, "near") == 0)
    {
        int i;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            if (gObjectEvents[i].active)
            {
                struct Sprite *sp = &gSprites[gObjectEvents[i].spriteId];
                u16 pal = 256 + sp->oam.paletteNum * 16;
                printf("[test] obj %d gfx %d at %d,%d facing %d local %d pal %d (tag %04x: %04x %04x %04x, faded %04x %04x)\n", i,
                       OBJ_GFX_ID(&gObjectEvents[i]), gObjectEvents[i].currentCoords.x, gObjectEvents[i].currentCoords.y,
                       gObjectEvents[i].facingDirection, gObjectEvents[i].localId, sp->oam.paletteNum,
                       GetSpritePaletteTagByPaletteNum(sp->oam.paletteNum), gPlttBufferUnfaded[pal + 1], gPlttBufferUnfaded[pal + 2],
                       gPlttBufferUnfaded[pal + 3], gPlttBufferFaded[pal + 1], gPlttBufferFaded[pal + 2]);
            }
    }
    else if (strcmp(cmd, "lend") == 0)
        MP_Lend_Debug();
    else if (strcmp(cmd, "pp") == 0)
    {
        // pp SLOT MOVEINDEX VALUE - a party Moemon's PP for one move; prints all four back
        int slot = 0, m = 0, v = 0, i;
        if (sscanf(arg, "%d %d %d", &slot, &m, &v) == 3 && slot >= 0 && slot < PARTY_SIZE && m >= 0 && m < MAX_MON_MOVES)
        {
            u8 pp = v;
            SetMonData(&gPlayerParty[slot], MON_DATA_PP1 + m, &pp);
        }
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            printf("[test] pp slot %d:", slot);
            for (i = 0; i < MAX_MON_MOVES; i++)
                printf(" %u", (unsigned)GetMonData(&gPlayerParty[slot], MON_DATA_PP1 + i));
            printf("\n");
        }
    }
    else if (strcmp(cmd, "teach") == 0)
    {
        // mp teach SLOT MOVE [MOVESLOT]: replaces the first move (or move MOVESLOT 0-3)
        int slot = atoi(arg), move = 0, which = 0;
        const char *sp = strchr(arg, ' ');
        if (sp)
            sscanf(sp + 1, "%d %d", &move, &which);
        if (slot >= 0 && slot < PARTY_SIZE && move > 0 && move < MOVES_COUNT && which >= 0 && which < MAX_MON_MOVES)
        {
            u16 m = move;
            u8 pp = gBattleMoves[move].pp;
            SetMonData(&gPlayerParty[slot], MON_DATA_MOVE1 + which, &m);
            SetMonData(&gPlayerParty[slot], MON_DATA_PP1 + which, &pp);
            printf("[test] teach: slot %d move %d\n", slot, move);
        }
    }
    else if (strcmp(cmd, "bmove") == 0)
    {
        // mp bmove MOVE: in a battle, every one of the player's battlers' first move (tests with a sure hit: 129 SWIFT)
        int move = atoi(arg), b;
        if (gMain.inBattle && move > 0 && move < MOVES_COUNT)
            for (b = 0; b < gBattlersCount; b++)
                if (GetBattlerSide(b) == B_SIDE_PLAYER)
                {
                    gBattleMons[b].moves[0] = move;
                    gBattleMons[b].pp[0] = gBattleMoves[move].pp;
                }
        printf("[test] bmove: %d\n", move);
    }
    else if (strcmp(cmd, "setmove") == 0)
    {
        // mp setmove SLOT INDEX MOVE: sets one of a party Pokémon's four moves (INDEX 0-3)
        int slot = 0, idx = 0, move = 0;
        if (sscanf(arg, "%d %d %d", &slot, &idx, &move) == 3 && slot >= 0 && slot < PARTY_SIZE && idx >= 0 && idx < MAX_MON_MOVES
         && move >= 0 && move < MOVES_COUNT)
        {
            u16 m = move;
            SetMonData(&gPlayerParty[slot], MON_DATA_MOVE1 + idx, &m);
            printf("[test] setmove: slot %d move %d = %d\n", slot, idx, move);
        }
    }
    else if (strcmp(cmd, "hold") == 0)
    {
        // mp hold SLOT ITEM: gives a party Pokémon a held item
        int slot = 0, item = 0;
        if (sscanf(arg, "%d %d", &slot, &item) == 2 && slot >= 0 && slot < PARTY_SIZE && item >= 0 && item < ITEMS_COUNT)
        {
            u16 it = item;
            SetMonData(&gPlayerParty[slot], MON_DATA_HELD_ITEM, &it);
            printf("[test] hold: slot %d item %d\n", slot, item);
        }
    }
    else if (strcmp(cmd, "setmon") == 0)
    {
        // setmon SLOT SPECIES LEVEL [ITEM] - a new party Moemon of that species in SLOT, holding ITEM (tests that go
        // through every species)
        int slot = 0, species = 1, level = 50, item = 0;
        sscanf(arg, "%d %d %d %d", &slot, &species, &level, &item);
        if (slot >= 0 && slot < PARTY_SIZE && species > 0 && species < NUM_SPECIES)
        {
            u16 held = item;
            CreateMon(&gPlayerParty[slot], species, level, 32, 0, 0, OT_ID_PLAYER_ID, 0);
            SetMonData(&gPlayerParty[slot], MON_DATA_HELD_ITEM, &held);
            CalculatePlayerPartyCount();
        }
    }
    else if (strcmp(cmd, "species") == 0)
    {
        // mp species SLOT - logs a party Pokémon's species and held item
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
            printf("[test] species: slot %d = %d item %d\n", slot, (int)GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_HELD_ITEM));
    }
    else if (strcmp(cmd, "moves") == 0)
    {
        // mp moves SLOT M1 [M2 M3 M4] - the party Moemon in SLOT knows these moves (full PP); missing ones are emptied
        int slot = 0, m[MAX_MON_MOVES] = {0}, n, i;
        n = sscanf(arg, "%d %d %d %d %d", &slot, &m[0], &m[1], &m[2], &m[3]);
        if (n >= 2 && slot >= 0 && slot < PARTY_SIZE)
        {
            for (i = 0; i < MAX_MON_MOVES; i++)
                SetMonMoveSlot(&gPlayerParty[slot], m[i], i);
            printf("[test] moves: slot %d = %d %d %d %d\n", slot, m[0], m[1], m[2], m[3]);
        }
        else if (n == 1 && slot >= 0 && slot < PARTY_SIZE)   // (no moves: the ones it knows)
            printf("[test] knows: slot %d = %d %d %d %d\n", slot, (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE1),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE2), (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE3),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE4));
    }
    else if (strcmp(cmd, "battlers") == 0)
    {
        // battlers - each battler's species and HP in the running battle
        int b;
        printf("[test] battlers:");
        for (b = 0; gMain.inBattle && b < gBattlersCount; b++)
            printf(" %d=%d:%d/%d", b, gBattleMons[b].species, gBattleMons[b].hp, gBattleMons[b].maxHP);
        printf("\n");
    }
    else if (strcmp(cmd, "battlerhp") == 0)
    {
        // battlerhp BATTLER HP - a battler's HP in the running battle (and its party slot's): 0 = it faints at the turn's
        // end (the game running the battle: the co-op master)
        int b = 0, v = 0;
        sscanf(arg, "%d %d", &b, &v);
        if (gMain.inBattle && b >= 0 && b < gBattlersCount)
        {
            u16 hp = v;
            gBattleMons[b].hp = hp;
            SetMonData(GetBattlerSide(b) == B_SIDE_PLAYER ? &gPlayerParty[gBattlerPartyIndexes[b]]
                                                          : &gEnemyParty[gBattlerPartyIndexes[b]], MON_DATA_HP, &hp);
            printf("[test] battlerhp: battler %d = %d\n", b, v);
        }
    }
    else if (strcmp(cmd, "foeparty") == 0)
    {
        // foeparty - the foes' team in the running battle (species:level each)
        int i;
        printf("[test] foeparty:");
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES))
                printf(" %u:%u", (unsigned)GetMonData(&gEnemyParty[i], MON_DATA_SPECIES),
                       (unsigned)GetMonData(&gEnemyParty[i], MON_DATA_LEVEL));
        printf("\n");
    }
    else if (strcmp(cmd, "announcer") == 0)
    {
        // announcer: POKeMON STADIUM's announcer, what it learned of the battle's pace (mp_stadium_announcer.c);
        // announcer say N: clip N said now, as a screen's line would be
        void MP_StadiumAnnouncer_Debug(void);
        void MP_StadiumAnnouncer_Screen(u16 clip, const char *why);
        int n;
        void MP_StadiumAnnouncer_SayAgain(void);
        if (sscanf(arg, "say %d", &n) == 1)
            MP_StadiumAnnouncer_Screen((u16)n, "the debug command");
        else if (strncmp(arg, "again", 5) == 0)
            MP_StadiumAnnouncer_SayAgain();   // (announcer again: the last line asked for once more, with nothing new)
        else
            MP_StadiumAnnouncer_Debug();
    }
    else if (strcmp(cmd, "pinhp") == 0)
    {
        // pinhp FOE MINE - every frame of a battle, the foes out at FOE HP at most and ours at MINE at least (their max
        // HP raised to it if need be; 0 0: off), for tests
        // whose battles must end one way whatever the moves do
        sscanf(arg, "%d %d", &sPinFoeHp, &sPinMyHp);
        printf("[test] pinhp: foes %d, ours %d\n", sPinFoeHp, sPinMyHp);
    }
    else if (strcmp(cmd, "foehp") == 0)
    {
        // foehp HP - every foe's HP in the running battle (its party and the ones out), for tests of long battles
        int v = atoi(arg), i;
        u16 hp = v;
        if (gMain.inBattle)
        {
            for (i = 0; i < PARTY_SIZE; i++)
                if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) && GetMonData(&gEnemyParty[i], MON_DATA_HP))
                    SetMonData(&gEnemyParty[i], MON_DATA_HP, &hp);
            for (i = 0; i < gBattlersCount; i++)
                if (GetBattlerSide(i) == B_SIDE_OPPONENT && gBattleMons[i].hp)
                {
                    gBattleMons[i].hp = hp;
                    UpdateHealthboxAttribute(gHealthboxSpriteIds[i], &gEnemyParty[gBattlerPartyIndexes[i]], HEALTHBOX_HEALTH_BAR);
                }
            printf("[test] foehp: %d\n", v);
        }
    }
    else if (strcmp(cmd, "status") == 0)
    {
        // mp status SLOT VALUE: the status word (8 poison, 16 burn, 32 freeze, 64 paralysis, 1-7 sleep turns, 128 bad poison)
        int slot = atoi(arg);
        u32 v = 0;
        const char *sp = strchr(arg, ' ');
        if (sp)
            v = (u32)strtoul(sp + 1, NULL, 0);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            SetMonData(&gPlayerParty[slot], MON_DATA_STATUS, &v);
            printf("[test] status: slot %d = %u\n", slot, (unsigned)v);
        }
    }
    else if (strcmp(cmd, "hp") == 0)
    {
        // mp hp SLOT VALUE (0: it fainted now, for REVIVE's cursor); mp partyhp: every slot's HP and status
        int slot = atoi(arg), v = 0;
        const char *sp = strchr(arg, ' ');
        if (sp)
            v = atoi(sp + 1);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u16 hp = v;
            SetMonData(&gPlayerParty[slot], MON_DATA_HP, &hp);
            if (hp == 0)   // (a faint, as far as REVIVE's cursor knows)
                MP_NoteFaint(GetMonData(&gPlayerParty[slot], MON_DATA_PERSONALITY));
            if (gMain.inBattle)
            {
                int b;
                for (b = 0; b < gBattlersCount; b++)
                    if (GetBattlerSide(b) == B_SIDE_PLAYER && gBattlerPartyIndexes[b] == slot && gBattleMons[b].hp)
                    {
                        gBattleMons[b].hp = hp;
                        UpdateHealthboxAttribute(gHealthboxSpriteIds[b], &gPlayerParty[slot], HEALTHBOX_HEALTH_BAR);
                    }
            }
            printf("[test] hp: slot %d = %d\n", slot, v);
        }
    }
    else if (strcmp(cmd, "partyhp") == 0)
    {
        int i;
        for (i = 0; i < PARTY_SIZE && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES); i++)
            printf("[test] partyhp %d: hp %u status %u\n", i, (unsigned)GetMonData(&gPlayerParty[i], MON_DATA_HP),
                   (unsigned)GetMonData(&gPlayerParty[i], MON_DATA_STATUS));
    }
    else if (strcmp(cmd, "givebadges") == 0)
    {
        // givebadges [REGION] - all 8 badges (HOENN's, or region 1 JOHTO / 2 KANTO's)
        int b, region = *arg ? atoi(arg) : 0;
        void MP_HmRescue_GiveAll(void);
        for (b = 0; b < 8; b++)
            FlagSet(region ? MP_Region_BadgeFlag(region, b) : FLAG_BADGE01_GET + b);
        if (!region)
            MP_HmRescue_GiveAll();   // (the HMs a player with them has had: no HM rescue in tests that skip the story)
    }
    else if (strcmp(cmd, "photo") == 0)
    {
        const char *why = MP_Photo_Check();
        if (why)
            printf("[test] photo: %s\n", why);
        else
            MP_Photo_Start();
    }
    else if (strcmp(cmd, "options") == 0)
    {
        // options - the OPTIONS screen (back to the field)
        void CB2_InitOptionMenu(void);
        void CB2_ReturnToField(void);
        gMain.savedCallback = CB2_ReturnToField;
        SetMainCallback2(CB2_InitOptionMenu);
    }
    else if (strcmp(cmd, "creatures") == 0 || strcmp(cmd, "namecase") == 0)
    {
        // creatures 0|1 (MOEMON / POKEMON pictures and word), namecase 0|1|2 (as typed / ALL CAPS / Mixed Case)
        if (cmd[0] == 'c')
            gSaveBlock2Ptr->optionsPokemonArt = atoi(arg) != 0;
        else
            gSaveBlock2Ptr->optionsNameCase = atoi(arg) % MP_NAMECASE_COUNT;
        printf("[test] creatures %d namecase %d\n", gSaveBlock2Ptr->optionsPokemonArt, gSaveBlock2Ptr->optionsNameCase);
    }
    else if (strcmp(cmd, "streamer") == 0)
    {
        // streamer N - OPTIONS > CO-OP > STREAMER MODE (0 off, 1 game, 2 anime, 3 original)
        gMpConfig.streamer = atoi(arg) % MP_STREAMER_COUNT;
        printf("[test] streamer %d\n", gMpConfig.streamer);
    }
    else if (strcmp(cmd, "namefilter") == 0)
    {
        // namefilter TEXT - log TEXT as OPTIONS > NAME CASE / CREATURES would show it (~ is é)
        static u8 in[200];
        const u8 *f;
        int i;
        MP_AsciiToGame(in, arg, sizeof(in) - 1);
        for (i = 0; arg[i] && i < (int)sizeof(in) - 1; i++)
            if (arg[i] == '~')
                in[i] = CHAR_e_ACUTE;
        f = MP_TextOpts_Filter(in, 0);
        printf("[test] namefilter <");
        for (i = 0; f[i] != EOS; i++)
        {
            u8 one[2] = {f[i], EOS};
            char c[8];
            MP_GameToAscii(c, one, sizeof(c) - 1);
            printf("%s", f[i] == CHAR_e_ACUTE ? "~" : c);
        }
        printf(">\n");
    }
    else if (strcmp(cmd, "legacyrand") == 0)
    {
        // legacyrand B,B,... - the random bytes the next GENESIS / EPOCH BALL throws use
        u8 bytes[16];
        int n = 0, v, used;
        const char *p = arg;
        while (n < 16 && sscanf(p, "%d%n", &v, &used) == 1)
        {
            bytes[n++] = (u8)v;
            p += used;
            if (*p == ',')
                p++;
        }
        MP_LegacyBall_RigRandom(bytes, n);
    }
    else if (strcmp(cmd, "typeicons") == 0)
    {
        // typeicons [0|1] - OPTIONS > TYPE ICONS
        gSaveBlock2Ptr->optionsTypeIcons = arg[0] ? atoi(arg) != 0 : 1;
    }
    else if (strcmp(cmd, "safari") == 0)
    {
        // safari [info] - Safari Zone mode on (balls and steps), so the next wild battle is a Safari battle; info: the
        // SAFARI GAME's state
        void EnterSafariMode(void);
        u16 SafariZone_StepsLeft(void);
        extern u8 gNumSafariBalls;
        if (strncmp(arg, "info", 4) == 0)
            printf("[test] safari: mode %d balls %d steps %d\n", FlagGet(FLAG_SYS_SAFARI_MODE), gNumSafariBalls,
                   SafariZone_StepsLeft());
        else
            EnterSafariMode();
    }
    else if (strcmp(cmd, "bmons") == 0)
    {
        // bmons - every battler's species and moves as the battle has them (not the party's)
        int b;
        for (b = 0; b < gBattlersCount; b++)
            printf("[test] bmon %d: species %u moves %u %u %u %u\n", b, gBattleMons[b].species, gBattleMons[b].moves[0],
                   gBattleMons[b].moves[1], gBattleMons[b].moves[2], gBattleMons[b].moves[3]);
    }
    else if (strcmp(cmd, "wilddouble") == 0)
    {
        extern u8 gMpWildDoublePercent;
        gMpWildDoublePercent = arg[0] ? atoi(arg) : 100;
    }
    else if (strcmp(cmd, "bagorder") == 0)
    {
        // bagorder POCKET - the item numbers in a pocket's order (a * after a new one)
        int p = 0;
        u16 i;
        sscanf(arg, "%d", &p);
        if (p >= 0 && p < POCKETS_COUNT)
        {
            printf("[test] bagorder %d:", p);
            for (i = 0; i < gBagPockets[p].capacity && gBagPockets[p].itemSlots[i].itemId != ITEM_NONE; i++)
                printf(" %d%s", gBagPockets[p].itemSlots[i].itemId, MP_Bag_IsNew(gBagPockets[p].itemSlots[i].itemId) ? "*" : "");
            printf("\n");
        }
    }
    else if (strcmp(cmd, "itemdesc") == 0)
    {
        // itemdesc - how the last item description was laid out (font, lines, paging)
        void MP_ItemDesc_Debug(void);
        MP_ItemDesc_Debug();
    }
    else if (strcmp(cmd, "bagfav") == 0)
    {
        // bagfav ITEM [0] - marks an item FAVORITE (0: takes it out), as its menu in the BAG does
        int item = 0, on = 1;
        sscanf(arg, "%d %d", &item, &on);
        MP_Bag_SetFavorite(item, on != 0);
    }
    else if (strcmp(cmd, "bagview") == 0)
    {
        // bagview VIEW - the item numbers one of the BAG's eight pockets shows (MP_BAGVIEW_*), in its order
        static struct ItemSlot *slots[MP_BAG_ITEMS_COUNT + MP_BAG_KEYITEMS_COUNT + MP_BAG_POKEBALLS_COUNT
                                      + MP_BAG_TMHM_COUNT + MP_BAG_BERRIES_COUNT];
        int v = 0;
        u16 i, n;
        sscanf(arg, "%d", &v);
        if (v >= 0 && v < MP_BAGVIEW_COUNT)
        {
            n = MP_Bag_ViewSlots(v, slots, ARRAY_COUNT(slots));
            printf("[test] bagview %d:", v);
            for (i = 0; i < n; i++)
                printf(" %d", slots[i]->itemId);
            printf("\n");
        }
    }
    else if (strcmp(cmd, "bag") == 0)
    {
        // bag - how many kinds of item each pocket holds, and the biggest stack
        u8 p;
        for (p = 0; p < POCKETS_COUNT; p++)
        {
            u16 i, kinds = 0, biggest = 0;
            for (i = 0; i < gBagPockets[p].capacity; i++)
                if (gBagPockets[p].itemSlots[i].itemId != ITEM_NONE)
                {
                    u16 q = gBagPockets[p].itemSlots[i].quantity ^ (u16)gSaveBlock2Ptr->encryptionKey;
                    kinds++;
                    if (q > biggest)
                        biggest = q;
                }
            printf("[test] bag pocket %d: %d kinds (of %d), biggest stack %d\n", p, kinds, gBagPockets[p].capacity, biggest);
        }
    }
    else if (strcmp(cmd, "savesum") == 0)
    {
        // savesum: a summary of the loaded save (tools/test/save_compat.sh compares older formats' with today's)
        u32 h = 2166136261u, i;
        const u8 *p = (const u8 *)gSaveBlock1Ptr;
        for (i = 0; i < sizeof(struct SaveBlock1); i++)
        {
            if (i >= offsetof(struct SaveBlock1, objectEvents) && i < offsetof(struct SaveBlock1, objectEvents) + sizeof(gSaveBlock1Ptr->objectEvents))
                continue;   // (the object events: below, field by field)
            h = (h ^ p[i]) * 16777619u;
        }
        printf("[test] savesum: map %d.%d at %d,%d money %u badges", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum,
               gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y, (unsigned)GetMoney(&gSaveBlock1Ptr->money));
        for (i = 0; i < 8; i++)
            printf("%d", FlagGet(FLAG_BADGE01_GET + i));
        printf(" sb1 %08x objs", h);
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            if (gSaveBlock1Ptr->objectEvents[i].active)
                printf(" %d:%x@%d,%d", gSaveBlock1Ptr->objectEvents[i].localId, OBJ_GFX_ID(&gSaveBlock1Ptr->objectEvents[i]),
                       gSaveBlock1Ptr->objectEvents[i].currentCoords.x, gSaveBlock1Ptr->objectEvents[i].currentCoords.y);
        printf(" tmpl");
        for (i = 0; i < 8; i++)
            printf(" %x/%d", TEMPLATE_GFX_ID(&gSaveBlock1Ptr->objectEventTemplates[i]), gSaveBlock1Ptr->objectEventTemplates[i].kind);
        printf("\n");
    }
    else if (strcmp(cmd, "save") == 0)
    {
        u8 TrySavingData(u8 saveType);
        TrySavingData(SAVE_NORMAL);
    }
    else if (strcmp(cmd, "fishbite") == 0)
    {
        // fishbite N - the next casts: 1 always a bite, 2 never, 0 as the dice fall
        extern u8 gMpFishBiteRig;
        gMpFishBiteRig = atoi(arg);
    }
    else if (strcmp(cmd, "fish") == 0)
    {
        // fish ROD - cast a rod (0 old, 1 good, 2 super) where you stand
        StartFishing(atoi(arg));
    }
    else if (strcmp(cmd, "avatarstate") == 0)
    {
        // avatarstate N - the player's sprite for PLAYER_AVATAR_STATE_* N (5 field move, 6 fishing, 7 watering...)
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        ObjectEventSetGraphicsId(player, GetPlayerAvatarGraphicsIdByStateId(atoi(arg)));
        StartSpriteAnim(&gSprites[player->spriteId], 0);
        printf("[test] avatar state %d\n", atoi(arg));
    }
    else if (strcmp(cmd, "created") == 0)
    {
        // created HEX - CUSTOM 1 = that recipe (the ini's custom1 = HEX) and wear it
        if (MP_Created_FromHex(arg, &gMpConfig.custom[0]) && MP_Avatar_ParseSpriteName("custom_1", &gMpConfig.app))
        {
            gMpConfig.app.recipe = gMpConfig.custom[0];
            gMpConfig.app.hasCustom = FALSE;
            gMpConfig.customSprite[0] = 0;
            MP_Avatar_ApplyLocal();
            MP_SendAppearance();
            printf("[test] created %s\n", arg);
        }
        else
            printf("[test] created %s: bad recipe\n", arg);
    }
    else if (strcmp(cmd, "orrefoe") == 0)
    {
        // orrefoe PERSON [1] - the next battle's foe A (or B) shows ORRE person PERSON's picture (in ORRE)
        int person = 0, which = 0;
        sscanf(arg, "%d %d", &person, &which);
        Orre_People_SetFoe(which, person);
        printf("[test] orrefoe %d %d (%s)\n", person, which, Orre_People_Model(person));
    }
    else if (strcmp(cmd, "orrenpc") == 0)
    {
        // orrenpc PERSON [DX DY] - ORRE person PERSON stands beside the player (their LOOK field sprite)
        int person = 0, dx = 1, dy = 0;
        s16 x, y;
        u8 id;
        sscanf(arg, "%d %d %d", &person, &dx, &dy);
        PlayerGetDestCoords(&x, &y);
        id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_ORRE_FIRST + person, MOVEMENT_TYPE_FACE_DOWN,
                                                  0xE0 + (person & 0xF), x + dx, y + dy, 3);
        printf("[test] orrenpc %d %s object %d\n", person, Orre_People_Model(person), id);
    }
    else if (strcmp(cmd, "objs") == 0)
    {
        // objs - the active object events: id, local id, tile, elevation, graphics, unseen
        u32 i;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            if (gObjectEvents[i].active)
                printf("[test] obj %u local %u at %d,%d elev %u gfx %u%s\n", i, gObjectEvents[i].localId,
                       gObjectEvents[i].currentCoords.x, gObjectEvents[i].currentCoords.y,
                       gObjectEvents[i].currentElevation, OBJ_GFX_ID(&gObjectEvents[i]),
                       gObjectEvents[i].invisible ? " unseen" : "");
    }
    else if (strcmp(cmd, "createddump") == 0)
    {
        // createddump HEX DIR - a CREATE recipe's field frames, front and back pictures as PNGs in DIR
        struct MpCreated r;
        char hex[64] = "", dir[400] = ".";
        void MP_Created_Dump(const struct MpCreated *r, const char *dir);
        sscanf(arg, "%63s %399s", hex, dir);
        if (MP_Created_FromHex(hex, &r))
        {
            MP_Created_Dump(&r, dir);
            printf("[test] createddump %s %s\n", hex, dir);
        }
        else
            printf("[test] createddump %s: bad recipe\n", hex);
    }
    else if (strcmp(cmd, "remoteapp") == 0)
    {
        // remoteapp - each friend's look as we received it (base, CREATE recipe, LOOK > BIKE)
        int i;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (i != gMpLocalId && gMpPlayers[i].present)
            {
                char hex[MP_CREATED_BYTES * 2 + 1];
                MP_Created_ToHex(&gMpPlayers[i].app.recipe, hex);
                printf("[test] remoteapp %d: base %d recipe %s bike %d\n", i + 1, gMpPlayers[i].app.base, hex,
                       gMpPlayers[i].app.bike);
            }
    }
    else if (strcmp(cmd, "sprite") == 0)
    {
        // sprite NAME - wear a look by its ini name (hiker, frlg_lass, custom_1, rs_may...)
        if (MP_Avatar_ParseSpriteName(arg, &gMpConfig.app))
        {
            if (gMpConfig.app.base == MP_BASE_CREATED)
                gMpConfig.app.recipe = gMpConfig.custom[gMpConfig.app.created - 1];
            gMpConfig.app.hasCustom = FALSE;
            gMpConfig.customSprite[0] = 0;
            MP_Avatar_ApplyLocal();
            MP_SendAppearance();
            printf("[test] sprite %s\n", arg);
        }
        else
            printf("[test] sprite %s: no such look\n", arg);
    }
    else if (strcmp(cmd, "saveconfig") == 0)   // saveconfig - write the co-op settings (moemon_coop.ini, the save file's own)
    {
        char name[64];
        MP_SaveConfig();
        MP_Avatar_SpriteConfigName(&gMpConfig.app, name, sizeof(name));
        printf("[test] saveconfig sprite %s\n", name);
    }
    else if (strcmp(cmd, "lookname") == 0)   // lookname - the look worn now, by its ini name
    {
        char name[64];
        MP_Avatar_SpriteConfigName(&gMpConfig.app, name, sizeof(name));
        printf("[test] lookname %s\n", name);
    }
    else if (strcmp(cmd, "actiondump") == 0)
    {
        // actiondump FILE - the player's look in every avatar state (bike, surf, fishing...)
        void MP_Avatar_DumpActions(const char *path);
        MP_Avatar_DumpActions(arg);
    }
    else if (strcmp(cmd, "bike") == 0)
    {
        // bike - get on the Mach Bike (bike 2: the Acro Bike; bike 0: off)
        extern void SetPlayerAvatarTransitionFlags(u16 flags);
        SetPlayerAvatarTransitionFlags(arg[0] && atoi(arg) == 0 ? PLAYER_AVATAR_FLAG_ON_FOOT
                                     : atoi(arg) == 2 ? PLAYER_AVATAR_FLAG_ACRO_BIKE : PLAYER_AVATAR_FLAG_MACH_BIKE);
    }
    else if (strcmp(cmd, "metatileat") == 0)
    {
        // metatileat X,Y - the map's metatile id there (map coordinates)
        int x = 0, y = 0;
        sscanf(arg, "%d,%d", &x, &y);
        printf("[test] metatile %d,%d = 0x%X\n", x, y, MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET));
    }
    else if (strcmp(cmd, "behaviorat") == 0)
    {
        // behaviorat X,Y - the metatile behavior there (map coordinates; the Maker's tileset edits included)
        int x = 0, y = 0;
        sscanf(arg, "%d,%d", &x, &y);
        printf("[test] behavior %d,%d = 0x%X\n", x, y, MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET));
    }
    else if (strcmp(cmd, "riding") == 0)
    {
        // riding - the player's bike state (0 on foot, 1 MACH BIKE / BICYCLE, 2 ACRO BIKE)
        printf("[test] riding %d\n", TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE) ? 1
                                    : TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE) ? 2 : 0);
    }
    else if (strcmp(cmd, "mountbike") == 0)
    {
        // mountbike N - on (1 Mach, 2 Acro) or off (0) a bike the game's own way (GetOnOffBike: the bike song too)
        void GetOnOffBike(u8 transitionFlags);
        int n = atoi(arg);
        GetOnOffBike(n == 0 ? PLAYER_AVATAR_FLAG_ON_FOOT : n == 2 ? PLAYER_AVATAR_FLAG_ACRO_BIKE : PLAYER_AVATAR_FLAG_MACH_BIKE);
    }
    else if (strcmp(cmd, "iconfamily") == 0)
    {
        // iconfamily - the button icons drawn now (0 keyboard, 1 Nintendo, 2 PlayStation, 3 Xbox) and the setting
        printf("[test] iconfamily %d set %d\n", Gamepad_IconFamily(), Gamepad_GetIconSet());
    }
    else if (strcmp(cmd, "matchcalltest") == 0)
    {
        void MP_MatchCall_DebugSelect(int n);
        MP_MatchCall_DebugSelect(arg[0] ? atoi(arg) : 5);
    }
    else if (strcmp(cmd, "ridemusic") == 0)
    {
        // ridemusic BIKE SURF - OPTIONS > GAME > BIKE MUSIC / SURF MUSIC (1 on, 0 off)
        int b = 1, sf = 1;
        sscanf(arg, "%d %d", &b, &sf);
        gMpConfig.bikeMusic = b != 0;
        gMpConfig.surfMusic = sf != 0;
        printf("[test] ridemusic bike %d surf %d\n", gMpConfig.bikeMusic, gMpConfig.surfMusic);
    }
    else if (strcmp(cmd, "music") == 0)
    {
        // music - the song playing now (and the saved bike song); music play: the map's song again, as on arrival
        if (strcmp(arg, "play") == 0)
            Overworld_PlaySpecialMapMusic();
        printf("[test] music %u saved %u status %08x fanfare %d\n", (unsigned)GetCurrentMapMusic(),
               (unsigned)gSaveBlock1Ptr->savedMusic, (unsigned)gMPlayInfo_BGM.status, !IsFanfareTaskInactive());
    }
    else if (strcmp(cmd, "bikemode") == 0)
    {
        // bikemode N - LOOK > BIKE for a character who rides a Pokemon or a vehicle: 0 RIDE, 1 BIKE
        gMpConfig.app.bike = atoi(arg) ? 1 : 0;
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        printf("[test] bikemode %d (rides: %d)\n", gMpConfig.app.bike, MP_Avatar_HasRide(&gMpConfig.app));
    }
    else if (strcmp(cmd, "underwater") == 0)
    {
        // underwater - the diving sprite where you stand (looks only: nothing else about diving)
        extern void SetPlayerAvatarTransitionFlags(u16 flags);
        SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_UNDERWATER);
    }
    else if (strcmp(cmd, "repel") == 0)
    {
        // repel - one step of REPEL left, and another REPEL in the bag
        VarSet(VAR_REPEL_STEP_COUNT, 1);
        MP_SaveData_Set("repel:last", ITEM_REPEL);
        AddBagItem(ITEM_REPEL, 1);
    }
    else if (strcmp(cmd, "summary") == 0)
    {
        extern void CB2_ReturnToField(void);
        HideMapNamePopUpWindow();   // (as the START menu does: its task would draw over the summary's BG0)
        ShowPokemonSummaryScreen(SUMMARY_MODE_NORMAL, gPlayerParty, 0, gPlayerPartyCount - 1, CB2_ReturnToField);
    }
    else if (strcmp(cmd, "ivev") == 0)
    {
        // ivev SLOT [HP ATK DEF SPE SPA SPD IVs, then the six EVs]: set (all twelve given) and log a slot's IVs / EVs
        int v[13], n = sscanf(arg, "%d %d %d %d %d %d %d %d %d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5],
                              &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12]);
        if (n >= 1 && v[0] >= 0 && v[0] < PARTY_SIZE)
        {
            struct Pokemon *mon = &gPlayerParty[v[0]];
            int i;
            if (n == 13)
            {
                for (i = 0; i < NUM_STATS; i++)
                {
                    u8 iv = v[1 + i], ev = v[7 + i];
                    SetMonData(mon, MON_DATA_HP_IV + i, &iv);
                    SetMonData(mon, MON_DATA_HP_EV + i, &ev);
                }
                CalculateMonStats(mon);
            }
            MPNet_Log("ivev: slot %d IV %d %d %d %d %d %d EV %d %d %d %d %d %d ATK %d", v[0],
                      GetMonData(mon, MON_DATA_HP_IV), GetMonData(mon, MON_DATA_ATK_IV), GetMonData(mon, MON_DATA_DEF_IV),
                      GetMonData(mon, MON_DATA_SPEED_IV), GetMonData(mon, MON_DATA_SPATK_IV), GetMonData(mon, MON_DATA_SPDEF_IV),
                      GetMonData(mon, MON_DATA_HP_EV), GetMonData(mon, MON_DATA_ATK_EV), GetMonData(mon, MON_DATA_DEF_EV),
                      GetMonData(mon, MON_DATA_SPEED_EV), GetMonData(mon, MON_DATA_SPATK_EV), GetMonData(mon, MON_DATA_SPDEF_EV),
                      GetMonData(mon, MON_DATA_ATK));
        }
    }
    else if (strcmp(cmd, "frame") == 0)
    {
        // frame N - OPTIONS > FRAME (0-28; 20+ are the FireRed/LeafGreen ones)
        int n = 0;
        sscanf(arg, "%d", &n);
        gSaveBlock2Ptr->optionsWindowFrameType = n % WINDOW_FRAMES_COUNT;
    }
    else if (strcmp(cmd, "achmenu") == 0)
    {
        // achmenu TAB - CO-OP > DAILY > ACHIEVEMENTS on a tab (0 JOURNEY ... 5 EXTRAS)
        void MP_Daily_OpenAchievements(u8 tab);
        MP_Daily_OpenAchievements(atoi(arg));
    }
    else if (strcmp(cmd, "ach") == 0)
    {
        // ach - achievements done, in all and per tab
        s32 MP_Gatcha_Pulls(void);
        u8 c;
        printf("[test] achievements %d/%d:", MP_Achieve_DoneCount(), MP_Achieve_ListedTotal());
        for (c = 0; c < MP_Achieve_CatCount(); c++)
        {
            u16 done, n = MP_Achieve_CountIn(c, &done);
            printf(" %s %d/%d", MP_Achieve_CatName(c), done, n);
        }
        printf(" pulls %d\n", (int)MP_Gatcha_Pulls());
    }
    else if (strcmp(cmd, "fxunlock") == 0)
    {
        // fxunlock all | fxunlock KIND INDEX - MOEGATCHA ball pieces (0 color, 1 form, 2 particles, 3 recall)
        MP_BallUnlock_Debug(arg);
    }
    else if (strcmp(cmd, "lookskin") == 0)
    {
        // lookskin N - LOOK's skin color (0 = the sprite's own)
        gMpConfig.app.skin = atoi(arg);
    }
    else if (strcmp(cmd, "gamestat") == 0)
    {
        // gamestat N VALUE - a game stat (include/constants/game_stat.h)
        int n = 0, v = 0;
        sscanf(arg, "%d %d", &n, &v);
        SetGameStat(n, v);
    }
    else if (strcmp(cmd, "addstat") == 0)
    {
        // addstat N COUNT - a game stat counted up COUNT times, as play does (achievements count it per region)
        int n = 0, c = 1, k;
        sscanf(arg, "%d %d", &n, &c);
        for (k = 0; k < c; k++)
            IncrementGameStat(n);
    }
    else if (strcmp(cmd, "achcount") == 0)
    {
        // achcount NAME N - an achievement counter ("coop", "trade"...) counted as play does
        char name[24] = "";
        int n = 1;
        sscanf(arg, "%23s %d", name, &n);
        MP_Achieve_Count(name, n);
    }
    else if (strcmp(cmd, "money") == 0)
    {
        // money [N] - sets the money, or without N prints it
        if (arg[0])
            SetMoney(&gSaveBlock1Ptr->money, atoi(arg));
        else
            printf("[test] money %u\n", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
    }
    else if (strcmp(cmd, "daily") == 0)
    {
        void MP_Daily_OpenMenu(void);
        MP_Daily_OpenMenu();
    }
    else if (strcmp(cmd, "style") == 0)
    {
        // style SLOT VALUE: the ball style (hex: color bits 0-3, decoration 4-11, special 12-15)
        int slot = atoi(arg);
        const char *sp = strchr(arg, ' ');
        u16 v = sp ? (u16)strtol(sp + 1, NULL, 16) : 0;
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            SetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR, &v);
            printf("[test] style: slot %d = %04x\n", slot, v);
        }
    }
    else if (strcmp(cmd, "team") == 0)
    {
        // team SLOT: toggle a party slot on the co-op team (like CO-OP TEAM in the party menu)
        int slot = arg ? atoi(arg) : 0;
        u8 r = MP_Team_Toggle(slot);
        printf("[test] team: slot %d -> %d (count %d)\n", slot, r, MP_Team_Count());
    }
    else if (strcmp(cmd, "hideobjs") == 0)
    {
        // hideobjs: every object event on the map invisible, the player too (hidesprites doesn't last: the overworld
        // sets each sprite's visibility from its object every frame)
        int i;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            if (gObjectEvents[i].active)
                gObjectEvents[i].invisible = TRUE;
    }
    else if (strcmp(cmd, "hidesprites") == 0)
    {
        // hidesprites: every sprite invisible (a clean background for screenshots / mock-ups)
        int i;
        for (i = 0; i < MAX_SPRITES; i++)
            if (gSprites[i].inUse)
                gSprites[i].invisible = TRUE;
    }
    else if (strcmp(cmd, "stuckclock") == 0)
    {
        // stuckclock - make this save look like one from when the clock was stuck at day 0
        memset(&gSaveBlock2Ptr->lastBerryTreeUpdate, 0, sizeof(gSaveBlock2Ptr->lastBerryTreeUpdate));
        memset(&gSaveBlock2Ptr->localTimeOffset, 0, sizeof(gSaveBlock2Ptr->localTimeOffset));
        gSaveBlock2Ptr->localTimeOffset.hours = -10;   // the bedroom clock set to 10:00 against day 0
        FlagSet(FLAG_SYS_CLOCK_SET);
        FlagSet(FLAG_SET_WALL_CLOCK);
        VarSet(VAR_DAYS, 0);
        printf("[test] stuck clock save\n");
    }
    else if (strcmp(cmd, "berrywait") == 0)
    {
        // berrywait MIN - as if MIN minutes had passed for the Berry trees (then the time-based events run)
        void DoTimeBasedEvents(void);
        int min = 0;
        s32 total;
        struct Time *t = &gSaveBlock2Ptr->lastBerryTreeUpdate;
        sscanf(arg, "%d", &min);
        total = t->days * 1440 + t->hours * 60 + t->minutes - min;
        t->days = total / 1440;
        t->hours = (total / 60) % 24;
        t->minutes = total % 60;
        DoTimeBasedEvents();
        printf("[test] berrywait %d\n", min);
    }
    else if (strcmp(cmd, "gjoin") == 0)
    {
        // gjoin - join the test's room again (MOEMON_COOP_AUTO=join:ROOM) after goffline
        char buf[96], *room = NULL;
        const char *env = getenv("MOEMON_COOP_AUTO");
        if (env)
        {
            snprintf(buf, sizeof(buf), "%s", env);
            strtok(buf, ":");
            room = strtok(NULL, ":");
        }
        if (room)
        {
            MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
            MP_Lobby_JoinRoom(room, "");
            printf("[test] joining %s\n", room);
        }
    }
    else if (strcmp(cmd, "goffline") == 0)
    {
        // goffline - leave the room and the server (back to playing alone)
        MP_Lobby_Disconnect();
        printf("[test] offline\n");
    }
    else if (strcmp(cmd, "newday") == 0)
    {
        // newday - a new day for the daily events (daily flags, KURT's work...) without touching the clock
        void DoTimeBasedEvents(void);
        VarSet(VAR_DAYS, VarGet(VAR_DAYS) - 1);
        DoTimeBasedEvents();
        printf("[test] newday\n");
    }
    else if (strcmp(cmd, "weekday") == 0)
    {
        // weekday [D]: Crystal's weekday on the game clock (SUNDAY 0); D: the game clock moved to that weekday
        u16 MP_JohtoWeekday(void);
        int want = -1, now = MP_JohtoWeekday();
        if (sscanf(arg, "%d", &want) == 1 && want >= 0 && want < 7)
        {
            gSaveBlock2Ptr->localTimeOffset.days -= (want - now + 7) % 7;   // (local time = clock - offset)
            now = MP_JohtoWeekday();
        }
        printf("[test] weekday %d\n", now);
    }
    else if (strcmp(cmd, "hour") == 0)
    {
        // hour H: the game clock moved to that hour of the day (set it before weekday, which keeps the hour)
        int want = -1;
        RtcCalcLocalTime();
        if (sscanf(arg, "%d", &want) == 1 && want >= 0 && want < 24)
        {
            int delta = want - gLocalTime.hours;
            if (delta < 0)
                delta += 24;
            gSaveBlock2Ptr->localTimeOffset.hours -= delta;
            while (gSaveBlock2Ptr->localTimeOffset.hours < 0)
            {
                gSaveBlock2Ptr->localTimeOffset.hours += 24;   // (local = clock - offset: a day less of offset)
                gSaveBlock2Ptr->localTimeOffset.days--;
            }
            RtcCalcLocalTime();
        }
        printf("[test] hour %d\n", gLocalTime.hours);
    }
    else if (strcmp(cmd, "clock") == 0)
    {
        // clock - the game clock (online: the host's PC clock)
        struct SiiRtcInfo r;
        s64 off = 0;
        Platform_GetDateTime(&r);
        printf("[test] clock 20%02x-%02x-%02x %02x:%02x (%s)\n", r.year, r.month, r.day, r.hour, r.minute,
               MP_HostClockOffset(&off) ? "the host's" : "ours");
        {
            RtcCalcLocalTime();
            printf("[test] game time day %d %02d:%02d (rtc status %04x, offset %d.%02d:%02d)\n", gLocalTime.days, gLocalTime.hours,
                   gLocalTime.minutes, RtcGetErrorStatus(), gSaveBlock2Ptr->localTimeOffset.days,
                   gSaveBlock2Ptr->localTimeOffset.hours, gSaveBlock2Ptr->localTimeOffset.minutes);
        }
    }
    else if (strcmp(cmd, "poketch") == 0)
    {
        // poketch APP - the POKéTCH open on app APP (src/sinnoh/poketch.c)
        void MP_Poketch_Debug(u8 app);
        MP_Poketch_Debug(atoi(arg));
    }
    else if (strcmp(cmd, "poketchstick") == 0)
    {
        // poketchstick DIR - the right stick: 0 up, 1 down, 2 left, 3 right, 4 its click
        extern void Platform_ContextAction(int action);
        int d = atoi(arg);
        Platform_ContextAction(d == 4 ? INPUT_ACTION_RSTICK_CLICK : INPUT_ACTION_POKETCH_UP + d);
    }
    else if (strcmp(cmd, "poketchtap") == 0)
    {
        // poketchtap X,Y - a touch on the open POKéTCH at its LCD's X, Y
        void MP_Poketch_DebugTap(s16 x, s16 y);
        int x = 0, y = 0;
        sscanf(arg, "%d,%d", &x, &y);
        MP_Poketch_DebugTap(x, y);
    }
    else if (strcmp(cmd, "removeobj") == 0)
    {
        // removeobj LOCALID - the current map's person taken away as a script's removeobject does (SINNOH: fades out)
        RemoveObjectEventByLocalIdAndMap(atoi(arg), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    }
    else if (strcmp(cmd, "layers") == 0)
    {
        void S3D_DebugLayers(const char *arg);
        S3D_DebugLayers(arg);
    }
    else if (strcmp(cmd, "gym") == 0)
    {
        // gym - a SINNOH gym that moves: its state (src/sinnoh/s3d_gyms.c)
        void S3D_Gyms_Debug(const char *arg);
        S3D_Gyms_Debug(arg);
    }
    else if (strcmp(cmd, "tower") == 0)
    {
        // tower [BP] - the BATTLE TOWER's single battle streaks (level 50, open) and the BATTLE POINTS (BP: set them)
        if (arg[0] >= '0' && arg[0] <= '9')
            gSaveBlock2Ptr->frontier.battlePoints = atoi(arg);
        printf("[test] tower: streak %d / streak %d, bp %d\n", gSaveBlock2Ptr->frontier.towerWinStreaks[0][0],
               gSaveBlock2Ptr->frontier.towerWinStreaks[0][1], gSaveBlock2Ptr->frontier.battlePoints);
    }
    else if (strcmp(cmd, "radar") == 0)
    {
        // radar [shiny|charge] - the POKé RADAR: its patches and chain, or the next patches sparkling, or a full battery
        MP_PokeRadar_Debug(arg);
    }
    else if (strcmp(cmd, "honeyage") == 0)
    {
        // honeyage N - this place's HONEY tree's HONEY as if put on N minutes ago (src/sinnoh/s3d_wild.c)
        void Sinnoh_HoneyDebugAge(u32 minutes, s32 group);
        const char *g = strchr(arg, ',');
        Sinnoh_HoneyDebugAge(atoi(arg), g ? atoi(g + 1) : -1);
    }
    else if (strcmp(cmd, "swarm") == 0)
    {
        // swarm N - today's SINNOH swarm on route N of Platinum's list (src/sinnoh/s3d_wild.c sSwarmMaps)
        void Sinnoh_DebugSwarm(u8 r);
        Sinnoh_DebugSwarm(atoi(arg));
    }
    else if (strcmp(cmd, "wildodds") == 0)
    {
        // wildodds N - N grass encounters here as the game would choose them, counted by species
        void MP_WildOddsDump(u32 n);
        int n = 10000;
        sscanf(arg, "%d", &n);
        MP_WildOddsDump(n);
    }
    else if (strcmp(cmd, "landslots") == 0)
    {
        // landslots - grass slots 0-3 here as SINNOH changes them (time of day, swarm): the species
        u16 MP_LandSlotSpecies(u8 index);
        u16 sp[4];
        int k;
        for (k = 0; k < 4; k++)
            sp[k] = MP_LandSlotSpecies(k);
        printf("[test] land slots: %d %d %d %d\n", sp[0], sp[1], sp[2], sp[3]);
        sp[0] = MP_LandSlotSpecies(6);
        sp[1] = MP_LandSlotSpecies(7);
        printf("[test] land slots 6-7: %d %d\n", sp[0], sp[1]);
        printf("[test] land slots 8-11: %d %d %d %d\n", MP_LandSlotSpecies(8), MP_LandSlotSpecies(9), MP_LandSlotSpecies(10),
               MP_LandSlotSpecies(11));
    }
    else if (strcmp(cmd, "marshdaily") == 0)
    {
        // marshdaily - the GREAT MARSH's daily Pokemon today, areas 1-6 (src/sinnoh/s3d_legends.c)
        u16 Sinnoh_GreatMarshDailySpecies(u8 area);
        int k;
        printf("[test] marsh daily:");
        for (k = 0; k < 6; k++)
            printf(" %d", Sinnoh_GreatMarshDailySpecies(k));
        printf("\n");
    }
    else if (strcmp(cmd, "emptyparty") == 0)
    {
        // emptyparty: no POKéMON at all (a new game before its first one: SINNOH's ROUTE 201 briefcase)
        ZeroPlayerPartyMons();
        CalculatePlayerPartyCount();
        FlagClear(FLAG_SYS_POKEDEX_GET);
        printf("[test] emptyparty\n");
    }
    else if (strcmp(cmd, "rankings") == 0)
    {
        void Sinnoh_RankingsDebug(const char *arg);
        Sinnoh_RankingsDebug(arg);   // JUBILIFE TV's rankings (src/sinnoh/s3d_rankings.c)
    }
    else if (strcmp(cmd, "stower") == 0)
    {
        void Sinnoh_TowerDebug(const char *arg);
        Sinnoh_TowerDebug(arg);   // SINNOH's BATTLE TOWER (src/sinnoh/s3d_tower.c)
    }
    else if (strcmp(cmd, "brainstreak") == 0)
    {
        void Sinnoh_BrainDebug(const char *arg);
        Sinnoh_BrainDebug(arg);   // the Frontier Brains (src/sinnoh/s3d_frontier.c)
    }
    else if (strcmp(cmd, "hall") == 0)
    {
        void Sinnoh_HallDebug(void);
        Sinnoh_HallDebug();   // SINNOH's BATTLE HALL record (src/sinnoh/s3d_hall.c)
    }
    else if (strcmp(cmd, "where") == 0)
    {
        struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
        printf("[test] where map %d.%d xy %d,%d\n", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum,
               me->currentCoords.x - 7, me->currentCoords.y - 7);
    }
    else if (strcmp(cmd, "facing") == 0)   // the way the player faces (DIR_SOUTH 1, NORTH 2, WEST 3, EAST 4)
        printf("[test] facing %d\n", gObjectEvents[gPlayerAvatar.objectEventId].facingDirection);
    else if (strcmp(cmd, "layout") == 0)   // the layout the map shows (setmaplayoutindex can switch it)
        printf("[test] layout %d (map's own %d)\n", gSaveBlock1Ptr->mapLayoutId,
               Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum)->mapLayoutId);
    else if (strcmp(cmd, "friend") == 0)
    {
        // friend N: CO-OP > FRIENDS > player N (1-4)
        void MP_Linkx_OpenFriend(u8 id);
        if (atoi(arg) >= 1 && atoi(arg) <= MP_MAX_PLAYERS)
            MP_Linkx_OpenFriend(atoi(arg) - 1);
    }
    else if (strcmp(cmd, "trade") == 0)
    {
        void MP_Trade_OpenMenu(void);
        MP_Trade_OpenMenu();
    }
    else if (strcmp(cmd, "giftmenu") == 0)
    {
        void MP_Gift_OpenMenu(void);
        MP_Gift_OpenMenu();
    }
    else if (strcmp(cmd, "blender") == 0)
    {
        // blender - invite everyone online to the BERRY BLENDER
        MP_Minigame_HostBlender();
    }
    else if (strcmp(cmd, "crush") == 0)
    {
        // crush - invite everyone online to BERRY CRUSH
        MP_Minigame_HostCrush();
    }
    else if (strcmp(cmd, "joyful") == 0)
    {
        // joyful N - invite everyone online to POKEMON JUMP (0) or DODRIO BERRY PICKING (1), as TWO ISLAND's attendant
        void MP_Minigame_HostJoyful(void);
        gSpecialVar_Result = atoi(arg);
        MP_Minigame_HostJoyful();
    }
    else if (strcmp(cmd, "pokeblocks") == 0)
    {
        int i;
        printf("[test] pokeblocks:");
        for (i = 0; i < POKEBLOCKS_COUNT; i++)
            if (gSaveBlock1Ptr->pokeblocks[i].color)
                printf(" [c%d %d/%d/%d/%d/%d f%d]", gSaveBlock1Ptr->pokeblocks[i].color, gSaveBlock1Ptr->pokeblocks[i].spicy,
                       gSaveBlock1Ptr->pokeblocks[i].dry, gSaveBlock1Ptr->pokeblocks[i].sweet, gSaveBlock1Ptr->pokeblocks[i].bitter,
                       gSaveBlock1Ptr->pokeblocks[i].sour, gSaveBlock1Ptr->pokeblocks[i].feel);
        printf("  berries: cheri %d pecha %d\n", CountTotalItemQuantityInBag(ITEM_CHERI_BERRY), CountTotalItemQuantityInBag(ITEM_PECHA_BERRY));
    }
    else if (strcmp(cmd, "union") == 0)
    {
        void MP_Linkx_OpenUnion(void);
        MP_Linkx_OpenUnion();
    }
    else if (strcmp(cmd, "shopdump") == 0)
    {
        // shopdump - every department store clerk's and town mart's list now (shop.c MP_ShopDump)
        void MP_ShopDump(void);
        MP_ShopDump();
    }
    else if (strcmp(cmd, "crycheck") == 0)
    {
        // crycheck - every species' cry is in the cry tables, forward and reversed, with a sample
        void MP_CryCheck(void);
        MP_CryCheck();
    }
    else if (strcmp(cmd, "monartcheck") == 0)
    {
        // monartcheck - every species has OPTIONS > CREATURES POKEMON battle pictures (mp_monart_data.c)
        void MP_MonArt_Check(void);
        MP_MonArt_Check();
    }
    else if (strcmp(cmd, "gatchapull") == 0)
    {
        // gatchapull BANNER - a 10-pull on a banner (0 STANDARD, 1 PICKUP, 2 TRAINING, 3 BALLS), as the phone does
        void MP_Gatcha_AddPulls(s32 n);
        struct MpGatchaPrize got[10];
        u8 n = 0;
        MP_Gatcha_AddPulls(10);
        const char *res = MP_Gatcha_PhonePull(atoi(arg), 4, got, &n);
        MPNet_Log("gatchapull: %s, %d prizes", res ? res : "ok", n);   // (NULL: done)
    }
    else if (strcmp(cmd, "gatchadump") == 0)
    {
        // gatchadump - every banner's prizes at the GENERATION (mp gen N first to pick one)
        void MP_Gatcha_DebugDump(void);
        MP_Gatcha_DebugDump();
    }
    else if (strcmp(cmd, "gatcha") == 0)
    {
        void MP_Gatcha_Open(void);
        MP_Gatcha_Open();
    }
    else if (strcmp(cmd, "startmenu") == 0)
    {
        // startmenu - the START menu's rows now (POKEDEX / POKEMON by name, others by action number) and the NATIONAL DEX
        void MP_StartMenuDebug(void);
        MP_StartMenuDebug();
    }
    else if (strcmp(cmd, "menu") == 0)
    {
        // menu PAGE - open the CO-OP menu on a page (0 main, 1 look, ...)
        void MP_OpenMenuPageAt(u8 page, u8 cursor);
        int page = 0;
        sscanf(arg, "%d", &page);
        MP_OpenMenuPageAt(page, 0);
    }
    else if (strcmp(cmd, "monmoves") == 0)
    {
        // monmoves SLOT - a party Moemon's moves and held item
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
            printf("[test] monmoves: slot %d species %d moves %d %d %d %d held %d\n", slot,
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE1), (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE2),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE3), (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE4),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_HELD_ITEM));
    }
    else if (strcmp(cmd, "pcadd") == 0)
    {
        // pcadd ITEM QTY - into the item PC
        int item = 0, qty = 1;
        sscanf(arg, "%i %i", &item, &qty);
        printf("[test] pcadd %d x%d: %d\n", item, qty, AddPCItem(item, qty));
    }
    else if (strcmp(cmd, "pcqty") == 0)
    {
        // pcqty ITEM - how many the item PC holds
        int item = (int)strtol(arg, NULL, 0), i, n = 0;
        for (i = 0; i < PC_ITEMS_COUNT; i++)
            if (gSaveBlock1Ptr->pcItems[i].itemId == item)
                n += gSaveBlock1Ptr->pcItems[i].quantity;
        printf("[test] pc item %d x%d\n", item, n);
    }
    else if (strcmp(cmd, "globalitems") == 0)
    {
        // globalitems [deliver] - the GLOBAL PC's item slots (deliver: hand them to the item PC, as opening it does)
        if (strcmp(arg, "deliver") == 0)
            printf("[test] global items delivered %d\n", MP_Region_DeliverGlobalItems());
        printf("[test] global item slots %d\n", MP_Region_GlobalItemCount());
    }
    else if (strcmp(cmd, "bagqty") == 0)
    {
        // bagqty ITEM - how many the bag has
        int id = atoi(arg), n;
        for (n = 0; n < 999 && CheckBagHasItem(id, n + 1); n++)
            ;
        printf("[test] bagqty %d: %d\n", id, n);
    }
    else if (strcmp(cmd, "pc") == 0)
    {
        // pc - the POKeMON STORAGE SYSTEM, as from a PC
        void ShowPokemonStorageSystemPC(void);
        LockPlayerFieldControls();
        ShowPokemonStorageSystemPC();
    }
    else if (strcmp(cmd, "itempc") == 0)
    {
        // itempc - the player's PC (ITEM STORAGE, MAILBOX...), as from a PC
        void PlayerPC(void);
        LockPlayerFieldControls();
        PlayerPC();
    }
    else if (strcmp(cmd, "holdtest") == 0)
    {
        // holdtest - QUICK POWDER and EVIOLITE through the battle's own math on made-up battlers:
        // who moves first (GetWhoStrikesFirst) and the damage a Tackle / Water Gun does (CalculateBaseDamage).
        // In a battle only (the math reads its data); everything it touches is put back.
        u8 GetWhoStrikesFirst(u8 battler1, u8 battler2, bool8 ignoreChosenMoves);
        struct BattlePokemon saveMons[2], atk, def;
        u8 savePos[2] = {gBattlerPositions[0], gBattlerPositions[1]}, saveCount = gBattlersCount;
        u8 saveAtk = gBattlerAttacker, saveTarget = gBattlerTarget, saveCrit = gCritMultiplier;
        u16 savePower = gBattleMovePower;
        u32 saveFlags = gBattleTypeFlags;
        if (!gMain.inBattle || gBattleStruct == NULL)
        {
            printf("[test] holdtest: only in a battle\n");
            return;
        }
        static const struct { u16 species; u16 item; bool8 transformed; } sSpeed[] = {
            {SPECIES_DITTO, ITEM_NONE, FALSE}, {SPECIES_DITTO, ITEM_QUICK_POWDER, FALSE},
            {SPECIES_DITTO, ITEM_QUICK_POWDER, TRUE}, {SPECIES_PIKACHU, ITEM_QUICK_POWDER, FALSE},
        };
        static const u16 sDefenders[] = {SPECIES_BULBASAUR, SPECIES_VENUSAUR, SPECIES_DITTO};
        u32 i, j;
        memcpy(saveMons, gBattleMons, sizeof(saveMons));
        gBattlerPositions[0] = B_POSITION_PLAYER_LEFT;
        gBattlerPositions[1] = B_POSITION_OPPONENT_LEFT;
        gBattlersCount = 0;   // no abilities on the field
        gBattleTypeFlags = BATTLE_TYPE_LINK;   // no badge boost
        for (i = 0; i < ARRAY_COUNT(sSpeed); i++)
        {
            memset(&gBattleMons[0], 0, sizeof(gBattleMons[0]));
            memset(&gBattleMons[1], 0, sizeof(gBattleMons[1]));
            gBattleMons[0].species = sSpeed[i].species;
            gBattleMons[0].item = sSpeed[i].item;
            gBattleMons[0].speed = 50;
            gBattleMons[0].status2 = sSpeed[i].transformed ? STATUS2_TRANSFORMED : 0;
            gBattleMons[1].species = SPECIES_RATTATA;
            gBattleMons[1].speed = 80;
            for (j = 0; j < NUM_BATTLE_STATS; j++)
                gBattleMons[0].statStages[j] = gBattleMons[1].statStages[j] = DEFAULT_STAT_STAGE;
            printf("[test] holdtest speed species %d item %d transformed %d: %s first\n", sSpeed[i].species, sSpeed[i].item,
                   sSpeed[i].transformed, GetWhoStrikesFirst(0, 1, TRUE) == 0 ? "holder" : "foe");
        }
        for (i = 0; i < ARRAY_COUNT(sDefenders); i++)
        {
            u16 items[2] = {ITEM_NONE, ITEM_EVIOLITE}, moves[2] = {MOVE_TACKLE, MOVE_WATER_GUN};
            s32 dmg[2][2];
            u32 k, m;
            for (k = 0; k < 2; k++)
                for (m = 0; m < 2; m++)
                {
                    memset(&atk, 0, sizeof(atk));
                    memset(&def, 0, sizeof(def));
                    atk.species = SPECIES_RATTATA;
                    atk.level = 50;
                    atk.attack = atk.spAttack = 120;
                    def.species = sDefenders[i];
                    def.item = items[k];
                    def.defense = def.spDefense = 100;
                    for (j = 0; j < NUM_BATTLE_STATS; j++)
                        atk.statStages[j] = def.statStages[j] = DEFAULT_STAT_STAGE;
                    gCritMultiplier = 1;
                    dmg[k][m] = CalculateBaseDamage(&atk, &def, moves[m], 0, 0, 0, 0, 1);
                }
            printf("[test] holdtest eviolite species %d canevolve %d: tackle %d -> %d, water gun %d -> %d\n", sDefenders[i],
                   SpeciesCanEvolve(sDefenders[i]), dmg[0][0], dmg[1][0], dmg[0][1], dmg[1][1]);
        }
        memcpy(gBattleMons, saveMons, sizeof(saveMons));
        gBattlerPositions[0] = savePos[0];
        gBattlerPositions[1] = savePos[1];
        gBattlersCount = saveCount;
        gBattleTypeFlags = saveFlags;
        gBattlerAttacker = saveAtk;
        gBattlerTarget = saveTarget;
        gCritMultiplier = saveCrit;
        gBattleMovePower = savePower;
    }
    else if (strcmp(cmd, "bstatus2") == 0)
    {
        // bstatus2 BATTLER STATUS2 - sets a battler's status2 (7 = confused for 7 turns; -1 = leave it), prints both
        int b = 0, st = -1;
        sscanf(arg, "%d %d", &b, &st);
        if (b >= 0 && b < gBattlersCount)
        {
            if (st >= 0)
                gBattleMons[b].status2 = st;
            printf("[test] bstatus2 %d: status1 %u status2 %u\n", b, (unsigned)gBattleMons[b].status1, (unsigned)gBattleMons[b].status2);
        }
    }
    else if (strcmp(cmd, "pstatus") == 0)
    {
        // pstatus SLOT STATUS1 - a party Moemon's status (8 = poison, 64 = paralysis, 0 = none); prints it back
        int slot = 0, st = 0;
        u32 status;
        sscanf(arg, "%d %d", &slot, &st);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            status = st;
            if (st >= 0)
                SetMonData(&gPlayerParty[slot], MON_DATA_STATUS, &status);
            printf("[test] pstatus %d = %u\n", slot, (unsigned)GetMonData(&gPlayerParty[slot], MON_DATA_STATUS));
        }
    }
    else if (strcmp(cmd, "bgshow") == 0)
    {
        // bgshow BG 0|1 - hides or shows a background layer (debugging)
        int bg = 0, on = 1;
        sscanf(arg, "%d %d", &bg, &on);
        if (on)
            ShowBg(bg);
        else
            HideBg(bg);
    }
    else if (strcmp(cmd, "bgpeek") == 0)
    {
        // bgpeek BG TX TY - a background's tilemap entry at tile (TX, TY) and its first tile row's bytes
        int bg = 0, tx = 0, ty = 0, i;
        u16 cnt, entry;
        const u8 *tiles;
        sscanf(arg, "%d %d %d", &bg, &tx, &ty);
        cnt = GetGpuReg(REG_OFFSET_BG0CNT + bg * 2);
        entry = ((u16 *)(VRAM + ((cnt >> 8) & 0x1F) * 0x800))[(ty & 31) * 32 + (tx & 31)];
        tiles = (const u8 *)(VRAM + ((cnt >> 2) & 3) * 0x4000) + (entry & 0x3FF) * ((cnt & 0x80) ? 64 : 32);
        printf("[test] bgpeek %d %d,%d: cnt %04x entry %04x row0", bg, tx, ty, cnt, entry);
        for (i = 0; i < 8; i++)
            printf(" %02x", tiles[i]);
        printf("\n");
    }
    else if (strcmp(cmd, "gpuregs") == 0)
    {
        // gpuregs - the display registers (DISPCNT, BGnCNT, blending, windows)
        printf("[test] gpuregs dispcnt %04x bg0 %04x bg1 %04x bg2 %04x bg3 %04x bldcnt %04x bldalpha %04x bldy %04x winin %04x winout %04x\n",
               GetGpuReg(REG_OFFSET_DISPCNT), GetGpuReg(REG_OFFSET_BG0CNT), GetGpuReg(REG_OFFSET_BG1CNT), GetGpuReg(REG_OFFSET_BG2CNT),
               GetGpuReg(REG_OFFSET_BG3CNT), GetGpuReg(REG_OFFSET_BLDCNT), GetGpuReg(REG_OFFSET_BLDALPHA), GetGpuReg(REG_OFFSET_BLDY),
               GetGpuReg(REG_OFFSET_WININ), GetGpuReg(REG_OFFSET_WINOUT));
    }
    else if (strcmp(cmd, "pctag") == 0)
    {
        // pctag - the TAG button in the PC's boxes (the west button / F)
        gMpPcTagPressed = TRUE;
    }
    else if (strcmp(cmd, "pccursor") == 0)
    {
        // pccursor - the box screen's hand cursor: area, position, upside down or not
        void MP_PcStorage_LogCursor(void);
        MP_PcStorage_LogCursor();
    }
    else if (strcmp(cmd, "pctags") == 0)
    {
        // pctags - the PC's tags and the group being held
        MP_PcStorage_LogTags();
    }
    else if (strcmp(cmd, "setlevel") == 0)
    {
        // setlevel SLOT LEVEL - a party Moemon at that level (its EXP from its growth rate)
        int slot = 0, lv = 5;
        sscanf(arg, "%d %d", &slot, &lv);
        if (slot >= 0 && slot < PARTY_SIZE && lv >= 1 && lv <= MAX_LEVEL && GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES))
        {
            u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES)].growthRate][lv];
            SetMonData(&gPlayerParty[slot], MON_DATA_EXP, &exp);
            CalculateMonStats(&gPlayerParty[slot]);
            printf("[test] setlevel: slot %d level %d\n", slot, (int)GetMonData(&gPlayerParty[slot], MON_DATA_LEVEL));
        }
    }
    else if (strcmp(cmd, "candy") == 0)
    {
        // candy TYPE N - N candies of a type (TYPE -1: every type)
        int type = 0, n = 0, t;
        sscanf(arg, "%d %d", &type, &n);
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
            if ((type == t || type < 0) && t != TYPE_MYSTERY)
                gMpCandies[t] = n;
        printf("[test] candies: %d\n", (int)MP_Candy_Total());
    }
    else if (strcmp(cmd, "mart") == 0)
    {
        // mart ITEM... - a POKé MART selling these (up to 15)
        static u16 sItems[16];
        void CreatePokemartMenu(const u16 *items);
        const char *p = arg;
        int id, used, n = 0;
        while (n < 15 && sscanf(p, "%d%n", &id, &used) == 1)
        {
            sItems[n++] = id;
            p += used;
        }
        sItems[n] = ITEM_NONE;
        CreatePokemartMenu(sItems);
    }
    else if (strcmp(cmd, "tmrefund") == 0)
    {
        // tmrefund [reset]: the TMs the LOTTERY CORNER clerk would bring back (reset: she may come again)
        MP_TmRefund_Debug(arg);
    }
    else if (strcmp(cmd, "candyreload") == 0)
    {
        // candyreload - forget the candies and the BAG, then load them back from the save and its sidecar
        int t;
        MP_Candy_Reset();
        MP_Bag_LoadFromSave();
        printf("[test] candies after reload:");
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
            printf(" %d", gMpCandies[t]);
        printf(" | jar %d\n", CheckBagHasItem(ITEM_CANDY_JAR, 1));
    }
    else if (strcmp(cmd, "itemcounts") == 0)
    {
        // itemcounts ITEM - how many in the BAG, in our own PC, and in the shared PC (as last listed)
        u16 MP_SharedPc_ListedQty(u16 item);
        int item = atoi(arg), i, pc = 0;
        for (i = 0; i < PC_ITEMS_COUNT; i++)
            if (gSaveBlock1Ptr->pcItems[i].itemId == item)
                pc += gSaveBlock1Ptr->pcItems[i].quantity;
        printf("[test] itemcounts %d: bag %d pc %d shared %d\n", item, (int)CountTotalItemQuantityInBag(item), pc,
               (int)MP_SharedPc_ListedQty(item));
    }
    else if (strcmp(cmd, "sharedpc") == 0)
    {
        // sharedpc - CO-OP menu > SHARED PC (MOEMON / ITEMS / CANCEL)
        MP_SharedPc_Open();
    }
    else if (strcmp(cmd, "sharedin") == 0)
    {
        // sharedin BOX POS SPECIES LEVEL - a new Moemon straight into the shared PC (BOX from 1)
        void MP_SharedBox_DebugIn(u8 box, u8 pos, const struct BoxPokemon *b);
        int box = 1, pos = 0, species = 1, level = 5;
        struct BoxPokemon boxMon;
        sscanf(arg, "%d %d %d %d", &box, &pos, &species, &level);
        CreateBoxMon(&boxMon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
        MP_SharedBox_DebugIn(box - 1, pos, &boxMon);
        printf("[test] sharedin %d %d: %d\n", box, pos, species);
    }
    else if (strcmp(cmd, "sharedbox") == 0)
    {
        // sharedbox BOX - the shared PC's box as this game sees it (BOX from 1)
        void MP_SharedBox_DebugDump(u8 box);
        MP_SharedBox_DebugDump(atoi(arg) - 1);
    }
    else if (strcmp(cmd, "huecheck") == 0)
    {
        // huecheck - the Stadium hue math on fixed inputs, for tools/moemon_hues/check_hues.py (compares with the page)
        static const char *const names[][2] = {
            {"ASH", "PIKA"}, {"Red", "Sparky"}, {"LUNA", "Bubbles"}, {"ALICE", "Mr. Fluff"}, {"BOB", "a"},
            {"Ziggy", "ZZZZZZZZZZ"}, {"x", "(Hi):-?!"}, {"May", "Brendan"},
        };
        static const u16 ids[] = {0, 1, 255, 256, 12345, 54321, 65535};
        static const s16 hues[] = {-180, -90, -45, -1, 1, 13, 45, 90, 179, 180};
        u8 ot[16], nick[16];
        u32 i, j, c;
        for (i = 0; i < ARRAY_COUNT(names); i++)
            for (j = 0; j < ARRAY_COUNT(ids); j++)
            {
                MP_AsciiToGame(ot, names[i][0], 7);
                MP_AsciiToGame(nick, names[i][1], 10);
                printf("[hue] nv %s|%s|%u = %u\n", names[i][0], names[i][1], ids[j], MP_Hue_Number(ot, nick, ids[j]));
            }
        for (i = 1; i < NUM_SPECIES; i++)
            for (j = 0; j < 256; j += 15)
            {
                u8 mn, mx;
                MP_Hue_Limits(i, &mn, &mx);
                printf("[hue] deg %u %u %u %u = %d\n", (unsigned)i, (unsigned)j, mn, mx, MP_Hue_Degrees(i, j));
            }
        for (i = 0; i < ARRAY_COUNT(hues); i++)
            for (c = 0; c < 0x8000; c += 97)
            {
                u16 pal[16] = {0};
                pal[1] = c;
                MP_Hue_ShiftPalette(pal, 0, hues[i]);
                printf("[hue] col %d %u = %u\n", hues[i], (unsigned)c, pal[1]);
            }
        printf("[hue] done\n");
    }
    else if (strcmp(cmd, "nick") == 0)
    {
        // nick SLOT NAME - nickname a party Pokémon (SLOT from 1; NAME up to 10 letters, "-" = its species name)
        char *sp = strchr(arg, ' ');
        int slot = atoi(arg) - 1;
        if (sp != NULL && slot >= 0 && slot < PARTY_SIZE)
        {
            u8 name[POKEMON_NAME_LENGTH + 2];
            memset(name, EOS, sizeof(name));
            if (strcmp(sp + 1, "-") == 0)
                StringCopy(name, gSpeciesNames[GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES)]);
            else
                MP_AsciiToGame(name, sp + 1, POKEMON_NAME_LENGTH);
            SetMonData(&gPlayerParty[slot], MON_DATA_NICKNAME, name);
        }
    }
    else if (strcmp(cmd, "palbig") == 0)
    {
        // palbig: the species whose front palettes hold more than 16 colors (the phone link's snapshot reads them)
        const u32 *MP_SpeciesFrontPal(u16 species);
        u32 sp, n = 0;
        for (sp = 1; sp < NUM_SPECIES; sp++)
        {
            const u8 *b = (const u8 *)MP_SpeciesFrontPal(sp);
            u32 size = b[1] | (b[2] << 8) | (b[3] << 16);
            if (size > 32 && n++ < 12)
                printf("[test] palbig %u: %u bytes\n", sp, size);
        }
        printf("[test] palbig: %u species\n", n);
    }
    else if (strcmp(cmd, "shinymon") == 0)
    {
        // shinymon SLOT SPECIES LEVEL - a shiny Pokémon in a party slot (SLOT from 1; its Trainer ID is its
        // personality, which makes it shiny; the phone link test checks its colors)
        int slot = 1, species = 25, level = 5;
        sscanf(arg, "%d %d %d", &slot, &species, &level);
        if (slot >= 1 && slot <= PARTY_SIZE)
        {
            u32 pid = 0x1234ABCD;
            CreateMon(&gPlayerParty[slot - 1], species, level, 32, TRUE, pid, OT_ID_PRESET, pid);
            CalculatePlayerPartyCount();
            printf("[test] shinymon %d: %d shiny %d\n", slot, species, IsMonShiny(&gPlayerParty[slot - 1]));
        }
    }
    else if (strcmp(cmd, "huemon") == 0)
    {
        // huemon SLOT - a party Pokémon's Stadium hue
        int slot = atoi(arg) - 1;
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            struct Pokemon *mon = &gPlayerParty[slot];
            u16 species = GetMonData(mon, MON_DATA_SPECIES);
            u8 ot[PLAYER_NAME_LENGTH + 2], nick[POKEMON_NAME_LENGTH + 2], mn, mx;
            char aot[32], anick[32];
            void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
            memset(ot, EOS, sizeof(ot));
            memset(nick, EOS, sizeof(nick));
            GetMonData(mon, MON_DATA_OT_NAME, ot);
            GetMonData(mon, MON_DATA_NICKNAME, nick);
            MP_GameToAscii(aot, ot, PLAYER_NAME_LENGTH);
            MP_GameToAscii(anick, nick, POKEMON_NAME_LENGTH);
            MP_Hue_Limits(species, &mn, &mx);
            printf("[test] huemon %d species %d hue %d\n", slot + 1, species, MP_Hue_OfBoxMon(&mon->box));
            if (StringCompare(nick, gSpeciesNames[species]) != 0)   // nicknamed: the page's answer to compare
                printf("[hue] mon %s|%s|%u %u %u = %d\n", aot, anick, (unsigned)(GetMonData(mon, MON_DATA_OT_ID) & 0xFFFF), mn, mx,
                       MP_Hue_OfBoxMon(&mon->box));
        }
    }
    else if (strcmp(cmd, "sharedname") == 0)
    {
        // sharedname BOX NAME - rename a shared box (as the box screen's NAME does; BOX from 1, NAME up to 8 letters)
        void MP_SharedBox_DebugName(u8 box, const char *name);
        char *sp = strchr(arg, ' ');
        if (sp != NULL)
            MP_SharedBox_DebugName(atoi(arg) - 1, sp + 1);
    }
    else if (strcmp(cmd, "monids") == 0)
    {
        // monids - every Moemon of ours and of the shared PC by personality:OT ID
        void MP_SharedBox_DebugIds(void);
        MP_SharedBox_DebugIds();
    }
    else if (strcmp(cmd, "ownbox") == 0)
    {
        // ownbox BOX - our own box (even with the shared PC's on screen)
        struct PokemonStorage *own = MP_PersonalStorage();
        int box = atoi(arg) - 1, i;
        if (box >= 0 && box < TOTAL_BOXES_COUNT)
        {
            printf("[test] ownbox %d:", box + 1);
            for (i = 0; i < IN_BOX_COUNT; i++)
                if (GetBoxMonData(&own->boxes[box][i], MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d:%d", i, (int)GetBoxMonData(&own->boxes[box][i], MON_DATA_SPECIES));
            printf(" | party:");
            for (i = 0; i < PARTY_SIZE; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES));
            printf("\n");
        }
    }
    else if (strcmp(cmd, "candyjar") == 0)
    {
        MP_Candy_OpenJar(CB2_ReturnToField);
    }
    else if (strcmp(cmd, "jarstate") == 0)
    {
        MP_Candy_LogJar();
    }
    else if (strcmp(cmd, "boxput") == 0)
    {
        // boxput BOX POS SPECIES LEVEL - a Moemon in a box slot (BOX from 1)
        int box = 1, pos = 0, species = 1, level = 5;
        struct BoxPokemon boxMon;
        sscanf(arg, "%d %d %d %d", &box, &pos, &species, &level);
        if (box >= 1 && box <= TOTAL_BOXES_COUNT && pos >= 0 && pos < IN_BOX_COUNT)
        {
            CreateBoxMon(&boxMon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
            SetBoxMonAt(box - 1, pos, &boxMon);
            printf("[test] boxput %d %d: %d\n", box, pos, species);
        }
    }
    else if (strcmp(cmd, "boxnick") == 0)
    {
        // boxnick BOX POS NAME - nickname a boxed Moemon (BOX from 1; NAME up to 10 letters): its Stadium hue
        int box = 1, pos = 0, n = 0;
        sscanf(arg, "%d %d %n", &box, &pos, &n);
        if (n > 0 && box >= 1 && box <= TOTAL_BOXES_COUNT && pos >= 0 && pos < IN_BOX_COUNT)
        {
            struct BoxPokemon *b = GetBoxedMonPtr(box - 1, pos);
            u8 name[POKEMON_NAME_LENGTH + 2];
            memset(name, EOS, sizeof(name));
            MP_AsciiToGame(name, arg + n, POKEMON_NAME_LENGTH);
            SetBoxMonData(b, MON_DATA_NICKNAME, name);
            printf("[test] boxnick %d %d: hue %d\n", box, pos, MP_Hue_OfBoxMon(b));
        }
    }
    else if (strcmp(cmd, "curbox") == 0)
    {
        // curbox [BOX] - the PC's box in use (BOX from 1: set it first), its name and wallpaper
        struct PokemonStorage *own = MP_PersonalStorage();
        int box = atoi(arg);
        char name[32];
        if (box >= 1 && box <= TOTAL_BOXES_COUNT)
            own->currentBox = box - 1;
        MP_GameToAscii(name, own->boxNames[own->currentBox], sizeof(name) - 1);
        printf("[test] curbox %d %s wallpaper %d\n", own->currentBox + 1, name, own->boxWallpapers[own->currentBox]);
    }
    else if (strcmp(cmd, "boxmons") == 0)
    {
        // boxmons BOX - the box's Moemon (position:species) and the party's species
        int box = 1, i;
        sscanf(arg, "%d", &box);
        if (box >= 1 && box <= TOTAL_BOXES_COUNT)
        {
            printf("[test] boxmons %d:", box);
            for (i = 0; i < IN_BOX_COUNT; i++)
                if (GetBoxMonDataAt(box - 1, i, MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d:%d", i, (int)GetBoxMonDataAt(box - 1, i, MON_DATA_SPECIES));
            printf(" | party:");
            for (i = 0; i < PARTY_SIZE; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES));
            printf("\n");
        }
    }
    else if (strcmp(cmd, "friendship") == 0)
    {
        // friendship SLOT N: a party POKeMON's friendship
        int slot = 0, n = 0;
        if (sscanf(arg, "%d %d", &slot, &n) == 2 && slot >= 0 && slot < PARTY_SIZE)
        {
            u8 f = n;
            SetMonData(&gPlayerParty[slot], MON_DATA_FRIENDSHIP, &f);
        }
    }
    else if (strcmp(cmd, "cardshop") == 0)
    {
        // cardshop: the GAME CENTER's prices here (a POKeMON promo card, the Triple Triad seller's singles)
        extern unsigned Tcg_ShopPrice(unsigned card);
        extern unsigned TtUi_DebugShop(unsigned region, unsigned *topPrice);
        unsigned top, n = TtUi_DebugShop(1, &top), promo = 0, id;
        for (id = 7; id < 500 && !promo; id++)
            if (Tcg_ShopPrice(id) == 12000)
                promo = id;
        printf("[test] cardshop tcg promo %u price %u, tt singles %u top %u\n", promo, promo ? Tcg_ShopPrice(promo) : 0, n, top);
    }
    else if (strcmp(cmd, "evotarget") == 0)
    {
        // evotarget SLOT [ITEM]: what the party POKeMON would evolve into: on leveling up, or with ITEM used on it
        int slot = 0, item = 0;
        sscanf(arg, "%d %d", &slot, &item);
        if (slot >= 0 && slot < PARTY_SIZE)
            printf("[test] evotarget %d item %d: %u\n", slot, item, GetEvolutionTargetSpecies(&gPlayerParty[slot],
                   item ? EVO_MODE_ITEM_CHECK : EVO_MODE_NORMAL, item));
    }
    else if (strcmp(cmd, "ruinsmark") == 0)
    {
        // ruinsmark - as an ESCAPE ROPE and FLASH used where the player stands (the RUINS OF ALPH's walls, qz_specials.c)
        bool8 QzRuinsFlash(void);
        void QzRuinsEscapeRope(void);
        QzRuinsEscapeRope();
        printf("[test] ruinsmark: flash %d\n", QzRuinsFlash());
    }
    else if (strcmp(cmd, "evolve") == 0)
    {
        // evolve SLOT - the party POKeMON evolves now if leveling up would evolve it (the game's own check and scene)
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u16 target = GetEvolutionTargetSpecies(&gPlayerParty[slot], EVO_MODE_NORMAL, 0);
            extern void (*gCB2_AfterEvolution)(void);
            printf("[test] evolve %d: %u\n", slot, target);
            if (target != SPECIES_NONE)
            {
                void BeginEvolutionScene(struct Pokemon *mon, u16 postEvoSpecies, bool8 canStopEvo, u8 partyId);
                gCB2_AfterEvolution = CB2_ReturnToField;
                BeginEvolutionScene(&gPlayerParty[slot], target, FALSE, slot);
            }
        }
    }
    else if (strcmp(cmd, "useitemon") == 0)
    {
        // useitemon SLOT ITEM - use an item from the field on a party POKéMON as the party menu does (its table effect,
        // or an evolution: the evolution scene starts); logs whether it had an effect
        int slot = 0, item = 0;
        sscanf(arg, "%d %d", &slot, &item);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            bool8 none;
            extern void (*gCB2_AfterEvolution)(void);
            gCB2_AfterEvolution = CB2_ReturnToField;   // (the party menu sets its own)
            none = ExecuteTableBasedItemEffect(&gPlayerParty[slot], item, slot, 0);
            printf("[test] useitemon %d item %d: %s\n", slot, item, none ? "no effect" : "used");
        }
    }
    else if (strcmp(cmd, "item") == 0)
    {
        // item ID QTY - put items in the bag
        int id = 1, qty = 1;
        sscanf(arg, "%d %d", &id, &qty);
        printf("[test] item %d x%d: %s\n", id, qty, AddBagItem(id, qty) ? "ok" : "FAILED");
    }
    else if (strcmp(cmd, "unitem") == 0)
    {
        // unitem ID - take every one of an item out of the bag
        int id = atoi(arg);
        u16 n = CountTotalItemQuantityInBag(id);
        printf("[test] unitem %d x%d: %s\n", id, n, n == 0 || RemoveBagItem(id, n) ? "ok" : "FAILED");
    }
    else if (strcmp(cmd, "badegg") == 0)
    {
        // badegg SLOT - break a party Pokemon the way co-op catches broke (OT ID overwritten)
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            gPlayerParty[slot].box.otId ^= 0x00010203;
            printf("[test] badegg: slot %d species now %d\n", slot, (int)GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES));
        }
    }
    else if (strcmp(cmd, "giveforeign") == 0)
    {
        // giveforeign SPECIES[:LEVEL[:lead]] - a Pokemon made in another player's game (another OT
        // ID), given the way a co-op catch is (GiveMonToPlayer); ":lead" swaps it into slot 0
        static struct Pokemon mon;
        int species = 0, level = 5;
        char lead[8] = "";
        sscanf(arg, "%d:%d:%7s", &species, &level, lead);
        CreateMon(&mon, species > 0 ? species : 288, level > 0 && level <= 100 ? level : 5, USE_RANDOM_IVS, FALSE, 0, OT_ID_PRESET, 0x9ABC1234);
        GiveMonToPlayer(&mon);
        if (strcmp(lead, "lead") == 0 && CalculatePlayerPartyCount() > 1)
        {
            struct Pokemon tmp = gPlayerParty[0];
            gPlayerParty[0] = gPlayerParty[CalculatePlayerPartyCount() - 1];
            gPlayerParty[CalculatePlayerPartyCount() - 1] = tmp;
        }
        printf("[test] giveforeign: given, bad egg %d\n", (int)MP_IsBoxMonBadEgg(&gPlayerParty[CalculatePlayerPartyCount() - 1].box));
    }
    else if (strcmp(cmd, "heap") == 0)
    {
        void MP_HeapStats(u32 *used, u32 *freeTotal, u32 *largestFree);
        u32 used, freeTotal, largest;
        MP_HeapStats(&used, &freeTotal, &largest);
        printf("[test] heap: used %u, free %u, largest free %u\n", (unsigned)used, (unsigned)freeTotal, (unsigned)largest);
    }
    else if (strcmp(cmd, "ballfx") == 0)
    {
        // mp ballfx SLOT FORM PART COLOR RECALL SOUND  |  mp ballfx SLOT preset N
        int slot = 0, f = 0, pa = 0, c = 0, r = 0, so = 0;
        struct MpBallFx fx;
        if (sscanf(arg, "%d preset %d", &slot, &f) == 2)
            MP_BallFx_Preset(f, &fx);
        else
        {
            sscanf(arg, "%d %d %d %d %d %d", &slot, &f, &pa, &c, &r, &so);
            fx.form = f; fx.part = pa; fx.color = c; fx.recall = r; fx.sound = so;
        }
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            MP_BallFx_SetBoxMon(&gPlayerParty[slot].box, &fx);
            MP_BallFx_OfMon(&gPlayerParty[slot], &fx);
            printf("[test] ballfx: slot %d form %d part %d color %d recall %d sound %d\n", slot, fx.form, fx.part, fx.color, fx.recall, fx.sound);
        }
    }
    else if (strcmp(cmd, "settrend") == 0)
    {
        // settrend SLOT W1 W2 TRENDINESS - put a phrase straight into a saved trend slot (hex words)
        unsigned slot = 0, w1 = 0, w2 = 0, t = 50;
        if (sscanf(arg, "%u %x %x %u", &slot, &w1, &w2, &t) >= 3 && slot < SAVED_TRENDS_COUNT)
        {
            gSaveBlock1Ptr->dewfordTrends[slot].words[0] = w1;
            gSaveBlock1Ptr->dewfordTrends[slot].words[1] = w2;
            gSaveBlock1Ptr->dewfordTrends[slot].trendiness = t;
            gSaveBlock1Ptr->dewfordTrends[slot].maxTrendiness = t > 127 ? 127 : (t < 1 ? 1 : t);
        }
    }
    else if (strcmp(cmd, "trends") == 0)
    {
        int i;
        for (i = 0; i < SAVED_TRENDS_COUNT; i++)
        {
            char ascii[64];
            u8 buf[64];
            ConvertEasyChatWordsToString(buf, gSaveBlock1Ptr->dewfordTrends[i].words, 2, 1);
            MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
            printf("[test] trends[%d]: %04x %04x \"%s\" t %d max %d gain %d rand %04x\n", i, gSaveBlock1Ptr->dewfordTrends[i].words[0],
                   gSaveBlock1Ptr->dewfordTrends[i].words[1], ascii, gSaveBlock1Ptr->dewfordTrends[i].trendiness,
                   gSaveBlock1Ptr->dewfordTrends[i].maxTrendiness, gSaveBlock1Ptr->dewfordTrends[i].gainingTrendiness,
                   gSaveBlock1Ptr->dewfordTrends[i].rand);
        }
    }
    else if (strcmp(cmd, "easychat") == 0)
    {
        // easychat TYPE PERSON - the word screen as an NPC would open it (5 interview, 0/1 reporter, 2 boy)
        static u16 sWords[9] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
        int type = 5, person = 0;
        sscanf(arg, "%d %d", &type, &person);
        DoEasyChatScreen(type, sWords, CB2_ReturnToField, person);
    }
    else if (strcmp(cmd, "trend") == 0)
    {
        // trend - Dewford's current trendy phrase
        char ascii[64];
        u8 buf[64];
        ConvertEasyChatWordsToString(buf, gSaveBlock1Ptr->dewfordTrends[0].words, 2, 1);
        MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
        printf("[test] trend: %04x %04x \"%s\" (trendiness %d)\n", gSaveBlock1Ptr->dewfordTrends[0].words[0],
               gSaveBlock1Ptr->dewfordTrends[0].words[1], ascii, gSaveBlock1Ptr->dewfordTrends[0].trendiness);
    }
    else if (strcmp(cmd, "warp") == 0)
    {
        // warp GROUP,NUM,X,Y - a walk-in warp to that map spot (on the field)
        int g = 0, n = 0, x = 0, y = 0;
        if (sscanf(arg, "%d,%d,%d,%d", &g, &n, &x, &y) == 4)
        {
            SetWarpDestination(g, n, WARP_ID_NONE, x, y);
            DoWarp();
            printf("[test] warp: %d,%d %d,%d\n", g, n, x, y);
        }
    }
    else if (strcmp(cmd, "textspeed") == 0)
    {
        // textspeed 0|1|2 - OPTIONS > TEXT SPEED (slow, mid, fast)
        gSaveBlock2Ptr->optionsTextSpeed = atoi(arg) % 3;
        printf("[test] text speed %d\n", gSaveBlock2Ptr->optionsTextSpeed);
    }
    else if (strcmp(cmd, "face") == 0)
    {
        // face up|down|left|right - turn the player in place
        static const char *const sDirs[] = {"down", "up", "left", "right"};
        int d;
        if (!MP_IsInOverworld())
        {
            printf("[test] face: not on the field\n");   // (a battle or a menu: no player sprite to turn)
            return;
        }
        for (d = 0; d < 4; d++)
            if (strcmp(arg, sDirs[d]) == 0)
            {
                ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], DIR_SOUTH + d);
                printf("[test] face: %s\n", sDirs[d]);
            }
    }
    else if (strcmp(cmd, "ballstyle") == 0)
    {
        // ballstyle SLOT HEX - a party Moemon's POKé BALL style (c000 GENESIS BALL, d000 EPOCH BALL)
        int slot = 0;
        unsigned style = 0;
        if (sscanf(arg, "%d %x", &slot, &style) == 2 && slot >= 0 && slot < PARTY_SIZE)
        {
            u32 v = style;
            SetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR, &v);
            printf("[test] ballstyle: slot %d style %04x\n", slot, (int)GetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR));
        }
    }
    else if (strcmp(cmd, "fxget") == 0)
    {
        // fxget SLOT - a party Moemon's ball effect, markings and ball style as stored
        int slot = atoi(arg);
        struct MpBallFx fx;
        u16 style;
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            MP_BallFx_OfMon(&gPlayerParty[slot], &fx);
            style = MP_BallStyle_OfMon(&gPlayerParty[slot]);
            printf("[test] fxget: slot %d form %d part %d color %d recall %d sound %d markings %d style %04x (color %d deco %d special %d)\n",
                   slot, fx.form, fx.part, fx.color, fx.recall, fx.sound, (int)GetMonData(&gPlayerParty[slot], MON_DATA_MARKINGS),
                   style, MP_BallStyle_Color(style), MP_BallStyle_Decoration(style), MP_BallStyle_Special(style));
        }
    }
    else if (strcmp(cmd, "markings") == 0)
    {
        // markings SLOT VALUE
        int slot = 0, v = 0;
        sscanf(arg, "%d %d", &slot, &v);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u8 m = v;
            SetMonData(&gPlayerParty[slot], MON_DATA_MARKINGS, &m);
        }
    }
    else if (strcmp(cmd, "stylemake") == 0)
    {
        // stylemake SLOT COLOR DECORATION - a POKé BALL style through MP_BallStyle_Make
        int slot = 0, c = 0, d = 0;
        sscanf(arg, "%d %d %d", &slot, &c, &d);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u16 v = MP_BallStyle_Make(c, d);
            SetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR, &v);
        }
    }
    else if (strcmp(cmd, "fxedit") == 0)
    {
        // fxedit SLOT - the ball effect editor for a party Moemon (back to the field after)
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
            MP_BallFxEditor_Open(&gPlayerParty[slot].box, CB2_ReturnToField);
    }
    else if (strcmp(cmd, "budget") == 0)
    {
        void MP_SpriteBudget(u8 *freeSprites, u8 *freePalettes, u16 *freeTiles, u8 *freeMatrices);
        u8 sp, pal, mat;
        u16 tiles;
        MP_SpriteBudget(&sp, &pal, &tiles, &mat);
        printf("[test] sprites: free %d sprites, %d palettes, %d tiles, %d matrices\n", sp, pal, tiles, mat);
    }
    else if (strcmp(cmd, "compensate") == 0)
    {
        MP_CompensateBadEggs();
    }
    else if (strcmp(cmd, "bflags") == 0)
    {
        // bflags - the battle's type flags and battler count (and a few co-op bits)
        printf("[test] bflags %08x battlers %d coop %d wild %d triple %d inBattle %d\n", (unsigned)gBattleTypeFlags,
               gBattlersCount, gMpCoopBattle, gMpCoopWild, gIsTripleBattle, gMain.inBattle);
    }
    else if (strcmp(cmd, "usecut") == 0)
    {
        // usecut - CUT as if chosen in the party menu (party slot 0): what it does here
        bool8 SetUpFieldMove_Cut(void);
        extern void (*gPostMenuFieldCallback)(void);
        if (SetUpFieldMove_Cut() && gPostMenuFieldCallback != NULL)
        {
            gPostMenuFieldCallback();
            printf("[test] usecut: used\n");
        }
        else
            printf("[test] usecut: nothing\n");
    }
    else if (strcmp(cmd, "beattrainer") == 0)
    {
        // beattrainer ID - the trainer counts as beaten (its trainer flag set)
        int id = atoi(arg);
        if (id > 0 && id < 4096)   // (JOHTO / KANTO / SINNOH trainers too: TRAINER_FLAG maps them)
            FlagSet(TRAINER_FLAG(id));
        printf("[test] beattrainer %d\n", id);
    }
    else if (strcmp(cmd, "trainerflag") == 0)
    {
        // trainerflag ID [ID...] - whether each trainer counts as beaten on this game
        char *tok = strtok(arg, " ");
        while (tok != NULL)
        {
            int id = atoi(tok);
            if (id > 0 && id < 4096)
                printf("[test] trainerflag %d = %d\n", id, FlagGet(TRAINER_FLAG(id)) ? 1 : 0);
            tok = strtok(NULL, " ");
        }
        fflush(stdout);
    }
    else if (strcmp(cmd, "partyids") == 0)
    {
        // partyids - each party Moemon's identity (species:personality:OT ID) and level / EXP, to check nothing was
        // lost, swapped, doubled or turned into a BAD EGG
        int i;
        printf("[test] partyids:");
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
                continue;
            printf(" %d:%08x:%08x:L%d:E%u%s", (int)GetMonData(mon, MON_DATA_SPECIES), (unsigned)GetMonData(mon, MON_DATA_PERSONALITY),
                   (unsigned)GetMonData(mon, MON_DATA_OT_ID), (int)GetMonData(mon, MON_DATA_LEVEL), (unsigned)GetMonData(mon, MON_DATA_EXP),
                   GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG) ? ":BADEGG" : "");
        }
        printf("\n");
    }
    else if (strcmp(cmd, "bwshown") == 0)   // bwshown - the weather on screen and the battle's own (mp_battleweather.c)
    {
        void MP_BattleWeather_Debug(void);
        MP_BattleWeather_Debug();
    }
    else if (strcmp(cmd, "bweather") == 0)   // bweather rain|downpour|sun|sand|hail|none - the battle's weather now
    {
        static const struct { const char *name; u16 flags; } sW[] = {
            {"rain", B_WEATHER_RAIN_TEMPORARY}, {"downpour", B_WEATHER_RAIN_DOWNPOUR}, {"sun", B_WEATHER_SUN_TEMPORARY},
            {"sand", B_WEATHER_SANDSTORM_TEMPORARY}, {"hail", B_WEATHER_HAIL_TEMPORARY}, {"none", 0},
        };
        int i;
        for (i = 0; i < (int)ARRAY_COUNT(sW); i++)
            if (strcmp(arg, sW[i].name) == 0 && gMain.inBattle)
            {
                gBattleWeather = sW[i].flags;
                gWishFutureKnock.weatherDuration = sW[i].flags ? 5 : 0;
                printf("[test] battle weather: %s\n", sW[i].name);
            }
    }
    else if (strcmp(cmd, "partycursor") == 0)   // where the party screen's cursor is (7 = CANCEL) and its layout
    {
        printf("[test] party menu cursor %d layout %d type %d (triple %d co-op %d flags %x)\n", gPartyMenu.slotId, gPartyMenu.layout,
               gPartyMenu.menuType, gIsTripleBattle, gMpCoopBattle, (unsigned)gBattleTypeFlags);
    }
    else if (strcmp(cmd, "regioninfo") == 0)
    {
        // regioninfo: the region, its progress, the party's levels, the GLOBAL PC's room (mp_region.c)
        int i;
        printf("[test] region %d cleared %d started %d global free %d party:", MP_Region_Current(), gRegionState.cleared,
               gRegionState.started, MP_Region_GlobalFree());
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES))
                printf(" %d:L%d", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES), (int)GetMonData(&gPlayerParty[i], MON_DATA_LEVEL));
        printf(" box1 %d%s badges %d linked %d\n", (int)GetBoxMonData(&MP_PersonalStorage()->boxes[0][0], MON_DATA_SPECIES),
               gPokemonStoragePtr == MP_PersonalStorage() ? " (own boxes)" : " (other boxes shown)",
               MP_Region_BadgeCount(MP_Region_Current()), MP_Region_Cleared(MP_Region_Current()));
    }
    else if (strcmp(cmd, "pcmenu") == 0)
    {
        // pcmenu: a POKéMON CENTER's PC (its menu: storage, the player's PC, HALL OF FAME, GLOBAL PC...)
        extern const u8 EventScript_PC[];
        ScriptContext_SetupScript(EventScript_PC);
    }
    else if (strcmp(cmd, "tv") == 0)
    {
        // tv: SINNOH's TV (src/sinnoh/s3d_tv.c): the program on air, what's kept
        void Sinnoh_TvDebug(void);
        Sinnoh_TvDebug();
    }
    else if (strcmp(cmd, "seagallop") == 0)
    {
        // seagallop FROM TO: the SEAGALLOP's trip as its script sails it (fadescreen, QzDoSeagallopFerryScene)
        u16 QzDoSeagallopFerryScene(void);
        unsigned from = 0, to = 1;
        sscanf(arg, "%u %u", &from, &to);
        gSpecialVar_0x8004 = from, gSpecialVar_0x8006 = to;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, 0);   // (to black)
        QzDoSeagallopFerryScene();
    }
    else if (strcmp(cmd, "tvscreens") == 0)
    {
        // tvscreens [off]: the map's TVs lit as when a show is on the air, or turned off (tv.c MP_Tv_DebugScreens)
        void MP_Tv_DebugScreens(bool8 on);
        MP_Tv_DebugScreens(strcmp(arg, "off") != 0);
    }
    else if (strcmp(cmd, "tvnow") == 0)
    {
        // tvnow: SINNOH NOW's BERRY LOOKOUT and POKéMON PHOTO RATING as they'd air now (s3d_tv.c)
        void Sinnoh_TvDebugNow(void);
        Sinnoh_TvDebugNow();
    }
    else if (strcmp(cmd, "tvprogram") == 0)
    {
        // tvprogram N: Platinum's program N (1-9) on air from now on, whatever the hour (0: the schedule's again)
        void Sinnoh_TvForceProgram(u8 program);
        Sinnoh_TvForceProgram(atoi(arg));
        printf("[test] tv program %d\n", atoi(arg));
    }
    else if (strcmp(cmd, "roamers") == 0)
    {
        // roamers [SLOT]: SINNOH's roaming POKéMON (s3d_roamer.c: where each is, its HP); SLOT: that one to this map
        void Sinnoh_RoamerDebug(void);
        void Sinnoh_RoamerHere(u8 slot);
        if (arg && strncmp(arg, "meet", 4) == 0)
        {
            // roamers meet: a wild encounter here until it's a roamer (roamer.c TryStartRoamerEncounter), its battle
            int tries;
            for (tries = 0; tries < 64; tries++)
                if (TryStartRoamerEncounter())
                {
                    BattleSetup_StartRoamerBattle();
                    break;
                }
            printf("[test] roamer met: %d\n", tries < 64);
        }
        else if (arg && *arg)
            Sinnoh_RoamerHere(atoi(arg));
        Sinnoh_RoamerDebug();
    }
    else if (strcmp(cmd, "jroamers") == 0)
    {
        // jroamers [init | SLOT | meet]: JOHTO's beasts (mp_johto_roamer.c): set going / SLOT to this map / a wild
        // encounter here until it's one of them, its battle
        void MP_JohtoRoamer_Debug(void);
        void MP_JohtoRoamer_Here(u8 slot);
        void MP_JohtoRoamer_Init(void);
        if (arg && strncmp(arg, "init", 4) == 0)
            MP_JohtoRoamer_Init();
        else if (arg && strncmp(arg, "meet", 4) == 0)
        {
            int tries;
            for (tries = 0; tries < 256; tries++)
                if (TryStartRoamerEncounter())
                {
                    BattleSetup_StartRoamerBattle();
                    break;
                }
            printf("[test] roamer met: %d\n", tries < 256);
        }
        else if (arg && *arg)
            MP_JohtoRoamer_Here(atoi(arg));
        MP_JohtoRoamer_Debug();
    }
    else if (strcmp(cmd, "tram") == 0)
    {
        // tram: the GREAT MARSH's tram (s3d_tram.c: its stop, where the car is, moving)
        void Sinnoh_TramDebug(void);
        Sinnoh_TramDebug();
    }
    else if (strcmp(cmd, "lift") == 0)
    {
        // lift: SINNOH's platform lift on this map (s3d_lift.c: which, its floor, height, riding)
        void Sinnoh_LiftDebug(void);
        Sinnoh_LiftDebug();
    }
    else if (strcmp(cmd, "underground") == 0)
    {
        // underground: SINNOH's Underground (where the player went down, the dig spots, SPIRITOMB's counter)
        Sinnoh_UndergroundDebug();
    }
    else if (strcmp(cmd, "explorerkit") == 0)
    {
        // explorerkit: the EXPLORER KIT used (down into the Underground, or back up), as from the BAG
        extern const u8 Sinnoh_EventScript_ExplorerKit[];
        const u8 *why = Sinnoh_ExplorerKitCantUse();
        printf("[test] explorer kit: %s\n", why ? "can't be used here" : "used");
        if (why == NULL)
            ScriptContext_SetupScript(Sinnoh_EventScript_ExplorerKit);
    }
    else if (strcmp(cmd, "ugspot") == 0)
    {
        // ugspot: to the tile beside the nearest dig spot (then ugface turns the player to it)
        s16 x, y;
        if (Sinnoh_UndergroundNearestSpot(&x, &y, &sUgFace))
        {
            SetWarpDestination(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE, x, y);
            DoWarp();
            printf("[test] ugspot: %d,%d dir %d\n", x, y, sUgFace);
        }
        else
            printf("[test] ugspot: none\n");
    }
    else if (strcmp(cmd, "ugface") == 0)
    {
        ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], sUgFace);
        printf("[test] ugface: %d\n", sUgFace);
    }
    else if (strcmp(cmd, "mining") == 0)
    {
        // mining: the digging screen's wall (what is buried, what lies bare)
        Sinnoh_MiningDebug();
    }
    else if (strcmp(cmd, "minecell") == 0)
    {
        // minecell: the digging screen's cursor to a cell over a treasure still under dirt
        u8 x, y;
        if (Sinnoh_MiningTestCell(&x, &y))
            printf("[test] minecell: %d,%d\n", x, y);
        else
            printf("[test] minecell: none\n");
    }
    else if (strcmp(cmd, "minestrike") == 0)
    {
        // minestrike: one blow at the digging screen's cursor, only while its wall is being dug (a button pressed
        // after the screen closed would start the next spot's digging)
        bool8 Sinnoh_MiningTestStrike(void);
        printf("[test] minestrike: %d\n", Sinnoh_MiningTestStrike());
    }
    else if (strcmp(cmd, "tvwatch") == 0)
    {
        // tvwatch: watch SINNOH's TV (as its TV tiles do: field_control_avatar.c)
        extern const u8 gSinnohTvBroadcast[];
        ScriptContext_SetupScript(gSinnohTvBroadcast);
    }
    else if (strcmp(cmd, "tvcatch") == 0)
    {
        // tvcatch SPECIES BALLS: a catch of the party's first POKéMON as if in SINNOH's wild (its CATCH THAT POKéMON SHOW)
        void Sinnoh_TvWildBattleEnd(struct Pokemon *mon, u8 result, u16 ballsThrown, u16 ball);
        int balls = 1;
        sscanf(arg, "%*d %d", &balls);
        Sinnoh_TvWildBattleEnd(&gPlayerParty[0], B_OUTCOME_CAUGHT, balls, ITEM_POKE_BALL);
    }
    else if (strcmp(cmd, "sinnohticket") == 0)
    {
        // sinnohticket: SINNOH's S.S. TICKET as the CHAMPION's win gives it (data/scripts/mp_region.inc)
        extern const u8 EventScript_SinnohTicket[];
        ScriptContext_SetupScript(EventScript_SinnohTicket);
    }
    else if (strcmp(cmd, "regionreload") == 0)
    {
        // regionreload: writes "<save>.region" and reads it back (chunks: other regions' PCs, the GLOBAL PC, levels)
        void AwFile_Flush(void);
        MP_Region_WriteSidecar();
        AwFile_Flush();   // (the write goes on in the background: wait for it before reading back)
        MP_Region_LoadFromSave();
    }
    else if (strcmp(cmd, "s3dtext") == 0)
    {
        // s3dtext BANK MSG: a message of SINNOH's text pack through the placeholder scripts use, in a message box
        // (the log has only its length)
        static u8 sPlaceholder[7];
        int bank = 0, msg = 0;
        sscanf(arg, "%d %d", &bank, &msg);
        sPlaceholder[0] = PLACEHOLDER_BEGIN;
        sPlaceholder[1] = PLACEHOLDER_ID_SINNOH_TEXT;
        sPlaceholder[2] = (bank >> 7) & 0x7F;
        sPlaceholder[3] = bank & 0x7F;
        sPlaceholder[4] = (msg >> 7) & 0x7F;
        sPlaceholder[5] = msg & 0x7F;
        sPlaceholder[6] = EOS;
        StringCopy(gStringVar1, gSaveBlock2Ptr->playerName);   // (what a script would have put in the buffers)
        StringCopy(gStringVar2, gSaveBlock2Ptr->playerName);
        StringCopy(gStringVar3, gSaveBlock2Ptr->playerName);
        StringExpandPlaceholders(gStringVar4, sPlaceholder);
        printf("[test] s3dtext %d %d: %d characters\n", bank, msg, (int)StringLength(gStringVar4));
        ShowFieldMessage(sPlaceholder);
    }
    else if (strcmp(cmd, "orrebattle") == 0)
    {
        // orrebattle BATTLE [PERSON]: Colosseum's battle BATTLE now, as a script starts it (src/orre/orre_battle.c);
        // the foe's picture PERSON's (orre_people.c), the result logged after
        int battle = 0, person = 0xFFFF;
        sscanf(arg, "%d %d", &battle, &person);
        gSpecialVar_0x8004 = battle;
        gSpecialVar_0x8005 = person;
        Orre_StartBattle();
    }
    else if (strcmp(cmd, "orrescript") == 0)
    {
        // orrescript BANK FUNC: a function of a Colosseum script bank (0x100 the room's, 0x596 the shared one) run as
        // a scene now (src/orre/orre_vm.c)
        unsigned bank = 0x100, func = 0;
        u32 args[4] = {0};
        int n = sscanf(arg, "%x %u %u %u %u %u", &bank, &func, &args[0], &args[1], &args[2], &args[3]);
        // (orrescript BANK FUNC A0 A1 A2 A3: with its four arguments, as a talk script's (1, person, 0, 0))
        printf("[test] orrescript %x %u: %s\n", bank, func,
               Orre_Vm_Debug(bank, func, n > 2 ? args : NULL) ? "started" : "no such script");
    }
    else if (strcmp(cmd, "orrecam") == 0)
    {
        // orrecam NAME|ID [FRAME] | stop: a camera shot of Colosseum's played on this ORRE map ("S1_out/cam_s1_out_shot_01.cam"
        // or its resource id in hex: src/orre/o3d_cam.c)
        char name[128] = "";
        float frame = 0;
        unsigned id;
        sscanf(arg, "%127s %f", name, &frame);
        if (strcmp(name, "stop") == 0)
            O3D_CamStop(), printf("[test] orrecam stopped\n");
        else if (strchr(name, '/'))
            printf("[test] orrecam %s: %s\n", name, O3D_CamPlay(name, frame, FALSE) ? "playing" : "none");
        else if (sscanf(name, "%x", &id) == 1)
            printf("[test] orrecam %08X: %s\n", id, O3D_CamPlayId(id, frame, FALSE) ? "playing" : "none");
    }
    else if (strcmp(cmd, "orreresult") == 0)
        printf("[test] orre battle result %d\n", Orre_BattleResult());
    else if (strcmp(cmd, "orrevar") == 0)
    {
        // orrevar CTL WORDS: ORRE's string slot for Colosseum's control CTL (89 npc, 45 item, 75 money...), as a
        // converted script sets it (src/orre/orre_text.c)
        int ctl = 0, at = 0;
        u8 words[ORRE_STR_VAR_SIZE];
        sscanf(arg, "%d %n", &ctl, &at);
        MP_AsciiToGame(words, arg + at, ORRE_STR_VAR_SIZE - 1);
        Orre_SetStrVar(ctl, words);
    }
    else if (strcmp(cmd, "orretext") == 0)
    {
        // orretext ID: Colosseum's message ID from ORRE's text pack through the placeholder scripts use, in a message
        // box; the log has it as text
        static u8 sPlaceholder[6];
        static u8 sExpanded[1000];   // (not gStringVar4: the box prints from it)
        char ascii[1000];
        u32 id = strtoul(arg, NULL, 0);
        if (!IsFieldMessageBoxHidden())
            HideFieldMessageBox();   // (the last one's box)
        Orre_TextPlaceholder(sPlaceholder, id);
        StringExpandPlaceholders(sExpanded, sPlaceholder);
        MP_GameToAscii(ascii, sExpanded, sizeof(ascii) - 1);
        printf("[test] orretext %u (%s, %u lines in the pack): %s\n", id, Orre_HasText(id) ? "found" : "missing",
               Orre_TextCount(), ascii);
        ShowFieldMessage(sPlaceholder);
    }
    else if (strcmp(cmd, "collisionmap") == 0)
    {
        // collisionmap X Y W H: the map grid's walls in a rectangle ('#' wall, '.' open), a row a line
        int x0 = 0, y0 = 0, w = 0, h = 0, x, y;
        sscanf(arg, "%d %d %d %d", &x0, &y0, &w, &h);
        for (y = y0; y < y0 + h && y < y0 + 64; y++)
        {
            char row[80];
            for (x = 0; x < w && x < 78; x++)
                row[x] = MapGridGetCollisionAt(x0 + x + MAP_OFFSET, y + MAP_OFFSET) ? '#' : '.';
            row[x] = 0;
            printf("[test] collisionmap %3d: %s\n", y, row);
        }
    }
    else if (strcmp(cmd, "canstep") == 0)
    {
        // canstep: the player's collision result each way (0 none; 1 outside range, 2 impassable, 3 elevation, 4 object)
        struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
        s16 cx = pl->currentCoords.x, cy = pl->currentCoords.y;
        printf("[test] canstep: down %d up %d left %d right %d (elevation %d, tile %d)\n",
               GetCollisionAtCoords(pl, cx, cy + 1, DIR_SOUTH), GetCollisionAtCoords(pl, cx, cy - 1, DIR_NORTH),
               GetCollisionAtCoords(pl, cx - 1, cy, DIR_WEST), GetCollisionAtCoords(pl, cx + 1, cy, DIR_EAST), pl->currentElevation,
               MapGridGetElevationAt(pl->currentCoords.x, pl->currentCoords.y));
    }
    else if (strcmp(cmd, "collision") == 0)
    {
        // collision X Y: the map grid's collision and metatile behavior at map tile (X, Y)
        int x = 0, y = 0;
        sscanf(arg, "%d %d", &x, &y);
        printf("[test] collision %d %d: %d behavior %d\n", x, y, MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET),
               MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET));
    }
    else if (strcmp(cmd, "mapname") == 0)
    {
        // mapname: this map's section and its name as the game shows it (GetMapName)
        u8 name[40], ascii[40];
        u32 k;
        GetMapName(name, gMapHeader.regionMapSectionId, 0);
        for (k = 0; k < sizeof(ascii) - 1 && name[k] != EOS; k++)
            ascii[k] = name[k] == CHAR_SPACE ? ' ' : (name[k] >= CHAR_A && name[k] <= CHAR_Z) ? 'A' + name[k] - CHAR_A : '?';
        ascii[k] = 0;
        printf("[test] mapname 0x%X %s\n", (unsigned)gMapHeader.regionMapSectionId, ascii);
    }
    else if (strcmp(cmd, "orrecheck") == 0 || strcmp(cmd, "orredownload") == 0 || strcmp(cmd, "orrestate") == 0)
    {
        // ORRE's packs (updater.c sOrre): orrecheck asks the release what it has, orredownload fetches and installs
        // them when it has newer ones, orrestate prints how it's going (UPDATER state: 2 available, 3 up to date,
        // 4 none, 7 done, 8 failed; bytes or files done / total; why it failed)
        void Updater_OrreCheck(void);
        void Updater_OrreDownload(void);
        int Updater_OrreState(long *done, long *total, const char **error);
        long done, total;
        const char *err;
        int st;
        if (cmd[4] == 'c')
            Updater_OrreCheck();
        else if (cmd[4] == 'd')
            Updater_OrreDownload();
        st = Updater_OrreState(&done, &total, &err);
        printf("[test] orre state %d %ld/%ld %s\n", st, done, total, err ? err : "");
    }
    else if (strcmp(cmd, "shadow") == 0)
        Orre_Shadow_Debug(arg);   // shadow make|drop|steps|purify|table ... (src/orre/orre_shadow.c)
    else if (strcmp(cmd, "orreworldmap") == 0)
    {
        // orreworldmap: Colosseum's world map, as a place's exit opens it (src/orre/worldmap.c)
        extern const u8 Orre_EventScript_WorldMap[];
        ScriptContext_SetupScript(Orre_EventScript_WorldMap);
        printf("[test] orre world map opened\n");
    }
    else if (strcmp(cmd, "orreflag") == 0)
    {
        // orreflag N [VALUE]: Colosseum's story flag N (hex with 0x) set to 1, or VALUE (src/orre/orre_story.c)
        char *on;
        unsigned long flag = strtoul(arg, &on, 0);
        Orre_SetFlagValue(flag, *on ? strtoul(on, NULL, 0) : 1);
        printf("[test] orreflag 0x%lX %u (width %u)\n", flag, (unsigned)Orre_FlagValue(flag), Orre_FlagWidth(flag));
    }
    else if (strcmp(cmd, "orreget") == 0)   // orreget N: Colosseum's story flag N's value
        printf("[test] orreget 0x%lX %u\n", strtoul(arg, NULL, 0), (unsigned)Orre_FlagValue(strtoul(arg, NULL, 0)));
    else if (strcmp(cmd, "orreorder") == 0)
    {
        void Orre_StoryPrintOrder(void);
        Orre_StoryPrintOrder();   // orreorder: Colosseum's story flags in their order
    }
    else if (strcmp(cmd, "orrestory") == 0)
    {
        // orrestory N: Colosseum's story jump to flag N (the flags before it in the story's order set)
        Orre_StoryJump(strtoul(arg, NULL, 0));
    }
    else if (strcmp(cmd, "s3dground") == 0)
    {
        // s3dground X Y [NEAR]: SINNOH's ground height (BDHC) at map pixel (X, Y) of this map, the layer nearest NEAR
        int x = 0, y = 0;
        float near = 0.0f;
        sscanf(arg, "%d %d %f", &x, &y, &near);
        printf("[test] s3dground %d %d: %.2f\n", x, y, S3D_TestGround(x, y, near));
    }
    else if (strcmp(cmd, "wildroll") == 0)
    {
        // wildroll N: N land Pokémon from this map's table, as wild encounters would pick (species list)
        bool8 MP_RollExtraLandMon(struct Pokemon *out);
        struct Pokemon mon;
        int i, k = 0, n = atoi(arg);
        static char sLine[8 * 2000];
        k = snprintf(sLine, sizeof(sLine), "[test] wildroll:");   // (one line: the clock's own log lines come between rolls)
        for (i = 0; i < n; i++)
            if (MP_RollExtraLandMon(&mon) && k < (int)sizeof(sLine) - 8)
                k += snprintf(sLine + k, sizeof(sLine) - k, " %d", (int)GetMonData(&mon, MON_DATA_SPECIES));
        printf("%s\n", sLine);
        if (n > 0)
            printf("[test] wildroll met game %d location %d\n", (int)GetMonData(&mon, MON_DATA_MET_GAME), (int)GetMonData(&mon, MON_DATA_MET_LOCATION));
    }
    else if (strcmp(cmd, "gencheck") == 0)
    {
        // gencheck ITEM: the cap, each party Pokémon's SEALED state and held item, and how many ITEM the bag has
        int i, item = atoi(arg);
        printf("[test] gencheck: cap %d sealed", MP_Gen_Cap(MP_Region_Current()));
        for (i = 0; i < PARTY_SIZE && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES); i++)
            printf(" %d", MP_Gen_SealedMon(&gPlayerParty[i]));
        printf(" held");
        for (i = 0; i < PARTY_SIZE && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES); i++)
            printf(" %d", (int)GetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM));
        printf(" bag %d usable %d\n", (int)CountTotalItemQuantityInBag(item), MP_Gen_ItemAllowed(item));
    }
    else if (strcmp(cmd, "genrefresh") == 0)
    {
        // genrefresh: as when a map loads (held items past the GENERATION go back to the bag)
        MP_Gen_ReturnHeldItems();
    }
    else if (strcmp(cmd, "gensetting") == 0)
    {
        // gensetting N: the GENERATION setting (3 = vanilla: HOENN Gen 1-3, JOHTO / KANTO Gen 1-2)
        gMpGenSetting = atoi(arg);
    }
    else if (strcmp(cmd, "tripok") == 0)
    {
        // tripok - whether this game may lead the way to another region (online: the room's host only)
        printf("[test] trip allowed %d\n", MP_RegionTripAllowed());
    }
    else if (strcmp(cmd, "regionitems") == 0)
    {
        // regionitems - the key items' sorting run now (as on a map load); each region's stash
        MP_RegionItems_Update();
        printf("[test] region items stashes %u %u %u %u\n", MP_RegionItems_StashCount(0), MP_RegionItems_StashCount(1),
               MP_RegionItems_StashCount(2), MP_RegionItems_StashCount(3));
    }
    else if (strcmp(cmd, "travel") == 0)
    {
        // travel R: go to region R now, as a trip does (customs keeps the first POKeMON)
        bool8 MP_Region_FollowHost(u8 to);
        printf("[test] travel %d: %d\n", atoi(arg), MP_Region_FollowHost(atoi(arg)));
    }
    else if (strcmp(cmd, "s3dhour") == 0)
    {
        // s3dhour H - SINNOH's lighting hour from now on (MOEMON_S3D_HOUR), on the map the player is on
        char v[8];
        snprintf(v, sizeof(v), "%d", atoi(arg));
#ifdef _WIN32
        {
            char e[24];
            snprintf(e, sizeof(e), "MOEMON_S3D_HOUR=%s", v);
            _putenv(e);
        }
#else
        setenv("MOEMON_S3D_HOUR", v, 1);
#endif
        printf("[test] s3d hour %s\n", v);
    }
    else if (strcmp(cmd, "daytint") == 0)
    {
        // daytint - Platinum's time-of-day tint outside SINNOH (mp_daytint.c)
        void MP_DayTint_Debug(void);
        MP_DayTint_Debug();
    }
    else if (strcmp(cmd, "mombank") == 0)
    {
        // mombank [SAVINGS] - MOM's savings and state (mp_mombank.c); a number sets her savings
        void MP_MomBank_Debug(const char *arg);
        MP_MomBank_Debug(arg);
    }
    else if (strcmp(cmd, "amity") == 0)
    {
        // amity [STEPS] - SINNOH's AMITY SQUARE: is the follower its walker, its steps (s3d_misc.c); STEPS sets them
        void Sinnoh_AmityDebug(const char *arg);
        Sinnoh_AmityDebug(arg);
    }
    else if (strcmp(cmd, "pokegear") == 0)
    {
        // pokegear [open | tune FREQ] - JOHTO's POKéGEAR: its card, cursor, phone and radio (mp_pokegear.c); open: from
        // the field; tune: the RADIO card at FREQ (half MHz: 9 = 4.5)
        void MP_Pokegear_Debug(const char *arg);
        MP_Pokegear_Debug(arg);
    }
    else if (strcmp(cmd, "qzphone") == 0)   // qzphone register|ready|fights|steps|call N [V] - JOHTO's phone (mp_qzphone.c)
    {
        void MP_QzPhone_Debug(const char *arg);
        MP_QzPhone_Debug(arg);
    }
    else if (strcmp(cmd, "apricorn") == 0)
    {
        // apricorn - JOHTO's fruit trees picked today, KURT's BALLS, the BERRY patches planted (mp_apricorn.c)
        void MP_Apricorn_Debug(void);
        MP_Apricorn_Debug();
    }
    else if (strcmp(cmd, "rivalname") == 0)
    {
        // rivalname [open] - JOHTO's rival's name (qz_specials.c); open: the naming at ELM's and the officer's line
        extern const u8 EventScript_MpNameJohtoRival[];
        const u8 *MP_JohtoRivalName(void);
        char buf[16];
        if (strcmp(arg, "open") == 0)
            ScriptContext_SetupScript(EventScript_MpNameJohtoRival);
        MP_GameToAscii(buf, MP_JohtoRivalName(), sizeof(buf) - 1);
        printf("[test] rival name: %s\n", buf);
    }
    else if (strcmp(cmd, "regionstarted") == 0)
    {
        // regionstarted R: as if that region had been visited (its first visit's guide is over)
        gRegionState.started |= 1 << atoi(arg);
    }
    else if (strcmp(cmd, "regionclear") == 0)
    {
        // regionclear R: as if that region's Elite Four were beaten (0 HOENN: FLAG_SYS_GAME_CLEAR too)
        int r = atoi(arg);
        void MP_HmRescue_GiveAll(void);
        MP_Region_SetCleared(r);
        if (r == 0)
            FlagSet(FLAG_SYS_GAME_CLEAR);
        MP_HmRescue_GiveAll();   // (a cleared region's player has had its HMs: no HM rescue in tests that skip the story)
    }
    else if (strcmp(cmd, "party") == 0)
    {
        int i;
        printf("[test] party:");
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES))
                printf(" %d(%d/%d ball %d style %04x fx %04x/%d friend %d%s)", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_HP), (int)GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_POKEBALL), (int)GetMonData(&gPlayerParty[i], MON_DATA_ENCRYPT_SEPARATOR),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_MP_BALLFX), (int)GetMonData(&gPlayerParty[i], MON_DATA_MP_BALLFX_SOUND),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP),
                       GetMonData(&gPlayerParty[i], MON_DATA_SANITY_IS_BAD_EGG) ? " BAD EGG" : "");
            else
                printf(" 0");
        printf("  pc box1 slot1 %d\n", (int)GetBoxMonDataAt(0, 0, MON_DATA_SPECIES));
    }
    else if (strcmp(cmd, "monid") == 0)   // mp monid SLOT: what shows of a party Moemon's personality (ASTRAL BALL tests)
    {
        struct Pokemon *mon = &gPlayerParty[atoi(arg) % PARTY_SIZE];
        u32 p = GetMonData(mon, MON_DATA_PERSONALITY);
        printf("[test] mon %d: species %d personality %08x nature %d gender %d shiny %d style %04x bad %d\n", atoi(arg) % PARTY_SIZE,
               (int)GetMonData(mon, MON_DATA_SPECIES), (unsigned)p, (int)GetNature(mon), (int)GetMonGender(mon),
               (int)IsMonShiny(mon), (int)GetMonData(mon, MON_DATA_ENCRYPT_SEPARATOR), (int)GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG));
    }
    else if (strcmp(cmd, "badges") == 0)
    {
        int b;
        printf("[test] badges:");
        for (b = 0; b < 8; b++)
            printf(" %d", FlagGet(FLAG_BADGE01_GET + b));
        printf("  money %u\n", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
    }
    else if (strcmp(cmd, "sweepnote") == 0)
    {
        // sweepnote WORDS: a label in the log (tools/test/npc_sweep.sh, story_sweep.sh) and the scripts started since
        // the last sweepmark
        extern u32 gMpScriptStarts;
        printf("[test] sweepnote %s starts %u\n", arg, (unsigned)gMpScriptStarts);
    }
    else if (strcmp(cmd, "sweepmark") == 0)
    {
        extern u32 gMpScriptStarts;
        gMpScriptStarts = 0;
        printf("[test] sweepmark\n");
    }
    else if (strcmp(cmd, "ctx") == 0)
    {
        extern int Platform_InputContext(void);
        extern const u8 *MP_ScriptCurrentPtr(void);
        extern const u8 *const gStdScripts[];
        printf("[test] ctx %d free %d overworld %d locked %d script %d fade %d\n", Platform_InputContext(), MP_IsLocalFree(),
               MP_IsInOverworld(), ArePlayerFieldControlsLocked(), ScriptContext_IsEnabled(), gPaletteFade.active);
        // the script's place as an offset from gStdScripts (nm the game for the label: tools/test/script_at.sh)
        if (ScriptContext_IsEnabled() && MP_ScriptCurrentPtr())
            printf("[test] ctx script at gStdScripts%+ld\n", (long)((const u8 *)MP_ScriptCurrentPtr() - (const u8 *)gStdScripts));
        {
            // (what a script waits on: the text printers going, the native wait)
            extern void *MP_ScriptNativePtr(void);
            extern bool16 IsTextPrinterActive(u8 id);
            int w;
            printf("[test] ctx printers:");
            for (w = 0; w < 32; w++)
                if (IsTextPrinterActive(w))
                    printf(" %d", w);
            printf(" native %p\n", MP_ScriptNativePtr());
        }
    }
    else if (strcmp(cmd, "event") == 0)
    {
        // event trainer|coop|wild|catch LEVEL|trade|gift [count]
        char what[16] = "";
        int n = 1, level = 10, i;
        sscanf(arg, "%15s %d %d", what, &n, &level);
        for (i = 0; i < n; i++)
        {
            if (strcmp(what, "trainer") == 0) MP_Events_OnTrainerWin();
            else if (strcmp(what, "coop") == 0) MP_Events_OnCoopWin((u8)level);
            else if (strcmp(what, "wild") == 0) MP_Events_OnWildEnd(B_OUTCOME_WON, &gEnemyParty[0]);
            else if (strcmp(what, "catch") == 0) MP_Events_OnWildEnd(B_OUTCOME_CAUGHT, &gPlayerParty[0]);
            else if (strcmp(what, "trade") == 0) MP_Events_OnTrade();
            else if (strcmp(what, "gift") == 0) MP_Events_OnGiftSent();
        }
    }
    else if (strcmp(cmd, "savedata") == 0)
    {
        // savedata KEY VALUE (KEY may contain TODAY, replaced by today's date)
        char key[64] = "", out[64];
        int v = 0;
        char *t;
        sscanf(arg, "%63s %d", key, &v);
        if ((t = strstr(key, "TODAY")))
        {
            *t = 0;
            snprintf(out, sizeof(out), "%s%d%s", key, (int)MP_Today(), t + 5);
        }
        else
            snprintf(out, sizeof(out), "%s", key);
        MP_SaveData_Set(out, v);
        printf("[test] savedata %s = %d\n", out, v);
    }
    else if (strcmp(cmd, "goals") == 0)
    {
        void MP_Events_DebugGoals(void);
        MP_Events_DebugGoals();
    }
    else if (strcmp(cmd, "outbreak") == 0)
    {
        int i, hits = 0;
        for (i = 0; i < 1000; i++)
            hits += MP_Events_MaybeOutbreak(SPECIES_ZIGZAGOON) != SPECIES_ZIGZAGOON;
        printf("[test] outbreak hits %d/1000 here\n", hits);
    }
    else if (strcmp(cmd, "battleflag") == 0)
    {
        MP_Trainer_Toggle();
    }
    else if (strcmp(cmd, "useflag") == 0)
    {
        // useflag - use the BATTLE FLAG from the field ("Set phrases, or plant?"); only on the field, like the item
        if (MP_IsInOverworld())
            MP_Trainer_UseFlag(CreateTask(TaskDummy, 80));
        else
            printf("[test] useflag: not on the field\n");
    }
    else if (strcmp(cmd, "easychat") == 0)
    {
        // easychat TYPE - the phrase builder the way the game's scripts open it (special ShowEasyChatScreen)
        void ShowEasyChatScreen(void);
        gSpecialVar_0x8004 = atoi(arg);
        ShowEasyChatScreen();
    }
    else if (strcmp(cmd, "phrasedefault") == 0)
    {
        // phrasedefault - the new game's phrases (profile, battle phrases; mail words cleared)
        void InitEasyChatPhrases(void);
        InitEasyChatPhrases();
    }
    else if (strcmp(cmd, "phraseclear") == 0)
    {
        // phraseclear N - empties a battle phrase (0 CHALLENGE, 1 WIN, 2 LOSE)
        int n = atoi(arg), i;
        u16 *w = n == 0 ? gSaveBlock1Ptr->easyChatBattleStart : n == 1 ? gSaveBlock1Ptr->easyChatBattleWon
                                                                        : gSaveBlock1Ptr->easyChatBattleLost;
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            w[i] = 0xFFFF;
        printf("[test] phrase %d cleared\n", n);
    }
    else if (strcmp(cmd, "phrases") == 0)
    {
        // phrases - the three battle phrases' words
        int i;
        printf("[test] phrases:");
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            printf(" %04x", gSaveBlock1Ptr->easyChatBattleStart[i]);
        printf(" |");
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            printf(" %04x", gSaveBlock1Ptr->easyChatBattleWon[i]);
        printf(" |");
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            printf(" %04x", gSaveBlock1Ptr->easyChatBattleLost[i]);
        printf("\n");
    }
    else if (strcmp(cmd, "objat") == 0)
    {
        // objat X Y: the object standing on a map cell (local id, graphics), or none
        int x = 0, y = 0;
        u8 id;
        sscanf(arg, "%d %d", &x, &y);
        id = GetObjectEventIdByXY(x + MAP_OFFSET, y + MAP_OFFSET);
        if (id < OBJECT_EVENTS_COUNT)
            printf("[test] objat %d,%d: local %d gfx %d\n", x, y, gObjectEvents[id].localId, OBJ_GFX_ID(&gObjectEvents[id]));
        else
            printf("[test] objat %d,%d: none\n", x, y);
    }
    else if (strcmp(cmd, "findbehavior") == 0)
    {
        // findbehavior N: up to 5 cells of this map with metatile behavior N (MB_*)
        int x, y, n = 0, want = atoi(arg);
        for (y = 0; y < gMapHeader.mapLayout->height && n < 5; y++)
            for (x = 0; x < gMapHeader.mapLayout->width && n < 5; x++)
                if ((int)MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET) == want)
                    printf("[test] behavior %d at %d,%d\n", want, x, y), n++;
    }
    else if (strcmp(cmd, "shorespots") == 0)
    {
        // shorespots: up to 5 cells on this map that stand two tiles north of reflecting water with dry, walkable
        // ground between (reflection tests: the reflection shows only over the water)
        int x, y, n = 0;
        for (y = 0; y < gMapHeader.mapLayout->height - 2 && n < 5; y++)
            for (x = 0; x < gMapHeader.mapLayout->width && n < 5; x++)
            {
                u32 b0 = MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET);
                u32 b1 = MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + 1 + MAP_OFFSET);
                u32 b2 = MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + 2 + MAP_OFFSET);
                if (!MetatileBehavior_IsReflective(b0) && !MetatileBehavior_IsReflective(b1) && MetatileBehavior_IsReflective(b2)
                 && !MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET) && !MapGridGetCollisionAt(x + MAP_OFFSET, y + 1 + MAP_OFFSET))
                    printf("[test] shorespot %d,%d\n", x, y), n++;
            }
    }
    else if (strcmp(cmd, "objlist") == 0)
    {
        // objlist: every object on the field (local id, graphics, cell, hidden, its sprite's state and palette)
        int i;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        {
            struct ObjectEvent *o = &gObjectEvents[i];
            struct Sprite *sp;
            if (!o->active)
                continue;
            sp = o->spriteId < MAX_SPRITES ? &gSprites[o->spriteId] : NULL;
            printf("[test] obj %d local %d gfx %d at %d,%d move %d inv %d offscreen %d sprite %d inUse %d sinv %d pal %d xy %d,%d refl %d\n",
                   i, o->localId, OBJ_GFX_ID(o), o->currentCoords.x - MAP_OFFSET, o->currentCoords.y - MAP_OFFSET, o->movementType,
                   o->invisible, o->offScreen, o->spriteId, sp ? sp->inUse : 0, sp ? sp->invisible : 0, sp ? sp->oam.paletteNum : -1,
                   sp ? sp->x : 0, sp ? sp->y : 0, o->hasReflection);
        }
    }
    else if (strcmp(cmd, "steptrigger") == 0)
    {
        // steptrigger X Y: as if the player stepped onto that cell: its step trigger runs if its variable holds the value
        int x = 0, y = 0, i;
        const struct CoordEvent *ev = gMapHeader.events->coordEvents;
        sscanf(arg, "%d %d", &x, &y);
        for (i = 0; i < gMapHeader.events->coordEventCount; i++)
            if (ev[i].x == x && ev[i].y == y && ev[i].script != NULL && ev[i].trigger != TRIGGER_RUN_IMMEDIATELY
             && (ev[i].trigger == 0 || VarGet(ev[i].trigger) == ev[i].index))
            {
                ScriptContext_SetupScript(ev[i].script);
                printf("[test] steptrigger %d,%d: runs\n", x, y);
                break;
            }
        if (i == gMapHeader.events->coordEventCount)
            printf("[test] steptrigger %d,%d: none\n", x, y);
    }
    else if (strcmp(cmd, "elevation") == 0)
    {
        // elevation N: the player's height (a step trigger only fires at its own: tools/test/story_sweep.sh)
        struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
        me->currentElevation = me->previousElevation = atoi(arg);
        printf("[test] elevation %d\n", atoi(arg));
    }
    else if (strcmp(cmd, "boulderswitch") == 0)
    {
        // boulderswitch X Y: as if a pushed boulder stopped on that cell (FireRed's switches)
        int x = 0, y = 0;
        sscanf(arg, "%d %d", &x, &y);
        TryRunBoulderSwitchScript(x, y);
    }
    else if (strcmp(cmd, "npcs") == 0)
    {
        // npcs: the map's people as they are now: local id, look, place, hidden by the game, off-screen hidden
        int i;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        {
            struct ObjectEvent *o = &gObjectEvents[i];
            extern s16 gS3dSpriteFeet[MAX_SPRITES];
            if (!o->active)
                continue;
            // (and where its sprite is drawn: OAM x, y and, on a SINNOH map, its feet's screen row)
            printf("[test] npc local %d gfx %d xy %d,%d invisible %d offscreen %d oam %d,%d feet %d%s\n", o->localId,
                   OBJ_GFX_ID(o), o->currentCoords.x - MAP_OFFSET, o->currentCoords.y - MAP_OFFSET, o->invisible,
                   o->offScreen, gSprites[o->spriteId].oam.x, gSprites[o->spriteId].oam.y, gS3dSpriteFeet[o->spriteId],
                   o->isPlayer ? " player" : "");
        }
    }
    else if (strcmp(cmd, "objs") == 0)
    {
        void MP_Overworld_DebugDump(void);
        MP_Overworld_DebugDump();
        {
            void MP_Follower_DebugDump(void);
            MP_Follower_DebugDump();
        }
    }
    else if (strcmp(cmd, "drop") == 0)
    {
        MP_OnFileDropped(arg);
    }
    fflush(stdout);
}
