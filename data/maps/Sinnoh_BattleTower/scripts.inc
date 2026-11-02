Sinnoh_BattleTower_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_BattleTower_EventScript_Bg2::
	msgbox Sinnoh_BattleTower_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_BattleTower_EventScript_Bg3::
	msgbox Sinnoh_BattleTower_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_BattleTower_EventScript_Person1::
	goto SinnohP_battle_tower_1
	end

Sinnoh_BattleTower_EventScript_Person2::
	goto SinnohP_battle_tower_2
	end

Sinnoh_BattleTower_EventScript_Person5::
	goto SinnohP_battle_tower_19
	end

Sinnoh_BattleTower_EventScript_Person6::
	goto SinnohP_battle_tower_15
	end

Sinnoh_BattleTower_EventScript_Person7::
	goto SinnohP_battle_tower_16
	end

Sinnoh_BattleTower_EventScript_Person8::
	goto SinnohP_battle_tower_17
	end

Sinnoh_BattleTower_EventScript_Person9::
	goto SinnohP_battle_tower_18
	end

Sinnoh_BattleTower_EventScript_Person10::
	goto SinnohP_battle_tower_21
	end

Sinnoh_BattleTower_EventScript_Person11::
	goto SinnohP_battle_tower_20
	end

Sinnoh_BattleTower_EventScript_Person12::
	goto SinnohP_battle_tower_14
	end

Sinnoh_BattleTower_EventScript_Person13::
	msgbox Sinnoh_BattleTower_Text_Person13, MSGBOX_NPC
	end

Sinnoh_BattleTower_EventScript_Person15::
	goto SinnohP_battle_tower_24
	end

Sinnoh_BattleTower_Text_Bg2:
	.string "A machine that keeps records. Its\n"
	.string "screen is dark for now.$"

Sinnoh_BattleTower_Text_Bg3:
	.string "A machine that keeps records. Its\n"
	.string "screen is dark for now.$"

Sinnoh_BattleTower_Text_Person13:
	.string "I'm covering the BATTLE TOWER's\n"
	.string "records. Some of these streaks are\l"
	.string "legendary.$"

@ --- end of SINNOH words ---
