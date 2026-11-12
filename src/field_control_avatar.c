#include "global.h"
#include "u3d.h"   // Moemon Kai: UNOVA (trainers' eyes)
#include "orre_shadow.h"   // Moemon Kai: Shadow Pokémon
#include "constants/metatile_behaviors.h"
#include "s3d_underground.h"
#include "s3d.h"   // Moemon Kai
#include "mp_region.h"
#include "mp.h"
#include "mp_garden.h"   // Moemon Co-op
#include "battle_setup.h"
#include "bike.h"
#include "coord_event_weather.h"
#include "daycare.h"
#include "faraway_island.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_control_avatar.h"
#include "field_player_avatar.h"
#include "field_poison.h"
#include "field_screen_effect.h"
#include "field_specials.h"
#include "fldeff_misc.h"
#include "item_menu.h"
#include "link.h"
#include "match_call.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "pokemon.h"
#include "safari_zone.h"
#include "script.h"
#include "secret_base.h"
#include "sound.h"
#include "start_menu.h"
#include "trainer_see.h"
#include "trainer_hill.h"
#include "wild_encounter.h"
#include "constants/event_bg.h"
#include "constants/event_objects.h"
#include "constants/field_poison.h"
#include "constants/map_types.h"
#include "constants/songs.h"
#include "constants/trainer_hill.h"
#include "o3d.h"   // Moemon Kai: ORRE's people
#include "orre.h"

static EWRAM_DATA u8 sWildEncounterImmunitySteps = 0;
static EWRAM_DATA u16 sPrevMetatileBehavior = 0;

COMMON_DATA u8 gSelectedObjectEvent = 0;

static void GetPlayerPosition(struct MapPosition *);
static void GetInFrontOfPlayerPosition(struct MapPosition *);
static u16 GetPlayerCurMetatileBehavior(int);
static bool8 TryStartInteractionScript(struct MapPosition *, u16, u8);
static const u8 *GetInteractionScript(struct MapPosition *, u8, u8);
static const u8 *GetInteractedObjectEventScript(struct MapPosition *, u8, u8);
static const u8 *GetInteractedBackgroundEventScript(struct MapPosition *, u8, u8);
static const u8 *GetInteractedMetatileScript(struct MapPosition *, u8, u8);
extern const u8 EventScript_QzHeadbuttTree[];
static const u8 *GetInteractedWaterScript(struct MapPosition *, u8, u8);
static bool32 TrySetupDiveDownScript(void);
static bool32 TrySetupDiveEmergeScript(void);
static bool8 TryStartStepBasedScript(struct MapPosition *, u16, u16);
static bool8 CheckStandardWildEncounter(u16);
static bool8 TryArrowWarp(struct MapPosition *, u16, u8);
static bool8 IsWarpMetatileBehavior(u16);
static bool8 IsArrowWarpMetatileBehavior(u16, u8);
static s8 GetWarpEventAtMapPosition(struct MapHeader *, struct MapPosition *);
static void SetupWarp(struct MapHeader *, s8, struct MapPosition *);
static bool8 TryDoorWarp(struct MapPosition *, u16, u8);
static bool8 TryOrreWideDoor(struct MapPosition *);
static s8 GetWarpEventAtPosition(struct MapHeader *, u16, u16, u8);
static const u8 *GetCoordEventScriptAtPosition(struct MapHeader *, u16, u16, u8);
static const struct BgEvent *GetBackgroundEventAtPosition(struct MapHeader *, u16, u16, u8);
static bool8 TryStartCoordEventScript(struct MapPosition *);
static bool8 TryStartWarpEventScript(struct MapPosition *, u16);
static bool8 TryStartMiscWalkingScripts(u16);
static bool8 TryStartStepCountScript(u16);
static void UpdateFriendshipStepCounter(void);
static bool8 UpdatePoisonStepCounter(void);

void FieldClearPlayerInput(struct FieldInput *input)
{
    input->pressedAButton = FALSE;
    input->checkStandardWildEncounter = FALSE;
    input->pressedStartButton = FALSE;
    input->pressedSelectButton = FALSE;
    input->heldDirection = FALSE;
    input->heldDirection2 = FALSE;
    input->tookStep = FALSE;
    input->pressedBButton = FALSE;
    input->input_field_1_0 = FALSE;
    input->input_field_1_1 = FALSE;
    input->input_field_1_2 = FALSE;
    input->input_field_1_3 = FALSE;
    input->dpadDirection = 0;
    if (gMpItemXPressed)
        gMpItemXPressed--;   // a press mid-step still counts at the next tile
}

#define MP_SELECT_HOLD_FRAMES 20
static u8 sMpSelectHeld;
u8 gMpItemXPressed;   // Moemon Co-op QoL: pad west / F key (input_context.c), kept a few frames

void FieldGetPlayerInput(struct FieldInput *input, u16 newKeys, u16 heldKeys)
{
    u8 tileTransitionState = gPlayerAvatar.tileTransitionState;
    u8 runningState = gPlayerAvatar.runningState;
    bool8 forcedMove = MetatileBehavior_IsForcedMovementTile(GetPlayerCurMetatileBehavior(runningState));

    if ((tileTransitionState == T_TILE_CENTER && forcedMove == FALSE) || tileTransitionState == T_NOT_MOVING)
    {
        if (GetPlayerSpeed() != PLAYER_SPEED_FASTEST)
        {
            if (newKeys & START_BUTTON)
                input->pressedStartButton = TRUE;
            // Moemon Co-op QoL: a tap of SELECT uses the registered item (on
            // release), holding it opens the follower picker
            if ((heldKeys & SELECT_BUTTON) && sMpSelectHeld < 255)
            {
                if (++sMpSelectHeld == MP_SELECT_HOLD_FRAMES)
                    input->input_field_1_2 = TRUE;
            }
            else if (!(heldKeys & SELECT_BUTTON))
            {
                if (sMpSelectHeld > 0 && sMpSelectHeld < MP_SELECT_HOLD_FRAMES)
                    input->pressedSelectButton = TRUE;
                sMpSelectHeld = 0;
            }
            // Moemon Co-op QoL: the extra item button (pad west / F key)
            if (gMpItemXPressed)
            {
                input->input_field_1_3 = TRUE;
                gMpItemXPressed = 0;
            }
            // Moemon Co-op QoL: key items registered to L / R
            if (gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_NORMAL)
            {
                if (newKeys & L_BUTTON)
                    input->input_field_1_0 = TRUE;
                if (newKeys & R_BUTTON)
                    input->input_field_1_1 = TRUE;
            }
            if (newKeys & A_BUTTON)
                input->pressedAButton = TRUE;
            if (newKeys & B_BUTTON)
                input->pressedBButton = TRUE;
        }

        if (heldKeys & (DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT))
        {
            input->heldDirection = TRUE;
            input->heldDirection2 = TRUE;
        }
    }

    if (forcedMove == FALSE)
    {
        if (tileTransitionState == T_TILE_CENTER && runningState == MOVING)
            input->tookStep = TRUE;
        if (forcedMove == FALSE && tileTransitionState == T_TILE_CENTER)
            input->checkStandardWildEncounter = TRUE;
    }

    if (heldKeys & DPAD_UP)
        input->dpadDirection = DIR_NORTH;
    else if (heldKeys & DPAD_DOWN)
        input->dpadDirection = DIR_SOUTH;
    else if (heldKeys & DPAD_LEFT)
        input->dpadDirection = DIR_WEST;
    else if (heldKeys & DPAD_RIGHT)
        input->dpadDirection = DIR_EAST;
}

