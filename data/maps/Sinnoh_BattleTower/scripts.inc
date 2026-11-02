Sinnoh_BattleTower_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Sinnoh_BattleTower_OnFrame
	.byte 0

@ Moemon Kai: the SINNOH BATTLE TOWER's single battles run on Emerald's tower (its records, BP and trainers), in
@ Platinum's rooms: the counter here, then the battle room (data/maps/Sinnoh_BattleTowerBattleRoom). The words are
@ Platinum's own (SinnohP_T_<bank>_<line>: its battle tower's lines, bank 29, and the battle room's, 30; menus 263).
Sinnoh_BattleTower_OnFrame:
	map_script_2 VAR_TEMP_CHALLENGE_STATUS, 0, Sinnoh_BattleTower_EventScript_GetStatus
	map_script_2 VAR_TEMP_CHALLENGE_STATUS, CHALLENGE_STATUS_SAVING, Sinnoh_BattleTower_EventScript_Quit
	map_script_2 VAR_TEMP_CHALLENGE_STATUS, CHALLENGE_STATUS_PAUSED, Sinnoh_BattleTower_EventScript_Quit
	map_script_2 VAR_TEMP_CHALLENGE_STATUS, CHALLENGE_STATUS_WON, Sinnoh_BattleTower_EventScript_Won
	map_script_2 VAR_TEMP_CHALLENGE_STATUS, CHALLENGE_STATUS_LOST, Sinnoh_BattleTower_EventScript_Lost
	.short 0

Sinnoh_BattleTower_EventScript_GetStatus::
	frontier_getstatus
	end

Sinnoh_BattleTower_EventScript_Quit::
	lockall
	msgbox SinnohP_T_29_19, MSGBOX_DEFAULT
	closemessage
	tower_set TOWER_DATA_WIN_STREAK, 0
	tower_set TOWER_DATA_WIN_STREAK_ACTIVE, FALSE
	frontier_set FRONTIER_DATA_CHALLENGE_STATUS, 0
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
	releaseall
	end

Sinnoh_BattleTower_EventScript_Won::
	lockall
	message SinnohP_T_30_19
	waitmessage
	tower_giveribbons
	frontier_givepoints
	sinnoh 61, 25, 0, 0
	msgbox SinnohP_T_30_21, MSGBOX_GETPOINTS
	call Sinnoh_BattleTower_EventScript_SaveRecord
	msgbox SinnohP_T_29_2, MSGBOX_DEFAULT
	closemessage
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
	releaseall
	end

Sinnoh_BattleTower_EventScript_Lost::
	tower_set TOWER_DATA_WIN_STREAK_ACTIVE, FALSE
	lockall
	message SinnohP_T_29_53
	waitmessage
	message SinnohP_T_29_69
	waitmessage
	call Sinnoh_BattleTower_EventScript_SaveRecord
	msgbox SinnohP_T_29_2, MSGBOX_DEFAULT
	closemessage
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
	releaseall
	end

Sinnoh_BattleTower_EventScript_SaveRecord::
	special LoadPlayerParty
	special HealPlayerParty
	tower_save 0
	playse SE_SAVE
	waitse
	return

@ The single battle counter
Sinnoh_BattleTower_EventScript_Singles::
	lock
	faceplayer
	setvar VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER
	special SavePlayerParty
	msgbox SinnohP_T_29_0, MSGBOX_DEFAULT
Sinnoh_BattleTower_EventScript_Ask::
	message SinnohP_T_29_1
	waitmessage
	sinnoh 13, VAR_RESULT, 1, 0
	sinnoh 14, 263, 141, 0
	sinnoh 14, 263, 142, 1
	sinnoh 14, 263, 143, 2
	sinnoh 15, 0, 0, 0
	switch VAR_RESULT
	case 0, Sinnoh_BattleTower_EventScript_Try
	case 1, Sinnoh_BattleTower_EventScript_Info
	goto Sinnoh_BattleTower_EventScript_Cancel

Sinnoh_BattleTower_EventScript_Info::
	msgbox SinnohP_T_29_3, MSGBOX_DEFAULT
	goto Sinnoh_BattleTower_EventScript_Ask

Sinnoh_BattleTower_EventScript_Try::
	@ (Platinum's question: SINGLE or DOUBLE; its rooms scale every POKéMON to level 50, which Emerald's tower doesn't
	@ yet: OPEN LEVEL until the full port, docs/SINNOH_CHECKLIST.md)
	message SinnohP_T_29_4
	waitmessage
	sinnoh 13, VAR_RESULT, 1, 0
	sinnoh 14, 263, 6, 0
	sinnoh 14, 263, 7, 1
	sinnoh 14, 263, 94, 2
	sinnoh 15, 0, 0, 0
	goto_if_ge VAR_RESULT, 2, Sinnoh_BattleTower_EventScript_Cancel
	copyvar VAR_FRONTIER_BATTLE_MODE, VAR_RESULT
	setvar VAR_RESULT, FRONTIER_LVL_OPEN
	frontier_checkineligible
	goto_if_eq VAR_0x8004, TRUE, Sinnoh_BattleTower_EventScript_NotEnough
	frontier_set FRONTIER_DATA_LVL_MODE, VAR_RESULT
	msgbox SinnohP_T_29_8, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	frontier_get FRONTIER_DATA_LVL_MODE
	copyvar VAR_0x8004, VAR_RESULT
	setvar VAR_0x8005, FRONTIER_PARTY_SIZE
	goto_if_ne VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_DOUBLES, Sinnoh_BattleTower_EventScript_Choose
	setvar VAR_0x8005, FRONTIER_DOUBLES_PARTY_SIZE
