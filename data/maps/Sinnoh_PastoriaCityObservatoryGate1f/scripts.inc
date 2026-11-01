Sinnoh_PastoriaCityObservatoryGate1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_PastoriaCityObservatoryGate1f_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Sinnoh_PastoriaCityObservatoryGate1f_OnFrame
	.byte 0

@ The GREAT MARSH's SAFARI GAME (Emerald's: data/scripts/safari_zone.inc ends it here, src/sinnoh/s3d_legends.c)
Sinnoh_PastoriaCityObservatoryGate1f_OnTransition:
	goto_if_unset FLAG_SYS_SAFARI_MODE, Sinnoh_PastoriaCityObservatoryGate1f_OnTransitionEnd
	setvar VAR_SAFARI_ZONE_STATE, 3
Sinnoh_PastoriaCityObservatoryGate1f_OnTransitionEnd:
	end

Sinnoh_PastoriaCityObservatoryGate1f_OnFrame:
	map_script_2 VAR_SAFARI_ZONE_STATE, 1, Sinnoh_PastoriaCityObservatoryGate1f_EventScript_BackFromMarsh
	map_script_2 VAR_SAFARI_ZONE_STATE, 3, Sinnoh_PastoriaCityObservatoryGate1f_EventScript_WalkedOut
	.short 0

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_BackFromMarsh::
	lockall
	applymovement LOCALID_PLAYER, Sinnoh_PastoriaCityObservatoryGate1f_Movement_AwayFromDoor
	waitmovement 0
	setvar VAR_SAFARI_ZONE_STATE, 0
	releaseall
	end

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_WalkedOut::
	lockall
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_LeaveMarsh, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_PastoriaCityObservatoryGate1f_EventScript_BackIn
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_Collected, MSGBOX_DEFAULT
	closemessage
	setvar VAR_SAFARI_ZONE_STATE, 1
	special ExitSafariMode
	goto Sinnoh_PastoriaCityObservatoryGate1f_EventScript_BackFromMarsh

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_BackIn::
	closemessage
	setvar VAR_SAFARI_ZONE_STATE, 0
	warp MAP_SINNOH_GREAT_MARSH6, 68, 117
	waitstate
	end

Sinnoh_PastoriaCityObservatoryGate1f_Movement_AwayFromDoor:
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Trigger1::
	lockall
	applymovement LOCALID_PLAYER, Sinnoh_PastoriaCityObservatoryGate1f_Movement_FaceCounter
	waitmovement 0
	showmoneybox 0, 0
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_Welcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_PastoriaCityObservatoryGate1f_EventScript_NotNow
	getpartysize
	goto_if_ne VAR_RESULT, PARTY_SIZE, Sinnoh_PastoriaCityObservatoryGate1f_EventScript_HasRoom
	specialvar VAR_RESULT, ScriptCheckFreePokemonStorageSpace
	goto_if_eq VAR_RESULT, 1, Sinnoh_PastoriaCityObservatoryGate1f_EventScript_HasRoom
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_NoRoom, MSGBOX_DEFAULT
	goto Sinnoh_PastoriaCityObservatoryGate1f_EventScript_StepBack

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_HasRoom::
	checkmoney 500
	goto_if_eq VAR_RESULT, 0, Sinnoh_PastoriaCityObservatoryGate1f_EventScript_NoMoney
	playse SE_SHOP
	removemoney 500
	updatemoneybox
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_Paid, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Sinnoh_PastoriaCityObservatoryGate1f_Text_Received
	waitfanfare
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_HaveFun, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	special EnterSafariMode
	setvar VAR_SAFARI_ZONE_STATE, 0
	warp MAP_SINNOH_GREAT_MARSH6, 68, 117
	waitstate
	end

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_NoMoney::
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_NoMoney, MSGBOX_DEFAULT
	goto Sinnoh_PastoriaCityObservatoryGate1f_EventScript_StepBack

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_NotNow::
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_NotNow, MSGBOX_DEFAULT
Sinnoh_PastoriaCityObservatoryGate1f_EventScript_StepBack::
	closemessage
	hidemoneybox
	applymovement LOCALID_PLAYER, Sinnoh_PastoriaCityObservatoryGate1f_Movement_StepBack
	waitmovement 0
	releaseall
	end

Sinnoh_PastoriaCityObservatoryGate1f_Movement_StepBack:
	walk_down
	step_end

Sinnoh_PastoriaCityObservatoryGate1f_Movement_FaceCounter:
	walk_in_place_faster_left
	step_end

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Person1::
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Person2::
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PastoriaCityObservatoryGate1f_Text_Welcome:
	.string "Welcome to the GREAT MARSH!\n"
	.string "The SAFARI GAME is ¥500.\p"
	.string "You get 30 SAFARI BALLS and\n"
	.string "500 steps in the marsh. Play?$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_Paid:
	.string "¥500 it is! Here are your\n"
	.string "SAFARI BALLS.$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_Received:
	.string "{PLAYER} received 30 SAFARI BALLS.$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_HaveFun:
	.string "Mind your step out there.\n"
	.string "The mud is deeper than it looks!$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_NotNow:
	.string "Whenever you're ready, then.$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_NoMoney:
	.string "Oh dear, you're a little short.\n"
	.string "It's ¥500 to play.$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_NoRoom:
	.string "Your party and your PC BOXES\n"
	.string "are full. Make some room first!$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_LeaveMarsh:
	.string "Leaving the GREAT MARSH already?\n"
	.string "Your SAFARI GAME will end.$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_Collected:
	.string "Thanks for playing! Your SAFARI\n"
	.string "BALLS are collected here.$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_Person1:
	.string "The SAFARI GAME's counter is right\n"
	.string "here. Just walk up to play!$"

Sinnoh_PastoriaCityObservatoryGate1f_Text_Person2:
	.string "The marsh's Pokemon change with\n"
	.string "where you are. Explore all of it!$"

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Person3::
	msgbox Sinnoh_PastoriaCityObservatoryGate1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PastoriaCityObservatoryGate1f_Text_Person3:
	.string "Go up to the observatory! You can see\n"
	.string "the whole GREAT MARSH from there.$"

@ --- end of SINNOH words ---
