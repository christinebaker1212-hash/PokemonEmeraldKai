Johto_OlivinePort_MapScripts::
	.byte 0

@ Johto_OlivinePort object 3 (local id 3, gfx 0x3E8F) at 12,12
Johto_OlivinePort_EventScript_83E6BCF::
	msgbox Johto_OlivinePort_Text_83E6E9A, MSGBOX_NPC
	end

@ Johto_OlivinePort object 5 (local id 5, gfx 0x3E95) at 14,12
Johto_OlivinePort_EventScript_83E6BD8::
	msgbox Johto_OlivinePort_Text_83E6DEB, MSGBOX_NPC
	end

@ Johto_OlivinePort object 4 (local id 4, gfx 0x3E95) at 3,14
Johto_OlivinePort_EventScript_83E6BE1::
	msgbox Johto_OlivinePort_Text_83E6E2C, MSGBOX_NPC
	end

@ Johto_OlivinePort object 2 (local id 2, gfx 0x23) at 5,12
Johto_OlivinePort_EventScript_83E6BEA::
	msgbox Johto_OlivinePort_Text_83E6E6F, MSGBOX_NPC
	end

@ Johto_OlivinePort object 1 (local id 1, gfx 0x31) at 8,17
Johto_OlivinePort_EventScript_83E6BF3::
	lock
	faceplayer
	call Quetzal_EventScript_840F954
	goto_if_eq VAR_RESULT, 127, Johto_OlivinePort_EventScript_83E6C0A
	goto Quetzal_EventScript_840FA9D

Johto_OlivinePort_EventScript_83E6C0A::
	msgbox Johto_OlivinePort_Text_83E6D3A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivinePort_Text_83E6D3A::
	.string "We hope to see you again!$"

Johto_OlivinePort_Text_83E6DEB::
	.string "SHELLDER are easy to catch here.\p"
	.string "They’re kind of rare elsewhere.$"

Johto_OlivinePort_Text_83E6E2C::
	.string "How many RODS do you have?\p"
	.string "Different RODS catch different\n"
	.string "POKéMON.$"

Johto_OlivinePort_Text_83E6E6F::
	.string "S.S.AQUA uses jets to skim\n"
	.string "over the waves!$"

Johto_OlivinePort_Text_83E6E9A::
	.string "There are lots of POKéMON in KANTO.\p"
	.string "I wish I could go…$"
