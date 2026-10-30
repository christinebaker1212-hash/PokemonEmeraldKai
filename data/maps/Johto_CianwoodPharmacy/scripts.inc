Johto_CianwoodPharmacy_MapScripts::
	.byte 0

@ Johto_CianwoodPharmacy object 1 (local id 1, gfx 0x3E83) at 8,7
Johto_CianwoodPharmacy_EventScript_83CB959::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x500D, 5, Johto_CianwoodPharmacy_EventScript_83CB9A2
	checkitem ITEM_SECRET_POTION, 1
	goto_if_eq VAR_RESULT, 1, Johto_CianwoodPharmacy_EventScript_83CB99D
	msgbox Johto_CianwoodPharmacy_Text_83CB9AD, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_SECRET_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, 0, Johto_CianwoodPharmacy_EventScript_83CB99A
	setvar VAR_QZ_0x500D, 4

Johto_CianwoodPharmacy_EventScript_83CB99A::
	closemessage
	release
	end

Johto_CianwoodPharmacy_EventScript_83CB99D::
	setvar VAR_QZ_0x500D, 4

Johto_CianwoodPharmacy_EventScript_83CB9A2::
	msgbox Johto_CianwoodPharmacy_Text_83CBA51, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodPharmacy_Text_83CB9AD::
	.string "Your POKéMON appear to be fine.\p"
	.string "Is something worrying you?\p"
	.string "…\p"
	.string "The LIGHTHOUSE POKéMON is in\n"
	.string "trouble?\p"
	.string "I got it!\p"
	.string "This ought to do the trick.$"

Johto_CianwoodPharmacy_Text_83CBA51::
	.string "My SECRET POTION is a tad too\n"
	.string "strong, so I only offer it in an\l"
	.string "emergency.$"
