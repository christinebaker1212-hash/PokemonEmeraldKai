@ Moemon Kai: KANTO's TRAINER TOWER as FireRed's (audit case 10): pret/pokefirered data/scripts/trainer_tower.inc,
@ asm/macros/event.inc's ttower_* and the TRAINER TOWER maps' own scripts, on src/mp/mp_trainer_tower.c
@ (CallKantoTrainerTowerFunc). The maps (data/maps/Kanto_TrainerTower_*) point here. FireRed's words come from
@ tools/quetzal_port/restore_words.txt (the texts at the end).

	.macro ttower_initfloor
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_INIT_FLOOR
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_getspeech speech:req, trainer=0xFF
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_SPEECH
	setvar VAR_0x8005, \speech
	.if \trainer >= VARS_START && \trainer != 0xFF
	copyvar VAR_0x8006, \trainer
	.elseif \trainer != 0xFF
	setvar VAR_0x8006, \trainer
	.endif
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_dobattle
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_DO_BATTLE
	setvar VAR_0x8005, 0
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_getchallengetype
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_CHALLENGE_TYPE
	setvar VAR_0x8005, FALSE
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_clearedfloor
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CLEARED_FLOOR
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_isfloorcleared
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_FLOOR_CLEARED
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_startchallenge
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_START_CHALLENGE
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_getownerstate
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_OWNER_STATE
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_giveprize
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GIVE_PRIZE
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_checkfinaltime
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CHECK_FINAL_TIME
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_resumetimer
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_RESUME_TIMER
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_setlost
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_SET_LOST
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_getchallengestatus
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_gettime
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_TIME
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_showresults
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_SHOW_RESULTS
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_closeresults
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CLOSE_RESULTS
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_checkdoubles
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_CHECK_DOUBLES
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_getnumfloors
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_NUM_FLOORS
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_shouldexit
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_encountermusic
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_ENCOUNTER_MUSIC
	special CallKantoTrainerTowerFunc
	.endm

	.macro ttower_getbeatchallenge
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE
	special CallKantoTrainerTowerFunc
	.endm

.equ LOCALID_TOWER_TRAINER_DOUBLES1, 2
.equ LOCALID_TOWER_TRAINER_SINGLES,  3
.equ LOCALID_TOWER_TRAINER_KNOCKOUT, 4
.equ LOCALID_TOWER_TRAINER_DOUBLES2, 5

.equ HIDE_TRAINER_DOUBLES1, FLAG_TEMP_2
.equ HIDE_TRAINER_SINGLES,  FLAG_TEMP_3
.equ HIDE_TRAINER_KNOCKOUT, FLAG_TEMP_4
.equ HIDE_TRAINER_DOUBLES2, FLAG_TEMP_5
.equ HIDE_OWNER,            FLAG_TEMP_6

.equ DISABLE_SINGLES_TRIGGER,  VAR_TEMP_E
.equ DISABLE_DOUBLES_TRIGGER,  VAR_TEMP_F


@@ Map scripts (floors 1F-8F, the ROOF, the ELEVATOR)

KantoTT_OnResume::
	setvar VAR_TEMP_2, 0
	ttower_resumetimer
	end

KantoTT_OnTransition::
	ttower_initfloor
	switch VAR_RESULT
	case CHALLENGE_TYPE_SINGLE, KantoTT_EventScript_SetObjectsSingles
	case CHALLENGE_TYPE_DOUBLE, KantoTT_EventScript_SetObjectsDoubles
	case CHALLENGE_TYPE_KNOCKOUT, KantoTT_EventScript_SetObjectsKnockout
	setflag HIDE_TRAINER_DOUBLES1
	setflag HIDE_TRAINER_SINGLES
	setflag HIDE_TRAINER_KNOCKOUT
	setflag HIDE_TRAINER_DOUBLES2
	setvar DISABLE_SINGLES_TRIGGER, TRUE
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
	end

