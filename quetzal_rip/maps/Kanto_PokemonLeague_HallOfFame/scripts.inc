Kanto_PokemonLeague_HallOfFame_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_PokemonLeague_HallOfFame_OnFrame_8412A13
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_PokemonLeague_HallOfFame_OnWarp_8412A04
	.byte 0

Kanto_PokemonLeague_HallOfFame_OnFrame_8412A13:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_HallOfFame_EventScript_8412A1D
	.2byte 0

Kanto_PokemonLeague_HallOfFame_OnWarp_8412A04:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_HallOfFame_EventScript_8412A0E
	.2byte 0

Kanto_PokemonLeague_HallOfFame_EventScript_8412A0E::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_PokemonLeague_HallOfFame_EventScript_8412A1D::
	lockall
	textcolor 0
	applymovement LOCALID_PLAYER, Kanto_PokemonLeague_HallOfFame_Movement_8412A85
	waitmovement 0
	applymovement 1, Quetzal_Movement_8457AAA
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	delay 0x12
	msgbox Kanto_PokemonLeague_HallOfFame_Text_84298F2, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AAC
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	delay 0x14
	dofieldeffect 0x3E
	waitfieldeffect 0x3E
	delay 0x28
	call Quetzal_EventScript_835F6E4
	setvar VAR_TEMP_1, 1
	call Kanto_PokemonLeague_HallOfFame_EventScript_845699F
	setrespawn 0x2F
	fadescreenspeed 1, 0x18
	setflag FLAG_QZ_0x2109
	setflag FLAG_QZ_0x2108
	special GameClear
	waitstate
	releaseall
	end

Kanto_PokemonLeague_HallOfFame_EventScript_845699F::
	clearflag FLAG_QZ_0x21AA
	call Quetzal_EventScript_84569D3
	specialvar VAR_RESULT, QzIsNationalPokedexEnabled_719
	call_if_eq VAR_RESULT, 0, Kanto_PokemonLeague_HallOfFame_EventScript_84569BD
	call Kanto_PokemonLeague_HallOfFame_EventScript_84569C3
	return

Kanto_PokemonLeague_HallOfFame_EventScript_84569BD::
	setvar VAR_QZ_0x5118, 2
	return

Kanto_PokemonLeague_HallOfFame_EventScript_84569C3::
	clearflag FLAG_QZ_0x21DE
	clearflag FLAG_QZ_0x21DB
	clearflag FLAG_QZ_0x2126
	return

Kanto_PokemonLeague_HallOfFame_Movement_8412A85:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_PokemonLeague_HallOfFame_Text_84298F2::
	.string "OAK: Er-hem!\n"
	.string "Congratulations, {PLAYER}!\p"
	.string "This floor is the POKéMON HALL OF\n"
	.string "FAME.\p"
	.string "POKéMON LEAGUE CHAMPIONS are\n"
	.string "honored for their exploits here.\p"
	.string "Their POKéMON are also recorded in\n"
	.string "the HALL OF FAME.\p"
	.string "{PLAYER}!\p"
	.string "You have worked hard to become\n"
	.string "the new LEAGUE CHAMPION.\p"
	.string "Congratulations, {PLAYER}, you and\n"
	.string "your POKéMON are HALL OF FAMERS!$"
