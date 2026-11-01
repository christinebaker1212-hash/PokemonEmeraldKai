Sinnoh_CycleShop_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CycleShop_EventScript_Bg1::
	msgbox Sinnoh_CycleShop_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_CycleShop_EventScript_Bg2::
	msgbox Sinnoh_CycleShop_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_CycleShop_EventScript_Bg3::
	msgbox Sinnoh_CycleShop_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_CycleShop_EventScript_Bg4::
	msgbox Sinnoh_CycleShop_Text_Bg4, MSGBOX_SIGN
	end

Sinnoh_CycleShop_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DA6, Sinnoh_CycleShop_EventScript_Person1_Given
	checkitem ITEM_BICYCLE
	goto_if_eq VAR_RESULT, TRUE, Sinnoh_CycleShop_EventScript_Person1_Given
	msgbox Sinnoh_CycleShop_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_CycleShop_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA6
	release
	end

Sinnoh_CycleShop_EventScript_Person1_Given::
	msgbox Sinnoh_CycleShop_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_CycleShop_EventScript_Person2::
	msgbox Sinnoh_CycleShop_Text_Person2, MSGBOX_NPC
	end

Sinnoh_CycleShop_EventScript_Person3::
	msgbox Sinnoh_CycleShop_Text_Person3, MSGBOX_NPC
	end

Sinnoh_CycleShop_Text_Bg1:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

Sinnoh_CycleShop_Text_Bg2:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

Sinnoh_CycleShop_Text_Bg3:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

Sinnoh_CycleShop_Text_Bg4:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

Sinnoh_CycleShop_Text_Person1:
	.string "Thanks to you, my CLEFAIRY is home\n"
	.string "safe! Take a look around the shop.\l"
	.string "Everything rolls!$"

Sinnoh_CycleShop_Text_Person1Gift:
	.string "As thanks for saving my CLEFAIRY,\n"
	.string "take a BIKE! On the house!$"

Sinnoh_CycleShop_Text_Person2:
	.string "I want a bike so badly! I've been\n"
	.string "saving up for... a while. A long while.$"

Sinnoh_CycleShop_Text_Person3:
	.string "Clefairy twirls around happily.$"

@ --- end of SINNOH words ---
