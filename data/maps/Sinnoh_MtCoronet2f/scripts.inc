Sinnoh_MtCoronet2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_MtCoronet2f_EventScript_Person6::
	lock
	faceplayer
	goto_if_set 0x3DBB, Sinnoh_MtCoronet2f_EventScript_Person6_Given
	msgbox Sinnoh_MtCoronet2f_Text_Person6, MSGBOX_DEFAULT
	msgbox Sinnoh_MtCoronet2f_Text_Person6Gift, MSGBOX_DEFAULT
	giveitem ITEM_BLACK_FLUTE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DBB
	release
	end

Sinnoh_MtCoronet2f_EventScript_Person6_Given::
	msgbox Sinnoh_MtCoronet2f_Text_Person6, MSGBOX_DEFAULT
	release
	end

Sinnoh_MtCoronet2f_Text_Person6:
	.string "LOOKER: The mountain is full of TEAM\n"
	.string "GALACTIC. I will hold them off here.\p"
	.string "Keep climbing! The fate of SINNOH is\n"
	.string "at the top!$"

Sinnoh_MtCoronet2f_Text_Person6Gift:
	.string "Take this BLACK FLUTE. I found it on\n"
	.string "the way up. A detective notices these\l"
	.string "things.$"

@ --- end of SINNOH words ---