int ProcessPlayerFieldInput(struct FieldInput *input)
{
    struct MapPosition position;
    u8 playerDirection;
    u16 metatileBehavior;

    gSpecialVar_LastTalked = LOCALID_NONE;
    gSelectedObjectEvent = 0;

    playerDirection = GetPlayerFacingDirection();
    GetPlayerPosition(&position);
    metatileBehavior = MapGridGetMetatileBehaviorAt(position.x, position.y);

    if (CheckForTrainersWantingBattle() == TRUE)
        return TRUE;
    if (U3D_Vm_TrainerSight())   // Moemon Kai: UNOVA's trainers see as Black's (src/unova/u3d_vm.c)
        return TRUE;

    if (TryRunOnFrameMapScript() == TRUE)
        return TRUE;
    {
        bool8 Maker_TryAutorun(void);
        if (Maker_TryAutorun())   // Moemon Maker: an event's autorun page (src/maker/maker_overrides.c)
            return TRUE;
    }

    // Moemon Co-op: carrying a decoration in the MOéMON GARDEN: A puts it down in front, B puts it back
    if (input->pressedBButton && TrySetupDiveEmergeScript() == TRUE)
        return TRUE;
    if (input->tookStep)
    {
        IncrementGameStat(GAME_STAT_STEPS);
        IncrementBirthIslandRockStepCount();
        {
            void MP_ResortGorgeousStep(void);
            void MP_KantoMassageStep(void);
            MP_ResortGorgeousStep();   // Moemon Kai: FireRed's SELPHY wish timer (qz_specials.c)
            MP_KantoMassageStep();     // and DAISY's grooming
        }
        {
            u8 MakerLua_OnStep(void);
            if (MakerLua_OnStep())   // Moemon Maker M5: a Lua OnStep hook started an event (src/maker/maker_lua.cpp)
                return TRUE;
        }
        if (TryStartStepBasedScript(&position, metatileBehavior, playerDirection) == TRUE)
            return TRUE;
    }
    if (input->checkStandardWildEncounter && CheckStandardWildEncounter(metatileBehavior) == TRUE)
        return TRUE;
    if (input->heldDirection && input->dpadDirection == playerDirection)
    {
        if (TryArrowWarp(&position, metatileBehavior, playerDirection) == TRUE)
            return TRUE;
    }

    GetInFrontOfPlayerPosition(&position);
    metatileBehavior = MapGridGetMetatileBehaviorAt(position.x, position.y);
    if (input->pressedAButton)
        MP_Underground_TryTalk();   // Moemon Kai: SPIRITOMB's counter, a friend talked to in SINNOH's Underground
    // Moemon Kai: RUI between the player and their POKeMON: A held lets it past (src/orre/orre_npcs.c)
    if (input->pressedAButton && O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup) && Orre_Npc_TryFollowerPast())
        return TRUE;
    if (input->pressedAButton && TryStartInteractionScript(&position, metatileBehavior, playerDirection) == TRUE)
        return TRUE;

    if (input->heldDirection2 && input->dpadDirection == playerDirection)
    {
        if (TryDoorWarp(&position, metatileBehavior, playerDirection) == TRUE)
            return TRUE;
        if (TryOrreWideDoor(&position) == TRUE)
            return TRUE;
    }
    if (input->pressedAButton && TrySetupDiveDownScript() == TRUE)
        return TRUE;
    if (input->pressedStartButton)
    {
        PlaySE(SE_WIN_OPEN);
        ShowStartMenu();
        return TRUE;
    }
    if (input->pressedSelectButton && UseRegisteredKeyItemOnField() == TRUE)
        return TRUE;
    if (input->input_field_1_2 && MP_Qol_OpenFollowerPicker() == TRUE)
        return TRUE;
    if (input->input_field_1_0 && MP_UseRegisteredItemSlot(0) == TRUE)
        return TRUE;
    if (input->input_field_1_1 && MP_UseRegisteredItemSlot(1) == TRUE)
        return TRUE;
    if (input->input_field_1_3 && MP_UseRegisteredItemSlot(2) == TRUE)
        return TRUE;

    return FALSE;
}

static void GetPlayerPosition(struct MapPosition *position)
{
    PlayerGetDestCoords(&position->x, &position->y);
    position->elevation = PlayerGetElevation();
}

static void GetInFrontOfPlayerPosition(struct MapPosition *position)
{
    s16 x, y;

    GetXYCoordsOneStepInFrontOfPlayer(&position->x, &position->y);
    PlayerGetDestCoords(&x, &y);
    if (MapGridGetElevationAt(x, y) != ELEVATION_TRANSITION)
        position->elevation = PlayerGetElevation();
    else
        position->elevation = ELEVATION_TRANSITION;
}

static u16 GetPlayerCurMetatileBehavior(int runningState)
{
    s16 x, y;

    PlayerGetDestCoords(&x, &y);
    return MapGridGetMetatileBehaviorAt(x, y);
}

static bool8 TryStartInteractionScript(struct MapPosition *position, u16 metatileBehavior, u8 direction)
{
    const u8 *script = GetInteractionScript(position, metatileBehavior, direction);
    if (script == NULL)
        return FALSE;

    // Don't play interaction sound for certain scripts.
    if (script != LittlerootTown_BrendansHouse_2F_EventScript_PC
     && script != LittlerootTown_MaysHouse_2F_EventScript_PC
     && script != SecretBase_EventScript_PC
     && script != SecretBase_EventScript_RecordMixingPC
     && script != SecretBase_EventScript_DollInteract
     && script != SecretBase_EventScript_CushionInteract
     && script != EventScript_PC)
        PlaySE(SE_SELECT);

    ScriptContext_SetupScript(script);
    return TRUE;
}

static const u8 *GetInteractionScript(struct MapPosition *position, u8 metatileBehavior, u8 direction)
{
    const u8 *script = GetInteractedObjectEventScript(position, metatileBehavior, direction);
    if (script != NULL)
        return script;

    // Moemon Kai: a check spot of Colosseum's in front of the player (a PC, a sign: src/orre/orre_npcs.c)
    if (O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup) && (script = Orre_Check_InFront()) != NULL)
        return script;

    script = GetInteractedBackgroundEventScript(position, metatileBehavior, direction);
    if (script != NULL)
    {
        MP_Qol_SignStarted();   // Moemon Co-op QoL: signs print instantly
        MP_Social_OnSignRead(position->x, position->y);   // Moemon Co-op: friends' scribbles
        return script;
    }

    script = GetInteractedMetatileScript(position, metatileBehavior, direction);
    if (script != NULL)
        return script;

    script = GetInteractedWaterScript(position, metatileBehavior, direction);
    if (script != NULL)
        return script;

    return NULL;
}

