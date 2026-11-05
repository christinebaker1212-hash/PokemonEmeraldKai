@ Moemon Kai: FireRed's SAFARI ZONE entrance (pokefirered data/maps/FuchsiaCity_SafariZone_Entrance; audit case 62): the
@ SAFARI GAME on the game's own engine (EnterSafariMode / ExitSafariMode, src/safari_zone.c: 600 steps in KANTO, 30
@ SAFARI BALLS), FireRed's words. VAR_KANTO_SAFARI_SCENE is FireRed's VAR_MAP_SCENE_FUCHSIA_CITY_SAFARI_ZONE_ENTRANCE.
Kanto_FuchsiaCity_SafariZone_Entrance_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_FuchsiaCity_SafariZone_Entrance_OnFrame
	.byte 0

Kanto_FuchsiaCity_SafariZone_Entrance_OnFrame::
	map_script_2 VAR_KANTO_SAFARI_SCENE, 1, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExitWarpIn
	map_script_2 VAR_KANTO_SAFARI_SCENE, 2, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExitEarly
	map_script_2 VAR_KANTO_SAFARI_SCENE, 3, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExitWalkIn
	.2byte 0

@ When player runs out of balls mid-battle
Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExitWalkIn::
	lockall
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_ApproachCounter
	waitmovement 0
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_CatchFairShareComeAgain
	closemessage
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_Exit2
	waitmovement 0
	special ExitSafariMode
	setvar VAR_KANTO_SAFARI_SCENE, 0
	releaseall
	end

@ When player runs of out balls after catching a pokemon, or runs out of steps
Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExitWarpIn::
	lockall
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_CatchFairShareComeAgain
	closemessage
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_Exit
	waitmovement 0
	special ExitSafariMode
	setvar VAR_KANTO_SAFARI_SCENE, 0
	releaseall
	end

@ When player re-enters the entrance building with balls/steps remaining
Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExitEarly::
	lockall
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_GoingToLeaveSafariZoneEarly, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ReturnToSafariZone
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_PleaseReturnSafariBalls
	closemessage
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_Exit
	waitmovement 0
	special ExitSafariMode
	setvar VAR_KANTO_SAFARI_SCENE, 0
	releaseall
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ReturnToSafariZone::
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_GoodLuck
	closemessage
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_ReEnter
	waitmovement 0
	warp MAP_KANTO_SAFARI_ZONE_CENTER, 26, 30
	waitstate
	end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_Exit::
	walk_down
	walk_down
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_ReEnter::
	walk_up
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_Exit2::
	walk_down
	walk_down
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_ApproachCounter::
	walk_down
	delay_8
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EntryTriggerMid::
	lockall
	setvar VAR_TEMP_2, 0
	goto Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_AskEnterSafariZone
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EntryTriggerRight::
	lockall
	setvar VAR_TEMP_2, 1
	goto Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_AskEnterSafariZone
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EntryTriggerLeft::
	lockall
	setvar VAR_TEMP_2, 2
	goto Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_AskEnterSafariZone
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_AskEnterSafariZone::
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_WelcomeToSafariZone
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_FaceCounter
	waitmovement 0
	showmoneybox 0, 0
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_PlaySafariGameFor500, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_TryEnterSafariZone
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_OkayPleaseComeAgain
	goto Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ForcePlayerBack
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_TryEnterSafariZone::
	call Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_CheckSpaceForMons
	checkmoney 500
	goto_if_eq VAR_RESULT, FALSE, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_NotEnoughMoney
	removemoney 500
	updatemoneybox
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_ThatllBe500WeOnlyUseSpecialBalls
	playfanfare MUS_LEVEL_UP
	message Kanto_FuchsiaCity_SafariZone_Entrance_Text_PlayerReceived30SafariBalls
	waitfanfare
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_CallYouOnPAWhenYouRunOut
	closemessage
	hidemoneybox
	call_if_eq VAR_TEMP_2, 0, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneMid
	call_if_eq VAR_TEMP_2, 1, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneRight
	call_if_eq VAR_TEMP_2, 2, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneLeft
	special EnterSafariMode
	setvar VAR_KANTO_SAFARI_SCENE, 2
	warp MAP_KANTO_SAFARI_ZONE_CENTER, 255, 26, 30
	waitstate
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneRight::
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneRight
	waitmovement 0
	return

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneMid::
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneMid
	waitmovement 0
	return

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_EnterSafariZoneLeft::
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneLeft
	waitmovement 0
	return

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_Return::
	return

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_CheckSpaceForMons::
	getpartysize
	goto_if_ne VAR_RESULT, PARTY_SIZE, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_Return
	specialvar VAR_RESULT, ScriptCheckFreePokemonStorageSpace
	goto_if_eq VAR_RESULT, TRUE, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_Return
	msgbox SafariZone_Text_ExcuseMeYourPCBoxIsFullKanto
	goto Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ForcePlayerBack
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_NotEnoughMoney::
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_OopsNotEnoughMoney
	goto Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ForcePlayerBack
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ForcePlayerBack::
	closemessage
	hidemoneybox
	applymovement LOCALID_PLAYER, Kanto_FuchsiaCity_SafariZone_Entrance_Movement_ForceBack
	waitmovement 0
	releaseall
	end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_FaceCounter::
	walk_in_place_faster_right
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_ForceBack::
	walk_down
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneMid::
	walk_up
	walk_up
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneRight::
	walk_up
	walk_left
	walk_up
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_Movement_EnterSafariZoneLeft::
	walk_up
	walk_right
	walk_up
	step_end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_841D640::
Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_InfoAttendant::
	lock
	faceplayer
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_FirstTimeAtSafariZone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExplainSafariZone
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_SorryYoureARegularHere
	release
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_ExplainSafariZone::
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_ExplainSafariZone
	release
	end

