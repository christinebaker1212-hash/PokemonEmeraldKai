Kanto_PokemonLeague_ChampionsRoom_MapScripts::
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_PokemonLeague_ChampionsRoom_OnWarp_8412874
	map_script MAP_SCRIPT_ON_RESUME, Kanto_PokemonLeague_ChampionsRoom_EventScript_8412840
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_PokemonLeague_ChampionsRoom_OnFrame_8412883
	.byte 0

Kanto_PokemonLeague_ChampionsRoom_OnWarp_8412874:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_ChampionsRoom_EventScript_841287E
	.2byte 0

Kanto_PokemonLeague_ChampionsRoom_OnFrame_8412883:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_ChampionsRoom_EventScript_841288D
	.2byte 0

Kanto_PokemonLeague_ChampionsRoom_EventScript_8412840::
	setvar VAR_0x8004, 4
	call Quetzal_EventScript_8457A98
	call Kanto_PokemonLeague_ChampionsRoom_EventScript_8412850
	end

Kanto_PokemonLeague_ChampionsRoom_EventScript_8412850::
	checktrainerflag 0x21B6
	goto_if_eq Kanto_PokemonLeague_ChampionsRoom_EventScript_841286E
	return

Kanto_PokemonLeague_ChampionsRoom_EventScript_841286E::
	setvar VAR_TEMP_1, 1
	return

Kanto_PokemonLeague_ChampionsRoom_EventScript_841287E::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_PokemonLeague_ChampionsRoom_EventScript_841288D::
	lockall
	textcolor 0
	setflag FLAG_TEMP_2
	applymovement LOCALID_PLAYER, Kanto_PokemonLeague_ChampionsRoom_Movement_84129B4
	waitmovement 0
	delay 0x14
	call_if_unset FLAG_QZ_0x2108, Kanto_PokemonLeague_ChampionsRoom_EventScript_8412981
	call_if_set FLAG_QZ_0x2108, Kanto_PokemonLeague_ChampionsRoom_EventScript_841298A
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Kanto_PokemonLeague_ChampionsRoom_EventScript_8412974
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	savebgm MUS_RG_VICTORY_GYM_LEADER
	call Kanto_PokemonLeague_ChampionsRoom_EventScript_841297B
	setflag FLAG_TEMP_4
	clearflag FLAG_TEMP_5
	setflag FLAG_QZ_0x2109
	setflag FLAG_TEMP_4
	msgbox Kanto_PokemonLeague_ChampionsRoom_Text_84295EE, MSGBOX_DEFAULT
	playbgm MUS_RG_SLOW_PALLET, 0
	addobject 2
	msgbox Kanto_PokemonLeague_ChampionsRoom_Text_8429685, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_PokemonLeague_ChampionsRoom_Movement_84129CC
	applymovement 1, Kanto_PokemonLeague_ChampionsRoom_Movement_84129EE
	applymovement 2, Kanto_PokemonLeague_ChampionsRoom_Movement_84129D7
	waitmovement 0
	delay 0x19
	specialvar VAR_RESULT, QzGetRivalStarterSpecies
	bufferspeciesname 0, VAR_RESULT
	msgbox Kanto_PokemonLeague_ChampionsRoom_Text_842968E, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AAE
	applymovement 1, Quetzal_Movement_8457AAA
	waitmovement 0
	msgbox Kanto_PokemonLeague_ChampionsRoom_Text_842974E, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	delay 0x14
	msgbox Kanto_PokemonLeague_ChampionsRoom_Text_8429866, MSGBOX_DEFAULT
	closemessage
	delay 0xD
	applymovement 2, Kanto_PokemonLeague_ChampionsRoom_Movement_84129E4
	applymovement LOCALID_PLAYER, Kanto_PokemonLeague_ChampionsRoom_Movement_84129BF
	waitmovement 0
	setvar VAR_TEMP_1, 1
	warp MAP_KANTO_POKEMON_LEAGUE_HALL_OF_FAME, 255, 5, 12
	waitstate
	releaseall
	end

Kanto_PokemonLeague_ChampionsRoom_EventScript_8412974::
	call Kanto_PokemonLeague_ChampionsRoom_EventScript_841286E
	releaseall
	end

Kanto_PokemonLeague_ChampionsRoom_EventScript_841297B::
	call Kanto_PokemonLeague_ChampionsRoom_EventScript_8412993
	return

Kanto_PokemonLeague_ChampionsRoom_EventScript_8412981::
	msgbox Kanto_PokemonLeague_ChampionsRoom_Text_8429225, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_ChampionsRoom_EventScript_841298A::
	msgbox Kanto_PokemonLeague_ChampionsRoom_Text_842938F, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_ChampionsRoom_EventScript_8412993::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8630, 0, Kanto_PokemonLeague_ChampionsRoom_Text_84294FC
	return

