Sinnoh_CelesticTownNorthHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CelesticTownNorthHouse_EventScript_Bg1::
	goto SinnohP_celestic_town_north_house_4
	end

Sinnoh_CelesticTownNorthHouse_EventScript_Person1::
	goto SinnohP_celestic_town_north_house_1
	end

Sinnoh_CelesticTownNorthHouse_EventScript_Person2::
	msgbox Sinnoh_CelesticTownNorthHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_CelesticTownNorthHouse_EventScript_Person3::
	goto SinnohP_celestic_town_north_house_3
	end

Sinnoh_CelesticTownNorthHouse_EventScript_Sign4::
	goto SinnohP_celestic_town_north_house_6
	end

Sinnoh_CelesticTownNorthHouse_Text_Person2:
	.string "I'm the elder of CELESTIC. My\n"
	.string "granddaughter visits now and then.\l"
	.string "She's quite the TRAINER.$"

@ --- end of SINNOH words ---
