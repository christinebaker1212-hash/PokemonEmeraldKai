Johto_VictoryRoadGate_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_VictoryRoadGate_EventScript_83E8DCD
	map_script MAP_SCRIPT_ON_LOAD, Johto_VictoryRoadGate_EventScript_83E8DE8
	map_script MAP_SCRIPT_ON_RESUME, Johto_VictoryRoadGate_EventScript_83E8DE8
	map_script MAP_SCRIPT_ON_RETURN_TO_FIELD, Johto_VictoryRoadGate_EventScript_83E8E06
	.byte 0

Johto_VictoryRoadGate_EventScript_83E8DCD::
	call Johto_VictoryRoadGate_EventScript_83E8DEE
	goto_if_set FLAG_QZ_0x105E, Johto_VictoryRoadGate_EventScript_83E8DE7
	goto_if_unset FLAG_QZ_0x11B1, Johto_VictoryRoadGate_EventScript_83E8DE7
	setflag FLAG_QZ_0x105E

Johto_VictoryRoadGate_EventScript_83E8DE7::
	end

Johto_VictoryRoadGate_EventScript_83E8DE8::
	call Johto_VictoryRoadGate_EventScript_83E8DEE
	end

Johto_VictoryRoadGate_EventScript_83E8DEE::
	specialvar VAR_RESULT, QzVictoryRoadGateCheck_772
	goto_if_eq VAR_RESULT, 1, Johto_VictoryRoadGate_EventScript_83E8E02
	clearflag FLAG_QZ_0x105D
	return

Johto_VictoryRoadGate_EventScript_83E8E02::
	setflag FLAG_QZ_0x105D
	return

Johto_VictoryRoadGate_EventScript_83E8E06::
	call Johto_VictoryRoadGate_EventScript_83E8DEE
	goto_if_set FLAG_QZ_0x105D, Johto_VictoryRoadGate_EventScript_83E8E2A
	goto_if_set FLAG_QZ_0x1318, Johto_VictoryRoadGate_EventScript_83E8E2A
	goto_if_set FLAG_QZ_0x1425, Johto_VictoryRoadGate_EventScript_83E8E2A
	addobject 3
	end

Johto_VictoryRoadGate_EventScript_83E8E2A::
	removeobject 3
	end

@ Johto_VictoryRoadGate object 1 (local id 1, gfx 0x3EA9) at 9,14
Johto_VictoryRoadGate_EventScript_83E8F11::
	msgbox Johto_VictoryRoadGate_Text_83E906B, MSGBOX_NPC
	end

@ Johto_VictoryRoadGate trigger at 11,14 (VAR_QZ_0x5029 == 1)
Johto_VictoryRoadGate_EventScript_83E8F1A::
	lock
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x14
	goto_if_lt VAR_QZ_0x501A, 8, Johto_VictoryRoadGate_EventScript_83E8F54
	msgbox Johto_VictoryRoadGate_Text_83E8F90, MSGBOX_DEFAULT
	goto_if_unset FLAG_QZ_0x11B9, Johto_VictoryRoadGate_EventScript_83E8F69
	msgbox Johto_VictoryRoadGate_Text_83E906B, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5029, 2
	release
	end

Johto_VictoryRoadGate_EventScript_83E8F54::
	msgbox Johto_VictoryRoadGate_Text_83E8FC3, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	release
	end

Johto_VictoryRoadGate_EventScript_83E8F69::
	msgbox Johto_VictoryRoadGate_Text_83E9014, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	release
	end

@ Johto_VictoryRoadGate object 2 (local id 2, gfx 0x2C) at 7,7
Johto_VictoryRoadGate_EventScript_83E8F7E::
	msgbox Johto_VictoryRoadGate_Text_83E90A7, MSGBOX_NPC
	end

@ Johto_VictoryRoadGate object 3 (local id 3, gfx 0x2C) at 14,7
Johto_VictoryRoadGate_EventScript_83E8F87::
	msgbox Quetzal_Text_83E90EF, MSGBOX_NPC
	end

@ Johto_VictoryRoadGate object 4 (local id 4, gfx 0x3EB0) at 14,7
Johto_VictoryRoadGate_EventScript_8401CBB::
	lock
	special Script_ClearHeldMovement
	faceplayer
	goto_if_set FLAG_QZ_0x1425, Johto_VictoryRoadGate_EventScript_8401CE9
	goto_if_set FLAG_QZ_0x1319, Johto_VictoryRoadGate_EventScript_8401CDC
	msgbox Johto_VictoryRoadGate_Text_8401EA9, MSGBOX_DEFAULT
	release
	end

Johto_VictoryRoadGate_EventScript_8401CDC::
	msgbox Johto_VictoryRoadGate_Text_8401FEC, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Johto_VictoryRoadGate_EventScript_8401CE9::
	msgbox Johto_VictoryRoadGate_Text_840F359, MSGBOX_DEFAULT
	release
	end

Johto_VictoryRoadGate_Text_83E8F90::
	.string "Only trainers who have proven\n"
	.string "themselves may pass.$"

Johto_VictoryRoadGate_Text_83E8FC3::
	.string "Sorry kid, this way is closed until\n"
	.string "someone deals with that LEGENDARY\l"
	.string "POKéMON...$"

Johto_VictoryRoadGate_Text_83E9014::
	.string "You don’t have all the GYM BADGES\n"
	.string "of JOHTO.\p"
	.string "I’m sorry, but I can’t let you go\n"
	.string "through.$"

Johto_VictoryRoadGate_Text_83E906B::
	.string "Oh! The eight BADGES of JOHTO!\p"
	.string "Please, go right on through!$"

Johto_VictoryRoadGate_Text_83E90A7::
	.string "This way leads to MT.SILVER.\p"
	.string "You’ll see scary-strong POKéMON out\n"
	.string "there.$"

Johto_VictoryRoadGate_Text_8401EA9::
	.string "Access to KANTO is forbidden\n"
	.string "by GIOVANNI’s order.$"

Johto_VictoryRoadGate_Text_8401FEC::
	.string "The passage to KANTO is still closed. We\n"
	.string "must secure the road before opening it.$"

Johto_VictoryRoadGate_Text_840F359::
	.string "The darkness will not let you\n"
	.string "leave JOHTO.$"