Kanto_PokemonLeague_ChampionsRoom_Movement_84129B4:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_PokemonLeague_ChampionsRoom_Movement_84129BF:
	delay_16
	walk_left
	walk_up
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	set_invisible
	step_end

Kanto_PokemonLeague_ChampionsRoom_Movement_84129CC:
	delay_8
	walk_in_place_faster_down
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

Kanto_PokemonLeague_ChampionsRoom_Movement_84129D7:
	delay_16
	delay_16
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_up
	walk_up
	walk_in_place_faster_down
	delay_16
	step_end

Kanto_PokemonLeague_ChampionsRoom_Movement_84129E4:
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	set_invisible
	step_end

Kanto_PokemonLeague_ChampionsRoom_Movement_84129EE:
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

Kanto_PokemonLeague_ChampionsRoom_Text_8429225::
	.string "BLUE: Hey, {PLAYER}!\p"
	.string "I was looking forward to seeing\n"
	.string "you, {PLAYER}.\p"
	.string "My rival should be strong to keep\n"
	.string "me sharp.\p"
	.string "While working on my POKéDEX, I\n"
	.string "looked all over for POKéMON.\p"
	.string "Not only that, I assembled teams\n"
	.string "that would beat any POKéMON type.\p"
	.string "And now…\p"
	.string "I’m the POKéMON LEAGUE CHAMPION!\p"
	.string "{PLAYER}!\n"
	.string "Do you know what that means?\p"
	.string "I’ll tell you.\p"
	.string "I am the most powerful TRAINER in\n"
	.string "the world!$"

Kanto_PokemonLeague_ChampionsRoom_Text_842938F::
	.string "BLUE: Hey, {PLAYER}!\p"
	.string "You came back, {PLAYER}!\n"
	.string "Hahah, that is so great!\p"
	.string "My rival should be strong to keep\n"
	.string "me sharp.\p"
	.string "While working on my POKéDEX, I\n"
	.string "looked all over for POKéMON.\p"
	.string "Not only that, I assembled teams\n"
	.string "that would beat any POKéMON type.\p"
	.string "And now…\p"
	.string "I’m the POKéMON LEAGUE CHAMPION!\p"
	.string "{PLAYER}!\n"
	.string "Do you know what that means?\p"
	.string "I’ll tell you.\p"
	.string "I am the most powerful TRAINER in\n"
	.string "the world!$"

Kanto_PokemonLeague_ChampionsRoom_Text_84294FC::
	.string "NO!\n"
	.string "That can’t be!\l"
	.string "You beat me at my best!\p"
	.string "After all that work to become\n"
	.string "the LEAGUE CHAMP?\p"
	.string "My reign is over already?\n"
	.string "It’s not fair!$"

Kanto_PokemonLeague_ChampionsRoom_Text_84295EE::
	.string "Why?\n"
	.string "Why did I lose?\p"
	.string "I never made any mistakes raising\n"
	.string "my POKéMON…\p"
	.string "Darn it! You’re the new POKéMON\n"
	.string "LEAGUE CHAMPION!\p"
	.string "Although I don’t like to admit it…$"

Kanto_PokemonLeague_ChampionsRoom_Text_8429685::
	.string "OAK: {PLAYER}!$"

Kanto_PokemonLeague_ChampionsRoom_Text_842968E::
	.string "OAK: So, you’ve won!\n"
	.string "Sincerely, congratulations!\p"
	.string "You’re the new POKéMON LEAGUE\n"
	.string "CHAMPION!\p"
	.string "You’ve grown up so much since you\n"
	.string "first left with {STR_VAR_1} to work\l"
	.string "on the POKéDEX.\p"
	.string "{PLAYER}, you have come of age!$"

Kanto_PokemonLeague_ChampionsRoom_Text_842974E::
	.string "OAK: BLUE…\n"
	.string "I’m disappointed in you.\p"
	.string "I came when I heard you’d beaten\n"
	.string "the ELITE FOUR.\p"
	.string "But, when I got here, you had\n"
	.string "already lost!\p"
	.string "BLUE, do you understand why\n"
	.string "you lost?\p"
	.string "You have forgotten to treat your\n"
	.string "POKéMON with love and trust.\p"
	.string "Without them, you will never\n"
	.string "become a CHAMP again!$"

Kanto_PokemonLeague_ChampionsRoom_Text_8429866::
	.string "OAK: {PLAYER}.\p"
	.string "You understand that your victory\n"
	.string "was not just your own doing.\p"
	.string "The bond you share with your\n"
	.string "POKéMON is marvelous.\p"
	.string "{PLAYER}!\n"
	.string "Come with me!$"
