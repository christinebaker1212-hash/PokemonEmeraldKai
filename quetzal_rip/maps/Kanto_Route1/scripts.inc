Kanto_Route1_MapScripts::
	.byte 0

@ Kanto_Route1 object 1 (local id 1, gfx 0x3EFA) at 6,28
Kanto_Route1_EventScript_8417BA6::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x215E, Kanto_Route1_EventScript_8417BF0
	msgbox Kanto_Route1_Text_8433564, MSGBOX_DEFAULT
	textcolor 3
	checkitemspace ITEM_POTION, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_84570B3
	bufferitemname 1, ITEM_POTION
	playfanfare MUS_LEVEL_UP
	message Kanto_Route1_Text_84557FD
	waitmessage
	waitfanfare
	additem ITEM_POTION, 1
	msgbox Kanto_Route1_Text_843364E, MSGBOX_DEFAULT
	call Quetzal_EventScript_8456AB7
	setflag FLAG_QZ_0x215E
	release
	end

Kanto_Route1_EventScript_8417BF0::
	msgbox Kanto_Route1_Text_843360E, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route1 object 2 (local id 2, gfx 0x3EEE) at 19,16
Kanto_Route1_EventScript_8417BFA::
	msgbox Kanto_Route1_Text_8433680, MSGBOX_NPC
	end

@ Kanto_Route1 sign / bg event at 9,31
Kanto_Route1_EventScript_8417C03::
	msgbox Kanto_Route1_Text_8433701, MSGBOX_SIGN
	end

Kanto_Route1_Text_8433564::
	.string "Hi!\n"
	.string "I work at a POKéMON MART.\p"
	.string "It’s part of a convenient chain\n"
	.string "selling all sorts of items.\p"
	.string "Please, visit us in VIRIDIAN CITY.\p"
	.string "I know, I’ll give you a sample.\n"
	.string "Here you go!$"

Kanto_Route1_Text_843360E::
	.string "Please come see us if you need\n"
	.string "POKé BALLS for catching POKéMON.$"

Kanto_Route1_Text_843364E::
	.string "{PLAYER} put the POTION away in\n"
	.string "the BAG’s ITEMS POCKET.$"

Kanto_Route1_Text_8433680::
	.string "See those ledges along the road?\p"
	.string "It’s a bit scary, but you can jump\n"
	.string "from them.\p"
	.string "You can get back to PALLET TOWN\n"
	.string "quicker that way.$"

Kanto_Route1_Text_8433701::
	.string "ROUTE 1\n"
	.string "PALLET TOWN - VIRIDIAN CITY$"

Kanto_Route1_Text_84557FD::
	.string "Obtained the {STR_VAR_2}!$"