const u8 *GetInteractedLinkPlayerScript(struct MapPosition *position, u8 metatileBehavior, u8 direction)
{
    u8 objectEventId;
    s32 i;

    if (!MetatileBehavior_IsCounter(MapGridGetMetatileBehaviorAt(position->x, position->y)))
        objectEventId = GetObjectEventIdByPosition(position->x, position->y, position->elevation);
    else
        objectEventId = GetObjectEventIdByPosition(position->x + gDirectionToVectors[direction].x, position->y + gDirectionToVectors[direction].y, position->elevation);

    if (objectEventId == OBJECT_EVENTS_COUNT || gObjectEvents[objectEventId].localId == LOCALID_PLAYER)
        return NULL;

    for (i = 0; i < 4; i++)
    {
        if (gLinkPlayerObjectEvents[i].active == TRUE && gLinkPlayerObjectEvents[i].objEventId == objectEventId)
            return NULL;
    }

    // Moemon Co-op: a trainer another player is holding while they wait for a partner
    if (MP_Coop_IsNpcHeld(&gObjectEvents[objectEventId]))
    {
        MP_Coop_OnHeldNpcTalk();
        return NULL;
    }

    gSelectedObjectEvent = objectEventId;
    gSpecialVar_LastTalked = gObjectEvents[objectEventId].localId;
    gSpecialVar_Facing = direction;
    return GetObjectEventScriptPointerByObjectEventId(objectEventId);
}

static const u8 *GetInteractedObjectEventScript(struct MapPosition *position, u8 metatileBehavior, u8 direction)
{
    u8 objectEventId;
    const u8 *script;

    objectEventId = GetObjectEventIdByPosition(position->x, position->y, position->elevation);
    if (objectEventId == OBJECT_EVENTS_COUNT || gObjectEvents[objectEventId].localId == LOCALID_PLAYER)
    {
        // Moemon Kai: ORRE's maps have no counters: someone behind a blocked tile is talked to across it, as
        // Colosseum reaches people a little way off (people.c peopleFindTalkTarget)
        if (MetatileBehavior_IsCounter(metatileBehavior) != TRUE
         && !(O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup) && MapGridGetCollisionAt(position->x, position->y)))
            return NULL;
        // (... but a shut door or gate with a check spot of its own answers first: PYRITE's hideout bookshelf has its
        // button, and the lookout kid two tiles beyond it took every A from behind it)
        if (O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup)
         && Orre_Vm_GateShutAt(position->x - MAP_OFFSET, position->y - MAP_OFFSET) && Orre_Check_SpotInFront())
            return NULL;

        // Look for an object event on the other side of the counter.
        objectEventId = GetObjectEventIdByPosition(position->x + gDirectionToVectors[direction].x, position->y + gDirectionToVectors[direction].y, position->elevation);
        if (objectEventId == OBJECT_EVENTS_COUNT || gObjectEvents[objectEventId].localId == LOCALID_PLAYER)
            return NULL;
    }

    gSelectedObjectEvent = objectEventId;
    gSpecialVar_LastTalked = gObjectEvents[objectEventId].localId;
    gSpecialVar_Facing = direction;

    // Moemon Kai: an ORRE person: their own talk script, Colosseum's (src/orre/orre_npcs.c)
    if (O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup)
     && (script = Orre_Npc_TalkScript(gObjectEvents[objectEventId].localId)) != NULL)
        return script;

    if (InTrainerHill() == TRUE)
        script = GetTrainerHillTrainerScript();
    else
        script = GetObjectEventScriptPointerByObjectEventId(objectEventId);

    script = GetRamScript(gSpecialVar_LastTalked, script);
    script = MP_Rematch_ScriptForObject(script); // Moemon Co-op: team rematches with gym leaders
    return script;
}

static const u8 *GetInteractedBackgroundEventScript(struct MapPosition *position, u8 metatileBehavior, u8 direction)
{
    const struct BgEvent *bgEvent = GetBackgroundEventAtPosition(&gMapHeader, position->x - MAP_OFFSET, position->y - MAP_OFFSET, position->elevation);

    if (bgEvent == NULL)
        return NULL;
    if (bgEvent->bgUnion.script == NULL)
        return EventScript_TestSignpostMsg;

    switch (bgEvent->kind)
    {
    case BG_EVENT_PLAYER_FACING_ANY:
    default:
        return bgEvent->bgUnion.script;
    case BG_EVENT_PLAYER_FACING_NORTH:
        if (direction != DIR_NORTH)
            return NULL;
        break;
    case BG_EVENT_PLAYER_FACING_SOUTH:
        if (direction != DIR_SOUTH)
            return NULL;
        break;
    case BG_EVENT_PLAYER_FACING_EAST:
        if (direction != DIR_EAST)
            return NULL;
        break;
    case BG_EVENT_PLAYER_FACING_WEST:
        if (direction != DIR_WEST)
            return NULL;
        break;
    case 5:
    case 6:
    case BG_EVENT_HIDDEN_ITEM:
        gSpecialVar_0x8004 = ((u32)bgEvent->bgUnion.script >> 16) + FLAG_HIDDEN_ITEMS_START;
        gSpecialVar_0x8005 = (u32)bgEvent->bgUnion.script;
        if (FlagGet(gSpecialVar_0x8004) == TRUE)
            return NULL;
        return EventScript_HiddenItemScript;
    case BG_EVENT_SECRET_BASE:
        if (direction == DIR_NORTH)
        {
            gSpecialVar_0x8004 = bgEvent->bgUnion.secretBaseId;
            if (TrySetCurSecretBase())
                return SecretBase_EventScript_CheckEntrance;
        }
        return NULL;
    }

    return bgEvent->bgUnion.script;
}

