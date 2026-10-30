Kanto_TrainerTower_Lobby_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_TrainerTower_Lobby_EventScript_84145FC
	map_script MAP_SCRIPT_ON_RETURN_TO_FIELD, Kanto_TrainerTower_Lobby_EventScript_8414634
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_TrainerTower_Lobby_EventScript_841465A
	map_script MAP_SCRIPT_ON_LOAD, Kanto_TrainerTower_Lobby_EventScript_8414644
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_TrainerTower_Lobby_OnFrame_8414663
	.byte 0

Kanto_TrainerTower_Lobby_OnFrame_8414663:
	map_script_2 VAR_TEMP_0, 0, Kanto_TrainerTower_Lobby_EventScript_841469A
	map_script_2 VAR_TEMP_D, 17, Kanto_TrainerTower_Lobby_EventScript_8414675
	.2byte 0

Kanto_TrainerTower_Lobby_EventScript_84145FC::
	setvar VAR_TEMP_2, 0
	setvar VAR_0x8004, 0xA
	goto Quetzal_EventScript_84736F6

Kanto_TrainerTower_Lobby_EventScript_8414634::
	addobject 1
	addobject 2
	addobject 3
	addobject 4
	addobject 5
	end

Kanto_TrainerTower_Lobby_EventScript_8414644::
	call_if_eq VAR_TEMP_D, 17, Kanto_TrainerTower_Lobby_EventScript_8414650
	end

Kanto_TrainerTower_Lobby_EventScript_8414650::
	setmetatile 0x11, 0xA, 0x287, 0
	return

Kanto_TrainerTower_Lobby_EventScript_841465A::
	setflag FLAG_QZ_0x225E
	getplayerxy VAR_TEMP_D, VAR_RESULT
	end

Kanto_TrainerTower_Lobby_EventScript_8414675::
	lockall
	applymovement LOCALID_PLAYER, Kanto_TrainerTower_Lobby_Movement_8414697
	waitmovement 0
	setmetatile 0x11, 0xA, 0x2B4, 1
	special DrawWholeMapView
	playse SE_CLICK
	waitse
	setvar VAR_TEMP_D, 0
	releaseall
	end

Kanto_TrainerTower_Lobby_EventScript_841469A::
	setvar VAR_TEMP_0, 1
	setvar VAR_0x8004, 0xC
	goto Quetzal_EventScript_84736F6

@ Kanto_TrainerTower_Lobby object 1 (local id 1, gfx 0x3F2A) at 4,9
Kanto_TrainerTower_Lobby_EventScript_841470C::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_TrainerTower_Lobby object 3 (local id 3, gfx 0x3F54) at 11,7
Kanto_TrainerTower_Lobby_EventScript_8414715::
	lock
	faceplayer
	setvar VAR_0x8004, 0x14
	goto Quetzal_EventScript_84736F6

@ Kanto_TrainerTower_Lobby object 2 (local id 2, gfx 0x3F54) at 14,9
Kanto_TrainerTower_Lobby_EventScript_8414743::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart Kanto_TrainerTower_Lobby_Mart_841476C
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

@ Kanto_TrainerTower_Lobby trigger at 9,7 (VAR_QZ_0x5132 == 0)
Kanto_TrainerTower_Lobby_EventScript_8414782::
	lockall
	applymovement LOCALID_PLAYER, Kanto_TrainerTower_Lobby_Movement_841488E
	textcolor 1
	msgbox Kanto_TrainerTower_Lobby_Text_842CE9D, MSGBOX_DEFAULT
	setvar VAR_0x8004, 0x11
	goto Quetzal_EventScript_84736F6

@ Kanto_TrainerTower_Lobby sign / bg event at 8,10
Kanto_TrainerTower_Lobby_EventScript_8414892::
	lockall
	fadescreen 1
	setvar VAR_0x8004, 1
	special ShowLinkBattleRecords
	waitstate
	releaseall
	end

@ Kanto_TrainerTower_Lobby object 4 (local id 4, gfx 0x3EFC) at 12,13
Kanto_TrainerTower_Lobby_EventScript_84148A0::
	msgbox Kanto_TrainerTower_Lobby_Text_842D541, MSGBOX_NPC
	end

@ Kanto_TrainerTower_Lobby object 5 (local id 5, gfx 0x3EE6) at 2,12
Kanto_TrainerTower_Lobby_EventScript_84148A9::
	msgbox Kanto_TrainerTower_Lobby_Text_842D5A8, MSGBOX_NPC
	end

Kanto_TrainerTower_Lobby_Movement_8414697:
	walk_down
	walk_down
	step_end

Kanto_TrainerTower_Lobby_Mart_841476C:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_MAX_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_NONE

Kanto_TrainerTower_Lobby_Movement_841488E:
	face_right
	step_end

Kanto_TrainerTower_Lobby_Text_842CE9D::
	.string "Hello!\p"
	.string "Welcome to TRAINER TOWER where\n"
	.string "TRAINERS gather from all over!$"

Kanto_TrainerTower_Lobby_Text_842D541::
	.string "I’m here to see how good I am.\p"
	.string "I wonder what kinds of TRAINERS\n"
	.string "are waiting for me?\p"
	.string "It’s nerve-racking!$"

Kanto_TrainerTower_Lobby_Text_842D5A8::
	.string "Gasp, gasp…\n"
	.string "Gasp…\p"
	.string "Never mind battling! These stairs…\n"
	.string "They’re tougher than any battle…$"
