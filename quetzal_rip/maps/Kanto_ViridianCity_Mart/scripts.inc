Kanto_ViridianCity_Mart_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_ViridianCity_Mart_EventScript_841A1DF
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_ViridianCity_Mart_OnFrame_841A1FC
	.byte 0

Kanto_ViridianCity_Mart_OnFrame_841A1FC:
	map_script_2 VAR_QZ_0x513B, 0, Kanto_ViridianCity_Mart_EventScript_841A206
	.2byte 0

Kanto_ViridianCity_Mart_EventScript_841A1DF::
	goto_if_unset FLAG_QZ_0xA79, Kanto_ViridianCity_Mart_EventScript_841A1E9
	end

Kanto_ViridianCity_Mart_EventScript_841A1E9::
	setmetatile 1, 3, 0x2BF, 1
	setmetatile 1, 4, 0x2C0, 1
	end

Kanto_ViridianCity_Mart_EventScript_841A206::
	lockall
	textcolor 0
	applymovement 1, Quetzal_Movement_8457AB0
	waitmovement 0
	msgbox Kanto_ViridianCity_Mart_Text_8440576, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Kanto_ViridianCity_Mart_Movement_841A266
	applymovement LOCALID_PLAYER, Kanto_ViridianCity_Mart_Movement_841A260
	waitmovement 0
	msgbox Kanto_ViridianCity_Mart_Text_8440596, MSGBOX_DEFAULT
	setvar VAR_QZ_0x513B, 1
	additem ITEM_PARCEL, 1
	loadword 0, Kanto_ViridianCity_Mart_Text_84405E5
	setorcopyvar VAR_0x8000, 0x2EA
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	setvar VAR_QZ_0x511A, 5
	releaseall
	end

@ Kanto_ViridianCity_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_ViridianCity_Mart_EventScript_841A26C::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x513B, 1, Kanto_ViridianCity_Mart_EventScript_841A2AC
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	message Quetzal_Text_84563A6
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_ViridianCity_Mart_EventScript_841A2AC::
	msgbox Kanto_ViridianCity_Mart_Text_844061B, MSGBOX_DEFAULT
	release
	end

@ Kanto_ViridianCity_Mart object 3 (local id 3, gfx 0x3F57) at 9,5
Kanto_ViridianCity_Mart_EventScript_841A2B6::
	msgbox Kanto_ViridianCity_Mart_Text_8440651, MSGBOX_NPC
	end

@ Kanto_ViridianCity_Mart object 2 (local id 2, gfx 0x3F5C) at 6,2
Kanto_ViridianCity_Mart_EventScript_841A2BF::
	msgbox Kanto_ViridianCity_Mart_Text_8440688, MSGBOX_NPC
	end

Kanto_ViridianCity_Mart_Movement_841A260:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_in_place_faster_left
	step_end

Kanto_ViridianCity_Mart_Movement_841A266:
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_right
	step_end

Kanto_ViridianCity_Mart_Text_8440576::
	.string "Hey!\n"
	.string "You came from PALLET TOWN?$"

Kanto_ViridianCity_Mart_Text_8440596::
	.string "You know PROF. OAK, right?\p"
	.string "His order came in.\n"
	.string "Can I get you to take it to him?$"

Kanto_ViridianCity_Mart_Text_84405E5::
	.string "{PLAYER} received OAK’S PARCEL\n"
	.string "from the POKéMON MART clerk.$"

Kanto_ViridianCity_Mart_Text_844061B::
	.string "Okay, thanks! Please say hi to\n"
	.string "PROF. OAK for me, too.$"

Kanto_ViridianCity_Mart_Text_8440651::
	.string "This shop does good business in\n"
	.string "ANTIDOTES, I’ve heard.$"

Kanto_ViridianCity_Mart_Text_8440688::
	.string "I’ve got to buy some POTIONS.\p"
	.string "You never know when your POKéMON\n"
	.string "will need quick healing.$"
