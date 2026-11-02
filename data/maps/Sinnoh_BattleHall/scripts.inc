Sinnoh_BattleHall_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_BattleHall_EventScript_Person1::
	goto Sinnoh_EventScript_HallCounter
	end

Sinnoh_BattleHall_EventScript_Person2::
	goto Sinnoh_EventScript_HallCounter
	end

Sinnoh_BattleHall_EventScript_Person3::
	goto SinnohP_battle_hall_14
	end

Sinnoh_BattleHall_EventScript_Person4::
	goto SinnohP_battle_hall_13
	end

Sinnoh_BattleHall_EventScript_Person5::
	goto SinnohP_battle_hall_9
	end

Sinnoh_BattleHall_EventScript_Person6::
	msgbox Sinnoh_BattleHall_Text_Person6, MSGBOX_NPC
	end

Sinnoh_BattleHall_EventScript_Person7::
	goto SinnohP_battle_hall_8
	end

Sinnoh_BattleHall_EventScript_Person8::
	goto SinnohP_battle_hall_7
	end

Sinnoh_BattleHall_EventScript_Person9::
	goto SinnohP_battle_hall_11
	end

Sinnoh_BattleHall_EventScript_Person10::
	goto SinnohP_battle_hall_12
	end

Sinnoh_BattleHall_EventScript_Person11::
	msgbox Sinnoh_BattleHall_Text_Person11, MSGBOX_NPC
	end

Sinnoh_BattleHall_EventScript_Person12::
	goto SinnohP_battle_hall_15
	end

Sinnoh_BattleHall_EventScript_Person15::
	msgbox Sinnoh_BattleHall_Text_Person15, MSGBOX_NPC
	end

Sinnoh_BattleHall_Text_Person6:
	.string "PACHIRISU: Chipa chipa!\p"
	.string "It looks ready for a challenge.$"

Sinnoh_BattleHall_Text_Person11:
	.string "The BATTLE HALL's crowd is the\n"
	.string "loudest in SINNOH!$"

Sinnoh_BattleHall_Text_Person15:
	.string "I keep the records of the BATTLE\n"
	.string "HALL. One MOéMON, every type, ten\l"
	.string "ranks each. Good luck!$"

@ --- end of SINNOH words ---
