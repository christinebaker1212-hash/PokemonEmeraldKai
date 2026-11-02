Sinnoh_CelesticTownNorthwestHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CelesticTownNorthwestHouse_EventScript_Person1::
	msgbox Sinnoh_CelesticTownNorthwestHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_CelesticTownNorthwestHouse_EventScript_Person2::
	msgbox Sinnoh_CelesticTownNorthwestHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_CelesticTownNorthwestHouse_EventScript_Person3::
	goto SinnohP_celestic_town_northwest_house_4
	end

Sinnoh_CelesticTownNorthwestHouse_Text_Person1:
	.string "We've lived in CELESTIC for\n"
	.string "generations. The mountain protects\l"
	.string "us.$"

Sinnoh_CelesticTownNorthwestHouse_Text_Person2:
	.string "The cave behind town has a shrine. We\n"
	.string "visit it every year on the first snow.$"

@ --- end of SINNOH words ---
