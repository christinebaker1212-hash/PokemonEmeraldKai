Sinnoh_Route217WestHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route217WestHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DD1, Sinnoh_Route217WestHouse_EventScript_Person1_Given
	msgbox Sinnoh_Route217WestHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_Route217WestHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_ICICLE_PLATE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD1
	release
	end

Sinnoh_Route217WestHouse_EventScript_Person1_Given::
	msgbox Sinnoh_Route217WestHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route217WestHouse_Text_Person1:
	.string "I got lost in the snow and found this\n"
	.string "house. Best mistake I ever made.$"

Sinnoh_Route217WestHouse_Text_Person1Gift:
	.string "I found this plate frozen in the snow.\n"
	.string "Maybe it's yours now.$"

@ --- end of SINNOH words ---
