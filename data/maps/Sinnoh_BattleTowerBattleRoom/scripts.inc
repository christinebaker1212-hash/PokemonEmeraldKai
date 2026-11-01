Sinnoh_BattleTowerBattleRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Sinnoh_BattleTowerBattleRoom_OnFrame
	.byte 0

@ Moemon Kai: the SINNOH BATTLE TOWER's battle room (Emerald's tower battles: data/maps/Sinnoh_BattleTower). Its
@ people (tools/sinnoh/gen_maps.py OVERRIDES): 1 the attendant, 2 the opponent (VAR_OBJ_GFX_ID_0)
Sinnoh_BattleTowerBattleRoom_OnFrame:
	map_script_2 VAR_TEMP_0, 0, Sinnoh_BattleTowerBattleRoom_EventScript_Enter
	.short 0

Sinnoh_BattleTowerBattleRoom_EventScript_Enter::
	setvar VAR_TEMP_0, 1
	applymovement LOCALID_PLAYER, Sinnoh_BattleTowerBattleRoom_Movement_PlayerEnter
	waitmovement 0
Sinnoh_BattleTowerBattleRoom_EventScript_Opponent::
	tower_setopponent
	addobject 2
	applymovement 2, Sinnoh_BattleTowerBattleRoom_Movement_OpponentEnter
	waitmovement 0
	applymovement LOCALID_PLAYER, Sinnoh_BattleTowerBattleRoom_Movement_StepRight
	applymovement 2, Sinnoh_BattleTowerBattleRoom_Movement_StepLeft
	waitmovement 0
	tower_getopponentintro 0
	msgbox gStringVar4, MSGBOX_DEFAULT
	waitmessage
	call BattleFrontier_BattleTowerBattleRoom_EventScript_DoTowerBattle
	switch VAR_RESULT
	case B_OUTCOME_WON, Sinnoh_BattleTowerBattleRoom_EventScript_Won
	goto Sinnoh_BattleTowerBattleRoom_EventScript_Lost

Sinnoh_BattleTowerBattleRoom_EventScript_Won::
	call BattleFrontier_EventScript_IncrementWinStreak
	tower_setbattlewon
	switch VAR_RESULT
	case 7, Sinnoh_BattleTowerBattleRoom_EventScript_WarpWon
	applymovement 2, Sinnoh_BattleTowerBattleRoom_Movement_OpponentLeave
	waitmovement 0
	removeobject 2
	applymovement LOCALID_PLAYER, Sinnoh_BattleTowerBattleRoom_Movement_StepBack
	waitmovement 0
	applymovement 1, Sinnoh_BattleTowerBattleRoom_Movement_AttendantOut
	waitmovement 0
	msgbox Sinnoh_BattleTowerBattleRoom_Text_Heal, MSGBOX_DEFAULT
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
Sinnoh_BattleTowerBattleRoom_EventScript_AskNext::
	frontier_get FRONTIER_DATA_BATTLE_NUM
	addvar VAR_RESULT, 1
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Sinnoh_BattleTowerBattleRoom_Text_Next, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_BattleTowerBattleRoom_EventScript_Continue
	msgbox Sinnoh_BattleTowerBattleRoom_Text_Retire, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_BattleTowerBattleRoom_EventScript_Lost
	goto Sinnoh_BattleTowerBattleRoom_EventScript_AskNext

Sinnoh_BattleTowerBattleRoom_EventScript_Continue::
	closemessage
	applymovement 1, Sinnoh_BattleTowerBattleRoom_Movement_AttendantBack
	applymovement LOCALID_PLAYER, Sinnoh_BattleTowerBattleRoom_Movement_FaceRight
	waitmovement 0
	goto Sinnoh_BattleTowerBattleRoom_EventScript_Opponent

Sinnoh_BattleTowerBattleRoom_EventScript_Lost::
	frontier_set FRONTIER_DATA_CHALLENGE_STATUS, CHALLENGE_STATUS_LOST
	tower_set TOWER_DATA_LVL_MODE
	goto Sinnoh_BattleTowerBattleRoom_EventScript_Leave

Sinnoh_BattleTowerBattleRoom_EventScript_WarpWon::
	frontier_set FRONTIER_DATA_CHALLENGE_STATUS, CHALLENGE_STATUS_WON
	tower_set TOWER_DATA_LVL_MODE
Sinnoh_BattleTowerBattleRoom_EventScript_Leave::
	warp MAP_SINNOH_BATTLE_TOWER, 15, 6
	waitstate
	end

Sinnoh_BattleTowerBattleRoom_Movement_PlayerEnter:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	face_right
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_OpponentEnter:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	face_left
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_StepRight:
	walk_right
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_StepLeft:
	walk_left
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_OpponentLeave:
	walk_right
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_StepBack:
	walk_left
	face_left
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_FaceRight:
	face_right
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_AttendantOut:
	walk_right
	step_end

Sinnoh_BattleTowerBattleRoom_Movement_AttendantBack:
	walk_left
	face_right
	step_end

Sinnoh_BattleTowerBattleRoom_Text_Heal:
	.string "Well done! Let me restore your\n"
	.string "MOéMON.$"

Sinnoh_BattleTowerBattleRoom_Text_Next:
	.string "Your next opponent will be number\n"
	.string "{STR_VAR_1}. Are you ready?$"

Sinnoh_BattleTowerBattleRoom_Text_Retire:
	.string "Would you like to end your\n"
	.string "challenge here?$"
