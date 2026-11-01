Sinnoh_VeilstoneCityPrizeExchange_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCityPrizeExchange_EventScript_Person1::
	goto Sinnoh_EventScript_PrizeExchange
	end

Sinnoh_VeilstoneCityPrizeExchange_EventScript_Person2::
	msgbox Sinnoh_VeilstoneCityPrizeExchange_Text_Person2, MSGBOX_NPC
	end

Sinnoh_VeilstoneCityPrizeExchange_EventScript_Person3::
	msgbox Sinnoh_VeilstoneCityPrizeExchange_Text_Person3, MSGBOX_NPC
	end

Sinnoh_VeilstoneCityPrizeExchange_Text_Person2:
	.string "I won a prize once. It was a POTION.\n"
	.string "I'm still proud of it.$"

Sinnoh_VeilstoneCityPrizeExchange_Text_Person3:
	.string "In my day, prizes were fancier. But I\n"
	.string "still come to look.$"

@ --- end of SINNOH words ---