KantoTT_EventScript_SetObjectsSingles::
	setflag HIDE_TRAINER_DOUBLES1
	setflag HIDE_TRAINER_KNOCKOUT
	setflag HIDE_TRAINER_DOUBLES2
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
	setobjectxyperm       LOCALID_TOWER_TRAINER_SINGLES, 15, 13
	setobjectmovementtype LOCALID_TOWER_TRAINER_SINGLES, MOVEMENT_TYPE_FACE_LEFT
	goto KantoTT_EventScript_HideOwner

KantoTT_EventScript_SetObjectsDoubles::
	setflag HIDE_TRAINER_SINGLES
	setflag HIDE_TRAINER_KNOCKOUT
	setvar DISABLE_SINGLES_TRIGGER, TRUE
	ttower_isfloorcleared
	goto_if_eq VAR_RESULT, TRUE, KantoTT_EventScript_SetObjectsDoublesAlreadyBeaten
	setobjectxyperm       LOCALID_TOWER_TRAINER_DOUBLES1, 10, 12
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES1, MOVEMENT_TYPE_FACE_LEFT
	setobjectxyperm       LOCALID_TOWER_TRAINER_DOUBLES2, 10, 13
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES2, MOVEMENT_TYPE_FACE_LEFT
	goto KantoTT_EventScript_HideOwner

KantoTT_EventScript_SetObjectsDoublesAlreadyBeaten::
	setobjectxyperm       LOCALID_TOWER_TRAINER_DOUBLES1, 10, 12
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES1, MOVEMENT_TYPE_FACE_DOWN
	setobjectxyperm       LOCALID_TOWER_TRAINER_DOUBLES2, 11, 12
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES2, MOVEMENT_TYPE_FACE_DOWN
	goto KantoTT_EventScript_HideOwner

@ Knockout challenge re-uses trainer objects
KantoTT_EventScript_SetObjectsKnockout::
	setflag HIDE_TRAINER_DOUBLES2
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
	setobjectxyperm       LOCALID_TOWER_TRAINER_DOUBLES1, 10, 10
	setobjectmovementtype LOCALID_TOWER_TRAINER_DOUBLES1, MOVEMENT_TYPE_FACE_DOWN
	setobjectxyperm       LOCALID_TOWER_TRAINER_SINGLES, 14, 13
	setobjectmovementtype LOCALID_TOWER_TRAINER_SINGLES, MOVEMENT_TYPE_FACE_LEFT
	setobjectxyperm       LOCALID_TOWER_TRAINER_KNOCKOUT, 10, 16
	setobjectmovementtype LOCALID_TOWER_TRAINER_KNOCKOUT, MOVEMENT_TYPE_FACE_UP
KantoTT_EventScript_HideOwner:
	setflag HIDE_OWNER
	end

KantoTT_OnFrame::
	map_script_2 VAR_TEMP_2, 0, KantoTT_EventScript_EnterFloor
	.2byte 0

KantoTT_EventScript_EnterFloor::
	setvar VAR_TEMP_2, 1
	ttower_isfloorcleared
	goto_if_eq VAR_RESULT, FALSE, KantoTT_EventScript_EndEnterFloor
	setvar DISABLE_SINGLES_TRIGGER, TRUE
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
KantoTT_EventScript_EndEnterFloor:
	ttower_shouldexit
	goto_if_eq VAR_RESULT, TRUE, KantoTT_EventScript_WarpToLobby
	end

@ Never reached
KantoTT_EventScript_WarpToLobby::
	warp MAP_KANTO_TRAINER_TOWER_LOBBY, 9, 7
	waitstate


@@ Battle scripts

KantoTT_EventScript_TriggerBattle::
	ttower_getchallengetype
	switch VAR_RESULT
	case CHALLENGE_TYPE_SINGLE, KantoTT_EventScript_DoSingleBattle
	case CHALLENGE_TYPE_DOUBLE, KantoTT_EventScript_DoDoubleBattle
	case CHALLENGE_TYPE_KNOCKOUT, KantoTT_EventScript_DoKnockoutBattle

