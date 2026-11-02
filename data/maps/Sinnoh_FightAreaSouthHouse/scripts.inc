Sinnoh_FightAreaSouthHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FightAreaSouthHouse_EventScript_Person1::
	goto SinnohP_fight_area_south_house_1
	end

Sinnoh_FightAreaSouthHouse_EventScript_Person2::
	goto SinnohP_fight_area_south_house_3
	end

Sinnoh_FightAreaSouthHouse_EventScript_Person3::
	goto SinnohP_fight_area_south_house_2
	end

Sinnoh_FightAreaSouthHouse_EventScript_Person4::
	msgbox Sinnoh_FightAreaSouthHouse_Text_Person4, MSGBOX_NPC
	end

Sinnoh_FightAreaSouthHouse_Text_Person4:
	.string "Items can turn a battle around. A good\n"
	.string "held item is half the battle!$"

@ --- end of SINNOH words ---
