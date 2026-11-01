Sinnoh_CanalaveCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCityMart_EventScript_Person1::
	msgbox Sinnoh_CanalaveCityMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_CanalaveCityMart_EventScript_Person2::
	msgbox Sinnoh_CanalaveCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_CanalaveCityMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCityMart_EventScript_Person5::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCityMart_Text_Person1:
	.string "Sailors buy rope here, I buy POTIONS.\n"
	.string "We both leave happy.$"

Sinnoh_CanalaveCityMart_Text_Person2:
	.string "The prices are a bit higher in\n"
	.string "CANALAVE. Everything comes by boat,\l"
	.string "you see.$"

@ --- end of SINNOH words ---
