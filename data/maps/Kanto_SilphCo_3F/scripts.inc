Kanto_SilphCo_3F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_3F_EventScript_8411421
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

Kanto_SilphCo_3F_EventScript_8411421::
	call_if_unset FLAG_QZ_0x21F8, Kanto_SilphCo_3F_EventScript_8458668
	call_if_unset FLAG_QZ_0x21F9, Kanto_SilphCo_3F_EventScript_845869F
	end

@ Kanto_SilphCo_3F object 2 (local id 2, gfx 0x3F5B) at 31,13
Kanto_SilphCo_3F_EventScript_8411434::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_3F_EventScript_841144B
	msgbox Kanto_SilphCo_3F_Text_8425F8E, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_3F_EventScript_841144B::
	msgbox Kanto_SilphCo_3F_Text_8425FB4, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_3F sign / bg event at 29,1
Kanto_SilphCo_3F_EventScript_8411455::
	msgbox Kanto_SilphCo_3F_Text_84260BD, MSGBOX_SIGN
	end

@ Kanto_SilphCo_3F object 3 (local id 3, gfx 0x3F43) at 4,12
Kanto_SilphCo_3F_EventScript_841145E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8530, 0, Kanto_SilphCo_3F_Text_8426029, Kanto_SilphCo_3F_Text_842605E
	msgbox Kanto_SilphCo_3F_Text_8426071, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_3F object 1 (local id 1, gfx 0x3F3D) at 25,9
Kanto_SilphCo_3F_EventScript_8411475::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8567, 0, Kanto_SilphCo_3F_Text_8425FD7, Kanto_SilphCo_3F_Text_8425FF2
	msgbox Kanto_SilphCo_3F_Text_8425FFD, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_3F_EventScript_8458668::
	setmetatile 9, 0xB, 0x3C0, 1
	setmetatile 0xA, 0xB, 0x3C1, 1
	setmetatile 9, 0xC, 0x3C2, 1
	setmetatile 0xA, 0xC, 0x3C3, 1
	setmetatile 9, 0xD, 0x3C4, 1
	setmetatile 0xA, 0xD, 0x3C5, 1
	return

Kanto_SilphCo_3F_EventScript_845869F::
	setmetatile 0x14, 0xB, 0x3C0, 1
	setmetatile 0x15, 0xB, 0x3C1, 1
	setmetatile 0x14, 0xC, 0x3C2, 1
	setmetatile 0x15, 0xC, 0x3C3, 1
	setmetatile 0x14, 0xD, 0x3C4, 1
	setmetatile 0x15, 0xD, 0x3C5, 1
	return

@ Kanto_SilphCo_3F sign / bg event at 10,12
@ Kanto_SilphCo_3F sign / bg event at 10,13
@ Kanto_SilphCo_3F sign / bg event at 9,12
@ Kanto_SilphCo_3F sign / bg event at 9,13
Kanto_SilphCo_3F_EventScript_8458E12::
	lockall
	setvar VAR_TEMP_1, 3
	setvar VAR_0x8004, 0x21F8
	goto_if_set FLAG_QZ_0x21F8, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_3F sign / bg event at 21,12
@ Kanto_SilphCo_3F sign / bg event at 21,13
@ Kanto_SilphCo_3F sign / bg event at 20,12
@ Kanto_SilphCo_3F sign / bg event at 20,13
Kanto_SilphCo_3F_EventScript_8458E2C::
	lockall
	setvar VAR_TEMP_1, 4
	setvar VAR_0x8004, 0x21F9
	goto_if_set FLAG_QZ_0x21F9, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_3F object 4 (local id 4, gfx 0x3F0E) at 8,10
Kanto_SilphCo_3F_EventScript_846BD2C::
	setorcopyvar VAR_0x8000, ITEM_HYPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_3F_Text_8425F8E::
	.string "I work for SILPH CO.\n"
	.string "What am I to do?$"

Kanto_SilphCo_3F_Text_8425FB4::
	.string "{PLAYER}!\n"
	.string "You and your POKéMON saved us!$"

Kanto_SilphCo_3F_Text_8425FD7::
	.string "Quit messing with us, kid!$"

Kanto_SilphCo_3F_Text_8425FF2::
	.string "I give up!$"

Kanto_SilphCo_3F_Text_8425FFD::
	.string "A hint? You can open doors with\n"
	.string "a CARD KEY!$"

Kanto_SilphCo_3F_Text_8426029::
	.string "I support TEAM ROCKET more than\n"
	.string "I support SILPH CO.!$"

Kanto_SilphCo_3F_Text_842605E::
	.string "You really got me!$"

Kanto_SilphCo_3F_Text_8426071::
	.string "Humph…\p"
	.string "TEAM ROCKET said that if I helped\n"
	.string "them, they’d let me study POKéMON.$"

Kanto_SilphCo_3F_Text_84260BD::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "3F$"
