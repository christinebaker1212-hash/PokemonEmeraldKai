Johto_CharcoalKiln_MapScripts::
	.byte 0

@ Johto_CharcoalKiln object 1 (local id 1, gfx 0x2C) at 2,4
Johto_CharcoalKiln_EventScript_83C56D5::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1008, Johto_CharcoalKiln_EventScript_83C5713
	goto_if_set FLAG_QZ_0x1087, Johto_CharcoalKiln_EventScript_83C5708
	goto_if_set FLAG_QZ_0x1084, Johto_CharcoalKiln_EventScript_83C56FD
	msgbox Johto_CharcoalKiln_Text_83C5781, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CharcoalKiln_EventScript_83C56FD::
	msgbox Johto_CharcoalKiln_Text_83C5808, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CharcoalKiln_EventScript_83C5708::
	msgbox Johto_CharcoalKiln_Text_83C5824, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CharcoalKiln_EventScript_83C5713::
	msgbox Johto_CharcoalKiln_Text_83C589A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CharcoalKiln object 2 (local id 2, gfx 0xB) at 4,2
Johto_CharcoalKiln_EventScript_83C571E::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1008, Johto_CharcoalKiln_EventScript_83C5748
	goto_if_set FLAG_QZ_0x1084, Johto_CharcoalKiln_EventScript_83C573D
	msgbox Johto_CharcoalKiln_Text_83C5906, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CharcoalKiln_EventScript_83C573D::
	msgbox Johto_CharcoalKiln_Text_83C5948, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CharcoalKiln_EventScript_83C5748::
	goto_if_set FLAG_QZ_0x1030, Johto_CharcoalKiln_EventScript_83C5776
	msgbox Johto_CharcoalKiln_Text_83C5960, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_CHARCOAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1030
	closemessage
	release
	end

Johto_CharcoalKiln_EventScript_83C5776::
	msgbox Johto_CharcoalKiln_Text_83C59B6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CharcoalKiln_Text_83C5781::
	.string "All the SLOWPOKE have disappeared\n"
	.string "from the town.\p"
	.string "The forest’s protector may be\n"
	.string "angry with us…\p"
	.string "It may be a bad omen. We should\n"
	.string "stay in.$"

Johto_CharcoalKiln_Text_83C5808::
	.string "The SLOWPOKE have returned…$"

Johto_CharcoalKiln_Text_83C5824::
	.string "The SLOWPOKE have returned…\p"
	.string "But my APPRENTICE hasn’t come back\n"
	.string "from ILEX FOREST.\p"
	.string "Where in the world is that lazy guy?$"

Johto_CharcoalKiln_Text_83C589A::
	.string "You chased off TEAM ROCKET and\n"
	.string "went to ILEX FOREST alone?\p"
	.string "That takes guts! I like that.\n"
	.string "Come train with us.$"

Johto_CharcoalKiln_Text_83C5906::
	.string "Where have all the SLOWPOKE gone?\p"
	.string "Are they out playing somewhere?$"

Johto_CharcoalKiln_Text_83C5948::
	.string "The SLOWPOKE came back!$"

Johto_CharcoalKiln_Text_83C5960::
	.string "My boss left your gift with me.\p"
	.string "Thanks for finding FARFETCH’D!\n"
	.string "Here is your CHARCOAL.$"

Johto_CharcoalKiln_Text_83C59B6::
	.string "The SLOWPOKE came back, and you even\n"
	.string "found FARFETCH’D.\p"
	.string "You’re the coolest, man!$"
