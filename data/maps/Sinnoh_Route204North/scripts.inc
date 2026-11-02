Sinnoh_Route204North_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route204North_EventScript_Person5::
	goto SinnohP_route_204_north_1
	end

Sinnoh_Route204North_EventScript_Person11::
	lock
	faceplayer
	goto_if_set 0x3DC7, Sinnoh_Route204North_EventScript_Person11_Given
	msgbox Sinnoh_Route204North_Text_Person11, MSGBOX_DEFAULT
	msgbox Sinnoh_Route204North_Text_Person11Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM78, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC7
	release
	end

Sinnoh_Route204North_EventScript_Person11_Given::
	msgbox Sinnoh_Route204North_Text_Person11, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route204North_EventScript_Sign6::
	goto SinnohP_route_204_north_3
	end

Sinnoh_Route204North_Text_Person11:
	.string "I love this road. The breeze from the\n"
	.string "meadow is always sweet.$"

Sinnoh_Route204North_Text_Person11Gift:
	.string "This TM blows like that breeze. Take\n"
	.string "it!$"

@ --- end of SINNOH words ---