KantoTT_EventScript_DoSingleBattle:
	ttower_encountermusic
	applymovement LOCALID_TOWER_TRAINER_SINGLES, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_SINGLES, Common_Movement_Delay48
	waitmovement 0
	setvar DISABLE_SINGLES_TRIGGER, TRUE
	applymovement LOCALID_TOWER_TRAINER_SINGLES, KantoTT_Movement_SingleTrainerApproach
	waitmovement 0
	ttower_getspeech TRAINER_TOWER_TEXT_INTRO, 0
	msgbox gStringVar4
	closemessage
	goto KantoTT_EventScript_DoBattle

KantoTT_EventScript_DoDoubleBattle::
	ttower_getspeech TRAINER_TOWER_TEXT_INTRO, VAR_TEMP_3
	msgbox gStringVar4
	setvar VAR_0x8004, TRAINER_TOWER_FUNC_GET_SPEECH
	setvar VAR_0x8005, TRAINER_TOWER_TEXT_INTRO
	addvar VAR_TEMP_3, 1
	goto_if_eq VAR_TEMP_3, 1, KantoTT_EventScript_DoSecondTrainerIntro
	setvar VAR_TEMP_3, 0
KantoTT_EventScript_DoSecondTrainerIntro:
	copyvar VAR_0x8006, VAR_TEMP_3
	special CallKantoTrainerTowerFunc
	msgbox gStringVar4
	closemessage
	goto KantoTT_EventScript_DoBattle

KantoTT_EventScript_DoKnockoutBattle::
	switch VAR_TEMP_1
	case 0, KantoTT_EventScript_DoKnockoutBattle1
	case 1, KantoTT_EventScript_DoKnockoutBattle2
	case 2, KantoTT_EventScript_DoKnockoutBattle3

KantoTT_EventScript_DoKnockoutBattle1:
	ttower_encountermusic
	applymovement LOCALID_TOWER_TRAINER_KNOCKOUT, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_KNOCKOUT, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_KNOCKOUT, KantoTT_Movement_BottomKnockoutTrainerApproach
	waitmovement 0
	goto KantoTT_EventScript_DoKnockoutBattleIntro

KantoTT_EventScript_DoKnockoutBattle2::
	ttower_encountermusic
	applymovement LOCALID_TOWER_TRAINER_DOUBLES1, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_DOUBLES1, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_DOUBLES1, KantoTT_Movement_TopKnockoutTrainerApproach
	waitmovement 0
	goto KantoTT_EventScript_DoKnockoutBattleIntro

KantoTT_EventScript_DoKnockoutBattle3::
	ttower_encountermusic
	applymovement LOCALID_TOWER_TRAINER_SINGLES, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_SINGLES, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_SINGLES, KantoTT_Movement_RightKnockoutTrainerApproach
	waitmovement 0
KantoTT_EventScript_DoKnockoutBattleIntro:
	ttower_getspeech TRAINER_TOWER_TEXT_INTRO, VAR_TEMP_1
	msgbox gStringVar4
	closemessage
KantoTT_EventScript_DoBattle:
	ttower_dobattle
	waitstate
	switch VAR_RESULT
	case B_OUTCOME_WON, KantoTT_EventScript_WonBattle
	case B_OUTCOME_LOST, KantoTT_EventScript_WarpToLobbyLost
	case B_OUTCOME_DREW, KantoTT_EventScript_WarpToLobbyLost
	case B_OUTCOME_FORFEITED, KantoTT_EventScript_WarpToLobbyLost   @ (Emerald's battles can be forfeited; FireRed's could not)
KantoTT_EventScript_WonBattle:
	ttower_getchallengetype
	switch VAR_RESULT
	case CHALLENGE_TYPE_SINGLE, KantoTT_EventScript_SetFloorCleared
	case CHALLENGE_TYPE_DOUBLE, KantoTT_EventScript_MoveDoublesTrainers
	case CHALLENGE_TYPE_KNOCKOUT, KantoTT_EventScript_CheckNextKnockoutBattle

