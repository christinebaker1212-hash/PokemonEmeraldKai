Sinnoh_OldChateauBackMiddleWestRoom_MapScripts::
	.byte 0

@ The old TV: ROTOM, at night (Platinum's, Lv. 20), until caught
Sinnoh_OldChateauBackMiddleWestRoom_EventScript_Bg1::
Sinnoh_OldChateauBackMiddleWestRoom_EventScript_Bg2::
	lockall
	goto_if_set 0x3D0F, Sinnoh_OldChateauBackMiddleWestRoom_EventScript_TvOld
	specialvar VAR_RESULT, Sinnoh_IsNight
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_OldChateauBackMiddleWestRoom_EventScript_TvOff
	msgbox Sinnoh_OldChateauBackMiddleWestRoom_Text_TvOn, MSGBOX_DEFAULT
	closemessage
	playmoncry SPECIES_ROTOM, CRY_MODE_ENCOUNTER
	waitmoncry
	setwildbattle SPECIES_ROTOM, 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Sinnoh_OldChateauBackMiddleWestRoom_EventScript_TvEnd
	setflag 0x3D0F
Sinnoh_OldChateauBackMiddleWestRoom_EventScript_TvEnd::
	releaseall
	end

Sinnoh_OldChateauBackMiddleWestRoom_EventScript_TvOff::
	msgbox Sinnoh_OldChateauBackMiddleWestRoom_Text_TvOff, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_OldChateauBackMiddleWestRoom_EventScript_TvOld::
	msgbox Sinnoh_OldChateauBackMiddleWestRoom_Text_TvOld, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_OldChateauBackMiddleWestRoom_Text_TvOff:
	.string "An old TV. It's switched off…\n"
	.string "Did the screen just flicker?$"

Sinnoh_OldChateauBackMiddleWestRoom_Text_TvOn:
	.string "The TV switches on by itself!\n"
	.string "Something's inside the screen!$"

Sinnoh_OldChateauBackMiddleWestRoom_Text_TvOld:
	.string "Just an old, dusty TV.$"

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
@ --- end of SINNOH words ---
