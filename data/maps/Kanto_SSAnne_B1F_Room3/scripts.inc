Kanto_SSAnne_B1F_Room3_MapScripts::
	.byte 0

@ Kanto_SSAnne_B1F_Room3 object 1 (local id 1, gfx 0x3F41) at 4,4
Kanto_SSAnne_B1F_Room3_EventScript_8410908::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8331, 0, Kanto_SSAnne_B1F_Room3_Text_8424868, Kanto_SSAnne_B1F_Room3_Text_8424885
	msgbox Kanto_SSAnne_B1F_Room3_Text_8424894, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_B1F_Room3 object 2 (local id 2, gfx 0x3F0E) at 1,5
Kanto_SSAnne_B1F_Room3_EventScript_846BB48::
	setorcopyvar VAR_0x8000, ITEM_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SSAnne_B1F_Room3_Text_8424868::
	.string "I like feisty kids like you!$"

Kanto_SSAnne_B1F_Room3_Text_8424885::
	.string "Argh!\n"
	.string "Lost it!$"

Kanto_SSAnne_B1F_Room3_Text_8424894::
	.string "Sea POKéMON live in deep water.\n"
	.string "You’ll need a ROD to fish them up!$"
