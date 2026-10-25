Kanto_CinnabarIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841E975
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841E975::
	setrespawn 0x28
	end

@ Kanto_CinnabarIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841E979::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_CinnabarIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3F09) at 9,7
Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841E982::
	msgbox Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A519, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland_PokemonCenter_1F object 2 (local id 2, gfx 0x3EFC) at 2,4
Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841E98B::
	msgbox Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A453, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 14,6
Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841E994::
	msgbox Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A60C, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland_PokemonCenter_1F object 5 (local id 5, gfx 0x3EEA) at 11,5
Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841E99D::
	lock
	faceplayer
	msgbox Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A680, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA38
	msgbox Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A6E0, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_RG_FOLLOW_ME, 1
	savebgm MUS_RG_FOLLOW_ME
	setflag FLAG_QZ_0x2130
	setflag FLAG_QZ_0x2180
	delay 0x14
	call_if_eq VAR_FACING, 1, Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA02
	call_if_eq VAR_FACING, 4, Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA14
	call_if_eq VAR_FACING, 3, Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA26
	removeobject 5
	setvar VAR_QZ_0x510B, 1
	clearflag FLAG_QZ_0x217F
	warp MAP_KANTO_CINNABAR_ISLAND, 255, 14, 11
	waitstate
	release
	end

Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA02::
	applymovement 5, Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA42
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA4C
	waitmovement 0
	return

Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA14::
	applymovement 5, Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA56
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA62
	waitmovement 0
	return

Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA26::
	applymovement 5, Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA42
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA71
	waitmovement 0
	return

Kanto_CinnabarIsland_PokemonCenter_1F_EventScript_841EA38::
	msgbox Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A6C1, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA42:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	delay_8
	set_invisible
	step_end

Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA4C:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	delay_16
	walk_down
	step_end

Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA56:
	walk_up
	walk_left
	walk_left
	walk_down
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	delay_8
	set_invisible
	step_end

Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA62:
	delay_16
	delay_16
	delay_16
	face_left
	delay_16
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	delay_8
	walk_down
	delay_8
	set_invisible
	step_end

Kanto_CinnabarIsland_PokemonCenter_1F_Movement_841EA71:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	delay_16
	walk_down
	step_end

Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A453::
	.string "I came to visit the CINNABAR GYM,\n"
	.string "but the door is locked tight.\p"
	.string "There should be a key for it\n"
	.string "somewhere.\p"
	.string "Could it be in that burned-out\n"
	.string "mansion?\p"
	.string "The GYM LEADER’s friend used to\n"
	.string "live there, they say.$"

Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A519::
	.string "Do you have lots of friends?\p"
	.string "Linking up with the usual friends\n"
	.string "is fun, of course.\p"
	.string "But how about visiting the\n"
	.string "UNION ROOM every so often?\p"
	.string "Who knows, you may end up making\n"
	.string "new friends.\p"
	.string "I think it would be worth your time\n"
	.string "to check the UNION ROOM.$"

Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A60C::
	.string "POKéMON can still learn techniques\n"
	.string "after canceling evolution.\p"
	.string "Evolution can wait until new moves\n"
	.string "have been learned.$"

Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A680::
	.string "BILL: Hey, you kept me waiting!\n"
	.string "Ready to set sail to ONE ISLAND?$"

Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A6C1::
	.string "Oh, you’re still not done yet?$"

Kanto_CinnabarIsland_PokemonCenter_1F_Text_844A6E0::
	.string "Well, that’s it.\n"
	.string "Let’s go!$"
