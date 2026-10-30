Johto_NationalPark_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_NationalPark_EventScript_83E0C49
	.byte 0

Johto_NationalPark_EventScript_83E0C49::
	call Quetzal_EventScript_840EA62
	end

@ Johto_NationalPark object 1 (local id 1, gfx 0x2F) at 22,39
Johto_NationalPark_EventScript_83E0C4F::
	lock
	faceplayer
	msgbox Johto_NationalPark_Text_83E0D25, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalPark object 3 (local id 3, gfx 0x3E96) at 12,42
Johto_NationalPark_EventScript_83E0C5C::
	lock
	faceplayer
	msgbox Johto_NationalPark_Text_83E0EB3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalPark object 2 (local id 2, gfx 0x23) at 11,42
Johto_NationalPark_EventScript_83E0C69::
	lock
	faceplayer
	msgbox Johto_NationalPark_Text_83E0EE8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalPark object 9 (local id 9, gfx 0x1E) at 17,4
Johto_NationalPark_EventScript_83E0C76::
	lock
	faceplayer
	msgbox Johto_NationalPark_Text_83E0F18, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83602D9
	release
	end

@ Johto_NationalPark object 4 (local id 4, gfx 0x14) at 32,42
Johto_NationalPark_EventScript_83E0C88::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1043, Johto_NationalPark_EventScript_83E0CB5
	msgbox Johto_NationalPark_Text_83E0DB8, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_QUICK_CLAW
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1043

Johto_NationalPark_EventScript_83E0CB5::
	msgbox Johto_NationalPark_Text_83E0E61, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalPark object 37 (local id 37, gfx 0x17) at 33,42
Johto_NationalPark_EventScript_83E0CC0::
	lock
	faceplayer
	msgbox Johto_NationalPark_Text_83E0CCD, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalPark object 6 (local id 6, gfx 0xB) at 25,30
Johto_NationalPark_EventScript_83E0FCA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4268, 0, Johto_NationalPark_Text_83E1040, Johto_NationalPark_Text_83E10A9
	msgbox Johto_NationalPark_Text_83E10B7, MSGBOX_AUTOCLOSE
	end

@ Johto_NationalPark object 5 (local id 5, gfx 0x2D) at 11,27
Johto_NationalPark_EventScript_83E0FE1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4537, 0, Johto_NationalPark_Text_83E1118, Johto_NationalPark_Text_83E1166
	msgbox Johto_NationalPark_Text_83E1184, MSGBOX_AUTOCLOSE
	end

@ Johto_NationalPark object 8 (local id 8, gfx 0x3E84) at 25,9
Johto_NationalPark_EventScript_83E0FF8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4332, 0, Johto_NationalPark_Text_83E11B2, Johto_NationalPark_Text_83E1203
	msgbox Johto_NationalPark_Text_83E1211, MSGBOX_AUTOCLOSE
	end

@ Johto_NationalPark object 7 (local id 7, gfx 0x2F) at 11,13
Johto_NationalPark_EventScript_83E100F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4140, 0, Johto_NationalPark_Text_83E1258, Johto_NationalPark_Text_83E1288
	msgbox Johto_NationalPark_Text_83E128F, MSGBOX_AUTOCLOSE
	end

@ Johto_NationalPark object 11 (local id 11, gfx 0x3A9E) at 3,43
Johto_NationalPark_EventScript_83E1026::
	setorcopyvar VAR_0x8000, ITEM_MOON_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_NationalPark object 12 (local id 12, gfx 0x3A9E) at 39,15
Johto_NationalPark_EventScript_83E1033::
	setorcopyvar VAR_0x8000, ITEM_SOOTHE_BELL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_NationalPark_Text_83E0CCD::
	.string "We come here to see the flowers.\p"
	.string "With her by my side, even a simple\n"
	.string "walk feels special.$"

Johto_NationalPark_Text_83E0D25::
	.string "Look! Check out my bag!\p"
	.string "I printed out my favorites from my\n"
	.string "POKéDEX and stuck them on my bag.$"

Johto_NationalPark_Text_83E0DB8::
	.string "Pay attention, please!\p"
	.string "…Oops, I have to quit thinking like\n"
	.string "a teacher all the time.\p"
	.string "You must be a POKéMON trainer.\p"
	.string "Since you’re working so hard, I\n"
	.string "want you to have this.$"

Johto_NationalPark_Text_83E0E61::
	.string "Let a POKéMON hold that QUICK CLAW.\p"
	.string "Sometimes it will strike first\n"
	.string "during battle.$"

Johto_NationalPark_Text_83E0EB3::
	.string "I’m playing with stickers I printed\n"
	.string "from my POKéDEX.$"

Johto_NationalPark_Text_83E0EE8::
	.string "I get the other guy’s POKéDEX sticker\n"
	.string "if I win.$"

Johto_NationalPark_Text_83E0F18::
	.string "I take walks in the PARK, but I\n"
	.string "never go into the grass.\p"
	.string "Trainers always want to battle…\p"
	.string "You must have been in many battles.\n"
	.string "Let me heal your POKéMON for you.$"

Johto_NationalPark_Text_83E1040::
	.string "The world of POKéMON is deep.\p"
	.string "There are still lots of things we\n"
	.string "don’t know.\p"
	.string "But I know more than you do!$"

Johto_NationalPark_Text_83E10A9::
	.string "Wha-wha-what?$"

Johto_NationalPark_Text_83E10B7::
	.string "There is a lot to learn.\p"
	.string "For example…\p"
	.string "There are 50 kinds of TMs.\p"
	.string "Traded POKéMON level up faster.$"

Johto_NationalPark_Text_83E1118::
	.string "My POKéMON are simply darling.\p"
	.string "Let me tell you how proud my\n"
	.string "darlings make me.$"

Johto_NationalPark_Text_83E1166::
	.string "I can beat you in pride, but…$"

Johto_NationalPark_Text_83E1184::
	.string "I must say, your POKéMON are quite\n"
	.string "cute, too.$"

Johto_NationalPark_Text_83E11B2::
	.string "We adore our POKéMON, even if they\n"
	.string "dislike us.\p"
	.string "That’s what being a FAN is about.$"

Johto_NationalPark_Text_83E1203::
	.string "M-my POKéMON!$"

Johto_NationalPark_Text_83E1211::
	.string "I lost the battle, but my POKéMON win\n"
	.string "the prize for being most lovely.$"

Johto_NationalPark_Text_83E1258::
	.string "Hello? Why are you staring at me?\p"
	.string "Oh, a battle?$"

Johto_NationalPark_Text_83E1288::
	.string "…Hmmm…$"

Johto_NationalPark_Text_83E128F::
	.string "I thought you were staring at me\n"
	.string "because I’m cute!$"
