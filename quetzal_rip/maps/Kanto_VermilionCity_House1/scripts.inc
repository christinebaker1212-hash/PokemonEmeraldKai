Kanto_VermilionCity_House1_MapScripts::
	.byte 0

@ Kanto_VermilionCity_House1 object 1 (local id 1, gfx 0x3F06) at 4,5
Kanto_VermilionCity_House1_EventScript_841B37B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x215B, Kanto_VermilionCity_House1_EventScript_841B3A3
	msgbox Kanto_VermilionCity_House1_Text_844402D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_House1_EventScript_841B3AD
	msgbox Kanto_VermilionCity_House1_Text_8444194, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_House1_EventScript_841B3A3::
	msgbox Kanto_VermilionCity_House1_Text_84441B1, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_House1_EventScript_841B3AD::
	checkitem ITEM_OLD_ROD, 1
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_House1_EventScript_841B3EF
	additem ITEM_OLD_ROD, 1
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_House1_EventScript_841B3FC
	msgbox Kanto_VermilionCity_House1_Text_8444096, MSGBOX_DEFAULT
	loadword 0, Kanto_VermilionCity_House1_Text_84440ED
	setorcopyvar VAR_0x8000, 0x2C5
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E

Kanto_VermilionCity_House1_EventScript_841B3EF::
	setflag FLAG_QZ_0x215B
	msgbox Kanto_VermilionCity_House1_Text_844411B, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_House1_EventScript_841B3FC::
	msgbox Kanto_VermilionCity_House1_Text_84441DB, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_House1_Text_844402D::
	.string "I’m the FISHING GURU!\p"
	.string "I simply looove fishing!\n"
	.string "I can’t bear to go without.\p"
	.string "Tell me, do you like to fish?$"

Kanto_VermilionCity_House1_Text_8444096::
	.string "Grand! I like your style.\n"
	.string "I think we can be friends.\p"
	.string "Take this and fish, young friend!$"

Kanto_VermilionCity_House1_Text_84440ED::
	.string "{PLAYER} received an OLD ROD from\n"
	.string "the FISHING GURU.$"

Kanto_VermilionCity_House1_Text_844411B::
	.string "Fishing is a way of life!\n"
	.string "It is like the finest poetry.\p"
	.string "From the seas to rivers, go out\n"
	.string "and land the big one, my friend.$"

Kanto_VermilionCity_House1_Text_8444194::
	.string "Oh…\n"
	.string "That’s so disappointing…$"

Kanto_VermilionCity_House1_Text_84441B1::
	.string "Hello there, {PLAYER}!\p"
	.string "How are the fish biting?$"

Kanto_VermilionCity_House1_Text_84441DB::
	.string "Oh, no!\p"
	.string "I had a nice gift for you, but you\n"
	.string "have no room for it!$"
