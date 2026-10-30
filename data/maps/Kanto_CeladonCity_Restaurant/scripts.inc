Kanto_CeladonCity_Restaurant_MapScripts::
	.byte 0

@ Kanto_CeladonCity_Restaurant object 1 (local id 1, gfx 0x3EF8) at 12,7
Kanto_CeladonCity_Restaurant_EventScript_841D453::
	msgbox Kanto_CeladonCity_Restaurant_Text_8447A24, MSGBOX_NPC
	end

@ Kanto_CeladonCity_Restaurant object 2 (local id 2, gfx 0x3F58) at 11,4
Kanto_CeladonCity_Restaurant_EventScript_841D45C::
	msgbox Kanto_CeladonCity_Restaurant_Text_8447A53, MSGBOX_NPC
	end

@ Kanto_CeladonCity_Restaurant object 4 (local id 4, gfx 0x3EE6) at 1,2
Kanto_CeladonCity_Restaurant_EventScript_841D465::
	lock
	faceplayer
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_CeladonCity_Restaurant_EventScript_841D4B8
	msgbox Kanto_CeladonCity_Restaurant_Text_8447AFF, MSGBOX_DEFAULT
	additem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_Restaurant_EventScript_841D4AE
	loadword 0, Kanto_CeladonCity_Restaurant_Text_8447B76
	setorcopyvar VAR_0x8000, 0x2CE
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2143
	release
	end

Kanto_CeladonCity_Restaurant_EventScript_841D4AE::
	msgbox Kanto_CeladonCity_Restaurant_Text_8447B9C, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_Restaurant_EventScript_841D4B8::
	setflag FLAG_QZ_0x2143
	msgbox Kanto_CeladonCity_Restaurant_Text_8447BB0, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_Restaurant object 5 (local id 5, gfx 0x3F5B) at 3,7
Kanto_CeladonCity_Restaurant_EventScript_841D4C5::
	msgbox Kanto_CeladonCity_Restaurant_Text_8447A90, MSGBOX_NPC
	end

@ Kanto_CeladonCity_Restaurant object 3 (local id 3, gfx 0x3F04) at 9,4
Kanto_CeladonCity_Restaurant_EventScript_841D4CE::
	msgbox Kanto_CeladonCity_Restaurant_Text_8447AC8, MSGBOX_NPC
	end

Kanto_CeladonCity_Restaurant_Text_8447A24::
	.string "Hi!\p"
	.string "Sorry, but we’re taking a break\n"
	.string "right now.$"

Kanto_CeladonCity_Restaurant_Text_8447A53::
	.string "My POKéMON are weak, so I often\n"
	.string "have to go to the DRUGSTORE.$"

Kanto_CeladonCity_Restaurant_Text_8447A90::
	.string "Psst! There’s a basement under the\n"
	.string "GAME CORNER, I hear.$"

Kanto_CeladonCity_Restaurant_Text_8447AC8::
	.string "Munch…\p"
	.string "The man at that table lost it all\n"
	.string "at the slots.$"

Kanto_CeladonCity_Restaurant_Text_8447AFF::
	.string "Go ahead! Laugh!\n"
	.string "I’m flat-out busted!\p"
	.string "No more slots for me!\n"
	.string "I’m going straight!\p"
	.string "Here!\n"
	.string "I won’t be needing this anymore!$"

Kanto_CeladonCity_Restaurant_Text_8447B76::
	.string "{PLAYER} received a COIN CASE\n"
	.string "from the man.$"

Kanto_CeladonCity_Restaurant_Text_8447B9C::
	.string "Make room for this!$"

Kanto_CeladonCity_Restaurant_Text_8447BB0::
	.string "I always thought I was going to\n"
	.string "win it back…$"