Sinnoh_BattleTower_EventScript_Choose::
	special ChoosePartyForBattleFrontier
	goto_if_eq VAR_RESULT, 0, Sinnoh_BattleTower_EventScript_LoadCancel
	msgbox SinnohP_T_29_16, MSGBOX_DEFAULT
	frontier_set FRONTIER_DATA_SELECTED_MON_ORDER
	setvar VAR_TEMP_CHALLENGE_STATUS, 0
	tower_init
	tower_set TOWER_DATA_WIN_STREAK_ACTIVE, TRUE
	frontier_set FRONTIER_DATA_PAUSED, FALSE
	special LoadPlayerParty
	closemessage
	delay 2
	call Common_EventScript_SaveGame
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
	goto_if_eq VAR_RESULT, 0, Sinnoh_BattleTower_EventScript_SaveFailed
	incrementgamestat GAME_STAT_ENTERED_BATTLE_TOWER
	special SavePlayerParty
	setvar VAR_0x8004, FRONTIER_UTIL_FUNC_SET_PARTY_ORDER
	setvar VAR_0x8005, FRONTIER_PARTY_SIZE
	goto_if_ne VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_DOUBLES, Sinnoh_BattleTower_EventScript_Order
	setvar VAR_0x8005, FRONTIER_DOUBLES_PARTY_SIZE
Sinnoh_BattleTower_EventScript_Order::
	special CallFrontierUtilFunc
	msgbox SinnohP_T_29_11, MSGBOX_DEFAULT
	closemessage
	clearflag FLAG_CANCEL_BATTLE_ROOM_CHALLENGE
	warp MAP_SINNOH_BATTLE_TOWER_BATTLE_ROOM, 7, 10
	setvar VAR_TEMP_CHALLENGE_STATUS, 0
	waitstate
	end

Sinnoh_BattleTower_EventScript_SaveFailed::
	frontier_set FRONTIER_DATA_CHALLENGE_STATUS
	goto Sinnoh_BattleTower_EventScript_Cancel

Sinnoh_BattleTower_EventScript_NotEnough::
	buffernumberstring STR_VAR_1, 3
	msgbox SinnohP_T_29_91, MSGBOX_DEFAULT
	goto Sinnoh_BattleTower_EventScript_Cancel

Sinnoh_BattleTower_EventScript_LoadCancel::
	special LoadPlayerParty
Sinnoh_BattleTower_EventScript_Cancel::
	msgbox SinnohP_T_29_2, MSGBOX_DEFAULT
	release
	end















@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_BattleTower_EventScript_Bg2::
	msgbox Sinnoh_BattleTower_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_BattleTower_EventScript_Bg3::
	msgbox Sinnoh_BattleTower_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_BattleTower_EventScript_Person1::
	goto Sinnoh_BattleTower_EventScript_Singles
	end

Sinnoh_BattleTower_EventScript_Person2::
	msgbox Sinnoh_BattleTower_Text_Person2, MSGBOX_NPC
	end

Sinnoh_BattleTower_EventScript_Person5::
	goto SinnohP_battle_tower_19
	end

Sinnoh_BattleTower_EventScript_Person6::
	goto SinnohP_battle_tower_15
	end

Sinnoh_BattleTower_EventScript_Person7::
	goto SinnohP_battle_tower_16
	end

Sinnoh_BattleTower_EventScript_Person8::
	goto SinnohP_battle_tower_17
	end

Sinnoh_BattleTower_EventScript_Person9::
	goto SinnohP_battle_tower_18
	end

Sinnoh_BattleTower_EventScript_Person10::
	goto SinnohP_battle_tower_21
	end

Sinnoh_BattleTower_EventScript_Person11::
	goto SinnohP_battle_tower_20
	end

Sinnoh_BattleTower_EventScript_Person12::
	goto SinnohP_battle_tower_14
	end

Sinnoh_BattleTower_EventScript_Person13::
	msgbox Sinnoh_BattleTower_Text_Person13, MSGBOX_NPC
	end

Sinnoh_BattleTower_EventScript_Person15::
	msgbox Sinnoh_BattleTower_Text_Person15, MSGBOX_NPC
	end

Sinnoh_BattleTower_Text_Bg2:
	.string "A machine that keeps records. Its\n"
	.string "screen is dark for now.$"

Sinnoh_BattleTower_Text_Bg3:
	.string "A machine that keeps records. Its\n"
	.string "screen is dark for now.$"

Sinnoh_BattleTower_Text_Person2:
	.string "Welcome to the BATTLE TOWER. Battles\n"
	.string "with a partner aren't open yet. The\l"
	.string "single battle counter is!$"

Sinnoh_BattleTower_Text_Person13:
	.string "I'm covering the BATTLE TOWER's\n"
	.string "records. Some of these streaks are\l"
	.string "legendary.$"

Sinnoh_BattleTower_Text_Person15:
	.string "I can tell how your MOéMON's stats\n"
	.string "grew. Come back once the tower is\l"
	.string "open!$"

@ --- end of SINNOH words ---
