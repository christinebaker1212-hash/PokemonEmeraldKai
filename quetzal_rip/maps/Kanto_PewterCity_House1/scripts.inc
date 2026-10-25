Kanto_PewterCity_House1_MapScripts::
	.byte 0

@ Kanto_PewterCity_House1 object 1 (local id 1, gfx 0x3EE6) at 1,2
Kanto_PewterCity_House1_EventScript_841A725::
	msgbox Kanto_PewterCity_House1_Text_84418E1, MSGBOX_NPC
	end

@ Kanto_PewterCity_House1 object 2 (local id 2, gfx 0x3F17) at 5,3
Kanto_PewterCity_House1_EventScript_841A72E::
	lock
	msgbox Kanto_PewterCity_House1_Text_84418D3, MSGBOX_DEFAULT
	call Kanto_PewterCity_House1_EventScript_841A751
	release
	end

@ Kanto_PewterCity_House1 object 3 (local id 3, gfx 0x3F28) at 6,3
Kanto_PewterCity_House1_EventScript_841A73E::
	lock
	faceplayer
	call Kanto_PewterCity_House1_EventScript_841A751
	applymovement 3, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_PewterCity_House1_EventScript_841A751::
	textcolor 3
	waitse
	playmoncry SPECIES_NIDORAN_M, 0
	msgbox Kanto_PewterCity_House1_Text_84418C1, MSGBOX_DEFAULT
	waitmoncry
	call Quetzal_EventScript_8456AB7
	return

Kanto_PewterCity_House1_Text_84418C1::
	.string "NIDORAN♂: Bowbow!$"

Kanto_PewterCity_House1_Text_84418D3::
	.string "NIDORAN, sit!$"

Kanto_PewterCity_House1_Text_84418E1::
	.string "Our POKéMON’s an outsider, so it’s\n"
	.string "finicky and hard to handle.\p"
	.string "An outsider is a POKéMON that you\n"
	.string "get in a trade.\p"
	.string "It grows fast, but it may ignore an\n"
	.string "unskilled TRAINER in battle.\p"
	.string "If only we had some BADGES…$"
