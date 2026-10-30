Kanto_CeladonCity_Condominiums_RoofRoom_MapScripts::
	.byte 0

@ Kanto_CeladonCity_Condominiums_RoofRoom object 1 (local id 1, gfx 0x3EEB) at 3,5
Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C73A::
	msgbox Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446603, MSGBOX_NPC
	end

@ Kanto_CeladonCity_Condominiums_RoofRoom object 2 (local id 2, gfx 0x3F0E) at 7,3
Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C743::
	lock
	faceplayer
	setvar VAR_TEMP_1, 0x85
	call Quetzal_EventScript_8361A4D
	qz_givemon_ex SPECIES_EEVEE, 0x19, ITEM_NONE, 1, 2, 3
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C781
	goto_if_eq VAR_RESULT, 1, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7B5
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8459563
	release
	end

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C781::
	removeobject 2
	playfanfare MUS_LEVEL_UP
	message Kanto_CeladonCity_Condominiums_RoofRoom_Text_84466BD
	waitmessage
	waitfanfare
	bufferspeciesname 0, SPECIES_EEVEE
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7EF
	call Quetzal_EventScript_84590EA
	call Quetzal_EventScript_84579AE
	goto Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7EF

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7B5::
	removeobject 2
	playfanfare MUS_LEVEL_UP
	message Kanto_CeladonCity_Condominiums_RoofRoom_Text_84466BD
	waitmessage
	waitfanfare
	bufferspeciesname 0, SPECIES_EEVEE
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7E4
	call Quetzal_EventScript_84590F6
	goto Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7E4

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7E4::
	call Quetzal_EventScript_84590FF
	goto Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7EF

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7EF::
	setflag FLAG_QZ_0x2145
	release
	end

@ Kanto_CeladonCity_Condominiums_RoofRoom sign / bg event at 4,4
Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7F4::
	msgbox Kanto_CeladonCity_Condominiums_RoofRoom_Text_844690F, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_Condominiums_RoofRoom sign / bg event at 4,1
@ Kanto_CeladonCity_Condominiums_RoofRoom sign / bg event at 5,1
Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C7FD::
	lockall
	msgbox Kanto_CeladonCity_Condominiums_RoofRoom_Text_84466EE, MSGBOX_DEFAULT
	message Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446720
	waitmessage
	setvar VAR_0x8004, 4
	multichoice 0, 0, MULTI_QZ_0x6C, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C89D
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8AB
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8B9
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8C7
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8C7
	end

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C854::
	message Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446720
	waitmessage
	multichoice 0, 0, MULTI_QZ_0x6C, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C89D
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8AB
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8B9
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8C7
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8C7
	end

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C89D::
	msgbox Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446743, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C854

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8AB::
	msgbox Kanto_CeladonCity_Condominiums_RoofRoom_Text_84467C1, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C854

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8B9::
	msgbox Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446868, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C854

Kanto_CeladonCity_Condominiums_RoofRoom_EventScript_841C8C7::
	releaseall
	end

Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446603::
	.string "There is nothing that I don’t know,\n"
	.string "like I wrote on the blackboard.\p"
	.string "I know about the world of POKéMON\n"
	.string "in your GAME BOY ADVANCE!\p"
	.string "Get together with your friends and\n"
	.string "enjoy trading POKéMON!$"

Kanto_CeladonCity_Condominiums_RoofRoom_Text_84466BD::
	.string "{PLAYER} obtained an EEVEE!$"

Kanto_CeladonCity_Condominiums_RoofRoom_Text_84466EE::
	.string "POKéMON Lecture\p"
	.string "Playing with the Wireless Adapter$"

Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446720::
	.string "Which heading do you want to read?$"

Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446743::
	.string "The POKéMON WIRELESS CLUB is\n"
	.string "upstairs at any POKéMON CENTER.\p"
	.string "Visit one to link up with friend(s)\n"
	.string "using your Wireless Adapter.$"

Kanto_CeladonCity_Condominiums_RoofRoom_Text_84467C1::
	.string "To link only with a friend, go to\n"
	.string "the DIRECT CORNER.\p"
	.string "It is the right counter at the\n"
	.string "POKéMON WIRELESS CLUB.\p"
	.string "Go into the TRADE CORNER or the\n"
	.string "COLOSSEUM with your friend.$"

Kanto_CeladonCity_Condominiums_RoofRoom_Text_8446868::
	.string "To link and communicate with\n"
	.string "anyone, go to the UNION ROOM.\p"
	.string "It is the left counter at the\n"
	.string "POKéMON WIRELESS CLUB.\p"
	.string "Go to the UNION ROOM and enjoy\n"
	.string "meeting other TRAINERS.$"

Kanto_CeladonCity_Condominiums_RoofRoom_Text_844690F::
	.string "It’s a pamphlet on move training.\p"
	.string "… …\p"
	.string "Moves can be managed directly from\n"
	.string "the POKéMON team menu.\p"
	.string "Field techniques are unlocked by\n"
	.string "earning BADGES.\p"
	.string "SILPH CO.$"
