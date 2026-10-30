Johto_Route39Farmhouse_MapScripts::
	.byte 0

@ Johto_Route39Farmhouse object 2 (local id 2, gfx 0x18) at 7,5
Johto_Route39Farmhouse_EventScript_83F3909::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501B, 5, Johto_Route39Farmhouse_EventScript_83F3921
	msgbox Johto_Route39Farmhouse_Text_83F3BF2, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route39Farmhouse_EventScript_83F3921::
	goto_if_set FLAG_QZ_0x1045, Johto_Route39Farmhouse_EventScript_83F394C
	msgbox Johto_Route39Farmhouse_Text_83F3C53, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_LEFTOVERS
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1045

Johto_Route39Farmhouse_EventScript_83F394C::
	msgbox Johto_Route39Farmhouse_Text_83F3CB2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route39Farmhouse object 1 (local id 1, gfx 0x3E84) at 4,4
Johto_Route39Farmhouse_EventScript_83F3957::
	lock
	faceplayer
	goto_if_lt VAR_QZ_0x501B, 5, Johto_Route39Farmhouse_EventScript_83F39E1
	showmoneybox 0, 0, 0
	msgbox Johto_Route39Farmhouse_Text_83F3AB5, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route39Farmhouse_EventScript_83F3989
	msgbox Johto_Route39Farmhouse_Text_83F3B8E, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	release
	end

Johto_Route39Farmhouse_EventScript_83F3989::
	checkmoney 0x1F4, 0
	goto_if_eq VAR_RESULT, 0, Johto_Route39Farmhouse_EventScript_83F39FA
	additem ITEM_MOOMOO_MILK, 1
	goto_if_eq VAR_RESULT, 0, Johto_Route39Farmhouse_EventScript_83F39EC
	removemoney 0x1F4, 0
	updatemoneybox 0
	msgbox Johto_Route39Farmhouse_Text_83F3BB3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route39Farmhouse_EventScript_83F3989
	msgbox Johto_Route39Farmhouse_Text_83F3B36, MSGBOX_DEFAULT
	closemessage
	playse MUS_OBTAIN_ITEM
	msgbox Johto_Route39Farmhouse_Text_83F3BC5, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	release
	end

Johto_Route39Farmhouse_EventScript_83F39E1::
	msgbox Johto_Route39Farmhouse_Text_83F3A08, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route39Farmhouse_EventScript_83F39EC::
	msgbox Johto_Route39Farmhouse_Text_83F3B74, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	release
	end

Johto_Route39Farmhouse_EventScript_83F39FA::
	msgbox Johto_Route39Farmhouse_Text_83F3B54, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	release
	end

Johto_Route39Farmhouse_Text_83F3A08::
	.string "My MILTANK ain’t givin’ me\n"
	.string "milk no more.\p"
	.string "This here FARM’s got famous milk.\p"
	.string "Most everyone wants a drink.\p"
	.string "It’ll give me lots o’ milk if’n\n"
	.string "I feed it lots o’ BERRIES,\l"
	.string "I reckon.$"

Johto_Route39Farmhouse_Text_83F3AB5::
	.string "How’d you like my MOOMOO MILK?\p"
	.string "It’s my pride and joy, there.\p"
	.string "Give it to POKéMON to restore HP!\p"
	.string "I’ll give it to ya fer just\n"
	.string "¥500.$"

Johto_Route39Farmhouse_Text_83F3B36::
	.string "Here ya go!\n"
	.string "Drink up’n enjoy!$"

Johto_Route39Farmhouse_Text_83F3B54::
	.string "Sorry, there.\n"
	.string "No cash, no sale!$"

Johto_Route39Farmhouse_Text_83F3B74::
	.string "I reckon yer PACK’s full.$"

Johto_Route39Farmhouse_Text_83F3B8E::
	.string "You don’t want it?\n"
	.string "Come again, hear?$"

Johto_Route39Farmhouse_Text_83F3BB3::
	.string "How bout another?$"

Johto_Route39Farmhouse_Text_83F3BC5::
	.string "{PLAYER} got MOOMOO MILK.$"

Johto_Route39Farmhouse_Text_83F3BF2::
	.string "Our milk even goes out to\n"
	.string "KANTO.\p"
	.string "So if our own MILTANK won’t\n"
	.string "give us any milk, we’re in trouble.$"

Johto_Route39Farmhouse_Text_83F3C53::
	.string "You fixed our MILTANK, hon.\n"
	.string "Now it gives MOOMOO MILK again.\p"
	.string "Here’s somethin’ fer your trouble.$"

Johto_Route39Farmhouse_Text_83F3CB2::
	.string "A POKéMON holding LEFTOVERS\n"
	.string "regains a little HP every turn.\p"
	.string "That’ll help it keep its strength\n"
	.string "up in battle, hon!$"