static const u8 *GetInteractedMetatileScript(struct MapPosition *position, u8 metatileBehavior, u8 direction)
{
    s8 elevation;

    {
        const u8 *S3D_Gyms_VillaScript(s16 x, s16 y, u8 direction);
        const u8 *villa = S3D_Gyms_VillaScript(position->x, position->y, direction);
        if (villa)
            return villa;   // Moemon Kai: the VILLA's furniture (src/sinnoh/s3d_gyms.c)
    }
    if ((metatileBehavior == MB_UNUSED_4B || metatileBehavior == MB_UNUSED_4C)
     && S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup))
    {
        extern const u8 EventScript_SinnohRockClimb[];
        return EventScript_SinnohRockClimb;   // Moemon Kai: SINNOH's rock walls (src/sinnoh/s3d_misc.c S3D_RockClimb*)
    }
    if (MetatileBehavior_IsPlayerFacingTVScreen(metatileBehavior, direction) == TRUE)
    {
        extern const u8 gSinnohTvBroadcast[];
        if (S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup))
            return gSinnohTvBroadcast;   // Moemon Kai: SINNOH's TVs play Platinum's broadcast (src/sinnoh/s3d_tv.c)
        return EventScript_TV;
    }
    if (MP_Headbutt_IsTree(position->x, position->y))   // Moemon Kai: JOHTO / KANTO's trees
        return EventScript_QzHeadbuttTree;
    {
        // Moemon Kai: a dig spot in SINNOH's Underground (src/sinnoh/s3d_underground.c)
        const u8 *dig = Sinnoh_UndergroundDigScript(position->x - MAP_OFFSET, position->y - MAP_OFFSET);
        if (dig != NULL)
            return dig;
    }
    if (MetatileBehavior_IsPC(metatileBehavior) == TRUE)
        return EventScript_PC;
    if (MetatileBehavior_IsClosedSootopolisDoor(metatileBehavior) == TRUE)
        return EventScript_ClosedSootopolisDoor;
    if (MetatileBehavior_IsSkyPillarClosedDoor(metatileBehavior) == TRUE)
        return SkyPillar_Outside_EventScript_ClosedDoor;
    if (MetatileBehavior_IsCableBoxResults1(metatileBehavior) == TRUE)
        return EventScript_CableBoxResults;
    if (MetatileBehavior_IsPokeblockFeeder(metatileBehavior) == TRUE)
        return EventScript_PokeBlockFeeder;
    if (MetatileBehavior_IsTrickHousePuzzleDoor(metatileBehavior) == TRUE)
        return Route110_TrickHousePuzzle_EventScript_Door;
    if (MetatileBehavior_IsRegionMap(metatileBehavior) == TRUE)
        return EventScript_RegionMap;
    if (MetatileBehavior_IsRunningShoesManual(metatileBehavior) == TRUE)
        return EventScript_RunningShoesManual;
    if (MetatileBehavior_IsPictureBookShelf(metatileBehavior) == TRUE)
        return EventScript_PictureBookShelf;
    if (MetatileBehavior_IsBookShelf(metatileBehavior) == TRUE)
        return EventScript_BookShelf;
    if (MetatileBehavior_IsPokeCenterBookShelf(metatileBehavior) == TRUE)
        return EventScript_PokemonCenterBookShelf;
    if (MetatileBehavior_IsVase(metatileBehavior) == TRUE)
        return EventScript_Vase;
    if (MetatileBehavior_IsTrashCan(metatileBehavior) == TRUE)
        return EventScript_EmptyTrashCan;
    if (MetatileBehavior_IsShopShelf(metatileBehavior) == TRUE)
        return EventScript_ShopShelf;
    if (MetatileBehavior_IsBlueprint(metatileBehavior) == TRUE)
        return EventScript_Blueprint;
    if (MetatileBehavior_IsPlayerFacingWirelessBoxResults(metatileBehavior, direction) == TRUE)
        return EventScript_WirelessBoxResults;
    if (MetatileBehavior_IsCableBoxResults2(metatileBehavior, direction) == TRUE)
        return EventScript_CableBoxResults;
    if (MetatileBehavior_IsQuestionnaire(metatileBehavior) == TRUE)
        return EventScript_Questionnaire;
    if (MetatileBehavior_IsTrainerHillTimer(metatileBehavior) == TRUE)
        return EventScript_TrainerHillTimer;

    elevation = position->elevation;
    if (elevation == MapGridGetElevationAt(position->x, position->y))
    {
        if (MetatileBehavior_IsSecretBasePC(metatileBehavior) == TRUE)
            return SecretBase_EventScript_PC;
        if (MetatileBehavior_IsRecordMixingSecretBasePC(metatileBehavior) == TRUE)
            return SecretBase_EventScript_RecordMixingPC;
        if (MetatileBehavior_IsSecretBaseSandOrnament(metatileBehavior) == TRUE)
            return SecretBase_EventScript_SandOrnament;
        if (MetatileBehavior_IsSecretBaseShieldOrToyTV(metatileBehavior) == TRUE)
            return SecretBase_EventScript_ShieldOrToyTV;
        if (MetatileBehavior_IsSecretBaseDecorationBase(metatileBehavior) == TRUE)
        {
            CheckInteractedWithFriendsFurnitureBottom();
            return NULL;
        }
        if (MetatileBehavior_HoldsLargeDecoration(metatileBehavior) == TRUE)
        {
            CheckInteractedWithFriendsFurnitureMiddle();
            return NULL;
        }
        if (MetatileBehavior_HoldsSmallDecoration(metatileBehavior) == TRUE)
        {
            CheckInteractedWithFriendsFurnitureTop();
            return NULL;
        }
    }
    else if (MetatileBehavior_IsSecretBasePoster(metatileBehavior) == TRUE)
    {
        CheckInteractedWithFriendsPosterDecor();
        return NULL;
    }

    return NULL;
}

static const u8 *GetInteractedWaterScript(struct MapPosition *unused1, u8 metatileBehavior, u8 direction)
{
    extern const u8 Sinnoh_EventScript_DwSurf[];
    if (S3D_Dw_FacingWater())
        return Sinnoh_EventScript_DwSurf;   // Moemon Kai: SURF on a DISTORTION WORLD platform (src/sinnoh/s3d_distortion.c)
    if (MP_Region_HasFieldMoveBadge(4) == TRUE && PartyHasMonWithSurf() == TRUE && IsPlayerFacingSurfableFishableWater() == TRUE)
        return EventScript_UseSurf;

    if (MetatileBehavior_IsWaterfall(metatileBehavior) == TRUE)
    {
        if (MP_Region_HasFieldMoveBadge(7) == TRUE && IsPlayerSurfingNorth() == TRUE)
            return EventScript_UseWaterfall;
        else
            return EventScript_CannotUseWaterfall;
    }
    return NULL;
}

static bool32 TrySetupDiveDownScript(void)
{
    if (MP_Region_HasFieldMoveBadge(6) && TrySetDiveWarp() == 2)
    {
        ScriptContext_SetupScript(EventScript_UseDive);
        return TRUE;
    }
    return FALSE;
}

static bool32 TrySetupDiveEmergeScript(void)
{
    if (MP_Region_HasFieldMoveBadge(6) && gMapHeader.mapType == MAP_TYPE_UNDERWATER && TrySetDiveWarp() == 1)
    {
        ScriptContext_SetupScript(EventScript_UseDiveUnderwater);
        return TRUE;
    }
    return FALSE;
}

static bool8 TryStartStepBasedScript(struct MapPosition *position, u16 metatileBehavior, u16 direction)
{
    if (TryStartCoordEventScript(position) == TRUE)
        return TRUE;
    if (TryStartWarpEventScript(position, metatileBehavior) == TRUE)
        return TRUE;
    if (TryStartMiscWalkingScripts(metatileBehavior) == TRUE)
        return TRUE;
    if (TryStartStepCountScript(metatileBehavior) == TRUE)
        return TRUE;
    if (UpdateRepelCounter() == TRUE)
        return TRUE;
    return FALSE;
}

static bool8 TryStartCoordEventScript(struct MapPosition *position)
{
    const u8 *script = GetCoordEventScriptAtPosition(&gMapHeader, position->x - MAP_OFFSET, position->y - MAP_OFFSET, position->elevation);

    if (script == NULL)
        return FALSE;
    ScriptContext_SetupScript(script);
    return TRUE;
}

