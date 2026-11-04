Kanto_Route16_House_MapScripts::
	.byte 0

@ Kanto_Route16_House object 1 (local id 1, gfx 0x3F57) at 4,2
Kanto_Route16_House_EventScript_84201C1::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x214F, Kanto_Route16_House_EventScript_842021A
	msgbox Kanto_Route16_House_Text_844FCAD, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_HM02   @ Moemon Kai: FireRed's HM (tools/quetzal_port/fix_hm_givers.py)
	call EventScript_GiveHmOnce
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
	.string "You've found my secret retreat.\p"
	.string "Please don't tell anyone I'm here.\n"
	.string "I'll make it up to you with this!$"

Kanto_Route16_House_Text_844FD44::
	.string "HM02 is FLY.\n"
	.string "It's a wonderfully convenient move.\p"
	.string "Please, put it to good use.$"

Kanto_Route16_House_Text_844FD8F::
	.string "You don't have any room for this.$"

Kanto_Route16_House_Text_844FDB1::
	.string "FEAROW: Kyueen!$"
