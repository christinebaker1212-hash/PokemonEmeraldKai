Sinnoh_SolaceonTownMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SolaceonTownMart_EventScript_Person1::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SolaceonTownMart_EventScript_Person2::
	msgbox Sinnoh_SolaceonTownMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SolaceonTownMart_EventScript_Person3::
	msgbox Sinnoh_SolaceonTownMart_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SolaceonTownMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SolaceonTownMart_Text_Person2:
	.string "I'm buying POKé BALLS to catch\n"
	.string "something at the ruins!$"

Sinnoh_SolaceonTownMart_Text_Person3:
	.string "The DAY CARE couple buys their\n"
	.string "supplies here. They always buy a lot!$"

@ --- end of SINNOH words ---
