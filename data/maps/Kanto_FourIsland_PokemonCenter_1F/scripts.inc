Kanto_FourIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FourIsland_PokemonCenter_1F_EventScript_8421912
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_FourIsland_PokemonCenter_1F_EventScript_8421912::
	setrespawn 0x2A
	end

@ Kanto_FourIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_FourIsland_PokemonCenter_1F_EventScript_8421916::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_FourIsland_PokemonCenter_1F object 2 (local id 2, gfx 0x3F1E) at 10,6
Kanto_FourIsland_PokemonCenter_1F_EventScript_842191F::
	msgbox Kanto_FourIsland_PokemonCenter_1F_Text_8453BBF, MSGBOX_NPC
	end

@ Kanto_FourIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3F59) at 5,5
Kanto_FourIsland_PokemonCenter_1F_EventScript_8421928::
	msgbox Kanto_FourIsland_PokemonCenter_1F_Text_8453C2F, MSGBOX_NPC
	end

@ Kanto_FourIsland_PokemonCenter_1F object 4 (local id 4, gfx 0x3F09) at 2,2
Kanto_FourIsland_PokemonCenter_1F_EventScript_8421931::
	msgbox Kanto_FourIsland_PokemonCenter_1F_Text_8453CCC, MSGBOX_NPC
	end

@ Kanto_FourIsland_PokemonCenter_1F sign / bg event at 3,1
@ Kanto_FourIsland_PokemonCenter_1F sign / bg event at 2,1
Kanto_FourIsland_PokemonCenter_1F_EventScript_845D1A1::
	lockall
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 5
	special QzQuestLogNop
	textcolor 3
	msgbox Kanto_FourIsland_PokemonCenter_1F_Text_8461F16, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FourIsland_PokemonCenter_1F_Text_8453BBF::
	.string "There’s a rare kind of ICE POKéMON\n"
	.string "that lives in the cave.\p"
	.string "We get a lot of visitors hoping to\n"
	.string "see that POKéMON.$"

Kanto_FourIsland_PokemonCenter_1F_Text_8453C2F::
	.string "Oh? A POKéMON EGG?\n"
	.string "…I have no idea.\p"
	.string "…Oh, wait. The DAY-CARE people\n"
	.string "in town were saying something.\p"
	.string "They said that they’d found EGGS\n"
	.string "before on their property.$"

Kanto_FourIsland_PokemonCenter_1F_Text_8453CCC::
	.string "What’s this?\p"
	.string "The new POKéMON JOURNAL hasn’t\n"
	.string "arrived on this island?$"

Kanto_FourIsland_PokemonCenter_1F_Text_8461F16::
	.string "This is a POKéMON JOURNAL from\n"
	.string "years ago…\p"
	.string "POKéMON JOURNAL\n"
	.string "CONTEST Special!\p"
	.string "The Spring POKéMON CONTEST’s\n"
	.string "Grand Champion is DAISY OAK of\l"
	.string "PALLET TOWN!$"
