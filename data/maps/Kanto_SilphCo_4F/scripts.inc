Kanto_SilphCo_4F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_4F_EventScript_8411492
	.byte 0

Kanto_SilphCo_4F_EventScript_8411492::
	call_if_unset FLAG_QZ_0x21FA, Kanto_SilphCo_4F_EventScript_84586D6
	call_if_unset FLAG_QZ_0x21FB, Kanto_SilphCo_4F_EventScript_8458704
	end

@ Kanto_SilphCo_4F object 4 (local id 4, gfx 0x3F5B) at 5,5
Kanto_SilphCo_4F_EventScript_84114A5::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_4F_EventScript_84114BC
	msgbox Kanto_SilphCo_4F_Text_84260D6, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_4F_EventScript_84114BC::
	msgbox Kanto_SilphCo_4F_Text_84260F6, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_4F sign / bg event at 29,1
Kanto_SilphCo_4F_EventScript_84114C6::
	msgbox Kanto_SilphCo_4F_Text_8426238, MSGBOX_SIGN
	end

@ Kanto_SilphCo_4F object 3 (local id 3, gfx 0x3F43) at 14,8
Kanto_SilphCo_4F_EventScript_84114CF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8531, 0, Kanto_SilphCo_4F_Text_8426171, Kanto_SilphCo_4F_Text_8426191
	msgbox Kanto_SilphCo_4F_Text_84261A9, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_4F object 2 (local id 2, gfx 0x3F3D) at 10,19
Kanto_SilphCo_4F_EventScript_84114E6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8568, 0, Kanto_SilphCo_4F_Text_8426110, Kanto_SilphCo_4F_Text_842613C
	msgbox Kanto_SilphCo_4F_Text_8426143, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_4F object 1 (local id 1, gfx 0x3F3D) at 33,16
Kanto_SilphCo_4F_EventScript_84114FD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8569, 0, Kanto_SilphCo_4F_Text_84261FA, Kanto_SilphCo_4F_Text_842620C
	msgbox Kanto_SilphCo_4F_Text_8426219, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_4F_EventScript_84586D6::
	setmetatile 3, 0x10, 0x3B0, 1
	setmetatile 4, 0x10, 0x3B1, 1
	setmetatile 3, 0x11, 0x3B8, 1
	setmetatile 4, 0x11, 0x3B9, 1
	setmetatile 4, 0x12, 0x335, 0
	return

Kanto_SilphCo_4F_EventScript_8458704::
	setmetatile 0xE, 0xB, 0x3B0, 1
	setmetatile 0xF, 0xB, 0x3B1, 1
	setmetatile 0xE, 0xC, 0x3B8, 1
	setmetatile 0xF, 0xC, 0x3B9, 1
	setmetatile 0xF, 0xD, 0x335, 0
	return

@ Kanto_SilphCo_4F sign / bg event at 3,16
@ Kanto_SilphCo_4F sign / bg event at 4,16
@ Kanto_SilphCo_4F sign / bg event at 3,17
@ Kanto_SilphCo_4F sign / bg event at 4,17
Kanto_SilphCo_4F_EventScript_8458E46::
	lockall
	setvar VAR_TEMP_1, 5
	setvar VAR_0x8004, 0x21FA
	goto_if_set FLAG_QZ_0x21FA, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_4F sign / bg event at 15,11
@ Kanto_SilphCo_4F sign / bg event at 14,11
@ Kanto_SilphCo_4F sign / bg event at 14,12
@ Kanto_SilphCo_4F sign / bg event at 15,12
Kanto_SilphCo_4F_EventScript_8458E60::
	lockall
	setvar VAR_TEMP_1, 6
	setvar VAR_0x8004, 0x21FB
	goto_if_set FLAG_QZ_0x21FB, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_4F object 5 (local id 5, gfx 0x3F0E) at 3,11
Kanto_SilphCo_4F_EventScript_846BD39::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_4F object 6 (local id 6, gfx 0x3F0E) at 4,12
Kanto_SilphCo_4F_EventScript_846BD46::
	setorcopyvar VAR_0x8000, ITEM_ESCAPE_ROPE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_4F object 7 (local id 7, gfx 0x3F0E) at 2,13
Kanto_SilphCo_4F_EventScript_846BD53::
	setorcopyvar VAR_0x8000, ITEM_FULL_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_4F object 8 (local id 8, gfx 0x3F0E) at 30,18
Kanto_SilphCo_4F_EventScript_846C2EF::
	setorcopyvar VAR_0x8000, ITEM_MENTAL_HERB
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_4F_Text_84260D6::
	.string "Sssh!\n"
	.string "Can’t you see I’m hiding?$"

Kanto_SilphCo_4F_Text_84260F6::
	.string "Huh?\n"
	.string "TEAM ROCKET is gone?$"

Kanto_SilphCo_4F_Text_8426110::
	.string "TEAM ROCKET has taken control\n"
	.string "of SILPH CO.!$"

Kanto_SilphCo_4F_Text_842613C::
	.string "Arrgh!$"

Kanto_SilphCo_4F_Text_8426143::
	.string "Fwahahaha!\n"
	.string "My BOSS has been after this place!$"

Kanto_SilphCo_4F_Text_8426171::
	.string "My POKéMON is my loyal servant.$"

Kanto_SilphCo_4F_Text_8426191::
	.string "Darn!\n"
	.string "You weak POKéMON!$"

Kanto_SilphCo_4F_Text_84261A9::
	.string "The doors are electronically\n"
	.string "locked.\p"
	.string "It takes a CARD KEY to open the\n"
	.string "doors here.$"

Kanto_SilphCo_4F_Text_84261FA::
	.string "Intruder spotted!$"

Kanto_SilphCo_4F_Text_842620C::
	.string "Who are you?$"

Kanto_SilphCo_4F_Text_8426219::
	.string "I better tell the BOSS on 11F!$"

Kanto_SilphCo_4F_Text_8426238::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "4F$"
