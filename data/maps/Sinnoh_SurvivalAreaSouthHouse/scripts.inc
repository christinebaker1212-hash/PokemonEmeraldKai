Sinnoh_SurvivalAreaSouthHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SurvivalAreaSouthHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DDD, Sinnoh_SurvivalAreaSouthHouse_EventScript_Person1_Given
	msgbox Sinnoh_SurvivalAreaSouthHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_SurvivalAreaSouthHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM42, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DDD
	release
	end

Sinnoh_SurvivalAreaSouthHouse_EventScript_Person1_Given::
	msgbox Sinnoh_SurvivalAreaSouthHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_SurvivalAreaSouthHouse_Text_Person1:
	.string "I train my body and my MOéMON\n"
	.string "together. One push-up for each of us!$"

Sinnoh_SurvivalAreaSouthHouse_Text_Person1Gift:
	.string "You've got the spirit of a fighter!\n"
	.string "Take this TM.$"

@ --- end of SINNOH words ---
