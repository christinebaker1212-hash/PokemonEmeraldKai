Kanto_PewterCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PewterCity_PokemonCenter_1F_EventScript_841A773
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_PewterCity_PokemonCenter_1F_EventScript_841A773::
	setrespawn 0x30
	end

@ Kanto_PewterCity_PokemonCenter_1F object 3 (local id 3, gfx 0x3F2A) at 7,2
Kanto_PewterCity_PokemonCenter_1F_EventScript_841A777::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_PewterCity_PokemonCenter_1F object 2 (local id 2, gfx 0x3F09) at 4,8
Kanto_PewterCity_PokemonCenter_1F_EventScript_841A780::
	msgbox Kanto_PewterCity_PokemonCenter_1F_Text_84419AF, MSGBOX_NPC
	end

@ Kanto_PewterCity_PokemonCenter_1F object 1 (local id 1, gfx 0x3F0F) at 2,2
Kanto_PewterCity_PokemonCenter_1F_EventScript_841A789::
	lock
	faceplayer
	fadeoutbgm 0
	playbgm MUS_RG_JIGGLYPUFF, 0
	message Kanto_PewterCity_PokemonCenter_1F_Text_84419F0
	waitmessage
	delay 0x1C7
	playbgm MUS_RG_POKE_CENTER, 1
	release
	end

@ Kanto_PewterCity_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 1,2
Kanto_PewterCity_PokemonCenter_1F_EventScript_841A7A0::
	msgbox Kanto_PewterCity_PokemonCenter_1F_Text_8441A07, MSGBOX_NPC
	end

@ Kanto_PewterCity_PokemonCenter_1F object 6 (local id 6, gfx 0x3F08) at 10,7
Kanto_PewterCity_PokemonCenter_1F_EventScript_841A7A9::
	lock
	msgbox Kanto_PewterCity_PokemonCenter_1F_Text_8441A49, MSGBOX_DEFAULT
	release
	end

@ Kanto_PewterCity_PokemonCenter_1F object 7 (local id 7, gfx 0x3F08) at 14,6
Kanto_PewterCity_PokemonCenter_1F_EventScript_841A7B4::
	msgbox Kanto_PewterCity_PokemonCenter_1F_Text_8441A86, MSGBOX_NPC
	end

@ Kanto_PewterCity_PokemonCenter_1F object 5 (local id 5, gfx 0x3F57) at 13,2
Kanto_PewterCity_PokemonCenter_1F_EventScript_846C2FC::
	msgbox Kanto_PewterCity_PokemonCenter_1F_Text_846C305, MSGBOX_NPC
	end

Kanto_PewterCity_PokemonCenter_1F_Text_84419AF::
	.string "What!?\p"
	.string "TEAM ROCKET is at MT. MOON?\n"
	.string "Huh?\p"
	.string "I’m on the phone!\n"
	.string "Scram!$"

Kanto_PewterCity_PokemonCenter_1F_Text_84419F0::
	.string "JIGGLYPUFF: Puu pupuu!$"

Kanto_PewterCity_PokemonCenter_1F_Text_8441A07::
	.string "Yawn!\p"
	.string "When JIGGLYPUFF sings, POKéMON get\n"
	.string "drowsy…\p"
	.string "…Me, too…\n"
	.string "Snore…$"

Kanto_PewterCity_PokemonCenter_1F_Text_8441A49::
	.string "I really want a PIKACHU, so I’m\n"
	.string "trading my CLEFAIRY for one.$"

Kanto_PewterCity_PokemonCenter_1F_Text_8441A86::
	.string "I’m trading POKéMON with that kid\n"
	.string "there.\p"
	.string "I had two PIKACHU, so I figured\n"
	.string "I might as well trade one.$"

Kanto_PewterCity_PokemonCenter_1F_Text_846C305::
	.string "TRAINERS from all sorts of places\n"
	.string "pass through here.\p"
	.string "Every trip is a chance to meet\n"
	.string "new POKéMON!$"
