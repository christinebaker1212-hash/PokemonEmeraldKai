Sinnoh_Route218GateToJubilifeCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route218GateToJubilifeCity_EventScript_Person1::
	msgbox Sinnoh_Route218GateToJubilifeCity_Text_Person1, MSGBOX_NPC
	end

Sinnoh_Route218GateToJubilifeCity_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DD2, Sinnoh_Route218GateToJubilifeCity_EventScript_Person2_Given
	msgbox Sinnoh_Route218GateToJubilifeCity_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_Route218GateToJubilifeCity_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_OLD_ROD, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD2
	release
	end

Sinnoh_Route218GateToJubilifeCity_EventScript_Person2_Given::
	msgbox Sinnoh_Route218GateToJubilifeCity_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route218GateToJubilifeCity_Text_Person1:
	.string "I keep watch at this gate. Mostly I\n"
	.string "just say hello.$"

Sinnoh_Route218GateToJubilifeCity_Text_Person2:
	.string "The fishing on ROUTE 218 is great.\n"
	.string "Every TRAINER should try it once!$"

Sinnoh_Route218GateToJubilifeCity_Text_Person2Gift:
	.string "Never fished before? Then start with\n"
	.string "this rod!$"

@ --- end of SINNOH words ---
