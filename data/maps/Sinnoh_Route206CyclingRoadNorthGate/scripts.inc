Sinnoh_Route206CyclingRoadNorthGate_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route206CyclingRoadNorthGate_EventScript_Person3::
	lock
	faceplayer
	goto_if_set 0x3DC8, Sinnoh_Route206CyclingRoadNorthGate_EventScript_Person3_Given
	msgbox Sinnoh_Route206CyclingRoadNorthGate_Text_Person3, MSGBOX_DEFAULT
	msgbox Sinnoh_Route206CyclingRoadNorthGate_Text_Person3Gift, MSGBOX_DEFAULT
	giveitem ITEM_EXP_SHARE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC8
	release
	end

Sinnoh_Route206CyclingRoadNorthGate_EventScript_Person3_Given::
	msgbox Sinnoh_Route206CyclingRoadNorthGate_Text_Person3, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route206CyclingRoadNorthGate_Text_Person3:
	.string "I'm counting cyclists for a study.\n"
	.string "You'd be amazed how many just ride in\l"
	.string "circles.$"

Sinnoh_Route206CyclingRoadNorthGate_Text_Person3Gift:
	.string "For my study, take this EXP. SHARE.\n"
	.string "Report back how your team grows!$"

@ --- end of SINNOH words ---
