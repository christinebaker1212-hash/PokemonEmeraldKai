Sinnoh_Route222_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route222_EventScript_Person16::
	goto SinnohP_route_222_1
	end

Sinnoh_Route222_EventScript_Person26::
	lock
	faceplayer
	goto_if_set 0x3DD4, Sinnoh_Route222_EventScript_Person26_Given
	msgbox Sinnoh_Route222_Text_Person26, MSGBOX_DEFAULT
	msgbox Sinnoh_Route222_Text_Person26Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM56, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD4
	release
	end

Sinnoh_Route222_EventScript_Person26_Given::
	msgbox Sinnoh_Route222_Text_Person26, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route222_EventScript_Person30::
	msgbox Sinnoh_Route222_Text_Person30, MSGBOX_NPC
	end

Sinnoh_Route222_EventScript_Sign12::
	goto SinnohP_route_222_4
	end

Sinnoh_Route222_EventScript_Sign13::
	goto SinnohP_route_222_3
	end

Sinnoh_Route222_EventScript_Sign14::
	goto SinnohP_route_222_5
	end

Sinnoh_Route222_EventScript_Sign15::
	goto SinnohP_route_222_6
	end

Sinnoh_Route222_Text_Person26:
	.string "I bought this whole beach. Just\n"
	.string "kidding! ...Or am I?$"

Sinnoh_Route222_Text_Person26Gift:
	.string "Since you're my guest on my beach,\n"
	.string "here's a TM. Money is no object!$"

Sinnoh_Route222_Text_Person30:
	.string "The beach is quiet by day. Night\n"
	.string "patrol's where the action is.$"

@ --- end of SINNOH words ---
