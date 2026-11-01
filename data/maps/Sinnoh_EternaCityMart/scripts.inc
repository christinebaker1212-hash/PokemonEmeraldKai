Sinnoh_EternaCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityMart_EventScript_Person1::
	msgbox Sinnoh_EternaCityMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_EternaCityMart_EventScript_Person2::
	msgbox Sinnoh_EternaCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_EternaCityMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCityMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCityMart_Text_Person1:
	.string "I write songs about the forest. The\n"
	.string "MART sells everything except\l"
	.string "inspiration.$"

Sinnoh_EternaCityMart_Text_Person2:
	.string "Stock up on ANTIDOTES before ETERNA\n"
	.string "FOREST. The BUG MOéMON there love\l"
	.string "POISON.$"

@ --- end of SINNOH words ---
