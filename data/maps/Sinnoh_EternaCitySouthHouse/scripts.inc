Sinnoh_EternaCitySouthHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCitySouthHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DA9, Sinnoh_EternaCitySouthHouse_EventScript_Person1_Given
	msgbox Sinnoh_EternaCitySouthHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_EternaCitySouthHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_UPGRADE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA9
	release
	end

Sinnoh_EternaCitySouthHouse_EventScript_Person1_Given::
	msgbox Sinnoh_EternaCitySouthHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCitySouthHouse_Text_Person1:
	.string "PROF. OAK: I'm visiting SINNOH to see\n"
	.string "its POKéMON for myself. Marvelous\l"
	.string "variety!\p"
	.string "Your POKéDEX must be getting heavy\n"
	.string "by now.$"

Sinnoh_EternaCitySouthHouse_Text_Person1Gift:
	.string "Here, take this UPGRADE. A curious\n"
	.string "little gadget. Some POKéMON love it!$"

@ --- end of SINNOH words ---
