Sinnoh_Route225House_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route225House_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DD6, Sinnoh_Route225House_EventScript_Person1_Given
	msgbox Sinnoh_Route225House_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_Route225House_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_FRESH_WATER, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD6
	release
	end

Sinnoh_Route225House_EventScript_Person1_Given::
	msgbox Sinnoh_Route225House_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route225House_Text_Person1:
	.string "I'm building a road through here. It's\n"
	.string "slow going with all these TRAINERS\l"
	.string "around!$"

Sinnoh_Route225House_Text_Person1Gift:
	.string "You look thirsty, climbing all the way\n"
	.string "up here. Have some water!$"

@ --- end of SINNOH words ---
