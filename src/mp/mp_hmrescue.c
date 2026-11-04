// Moemon Co-op: HMs a friend's progress made impossible to get. Four of HOENN's HMs are handed over in a scene that
// happens once, by someone who then leaves: STRENGTH in RUSTURF TUNNEL (Wanda's boyfriend, when the last boulder is
// smashed), FLY on ROUTE 119 (the rival, after their battle), DIVE in MOSSDEEP (STEVEN, before he leaves his house),
// WATERFALL in SOOTOPOLIS (WALLACE, before the last gym). Online, friends can move the story on while a player is
// away (a smashed boulder or a beaten trainer is the room's), so a player can come back to a story past the scene
// with the HM never handed over. Only then (the scene is over for good, the HM was never received, and it's nowhere
// in the BAG, the PC or held by a POKéMON) the one who would have given it walks up and hands it over, as the
// LOTTERY CORNER clerk does with TMs (mp_tmrefund.c). Every other HM's giver stays where they are.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_message_box.h"
#include "fieldmap.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#include "constants/songs.h"
#include "constants/vars.h"

#define LOCALID_COURIER 0xDC

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

enum { OWED_STRENGTH, OWED_FLY, OWED_DIVE, OWED_WATERFALL, OWED_COUNT };

struct OwedHm
{
    u16 item, flag;
    u16 giverMapGroup, giverMapNum;   // (the scene's own map: never there, the scene may still happen)
    const char *what, *hello;
};

static const struct OwedHm sOwed[OWED_COUNT] = {
    [OWED_STRENGTH] = {ITEM_HM_STRENGTH, FLAG_RECEIVED_HM_STRENGTH, MAP_GROUP(MAP_RUSTURF_TUNNEL), MAP_NUM(MAP_RUSTURF_TUNNEL), "HM04 STRENGTH",
                       "Hey, there you are! Remember me? From\nRUSTURF TUNNEL, with WANDA?\f"
                       "Your friends smashed the last boulder\nwhile you were away, and I never got\nto thank you all!\f"
                       "I had this for you. You'll need it\nfor the big boulders out there!"},
    [OWED_FLY] = {ITEM_HM_FLY, FLAG_RECEIVED_HM_FLY, MAP_GROUP(MAP_ROUTE119), MAP_NUM(MAP_ROUTE119), "HM02 FLY",
                  "@! There you are!\f"
                  "After our battle on ROUTE 119 you\nwere off before I could give you this!\f"
                  "Everyone should be able to FLY. Here!"},
    [OWED_DIVE] = {ITEM_HM_DIVE, FLAG_RECEIVED_HM_DIVE, MAP_GROUP(MAP_MOSSDEEP_CITY_STEVENS_HOUSE), MAP_NUM(MAP_MOSSDEEP_CITY_STEVENS_HOUSE), "HM08 DIVE",
                    "@, I'm glad I found you.\f"
                    "I meant to give you this at my house\nin MOSSDEEP, but you and I kept\nmissing each other.\f"
                    "The sea floor is waiting for you."},
    [OWED_WATERFALL] = {ITEM_HM_WATERFALL, FLAG_RECEIVED_HM_WATERFALL, MAP_GROUP(MAP_SOOTOPOLIS_CITY), MAP_NUM(MAP_SOOTOPOLIS_CITY), "HM07 WATERFALL",
                        "@, a moment of your time.\f"
                        "In all the commotion in SOOTOPOLIS,\nthis never reached you.\f"
                        "You'll need it on the way to the\nPOKeMON LEAGUE. Take it, please."},
};

static EWRAM_DATA struct
{
    u8 state, objId, steps, walked, dir;
    u8 which, slow;
    u16 timer, check;
} sC = {0};

// Is the HM anywhere the player keeps things: the BAG, the PC, held by a POKéMON in the party or the boxes?
static bool8 Carried(u16 item)
{
    struct PokemonStorage *st = MP_PersonalStorage();
    u16 b, i;
    if (CheckBagHasItem(item, 1) || CheckPCHasItem(item, 1))
        return TRUE;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM) == item)
            return TRUE;
    if (st)
        for (b = 0; b < TOTAL_BOXES_COUNT; b++)
            for (i = 0; i < IN_BOX_COUNT; i++)
                if (GetBoxMonData(&st->boxes[b][i], MON_DATA_SPECIES) != SPECIES_NONE
                 && GetBoxMonData(&st->boxes[b][i], MON_DATA_HELD_ITEM) == item)
                    return TRUE;
    return FALSE;
}