KantoTT_EventScript_CheckNextKnockoutBattle:
	switch VAR_TEMP_1
	case 0, KantoTT_EventScript_DoSecondKnockoutBattle
	case 1, KantoTT_EventScript_DoThirdKnockoutBattle
	case 2, KantoTT_EventScript_MoveLastKnockoutTrainer

KantoTT_EventScript_DoSecondKnockoutBattle:
	addvar VAR_TEMP_1, 1
	goto KantoTT_EventScript_DoKnockoutBattle

KantoTT_EventScript_DoThirdKnockoutBattle::
	addvar VAR_TEMP_1, 1
	goto KantoTT_EventScript_DoKnockoutBattle

KantoTT_EventScript_MoveDoublesTrainers::
	applymovement LOCALID_TOWER_TRAINER_DOUBLES2, KantoTT_Movement_DoublesTrainer2OutOfWay
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_DOUBLES1, KantoTT_Movement_DoublesTrainer1FaceDown
	goto KantoTT_EventScript_SetFloorCleared

KantoTT_EventScript_MoveLastKnockoutTrainer::
	applymovement LOCALID_TOWER_TRAINER_SINGLES, KantoTT_Movement_LastKnockoutTrainerOutOfWay
KantoTT_EventScript_SetFloorCleared:
	ttower_clearedfloor
	end

KantoTT_EventScript_WarpToLobbyLost::
	special HealPlayerParty
	ttower_setlost
	warp MAP_KANTO_TRAINER_TOWER_LOBBY, 9, 7
	waitstate


@@ Speaking to trainers after battle

KantoTT_EventScript_DoublesTrainer1::
	call KantoTT_EventScript_SpeakToDoublesTrainer1
	end

KantoTT_EventScript_SinglesTrainer::
	call KantoTT_EventScript_SpeakToSinglesTrainer
	end

KantoTT_EventScript_KnockoutTrainer::
	call KantoTT_EventScript_SpeakToKnockoutTrainer
	end

KantoTT_EventScript_DoublesTrainer2::
	call KantoTT_EventScript_SpeakToDoublesTrainer2
	end

KantoTT_EventScript_Owner::
	call KantoTT_EventScript_SpeakToOwner
	end

KantoTT_EventScript_SpeakToDoublesTrainer1::
	ttower_getchallengetype
	switch VAR_RESULT
	case CHALLENGE_TYPE_SINGLE, KantoTT_EventScript_DoublesTrainer1PostBattle
	case CHALLENGE_TYPE_DOUBLE, KantoTT_EventScript_DoublesTrainer1PostBattle
	case CHALLENGE_TYPE_KNOCKOUT, KantoTT_EventScript_KnockoutTrainer2PostBattle
KantoTT_EventScript_DoublesTrainer1PostBattle:
	setvar VAR_0x8006, 0
	goto KantoTT_EventScript_DoPostBattleText

KantoTT_EventScript_KnockoutTrainer2PostBattle::
	setvar VAR_0x8006, 1
	goto KantoTT_EventScript_DoPostBattleText

KantoTT_EventScript_SpeakToSinglesTrainer::
	ttower_getchallengetype
	switch VAR_RESULT
	case CHALLENGE_TYPE_SINGLE, KantoTT_EventScript_SinglesTrainerPostBattle
	case CHALLENGE_TYPE_DOUBLE, KantoTT_EventScript_SinglesTrainerPostBattle
	case CHALLENGE_TYPE_KNOCKOUT, KantoTT_EventScript_KnockoutTrainer3PostBattle
KantoTT_EventScript_SinglesTrainerPostBattle:
	setvar VAR_0x8006, 0
	goto KantoTT_EventScript_DoPostBattleText

KantoTT_EventScript_KnockoutTrainer3PostBattle::
	setvar VAR_0x8006, 2
	goto KantoTT_EventScript_DoPostBattleText

KantoTT_EventScript_SpeakToKnockoutTrainer::
	setvar VAR_0x8006, 0
	goto KantoTT_EventScript_DoPostBattleText

