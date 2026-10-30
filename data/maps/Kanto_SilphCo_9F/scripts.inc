Kanto_SilphCo_9F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_9F_EventScript_8411A23
	.byte 0

Kanto_SilphCo_9F_EventScript_8411A23::
	call_if_unset FLAG_QZ_0x2204, Kanto_SilphCo_9F_EventScript_84588CF
	call_if_unset FLAG_QZ_0x2205, Kanto_SilphCo_9F_EventScript_8458906
	call_if_unset FLAG_QZ_0x2206, Kanto_SilphCo_9F_EventScript_845893D
	call_if_unset FLAG_QZ_0x2207, Kanto_SilphCo_9F_EventScript_845896B
	end

@ Kanto_SilphCo_9F object 3 (local id 3, gfx 0x3F58) at 2,16
Kanto_SilphCo_9F_EventScript_8411A48::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_9F_EventScript_8411A6D
	msgbox Kanto_SilphCo_9F_Text_842723B, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_84570E0
	msgbox Kanto_SilphCo_9F_Text_8427268, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_9F_EventScript_8411A6D::
	msgbox Kanto_SilphCo_9F_Text_8427277, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_9F sign / bg event at 17,1
Kanto_SilphCo_9F_EventScript_8411A77::
	msgbox Kanto_SilphCo_9F_Text_84273BB, MSGBOX_SIGN
	end

@ Kanto_SilphCo_9F object 1 (local id 1, gfx 0x3F43) at 26,16
Kanto_SilphCo_9F_EventScript_8411A80::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8536, 0, Kanto_SilphCo_9F_Text_84272E2, Kanto_SilphCo_9F_Text_8427312
	msgbox Kanto_SilphCo_9F_Text_8427323, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_9F object 4 (local id 4, gfx 0x3F3D) at 1,6
Kanto_SilphCo_9F_EventScript_8411A97::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8579, 0, Kanto_SilphCo_9F_Text_842728A, Kanto_SilphCo_9F_Text_84272AF
	msgbox Kanto_SilphCo_9F_Text_84272B7, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_9F object 2 (local id 2, gfx 0x3F3D) at 15,18
Kanto_SilphCo_9F_EventScript_8411AAE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8580, 0, Kanto_SilphCo_9F_Text_8427361, Kanto_SilphCo_9F_Text_8427387
	msgbox Kanto_SilphCo_9F_Text_842739F, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_9F_EventScript_84588CF::
	setmetatile 2, 9, 0x3C0, 1
	setmetatile 3, 9, 0x3C1, 1
	setmetatile 2, 0xA, 0x3C2, 1
	setmetatile 3, 0xA, 0x3C3, 1
	setmetatile 2, 0xB, 0x3C4, 1
	setmetatile 3, 0xB, 0x3C5, 1
	return

Kanto_SilphCo_9F_EventScript_8458906::
	setmetatile 0xC, 0xF, 0x3C0, 1
	setmetatile 0xD, 0xF, 0x3C1, 1
	setmetatile 0xC, 0x10, 0x3C2, 1
	setmetatile 0xD, 0x10, 0x3C3, 1
	setmetatile 0xC, 0x11, 0x3C4, 1
	setmetatile 0xD, 0x11, 0x3C5, 1
	return

Kanto_SilphCo_9F_EventScript_845893D::
	setmetatile 0x15, 6, 0x3B0, 1
	setmetatile 0x16, 6, 0x3B1, 1
	setmetatile 0x15, 7, 0x3B8, 1
	setmetatile 0x16, 7, 0x3B9, 1
	setmetatile 0x16, 8, 0x335, 0
	return

Kanto_SilphCo_9F_EventScript_845896B::
	setmetatile 0x15, 0xC, 0x3B0, 1
	setmetatile 0x16, 0xC, 0x3B1, 1
	setmetatile 0x15, 0xD, 0x3B8, 1
	setmetatile 0x16, 0xD, 0x3B9, 1
	setmetatile 0x16, 0xE, 0x335, 0
	return

@ Kanto_SilphCo_9F sign / bg event at 3,10
@ Kanto_SilphCo_9F sign / bg event at 3,11
@ Kanto_SilphCo_9F sign / bg event at 2,11
@ Kanto_SilphCo_9F sign / bg event at 2,10
Kanto_SilphCo_9F_EventScript_8458F4A::
	lockall
	setvar VAR_TEMP_1, 0xF
	setvar VAR_0x8004, 0x2204
	goto_if_set FLAG_QZ_0x2204, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_9F sign / bg event at 13,16
@ Kanto_SilphCo_9F sign / bg event at 13,17
@ Kanto_SilphCo_9F sign / bg event at 12,16
@ Kanto_SilphCo_9F sign / bg event at 12,17
Kanto_SilphCo_9F_EventScript_8458F64::
	lockall
	setvar VAR_TEMP_1, 0x10
	setvar VAR_0x8004, 0x2205
	goto_if_set FLAG_QZ_0x2205, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_9F sign / bg event at 21,6
@ Kanto_SilphCo_9F sign / bg event at 22,6
@ Kanto_SilphCo_9F sign / bg event at 22,7
@ Kanto_SilphCo_9F sign / bg event at 21,7
Kanto_SilphCo_9F_EventScript_8458F7E::
	lockall
	setvar VAR_TEMP_1, 0x11
	setvar VAR_0x8004, 0x2206
	goto_if_set FLAG_QZ_0x2206, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_9F sign / bg event at 21,13
@ Kanto_SilphCo_9F sign / bg event at 21,12
@ Kanto_SilphCo_9F sign / bg event at 22,12
@ Kanto_SilphCo_9F sign / bg event at 22,13
Kanto_SilphCo_9F_EventScript_8458F98::
	lockall
	setvar VAR_TEMP_1, 0x12
	setvar VAR_0x8004, 0x2207
	goto_if_set FLAG_QZ_0x2207, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

Kanto_SilphCo_9F_Text_842723B::
	.string "You look tired.\n"
	.string "You should take a quick nap.$"

Kanto_SilphCo_9F_Text_8427268::
	.string "Don’t give up!$"

Kanto_SilphCo_9F_Text_8427277::
	.string "Thank you so much.$"

Kanto_SilphCo_9F_Text_842728A::
	.string "Your POKéMON seem to adore you,\n"
	.string "kid!$"

Kanto_SilphCo_9F_Text_84272AF::
	.string "Ghaaah!$"

Kanto_SilphCo_9F_Text_84272B7::
	.string "If I had started as a TRAINER at\n"
	.string "your age…$"

Kanto_SilphCo_9F_Text_84272E2::
	.string "Your POKéMON have weak points!\n"
	.string "I can nail them!$"

Kanto_SilphCo_9F_Text_8427312::
	.string "You hammered me!$"

Kanto_SilphCo_9F_Text_8427323::
	.string "Exploiting weak spots does work.\n"
	.string "Think about type advantages.$"

Kanto_SilphCo_9F_Text_8427361::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

Kanto_SilphCo_9F_Text_8427387::
	.string "Warg!\n"
	.string "Brothers, I lost!$"

Kanto_SilphCo_9F_Text_842739F::
	.string "My brothers will avenge me!$"

Kanto_SilphCo_9F_Text_84273BB::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "9F$"
