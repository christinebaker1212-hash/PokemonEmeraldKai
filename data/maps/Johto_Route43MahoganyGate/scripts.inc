Johto_Route43MahoganyGate_MapScripts::
	.byte 0

@ Johto_Route43MahoganyGate object 1 (local id 1, gfx 0x3EA9) at 3,6
Johto_Route43MahoganyGate_EventScript_83D1354::
	goto_if_ge VAR_QZ_0x500A, 11, Johto_Route43MahoganyGate_EventScript_83D1368
	msgbox Johto_Route43MahoganyGate_Text_83D1371, MSGBOX_NPC
	end

Johto_Route43MahoganyGate_EventScript_83D1368::
	msgbox Johto_Route43MahoganyGate_Text_83D13B6, MSGBOX_NPC
	end

Johto_Route43MahoganyGate_Text_83D1371::
	.string "Only people headed up to LAKE OF\n"
	.string "RAGE have been through here lately.$"

Johto_Route43MahoganyGate_Text_83D13B6::
	.string "Nobody goes up to LAKE OF RAGE\n"
	.string "these days.$"
