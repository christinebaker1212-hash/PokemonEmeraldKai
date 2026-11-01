Sinnoh_Route217NortheastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route217NortheastHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DD0, Sinnoh_Route217NortheastHouse_EventScript_Person1_Given
	msgbox Sinnoh_Route217NortheastHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_Route217NortheastHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_SPELL_TAG, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD0
	release
	end

Sinnoh_Route217NortheastHouse_EventScript_Person1_Given::
	msgbox Sinnoh_Route217NortheastHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route217NortheastHouse_Text_Person1:
	.string "We've got hot soup here for anyone\n"
	.string "braving the snow. You look like you\l"
	.string "need some!$"

Sinnoh_Route217NortheastHouse_Text_Person1Gift:
	.string "And take this charm too. It keeps the\n"
	.string "cold spirits away. Mostly.$"

@ --- end of SINNOH words ---
