Sinnoh_IronIsland_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_IronIsland_EventScript_Person1::
	lock
	faceplayer
	msgbox Sinnoh_IronIsland_Text_Person1Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_IronIsland_EventScript_Person1_Sail0
	msgbox Sinnoh_IronIsland_Text_Person1Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_IronIsland_EventScript_Person1_Sail0::
	msgbox Sinnoh_IronIsland_Text_Person1Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_CANALAVE_CITY, 11, 22
	waitstate
	releaseall
	end

Sinnoh_IronIsland_Text_Person1Ask0:
	.string "Shall I take you to CANALAVE CITY?$"

Sinnoh_IronIsland_Text_Person1Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_IronIsland_Text_Person1Off:
	.string "All aboard! Hold on tight!$"

@ --- end of SINNOH words ---
