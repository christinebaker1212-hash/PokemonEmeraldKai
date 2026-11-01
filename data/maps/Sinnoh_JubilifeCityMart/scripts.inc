Sinnoh_JubilifeCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_JubilifeCityMart_EventScript_Person1::
	msgbox Sinnoh_JubilifeCityMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_JubilifeCityMart_EventScript_Person2::
	msgbox Sinnoh_JubilifeCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_JubilifeCityMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_JubilifeCityMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_JubilifeCityMart_EventScript_Person6::
	msgbox Sinnoh_JubilifeCityMart_Text_Person6, MSGBOX_NPC
	end

Sinnoh_JubilifeCityMart_Text_Person1:
	.string "I buy new guitar strings here. The\n"
	.string "clerk always asks if I want a POTION\l"
	.string "too. I always say yes.$"

Sinnoh_JubilifeCityMart_Text_Person2:
	.string "JUBILIFE's MART is the busiest in\n"
	.string "SINNOH. Come early or wait in line!$"

Sinnoh_JubilifeCityMart_Text_Person6:
	.string "I'm stocking up before I head west.\n"
	.string "You never know what the road will\l"
	.string "throw at you.$"

@ --- end of SINNOH words ---
