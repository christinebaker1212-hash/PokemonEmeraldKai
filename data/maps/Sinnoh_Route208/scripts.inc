Sinnoh_Route208_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route208_EventScript_Person23::
	goto Sinnoh_EventScript_OddKeystoneGiver
	end

Sinnoh_Route208_EventScript_Sign11::
	msgbox Sinnoh_Route208_Text_Sign11, MSGBOX_SIGN
	end

Sinnoh_Route208_EventScript_Sign12::
	msgbox Sinnoh_Route208_Text_Sign12, MSGBOX_SIGN
	end

Sinnoh_Route208_EventScript_Sign13::
	msgbox Sinnoh_Route208_Text_Sign13, MSGBOX_SIGN
	end

Sinnoh_Route208_EventScript_Sign24::
	msgbox Sinnoh_Route208_Text_Sign24, MSGBOX_SIGN
	end

Sinnoh_Route208_Text_Sign11:
	.string "ROUTE 208\p"
	.string "To HEARTHOME CITY$"

Sinnoh_Route208_Text_Sign12:
	.string "BERRY MASTER'S HOUSE\p"
	.string "Every BERRY grows best with love.$"

Sinnoh_Route208_Text_Sign13:
	.string "ROUTE 208\p"
	.string "To MT. CORONET$"

Sinnoh_Route208_Text_Sign24:
	.string "TRAINER TIPS\p"
	.string "BERRIES grow faster in soft soil.\n"
	.string "Water them, and they'll thank you.$"

@ --- end of SINNOH words ---