KantoTT_EventScript_SpeakToDoublesTrainer2::
	setvar VAR_0x8006, 1
KantoTT_EventScript_DoPostBattleText:
	ttower_getspeech TRAINER_TOWER_TEXT_AFTER
	lock
	faceplayer
	msgbox gStringVar4
	release
	return


@@ Speaking to owner

KantoTT_EventScript_SpeakToOwner::
	lock
	faceplayer
	ttower_getownerstate
	switch VAR_RESULT
	case 0, KantoTT_Roof_EventScript_Arrived
	case 1, KantoTT_Roof_EventScript_GivePrize
	case 2, KantoTT_Roof_EventScript_OwnerEnd

KantoTT_Roof_EventScript_Arrived:
	msgbox KantoTT_Roof_Text_ImOwnerBattledPerfectly
KantoTT_Roof_EventScript_GivePrize:
	ttower_giveprize
	switch VAR_RESULT
	case 0, KantoTT_Roof_EventScript_ReceivePrize
	case 1, KantoTT_Roof_EventScript_NoRoomForPrize
	case 2, KantoTT_Roof_EventScript_CheckFinalTime

KantoTT_Roof_EventScript_ReceivePrize:
	msgbox KantoTT_Roof_Text_ThisIsForYou
	playfanfare MUS_LEVEL_UP
	message KantoTT_Text_ObtainedTheX
	waitfanfare
	waitmessage
	msgbox KantoTT_Text_PutItemAway
	goto KantoTT_Roof_EventScript_CheckFinalTime

KantoTT_Roof_EventScript_NoRoomForPrize::
	msgbox KantoTT_Roof_Text_ThisIsForYou
	msgbox KantoTT_Text_BagIsFull
	goto KantoTT_Roof_EventScript_CheckFinalTime

KantoTT_Roof_EventScript_CheckFinalTime::
	ttower_checkfinaltime
	switch VAR_RESULT
	case 0, KantoTT_Roof_EventScript_NewRecord
	case 1, KantoTT_Roof_EventScript_NoNewRecord
	case 2, KantoTT_Roof_EventScript_OwnerEnd

KantoTT_Roof_EventScript_NewRecord:
	msgbox KantoTT_Roof_Text_DoneItInRecordTime
	goto KantoTT_Roof_EventScript_OwnerEnd

KantoTT_Roof_EventScript_NoNewRecord::
	msgbox KantoTT_Roof_Text_TookSweetTimeGettingHere
KantoTT_Roof_EventScript_OwnerEnd:
	msgbox KantoTT_Roof_Text_IdLikeToSeeBetterTime
	release
	return

KantoTT_EventScript_ShowTime::
	lockall
	ttower_gettime
	msgbox KantoTT_Text_XMinYZSec
	releaseall
	end


@@ Battle triggers

KantoTT_EventScript_SingleBattleTrigger::
	goto KantoTT_EventScript_TriggerBattle

KantoTT_EventScript_DoubleBattleTriggerTop::
	setvar VAR_TEMP_3, 0
	ttower_checkdoubles
	goto_if_ne VAR_RESULT, PLAYER_HAS_TWO_USABLE_MONS, KantoTT_EventScript_IneligibleForDoubleBattle
	ttower_encountermusic
	applymovement LOCALID_TOWER_TRAINER_DOUBLES1, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_DOUBLES1, Common_Movement_Delay48
	goto KantoTT_EventScript_TriggerDoubleBattle

KantoTT_EventScript_DoubleBattleTriggerBottom::
	setvar VAR_TEMP_3, 1
	ttower_checkdoubles
	goto_if_ne VAR_RESULT, PLAYER_HAS_TWO_USABLE_MONS, KantoTT_EventScript_IneligibleForDoubleBattle
	ttower_encountermusic
	applymovement LOCALID_TOWER_TRAINER_DOUBLES2, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_TOWER_TRAINER_DOUBLES2, Common_Movement_Delay48
