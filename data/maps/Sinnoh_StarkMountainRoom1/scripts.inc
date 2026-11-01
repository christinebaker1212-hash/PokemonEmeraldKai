Sinnoh_StarkMountainRoom1_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_StarkMountainRoom1_OnTransition
	.byte 0

Sinnoh_StarkMountainRoom1_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_StarkMountainRoom1_EventScript_Person15::
	msgbox Sinnoh_StarkMountainRoom1_Text_Person15, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom1_EventScript_Person16::
	msgbox Sinnoh_StarkMountainRoom1_Text_Person16, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom1_EventScript_Person17::
	msgbox Sinnoh_StarkMountainRoom1_Text_Person17, MSGBOX_NPC
	end

Sinnoh_StarkMountainRoom1_Text_Person15:
	.string "CHARON: Heh heh. With the POKéMON in\n"
	.string "this mountain, TEAM GALACTIC will rise\l"
	.string "again. Led by me, of course.$"

Sinnoh_StarkMountainRoom1_Text_Person16:
	.string "COMMANDER CHARON's in charge now. I\n"
	.string "think. Nobody told me.$"

Sinnoh_StarkMountainRoom1_Text_Person17:
	.string "I miss the HQ. It had air conditioning.$"

@ --- end of SINNOH words ---
