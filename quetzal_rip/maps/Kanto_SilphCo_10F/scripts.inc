Kanto_SilphCo_10F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_10F_EventScript_8411ACB
	.byte 0

Kanto_SilphCo_10F_EventScript_8411ACB::
	call_if_unset FLAG_QZ_0x21F4, Kanto_SilphCo_10F_EventScript_8458999
	end

@ Kanto_SilphCo_10F object 2 (local id 2, gfx 0x3F5A) at 10,17
Kanto_SilphCo_10F_EventScript_8411AD5::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_10F_EventScript_8411AEC
	msgbox Kanto_SilphCo_10F_Text_84274A1, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_10F_EventScript_8411AEC::
	msgbox Kanto_SilphCo_10F_Text_84274B5, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_10F sign / bg event at 7,1
Kanto_SilphCo_10F_EventScript_8411AF6::
	msgbox Kanto_SilphCo_10F_Text_84274DF, MSGBOX_SIGN
	end

@ Kanto_SilphCo_10F object 1 (local id 1, gfx 0x3F43) at 2,7
Kanto_SilphCo_10F_EventScript_8411AFF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8537, 0, Kanto_SilphCo_10F_Text_842743E, Kanto_SilphCo_10F_Text_842745A
	msgbox Kanto_SilphCo_10F_Text_842746D, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_10F object 3 (local id 3, gfx 0x3F3D) at 0,13
Kanto_SilphCo_10F_EventScript_8411B16::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8581, 0, Kanto_SilphCo_10F_Text_84273D4, Kanto_SilphCo_10F_Text_84273FF
	msgbox Kanto_SilphCo_10F_Text_842740C, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_10F_EventScript_8458999::
	setmetatile 0xC, 0xB, 0x3B0, 1
	setmetatile 0xD, 0xB, 0x3B1, 1
	setmetatile 0xC, 0xC, 0x3B8, 1
	setmetatile 0xD, 0xC, 0x3B9, 1
	setmetatile 0xD, 0xD, 0x335, 0
	return

@ Kanto_SilphCo_10F sign / bg event at 12,11
@ Kanto_SilphCo_10F sign / bg event at 13,11
@ Kanto_SilphCo_10F sign / bg event at 12,12
@ Kanto_SilphCo_10F sign / bg event at 13,12
Kanto_SilphCo_10F_EventScript_8458FB2::
	lockall
	setvar VAR_TEMP_1, 0x13
	setvar VAR_0x8004, 0x21F4
	goto_if_set FLAG_QZ_0x21F4, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_10F object 4 (local id 4, gfx 0x3F0E) at 5,15
Kanto_SilphCo_10F_EventScript_846BDCB::
	setorcopyvar VAR_0x8000, 0x46
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_10F object 5 (local id 5, gfx 0x3F0E) at 2,15
Kanto_SilphCo_10F_EventScript_846BDD8::
	setorcopyvar VAR_0x8000, 3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_10F object 6 (local id 6, gfx 0x3F0E) at 3,17
Kanto_SilphCo_10F_EventScript_846BDE5::
	setorcopyvar VAR_0x8000, 0x66
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_10F_Text_84273D4::
	.string "Welcome to 10F!\n"
	.string "So good of you to join me!$"

Kanto_SilphCo_10F_Text_84273FF::
	.string "I’m stunned!$"

Kanto_SilphCo_10F_Text_842740C::
	.string "Nice try, but the boardroom is up\n"
	.string "one more floor.$"

Kanto_SilphCo_10F_Text_842743E::
	.string "Enough of your silly games!$"

Kanto_SilphCo_10F_Text_842745A::
	.string "No continues left!$"

Kanto_SilphCo_10F_Text_842746D::
	.string "Are you satisfied with beating me?\n"
	.string "Then go on home!$"

Kanto_SilphCo_10F_Text_84274A1::
	.string "Waaaaa!\n"
	.string "I’m scared!$"

Kanto_SilphCo_10F_Text_84274B5::
	.string "About me crying…\n"
	.string "Please keep it a secret.$"

Kanto_SilphCo_10F_Text_84274DF::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "10F$"
