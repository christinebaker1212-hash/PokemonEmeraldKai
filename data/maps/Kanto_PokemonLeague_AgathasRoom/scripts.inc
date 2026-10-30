Kanto_PokemonLeague_AgathasRoom_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_PokemonLeague_AgathasRoom_EventScript_84125A3
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonLeague_AgathasRoom_EventScript_84125AE
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_PokemonLeague_AgathasRoom_OnWarp_84125CF
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_PokemonLeague_AgathasRoom_OnFrame_84125DE
	.byte 0

Kanto_PokemonLeague_AgathasRoom_OnWarp_84125CF:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_AgathasRoom_EventScript_84125D9
	.2byte 0

Kanto_PokemonLeague_AgathasRoom_OnFrame_84125DE:
	map_script_2 VAR_QZ_0x5120, 2, Kanto_PokemonLeague_AgathasRoom_EventScript_84125E8
	.2byte 0

Kanto_PokemonLeague_AgathasRoom_EventScript_84125A3::
	setvar VAR_0x8004, 2
	call Quetzal_EventScript_8457A98
	end

Kanto_PokemonLeague_AgathasRoom_EventScript_84125AE::
	call_if_set FLAG_QZ_0x2119, Kanto_PokemonLeague_AgathasRoom_EventScript_84125C9
	call_if_eq VAR_QZ_0x5120, 3, Kanto_PokemonLeague_AgathasRoom_EventScript_84125C3
	end

Kanto_PokemonLeague_AgathasRoom_EventScript_84125C3::
	call Quetzal_EventScript_8457A5F
	return

Kanto_PokemonLeague_AgathasRoom_EventScript_84125C9::
	call Quetzal_EventScript_84579FB
	return

Kanto_PokemonLeague_AgathasRoom_EventScript_84125D9::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_PokemonLeague_AgathasRoom_EventScript_84125E8::
	lockall
	call Quetzal_EventScript_84579E2
	setvar VAR_QZ_0x5120, 3
	releaseall
	end

@ Kanto_PokemonLeague_AgathasRoom object 1 (local id 1, gfx 0x3EE4) at 6,5
Kanto_PokemonLeague_AgathasRoom_EventScript_84125F5::
	lock
	faceplayer
	setvar VAR_0x8004, 0xB
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	goto_if_set FLAG_QZ_0x2119, Kanto_PokemonLeague_AgathasRoom_EventScript_8412677
	setvar VAR_0x8004, 0xB
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	setvar VAR_0x8004, 0xB
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	setvar VAR_0x8004, 0
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	call_if_unset FLAG_QZ_0x2108, Kanto_PokemonLeague_AgathasRoom_EventScript_841265A
	call_if_set FLAG_QZ_0x2108, Kanto_PokemonLeague_AgathasRoom_EventScript_8412663
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	call Kanto_PokemonLeague_AgathasRoom_EventScript_841266C
	clearflag FLAG_TEMP_5
	goto Kanto_PokemonLeague_AgathasRoom_EventScript_8412681

Kanto_PokemonLeague_AgathasRoom_EventScript_841265A::
	msgbox Kanto_PokemonLeague_AgathasRoom_Text_8428B04, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_AgathasRoom_EventScript_8412663::
	msgbox Kanto_PokemonLeague_AgathasRoom_Text_8428C44, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_AgathasRoom_EventScript_841266C::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8604, 0, Kanto_PokemonLeague_AgathasRoom_Text_8428D8E
	return

Kanto_PokemonLeague_AgathasRoom_EventScript_8412677::
	msgbox Kanto_PokemonLeague_AgathasRoom_Text_8428DB7, MSGBOX_DEFAULT
	release
	end

Kanto_PokemonLeague_AgathasRoom_EventScript_8412681::
	setflag FLAG_QZ_0x2119
	call Quetzal_EventScript_84579C9
	msgbox Kanto_PokemonLeague_AgathasRoom_Text_8428DB7, MSGBOX_DEFAULT
	release
	end

Kanto_PokemonLeague_AgathasRoom_Text_8428B04::
	.string "I am AGATHA of the ELITE FOUR.\p"
	.string "I hear OAK’s taken a lot of\n"
	.string "interest in you, child.\p"
	.string "That old duff was once tough and\n"
	.string "handsome.\p"
	.string "But that was decades ago.\n"
	.string "He’s a shadow of his former self.\p"
	.string "Now he just wants to fiddle with\n"
	.string "his POKéDEX.\p"
	.string "He’s wrong.\n"
	.string "POKéMON are for battling!\p"
	.string "{PLAYER}! I’ll show you how a real\n"
	.string "TRAINER battles!{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_AgathasRoom_Text_8428C44::
	.string "I am AGATHA of the ELITE FOUR.\p"
	.string "You’re the child that OAK’s taken\n"
	.string "under his wing, aren’t you?\p"
	.string "That old duff was once tough and\n"
	.string "handsome.\p"
	.string "But that was decades ago.\n"
	.string "He’s a shadow of his former self.\p"
	.string "Now he just wants to fiddle with\n"
	.string "his POKéDEX.\p"
	.string "He’s wrong.\n"
	.string "POKéMON are for battling!\p"
	.string "{PLAYER}! I’ll show you how a real\n"
	.string "TRAINER battles!{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_AgathasRoom_Text_8428D8E::
	.string "Oh, my!\n"
	.string "You’re something special, child!$"

Kanto_PokemonLeague_AgathasRoom_Text_8428DB7::
	.string "You win!\p"
	.string "I see what the old duff sees in\n"
	.string "you now.\p"
	.string "I have nothing else to say.\n"
	.string "Run along now, child!$"
