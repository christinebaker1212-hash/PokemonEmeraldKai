Kanto_RocketHideout_B2F_MapScripts::
	.byte 0

@ Kanto_RocketHideout_B2F object 1 (local id 1, gfx 0x3F3D) at 20,6
Kanto_RocketHideout_B2F_EventScript_8410F4E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8555, 0, Kanto_RocketHideout_B2F_Text_8425744, Kanto_RocketHideout_B2F_Text_8425777
	msgbox Kanto_RocketHideout_B2F_Text_8425784, MSGBOX_AUTOCLOSE
	end

@ Kanto_RocketHideout_B2F object 2 (local id 2, gfx 0x3F0E) at 15,3
Kanto_RocketHideout_B2F_EventScript_846BB96::
	setorcopyvar VAR_0x8000, ITEM_X_SPEED
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B2F object 3 (local id 3, gfx 0x3F0E) at 0,6
Kanto_RocketHideout_B2F_EventScript_846BBA3::
	setorcopyvar VAR_0x8000, ITEM_MOON_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B2F object 4 (local id 4, gfx 0x3F0E) at 5,13
Kanto_RocketHideout_B2F_EventScript_846BBB0::
	setorcopyvar VAR_0x8000, ITEM_MENTAL_HERB
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B2F object 5 (local id 5, gfx 0x3F0E) at 1,17
Kanto_RocketHideout_B2F_EventScript_846BBBD::
	setorcopyvar VAR_0x8000, ITEM_SUPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_RocketHideout_B2F_Text_8425744::
	.string "BOSS said you can see ghosts with\n"
	.string "the SILPH SCOPE.$"

Kanto_RocketHideout_B2F_Text_8425777::
	.string "I surrender!$"

Kanto_RocketHideout_B2F_Text_8425784::
	.string "The TEAM ROCKET HQ has four\n"
	.string "underground floors.\p"
	.string "Think you can reach the BOSS?$"
