Johto_CharcoalKiln_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_CharcoalKiln_OnTransition
	.byte 0

@ Moemon Kai: Crystal's FARFETCH'D at the kiln once it's home (EVENT_CHARCOAL_KILN_FARFETCH_D, cleared in ILEX FOREST
@ when the master gives HM01)
Johto_CharcoalKiln_OnTransition:
	setflag FLAG_JOHTO_HIDE_KILN_FARFETCHD
	goto_if_unset FLAG_QZ_0x1030, Johto_CharcoalKiln_OnTransitionEnd
	clearflag FLAG_JOHTO_HIDE_KILN_FARFETCHD
Johto_CharcoalKiln_OnTransitionEnd:
	end

Johto_CharcoalKiln_EventScript_Farfetchd::
	lock
	faceplayer
	msgbox Johto_CharcoalKiln_Text_Kwaa, MSGBOX_DEFAULT
	playmoncry SPECIES_FARFETCHD, 0
	waitmoncry
	release
	end

Johto_CharcoalKiln_Text_Kwaa::
	.string "FARFETCH'D: Kwaa!$"

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
	goto_if_set FLAG_JOHTO_GOT_KILN_CHARCOAL, Johto_CharcoalKiln_EventScript_83C5776   @ Moemon Kai: Crystal's own flag (0x1030 is HM01's)
	msgbox Johto_CharcoalKiln_Text_83C5960, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_CHARCOAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_JOHTO_GOT_KILN_CHARCOAL
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
	.string "I'm sorry--I forgot to thank you.\p"
	.string "This is CHARCOAL that I made.$"

Johto_CharcoalKiln_Text_83C59B6::
	.string "The SLOWPOKE came back, and you even\n"
	.string "found FARFETCH’D.\p"
	.string "You’re the coolest, man!$"