KantoTT_EventScript_TriggerDoubleBattle:
	waitmovement 0
	setvar DISABLE_DOUBLES_TRIGGER, TRUE
	goto KantoTT_EventScript_TriggerBattle

KantoTT_EventScript_IneligibleForDoubleBattle::
	lockall
	playse SE_DING_DONG
	msgbox KantoTT_Lobby_Text_NeedTwoMonsForDouble
	closemessage
	applymovement LOCALID_PLAYER, KantoTT_Movement_PushPlayerBack
	waitmovement 0
	releaseall
	end

KantoTT_Movement_SingleTrainerApproach:
	walk_left
KantoTT_Movement_RightKnockoutTrainerApproach::
	walk_left
	walk_left
KantoTT_Movement_PushPlayerBack:
	walk_left
	step_end

KantoTT_Movement_BottomKnockoutTrainerApproach::
	walk_up
	walk_up
	step_end

KantoTT_Movement_TopKnockoutTrainerApproach::
	walk_down
	walk_down
	step_end

KantoTT_Movement_DoublesTrainer2OutOfWay::
	walk_right
KantoTT_Movement_LastKnockoutTrainerOutOfWay:
	walk_up
	face_down
	step_end

KantoTT_Movement_DoublesTrainer1FaceDown::
	face_down
	step_end


@@ The LOBBY (FireRed's TrainerTower_Lobby scripts; its nurse, clerk and two visitors stay the map's own)

KantoTT_Lobby_OnResume::
	setvar VAR_TEMP_2, 0
	ttower_resumetimer
	ttower_shouldexit
	goto_if_eq VAR_RESULT, FALSE, KantoTT_Lobby_OnResumeEnd
	@ Never reached, above always FALSE
	setvar VAR_TEMP_0, 0
	setobjectxy LOCALID_PLAYER, 9, 7
	applymovement LOCALID_PLAYER, KantoTT_Lobby_Movement_FaceReceptionist
KantoTT_Lobby_OnResumeEnd::
	end

KantoTT_Lobby_EventScript_Enter::
	setvar VAR_TEMP_0, 1
	ttower_getchallengestatus
	switch VAR_RESULT
	case TT_STATUS_LOST, KantoTT_Lobby_EventScript_LostChallenge
	case TT_STATUS_UNK, KantoTT_Lobby_EventScript_ExitChallengeSpeakToReceptionist
	case TT_STATUS_NORMAL, KantoTT_Lobby_EventScript_EnterEnd

KantoTT_Lobby_EventScript_LostChallenge::
	lock
	faceplayer
	applymovement LOCALID_PLAYER, KantoTT_Lobby_Movement_FaceReceptionist
	msgbox KantoTT_Lobby_Text_TooBadComeBackTryAgain
	goto KantoTT_Lobby_EventScript_ExitChallenge

