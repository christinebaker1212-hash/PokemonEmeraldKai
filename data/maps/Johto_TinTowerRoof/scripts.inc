Johto_TinTowerRoof_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_TinTowerRoof_EventScript_83FB60B
	.byte 0

Johto_TinTowerRoof_EventScript_83FB60B::
	special QzJohtoStaticEncountersRefresh
	call_if_set FLAG_QZ_0x1016, Johto_TinTowerRoof_EventScript_83FB637
	call_if_unset FLAG_QZ_0x1016, Johto_TinTowerRoof_EventScript_83FB63B
	goto_if_eq VAR_QZ_0x5007, 2, Johto_TinTowerRoof_EventScript_83FB63F
	goto_if_eq VAR_QZ_0x5007, 3, Johto_TinTowerRoof_EventScript_83FB643
	end

Johto_TinTowerRoof_EventScript_83FB637::
	setmaplayoutindex 0x294
	return

Johto_TinTowerRoof_EventScript_83FB63B::
	setmaplayoutindex 0x293
	return

Johto_TinTowerRoof_EventScript_83FB63F::
	clearflag FLAG_QZ_0x1060
	end

Johto_TinTowerRoof_EventScript_83FB643::
	setflag FLAG_QZ_0x1014
	setflag FLAG_QZ_0x1060
	end

@ Johto_TinTowerRoof object 9 (local id 9, gfx 0x28AE) at 10,10
Johto_TinTowerRoof_EventScript_840EBDE::
	goto_if_set FLAG_QZ_0x1311, Johto_TinTowerRoof_EventScript_840EC4F
	goto_if_unset FLAG_QZ_0x1316, Johto_TinTowerRoof_EventScript_840EBF7
	lock
	faceplayer
	goto Johto_TinTowerRoof_EventScript_840EC1A

Johto_TinTowerRoof_EventScript_840EBF7::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_4960, 0, Johto_TinTowerRoof_Text_840EDDC, Johto_TinTowerRoof_Text_840EE3A, Johto_TinTowerRoof_EventScript_840EC12
	msgbox Johto_TinTowerRoof_Text_840EE6A, MSGBOX_AUTOCLOSE
	end

Johto_TinTowerRoof_EventScript_840EC12::
	setflag FLAG_QZ_0x1316
	goto Johto_TinTowerRoof_EventScript_840EC1A

Johto_TinTowerRoof_EventScript_840EC1A::
	checkitem ITEM_RED_ORB, 1
	goto_if_eq VAR_RESULT, 1, Johto_TinTowerRoof_EventScript_840EC41
	setorcopyvar VAR_0x8000, ITEM_RED_ORB
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840ED18

Johto_TinTowerRoof_EventScript_840EC41::
	msgbox Johto_TinTowerRoof_Text_840EE6A, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1311
	release
	end

Johto_TinTowerRoof_EventScript_840EC4F::
	lock
	faceplayer
	msgbox Johto_TinTowerRoof_Text_840EEBB, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Quetzal_EventScript_840ED15
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4960, 0, Johto_TinTowerRoof_Text_840EE3A
	msgbox Johto_TinTowerRoof_Text_840EEF9, MSGBOX_DEFAULT
	goto Quetzal_EventScript_840ED15

Johto_TinTowerRoof_Text_840EDDC::
	.string "I came to study HO-OH’s power.\p"
	.string "But first, let’s see if you live\n"
	.string "up to your title as Champion!$"

Johto_TinTowerRoof_Text_840EE3A::
	.string "Your resolve is firmer\n"
	.string "than the earth itself...$"

Johto_TinTowerRoof_Text_840EE6A::
	.string "This orb responds to your strength.\p"
	.string "GROUDON’s power is no toy.\n"
	.string "Use it with care.$"

Johto_TinTowerRoof_Text_840EEBB::
	.string "I still have much to learn.\n"
	.string "Shall we test our strength again?$"

Johto_TinTowerRoof_Text_840EEF9::
	.string "A good battle always reveals\n"
	.string "something we can improve.$"
