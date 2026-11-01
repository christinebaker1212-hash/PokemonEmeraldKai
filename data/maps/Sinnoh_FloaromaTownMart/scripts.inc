Sinnoh_FloaromaTownMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaTownMart_EventScript_Person1::
	msgbox Sinnoh_FloaromaTownMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_FloaromaTownMart_EventScript_Person2::
	msgbox Sinnoh_FloaromaTownMart_Text_Person2, MSGBOX_NPC
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
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_FloaromaTownMart_Text_Person1:
	.string "I'm buying HONEY for the meadow trees.\n"
	.string "The MOéMON there love it!$"

Sinnoh_FloaromaTownMart_Text_Person2:
	.string "I'm picking up a gift for my friend's\n"
	.string "birthday. A pretty ribbon, I think.$"

@ --- end of SINNOH words ---