static bool8 TryStartMiscWalkingScripts(u16 metatileBehavior)
{
    s16 x, y;

    if (MetatileBehavior_IsCrackedFloorHole(metatileBehavior))
    {
        ScriptContext_SetupScript(EventScript_FallDownHole);
        return TRUE;
    }
    else if (MetatileBehavior_IsBattlePyramidWarp(metatileBehavior))
    {
        ScriptContext_SetupScript(BattlePyramid_WarpToNextFloor);
        return TRUE;
    }
    else if (MetatileBehavior_IsSecretBaseGlitterMat(metatileBehavior) == TRUE)
    {
        DoSecretBaseGlitterMatSparkle();
        return FALSE;
    }
    else if (MetatileBehavior_IsSecretBaseSoundMat(metatileBehavior) == TRUE)
    {
        PlayerGetDestCoords(&x, &y);
        PlaySecretBaseMusicNoteMatSound(MapGridGetMetatileIdAt(x, y));
        return FALSE;
    }
    return FALSE;
}

static bool8 TryStartStepCountScript(u16 metatileBehavior)
{
    if (InUnionRoom() == TRUE)
    {
        return FALSE;
    }

    IncrementRematchStepCounter();
    UpdateFriendshipStepCounter();
    UpdateFarawayIslandStepCounter();

    if (!(gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_FORCED_MOVE) && !MetatileBehavior_IsForcedMovementTile(metatileBehavior))
    {
        if (UpdatePoisonStepCounter() == TRUE)
        {
            ScriptContext_SetupScript(EventScript_FieldPoison);
            return TRUE;
        }
        if (ShouldEggHatch())
        {
            IncrementGameStat(GAME_STAT_HATCHED_EGGS);
            ScriptContext_SetupScript(EventScript_EggHatch);
            return TRUE;
        }
        if (AbnormalWeatherHasExpired() == TRUE)
        {
            ScriptContext_SetupScript(AbnormalWeather_EventScript_EndEventAndCleanup_1);
            return TRUE;
        }
        if (ShouldDoBrailleRegicePuzzle() == TRUE)
        {
            ScriptContext_SetupScript(IslandCave_EventScript_OpenRegiEntrance);
            return TRUE;
        }
        if (ShouldDoWallyCall() == TRUE)
        {
            ScriptContext_SetupScript(MauvilleCity_EventScript_RegisterWallyCall);
            return TRUE;
        }
        if (ShouldDoScottFortreeCall() == TRUE)
        {
            ScriptContext_SetupScript(Route119_EventScript_ScottWonAtFortreeGymCall);
            return TRUE;
        }
        if (ShouldDoScottBattleFrontierCall() == TRUE)
        {
            ScriptContext_SetupScript(LittlerootTown_ProfessorBirchsLab_EventScript_ScottAboardSSTidalCall);
            return TRUE;
        }
        if (ShouldDoRoxanneCall() == TRUE)
        {
            ScriptContext_SetupScript(RustboroCity_Gym_EventScript_RegisterRoxanne);
            return TRUE;
        }
        if (ShouldDoRivalRayquazaCall() == TRUE)
        {
            ScriptContext_SetupScript(MossdeepCity_SpaceCenter_2F_EventScript_RivalRayquazaCall);
            return TRUE;
        }
    }

    {
        void Sinnoh_MiscStep(void);
        Sinnoh_MiscStep();   // Moemon Kai: AMITY SQUARE's walker counts its steps (src/sinnoh/s3d_misc.c)
        Orre_Shadow_Step();   // Moemon Kai: Shadow Pokémon's hearts open as they walk (src/orre/orre_shadow.c)
    }
    if (SafariZoneTakeStep() == TRUE)
        return TRUE;
    if (CountSSTidalStep(1) == TRUE)
    {
        ScriptContext_SetupScript(SSTidalCorridor_EventScript_ReachedStepCount);
        return TRUE;
    }
    if (TryStartMatchCall())
        return TRUE;
    {
        // Moemon Kai: JOHTO's phone (src/mp/mp_qzphone.c): a registered contact calls now and then
        bool8 MP_QzPhone_TryCall(void);
        extern const u8 QzPhone_EventScript_IncomingCall[];
        if (MP_QzPhone_TryCall())
        {
            ScriptContext_SetupScript(QzPhone_EventScript_IncomingCall);
            return TRUE;
        }
    }
    return FALSE;
}

static void UNUSED ClearFriendshipStepCounter(void)
{
    VarSet(VAR_FRIENDSHIP_STEP_COUNTER, 0);
}

static void UpdateFriendshipStepCounter(void)
{
    u16 *ptr = GetVarPointer(VAR_FRIENDSHIP_STEP_COUNTER);
    int i;

    (*ptr)++;
    (*ptr) %= 128;
    if (*ptr == 0)
    {
        struct Pokemon *mon = gPlayerParty;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            AdjustFriendship(mon, FRIENDSHIP_EVENT_WALKING);
            mon++;
        }
        MP_Follower_OnWalkSteps();   // Moemon Co-op: the one walking with you gains more
    }
}

void ClearPoisonStepCounter(void)
{
    VarSet(VAR_POISON_STEP_COUNTER, 0);
}

static bool8 UpdatePoisonStepCounter(void)
{
    u16 *ptr;

    if (gMapHeader.mapType != MAP_TYPE_SECRET_BASE)
    {
        ptr = GetVarPointer(VAR_POISON_STEP_COUNTER);
        (*ptr)++;
        (*ptr) %= 4;
        if (*ptr == 0)
        {
            switch (DoPoisonFieldEffect())
            {
            case FLDPSN_NONE:
                return FALSE;
            case FLDPSN_PSN:
                return FALSE;
            case FLDPSN_FNT:
                return TRUE;
            }
        }
    }
    return FALSE;
}

void RestartWildEncounterImmunitySteps(void)
{
    // Starts at 0 and counts up to 4 steps.
    sWildEncounterImmunitySteps = 0;
}

static bool8 CheckStandardWildEncounter(u16 metatileBehavior)
{
    // Moemon Kai: the POKé RADAR's shaking grass (src/mp/mp_pokeradar.c): a step into a patch is its encounter (even
    // right after a battle: the chain goes on); while the patches shake no other grass has one
    switch (MP_PokeRadar_Step())
    {
    case 1:
        sWildEncounterImmunitySteps = 0;
        sPrevMetatileBehavior = metatileBehavior;
        return TRUE;
    case 2:
        sPrevMetatileBehavior = metatileBehavior;
        return FALSE;
    }
    if (sWildEncounterImmunitySteps < 4)
    {
        sWildEncounterImmunitySteps++;
        sPrevMetatileBehavior = metatileBehavior;
        return FALSE;
    }

    if (StandardWildEncounter(metatileBehavior, sPrevMetatileBehavior) == TRUE)
    {
        sWildEncounterImmunitySteps = 0;
        sPrevMetatileBehavior = metatileBehavior;
        return TRUE;
    }

    sPrevMetatileBehavior = metatileBehavior;
    return FALSE;
}