Kanto_FuchsiaCity_SafariZone_Entrance_Text_CatchFairShareComeAgain::
	.string "Did you catch your fair share?\n"
	.string "Come again!$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_GoingToLeaveSafariZoneEarly::
	.string "Are you going to leave the\n"
	.string "SAFARI ZONE early?$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_PleaseReturnSafariBalls::
	.string "Please return any SAFARI BALLS\n"
	.string "you may have left.$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_GoodLuck::
	.string "Good luck!$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_WelcomeToSafariZone::
	.string "Welcome to the SAFARI ZONE!$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_PlaySafariGameFor500::
	.string "For just ¥500, you can play the\n"
	.string "SAFARI GAME.\p"
	.string "You can roam the wide-open safari\n"
	.string "and catch what you like.\p"
	.string "Would you like to play?$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_OkayPleaseComeAgain::
	.string "Okay.\n"
	.string "Please come again!$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_ThatllBe500WeOnlyUseSpecialBalls::
	.string "That’ll be ¥500, please!\p"
	.string "We only use a special kind of \n"
	.string "POKé BALLS here.$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_PlayerReceived30SafariBalls::
	.string "{PLAYER} received 30 SAFARI BALLS\n"
	.string "from the attendant.$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_CallYouOnPAWhenYouRunOut::
	.string "We’ll call you on the PA when you\n"
	.string "run out of time or SAFARI BALLS.\p"
	.string "Well, I’ll wish you the best of\n"
	.string "luck!$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_OopsNotEnoughMoney::
	.string "Oops!\n"
	.string "Not enough money!$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_FirstTimeAtSafariZone::
	.string "Hi! Is it your first time here at\n"
	.string "the SAFARI ZONE?$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_SorryYoureARegularHere::
	.string "Sorry, you’re a regular here!$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_ExplainSafariZone::
	.string "The SAFARI ZONE actually has\n"
	.string "four zones in it.\p"
	.string "Each zone has different kinds of\n"
	.string "MOéMON, even some rare ones.\p"
	.string "Use the supplied SAFARI BALLS to\n"
	.string "catch them.\p"
	.string "You can also throw BAIT or ROCKS\n"
	.string "besides the SAFARI BALLS.\p"
	.string "If you throw BAIT, the MOéMON\n"
	.string "becomes less likely to flee, but\l"
	.string "harder to catch.\p"
	.string "If you throw ROCKS, the MOéMON\n"
	.string "becomes more likely to flee, but\l"
	.string "easier to catch.\p"
	.string "When you run out of time or SAFARI\n"
	.string "BALLS, it’s game over for you!$"
