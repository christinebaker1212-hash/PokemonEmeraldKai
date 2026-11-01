Sinnoh_OreburghCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityMart_EventScript_Person1::
	msgbox Sinnoh_OreburghCityMart_Text_Person1, MSGBOX_NPC
	end

Sinnoh_OreburghCityMart_EventScript_Person2::
	msgbox Sinnoh_OreburghCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_OreburghCityMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCityMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Oreburgh
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCityMart_Text_Person1:
	.string "Miners buy lots of ESCAPE ROPES.\n"
	.string "Nobody wants to get lost underground.$"

Sinnoh_OreburghCityMart_Text_Person2:
	.string "I'm buying REPELS. I want to collect\n"
	.string "fossils in the mine, not battle ZUBAT!$"

@ --- end of SINNOH words ---
