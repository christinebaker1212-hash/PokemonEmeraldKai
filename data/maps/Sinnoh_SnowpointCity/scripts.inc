Sinnoh_SnowpointCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
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

Sinnoh_SnowpointCity_Text_Person6Ask0:
	.string "Shall I take you to the BATTLE ZONE?$"

Sinnoh_SnowpointCity_Text_Person6Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_SnowpointCity_Text_Person6Off:
	.string "All aboard! Hold on tight!$"

@ --- end of SINNOH words ---