// Moemon Kai: SINNOH's cave mouths and gate entrances. Platinum puts the warp on the tile in front of the entrance and
// marks the entrance itself (WARP_ENTRANCE_EAST / WEST / NORTH: our east / west / north arrow warps, with no warp of its
// own): the player warps by pushing from the warp tile into the entrance. The pack made that front tile a step-on warp,
// so stepping onto it from any side warped a tile short of the mouth (the user, 2026-11-06). The entrance beside the
// player going DIR (0: none)
static bool8 SinnohEntranceToward(struct MapPosition *position, u8 dir)
{
    struct MapPosition at = *position;
    u16 behavior;
    if (!S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup))
        return FALSE;
    switch (dir)
    {
    case DIR_EAST:
        at.x++;
        break;
    case DIR_WEST:
        at.x--;
        break;
    case DIR_NORTH:
        at.y--;
        break;
    default:
        return FALSE;
    }
    behavior = MapGridGetMetatileBehaviorAt(at.x, at.y);
    if (MetatileBehavior_IsSinnohStairs(behavior) || GetWarpEventAtMapPosition(&gMapHeader, &at) != WARP_ID_NONE)
        return FALSE;
    return (dir == DIR_EAST && MetatileBehavior_IsEastArrowWarp(behavior))
        || (dir == DIR_WEST && MetatileBehavior_IsWestArrowWarp(behavior))
        || (dir == DIR_NORTH && MetatileBehavior_IsNorthArrowWarp(behavior));
}

static bool8 IsSinnohEntranceFront(struct MapPosition *position)
{
    return SinnohEntranceToward(position, DIR_EAST) || SinnohEntranceToward(position, DIR_WEST)
        || SinnohEntranceToward(position, DIR_NORTH);
}

// Moemon Kai: FireRed's exit mats inside KANTO / JOHTO are three warp tiles under a mat one tile wide (its edges drawn
// a few pixels onto the tiles beside it): a run of three or more same-way exits to one map leaves by its inner tiles
// only, so stepping off beside the mat no longer leaves (the user, 2026-11-11)
static bool8 IsWideMatEdge(struct MapPosition *position, u16 metatileBehavior, u8 direction)
{
    s32 sx = (direction == DIR_NORTH || direction == DIR_SOUTH), sy = !sx, side, n[2] = {0, 0};
    s8 id = GetWarpEventAtMapPosition(&gMapHeader, position);
    u8 region = MP_Region_Current();
    if (id == WARP_ID_NONE || (region != REGION_KANTO && region != REGION_JOHTO) || gMapHeader.mapType != MAP_TYPE_INDOOR)
        return FALSE;
    for (side = 0; side < 2; side++)
        for (n[side] = 0; n[side] < 8; n[side]++)
        {
            s32 d = (n[side] + 1) * (side ? 1 : -1);
            struct MapPosition at = {position->x + sx * d, position->y + sy * d, position->elevation};
            s8 other = GetWarpEventAtMapPosition(&gMapHeader, &at);
            if (other == WARP_ID_NONE || MapGridGetMetatileBehaviorAt(at.x, at.y) != metatileBehavior
             || gMapHeader.events->warps[other].mapGroup != gMapHeader.events->warps[id].mapGroup
             || gMapHeader.events->warps[other].mapNum != gMapHeader.events->warps[id].mapNum)
                break;
        }
    return n[0] + n[1] + 1 >= 3 && (n[0] == 0 || n[1] == 0);
}

// (the story runs, mp_debug.c: a warp tile that never leaves)
bool8 MP_ArrowWarpIsMatEdge(s16 x, s16 y)
{
    struct MapPosition at = {x + MAP_OFFSET, y + MAP_OFFSET, 0};
    u16 b = MapGridGetMetatileBehaviorAt(at.x, at.y);
    u8 dir = MetatileBehavior_IsSouthArrowWarp(b) ? DIR_SOUTH : MetatileBehavior_IsNorthArrowWarp(b) ? DIR_NORTH
           : MetatileBehavior_IsEastArrowWarp(b) ? DIR_EAST : MetatileBehavior_IsWestArrowWarp(b) ? DIR_WEST : DIR_NONE;
    s8 id;
    if (dir == DIR_NONE)
        return FALSE;
    id = GetWarpEventAtPosition(&gMapHeader, x, y, 0);
    if (id == WARP_ID_NONE)
        return FALSE;
    at.elevation = gMapHeader.events->warps[id].elevation;
    return IsWideMatEdge(&at, b, dir);
}

static bool8 TryArrowWarp(struct MapPosition *position, u16 metatileBehavior, u8 direction)
{
    s8 warpEventId = GetWarpEventAtMapPosition(&gMapHeader, position);

    if (warpEventId != WARP_ID_NONE && IsArrowWarpMetatileBehavior(metatileBehavior, direction) == TRUE
     && IsWideMatEdge(position, metatileBehavior, direction))
        return FALSE;

    if (warpEventId != WARP_ID_NONE && SinnohEntranceToward(position, direction))
    {
        StoreInitialPlayerAvatarState();   // Moemon Kai: pushed into a SINNOH cave mouth / gate entrance (above)
        SetupWarp(&gMapHeader, warpEventId, position);
        DoWarp();
        return TRUE;
    }
    if (IsArrowWarpMetatileBehavior(metatileBehavior, direction) == TRUE && warpEventId != WARP_ID_NONE)
    {
        StoreInitialPlayerAvatarState();
        SetupWarp(&gMapHeader, warpEventId, position);
        if (DoSinnohRideWarp(metatileBehavior))   // Moemon Kai: SINNOH's stairs, walked into (field_screen_effect.c)
            return TRUE;
        DoWarp();
        return TRUE;
    }
    return FALSE;
}

static bool8 TryStartWarpEventScript(struct MapPosition *position, u16 metatileBehavior)
{
    s8 warpEventId = GetWarpEventAtMapPosition(&gMapHeader, position);

    if (warpEventId != WARP_ID_NONE && IsWarpMetatileBehavior(metatileBehavior) == TRUE
     && !(MetatileBehavior_IsNonAnimDoor(metatileBehavior) && IsSinnohEntranceFront(position)))   // (Moemon Kai: above)
    {
        // Moemon Kai: a closed way (JOHTO's / KANTO's GAME CENTER has no ROOFTOP): a word, and the player steps back
        if (MP_Region_WarpClosed(gMapHeader.events->warps[warpEventId].mapGroup, gMapHeader.events->warps[warpEventId].mapNum))
        {
            extern const u8 EventScript_GameCenterRoofClosed[];
            ScriptContext_SetupScript(EventScript_GameCenterRoofClosed);
            return TRUE;
        }
        StoreInitialPlayerAvatarState();
        SetupWarp(&gMapHeader, warpEventId, position);
        if (DoSinnohRideWarp(metatileBehavior))   // Moemon Kai: SINNOH's stairs and escalators (field_screen_effect.c)
            return TRUE;
        if (MetatileBehavior_IsEscalator(metatileBehavior) == TRUE)
        {
            DoEscalatorWarp(metatileBehavior);
            return TRUE;
        }
        if (MetatileBehavior_IsLavaridgeB1FWarp(metatileBehavior) == TRUE)
        {
            DoLavaridgeGymB1FWarp();
            return TRUE;
        }
        if (MetatileBehavior_IsLavaridge1FWarp(metatileBehavior) == TRUE)
        {
            DoLavaridgeGym1FWarp();
            return TRUE;
        }
        if (MetatileBehavior_IsAquaHideoutWarp(metatileBehavior) == TRUE)
        {
            DoTeleportTileWarp();
            return TRUE;
        }
        if (MetatileBehavior_IsUnionRoomWarp(metatileBehavior) == TRUE)
        {
            DoSpinExitWarp();
            return TRUE;
        }
        if (MetatileBehavior_IsMtPyreHole(metatileBehavior) == TRUE)
        {
            ScriptContext_SetupScript(EventScript_FallDownHoleMtPyre);
            return TRUE;
        }
        if (MetatileBehavior_IsMossdeepGymWarp(metatileBehavior) == TRUE)
        {
            DoMossdeepGymWarp();
            return TRUE;
        }
        DoWarp();
        return TRUE;
    }
    return FALSE;
}

