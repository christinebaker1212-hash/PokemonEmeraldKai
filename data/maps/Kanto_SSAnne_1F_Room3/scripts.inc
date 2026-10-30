Kanto_SSAnne_1F_Room3_MapScripts::
	.byte 0

@ Kanto_SSAnne_1F_Room3 object 1 (local id 1, gfx 0x3F18) at 3,4
Kanto_SSAnne_1F_Room3_EventScript_8410797::
	msgbox Kanto_SSAnne_1F_Room3_Text_84240D6, MSGBOX_NPC
	end

@ Kanto_SSAnne_1F_Room3 object 3 (local id 3, gfx 0x3EE6) at 0,4
Kanto_SSAnne_1F_Room3_EventScript_84107A0::
	msgbox Kanto_SSAnne_1F_Room3_Text_84240AF, MSGBOX_NPC
	end

@ Kanto_SSAnne_1F_Room3 object 2 (local id 2, gfx 0x3F56) at 4,2
Kanto_SSAnne_1F_Room3_EventScript_84107A9::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_WIGGLYTUFF, 0
	msgbox Kanto_SSAnne_1F_Room3_Text_8424116, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Kanto_SSAnne_1F_Room3_Text_84240AF::
	.string "A world cruise is so elegant yet\n"
	.string "cozy!$"

Kanto_SSAnne_1F_Room3_Text_84240D6::
	.string "I always travel with WIGGLYTUFF.\n"
	.string "I never leave home without it.$"

Kanto_SSAnne_1F_Room3_Text_8424116::
	.string "WIGGLYTUFF: Puup pupuu!$"
