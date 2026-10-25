Kanto_FuchsiaCity_House2_MapScripts::
	.byte 0

@ Kanto_FuchsiaCity_House2 object 1 (local id 1, gfx 0x3F06) at 6,5
Kanto_FuchsiaCity_House2_EventScript_841D9B5::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x214B, Kanto_FuchsiaCity_House2_EventScript_841D9DD
	msgbox Kanto_FuchsiaCity_House2_Text_84490D6, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_FuchsiaCity_House2_EventScript_841D9E7
	msgbox Kanto_FuchsiaCity_House2_Text_844927F, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_House2_EventScript_841D9DD::
	msgbox Kanto_FuchsiaCity_House2_Text_844929C, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_House2_EventScript_841D9E7::
	checkitem ITEM_GOOD_ROD, 1
	goto_if_eq VAR_RESULT, 1, Kanto_FuchsiaCity_House2_EventScript_841DA29
	msgbox Kanto_FuchsiaCity_House2_Text_844914F, MSGBOX_DEFAULT
	additem ITEM_GOOD_ROD, 1
	goto_if_eq VAR_RESULT, 0, Kanto_FuchsiaCity_House2_EventScript_841DA36
	loadword 0, Kanto_FuchsiaCity_House2_Text_84491A6
	setorcopyvar VAR_0x8000, 0x2C6
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E

Kanto_FuchsiaCity_House2_EventScript_841DA29::
	msgbox Kanto_FuchsiaCity_House2_Text_84491DE, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x214B
	release
	end

Kanto_FuchsiaCity_House2_EventScript_841DA36::
	msgbox Kanto_FuchsiaCity_House2_Text_84492C6, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_House2_Text_84490D6::
	.string "I’m the FISHING GURU’s older\n"
	.string "brother.\p"
	.string "I simply looove fishing!\n"
	.string "I can’t bear to go without.\p"
	.string "Tell me, do you like to fish?$"

Kanto_FuchsiaCity_House2_Text_844914F::
	.string "Grand! I like your style.\n"
	.string "I think we can be friends.\p"
	.string "Take this and fish, young friend!$"

Kanto_FuchsiaCity_House2_Text_84491A6::
	.string "{PLAYER} received a GOOD ROD from\n"
	.string "the FISHING GURU’s brother.$"

Kanto_FuchsiaCity_House2_Text_84491DE::
	.string "Fishing is a way of life!\n"
	.string "It is like the finest poetry.\p"
	.string "A crummy OLD ROD could only catch\n"
	.string "MAGIKARP, yes?\p"
	.string "But with a GOOD ROD, you can\n"
	.string "catch much better POKéMON.$"

Kanto_FuchsiaCity_House2_Text_844927F::
	.string "Oh…\n"
	.string "That’s so disappointing…$"

Kanto_FuchsiaCity_House2_Text_844929C::
	.string "Hello there, {PLAYER}!\p"
	.string "How are the fish biting?$"

Kanto_FuchsiaCity_House2_Text_84492C6::
	.string "Oh, no!\p"
	.string "I had a gift for you, but you have\n"
	.string "no room for it!$"
