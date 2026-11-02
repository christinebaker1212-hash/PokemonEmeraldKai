Sinnoh_SunyshoreMarket_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SunyshoreMarket_EventScript_Person1::
	msgbox Sinnoh_SunyshoreMarket_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SunyshoreMarket_EventScript_Person2::
	goto SinnohP_sunyshore_market_4
	end

Sinnoh_SunyshoreMarket_EventScript_Person3::
	goto SinnohP_sunyshore_market_2
	end

Sinnoh_SunyshoreMarket_EventScript_Person4::
	goto SinnohP_sunyshore_market_3
	end

Sinnoh_SunyshoreMarket_EventScript_Person5::
	goto SinnohP_sunyshore_market_6
	end

Sinnoh_SunyshoreMarket_Text_Person1:
	.string "The market's stalls change every day.\n"
	.string "Come back tomorrow for something new!$"

@ --- end of SINNOH words ---
