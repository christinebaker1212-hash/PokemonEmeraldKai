Kanto_RocketHideout_B3F_MapScripts::
	.byte 0

@ Kanto_RocketHideout_B3F object 2 (local id 2, gfx 0x3F3D) at 2,20
Kanto_RocketHideout_B3F_EventScript_8410F66::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8556, 0, Kanto_RocketHideout_B3F_Text_84257D2, Kanto_RocketHideout_B3F_Text_8425802
	msgbox Kanto_RocketHideout_B3F_Text_8425813, MSGBOX_AUTOCLOSE
	end

@ Kanto_RocketHideout_B3F object 1 (local id 1, gfx 0x3F3D) at 19,9
Kanto_RocketHideout_B3F_EventScript_8410F7D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8557, 0, Kanto_RocketHideout_B3F_Text_8425856, Kanto_RocketHideout_B3F_Text_8425886
	msgbox Kanto_RocketHideout_B3F_Text_8425898, MSGBOX_AUTOCLOSE
	end

@ Kanto_RocketHideout_B3F object 3 (local id 3, gfx 0x3F0E) at 12,12
Kanto_RocketHideout_B3F_EventScript_846BBCA::
	setorcopyvar VAR_0x8000, 0x66
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B3F object 4 (local id 4, gfx 0x3F0E) at 19,14
Kanto_RocketHideout_B3F_EventScript_846BBD7::
	setorcopyvar VAR_0x8000, 0x88
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B3F object 5 (local id 5, gfx 0x3F0E) at 14,24
Kanto_RocketHideout_B3F_EventScript_846C0B3::
	setorcopyvar VAR_0x8000, 0x1B8
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_RocketHideout_B3F_Text_84257D2::
	.string "Stop meddling in TEAM ROCKET’s\n"
	.string "affairs or else!$"

Kanto_RocketHideout_B3F_Text_8425802::
	.string "Oof!\n"
	.string "Taken down!$"

Kanto_RocketHideout_B3F_Text_8425813::
	.string "SILPH SCOPE?\n"
	.string "Oh, that machine the BOSS stole.\p"
	.string "It’s here somewhere.$"

Kanto_RocketHideout_B3F_Text_8425856::
	.string "We got word from upstairs that you\n"
	.string "were coming!$"

Kanto_RocketHideout_B3F_Text_8425886::
	.string "What?\n"
	.string "I lost? No!$"

Kanto_RocketHideout_B3F_Text_8425898::
	.string "Go ahead and go!\p"
	.string "But you can’t run the elevator\n"
	.string "without the LIFT KEY.$"
