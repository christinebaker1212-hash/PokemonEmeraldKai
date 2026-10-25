Kanto_SSAnne_B1F_Room5_MapScripts::
	.byte 0

@ Kanto_SSAnne_B1F_Room5 object 1 (local id 1, gfx 0x3EEE) at 3,5
Kanto_SSAnne_B1F_Room5_EventScript_841094F::
	msgbox Kanto_SSAnne_B1F_Room5_Text_84249BA, MSGBOX_NPC
	end

@ Kanto_SSAnne_B1F_Room5 object 2 (local id 2, gfx 0x3F1C) at 2,5
Kanto_SSAnne_B1F_Room5_EventScript_8410958::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MACHOKE, 0
	msgbox Kanto_SSAnne_B1F_Room5_Text_8424A06, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_SSAnne_B1F_Room5 object 3 (local id 3, gfx 0x3F0E) at 2,2
Kanto_SSAnne_B1F_Room5_EventScript_846BB55::
	setorcopyvar VAR_0x8000, 0x1D
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SSAnne_B1F_Room5_Text_84249BA::
	.string "My buddy MACHOKE is super strong!\p"
	.string "He has enough STRENGTH to move\n"
	.string "big rocks!$"

Kanto_SSAnne_B1F_Room5_Text_8424A06::
	.string "MACHOKE: Gwoh! Goggoh!$"
