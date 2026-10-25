Kanto_SSAnne_CaptainsOffice_MapScripts::
	.byte 0

@ Kanto_SSAnne_CaptainsOffice object 1 (local id 1, gfx 0x3EF4) at 5,4
Kanto_SSAnne_CaptainsOffice_EventScript_84106B0::
	lock
	goto_if_set FLAG_QZ_0x214E, Kanto_SSAnne_CaptainsOffice_EventScript_841072E
	msgbox Kanto_SSAnne_CaptainsOffice_Text_8423C31, MSGBOX_DEFAULT
	textcolor 3
	message Kanto_SSAnne_CaptainsOffice_Text_8423C61
	waitmessage
	playfanfare MUS_HEAL
	waitfanfare
	call Quetzal_EventScript_8456AB7
	delay 0x32
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_SSAnne_CaptainsOffice_Text_8423C91, MSGBOX_DEFAULT
	additem ITEM_METAL_COAT, 1
	loadword 0, Kanto_SSAnne_CaptainsOffice_Text_8423D4F
	setorcopyvar VAR_0x8000, 0x1B9
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	msgbox Kanto_SSAnne_CaptainsOffice_Text_8423D7A, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x214E
	setvar VAR_QZ_0x5135, 1
	release
	end

Kanto_SSAnne_CaptainsOffice_EventScript_841072E::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_SSAnne_CaptainsOffice_Text_8423DE6, MSGBOX_DEFAULT
	release
	end

@ Kanto_SSAnne_CaptainsOffice sign / bg event at 5,3
Kanto_SSAnne_CaptainsOffice_EventScript_8410742::
	msgbox Kanto_SSAnne_CaptainsOffice_Text_8423E9B, MSGBOX_SIGN
	end

@ Kanto_SSAnne_CaptainsOffice sign / bg event at 2,4
@ Kanto_SSAnne_CaptainsOffice sign / bg event at 2,3
Kanto_SSAnne_CaptainsOffice_EventScript_841074B::
	msgbox Kanto_SSAnne_CaptainsOffice_Text_8423EB8, MSGBOX_SIGN
	end

Kanto_SSAnne_CaptainsOffice_Text_8423C31::
	.string "CAPTAIN: Ooargh…\n"
	.string "I feel hideous…\l"
	.string "Urrp! Seasick…$"

Kanto_SSAnne_CaptainsOffice_Text_8423C61::
	.string "{PLAYER} rubbed the CAPTAIN’s\n"
	.string "back!\p"
	.string "Rub-rub…\n"
	.string "Rub-rub…$"

Kanto_SSAnne_CaptainsOffice_Text_8423C91::
	.string "CAPTAIN: Whew! Thank you!\n"
	.string "I’m feeling much better now.\p"
	.string "Let me teach you my prized CUT\n"
	.string "technique.\p"
	.string "With the CASCADE BADGE, your\n"
	.string "POKéMON can now cut small trees.\p"
	.string "Please take this gift as well!$"

Kanto_SSAnne_CaptainsOffice_Text_8423D4F::
	.string "{PLAYER} received a METAL COAT\n"
	.string "from the CAPTAIN!$"

Kanto_SSAnne_CaptainsOffice_Text_8423D7A::
	.string "A METAL COAT helps certain\n"
	.string "POKéMON evolve.\p"
	.string "And now that you know CUT, try it\n"
	.string "on the trees around VERMILION!$"

Kanto_SSAnne_CaptainsOffice_Text_8423DE6::
	.string "CAPTAIN: …Whew!\p"
	.string "Now that I’m not sick anymore,\n"
	.string "I guess it’s time.\p"
	.string "The S.S. ANNE will set sail soon!\p"
	.string "Farewell, until our return to\n"
	.string "VERMILION CITY!$"

Kanto_SSAnne_CaptainsOffice_Text_8423E9B::
	.string "Yuck!\n"
	.string "Shouldn’t have looked!$"

Kanto_SSAnne_CaptainsOffice_Text_8423EB8::
	.string "How to Conquer Seasickness…\n"
	.string "The CAPTAIN’s reading this!$"
