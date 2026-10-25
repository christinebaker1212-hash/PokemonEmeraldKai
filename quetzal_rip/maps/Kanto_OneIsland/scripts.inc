Kanto_OneIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_OneIsland_EventScript_8417184
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_OneIsland_OnFrame_841718B
	.byte 0

Kanto_OneIsland_OnFrame_841718B:
	map_script_2 VAR_QZ_0x5116, 2, Kanto_OneIsland_EventScript_8417195
	.2byte 0

Kanto_OneIsland_EventScript_8417184::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2244
	end

Kanto_OneIsland_EventScript_8417195::
	lockall
	textcolor 0
	applymovement LOCALID_PLAYER, Kanto_OneIsland_Movement_84171FD
	waitmovement 0
	msgbox Kanto_OneIsland_Text_84317FA, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Kanto_OneIsland_Movement_84171FF
	applymovement LOCALID_PLAYER, Kanto_OneIsland_Movement_84171EC
	waitmovement 0
	opendoor 0xE, 5
	waitdooranim
	applymovement 1, Kanto_OneIsland_Movement_841720C
	applymovement LOCALID_PLAYER, Kanto_OneIsland_Movement_84171F9
	waitmovement 0
	closedoor 0xE, 5
	waitdooranim
	removeobject 1
	setvar VAR_QZ_0x5116, 3
	warp MAP_KANTO_ONE_ISLAND_POKEMON_CENTER_1F, 255, 9, 9
	waitstate
	releaseall
	end

@ Kanto_OneIsland object 2 (local id 2, gfx 0x3F2C) at 16,12
Kanto_OneIsland_EventScript_841720F::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x220A, Kanto_OneIsland_EventScript_8417237
	goto_if_set FLAG_QZ_0x21F1, Kanto_OneIsland_EventScript_841722D
	msgbox Kanto_OneIsland_Text_8431938, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_EventScript_841722D::
	msgbox Kanto_OneIsland_Text_8431A42, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_EventScript_8417237::
	msgbox Kanto_OneIsland_Text_8431AEB, MSGBOX_DEFAULT
	release
	end

@ Kanto_OneIsland object 3 (local id 3, gfx 0x3EE6) at 13,9
Kanto_OneIsland_EventScript_8417241::
	msgbox Kanto_OneIsland_Text_8431C34, MSGBOX_NPC
	end

@ Kanto_OneIsland sign / bg event at 14,13
Kanto_OneIsland_EventScript_841724A::
	msgbox Kanto_OneIsland_Text_8431BCB, MSGBOX_SIGN
	end

@ Kanto_OneIsland sign / bg event at 15,6
Kanto_OneIsland_EventScript_8417253::
	msgbox Kanto_OneIsland_Text_8431BF4, MSGBOX_SIGN
	end

Kanto_OneIsland_Movement_84171EC:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_OneIsland_Movement_84171F9:
	walk_up
	walk_up
	set_invisible
	step_end

Kanto_OneIsland_Movement_84171FD:
	walk_up
	step_end

Kanto_OneIsland_Movement_84171FF:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_OneIsland_Movement_841720C:
	walk_up
	set_invisible
	step_end

Kanto_OneIsland_Text_84317FA::
	.string "BILL: Here we are!\n"
	.string "This is ONE ISLAND.\p"
	.string "There are several islands around\n"
	.string "here, and this is one of them.\p"
	.string "My friend CELIO sent the boat to\n"
	.string "fetch me here.\p"
	.string "He’s in charge of the island’s PC\n"
	.string "network by his lonesome.\p"
	.string "…Why am I telling you this now?\n"
	.string "Let’s just go see CELIO!$"

Kanto_OneIsland_Text_8431938::
	.string "Here we are on an island smack in\n"
	.string "the middle of nowhere.\p"
	.string "We’re lucky to have an upstanding\n"
	.string "young man like CELIO here.\p"
	.string "My granddaughter was in a tizzy\n"
	.string "over CELIO’s friend.\p"
	.string "Something about a famous\n"
	.string "POKéMANIAC?\p"
	.string "I’m not sure what that means, but\n"
	.string "CELIO is quite the man!$"

Kanto_OneIsland_Text_8431A42::
	.string "My granddaughter was in a tizzy\n"
	.string "over a new breakthrough.\p"
	.string "She said we now have PC linkage\n"
	.string "with people in KANTO.\p"
	.string "I’m not sure what that means, but\n"
	.string "CELIO is quite the man!$"

Kanto_OneIsland_Text_8431AEB::
	.string "My granddaughter was in a tizzy\n"
	.string "over a new breakthrough.\p"
	.string "She said we finally have PC linkage\n"
	.string "with people in HOENN.\p"
	.string "I’m not sure what that means, but\n"
	.string "CELIO is quite the man.\p"
	.string "He would make a fine husband for\n"
	.string "my granddaughter!$"

Kanto_OneIsland_Text_8431BCB::
	.string "ONE ISLAND\n"
	.string "Friends Gather at Knot Island$"

Kanto_OneIsland_Text_8431BF4::
	.string "Expanding the POKéMON World!\n"
	.string "POKéMON NET CENTER$"

Kanto_OneIsland_Text_8431C34::
	.string "Hi, sight-seeing, are you?\n"
	.string "Isn’t the warm climate here great?$"
