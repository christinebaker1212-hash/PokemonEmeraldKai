Johto_IlexForestAzaleaGate_MapScripts::
	.byte 0

@ Johto_IlexForestAzaleaGate object 1 (local id 1, gfx 0x3EA9) at 6,2
Johto_IlexForestAzaleaGate_EventScript_83D0CA4::
	lock
	faceplayer
	msgbox Johto_IlexForestAzaleaGate_Text_83D0CBE, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_IlexForestAzaleaGate object 2 (local id 2, gfx 0x1E) at 4,6
Johto_IlexForestAzaleaGate_EventScript_83D0CB1::
	lock
	faceplayer
	msgbox Johto_IlexForestAzaleaGate_Text_83D0CEE, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_IlexForestAzaleaGate_Text_83D0CBE::
	.string "ILEX FOREST is big.\n"
	.string "Be careful!\n"
	.string "Don’t get lost.$"

Johto_IlexForestAzaleaGate_Text_83D0CEE::
	.string "The ILEX FOREST is watched\n"
	.string "over by its protector.\p"
	.string "Stay out of mischief!$"
