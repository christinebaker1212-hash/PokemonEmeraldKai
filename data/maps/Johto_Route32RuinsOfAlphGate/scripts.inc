Johto_Route32RuinsOfAlphGate_MapScripts::
	.byte 0

@ Johto_Route32RuinsOfAlphGate object 1 (local id 1, gfx 0x3E87) at 7,6
Johto_Route32RuinsOfAlphGate_EventScript_83D2214::
	lock
	faceplayer
	msgbox Johto_Route32RuinsOfAlphGate_Text_83D225D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route32RuinsOfAlphGate_Text_83D225D::
	.string "There are drawings on stone\n"
	.string "panels.\p"
	.string "I tried moving them. I wonder what\n"
	.string "they are.$"

@ Moemon Kai: Crystal's officer and POKéFAN (Route32RuinsOfAlphGate.asm, docs/REGIONS_AUDIT.md 138)
Johto_Route32RuinsOfAlphGate_EventScript_Officer::
	msgbox Johto_Route32RuinsOfAlphGate_Text_Officer, MSGBOX_NPC
	end

Johto_Route32RuinsOfAlphGate_EventScript_PokefanM::
	msgbox Johto_Route32RuinsOfAlphGate_Text_PokefanM, MSGBOX_NPC
	end

Johto_Route32RuinsOfAlphGate_Text_Officer::
	.string "RUINS OF ALPH\p"
	.string "A Look-and-Touch Tourist Site Try\n"
	.string "the sliding stone panels!$"

Johto_Route32RuinsOfAlphGate_Text_PokefanM::
	.string "You’re studying the RUINS?\p"
	.string "I see a scientist in the making.$"
