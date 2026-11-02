Sinnoh_FloaromaTownMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaTownMart_EventScript_Person1::
	goto SinnohP_floaroma_town_mart_3
	end

Sinnoh_FloaromaTownMart_EventScript_Person2::
	goto SinnohP_floaroma_town_mart_4
	end

Sinnoh_FloaromaTownMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_FloaromaTownMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Floaroma
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

@ --- end of SINNOH words ---
