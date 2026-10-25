Kanto_SSAnne_B1F_Room2_MapScripts::
	.byte 0

@ Kanto_SSAnne_B1F_Room2 object 1 (local id 1, gfx 0x3F41) at 3,5
Kanto_SSAnne_B1F_Room2_EventScript_84108F0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8330, 0, Kanto_SSAnne_B1F_Room2_Text_84247FB, Kanto_SSAnne_B1F_Room2_Text_842481E
	msgbox Kanto_SSAnne_B1F_Room2_Text_8424834, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_B1F_Room2 object 2 (local id 2, gfx 0x3F0E) at 3,2
Kanto_SSAnne_B1F_Room2_EventScript_846BB3B::
	setorcopyvar VAR_0x8000, 0x1E6
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SSAnne_B1F_Room2_Text_84247FB::
	.string "Even us sailors have POKéMON, too!$"

Kanto_SSAnne_B1F_Room2_Text_842481E::
	.string "Okay, you’re not bad.$"

Kanto_SSAnne_B1F_Room2_Text_8424834::
	.string "We caught all our POKéMON while\n"
	.string "we were out at sea.$"
