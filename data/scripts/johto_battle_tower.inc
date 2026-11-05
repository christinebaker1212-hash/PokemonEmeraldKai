@ Moemon Kai: JOHTO's BATTLE TOWER as Crystal's (audit case 37): pret/pokecrystal maps/BattleTower1F.asm,
@ BattleTowerElevator.asm, BattleTowerHallway.asm, BattleTowerBattleRoom.asm, BattleTowerOutside.asm and
@ Route40BattleTowerGate.asm, on src/mp/mp_johto_tower.c (CallJohtoBattleTowerFunc). The tower's rooms are the
@ game's own BATTLE TOWER rooms (the same layouts as the BATTLE FRONTIER's), walked the way that tower walks them;
@ Crystal's words come from tools/quetzal_port/restore_words.txt (the texts at the end).

	.macro jbt func:req, arg
	setvar VAR_0x8004, \func
	.ifnb \arg
	setvar VAR_0x8005, \arg
	.endif
	special CallJohtoBattleTowerFunc
	.endm

.equ LOCALID_JBT_RECEPTIONIST, 1
.equ LOCALID_JBT_OPPONENT, 1
.equ LOCALID_JBT_ROOM_RECEPTIONIST, 2

@ the challenge states a warp back to the 1F carries (a script ends at a warp here; Crystal's goes on in the 1F)
.equ JBT_BACK_LOST, 5
.equ JBT_BACK_CANCELED, 6


@@ The BATTLE TOWER's grounds and ROUTE 40's gate

JohtoBT_Outside_EventScript_Youngster::
	msgbox JohtoBT_Outside_Text_Youngster, MSGBOX_NPC
	end

JohtoBT_Outside_EventScript_Beauty::
	msgbox JohtoBT_Outside_Text_Beauty, MSGBOX_NPC
	end

JohtoBT_Outside_EventScript_Sailor::
	msgbox JohtoBT_Outside_Text_Sailor, MSGBOX_NPC
	end

JohtoBT_Outside_EventScript_Sign::
	msgbox JohtoBT_Outside_Text_Sign, MSGBOX_SIGN
	end

@ (Crystal's grounds end at the gate's doors: here the south edge of the plaza)
JohtoBT_Outside_EventScript_ToGate::
	warp MAP_JOHTO_ROUTE40_TRAINER_HILL_GATE, 7, 2
	waitstate
	end

JohtoBT_Gate_EventScript_Rocker::
	msgbox JohtoBT_Gate_Text_Rocker, MSGBOX_NPC
	end

JohtoBT_Gate_EventScript_Twin::
	msgbox JohtoBT_Gate_Text_Twin, MSGBOX_NPC
	end


@@ The 1F (Crystal's BattleTower1F)

@ BattleTower1FCheckStateScene, and what a warp back from the BATTLE ROOM leaves to say
JohtoBT_1F_OnFrame::
	map_script_2 VAR_TEMP_0, 0, JohtoBT_1F_EventScript_CheckState
	.2byte 0

JohtoBT_1F_EventScript_CheckState::
	setvar VAR_TEMP_0, 1
	jbt JBT_FUNC_GET_STATE
	switch VAR_RESULT
	case JBT_SAVED_AND_LEFT, JohtoBT_1F_EventScript_WaitingForYou
	case JBT_CHALLENGE_IN_PROGRESS, JohtoBT_1F_EventScript_LeftWithoutSaving
	case JBT_WON_CHALLENGE, JohtoBT_1F_EventScript_BeatenAllTrainers
	case JBT_BACK_LOST, JohtoBT_1F_EventScript_ThanksForVisiting
	case JBT_BACK_CANCELED, JohtoBT_1F_EventScript_BackCanceled
	end

JohtoBT_1F_EventScript_WaitingForYou::
	lockall
	msgbox JohtoBT_Text_WeveBeenWaitingForYou
	closemessage
	jbt JBT_FUNC_SET_STATE, JBT_CHALLENGE_IN_PROGRESS
	goto JohtoBT_1F_EventScript_WalkToElevator

JohtoBT_1F_EventScript_LeftWithoutSaving::
	lockall
	jbt JBT_FUNC_SET_STATE, JBT_NO_CHALLENGE
	msgbox JohtoBT_Text_LeftWithoutSaving
	msgbox JohtoBT_Text_WeHopeToServeYouAgain
	releaseall
	end

JohtoBT_1F_EventScript_ThanksForVisiting::
	lockall
	jbt JBT_FUNC_SET_STATE, JBT_NO_CHALLENGE
	msgbox JohtoBT_Text_ThanksForVisiting
	releaseall
	end

JohtoBT_1F_EventScript_BackCanceled::
	lockall
	jbt JBT_FUNC_SET_STATE, JBT_NO_CHALLENGE
	msgbox JohtoBT_Text_WeHopeToServeYouAgain
	releaseall
	end

JohtoBT_1F_EventScript_BeatenAllTrainers::
	lockall
	goto JohtoBT_1F_EventScript_BeatenAllTrainers2

JohtoBT_1F_EventScript_RulesSign::
	lockall
	msgbox JohtoBT_Text_ReadBattleTowerRules, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoBT_1F_EventScript_RulesSignEnd
	msgbox JohtoBT_Text_BattleTowerRules
JohtoBT_1F_EventScript_RulesSignEnd::
	releaseall
	end

JohtoBT_1F_EventScript_Receptionist::
	lock
	faceplayer
	jbt JBT_FUNC_GET_STATE
	goto_if_eq VAR_RESULT, JBT_WON_CHALLENGE, JohtoBT_1F_EventScript_BeatenAllTrainers2
	msgbox JohtoBT_Text_BattleTowerWelcomesYou
	jbt JBT_FUNC_EXPLANATION_READ
	goto_if_ne VAR_RESULT, FALSE, JohtoBT_1F_EventScript_Menu
	goto JohtoBT_1F_EventScript_IntroductionYesNo

JohtoBT_1F_EventScript_Menu::
	message JohtoBT_Text_WantToGoIntoABattleRoom
	waitmessage
	multichoice 0, 0, MULTI_KAI_BATTLE_TOWER, FALSE
	switch VAR_RESULT
	case 0, JohtoBT_1F_EventScript_ChooseChallenge
	case 1, JohtoBT_1F_EventScript_Explanation
	goto JohtoBT_1F_EventScript_HopeToServeYouAgain

JohtoBT_1F_EventScript_ChooseChallenge::
	jbt JBT_FUNC_CHECK_RULES
	goto_if_ne VAR_RESULT, 0, JohtoBT_1F_EventScript_NotReady
	msgbox JohtoBT_Text_SaveBeforeEnteringBattleRoom, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoBT_1F_EventScript_Menu
	message JohtoBT_Text_SavingDontTurnOff
	waitmessage
	jbt JBT_FUNC_QUICK_SAVE
	playse SE_SAVE
	waitse
	goto_if_eq VAR_RESULT, FALSE, JohtoBT_1F_EventScript_Menu
	jbt JBT_FUNC_SET_EXPLAINED
@ the level choice (Crystal's BattleTowerRoomMenu)
JohtoBT_1F_EventScript_ChooseLevel::
	closemessage
	jbt JBT_FUNC_LEVEL_MENU
	waitstate
	goto_if_eq VAR_RESULT, 0, JohtoBT_1F_EventScript_Menu
	copyvar VAR_0x8005, VAR_RESULT
	copyvar VAR_TEMP_1, VAR_RESULT
	setvar VAR_0x8004, JBT_FUNC_LEVEL_CHECK
	special CallJohtoBattleTowerFunc
	goto_if_eq VAR_RESULT, 1, JohtoBT_1F_EventScript_LevelExceeds
	goto_if_eq VAR_RESULT, 2, JohtoBT_1F_EventScript_UnderL70
	copyvar VAR_0x8005, VAR_TEMP_1
	setvar VAR_0x8004, JBT_FUNC_START
	special CallJohtoBattleTowerFunc
	msgbox JohtoBT_Text_RightThisWayToYourBattleRoom
	closemessage
JohtoBT_1F_EventScript_WalkToElevator::
	applymovement LOCALID_JBT_RECEPTIONIST, JohtoBT_Movement_WalkToElevator
	applymovement LOCALID_PLAYER, JohtoBT_Movement_WalkToElevator
	waitmovement 0
	opendoor 6, 1
	waitdooranim
	applymovement LOCALID_JBT_RECEPTIONIST, JohtoBT_Movement_ReceptionistEnterElevator
	applymovement LOCALID_PLAYER, JohtoBT_Movement_PlayerEnterElevator
	waitmovement 0
	closedoor 6, 1
	waitdooranim
	warp MAP_JOHTO_BATTLE_TOWER_ELEVATOR, 1, 6
	waitstate
	end

JohtoBT_1F_EventScript_LevelExceeds::
	msgbox JohtoBT_Text_AMonLevelExceeds
	goto JohtoBT_1F_EventScript_Menu

JohtoBT_1F_EventScript_UnderL70::
	msgbox JohtoBT_Text_MayNotEnterABattleRoomUnderL70
	goto JohtoBT_1F_EventScript_Menu

@ _CheckForBattleTowerRules: "Excuse me", each rule broken, then "Please return when you're ready"
JohtoBT_1F_EventScript_NotReady::
	msgbox JohtoBT_Text_ExcuseMeYoureNotReady
	call_if_eq VAR_TEMP_2, TRUE, JohtoBT_1F_EventScript_RuleThree
	call_if_eq VAR_TEMP_3, TRUE, JohtoBT_1F_EventScript_RuleSpecies
	call_if_eq VAR_TEMP_4, TRUE, JohtoBT_1F_EventScript_RuleItems
	call_if_eq VAR_TEMP_5, TRUE, JohtoBT_1F_EventScript_RuleEgg
	msgbox JohtoBT_Text_ReturnWhenReady
	release
	end

JohtoBT_1F_EventScript_RuleThree::
	msgbox JohtoBT_Text_OnlyThreeMonMayBeEntered
	return

JohtoBT_1F_EventScript_RuleSpecies::
	msgbox JohtoBT_Text_TheMonMustAllBeDifferentKinds
	return

JohtoBT_1F_EventScript_RuleItems::
	msgbox JohtoBT_Text_TheMonMustNotHoldTheSameItems
	return

JohtoBT_1F_EventScript_RuleEgg::
	msgbox JohtoBT_Text_YouCantTakeAnEgg
	return

JohtoBT_1F_EventScript_IntroductionYesNo::
	msgbox JohtoBT_Text_WouldYouLikeToHearAboutTheBattleTower, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoBT_1F_EventScript_SkipExplanation
JohtoBT_1F_EventScript_Explanation::
	msgbox JohtoBT_Text_BattleTowerIntroduction
JohtoBT_1F_EventScript_SkipExplanation::
	jbt JBT_FUNC_SET_EXPLAINED
	goto JohtoBT_1F_EventScript_Menu

JohtoBT_1F_EventScript_HopeToServeYouAgain::
	msgbox JohtoBT_Text_WeHopeToServeYouAgain
	release
	end

@ Script_BeatenAllTrainers2 / Script_GivePlayerHisPrize
JohtoBT_1F_EventScript_BeatenAllTrainers2::
	msgbox JohtoBT_Text_CongratulationsYouveBeatenAllTheTrainers
	jbt JBT_FUNC_GIVE_REWARD
	goto_if_eq VAR_RESULT, 1, JohtoBT_1F_EventScript_PackIsStuffedFull
	playfanfare MUS_OBTAIN_ITEM
	message JohtoBT_Text_PlayerGotFive
	waitfanfare
	waitmessage
	waitbuttonpress
	releaseall
	end

JohtoBT_1F_EventScript_PackIsStuffedFull::
	msgbox JohtoBT_Text_YourPackIsStuffedFull
	releaseall
	end

JohtoBT_1F_EventScript_Youngster::
	msgbox JohtoBT_1F_Text_Youngster, MSGBOX_NPC
	end

JohtoBT_1F_EventScript_CooltrainerF::
	msgbox JohtoBT_1F_Text_CooltrainerF, MSGBOX_NPC
	end

JohtoBT_1F_EventScript_BugCatcher::
	msgbox JohtoBT_1F_Text_BugCatcher, MSGBOX_NPC
	end

JohtoBT_1F_EventScript_Granny::
	msgbox JohtoBT_1F_Text_Granny, MSGBOX_NPC
	end

JohtoBT_Movement_WalkToElevator:
	walk_up
	walk_up
	walk_up
	step_end

JohtoBT_Movement_ReceptionistEnterElevator:
	walk_up
	set_invisible
	step_end

JohtoBT_Movement_PlayerEnterElevator:
	walk_up
	walk_up
	set_invisible
	step_end


@@ The elevator (Crystal's BattleTowerElevator)

JohtoBT_Elevator_OnFrame::
	map_script_2 VAR_TEMP_0, 0, JohtoBT_Elevator_EventScript_Ride
	.2byte 0

JohtoBT_Elevator_EventScript_Ride::
	setvar VAR_TEMP_0, 1
	applymovement LOCALID_JBT_RECEPTIONIST, JohtoBT_Movement_ElevatorReceptionistEnter
	applymovement LOCALID_PLAYER, JohtoBT_Movement_ElevatorPlayerEnter
	waitmovement 0
	jbt JBT_FUNC_GET_GROUP
	setvar VAR_0x8005, 0
	copyvar VAR_0x8006, VAR_RESULT
	waitse
	special MoveElevator
	waitstate
	delay 48
	applymovement LOCALID_JBT_RECEPTIONIST, JohtoBT_Movement_ElevatorReceptionistExit
	applymovement LOCALID_PLAYER, JohtoBT_Movement_ElevatorPlayerExit
	waitmovement 0
	warp MAP_JOHTO_BATTLE_TOWER_HALLWAY, 8, 1
	waitstate
	end

JohtoBT_Movement_ElevatorReceptionistEnter:
	walk_up
	walk_right
	face_down
	step_end

JohtoBT_Movement_ElevatorPlayerEnter:
	walk_up
	walk_up
	face_down
	step_end

JohtoBT_Movement_ElevatorReceptionistExit:
	walk_down
	walk_down
	set_invisible
	step_end

JohtoBT_Movement_ElevatorPlayerExit:
	walk_right
	walk_down
	walk_down
	step_end


@@ The hallway (Crystal's BattleTowerHallway: the room the level group battles in, "at least it should look like that")

JohtoBT_Hallway_OnLoad::
	jbt JBT_FUNC_GET_GROUP
	goto_if_ge VAR_RESULT, 6, JohtoBT_Hallway_EventScript_OpenFarDoor
	setmetatile 12, 0, METATILE_BattleFrontier_CorridorOpenDoor_Top, FALSE
	setmetatile 12, 1, METATILE_BattleFrontier_CorridorOpenDoor_Bottom, FALSE
	end

JohtoBT_Hallway_EventScript_OpenFarDoor::
	setmetatile 15, 0, METATILE_BattleFrontier_CorridorOpenDoor_Top, FALSE
	setmetatile 15, 1, METATILE_BattleFrontier_CorridorOpenDoor_Bottom, FALSE
	end

JohtoBT_Hallway_OnFrame::
	map_script_2 VAR_TEMP_0, 0, JohtoBT_Hallway_EventScript_ChooseBattleRoom
	.2byte 0

JohtoBT_Hallway_EventScript_ChooseBattleRoom::
	setvar VAR_TEMP_0, 1
	jbt JBT_FUNC_GET_GROUP
	goto_if_ge VAR_RESULT, 6, JohtoBT_Hallway_EventScript_FarRoom
	applymovement LOCALID_JBT_RECEPTIONIST, JohtoBT_Movement_HallwayReceptionistToDoor
	applymovement LOCALID_PLAYER, JohtoBT_Movement_HallwayPlayerToDoor
	waitmovement 0
	goto JohtoBT_Hallway_EventScript_EnterBattleRoom

JohtoBT_Hallway_EventScript_FarRoom::
	applymovement LOCALID_JBT_RECEPTIONIST, JohtoBT_Movement_HallwayReceptionistToFarDoor
	applymovement LOCALID_PLAYER, JohtoBT_Movement_HallwayPlayerToFarDoor
	waitmovement 0
JohtoBT_Hallway_EventScript_EnterBattleRoom::
	msgbox JohtoBT_Text_PleaseStepThisWay
	closemessage
	applymovement LOCALID_PLAYER, JohtoBT_Movement_HallwayPlayerEnterRoom
	waitmovement 0
	warp MAP_JOHTO_BATTLE_TOWER_BATTLE_ROOM, 4, 8
	waitstate
	end

JohtoBT_Movement_HallwayPlayerToFarDoor:
	walk_right
JohtoBT_Movement_HallwayReceptionistToFarDoor:
	walk_right
	walk_right
JohtoBT_Movement_HallwayPlayerToDoor:
	walk_right
JohtoBT_Movement_HallwayReceptionistToDoor:
	walk_right
	walk_right
	walk_right
	face_left
	step_end

JohtoBT_Movement_HallwayPlayerEnterRoom:
	face_up
	walk_up
	set_invisible
	step_end


@@ The BATTLE ROOM (Crystal's BattleTowerBattleRoom: seven opponents in a row)

JohtoBT_Room_OnTransition::
	setflag FLAG_TEMP_1
	end

JohtoBT_Room_OnFrame::
	map_script_2 VAR_TEMP_0, 0, JohtoBT_Room_EventScript_Enter
	.2byte 0

JohtoBT_Room_EventScript_Enter::
	setvar VAR_TEMP_0, 1
	applymovement LOCALID_PLAYER, JohtoBT_Movement_RoomPlayerWalksIn
	waitmovement 0
JohtoBT_Room_EventScript_Loop::
	jbt JBT_FUNC_LOAD_OPPONENT
	clearflag FLAG_TEMP_1
	addobject LOCALID_JBT_OPPONENT
	playse SE_WARP_IN
	waitse
	applymovement LOCALID_JBT_OPPONENT, JohtoBT_Movement_RoomOpponentWalksIn
	waitmovement 0
	jbt JBT_FUNC_GET_TEXT, 1
	msgbox gStringVar4
	closemessage
	jbt JBT_FUNC_DO_BATTLE
	waitstate
	goto_if_ne VAR_RESULT, 0, JohtoBT_Room_EventScript_Failed
	jbt JBT_FUNC_GET_BEATEN
	goto_if_ge VAR_RESULT, BATTLETOWER_STREAK_LENGTH, JohtoBT_Room_EventScript_BeatenAll
	applymovement LOCALID_JBT_OPPONENT, JohtoBT_Movement_RoomOpponentWalksOut
	waitmovement 0
	playse SE_WARP_OUT
	removeobject LOCALID_JBT_OPPONENT
	setflag FLAG_TEMP_1
	applymovement LOCALID_JBT_ROOM_RECEPTIONIST, JohtoBT_Movement_RoomReceptionistWalksToPlayer
	waitmovement 0
	applymovement LOCALID_PLAYER, JohtoBT_Movement_RoomPlayerFacesReceptionist
	waitmovement 0
	msgbox JohtoBT_Text_YourMonWillBeHealedToFullHealth
	closemessage
	playfanfare MUS_HEAL
	fadescreen FADE_TO_WHITE
	special HealPlayerParty
	waitfanfare
	fadescreen FADE_FROM_WHITE
	jbt JBT_FUNC_GET_BEATEN
JohtoBT_Room_EventScript_AskNext::
	msgbox JohtoBT_Text_NextUpOpponentNo, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoBT_Room_EventScript_DontBattleNext
JohtoBT_Room_EventScript_ContinueToNext::
	closemessage
	applymovement LOCALID_PLAYER, JohtoBT_Movement_RoomPlayerFacesOpponent
	applymovement LOCALID_JBT_ROOM_RECEPTIONIST, JohtoBT_Movement_RoomReceptionistWalksAway
	waitmovement 0
	goto JohtoBT_Room_EventScript_Loop

JohtoBT_Room_EventScript_DontBattleNext::
	msgbox JohtoBT_Text_SaveAndEndTheSession, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoBT_Room_EventScript_DontSave
	setdynamicwarp MAP_JOHTO_BATTLE_TOWER_1F, 6, 6
	message JohtoBT_Text_SavingDontTurnOff
	waitmessage
	jbt JBT_FUNC_SAVE_AND_QUIT
	playse SE_SAVE
	waitse
	fadescreen FADE_TO_WHITE
	frontier_reset
	end

JohtoBT_Room_EventScript_DontSave::
	msgbox JohtoBT_Text_CancelYourBattleRoomChallenge, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoBT_Room_EventScript_ContinueToNext
	jbt JBT_FUNC_SET_STATE, JBT_BACK_CANCELED
	closemessage
	fadescreen FADE_TO_WHITE
	warp MAP_JOHTO_BATTLE_TOWER_1F, 6, 6
	waitstate
	end

JohtoBT_Room_EventScript_Failed::
	delay 60
	jbt JBT_FUNC_SET_STATE, JBT_BACK_LOST
	warp MAP_JOHTO_BATTLE_TOWER_1F, 6, 6
	waitstate
	end

JohtoBT_Room_EventScript_BeatenAll::
	delay 60
	jbt JBT_FUNC_SET_STATE, JBT_WON_CHALLENGE
	warp MAP_JOHTO_BATTLE_TOWER_1F, 6, 6
	waitstate
	end

JohtoBT_Movement_RoomPlayerWalksIn:
	walk_up
	walk_up
	walk_up
	face_right
	step_end

JohtoBT_Movement_RoomOpponentWalksIn:
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	face_left
	step_end

JohtoBT_Movement_RoomOpponentWalksOut:
	face_up
	walk_slow_up
	walk_slow_up
	walk_slow_up
	walk_slow_up
	step_end

JohtoBT_Movement_RoomReceptionistWalksToPlayer:
	walk_slow_right
	walk_slow_right
	walk_slow_right
	walk_slow_up
	step_end

JohtoBT_Movement_RoomReceptionistWalksAway:
	walk_slow_down
	walk_slow_left
	walk_slow_left
	walk_slow_left
	face_right
	step_end

JohtoBT_Movement_RoomPlayerFacesReceptionist:
	face_down
	step_end

JohtoBT_Movement_RoomPlayerFacesOpponent:
	face_right
	step_end


@@ Crystal's words (tools/quetzal_port/restore_words.txt)

JohtoBT_Text_BattleTowerWelcomesYou::
	.string "BATTLE TOWER welcomes you!\p"
	.string "I could show you to a BATTLE ROOM.$"

JohtoBT_Text_WantToGoIntoABattleRoom::
	.string "Want to go into a BATTLE ROOM?$"

JohtoBT_Text_RightThisWayToYourBattleRoom::
	.string "Right this way to your BATTLE\n"
	.string "ROOM.$"

JohtoBT_Text_BattleTowerIntroduction::
	.string "BATTLE TOWER is a facility made\n"
	.string "for MOéMON battles.\p"
	.string "Countless MOéMON trainers gather\n"
	.string "from all over to hold battles in\l"
	.string "specially designed BATTLE ROOMS.\p"
	.string "There are many BATTLE ROOMS in the\n"
	.string "BATTLE TOWER.\p"
	.string "Each ROOM holds seven trainers.\p"
	.string "Beat them all, and win a prize.\p"
	.string "To interrupt a session, you must\n"
	.string "SAVE. If not, you won’t be able to\l"
	.string "resume your ROOM challenge.$"

JohtoBT_Text_ThanksForVisiting::
	.string "Thanks for visiting!$"

JohtoBT_Text_CongratulationsYouveBeatenAllTheTrainers::
	.string "Congratulations!\p"
	.string "You’ve beaten all the trainers!\p"
	.string "For that, you get this great\n"
	.string "prize!$"

JohtoBT_Text_PlayerGotFive::
	.string "{PLAYER} got five {STR_VAR_2}!$"

JohtoBT_Text_YourPackIsStuffedFull::
	.string "Oops, your PACK is stuffed full.\p"
	.string "Please make room and come back.$"

JohtoBT_Text_WeHopeToServeYouAgain::
	.string "We hope to serve you again.$"

JohtoBT_Text_PleaseStepThisWay::
	.string "Please step this way.$"

JohtoBT_Text_WouldYouLikeToHearAboutTheBattleTower::
	.string "Would you like to hear about the\n"
	.string "BATTLE TOWER?$"

JohtoBT_Text_ReadBattleTowerRules::
	.string "BATTLE TOWER rules are written\n"
	.string "here.\p"
	.string "Read the rules?$"

JohtoBT_Text_BattleTowerRules::
	.string "Three MOéMON may enter battles.\p"
	.string "All three must be different.\p"
	.string "The items they hold must also be\n"
	.string "different.\p"
	.string "Certain MOéMON may also have level\n"
	.string "restrictions placed on them.$"

JohtoBT_Text_LeftWithoutSaving::
	.string "Excuse me! You didn’t SAVE before\n"
	.string "exiting the BATTLE ROOM.\p"
	.string "I’m awfully sorry, but your\n"
	.string "challenge will be declared\l"
	.string "invalid.$"

JohtoBT_Text_YourMonWillBeHealedToFullHealth::
	.string "Your MOéMON will be healed to full\n"
	.string "health.$"

JohtoBT_Text_NextUpOpponentNo::
	.string "Next up, opponent no.{STR_VAR_1}.\n"
	.string "Ready?$"

JohtoBT_Text_SaveBeforeEnteringBattleRoom::
	.string "Before entering the BATTLE ROOM,\n"
	.string "your progress will be saved.$"

JohtoBT_Text_SaveAndEndTheSession::
	.string "SAVE and end the session?$"

JohtoBT_Text_CancelYourBattleRoomChallenge::
	.string "Cancel your BATTLE ROOM challenge?$"

JohtoBT_Text_WeveBeenWaitingForYou::
	.string "We’ve been waiting for you. This\n"
	.string "way to a BATTLE ROOM, please.$"

JohtoBT_Text_AMonLevelExceeds::
	.string "One or more of your MOéMON’s\n"
	.string "levels exceeds {STR_VAR_2}.$"

JohtoBT_Text_MayNotEnterABattleRoomUnderL70::
	.string "{STR_VAR_1} may not enter a BATTLE\n"
	.string "ROOM under L70.\p"
	.string "This BATTLE ROOM is for\n"
	.string "L{STR_VAR_2}.$"

JohtoBT_1F_Text_Youngster::
	.string "Destroyed by the first opponent in\n"
	.string "no time at all… I’m no good…$"

JohtoBT_1F_Text_CooltrainerF::
	.string "There are lots of BATTLE ROOMS,\n"
	.string "but I’m going to win them all!$"

JohtoBT_1F_Text_Granny::
	.string "It’s a grueling task, not being\n"
	.string "able to use items in battle.\p"
	.string "Making your MOéMON hold items is\n"
	.string "the key to winning battles.$"

JohtoBT_1F_Text_BugCatcher::
	.string "I’m trying to see how far I can go\n"
	.string "using just bug MOéMON.\p"
	.string "Don’t let there be any fire\n"
	.string "MOéMON…$"

JohtoBT_Text_ExcuseMeYoureNotReady::
	.string "Excuse me. You’re not ready.$"

JohtoBT_Text_ReturnWhenReady::
	.string "Please return when you’re ready.$"

JohtoBT_Text_OnlyThreeMonMayBeEntered::
	.string "Only three MOéMON may be entered.$"

JohtoBT_Text_TheMonMustAllBeDifferentKinds::
	.string "The {STR_VAR_2} MOéMON must all be\n"
	.string "different kinds.$"

JohtoBT_Text_TheMonMustNotHoldTheSameItems::
	.string "The {STR_VAR_2} MOéMON must not\n"
	.string "hold the same items.$"

JohtoBT_Text_YouCantTakeAnEgg::
	.string "You can’t take an EGG!$"

JohtoBT_Text_SavingDontTurnOff::
	.string "SAVING… DON’T TURN OFF THE POWER.$"

JohtoBT_Outside_Text_Youngster::
	.string "Wow, the BATTLE TOWER is huge!\p"
	.string "There must be many kinds of MOéMON\n"
	.string "in there!$"

JohtoBT_Outside_Text_Beauty::
	.string "You can use only three MOéMON.\p"
	.string "It’s so hard to decide which three\n"
	.string "should go into battle…$"

JohtoBT_Outside_Text_Sailor::
	.string "Hehehe, I snuck out from work.\p"
	.string "I can’t bail out until I’ve won!\p"
	.string "I have to win it all. That I must!$"

JohtoBT_Outside_Text_Sign::
	.string "BATTLE TOWER\p"
	.string "Take the Ultimate Trainer\n"
	.string "Challenge!$"

JohtoBT_Gate_Text_Rocker::
	.string "Are you going to the BATTLE TOWER?\p"
	.string "This is a secret, but if you win a\n"
	.string "whole lot, you can win special\l"
	.string "gifts.$"

JohtoBT_Gate_Text_Twin::
	.string "The levels of the MOéMON I want to\n"
	.string "use are all different.\p"
	.string "I have to go train them now!$"