// The scene is over for good (its giver has left)
static bool8 SceneOver(u8 which)
{
    switch (which)
    {
    case OWED_STRENGTH:
        return VarGet(VAR_RUSTURF_TUNNEL_STATE) >= 6;   // the tunnel open: Wanda and her boyfriend went home
    case OWED_FLY:
        return HasTrainerBeenFought(TRAINER_BRENDAN_ROUTE_119_MUDKIP) || HasTrainerBeenFought(TRAINER_BRENDAN_ROUTE_119_TREECKO)
            || HasTrainerBeenFought(TRAINER_BRENDAN_ROUTE_119_TORCHIC) || HasTrainerBeenFought(TRAINER_MAY_ROUTE_119_MUDKIP)
            || HasTrainerBeenFought(TRAINER_MAY_ROUTE_119_TREECKO) || HasTrainerBeenFought(TRAINER_MAY_ROUTE_119_TORCHIC);
    case OWED_DIVE:
        return FlagGet(FLAG_HIDE_MOSSDEEP_CITY_STEVENS_HOUSE_STEVEN) && FlagGet(FLAG_BADGE07_GET);
    case OWED_WATERFALL:
        return FlagGet(FLAG_BADGE08_GET);   // past SOOTOPOLIS's gym: WALLACE is long gone
    }
    return FALSE;
}

static bool8 Owed(u8 which)
{
    const struct OwedHm *o = &sOwed[which];
    return !FlagGet(o->flag) && SceneOver(which) && !Carried(o->item);
}

static u16 CourierGfx(u8 which)
{
    switch (which)
    {
    case OWED_STRENGTH:  return OBJ_EVENT_GFX_BLACK_BELT;
    case OWED_FLY:       return gSaveBlock2Ptr->playerGender == MALE ? OBJ_EVENT_GFX_RIVAL_MAY_NORMAL : OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL;
    case OWED_DIVE:      return OBJ_EVENT_GFX_STEVEN;
    default:             return OBJ_EVENT_GFX_WALLACE;
    }
}

static bool8 TileFree(s16 x, s16 y, u8 elevation)
{
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if (MapGridGetElevationAt(x, y) != elevation && MapGridGetElevationAt(x, y) != 0 && elevation != 0)
        return FALSE;
    return TRUE;
}

static void Say(const char *ascii)
{
    static u8 sText[480];
    char buf[480], name[16];
    const char *p;
    int n = 0;
    MP_GameToAscii(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    for (p = ascii; *p && n < (int)sizeof(buf) - 16; p++)   // "@": the player's name
    {
        if (*p == '@')
            n += snprintf(buf + n, sizeof(buf) - n, "%s", name);
        else
            buf[n++] = *p;
    }
    buf[n] = 0;
    MP_AsciiToGame(sText, buf, sizeof(sText) - 1);
    MP_ShowFieldMessageFitted(sText);
}

static void Give(void)
{
    const struct OwedHm *o = &sOwed[sC.which];
    AddBagItem(o->item, 1);
    FlagSet(o->flag);
    MPNet_Log("hms: %s handed over (its scene was over before the player saw it)", o->what);
}

static void Task_Courier(u8 taskId)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *courier = sC.objId < OBJECT_EVENTS_COUNT ? &gObjectEvents[sC.objId] : NULL;
    static const s8 sDx[] = {0, 0, 0, -1, 1}, sDy[] = {0, 1, -1, 0, 0};   // DIR_SOUTH, NORTH, WEST, EAST
    char text[160];

    switch (sC.state)
    {
    case 0:
    {
        // from the side with the longest free line (up to 4 tiles), south first
        static const u8 sOrder[] = {DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH};
        s16 px = player->currentCoords.x, py = player->currentCoords.y;
        u8 i, best = 0, bestLen = 0;
        for (i = 0; i < ARRAY_COUNT(sOrder); i++)
        {
            u8 d = sOrder[i], len = 0;
            while (len < 4 && TileFree(px + sDx[d] * (len + 1), py + sDy[d] * (len + 1), player->currentElevation))
                len++;
            if (len > bestLen)
                bestLen = len, best = d;
        }
        sC.objId = OBJECT_EVENTS_COUNT;
        if (bestLen > 0)
            sC.objId = SpawnSpecialObjectEventParameterized(CourierGfx(sC.which), MOVEMENT_TYPE_NONE, LOCALID_COURIER,
                                                            px + sDx[best] * bestLen, py + sDy[best] * bestLen,
                                                            player->currentElevation);
        if (sC.objId >= OBJECT_EVENTS_COUNT)
        {
            UnlockPlayerFieldControls();   // (nowhere to stand: next time, somewhere roomier)
            DestroyTask(taskId);
            break;
        }
        sC.dir = (best == DIR_SOUTH) ? DIR_NORTH : (best == DIR_NORTH) ? DIR_SOUTH : (best == DIR_WEST) ? DIR_EAST : DIR_WEST;
        sC.steps = bestLen - 1;
        sC.walked = 0;
        SetObjectEventDirection(&gObjectEvents[sC.objId], sC.dir);
        ObjectEventTurn(player, best);
        MPNet_Log("hms: %s comes, %d tiles away", sOwed[sC.which].what, bestLen);
        PlaySE(SE_PIN);
        sC.timer = 0;
        sC.state++;
        break;
    }
    case 1:
        if (++sC.timer < 20)
            break;
        sC.state++;
        // fallthrough
    case 2:
        if (ObjectEventIsHeldMovementActive(courier) && !ObjectEventClearHeldMovementIfFinished(courier))
            break;
        if (sC.walked < sC.steps)
        {
            ObjectEventSetHeldMovement(courier, GetWalkNormalMovementAction(sC.dir));
            sC.walked++;
            break;
        }
        Say(sOwed[sC.which].hello);
        sC.state = 3;
        break;
    case 3:
        if (IsTextPrinterActive(0) || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        Give();
        PlayFanfare(MUS_OBTAIN_TMHM);
        snprintf(text, sizeof(text), "@ received %s!", sOwed[sC.which].what);
        Say(text);
        sC.state = 4;
        break;
    case 4:
        if (IsTextPrinterActive(0) || !IsFanfareTaskInactive() || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        sC.walked = 0;
        sC.state = 5;
        break;
    case 5:
        // back the way they came, then gone
        if (ObjectEventIsHeldMovementActive(courier) && !ObjectEventClearHeldMovementIfFinished(courier))
            break;
        if (sC.walked < sC.steps + 1)
        {
            u8 away = (sC.dir == DIR_NORTH) ? DIR_SOUTH : (sC.dir == DIR_SOUTH) ? DIR_NORTH
                    : (sC.dir == DIR_WEST) ? DIR_EAST : DIR_WEST;
            ObjectEventSetHeldMovement(courier, GetWalkNormalMovementAction(away));
            sC.walked++;
            break;
        }
        RemoveObjectEventByLocalIdAndMap(LOCALID_COURIER, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
        break;
    }
}

// RUSTURF TUNNEL: a friend smashed a boulder while we were on the map (mp_world.c sets the rock's flag, so it's gone
// for us for good), but only the smasher's EventScript_SmashRock runs TryUpdateRusturfTunnelState: our tunnel stayed
// shut in the story, Wanda's boyfriend kept waiting and STRENGTH could never come. Once our own story is there (the
// AQUA grunt's scene done: state 3) and we're in sight of the spot, it catches up: the map's ON_FRAME scene plays
// (state 4 / 5), as if we had smashed it, and the boyfriend hands STRENGTH over.
static void TunnelCatchUp(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 TryUpdateRusturfTunnelState(void);
    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_RUSTURF_TUNNEL) || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_RUSTURF_TUNNEL))
        return;
    if (FlagGet(FLAG_RUSTURF_TUNNEL_OPENED) || VarGet(VAR_RUSTURF_TUNNEL_STATE) != 3)
        return;
    if (!FlagGet(FLAG_HIDE_RUSTURF_TUNNEL_ROCK_1) && !FlagGet(FLAG_HIDE_RUSTURF_TUNNEL_ROCK_2))
        return;
    if (abs(player->currentCoords.x - (24 + MAP_OFFSET)) > 7 || abs(player->currentCoords.y - (5 + MAP_OFFSET)) > 5)
        return;   // (the scene plays in sight of the boulders, where its people stand)
    if (TryUpdateRusturfTunnelState())
        MPNet_Log("hms: RUSTURF TUNNEL's boulder went with a friend: the tunnel scene plays (state %d)", VarGet(VAR_RUSTURF_TUNNEL_STATE));
}

