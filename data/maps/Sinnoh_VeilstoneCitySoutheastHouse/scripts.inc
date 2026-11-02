Sinnoh_VeilstoneCitySoutheastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCitySoutheastHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DE2, Sinnoh_VeilstoneCitySoutheastHouse_EventScript_Person1_Given
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, TRUE, Sinnoh_VeilstoneCitySoutheastHouse_EventScript_Person1_Given
	msgbox Sinnoh_VeilstoneCitySoutheastHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_VeilstoneCitySoutheastHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DE2
	release
	end

Sinnoh_VeilstoneCitySoutheastHouse_EventScript_Person1_Given::
	msgbox Sinnoh_VeilstoneCitySoutheastHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_VeilstoneCitySoutheastHouse_EventScript_Person2::
	goto SinnohP_veilstone_city_southeast_house_2
	end

Sinnoh_VeilstoneCitySoutheastHouse_Text_Person1:
	.string "I juggle POKé BALLS for fun! ...Empty\n"
	.string "ones. Mostly.$"

Sinnoh_VeilstoneCitySoutheastHouse_Text_Person1Gift:
	.string "You'll need somewhere to keep your\n"
	.string "COINS for the GAME CORNER. Take this!$"

@ --- end of SINNOH words ---
