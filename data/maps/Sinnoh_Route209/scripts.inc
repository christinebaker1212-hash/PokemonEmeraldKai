Sinnoh_Route209_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route209_EventScript_Bg3::
	goto Sinnoh_EventScript_HallowedTower
	end

Sinnoh_Route209_EventScript_Bg4::
	goto Sinnoh_EventScript_HallowedTower
	end

Sinnoh_Route209_EventScript_Bg5::
	goto Sinnoh_EventScript_HallowedTower
	end

Sinnoh_Route209_EventScript_Bg6::
	goto Sinnoh_EventScript_HallowedTower
	end

Sinnoh_Route209_EventScript_Person22::
	msgbox Sinnoh_Route209_Text_Person22, MSGBOX_NPC
	end

Sinnoh_Route209_EventScript_Person29::
	msgbox Sinnoh_Route209_Text_Person29, MSGBOX_NPC
	end

Sinnoh_Route209_EventScript_Sign10::
	msgbox Sinnoh_Route209_Text_Sign10, MSGBOX_SIGN
	end

Sinnoh_Route209_EventScript_Sign11::
	msgbox Sinnoh_Route209_Text_Sign11, MSGBOX_SIGN
	end

Sinnoh_Route209_EventScript_Sign32::
	msgbox Sinnoh_Route209_Text_Sign32, MSGBOX_SIGN
	end

Sinnoh_Route209_Text_Person22:
	.string "There's an old tower on this route.\n"
	.string "People come here to remember MOéMON\l"
	.string "that have passed on.$"

Sinnoh_Route209_Text_Person29:
	.string "I fish here every morning. The mist on\n"
	.string "the water makes everything quiet.$"

Sinnoh_Route209_Text_Sign10:
	.string "ROUTE 209\p"
	.string "To HEARTHOME CITY$"

Sinnoh_Route209_Text_Sign11:
	.string "ROUTE 209\p"
	.string "To SOLACEON TOWN$"

Sinnoh_Route209_Text_Sign32:
	.string "TRAINER TIPS\p"
	.string "Talk to your MOéMON often. A happy\n"
	.string "MOéMON battles better.$"

@ --- end of SINNOH words ---
