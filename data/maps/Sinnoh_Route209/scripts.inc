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
	lock
	faceplayer
	goto_if_set 0x3DCA, Sinnoh_Route209_EventScript_Person29_Given
	msgbox Sinnoh_Route209_Text_Person29, MSGBOX_DEFAULT
	msgbox Sinnoh_Route209_Text_Person29Gift, MSGBOX_DEFAULT
	giveitem ITEM_GOOD_ROD, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DCA
	release
	end

Sinnoh_Route209_EventScript_Person29_Given::
	msgbox Sinnoh_Route209_Text_Person29, MSGBOX_DEFAULT
	release
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

Sinnoh_Route209_Text_Person29Gift:
	.string "You don't have a good rod? Take my old\n"
	.string "one. Patience does the rest!$"

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
