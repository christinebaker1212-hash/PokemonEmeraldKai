Kanto_CeruleanCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CeruleanCity_PokemonCenter_1F_EventScript_841AA41
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_CeruleanCity_PokemonCenter_1F_EventScript_841AA41::
	setrespawn 0x27
	end

@ Kanto_CeruleanCity_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_CeruleanCity_PokemonCenter_1F_EventScript_841AA45::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_CeruleanCity_PokemonCenter_1F object 2 (local id 2, gfx 0x3F09) at 12,5
Kanto_CeruleanCity_PokemonCenter_1F_EventScript_841AA4E::
	msgbox Kanto_CeruleanCity_PokemonCenter_1F_Text_844236B, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_PokemonCenter_1F object 3 (local id 3, gfx 0x3F3B) at 5,4
Kanto_CeruleanCity_PokemonCenter_1F_EventScript_841AA57::
	msgbox Kanto_CeruleanCity_PokemonCenter_1F_Text_84422ED, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 4,8
Kanto_CeruleanCity_PokemonCenter_1F_EventScript_841AA60::
	lock
	faceplayer
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 1
	special QzQuestLogNop_737
	msgbox Kanto_CeruleanCity_PokemonCenter_1F_Text_8442408, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeruleanCity_PokemonCenter_1F object 5 (local id 5, gfx 0x3F16) at 3,3
Kanto_CeruleanCity_PokemonCenter_1F_EventScript_841AA79::
	msgbox Kanto_CeruleanCity_PokemonCenter_1F_Text_844243E, MSGBOX_NPC
	end

Kanto_CeruleanCity_PokemonCenter_1F_Text_84422ED::
	.string "That BILL!\p"
	.string "I heard that he’ll do whatever it\n"
	.string "takes to get rare POKéMON.\p"
	.string "He’s not above doing all sorts\n"
	.string "of things, I’ve heard.$"

Kanto_CeruleanCity_PokemonCenter_1F_Text_844236B::
	.string "Have you heard about BILL?\p"
	.string "Everyone calls him a POKéMANIAC!\p"
	.string "I think people are just jealous of\n"
	.string "BILL, though.\p"
	.string "Who wouldn’t want to boast about\n"
	.string "their POKéMON?$"

Kanto_CeruleanCity_PokemonCenter_1F_Text_8442408::
	.string "BILL has lots of POKéMON!\n"
	.string "He collects rare ones, too!$"

Kanto_CeruleanCity_PokemonCenter_1F_Text_844243E::
	.string "Why don’t you go upstairs and try\n"
	.string "trading POKéMON with your friends?\p"
	.string "You could get a lot more variety\n"
	.string "by trading.\p"
	.string "The POKéMON you get in trades grow\n"
	.string "quickly, too.$"
