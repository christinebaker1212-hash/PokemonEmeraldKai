Kanto_SilphCo_2F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_2F_EventScript_8411398
	.byte 0

Kanto_SilphCo_2F_EventScript_8411398::
	call_if_unset FLAG_QZ_0x21F6, Kanto_SilphCo_2F_EventScript_845860C
	call_if_unset FLAG_QZ_0x21F7, Kanto_SilphCo_2F_EventScript_845863A
	end

@ Kanto_SilphCo_2F object 5 (local id 5, gfx 0x3F58) at 10,4
Kanto_SilphCo_2F_EventScript_84113AB::
	goto Kanto_SilphCo_2F_EventScript_8471C5D

@ Kanto_SilphCo_2F sign / bg event at 29,1
Kanto_SilphCo_2F_EventScript_84113B1::
	msgbox Kanto_SilphCo_2F_Text_8425F75, MSGBOX_SIGN
	end

@ Kanto_SilphCo_2F object 4 (local id 4, gfx 0x3F43) at 5,19
Kanto_SilphCo_2F_EventScript_84113BA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8528, 0, Kanto_SilphCo_2F_Text_8425C43, Kanto_SilphCo_2F_Text_8425C5F
	msgbox Kanto_SilphCo_2F_Text_8425C80, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_2F object 2 (local id 2, gfx 0x3F43) at 28,18
Kanto_SilphCo_2F_EventScript_84113D1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8529, 0, Kanto_SilphCo_2F_Text_8425CA7, Kanto_SilphCo_2F_Text_8425CC6
	msgbox Kanto_SilphCo_2F_Text_8425CD3, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_2F object 3 (local id 3, gfx 0x3F3D) at 20,14
Kanto_SilphCo_2F_EventScript_84113E8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8565, 0, Kanto_SilphCo_2F_Text_8425D0E, Kanto_SilphCo_2F_Text_8425D2B
	msgbox Kanto_SilphCo_2F_Text_8425D32, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_2F object 1 (local id 1, gfx 0x3F3D) at 29,12
Kanto_SilphCo_2F_EventScript_84113FF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8566, 0, Kanto_SilphCo_2F_Text_8425D8E, Kanto_SilphCo_2F_Text_8425DB9
	msgbox Kanto_SilphCo_2F_Text_8425DC3, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_2F_EventScript_845860C::
	setmetatile 5, 8, 0x3B0, 1
	setmetatile 6, 8, 0x3B1, 1
	setmetatile 5, 9, 0x3B8, 1
	setmetatile 6, 9, 0x3B9, 1
	setmetatile 6, 0xA, 0x335, 0
	return

Kanto_SilphCo_2F_EventScript_845863A::
	setmetatile 5, 0xF, 0x3B0, 1
	setmetatile 6, 0xF, 0x3B1, 1
	setmetatile 5, 0x10, 0x3B8, 1
	setmetatile 6, 0x10, 0x3B9, 1
	setmetatile 6, 0x11, 0x335, 0
	return

@ Kanto_SilphCo_2F sign / bg event at 5,8
@ Kanto_SilphCo_2F sign / bg event at 6,8
@ Kanto_SilphCo_2F sign / bg event at 5,9
@ Kanto_SilphCo_2F sign / bg event at 6,9
Kanto_SilphCo_2F_EventScript_8458DDE::
	lockall
	setvar VAR_TEMP_1, 1
	setvar VAR_0x8004, 0x21F6
	goto_if_set FLAG_QZ_0x21F6, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_2F sign / bg event at 5,15
@ Kanto_SilphCo_2F sign / bg event at 6,15
@ Kanto_SilphCo_2F sign / bg event at 5,16
@ Kanto_SilphCo_2F sign / bg event at 6,16
Kanto_SilphCo_2F_EventScript_8458DF8::
	lockall
	setvar VAR_TEMP_1, 2
	setvar VAR_0x8004, 0x21F7
	goto_if_set FLAG_QZ_0x21F7, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

Kanto_SilphCo_2F_EventScript_8471C5D::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_SilphCo_2F_Text_8425C43::
	.string "Help!\n"
	.string "I’m a SILPH employee.$"

Kanto_SilphCo_2F_Text_8425C5F::
	.string "How did you know\n"
	.string "I was a ROCKET?$"

Kanto_SilphCo_2F_Text_8425C80::
	.string "I work for both SILPH and TEAM\n"
	.string "ROCKET.$"

Kanto_SilphCo_2F_Text_8425CA7::
	.string "It’s off-limits here!\n"
	.string "Go home!$"

Kanto_SilphCo_2F_Text_8425CC6::
	.string "You’re good.$"

Kanto_SilphCo_2F_Text_8425CD3::
	.string "Can you solve the maze in here?\n"
	.string "It’s not easy, I tell you!$"

Kanto_SilphCo_2F_Text_8425D0E::
	.string "No kids are allowed in here!$"

Kanto_SilphCo_2F_Text_8425D2B::
	.string "Tough!$"

Kanto_SilphCo_2F_Text_8425D32::
	.string "Diamond-shaped tiles are teleport\n"
	.string "blocks.\p"
	.string "They’re transporters for this\n"
	.string "high-tech building.$"

Kanto_SilphCo_2F_Text_8425D8E::
	.string "Hey, kid!\n"
	.string "What are you wandering here for?$"

Kanto_SilphCo_2F_Text_8425DB9::
	.string "I goofed!$"

Kanto_SilphCo_2F_Text_8425DC3::
	.string "SILPH CO. will be associated with\n"
	.string "TEAM ROCKET!$"

Kanto_SilphCo_2F_Text_8425F75::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "2F$"
