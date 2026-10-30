Kanto_FuchsiaCity_WardensHouse_MapScripts::
	.byte 0

@ Kanto_FuchsiaCity_WardensHouse object 1 (local id 1, gfx 0x3F2D) at 3,5
Kanto_FuchsiaCity_WardensHouse_EventScript_841D8E7::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2151, Kanto_FuchsiaCity_WardensHouse_EventScript_841D998
	goto_if_set FLAG_QZ_0x21B3, Kanto_FuchsiaCity_WardensHouse_EventScript_841D91B
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_8448D74, MSGBOX_YESNO
	call_if_eq VAR_RESULT, 1, Kanto_FuchsiaCity_WardensHouse_EventScript_841D986
	call_if_eq VAR_RESULT, 0, Kanto_FuchsiaCity_WardensHouse_EventScript_841D98F
	release
	end

Kanto_FuchsiaCity_WardensHouse_EventScript_841D91B::
	textcolor 3
	playfanfare MUS_OBTAIN_TMHM
	message Kanto_FuchsiaCity_WardensHouse_Text_8448DFF
	waitmessage
	waitfanfare
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_8448E25, MSGBOX_DEFAULT
	call Quetzal_EventScript_8456AB7
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_FuchsiaCity_WardensHouse_EventScript_841D974
	call_if_eq VAR_RESULT, 1, Kanto_FuchsiaCity_WardensHouse_EventScript_841D97D
	additem ITEM_NUGGET, 1
	loadword 0, Kanto_FuchsiaCity_WardensHouse_Text_8448FE3
	setorcopyvar VAR_0x8000, 0x31B
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2151
	removeitem ITEM_GOLD_TEETH, 1
	release
	end

Kanto_FuchsiaCity_WardensHouse_EventScript_841D974::
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_8448E45, MSGBOX_DEFAULT
	return

Kanto_FuchsiaCity_WardensHouse_EventScript_841D97D::
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_8448F11, MSGBOX_DEFAULT
	return

Kanto_FuchsiaCity_WardensHouse_EventScript_841D986::
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_8448DBA, MSGBOX_DEFAULT
	return

Kanto_FuchsiaCity_WardensHouse_EventScript_841D98F::
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_8448DDE, MSGBOX_DEFAULT
	return

Kanto_FuchsiaCity_WardensHouse_EventScript_841D998::
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_844900F, MSGBOX_DEFAULT
	release
	end

@ Kanto_FuchsiaCity_WardensHouse sign / bg event at 4,4
@ Kanto_FuchsiaCity_WardensHouse sign / bg event at 5,4
Kanto_FuchsiaCity_WardensHouse_EventScript_841D9A2::
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_8449084, MSGBOX_SIGN
	end

@ Kanto_FuchsiaCity_WardensHouse sign / bg event at 7,4
@ Kanto_FuchsiaCity_WardensHouse sign / bg event at 6,4
Kanto_FuchsiaCity_WardensHouse_EventScript_841D9AB::
	msgbox Kanto_FuchsiaCity_WardensHouse_Text_84490AF, MSGBOX_SIGN
	end

@ Kanto_FuchsiaCity_WardensHouse object 3 (local id 3, gfx 0x3F0E) at 11,5
Kanto_FuchsiaCity_WardensHouse_EventScript_846BF88::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_FuchsiaCity_WardensHouse_Text_8448D74::
	.string "WARDEN: Hif fuff hefifoo!\p"
	.string "Ha lof ha feef ee hafahi ho.\n"
	.string "Heff hee fwee!$"

Kanto_FuchsiaCity_WardensHouse_Text_8448DBA::
	.string "Ah howhee ho hoo!\n"
	.string "Eef ee hafahi ho!$"

Kanto_FuchsiaCity_WardensHouse_Text_8448DDE::
	.string "Ha?\n"
	.string "He ohay heh ha hoo ee haheh!$"

Kanto_FuchsiaCity_WardensHouse_Text_8448DFF::
	.string "{PLAYER} gave the GOLD TEETH\n"
	.string "to the WARDEN.$"

Kanto_FuchsiaCity_WardensHouse_Text_8448E25::
	.string "The WARDEN popped in his teeth!$"

Kanto_FuchsiaCity_WardensHouse_Text_8448E45::
	.string "WARDEN: Thanks, son!\n"
	.string "You’re a real lifesaver!\p"
	.string "No one could understand a word\n"
	.string "that I said, not a one!\p"
	.string "I was too ashamed to show my face\n"
	.string "around the OFFICE, even.\p"
	.string "Let me give you something for your\n"
	.string "trouble.$"

Kanto_FuchsiaCity_WardensHouse_Text_8448F11::
	.string "WARDEN: Thank you, lassie!\n"
	.string "You’re a real lifesaver!\p"
	.string "No one could understand a word\n"
	.string "that I said, not a one!\p"
	.string "I was too ashamed to show my face\n"
	.string "around the OFFICE, even.\p"
	.string "Let me give you something for your\n"
	.string "trouble.$"

Kanto_FuchsiaCity_WardensHouse_Text_8448FE3::
	.string "{PLAYER} received BLACK AUGURITE\n"
	.string "from the WARDEN.$"

Kanto_FuchsiaCity_WardensHouse_Text_844900F::
	.string "WARDEN: BLACK AUGURITE can make\n"
	.string "a certain POKéMON evolve.\p"
	.string "Thanks again for returning my\n"
	.string "GOLD TEETH!$"

Kanto_FuchsiaCity_WardensHouse_Text_8449084::
	.string "POKéMON photos and fossils are on\n"
	.string "display.$"

Kanto_FuchsiaCity_WardensHouse_Text_84490AF::
	.string "Old POKéMON merchandise is on\n"
	.string "display.$"
