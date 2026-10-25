Kanto_SSAnne_1F_Room2_MapScripts::
	.byte 0

@ Kanto_SSAnne_1F_Room2 object 3 (local id 3, gfx 0x3F58) at 2,6
Kanto_SSAnne_1F_Room2_EventScript_841075F::
	msgbox Kanto_SSAnne_1F_Room2_Text_842407A, MSGBOX_NPC
	end

@ Kanto_SSAnne_1F_Room2 object 1 (local id 1, gfx 0x3F16) at 5,3
Kanto_SSAnne_1F_Room2_EventScript_8410768::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8318, 0, Kanto_SSAnne_1F_Room2_Text_8423FCF, Kanto_SSAnne_1F_Room2_Text_8424004
	msgbox Kanto_SSAnne_1F_Room2_Text_842402F, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_1F_Room2 object 2 (local id 2, gfx 0x3F5C) at 0,4
Kanto_SSAnne_1F_Room2_EventScript_841077F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8288, 0, Kanto_SSAnne_1F_Room2_Text_8423F4E, Kanto_SSAnne_1F_Room2_Text_8423F66
	msgbox Kanto_SSAnne_1F_Room2_Text_8423F7A, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_1F_Room2 object 4 (local id 4, gfx 0x3F0E) at 2,7
Kanto_SSAnne_1F_Room2_EventScript_846BB14::
	setorcopyvar VAR_0x8000, 0x1AF
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SSAnne_1F_Room2_Text_8423F4E::
	.string "I love POKéMON!\n"
	.string "Do you?$"

Kanto_SSAnne_1F_Room2_Text_8423F66::
	.string "Wow! \n"
	.string "You’re great!$"

Kanto_SSAnne_1F_Room2_Text_8423F7A::
	.string "Listen, listen!\n"
	.string "Let me be your friend, okay?\p"
	.string "Then we can trade POKéMON and\n"
	.string "do stuff.$"

Kanto_SSAnne_1F_Room2_Text_8423FCF::
	.string "I collected these POKéMON\n"
	.string "from all around the world!$"

Kanto_SSAnne_1F_Room2_Text_8424004::
	.string "Oh, no!\n"
	.string "I went around the world for these!$"

Kanto_SSAnne_1F_Room2_Text_842402F::
	.string "You hurt my poor POKéMON!\p"
	.string "I demand that you heal them at a\n"
	.string "POKéMON CENTER!$"

Kanto_SSAnne_1F_Room2_Text_842407A::
	.string "We are cruising around the world,\n"
	.string "my children and I.$"