// Every frame (MP_FrameUpdate): now and then, on HOENN's maps, free on the map, not on the scene's own map
void MP_HmRescue_Update(void)
{
    u8 i;
    if (gMain.inBattle || FuncIsActiveTask(Task_Courier) || !MP_IsInOverworld() || !MP_IsLocalFree())
        return;
    if (++sC.check < 30)
        return;
    sC.check = 0;
    if (MP_Region_Current() != REGION_HOENN)
        return;
    TunnelCatchUp();
    if (++sC.slow < 4)
        return;   // (the couriers: every 2 seconds)
    sC.slow = 0;
    for (i = 0; i < OWED_COUNT; i++)
    {
        if (gSaveBlock1Ptr->location.mapGroup == sOwed[i].giverMapGroup && gSaveBlock1Ptr->location.mapNum == sOwed[i].giverMapNum)
            continue;
        if (!Owed(i))
            continue;
        memset(&sC, 0, sizeof(sC));
        sC.which = i;
        LockPlayerFieldControls();
        CreateTask(Task_Courier, 80);
        return;
    }
}

// The harness: "mp givebadges" (HOENN's eight) also counts every owed HM as given, as a player with the badges has them
void MP_HmRescue_GiveAll(void)
{
    u8 i;
    for (i = 0; i < OWED_COUNT; i++)
        FlagSet(sOwed[i].flag);
}

// The harness: "mp hmrescue" lists what's owed
void MP_HmRescue_Debug(void)
{
    u8 i;
    printf("[test] hmrescue:");
    for (i = 0; i < OWED_COUNT; i++)
        printf(" %s=%s", sOwed[i].what, Owed(i) ? "owed" : FlagGet(sOwed[i].flag) ? "got" : SceneOver(i) ? "carried" : "not yet");
    printf("\n");
}
