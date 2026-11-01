Sinnoh_Route215_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route215_EventScript_Person24::
	lock
	faceplayer
	goto_if_set 0x3DCF, Sinnoh_Route215_EventScript_Person24_Given
	msgbox Sinnoh_Route215_Text_Person24, MSGBOX_DEFAULT
	msgbox Sinnoh_Route215_Text_Person24Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM66, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DCF
	release
	end

Sinnoh_Route215_EventScript_Person24_Given::
	msgbox Sinnoh_Route215_Text_Person24, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route215_EventScript_Sign9::
	msgbox Sinnoh_Route215_Text_Sign9, MSGBOX_SIGN
	end

Sinnoh_Route215_EventScript_Sign10::
	msgbox Sinnoh_Route215_Text_Sign10, MSGBOX_SIGN
	end

Sinnoh_Route215_EventScript_Sign28::
	msgbox Sinnoh_Route215_Text_Sign28, MSGBOX_SIGN
	end

Sinnoh_Route215_Text_Person24:
	.string "Rain or shine, I train on this road!\n"
	.string "Today it's rain.$"

Sinnoh_Route215_Text_Person24Gift:
	.string "You train in the rain too? Then you've\n"
	.string "earned this TM!$"

Sinnoh_Route215_Text_Sign9:
	.string "ROUTE 215\p"
	.string "To ROUTE 210$"

Sinnoh_Route215_Text_Sign10:
	.string "ROUTE 215\p"
	.string "To VEILSTONE CITY$"

Sinnoh_Route215_Text_Sign28:
	.string "TRAINER TIPS\p"
	.string "Rain powers up WATER moves and\n"
	.string "weakens FIRE moves. Plan your\l"
	.string "battles!$"

@ --- end of SINNOH words ---
