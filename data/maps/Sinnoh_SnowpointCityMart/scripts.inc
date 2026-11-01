Sinnoh_SnowpointCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SnowpointCityMart_EventScript_Person1::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SnowpointCityMart_EventScript_Person2::
	msgbox Sinnoh_SnowpointCityMart_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SnowpointCityMart_EventScript_Person3::
	msgbox Sinnoh_SnowpointCityMart_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SnowpointCityMart_EventScript_Person5::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SnowpointCityMart_Text_Person2:
	.string "I'm buying BURN HEALS. Not for burns.\n"
	.string "They're just warm to hold.$"

Sinnoh_SnowpointCityMart_Text_Person3:
	.string "Bundle up and stock up! That's\n"
	.string "SNOWPOINT's motto.$"

@ --- end of SINNOH words ---