static bool8 IsWarpMetatileBehavior(u16 metatileBehavior)
{
    if (MetatileBehavior_IsWarpDoor(metatileBehavior) != TRUE
     && MetatileBehavior_IsLadder(metatileBehavior) != TRUE
     && MetatileBehavior_IsEscalator(metatileBehavior) != TRUE
     && MetatileBehavior_IsNonAnimDoor(metatileBehavior) != TRUE
     && MetatileBehavior_IsLavaridgeB1FWarp(metatileBehavior) != TRUE
     && MetatileBehavior_IsLavaridge1FWarp(metatileBehavior) != TRUE
     && MetatileBehavior_IsAquaHideoutWarp(metatileBehavior) != TRUE
     && MetatileBehavior_IsMtPyreHole(metatileBehavior) != TRUE
     && MetatileBehavior_IsMossdeepGymWarp(metatileBehavior) != TRUE
     && MetatileBehavior_IsUnionRoomWarp(metatileBehavior) != TRUE)
        return FALSE;
    return TRUE;
}

static bool8 IsArrowWarpMetatileBehavior(u16 metatileBehavior, u8 direction)
{
    switch (direction)
    {
    case DIR_NORTH:
        return MetatileBehavior_IsNorthArrowWarp(metatileBehavior);
    case DIR_SOUTH:
        return MetatileBehavior_IsSouthArrowWarp(metatileBehavior);
    case DIR_WEST:
        return MetatileBehavior_IsWestArrowWarp(metatileBehavior);
    case DIR_EAST:
        return MetatileBehavior_IsEastArrowWarp(metatileBehavior);
    }
    return FALSE;
}

static s8 GetWarpEventAtMapPosition(struct MapHeader *mapHeader, struct MapPosition *position)
{
    return GetWarpEventAtPosition(mapHeader, position->x - MAP_OFFSET, position->y - MAP_OFFSET, position->elevation);
}

static void SetupWarp(struct MapHeader *unused, s8 warpEventId, struct MapPosition *position)
{
    const struct WarpEvent *warpEvent;

    u8 trainerHillMapId = GetCurrentTrainerHillMapId();

    if (trainerHillMapId)
    {
        if (trainerHillMapId == GetNumFloorsInTrainerHillChallenge())
        {
            if (warpEventId == 0)
                warpEvent = &gMapHeader.events->warps[0];
            else
                warpEvent = SetWarpDestinationTrainerHill4F();
        }
        else if (trainerHillMapId == TRAINER_HILL_ROOF)
        {
            warpEvent = SetWarpDestinationTrainerHillFinalFloor(warpEventId);
        }
        else
        {
            warpEvent = &gMapHeader.events->warps[warpEventId];
        }
    }
    else
    {
        warpEvent = &gMapHeader.events->warps[warpEventId];
    }

    if (warpEvent->mapNum == MAP_NUM(MAP_DYNAMIC))
    {
        SetWarpDestinationToDynamicWarp(warpEvent->warpId);
    }
    else
    {
        const struct MapHeader *mapHeader;

        SetWarpDestinationToMapWarp(warpEvent->mapGroup, warpEvent->mapNum, warpEvent->warpId);
        UpdateEscapeWarp(position->x, position->y);
        mapHeader = Overworld_GetMapHeaderByGroupAndId(warpEvent->mapGroup, warpEvent->mapNum);
        if (mapHeader->events->warps[warpEvent->warpId].mapNum == MAP_NUM(MAP_DYNAMIC))
            SetDynamicWarp(mapHeader->events->warps[warpEventId].warpId, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, warpEventId);
        S3D_Gyms_RedirectWarp(position->x, position->y);   // Moemon Kai: HEARTHOME's gym: a wrong door goes back
        // Moemon Co-op: every POKéMON CENTER's escalator goes up to the MOéMON GARDEN, whose escalator goes back down
        MP_Garden_RedirectWarp(warpEvent->mapGroup, warpEvent->mapNum);
        MP_Region_SharedExit(warpEvent->mapGroup, warpEvent->mapNum);   // Moemon Kai: out of the GARDEN / GAME CENTER in JOHTO / KANTO
    }
}

// Moemon Kai: an ORRE door is wider than the tiles a person can stand on in it (the MAYOR's in PHENAC: three tiles of
// its trigger, one reached). Walking into a part of a door's trigger no one can stand on takes the door its reached
// tile takes, unless that tile is shut (a story gate: orre_vm.c SetGate), the user 2026-11-08
static bool8 TryOrreWideDoor(struct MapPosition *position)
{
    s32 tid, dx, dy;
    if (!O3D_Active() || MapGridGetCollisionAt(position->x, position->y) == 0)
        return FALSE;
    tid = O3D_TriggerAt(position->x - MAP_OFFSET, position->y - MAP_OFFSET);
    if (tid < 0)
        return FALSE;
    for (dy = -3; dy <= 3; dy++)
        for (dx = -3; dx <= 3; dx++)
        {
            struct MapPosition at = {position->x + dx, position->y + dy, position->elevation};
            s8 warpEventId;
            u16 behavior;
            if (O3D_TriggerAt(at.x - MAP_OFFSET, at.y - MAP_OFFSET) != tid || MapGridGetCollisionAt(at.x, at.y) != 0)
                continue;
            behavior = MapGridGetMetatileBehaviorAt(at.x, at.y);
            warpEventId = GetWarpEventAtMapPosition(&gMapHeader, &at);
            if (warpEventId == WARP_ID_NONE || !IsWarpMetatileBehavior(behavior))
                continue;
            StoreInitialPlayerAvatarState();
            SetupWarp(&gMapHeader, warpEventId, &at);
            DoWarp();
            return TRUE;
        }
    return FALSE;
}

static bool8 TryDoorWarp(struct MapPosition *position, u16 metatileBehavior, u8 direction)
{
    s8 warpEventId;

    if (direction == DIR_NORTH)
    {
        if (MetatileBehavior_IsOpenSecretBaseDoor(metatileBehavior) == TRUE)
        {
            WarpIntoSecretBase(position, gMapHeader.events);
            return TRUE;
        }

        if (MetatileBehavior_IsWarpDoor(metatileBehavior) == TRUE)
        {
            warpEventId = GetWarpEventAtMapPosition(&gMapHeader, position);
            if (warpEventId != WARP_ID_NONE && IsWarpMetatileBehavior(metatileBehavior) == TRUE)
            {
                StoreInitialPlayerAvatarState();
                SetupWarp(&gMapHeader, warpEventId, position);
                DoDoorWarp();
                return TRUE;
            }
        }
    }
    return FALSE;
}

