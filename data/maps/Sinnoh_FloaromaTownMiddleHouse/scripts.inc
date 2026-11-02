Sinnoh_FloaromaTownMiddleHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaTownMiddleHouse_EventScript_Person1::
	goto SinnohP_floaroma_town_middle_house_1
	end

Sinnoh_FloaromaTownMiddleHouse_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DAE, Sinnoh_FloaromaTownMiddleHouse_EventScript_Person2_Given
	msgbox Sinnoh_FloaromaTownMiddleHouse_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_FloaromaTownMiddleHouse_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM88, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DAE
	release
	end

Sinnoh_FloaromaTownMiddleHouse_EventScript_Person2_Given::
	msgbox Sinnoh_FloaromaTownMiddleHouse_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_FloaromaTownMiddleHouse_EventScript_Person3::
	msgbox Sinnoh_FloaromaTownMiddleHouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_FloaromaTownMiddleHouse_Text_Person2:
	.string "I'm making a flower crown for\n"
	.string "CLEFAIRY!$"

Sinnoh_FloaromaTownMiddleHouse_Text_Person2Gift:
	.string "You can have this! It was in the\n"
	.string "flower basket. I don't know what it\l"
	.string "does.$"

Sinnoh_FloaromaTownMiddleHouse_Text_Person3:
	.string "CLEFAIRY: Pippi!\p"
	.string "It's wearing a tiny flower crown.$"

@ --- end of SINNOH words ---
