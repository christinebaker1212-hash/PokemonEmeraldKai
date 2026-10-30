Kanto_SixIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SixIsland_PokemonCenter_1F_EventScript_8421A84
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_SixIsland_PokemonCenter_1F_OnFrame_8421A97
	.byte 0

Kanto_SixIsland_PokemonCenter_1F_OnFrame_8421A97:
	map_script_2 VAR_QZ_0x512F, 0, Kanto_SixIsland_PokemonCenter_1F_EventScript_8421AA1
	.2byte 0

Kanto_SixIsland_PokemonCenter_1F_EventScript_8421A84::
	setrespawn 0x35
	call_if_eq VAR_QZ_0x512F, 0, Kanto_SixIsland_PokemonCenter_1F_EventScript_8421A93
	end

Kanto_SixIsland_PokemonCenter_1F_EventScript_8421A93::
	clearflag FLAG_QZ_0x21CC
	return

Kanto_SixIsland_PokemonCenter_1F_EventScript_8421AA1::
	lockall
	textcolor 0
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	applymovement 2, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 2, Quetzal_Movement_8457AA0
	waitmovement 0
	delay 0x19
	applymovement 2, Kanto_SixIsland_PokemonCenter_1F_Movement_8421B03
	waitmovement 0
	msgbox Kanto_SixIsland_PokemonCenter_1F_Text_84544C8, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_SixIsland_PokemonCenter_1F_Movement_8421B0C
	applymovement 2, Kanto_SixIsland_PokemonCenter_1F_Movement_8421B08
	waitmovement 0
	fadedefaultbgm
	playse SE_EXIT
	delay 0x23
	removeobject 2
	setvar VAR_QZ_0x512F, 1
	setvar VAR_QZ_0x5110, 1
	releaseall
	end

@ Kanto_SixIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_SixIsland_PokemonCenter_1F_EventScript_8421B0F::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_SixIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3F0C) at 13,3
Kanto_SixIsland_PokemonCenter_1F_EventScript_8421B18::
	msgbox Kanto_SixIsland_PokemonCenter_1F_Text_845439A, MSGBOX_NPC
	end

@ Kanto_SixIsland_PokemonCenter_1F object 4 (local id 4, gfx 0x3F2D) at 12,5
Kanto_SixIsland_PokemonCenter_1F_EventScript_8421B21::
	msgbox Kanto_SixIsland_PokemonCenter_1F_Text_845441F, MSGBOX_NPC
	end

Kanto_SixIsland_PokemonCenter_1F_Movement_8421B03:
	walk_right
	walk_right
	walk_down
	walk_down
	step_end

Kanto_SixIsland_PokemonCenter_1F_Movement_8421B08:
	delay_16
	delay_4
	walk_down
	step_end

Kanto_SixIsland_PokemonCenter_1F_Movement_8421B0C:
	walk_left
	walk_in_place_faster_right
	step_end

Kanto_SixIsland_PokemonCenter_1F_Text_845439A::
	.string "For ten years, I’ve searched for\n"
	.string "RUINS.\p"
	.string "No, make that twenty years.\p"
	.string "I get this feeling that there’s\n"
	.string "something hidden on this island.$"

Kanto_SixIsland_PokemonCenter_1F_Text_845441F::
	.string "You know of POKéMON that evolve\n"
	.string "only upon trading, yes?\p"
	.string "But did you know that there are\n"
	.string "still others?\p"
	.string "Some POKéMON evolve only when\n"
	.string "traded while holding specific items.$"

Kanto_SixIsland_PokemonCenter_1F_Text_84544C8::
	.string "BLUE: Hey, {PLAYER}!\n"
	.string "How’s your POKéDEX filling up?\p"
	.string "It looks like it’s impossible to get\n"
	.string "all the POKéMON by hanging around\l"
	.string "just these parts.\p"
	.string "Maybe there are POKéMON we don’t\n"
	.string "know about somewhere far away…\p"
	.string "… … …  … … …  … … …\p"
	.string "Well, if I can’t do it, there’s no\n"
	.string "way for you to get it done.\p"
	.string "I’m not going to get all desperate\n"
	.string "over this.\p"
	.string "I’ll keep collecting POKéMON at my\n"
	.string "own pace while I train them.\p"
	.string "That’s what I’ll do.\n"
	.string "So there’s no point staying here.\l"
	.string "I may as well leave for home.\p"
	.string "That’s that, then!\n"
	.string "Smell ya later!$"
