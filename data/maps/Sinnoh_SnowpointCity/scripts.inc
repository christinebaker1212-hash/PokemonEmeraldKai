Sinnoh_SnowpointCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SnowpointCity_EventScript_Person1::
	msgbox Sinnoh_SnowpointCity_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SnowpointCity_EventScript_Person4::
	msgbox Sinnoh_SnowpointCity_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SnowpointCity_EventScript_Person5::
	msgbox Sinnoh_SnowpointCity_Text_Person5, MSGBOX_NPC
	end

Sinnoh_SnowpointCity_EventScript_Person6::
	lock
	faceplayer
	msgbox Sinnoh_SnowpointCity_Text_Person6Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_SnowpointCity_EventScript_Person6_Sail0
	msgbox Sinnoh_SnowpointCity_Text_Person6Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_SnowpointCity_EventScript_Person6_Sail0::
	msgbox Sinnoh_SnowpointCity_Text_Person6Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_FIGHT_AREA, 26, 27
	waitstate
	releaseall
	end

Sinnoh_SnowpointCity_EventScript_Person7::
	msgbox Sinnoh_SnowpointCity_Text_Person7, MSGBOX_NPC
	end

Sinnoh_SnowpointCity_EventScript_Person10::
	msgbox Sinnoh_SnowpointCity_Text_Person10, MSGBOX_NPC
	end

Sinnoh_SnowpointCity_EventScript_Sign2::
	msgbox Sinnoh_SnowpointCity_Text_Sign2, MSGBOX_SIGN
	end

Sinnoh_SnowpointCity_EventScript_Sign3::
	msgbox Sinnoh_SnowpointCity_Text_Sign3, MSGBOX_SIGN
	end

Sinnoh_SnowpointCity_Text_Person1:
	.string "It snows almost every day in\n"
	.string "SNOWPOINT. Bundle up!$"

Sinnoh_SnowpointCity_Text_Person4:
	.string "The boat from the harbor goes to the\n"
	.string "BATTLE ZONE. Only strong TRAINERS go\l"
	.string "there.$"

Sinnoh_SnowpointCity_Text_Person5:
	.string "My ears get so cold here! My MOéMON\n"
	.string "keeps them warm, though.$"

Sinnoh_SnowpointCity_Text_Person6Ask0:
	.string "Shall I take you to the BATTLE ZONE?$"

Sinnoh_SnowpointCity_Text_Person6Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_SnowpointCity_Text_Person6Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_SnowpointCity_Text_Person7:
	.string "The temple north of the city is very\n"
	.string "old. Only people who have the LEAGUE's\l"
	.string "blessing may go inside.$"

Sinnoh_SnowpointCity_Text_Person10:
	.string "We make snow sculptures of MOéMON\n"
	.string "every winter. Last year's SNORLAX\l"
	.string "took three days!$"

Sinnoh_SnowpointCity_Text_Sign2:
	.string "SNOWPOINT CITY\p"
	.string "The frozen town at the top of SINNOH.$"

Sinnoh_SnowpointCity_Text_Sign3:
	.string "SNOWPOINT CITY MOéMON GYM\p"
	.string "LEADER: CANDICE\p"
	.string "The diamond-hard focus!$"

@ --- end of SINNOH words ---
