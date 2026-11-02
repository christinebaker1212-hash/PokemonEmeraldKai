Sinnoh_EternaCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityMart_EventScript_Person1::
	msgbox Sinnoh_EternaCityMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_EternaCityMart_EventScript_Person2::
	goto SinnohP_eterna_city_mart_4
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
	pokemart Sinnoh_Mart_EternaMart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCityMart_Text_Person1:
	.string "I write songs about the forest. The\n"
	.string "MART sells everything except\l"
	.string "inspiration.$"

@ --- end of SINNOH words ---
