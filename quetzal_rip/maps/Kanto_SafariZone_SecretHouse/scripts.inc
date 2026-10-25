Kanto_SafariZone_SecretHouse_MapScripts::
	.byte 0

@ Kanto_SafariZone_SecretHouse object 1 (local id 1, gfx 0x3F5B) at 6,5
Kanto_SafariZone_SecretHouse_EventScript_84121A9::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2150, Kanto_SafariZone_SecretHouse_EventScript_8412202
	msgbox Kanto_SafariZone_SecretHouse_Text_84284C2, MSGBOX_DEFAULT
	checkitemspace ITEM_PRISM_SCALE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_SafariZone_SecretHouse_EventScript_84121F8
	additem ITEM_PRISM_SCALE, 1
	loadword 0, Kanto_SafariZone_SecretHouse_Text_8428593
	setorcopyvar VAR_0x8000, 0xEA
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	msgbox Kanto_SafariZone_SecretHouse_Text_84285C1, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2150
	release
	end

Kanto_SafariZone_SecretHouse_EventScript_84121F8::
	msgbox Kanto_SafariZone_SecretHouse_Text_8428629, MSGBOX_DEFAULT
	release
	end

Kanto_SafariZone_SecretHouse_EventScript_8412202::
	msgbox Kanto_SafariZone_SecretHouse_Text_84285C1, MSGBOX_DEFAULT
	release
	end

Kanto_SafariZone_SecretHouse_Text_84284C2::
	.string "Ah! Finally!\p"
	.string "You’re the first person to reach\n"
	.string "the SECRET HOUSE!\p"
	.string "Although I made a campaign\n"
	.string "for our grand opening,\p"
	.string "I was getting worried that no one\n"
	.string "would win our campaign prize.\p"
	.string "Congratulations!\n"
	.string "You have won!$"

Kanto_SafariZone_SecretHouse_Text_8428593::
	.string "{PLAYER} received a PRISM SCALE\n"
	.string "from the attendant!$"

Kanto_SafariZone_SecretHouse_Text_84285C1::
	.string "A PRISM SCALE can make a certain\n"
	.string "POKéMON evolve.\p"
	.string "You earned it by reaching this\n"
	.string "house. Congratulations!$"

Kanto_SafariZone_SecretHouse_Text_8428629::
	.string "You don’t have room for this\n"
	.string "fabulous prize!$"
