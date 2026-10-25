Johto_OlivineGoodRodHouse_MapScripts::
	.byte 0

@ Johto_OlivineGoodRodHouse object 1 (local id 1, gfx 0x3E95) at 4,4
Johto_OlivineGoodRodHouse_EventScript_83E5757::
	lock
	faceplayer
	checkitem ITEM_GOOD_ROD, 1
	goto_if_eq VAR_RESULT, 1, Johto_OlivineGoodRodHouse_EventScript_83E579B
	msgbox Johto_OlivineGoodRodHouse_Text_83E57B1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_OlivineGoodRodHouse_EventScript_83E57A6
	msgbox Johto_OlivineGoodRodHouse_Text_83E5858, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x2C6
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	msgbox Johto_OlivineGoodRodHouse_Text_83E5891, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGoodRodHouse_EventScript_83E579B::
	msgbox Johto_OlivineGoodRodHouse_Text_83E5979, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGoodRodHouse_EventScript_83E57A6::
	msgbox Johto_OlivineGoodRodHouse_Text_83E593F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGoodRodHouse_Text_83E57B1::
	.string "OLIVINE is on the sea!\p"
	.string "And if it’s on the sea, there\n"
	.string "are bound to be fish!\p"
	.string "I’ve fished here for 30 years.\p"
	.string "Would you like to face the sea\n"
	.string "and try your hand at fishing?$"

Johto_OlivineGoodRodHouse_Text_83E5858::
	.string "Ah, hahah! We have ourselves\n"
	.string "a new angler in the making!$"

Johto_OlivineGoodRodHouse_Text_83E5891::
	.string "Fish aren’t only found in the\n"
	.string "sea--they’re wherever there’s\l"
	.string "water.\p"
	.string "By the way, your POKéDEX shows\n"
	.string "fishing groups for POKéMON.\p"
	.string "Different spots may contain\n"
	.string "similar fish types.$"

Johto_OlivineGoodRodHouse_Text_83E593F::
	.string "Whaaat? You don’t like to fish!?\n"
	.string "That’s incomprehensible!$"

Johto_OlivineGoodRodHouse_Text_83E5979::
	.string "How are things?\n"
	.string "Did you land the big one?$"