@ Presumably E-Reader related (FireRed's note)
KantoTT_Lobby_EventScript_ExitChallengeSpeakToReceptionist::
	lock
	faceplayer
	applymovement LOCALID_PLAYER, KantoTT_Lobby_Movement_FaceReceptionist
	msgbox KantoTT_Lobby_Text_MoveCounterHereWhenTrainersSwitch
KantoTT_Lobby_EventScript_ExitChallenge::
	closemessage
	applymovement LOCALID_PLAYER, KantoTT_Lobby_Movement_WalkDown
	waitmovement 0
	setvar VAR_MAP_SCENE_TRAINER_TOWER, 0
	release
KantoTT_Lobby_EventScript_EnterEnd::
	end

KantoTT_Lobby_EventScript_Receptionist::
	lock
	faceplayer
	ttower_getbeatchallenge
	goto_if_eq VAR_RESULT, TRUE, KantoTT_Lobby_EventScript_ThanksForCompeting
	msgbox KantoTT_Lobby_Text_GiveItYourBest
	goto KantoTT_Lobby_EventScript_ReceptionistEnd

KantoTT_Lobby_EventScript_ThanksForCompeting::
	msgbox KantoTT_Lobby_Text_ThanksForCompeting
KantoTT_Lobby_EventScript_ReceptionistEnd::
	release
	end

KantoTT_Lobby_EventScript_EntryTrigger::
	lockall
	applymovement LOCALID_PLAYER, KantoTT_Lobby_Movement_FaceReceptionist
	msgbox KantoTT_Lobby_Text_WelcomeToTrainerTower
	ttower_getnumfloors
	goto_if_eq VAR_RESULT, FALSE, KantoTT_Lobby_EventScript_AllFloorsUsed
	msgbox KantoTT_Lobby_Text_TrainersUpToFloorNum
	goto KantoTT_Lobby_EventScript_AskEnterChallenge

KantoTT_Lobby_EventScript_AllFloorsUsed::
	msgbox KantoTT_Lobby_Text_TrainersUpEighthFloor
KantoTT_Lobby_EventScript_AskEnterChallenge::
	message KantoTT_Lobby_Text_LikeToChallengeTrainers
	waitmessage
	multichoice 18, 6, MULTI_QZ_0x84, FALSE   @ FireRed's MULTICHOICE_YES_NO_INFO
	switch VAR_RESULT
	case 0, KantoTT_Lobby_EventScript_ChooseChallenge
	case 1, KantoTT_Lobby_EventScript_DeclineChallenge
	case 2, KantoTT_Lobby_EventScript_ChallengeInfo
	case MULTI_B_PRESSED, KantoTT_Lobby_EventScript_DeclineChallenge
	end

KantoTT_Lobby_EventScript_ChallengeInfo::
	msgbox KantoTT_Lobby_Text_ExplainTrainerTower
	goto KantoTT_Lobby_EventScript_AskEnterChallenge
	end

KantoTT_Lobby_EventScript_ChooseChallenge::
	multichoice 13, 3, MULTI_QZ_0x81, FALSE   @ FireRed's MULTICHOICE_TRAINER_TOWER_MODE
	switch VAR_RESULT
	case 0, KantoTT_Lobby_EventScript_BeginChallenge
	case 1, KantoTT_Lobby_EventScript_BeginChallenge
	case 2, KantoTT_Lobby_EventScript_BeginChallenge
	case 3, KantoTT_Lobby_EventScript_BeginChallenge
	case 4, KantoTT_Lobby_EventScript_DeclineChallenge
	case MULTI_B_PRESSED, KantoTT_Lobby_EventScript_DeclineChallenge
	end

KantoTT_Lobby_EventScript_BeginChallenge::
	copyvar VAR_0x8005, VAR_RESULT
	setvar VAR_MAP_SCENE_TRAINER_TOWER, 1
	special HealPlayerParty
	msgbox KantoTT_Lobby_Text_StartClockGetSetGo
	ttower_startchallenge
	releaseall
	end

KantoTT_Lobby_EventScript_DeclineChallenge::
	msgbox KantoTT_Lobby_Text_PleaseVisitUsAgain
	closemessage
	applymovement LOCALID_PLAYER, KantoTT_Lobby_Movement_WalkDown
	waitmovement 0
	releaseall
	end

KantoTT_Lobby_Movement_FaceReceptionist::
	face_right
	step_end

KantoTT_Lobby_Movement_WalkDown::
	walk_down
	step_end

@ The Time Board (FireRed's ShowBattleRecords page for the TOWER, as its results window)
KantoTT_Lobby_EventScript_ShowRecords::
	lockall
	ttower_showresults
	waitbuttonpress
	ttower_closeresults
	releaseall
	end


@@ FireRed's words (tools/quetzal_port/restore_words.txt)

KantoTT_Lobby_Text_WelcomeToTrainerTower::
	.string "Hello!\p"
	.string "Welcome to TRAINER TOWER where\n"
	.string "TRAINERS gather from all over!$"

KantoTT_Lobby_Text_TrainersUpToFloorNum::
	.string "TRAINERS from all over the world\n"
	.string "gather here to battle.\p"
	.string "Let me see…\p"
	.string "Right now, there are TRAINERS only\n"
	.string "up to Floor {STR_VAR_1}.$"

KantoTT_Lobby_Text_TrainersUpEighthFloor::
	.string "TRAINERS are awaiting your\n"
	.string "challenge up to the eighth floor.$"

KantoTT_Lobby_Text_LikeToChallengeTrainers::
	.string "Would you like to challenge the\n"
	.string "waiting TRAINERS?$"

KantoTT_Lobby_Text_StartClockGetSetGo::
	.string "Okay, I’ll get the clock started,\n"
	.string "so give it everything you have.\p"
	.string "On your marks…\p"
	.string "Get set…\p"
	.string "Go!$"

KantoTT_Lobby_Text_PleaseVisitUsAgain::
	.string "Please do visit us again!$"

KantoTT_Lobby_Text_TooBadComeBackTryAgain::
	.string "That was too bad.\p"
	.string "I think you put in a tremendous\n"
	.string "effort in your battling.\p"
	.string "Please come back and try again!$"

KantoTT_Lobby_Text_GiveItYourBest::
	.string "I hope you give it your best.$"

KantoTT_Lobby_Text_MoveCounterHereWhenTrainersSwitch::
	.string "When the TRAINERS switch places,\n"
	.string "the movement can be hectic.\p"
	.string "To avoid the stampede, we moved\n"
	.string "the reception counter here.\p"
	.string "I’m sorry for the inconvenience.$"

KantoTT_Roof_Text_ImOwnerBattledPerfectly::
	.string "Hello…\p"
	.string "I am the owner of this TOWER…\p"
	.string "How the sky soars above this\n"
	.string "rooftop…\p"
	.string "The caress of the winds up here…\p"
	.string "It’s all so perfect…\p"
	.string "The way you battled…\n"
	.string "It, too, was perfection…$"

KantoTT_Roof_Text_ThisIsForYou::
	.string "This is for you…$"

KantoTT_Roof_Text_DoneItInRecordTime::
	.string "Oh!\n"
	.string "Stupendous!\p"
	.string "It’s marvelous how you’ve come up\n"
	.string "here so quickly.\p"
	.string "The fact is, you’ve done it in\n"
	.string "record time…\p"
	.string "I’ll have your record posted at\n"
	.string "the reception counter.$"

KantoTT_Roof_Text_TookSweetTimeGettingHere::
	.string "You seem to have taken your sweet\n"
	.string "time getting here…$"

KantoTT_Roof_Text_IdLikeToSeeBetterTime::
	.string "What I would like to see is a\n"
	.string "better time out of you…\p"
	.string "I’m counting on you.\p"
	.string "Until then, farewell…$"

KantoTT_Text_XMinYZSec::
	.string "{STR_VAR_1} min. {STR_VAR_2}.{STR_VAR_3} sec.$"

KantoTT_Lobby_Text_NeedTwoMonsForDouble::
	.string "This is a two-on-two battle.\p"
	.string "You may not battle unless you have\n"
	.string "at least two MOéMON.$"

KantoTT_Lobby_Text_ExplainTrainerTower::
	.string "Here at TRAINER TOWER, there is an\n"
	.string "event called TIME ATTACK.\p"
	.string "You will be timed on how quickly\n"
	.string "you can get from the reception\l"
	.string "counter to the OWNER on the roof.\p"
	.string "The best times will be recorded on\n"
	.string "the Time Board.\p"
	.string "Try competing with friends to see\n"
	.string "who can beat it the fastest.\p"
	.string "You will not earn any EXP. Points\n"
	.string "or money by beating TRAINERS here.$"

KantoTT_Lobby_Text_ThanksForCompeting::
	.string "Thank you for competing!$"

KantoTT_Text_ObtainedTheX::
	.string "Obtained the {STR_VAR_2}!$"

KantoTT_Text_BagIsFull::
	.string "The BAG is full…$"

KantoTT_Text_PutItemAway::
	.string "{PLAYER} put the {STR_VAR_2}\n"
	.string "in the {STR_VAR_3}.$"
