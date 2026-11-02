Sinnoh_BattleTower_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Sinnoh_BattleTower_OnFrame
	.byte 0

@ Moemon Kai: the SINNOH BATTLE TOWER's single battles run on Emerald's tower (its records, BP and trainers), in
@ Platinum's rooms: the counter here, then the battle room (data/maps/Sinnoh_BattleTowerBattleRoom). The words are
@ this game's own.
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
	msgbox Sinnoh_BattleTower_Text_Quit, MSGBOX_DEFAULT
	closemessage
	tower_set TOWER_DATA_WIN_STREAK, 0
	tower_set TOWER_DATA_WIN_STREAK_ACTIVE, FALSE
	frontier_set FRONTIER_DATA_CHALLENGE_STATUS, 0
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
	releaseall
	end

Sinnoh_BattleTower_EventScript_Won::
	lockall
	message Sinnoh_BattleTower_Text_Won
	waitmessage
	tower_giveribbons
	msgbox Sinnoh_BattleTower_Text_Points, MSGBOX_DEFAULT
	frontier_givepoints
	msgbox Sinnoh_BattleTower_Text_GotPoints, MSGBOX_GETPOINTS
	call Sinnoh_BattleTower_EventScript_SaveRecord
	msgbox Sinnoh_BattleTower_Text_Again, MSGBOX_DEFAULT
	closemessage
	setvar VAR_TEMP_CHALLENGE_STATUS, 255
	releaseall
	end

Sinnoh_BattleTower_EventScript_Lost::
	tower_set TOWER_DATA_WIN_STREAK_ACTIVE, FALSE
	lockall
	message Sinnoh_BattleTower_Text_Lost
	waitmessage
	call Sinnoh_BattleTower_EventScript_SaveRecord
	msgbox Sinnoh_BattleTower_Text_Again, MSGBOX_DEFAULT
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
	msgbox Sinnoh_BattleTower_Text_Welcome, MSGBOX_DEFAULT
Sinnoh_BattleTower_EventScript_Ask::
	message Sinnoh_BattleTower_Text_Take
	waitmessage
	multichoice 17, 6, MULTI_CHALLENGEINFO, FALSE
	switch VAR_RESULT
	case 0, Sinnoh_BattleTower_EventScript_Try
	case 1, Sinnoh_BattleTower_EventScript_Info
	goto Sinnoh_BattleTower_EventScript_Cancel

Sinnoh_BattleTower_EventScript_Info::
	msgbox Sinnoh_BattleTower_Text_Info, MSGBOX_DEFAULT
	goto Sinnoh_BattleTower_EventScript_Ask

Sinnoh_BattleTower_EventScript_Try::
	setvar VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_SINGLES
	message Sinnoh_BattleTower_Text_Level
	waitmessage
	multichoice 17, 6, MULTI_LEVEL_MODE, FALSE
	switch VAR_RESULT
	case FRONTIER_LVL_TENT, Sinnoh_BattleTower_EventScript_Cancel
	case MULTI_B_PRESSED, Sinnoh_BattleTower_EventScript_Cancel
	frontier_checkineligible
	goto_if_eq VAR_0x8004, TRUE, Sinnoh_BattleTower_EventScript_NotEnough
	frontier_set FRONTIER_DATA_LVL_MODE, VAR_RESULT
	msgbox Sinnoh_BattleTower_Text_Pick, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	frontier_get FRONTIER_DATA_LVL_MODE
	copyvar VAR_0x8004, VAR_RESULT
	setvar VAR_0x8005, FRONTIER_PARTY_SIZE
	special ChoosePartyForBattleFrontier
	goto_if_eq VAR_RESULT, 0, Sinnoh_BattleTower_EventScript_LoadCancel
	msgbox Sinnoh_BattleTower_Text_Save, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, Sinnoh_BattleTower_EventScript_LoadCancel
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
	special CallFrontierUtilFunc
	msgbox Sinnoh_BattleTower_Text_ThisWay, MSGBOX_DEFAULT
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
	msgbox Sinnoh_BattleTower_Text_NotEnough, MSGBOX_DEFAULT
	goto Sinnoh_BattleTower_EventScript_Cancel

Sinnoh_BattleTower_EventScript_LoadCancel::
	special LoadPlayerParty
Sinnoh_BattleTower_EventScript_Cancel::
	msgbox Sinnoh_BattleTower_Text_Again, MSGBOX_DEFAULT
	release
	end

Sinnoh_BattleTower_Text_Welcome:
	.string "Welcome to the BATTLE TOWER!\n"
	.string "This counter is for single battles.$"

Sinnoh_BattleTower_Text_Take:
	.string "Will you take the challenge?$"

Sinnoh_BattleTower_Text_Info:
	.string "Bring three MOéMON and face seven\n"
	.string "TRAINERS in a row.\p"
	.string "Your MOéMON are healed between\n"
	.string "battles. Win all seven to earn\l"
	.string "BATTLE POINTS, and your streak goes\l"
	.string "on next time!$"

Sinnoh_BattleTower_Text_Level:
	.string "Which level will you battle at?$"

Sinnoh_BattleTower_Text_NotEnough:
	.string "You'll need three different MOéMON\n"
	.string "that can enter, holding different\l"
	.string "items. Eggs can't battle here.$"

Sinnoh_BattleTower_Text_Pick:
	.string "Please choose your three MOéMON.$"

Sinnoh_BattleTower_Text_Save:
	.string "The game will be saved before you\n"
	.string "go in. Is that all right?$"

Sinnoh_BattleTower_Text_ThisWay:
	.string "Right this way to the battle room!$"

Sinnoh_BattleTower_Text_Won:
	.string "Seven wins in a row! Splendid!$"

Sinnoh_BattleTower_Text_Points:
	.string "Please accept these BATTLE POINTS\n"
	.string "for your victory.$"

Sinnoh_BattleTower_Text_GotPoints:
	.string "{PLAYER} obtained\n"
	.string "{STR_VAR_1} BATTLE POINT(S).$"

Sinnoh_BattleTower_Text_Lost:
	.string "Thank you for taking on the tower.\n"
	.string "Your record will be saved.$"

Sinnoh_BattleTower_Text_Quit:
	.string "You left in the middle of your\n"
	.string "challenge, so it can't go on.$"

Sinnoh_BattleTower_Text_Again:
	.string "We look forward to your next\n"
	.string "challenge!$"

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
	msgbox Sinnoh_BattleTower_Text_Person10, MSGBOX_NPC
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

Sinnoh_BattleTower_Text_Person10:
	.string "PIKACHU: Chu!\p"
	.string "It's looking around excitedly.$"

Sinnoh_BattleTower_Text_Person13:
	.string "I'm covering the BATTLE TOWER's\n"
	.string "records. Some of these streaks are\l"
	.string "legendary.$"

Sinnoh_BattleTower_Text_Person15:
	.string "I can tell how your MOéMON's stats\n"
	.string "grew. Come back once the tower is\l"
	.string "open!$"

@ --- end of SINNOH words ---
