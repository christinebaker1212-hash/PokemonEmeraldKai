Sinnoh_CelesticTownNorthwestHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CelesticTownNorthwestHouse_EventScript_Person1::
	msgbox Sinnoh_CelesticTownNorthwestHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_CelesticTownNorthwestHouse_EventScript_Person2::
	msgbox Sinnoh_CelesticTownNorthwestHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_CelesticTownNorthwestHouse_EventScript_Person3::
	lock
	faceplayer
	goto_if_set 0x3DA4, Sinnoh_CelesticTownNorthwestHouse_EventScript_Person3_Given
	msgbox Sinnoh_CelesticTownNorthwestHouse_Text_Person3, MSGBOX_DEFAULT
	msgbox Sinnoh_CelesticTownNorthwestHouse_Text_Person3Gift, MSGBOX_DEFAULT
	giveitem ITEM_WISE_GLASSES, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA4
	release
	end

Sinnoh_CelesticTownNorthwestHouse_EventScript_Person3_Given::
	msgbox Sinnoh_CelesticTownNorthwestHouse_Text_Person3, MSGBOX_DEFAULT
	release
	end

Sinnoh_CelesticTownNorthwestHouse_Text_Person1:
	.string "We've lived in CELESTIC for\n"
	.string "generations. The mountain protects\l"
	.string "us.$"

Sinnoh_CelesticTownNorthwestHouse_Text_Person2:
	.string "The cave behind town has a shrine. We\n"
	.string "visit it every year on the first snow.$"

Sinnoh_CelesticTownNorthwestHouse_Text_Person3:
	.string "I help keep the shrine clean. It's an\n"
	.string "honor.$"

Sinnoh_CelesticTownNorthwestHouse_Text_Person3Gift:
	.string "The elder gave me these glasses, but\n"
	.string "they suit a clever MOéMON better.\l"
	.string "Here.$"

@ --- end of SINNOH words ---
