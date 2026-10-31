Sinnoh_SandgemTownMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SandgemTownMart_EventScript_Person1::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SandgemTownMart_EventScript_Person2::
	msgbox Sinnoh_SandgemTownMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SandgemTownMart_EventScript_Person3::
	msgbox Sinnoh_SandgemTownMart_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SandgemTownMart_Text_Person2:
	.string "I spent all my allowance on POKé\n"
	.string "BALLS.\p"
	.string "Now I just need to get good at\n"
	.string "throwing them.$"

Sinnoh_SandgemTownMart_Text_Person3:
	.string "A POTION for the trip, an ANTIDOTE for\n"
	.string "the grass... and a snack for me.\p"
	.string "Shopping for a journey is the best\n"
	.string "part!$"

@ --- end of SINNOH words ---
