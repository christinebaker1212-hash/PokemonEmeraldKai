Sinnoh_FlowerShop_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FlowerShop_EventScript_Person1::
	msgbox Sinnoh_FlowerShop_Text_Person1, MSGBOX_NPC
	end

Sinnoh_FlowerShop_EventScript_Person2::
	goto SinnohP_flower_shop_2
	end

Sinnoh_FlowerShop_EventScript_Person3::
	goto SinnohP_flower_shop_1
	end

Sinnoh_FlowerShop_Text_Person1:
	.string "Welcome! Our flowers bring a smile to\n"
	.string "every face.$"

@ --- end of SINNOH words ---
