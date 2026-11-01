Sinnoh_Route222EastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route222EastHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DD5, Sinnoh_Route222EastHouse_EventScript_Person1_Given
	msgbox Sinnoh_Route222EastHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_Route222EastHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_NET_BALL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD5
	release
	end

Sinnoh_Route222EastHouse_EventScript_Person1_Given::
	msgbox Sinnoh_Route222EastHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route222EastHouse_Text_Person1:
	.string "I've caught every kind of fish on this\n"
	.string "route. Well, almost. There's always one\l"
	.string "more.$"

Sinnoh_Route222EastHouse_Text_Person1Gift:
	.string "This BALL is made for MOéMON of the\n"
	.string "water. Take it!$"

@ --- end of SINNOH words ---
