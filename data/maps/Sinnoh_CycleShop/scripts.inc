Sinnoh_CycleShop_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CycleShop_EventScript_Bg1::
	goto SinnohP_cycle_shop_4
	end

Sinnoh_CycleShop_EventScript_Bg2::
	goto SinnohP_cycle_shop_4
	end

Sinnoh_CycleShop_EventScript_Bg3::
	goto SinnohP_cycle_shop_5
	end

Sinnoh_CycleShop_EventScript_Bg4::
	goto SinnohP_cycle_shop_5
	end

Sinnoh_CycleShop_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DA6, Sinnoh_CycleShop_EventScript_Person1_Given
	checkitem ITEM_MTN_BIKE
	goto_if_eq VAR_RESULT, TRUE, Sinnoh_CycleShop_EventScript_Person1_Given
	msgbox Sinnoh_CycleShop_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_CycleShop_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_MTN_BIKE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA6
	release
	end

Sinnoh_CycleShop_EventScript_Person1_Given::
	msgbox Sinnoh_CycleShop_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_CycleShop_EventScript_Person2::
	goto SinnohP_cycle_shop_3
	end

Sinnoh_CycleShop_EventScript_Person3::
	msgbox Sinnoh_CycleShop_Text_Person3, MSGBOX_NPC
	end

Sinnoh_CycleShop_Text_Person1:
	.string "Thanks to you, my CLEFAIRY is home\n"
	.string "safe! Take a look around the shop.\l"
	.string "Everything rolls!$"

Sinnoh_CycleShop_Text_Person1Gift:
	.string "As thanks for saving my CLEFAIRY,\n"
	.string "take a BIKE! On the house!$"

Sinnoh_CycleShop_Text_Person3:
	.string "Clefairy twirls around happily.$"

@ --- end of SINNOH words ---
