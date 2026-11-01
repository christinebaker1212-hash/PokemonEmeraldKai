Sinnoh_Route211East_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route211East_EventScript_Person18::
	lock
	faceplayer
	goto_if_set 0x3DCE, Sinnoh_Route211East_EventScript_Person18_Given
	msgbox Sinnoh_Route211East_Text_Person18, MSGBOX_DEFAULT
	msgbox Sinnoh_Route211East_Text_Person18Gift, MSGBOX_DEFAULT
	giveitem ITEM_TM77, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DCE
	release
	end

Sinnoh_Route211East_EventScript_Person18_Given::
	msgbox Sinnoh_Route211East_Text_Person18, MSGBOX_DEFAULT
	release
	end

Sinnoh_Route211East_EventScript_Sign11::
	msgbox Sinnoh_Route211East_Text_Sign11, MSGBOX_SIGN
	end

Sinnoh_Route211East_EventScript_Sign12::
	msgbox Sinnoh_Route211East_Text_Sign12, MSGBOX_SIGN
	end

Sinnoh_Route211East_EventScript_Sign21::
	msgbox Sinnoh_Route211East_Text_Sign21, MSGBOX_SIGN
	end

Sinnoh_Route211East_Text_Person18:
	.string "MT. CORONET splits SINNOH right down\n"
	.string "the middle. Every TRAINER crosses it\l"
	.string "at least once.$"

Sinnoh_Route211East_Text_Person18Gift:
	.string "Here's a TM for the long climb. It'll\n"
	.string "pump your MOéMON up!$"

Sinnoh_Route211East_Text_Sign11:
	.string "TRAINER TIPS\p"
	.string "Caves are full of MOéMON you won't see\n"
	.string "anywhere else. Bring REPELS if you're\l"
	.string "in a hurry!$"

Sinnoh_Route211East_Text_Sign12:
	.string "ROUTE 211\p"
	.string "To MT. CORONET$"

Sinnoh_Route211East_Text_Sign21:
	.string "ROUTE 211\p"
	.string "To CELESTIC TOWN$"

@ --- end of SINNOH words ---
