Sinnoh_StarkMountainRoom3_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_StarkMountainRoom3_OnTransition
	.byte 0

Sinnoh_StarkMountainRoom3_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_StarkMountainRoom3_EventScript_Person1::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person1, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_EventScript_Person3::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person3, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_EventScript_Person5::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person5, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_EventScript_Person6::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person6, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_EventScript_Person8::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person8, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_EventScript_Person9::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person9, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_EventScript_Person10::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person10, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_EventScript_Person11::
	msgbox Sinnoh_StarkMountainRoom3_Text_Person11, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom3_Text_Person1:
	.string "CROAGUNK croaks proudly at CHARON's\n"
	.string "feet.$"

Sinnoh_StarkMountainRoom3_Text_Person3:
	.string "BUCK: Phew. The mountain's quiet\n"
	.string "again. Thanks for the help, CHAMPION!$"

Sinnoh_StarkMountainRoom3_Text_Person5:
	.string "LOOKER: CHARON is under arrest!\n"
	.string "Excellent work. TEAM GALACTIC is truly\l"
	.string "finished.$"

Sinnoh_StarkMountainRoom3_Text_Person6:
	.string "CHARON: Arrested? Me? I'm just a\n"
	.string "harmless old scientist! ...Fine, a\l"
	.string "mostly harmless one.$"

Sinnoh_StarkMountainRoom3_Text_Person8:
	.string "We'll take it from here. Thank you for\n"
	.string "your help.$"

Sinnoh_StarkMountainRoom3_Text_Person9:
	.string "Everyone stay calm. The suspect is\n"
	.string "very slow. This won't take long.$"

Sinnoh_StarkMountainRoom3_Text_Person10:
	.string "We surrender! Can we leave the\n"
	.string "volcano now?$"

Sinnoh_StarkMountainRoom3_Text_Person11:
	.string "I'm going home to become a florist.$"

@ --- end of SINNOH words ---
