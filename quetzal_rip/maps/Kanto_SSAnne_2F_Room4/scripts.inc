Kanto_SSAnne_2F_Room4_MapScripts::
	.byte 0

@ Kanto_SSAnne_2F_Room4 object 1 (local id 1, gfx 0x3F09) at 0,5
Kanto_SSAnne_2F_Room4_EventScript_841086C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8675, 0, Kanto_SSAnne_2F_Room4_Text_84244C9, Kanto_SSAnne_2F_Room4_Text_8424504
	msgbox Kanto_SSAnne_2F_Room4_Text_8424517, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_2F_Room4 object 2 (local id 2, gfx 0x3F16) at 3,3
Kanto_SSAnne_2F_Room4_EventScript_8410883::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8319, 0, Kanto_SSAnne_2F_Room4_Text_8424545, Kanto_SSAnne_2F_Room4_Text_842456D
	msgbox Kanto_SSAnne_2F_Room4_Text_842457B, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_2F_Room4 object 3 (local id 3, gfx 0x3F0E) at 2,4
Kanto_SSAnne_2F_Room4_EventScript_846BB2E::
	setorcopyvar VAR_0x8000, 0x79
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SSAnne_2F_Room4_Text_84244C9::
	.string "Which do you find more worthy,\n"
	.string "a strong or a rare POKéMON?$"

Kanto_SSAnne_2F_Room4_Text_8424504::
	.string "I must salute you!$"

Kanto_SSAnne_2F_Room4_Text_8424517::
	.string "Personally, I prefer strong and\n"
	.string "rare POKéMON.$"

Kanto_SSAnne_2F_Room4_Text_8424545::
	.string "I don’t believe I saw you at the\n"
	.string "party?$"

Kanto_SSAnne_2F_Room4_Text_842456D::
	.string "Take it easy!$"

Kanto_SSAnne_2F_Room4_Text_842457B::
	.string "Oh, I adore your strong POKéMON!\n"
	.string "Oh, how I envy you for them!$"
