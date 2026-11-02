Sinnoh_PoketchCo1f_MapScripts::
	.byte 0

@ The president gives the POKéTCH (src/sinnoh/poketch.c); its apps come with SINNOH's badges
Sinnoh_PoketchCo1f_EventScript_Person4::
	lock
	faceplayer
	checkitem ITEM_POKETCH
	goto_if_eq VAR_RESULT, TRUE, Sinnoh_PoketchCo1f_EventScript_PresidentApps
	msgbox Sinnoh_PoketchCo1f_Text_PresidentGift, MSGBOX_DEFAULT
	giveitem ITEM_POKETCH
	msgbox Sinnoh_PoketchCo1f_Text_PresidentHowTo, MSGBOX_DEFAULT
	release
	end

Sinnoh_PoketchCo1f_EventScript_PresidentApps::
	msgbox Sinnoh_PoketchCo1f_Text_PresidentApps, MSGBOX_DEFAULT
	release
	end

Sinnoh_PoketchCo1f_Text_PresidentGift:
	.string "I'm the president of the POKéTCH\n"
	.string "COMPANY! You look like a TRAINER\p"
	.string "who's always on the move. Our new\n"
	.string "watch will keep up with you.\p"
	.string "Here, take one!$"

Sinnoh_PoketchCo1f_Text_PresidentHowTo:
	.string "Use it from your BAG, or register\n"
	.string "it to a button. L and R switch\l"
	.string "apps, and you can tap the screen.\p"
	.string "It works anywhere you travel!$"

Sinnoh_PoketchCo1f_Text_PresidentApps:
	.string "Every BADGE you earn in SINNOH\n"
	.string "unlocks more POKéTCH apps.\p"
	.string "Our engineers never sleep!$"

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PoketchCo1f_EventScript_Person1::
	goto SinnohP_poketch_co_1f_2
	end

Sinnoh_PoketchCo1f_EventScript_Person2::
	msgbox Sinnoh_PoketchCo1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PoketchCo1f_EventScript_Person3::
	goto SinnohP_poketch_co_1f_3
	end

Sinnoh_PoketchCo1f_EventScript_Person5::
	goto SinnohP_poketch_co_1f_4
	end

Sinnoh_PoketchCo1f_Text_Person2:
	.string "I'm here to interview the PRESIDENT\n"
	.string "about the newest PÓKETCH app. He\l"
	.string "keeps showing me the clock.$"

@ --- end of SINNOH words ---
