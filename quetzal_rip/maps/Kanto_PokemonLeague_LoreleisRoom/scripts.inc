Kanto_PokemonLeague_LoreleisRoom_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_PokemonLeague_LoreleisRoom_EventScript_841230E
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonLeague_LoreleisRoom_EventScript_8412319
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PokemonLeague_LoreleisRoom_EventScript_8412360
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_PokemonLeague_LoreleisRoom_OnWarp_8412364
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_PokemonLeague_LoreleisRoom_OnFrame_8412373
	.byte 0

Kanto_PokemonLeague_LoreleisRoom_OnWarp_8412364:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_LoreleisRoom_EventScript_841236E
	.2byte 0

Kanto_PokemonLeague_LoreleisRoom_OnFrame_8412373:
	map_script_2 VAR_QZ_0x5120, 0, Kanto_PokemonLeague_LoreleisRoom_EventScript_841237D
	.2byte 0

Kanto_PokemonLeague_LoreleisRoom_EventScript_841230E::
	setvar VAR_0x8004, 0
	call Quetzal_EventScript_8457A98
	end

Kanto_PokemonLeague_LoreleisRoom_EventScript_8412319::
	goto_if_eq VAR_QZ_0x5120, 0, Kanto_PokemonLeague_LoreleisRoom_EventScript_841233F
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 6, Kanto_PokemonLeague_LoreleisRoom_EventScript_841233F
	call_if_eq VAR_0x8005, 12, Quetzal_EventScript_84569D3

Kanto_PokemonLeague_LoreleisRoom_EventScript_841233F::
	call_if_set FLAG_QZ_0x2121, Kanto_PokemonLeague_LoreleisRoom_EventScript_841235A
	call_if_eq VAR_QZ_0x5120, 1, Kanto_PokemonLeague_LoreleisRoom_EventScript_8412354
	end

Kanto_PokemonLeague_LoreleisRoom_EventScript_8412354::
	call Quetzal_EventScript_8457A5F
	return

Kanto_PokemonLeague_LoreleisRoom_EventScript_841235A::
	call Quetzal_EventScript_84579FB
	return

Kanto_PokemonLeague_LoreleisRoom_EventScript_8412360::
	setflag FLAG_QZ_0x2247
	end

Kanto_PokemonLeague_LoreleisRoom_EventScript_841236E::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_PokemonLeague_LoreleisRoom_EventScript_841237D::
	lockall
	call Quetzal_EventScript_84579E2
	setvar VAR_QZ_0x5120, 1
	releaseall
	end

@ Kanto_PokemonLeague_LoreleisRoom object 1 (local id 1, gfx 0x3F19) at 6,5
Kanto_PokemonLeague_LoreleisRoom_EventScript_841238A::
	lock
	faceplayer
	setvar VAR_0x8004, 9
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	goto_if_set FLAG_QZ_0x2121, Kanto_PokemonLeague_LoreleisRoom_EventScript_84123FF
	setvar VAR_0x8004, 9
	setvar VAR_0x8005, 0
	special QzQuestLogNop_737
	setvar VAR_0x8004, 9
	setvar VAR_0x8005, 1
	special QzQuestLogNop_737
	call_if_unset FLAG_QZ_0x2108, Kanto_PokemonLeague_LoreleisRoom_EventScript_84123E2
	call_if_set FLAG_QZ_0x2108, Kanto_PokemonLeague_LoreleisRoom_EventScript_84123EB
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	call Kanto_PokemonLeague_LoreleisRoom_EventScript_84123F4
	clearflag FLAG_TEMP_5
	goto Kanto_PokemonLeague_LoreleisRoom_EventScript_8412409

Kanto_PokemonLeague_LoreleisRoom_EventScript_84123E2::
	msgbox Kanto_PokemonLeague_LoreleisRoom_Text_842865B, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_LoreleisRoom_EventScript_84123EB::
	msgbox Kanto_PokemonLeague_LoreleisRoom_Text_8428740, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_LoreleisRoom_EventScript_84123F4::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8602, 0, Kanto_PokemonLeague_LoreleisRoom_Text_8428848
	return

Kanto_PokemonLeague_LoreleisRoom_EventScript_84123FF::
	msgbox Kanto_PokemonLeague_LoreleisRoom_Text_8428867, MSGBOX_DEFAULT
	release
	end

Kanto_PokemonLeague_LoreleisRoom_EventScript_8412409::
	setflag FLAG_QZ_0x2121
	call Quetzal_EventScript_84579C9
	msgbox Kanto_PokemonLeague_LoreleisRoom_Text_8428867, MSGBOX_DEFAULT
	release
	end

Kanto_PokemonLeague_LoreleisRoom_Text_842865B::
	.string "Welcome to the POKéMON LEAGUE.\p"
	.string "I am LORELEI of the ELITE FOUR.\p"
	.string "No one can best me when it comes\n"
	.string "to icy POKéMON.\p"
	.string "Freezing moves are powerful.\p"
	.string "Your POKéMON will be at my mercy\n"
	.string "when they are frozen solid.\p"
	.string "Hahaha!\n"
	.string "Are you ready?{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_LoreleisRoom_Text_8428740::
	.string "Welcome to the POKéMON LEAGUE.\p"
	.string "I, LORELEI of the ELITE FOUR,\n"
	.string "have returned!\p"
	.string "You know how it goes.\n"
	.string "No one can best me when it comes\l"
	.string "to icy POKéMON.\p"
	.string "Freezing moves are powerful.\p"
	.string "Your POKéMON will be at my mercy\n"
	.string "when they are frozen solid.\p"
	.string "Hahaha!\n"
	.string "Are you ready?{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_LoreleisRoom_Text_8428848::
	.string "…Things shouldn’t be this way!$"

Kanto_PokemonLeague_LoreleisRoom_Text_8428867::
	.string "You’re better than I thought.\n"
	.string "Go on ahead.\p"
	.string "You only got a taste of the\n"
	.string "POKéMON LEAGUE’s power.$"
