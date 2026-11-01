Sinnoh_IronIslandHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_IronIslandHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DB9, Sinnoh_IronIslandHouse_EventScript_Person1_Given
	msgbox Sinnoh_IronIslandHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_IronIslandHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_METAL_COAT, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DB9
	release
	end

Sinnoh_IronIslandHouse_EventScript_Person1_Given::
	msgbox Sinnoh_IronIslandHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_IronIslandHouse_Text_Person1:
	.string "BYRON: Gahaha! My son runs the\n"
	.string "CANALAVE GYM now, so I can dig all day\l"
	.string "long!\p"
	.string "This island has iron in its bones. Like\n"
	.string "me!$"

Sinnoh_IronIslandHouse_Text_Person1Gift:
	.string "Gahaha! A METAL COAT for you, dug from\n"
	.string "this very island!$"

@ --- end of SINNOH words ---
