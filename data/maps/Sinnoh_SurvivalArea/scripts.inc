Sinnoh_SurvivalArea_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SurvivalArea_OnTransition
	.byte 0

Sinnoh_SurvivalArea_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SurvivalArea_EventScript_Bg2::
	msgbox Sinnoh_SurvivalArea_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_SurvivalArea_EventScript_Person4::
	msgbox Sinnoh_SurvivalArea_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SurvivalArea_EventScript_Person5::
	msgbox Sinnoh_SurvivalArea_Text_Person5, MSGBOX_NPC
	end

Sinnoh_SurvivalArea_EventScript_Person6::
	msgbox Sinnoh_SurvivalArea_Text_Person6, MSGBOX_NPC
	end

Sinnoh_SurvivalArea_EventScript_Person8::
	msgbox Sinnoh_SurvivalArea_Text_Person8, MSGBOX_NPC
	end

Sinnoh_SurvivalArea_EventScript_Person9::
	msgbox Sinnoh_SurvivalArea_Text_Person9, MSGBOX_NPC
	end

Sinnoh_SurvivalArea_EventScript_Sign1::
	msgbox Sinnoh_SurvivalArea_Text_Sign1, MSGBOX_SIGN
	end

Sinnoh_SurvivalArea_Text_Bg2:
	.string "The door is shut tight.$"

Sinnoh_SurvivalArea_Text_Person4:
	.string "There are ruins up on the mountain. I'm\n"
	.string "sure there's something amazing inside.$"

Sinnoh_SurvivalArea_Text_Person5:
	.string "This place is all about lasting longer\n"
	.string "than the other TRAINER. I can do\l"
	.string "that!$"

Sinnoh_SurvivalArea_Text_Person6:
	.string "I came here to camp and got dragged\n"
	.string "into ten battles. Best trip ever!$"

Sinnoh_SurvivalArea_Text_Person8:
	.string "I'm filming a show about the BATTLE\n"
	.string "ZONE. Wave at the camera!$"

Sinnoh_SurvivalArea_Text_Person9:
	.string "BUCK: The FRONTIER's tough, but STARK\n"
	.string "MOUNTAIN's tougher. I'll see you up\l"
	.string "there!$"

Sinnoh_SurvivalArea_Text_Sign1:
	.string "SURVIVAL AREA\p"
	.string "Where TRAINERS come to test their\n"
	.string "endurance.$"

@ --- end of SINNOH words ---