static s8 GetWarpEventAtPosition(struct MapHeader *mapHeader, u16 x, u16 y, u8 elevation)
{
    s32 i;
    const struct WarpEvent *warpEvent = mapHeader->events->warps;
    u8 warpCount = mapHeader->events->warpCount;

    for (i = 0; i < warpCount; i++, warpEvent++)
    {
        if ((u16)warpEvent->x == x && (u16)warpEvent->y == y)
        {
            if (warpEvent->elevation == elevation || warpEvent->elevation == ELEVATION_TRANSITION)
                return i;
        }
    }
    return WARP_ID_NONE;
}

static const u8 *TryRunCoordEventScript(const struct CoordEvent *coordEvent)
{
    if (coordEvent != NULL)
    {
        if (coordEvent->script == NULL)
        {
            DoCoordEventWeather(coordEvent->trigger);
            return NULL;
        }
        if (coordEvent->trigger == TRIGGER_RUN_IMMEDIATELY)
        {
            RunScriptImmediately(coordEvent->script);
            return NULL;
        }
        if (VarGet(coordEvent->trigger) == (u8)coordEvent->index)
            return coordEvent->script;
    }
    return NULL;
}

static const u8 *GetCoordEventScriptAtPosition(struct MapHeader *mapHeader, u16 x, u16 y, u8 elevation)
{
    s32 i;
    const struct CoordEvent *coordEvents = mapHeader->events->coordEvents;
    u8 coordEventCount = mapHeader->events->coordEventCount;

    for (i = 0; i < coordEventCount; i++)
    {
        if ((u16)coordEvents[i].x == x && (u16)coordEvents[i].y == y)
        {
            if (coordEvents[i].elevation == elevation || coordEvents[i].elevation == ELEVATION_TRANSITION)
            {
                const u8 *script = TryRunCoordEventScript(&coordEvents[i]);
                if (script != NULL)
                    return script;
            }
        }
    }
    return NULL;
}

// Moemon Kai: FireRed's HandleBoulderActivateVictoryRoadSwitch. A step trigger waiting for the value 99 (which no
// script sets: KANTO's VICTORY ROAD switches, the TANOBY KEY) is pressed by a boulder pushed onto it, not by the player
void TryRunBoulderSwitchScript(s16 x, s16 y)
{
    s32 i;
    const struct CoordEvent *coordEvents = gMapHeader.events->coordEvents;
    for (i = 0; i < gMapHeader.events->coordEventCount; i++)
    {
        if (coordEvents[i].x == x && coordEvents[i].y == y && coordEvents[i].index == 99 && coordEvents[i].script != NULL
         && coordEvents[i].trigger != TRIGGER_RUN_IMMEDIATELY)
        {
            ScriptContext_SetupScript(coordEvents[i].script);
            return;
        }
    }
}

const u8 *GetCoordEventScriptAtMapPosition(struct MapPosition *position)
{
    return GetCoordEventScriptAtPosition(&gMapHeader, position->x - MAP_OFFSET, position->y - MAP_OFFSET, position->elevation);
}

static const struct BgEvent *GetBackgroundEventAtPosition(struct MapHeader *mapHeader, u16 x, u16 y, u8 elevation)
{
    u8 i;
    const struct BgEvent *bgEvents = mapHeader->events->bgEvents;
    u8 bgEventCount = mapHeader->events->bgEventCount;

    for (i = 0; i < bgEventCount; i++)
    {
        if ((u16)bgEvents[i].x == x && (u16)bgEvents[i].y == y)
        {
            if (bgEvents[i].elevation == elevation || bgEvents[i].elevation == ELEVATION_TRANSITION)
                return &bgEvents[i];
        }
    }
    return NULL;
}

bool8 TryDoDiveWarp(struct MapPosition *position, u16 metatileBehavior)
{
    if (gMapHeader.mapType == MAP_TYPE_UNDERWATER && !MetatileBehavior_IsUnableToEmerge(metatileBehavior))
    {
        if (SetDiveWarpEmerge(position->x - MAP_OFFSET, position->y - MAP_OFFSET))
        {
            StoreInitialPlayerAvatarState();
            DoDiveWarp();
            PlaySE(SE_M_DIVE);
            return TRUE;
        }
    }
    else if (MetatileBehavior_IsDiveable(metatileBehavior) == TRUE)
    {
        if (SetDiveWarpDive(position->x - MAP_OFFSET, position->y - MAP_OFFSET))
        {
            StoreInitialPlayerAvatarState();
            DoDiveWarp();
            PlaySE(SE_M_DIVE);
            return TRUE;
        }
    }
    return FALSE;
}

u8 TrySetDiveWarp(void)
{
    s16 x, y;
    u8 metatileBehavior;

    PlayerGetDestCoords(&x, &y);
    metatileBehavior = MapGridGetMetatileBehaviorAt(x, y);
    if (gMapHeader.mapType == MAP_TYPE_UNDERWATER && !MetatileBehavior_IsUnableToEmerge(metatileBehavior))
    {
        if (SetDiveWarpEmerge(x - MAP_OFFSET, y - MAP_OFFSET) == TRUE)
            return 1;
    }
    else if (MetatileBehavior_IsDiveable(metatileBehavior) == TRUE)
    {
        if (SetDiveWarpDive(x - MAP_OFFSET, y - MAP_OFFSET) == TRUE)
            return 2;
    }
    return 0;
}

const u8 *GetObjectEventScriptPointerPlayerFacing(void)
{
    u8 direction;
    struct MapPosition position;

    direction = GetPlayerMovementDirection();
    GetInFrontOfPlayerPosition(&position);
    return GetInteractedObjectEventScript(&position, MapGridGetMetatileBehaviorAt(position.x, position.y), direction);
}

int SetCableClubWarp(void)
{
    struct MapPosition position;

    GetPlayerMovementDirection();  //unnecessary
    GetPlayerPosition(&position);
    MapGridGetMetatileBehaviorAt(position.x, position.y);  //unnecessary
    SetupWarp(&gMapHeader, GetWarpEventAtMapPosition(&gMapHeader, &position), &position);
    return 0;
}

// Moemon Co-op: is the player facing a sign (a background event with a
// script)? Its tile is the key for scribbles.
bool8 MP_FacingSign(s16 *x, s16 *y)
{
    struct MapPosition position;
    const struct BgEvent *bgEvent;
    GetInFrontOfPlayerPosition(&position);
    bgEvent = GetBackgroundEventAtPosition(&gMapHeader, position.x - MAP_OFFSET, position.y - MAP_OFFSET, position.elevation);
    if (bgEvent == NULL || bgEvent->kind > BG_EVENT_PLAYER_FACING_WEST || bgEvent->bgUnion.script == NULL)
        return FALSE;
    *x = position.x;
    *y = position.y;
    return TRUE;
}
