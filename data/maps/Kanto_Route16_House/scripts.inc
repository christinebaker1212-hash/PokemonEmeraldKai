Kanto_Route16_House_MapScripts::
	.byte 0

@ Kanto_Route16_House object 1 (local id 1, gfx 0x3F57) at 4,2
Kanto_Route16_House_EventScript_84201C1::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x214F, Kanto_Route16_House_EventScript_842021A
	msgbox Kanto_Route16_House_Text_844FCAD, MSGBOX_DEFAULT
	checkitemspace ITEM_RAZOR_FANG, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route16_House_EventScript_8420210
	additem ITEM_RAZOR_FANG, 1
	loadword 0, Kanto_Route16_House_Text_844FD1C
	setorcopyvar VAR_0x8000, 0x1ED
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	msgbox Kanto_Route16_House_Text_844FD44, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x214F
	release
	end

Kanto_Route16_House_EventScript_8420210::
	msgbox Kanto_Route16_House_Text_844FD8F, MSGBOX_DEFAULT
	release
	end

Kanto_Route16_House_EventScript_842021A::
	msgbox Kanto_Route16_House_Text_844FD44, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route16_House object 2 (local id 2, gfx 0x3F05) at 9,3
Kanto_Route16_House_EventScript_8420224::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_FEAROW, 0
	msgbox Kanto_Route16_House_Text_844FDB1, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Kanto_Route16_House_Text_844FCAD::
	.string "Oh, dear.\n"
	.string "You’ve found my secret retreat.\p"
	.string "Please don’t tell anyone I’m here.\n"
	.string "I’ll make it up to you with this!$"

Kanto_Route16_House_Text_844FD1C::
	.string "{PLAYER} received a RAZOR FANG\n"
	.string "from the girl.$"

Kanto_Route16_House_Text_844FD44::
	.string "A RAZOR FANG can help a certain\n"
	.string "POKéMON evolve.\p"
	.string "Please put it to good use.$"

Kanto_Route16_House_Text_844FD8F::
	.string "You don’t have any room for this.$"

Kanto_Route16_House_Text_844FDB1::
	.string "FEAROW: Kyueen!$"
