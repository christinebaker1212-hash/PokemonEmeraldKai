Johto_ManiasHouse_MapScripts::
	.byte 0

@ Johto_ManiasHouse object 1 (local id 1, gfx 0x3EBF) at 4,4
Johto_ManiasHouse_EventScript_83CB203::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501D, 1, Johto_ManiasHouse_EventScript_83CB254
	goto_if_eq VAR_QZ_0x501D, 3, Johto_ManiasHouse_EventScript_83CB264
	call Quetzal_EventScript_8361A4D
	msgbox Johto_ManiasHouse_Text_83CB26F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_ManiasHouse_EventScript_83CB249
	setvar VAR_TEMP_1, 0xD5
	bufferspeciesname 0, SPECIES_SHUCKLE
	qz_givestorymon 2
	call Quetzal_EventScript_840181E
	goto Johto_ManiasHouse_EventScript_83CB254

Johto_ManiasHouse_EventScript_83CB249::
	msgbox Johto_ManiasHouse_Text_83CB3BE, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_EventScript_83CB254::
	msgbox Johto_ManiasHouse_Text_83CB472, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501D, 3
	release
	end

Johto_ManiasHouse_EventScript_83CB264::
	msgbox Johto_ManiasHouse_Text_83CB521, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ManiasHouse_Text_83CB26F::
	.string "I, I’m in shock!\p"
	.string "A guy about your age with piercing\n"
	.string "eyes and long hair came in.\p"
	.string "He scared me into giving him my\n"
	.string "prized POKéMON!\p"
	.string "I still have one left, but what if\n"
	.string "he comes back?\p"
	.string "You look strong. Would you like\n"
	.string "to take my POKéMON with you?$"

Johto_ManiasHouse_Text_83CB3BE::
	.string "Oh, no… What’ll I do if he comes\n"
	.string "back?$"

Johto_ManiasHouse_Text_83CB472::
	.string "I want my POKéMON to travel with you.\n"
	.string "You can keep it for good!\p"
	.string "Just promise to take good care of it.$"

Johto_ManiasHouse_Text_83CB521::
	.string "For POKéMON, happiness is being\n"
	.string "with a person who treats them well.$"
