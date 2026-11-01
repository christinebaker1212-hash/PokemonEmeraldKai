Sinnoh_GrandLakeRoute213NorthwestHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_GrandLakeRoute213NorthwestHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DB2, Sinnoh_GrandLakeRoute213NorthwestHouse_EventScript_Person1_Given
	msgbox Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM92, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DB2
	release
	end

Sinnoh_GrandLakeRoute213NorthwestHouse_EventScript_Person1_Given::
	msgbox Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_GrandLakeRoute213NorthwestHouse_EventScript_Person2::
	msgbox Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_GrandLakeRoute213NorthwestHouse_EventScript_Person3::
	msgbox Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person1:
	.string "I'm on vacation! No juggling for a\n"
	.string "whole week!$"

Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person1Gift:
	.string "Since I'm not working, you can have my\n"
	.string "act. It's on this TM!$"

Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person2:
	.string "The sea air clears my mind. I can\n"
	.string "almost hear my MOéMON's thoughts.$"

Sinnoh_GrandLakeRoute213NorthwestHouse_Text_Person3:
	.string "I play on the beach every night. The\n"
	.string "waves keep the rhythm.$"

@ --- end of SINNOH words ---
