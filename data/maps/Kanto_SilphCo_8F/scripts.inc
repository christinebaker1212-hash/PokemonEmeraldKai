Kanto_SilphCo_8F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_8F_EventScript_84119A4
	.byte 0

Kanto_SilphCo_8F_EventScript_84119A4::
	call_if_unset FLAG_QZ_0x2203, Kanto_SilphCo_8F_EventScript_8458898
	end

@ Kanto_SilphCo_8F object 4 (local id 4, gfx 0x3F5B) at 2,5
Kanto_SilphCo_8F_EventScript_84119AE::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_8F_EventScript_84119C5
	msgbox Kanto_SilphCo_8F_Text_8427054, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_8F_EventScript_84119C5::
	msgbox Kanto_SilphCo_8F_Text_8427073, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_8F sign / bg event at 27,1
Kanto_SilphCo_8F_EventScript_84119CF::
	msgbox Kanto_SilphCo_8F_Text_8427199, MSGBOX_SIGN
	end

@ Kanto_SilphCo_8F object 3 (local id 3, gfx 0x3F43) at 9,4
Kanto_SilphCo_8F_EventScript_84119D8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8535, 0, Kanto_SilphCo_8F_Text_84270E4, Kanto_SilphCo_8F_Text_8427100
	msgbox Kanto_SilphCo_8F_Text_842710D, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_8F object 1 (local id 1, gfx 0x3F3D) at 28,5
Kanto_SilphCo_8F_EventScript_84119EF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8574, 0, Kanto_SilphCo_8F_Text_8427089, Kanto_SilphCo_8F_Text_84270A5
	msgbox Kanto_SilphCo_8F_Text_84270B6, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_8F object 2 (local id 2, gfx 0x3F3D) at 10,17
Kanto_SilphCo_8F_EventScript_8411A06::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8578, 0, Kanto_SilphCo_8F_Text_842713D, Kanto_SilphCo_8F_Text_8427163
	msgbox Kanto_SilphCo_8F_Text_8427177, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_8F_EventScript_8458898::
	setmetatile 5, 9, 0x3C0, 1
	setmetatile 6, 9, 0x3C1, 1
	setmetatile 5, 0xA, 0x3C2, 1
	setmetatile 6, 0xA, 0x3C3, 1
	setmetatile 5, 0xB, 0x3C4, 1
	setmetatile 6, 0xB, 0x3C5, 1
	return

@ Kanto_SilphCo_8F sign / bg event at 6,10
@ Kanto_SilphCo_8F sign / bg event at 6,11
@ Kanto_SilphCo_8F sign / bg event at 5,11
@ Kanto_SilphCo_8F sign / bg event at 5,10
Kanto_SilphCo_8F_EventScript_8458F30::
	lockall
	setvar VAR_TEMP_1, 0xE
	setvar VAR_0x8004, 0x2203
	goto_if_set FLAG_QZ_0x2203, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_8F object 6 (local id 6, gfx 0x3F43) at 29,9
Kanto_SilphCo_8F_EventScript_845D4BE::
	lock
	faceplayer
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_SilphCo_8F_Text_84271B2, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_8F object 5 (local id 5, gfx 0x3F0E) at 24,8
Kanto_SilphCo_8F_EventScript_846C031::
	setorcopyvar VAR_0x8000, ITEM_IRON
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_8F_Text_8427054::
	.string "I wonder if SILPH is finished…$"

Kanto_SilphCo_8F_Text_8427073::
	.string "Thanks for saving us!$"

Kanto_SilphCo_8F_Text_8427089::
	.string "That’s as far as you’ll go!$"

Kanto_SilphCo_8F_Text_84270A5::
	.string "Not enough grit!$"

Kanto_SilphCo_8F_Text_84270B6::
	.string "If you don’t turn back…\n"
	.string "I’ll call for backup!$"

Kanto_SilphCo_8F_Text_84270E4::
	.string "You’re causing us problems!$"

Kanto_SilphCo_8F_Text_8427100::
	.string "Huh?\n"
	.string "I lost?$"

Kanto_SilphCo_8F_Text_842710D::
	.string "So, what do you think of SILPH\n"
	.string "BUILDING’s maze?$"

Kanto_SilphCo_8F_Text_842713D::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

Kanto_SilphCo_8F_Text_8427163::
	.string "Whoo!\n"
	.string "Oh, Brothers!$"

Kanto_SilphCo_8F_Text_8427177::
	.string "I’ll leave you up to my brothers.$"

Kanto_SilphCo_8F_Text_8427199::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "8F$"

Kanto_SilphCo_8F_Text_84271B2::
	.string "TEAM ROCKET’s BOSS is terribly\n"
	.string "cruel!\p"
	.string "To him, POKéMON are just tools to\n"
	.string "be used.\p"
	.string "What will happen if that tyrant\n"
	.string "takes over our company…$"
