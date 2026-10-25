Kanto_SaffronCity_House_MapScripts::
	.byte 0

@ Kanto_SaffronCity_House object 2 (local id 2, gfx 0x3F16) at 4,4
Kanto_SaffronCity_House_EventScript_841F020::
	msgbox Kanto_SaffronCity_House_Text_844BDCF, MSGBOX_NPC
	end

@ Kanto_SaffronCity_House object 3 (local id 3, gfx 0x3F5C) at 6,2
Kanto_SaffronCity_House_EventScript_841F029::
	msgbox Kanto_SaffronCity_House_Text_844BE30, MSGBOX_NPC
	end

@ Kanto_SaffronCity_House object 1 (local id 1, gfx 0x3F33) at 0,5
Kanto_SaffronCity_House_EventScript_841F032::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PIDGEY, 0
	msgbox Kanto_SaffronCity_House_Text_844BE1E, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_SaffronCity_House sign / bg event at 5,4
Kanto_SaffronCity_House_EventScript_841F045::
	msgbox Kanto_SaffronCity_House_Text_844BE62, MSGBOX_SIGN
	end

Kanto_SaffronCity_House_Text_844BDCF::
	.string "…Thank you for writing.\n"
	.string "I hope to see you soon!\p"
	.string "…Hey!\n"
	.string "Don’t look at my letter!$"

Kanto_SaffronCity_House_Text_844BE1E::
	.string "PIDGEY: Kurukkoo!$"

Kanto_SaffronCity_House_Text_844BE30::
	.string "The COPYCAT is cute!\n"
	.string "I’m getting her a POKé DOLL.$"

Kanto_SaffronCity_House_Text_844BE62::
	.string "I was given a PP UP as a gift.\p"
	.string "It’s used to increase the PP of\n"
	.string "a POKéMON’s move.\p"
	.string "I should use it on a powerful move\n"
	.string "with only a few PP.$"
