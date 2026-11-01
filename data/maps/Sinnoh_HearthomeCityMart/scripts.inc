Sinnoh_HearthomeCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_HearthomeCityMart_EventScript_Person1::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_HearthomeCityMart_EventScript_Person2::
	msgbox Sinnoh_HearthomeCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_HearthomeCityMart_EventScript_Person3::
	msgbox Sinnoh_HearthomeCityMart_Text_Person3, MSGBOX_NPC
	end

Sinnoh_HearthomeCityMart_EventScript_Person4::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Hearthome
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_HearthomeCityMart_Text_Person2:
	.string "I came down from the mountains to\n"
	.string "shop. The city's prices make my head\l"
	.string "spin!$"

Sinnoh_HearthomeCityMart_Text_Person3:
	.string "I'm buying gifts for my MOéMON. It's\n"
	.string "our anniversary. One year together!$"

@ --- end of SINNOH words ---
