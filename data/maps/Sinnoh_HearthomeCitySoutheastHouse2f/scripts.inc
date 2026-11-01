Sinnoh_HearthomeCitySoutheastHouse2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_HearthomeCitySoutheastHouse2f_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DB7, Sinnoh_HearthomeCitySoutheastHouse2f_EventScript_Person1_Given
	msgbox Sinnoh_HearthomeCitySoutheastHouse2f_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_HearthomeCitySoutheastHouse2f_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_SHELL_BELL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DB7
	release
	end

Sinnoh_HearthomeCitySoutheastHouse2f_EventScript_Person1_Given::
	msgbox Sinnoh_HearthomeCitySoutheastHouse2f_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_HearthomeCitySoutheastHouse2f_EventScript_Person2::
	msgbox Sinnoh_HearthomeCitySoutheastHouse2f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_HearthomeCitySoutheastHouse2f_Text_Person1:
	.string "My CLEFAIRY dances every night under\n"
	.string "the moonlight. It's magical!$"

Sinnoh_HearthomeCitySoutheastHouse2f_Text_Person1Gift:
	.string "A bell that rings like the sea.\n"
	.string "CLEFAIRY doesn't need it. Here!$"

Sinnoh_HearthomeCitySoutheastHouse2f_Text_Person2:
	.string "CLEFAIRY: Pi pi pippi!\p"
	.string "It's twirling around happily.$"

@ --- end of SINNOH words ---
