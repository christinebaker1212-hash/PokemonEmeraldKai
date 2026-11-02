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
	goto SinnohP_route_209_1
	end

Sinnoh_Route209_EventScript_Person29::
	lock
	faceplayer
	goto_if_set 0x3DCA, Sinnoh_Route209_EventScript_Person29_Given
	checkitem ITEM_GOOD_ROD
	goto_if_eq VAR_RESULT, TRUE, Sinnoh_Route209_EventScript_Person29_Given
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

Sinnoh_Route209_EventScript_Person30::
	msgbox Sinnoh_Route209_Text_Person30, MSGBOX_NPC
	end

Sinnoh_Route209_EventScript_Person31::
	msgbox Sinnoh_Route209_Text_Person31, MSGBOX_NPC
	end

Sinnoh_Route209_EventScript_Sign10::
	goto SinnohP_route_209_3
	end

Sinnoh_Route209_EventScript_Sign11::
	goto SinnohP_route_209_4
	end

Sinnoh_Route209_EventScript_Sign32::
	goto SinnohP_route_209_9
	end

Sinnoh_Route209_Text_Person29:
	.string "I fish here every morning. The mist on\n"
	.string "the water makes everything quiet.$"

Sinnoh_Route209_Text_Person29Gift:
	.string "You don't have a good rod? Take my old\n"
	.string "one. Patience does the rest!$"

Sinnoh_Route209_Text_Person30:
	.string "Night jogging is peaceful. Too dark to\n"
	.string "battle, though. Come back by day!$"

Sinnoh_Route209_Text_Person31:
	.string "Running at night keeps me cool. I'm\n"
	.string "too out of breath to battle anyway.$"

@ --- end of SINNOH words ---
