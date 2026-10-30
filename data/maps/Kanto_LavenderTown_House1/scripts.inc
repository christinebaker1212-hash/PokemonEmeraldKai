Kanto_LavenderTown_House1_MapScripts::
	.byte 0

@ Kanto_LavenderTown_House1 object 1 (local id 1, gfx 0x3EFC) at 4,4
Kanto_LavenderTown_House1_EventScript_841B21A::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x21ED, Kanto_LavenderTown_House1_EventScript_841B22F
	msgbox Kanto_LavenderTown_House1_Text_8443ADF, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_House1_EventScript_841B22F::
	msgbox Kanto_LavenderTown_House1_Text_8443B5A, MSGBOX_DEFAULT
	release
	end

@ Kanto_LavenderTown_House1 object 2 (local id 2, gfx 0x3EFE) at 8,6
Kanto_LavenderTown_House1_EventScript_841B239::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_CUBONE, 0
	msgbox Kanto_LavenderTown_House1_Text_8443ACD, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Kanto_LavenderTown_House1_Text_8443ACD::
	.string "CUBONE: Kyarugoo!$"

Kanto_LavenderTown_House1_Text_8443ADF::
	.string "Those horrible ROCKETS!\n"
	.string "They deserve no mercy!\p"
	.string "That poor CUBONE’s mother…\p"
	.string "It was killed trying to escape from\n"
	.string "TEAM ROCKET.$"

Kanto_LavenderTown_House1_Text_8443B5A::
	.string "The ghost of POKéMON TOWER is\n"
	.string "gone!\p"
	.string "Someone must have soothed its\n"
	.string "restless spirit!$"
