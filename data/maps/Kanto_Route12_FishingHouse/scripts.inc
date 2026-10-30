Kanto_Route12_FishingHouse_MapScripts::
	.byte 0

@ Kanto_Route12_FishingHouse object 1 (local id 1, gfx 0x3F06) at 4,4
Kanto_Route12_FishingHouse_EventScript_841FE95::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2164, Kanto_Route12_FishingHouse_EventScript_841FF16
	msgbox Kanto_Route12_FishingHouse_Text_844F47A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_FishingHouse_EventScript_841FEBD
	msgbox Kanto_Route12_FishingHouse_Text_844F694, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FEBD::
	checkitem ITEM_SUPER_ROD, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_FishingHouse_EventScript_841FEFF
	additem ITEM_SUPER_ROD, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_FishingHouse_EventScript_841FF0C
	msgbox Kanto_Route12_FishingHouse_Text_844F4F5, MSGBOX_DEFAULT
	loadword 0, Kanto_Route12_FishingHouse_Text_844F54C
	setorcopyvar VAR_0x8000, 0x2C7
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E

Kanto_Route12_FishingHouse_EventScript_841FEFF::
	setflag FLAG_QZ_0x2164
	msgbox Kanto_Route12_FishingHouse_Text_841FFFF, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FF0C::
	msgbox Kanto_Route12_FishingHouse_Text_844F92E, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FF16::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	goto_if_set FLAG_QZ_0x2160, Kanto_Route12_FishingHouse_EventScript_841FF6B
	setvar VAR_0x8004, SPECIES_MAGIKARP
	specialvar VAR_RESULT, QzPartyHasSpecies
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_FishingHouse_EventScript_841FF75
	msgbox Kanto_Route12_FishingHouse_Text_842004E, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_NET_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_FishingHouse_EventScript_841FFDD
	setflag FLAG_QZ_0x2160
	goto Kanto_Route12_FishingHouse_EventScript_841FF6B

Kanto_Route12_FishingHouse_EventScript_841FF6B::
	msgbox Kanto_Route12_FishingHouse_Text_8420092, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FF75::
	msgbox Kanto_Route12_FishingHouse_Text_841FFFF, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_FishingHouse_EventScript_841FFDD::
	msgbox Kanto_Route12_FishingHouse_Text_844F92E, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route12_FishingHouse sign / bg event at 9,1
Kanto_Route12_FishingHouse_EventScript_841FFE7::
	msgbox Kanto_Route12_FishingHouse_Text_84200BC, MSGBOX_SIGN
	end

Kanto_Route12_FishingHouse_Text_841FFFF::
	.string "I love seeing MAGIKARP!\p"
	.string "Bring one in your party,\n"
	.string "and I’ll have a gift for you!$"

Kanto_Route12_FishingHouse_Text_842004E::
	.string "What a fine MAGIKARP!\n"
	.string "Thanks for showing me.\p"
	.string "Please take this gift!$"

Kanto_Route12_FishingHouse_Text_8420092::
	.string "Enjoy fishing!\n"
	.string "Every catch has its charm.$"

Kanto_Route12_FishingHouse_Text_84200BC::
	.string "Fishing notes:\n"
	.string "Patience, a rod, and enthusiasm.$"

Kanto_Route12_FishingHouse_Text_844F47A::
	.string "I’m the FISHING GURU’s younger\n"
	.string "brother.\p"
	.string "I simply looove fishing!\n"
	.string "I can’t bear to go without.\p"
	.string "Tell me, do you like to fish?$"

Kanto_Route12_FishingHouse_Text_844F4F5::
	.string "Grand! I like your style.\n"
	.string "I think we can be friends.\p"
	.string "Take this and fish, young friend!$"

Kanto_Route12_FishingHouse_Text_844F54C::
	.string "{PLAYER} received a SUPER ROD from\n"
	.string "the FISHING GURU’s brother.$"

Kanto_Route12_FishingHouse_Text_844F694::
	.string "Oh…\n"
	.string "That’s so disappointing…$"

Kanto_Route12_FishingHouse_Text_844F92E::
	.string "Oh, no!\p"
	.string "I had a gift for you, but you have\n"
	.string "no room for it.$"
