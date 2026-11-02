Sinnoh_Route221House_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route221House_EventScript_Bg1::
	goto SinnohP_route_221_house_2
	end

Sinnoh_Route221House_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DDA, Sinnoh_Route221House_EventScript_Person1_Given
	msgbox Sinnoh_Route221House_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_Route221House_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_BLACK_BELT, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DDA
	release
	end

Sinnoh_Route221House_EventScript_Person1_Given::
	msgbox Sinnoh_Route221House_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route221House_Text_Person1:
	.string "The park up the road is still being\n"
	.string "built. They say MOéMON from far away\l"
	.string "will live there.$"

Sinnoh_Route221House_Text_Person1Gift:
	.string "I was going to give this belt to the\n"
	.string "park's workers. You take it!$"

@ --- end of SINNOH words ---
