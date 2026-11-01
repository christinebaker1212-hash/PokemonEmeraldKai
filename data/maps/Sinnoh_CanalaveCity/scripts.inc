Sinnoh_CanalaveCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCity_EventScript_Person17::
	lock
	faceplayer
	msgbox Sinnoh_CanalaveCity_Text_Person17Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_CanalaveCity_EventScript_Person17_Sail0
	msgbox Sinnoh_CanalaveCity_Text_Person17Ask1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_CanalaveCity_EventScript_Person17_Sail1
	msgbox Sinnoh_CanalaveCity_Text_Person17Ask2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_CanalaveCity_EventScript_Person17_Sail2
	msgbox Sinnoh_CanalaveCity_Text_Person17Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCity_EventScript_Person17_Sail0::
	msgbox Sinnoh_CanalaveCity_Text_Person17Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_IRON_ISLAND, 4, 22
	waitstate
	releaseall
	end

Sinnoh_CanalaveCity_EventScript_Person17_Sail1::
	msgbox Sinnoh_CanalaveCity_Text_Person17Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_FULLMOON_ISLAND, 9, 21
	waitstate
	releaseall
	end

Sinnoh_CanalaveCity_EventScript_Person17_Sail2::
	msgbox Sinnoh_CanalaveCity_Text_Person17Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_NEWMOON_ISLAND, 24, 21
	waitstate
	releaseall
	end

Sinnoh_CanalaveCity_Text_Person17Ask0:
	.string "Shall I take you to IRON ISLAND?$"

Sinnoh_CanalaveCity_Text_Person17Ask1:
	.string "Shall I take you to FULLMOON ISLAND?$"

Sinnoh_CanalaveCity_Text_Person17Ask2:
	.string "Shall I take you to NEWMOON ISLAND?$"

Sinnoh_CanalaveCity_Text_Person17Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_CanalaveCity_Text_Person17Off:
	.string "All aboard! Hold on tight!$"

@ --- end of SINNOH words ---
