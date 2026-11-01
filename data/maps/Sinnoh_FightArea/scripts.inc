Sinnoh_FightArea_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FightArea_EventScript_Person6::
	lock
	faceplayer
	msgbox Sinnoh_FightArea_Text_Person6Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_FightArea_EventScript_Person6_Sail0
	msgbox Sinnoh_FightArea_Text_Person6Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_FightArea_EventScript_Person6_Sail0::
	msgbox Sinnoh_FightArea_Text_Person6Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_SNOWPOINT_CITY, 21, 37
	waitstate
	releaseall
	end

Sinnoh_FightArea_EventScript_Person19::
	lock
	faceplayer
	msgbox Sinnoh_FightArea_Text_Person19Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_FightArea_EventScript_Person19_Sail0
	msgbox Sinnoh_FightArea_Text_Person19Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_FightArea_EventScript_Person19_Sail0::
	msgbox Sinnoh_FightArea_Text_Person19Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_SNOWPOINT_CITY, 21, 37
	waitstate
	releaseall
	end

Sinnoh_FightArea_Text_Person6Ask0:
	.string "Shall I take you to SNOWPOINT CITY?$"

Sinnoh_FightArea_Text_Person6Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_FightArea_Text_Person6Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_FightArea_Text_Person19Ask0:
	.string "Shall I take you to SNOWPOINT CITY?$"

Sinnoh_FightArea_Text_Person19Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_FightArea_Text_Person19Off:
	.string "All aboard! Hold on tight!$"

@ --- end of SINNOH words ---
