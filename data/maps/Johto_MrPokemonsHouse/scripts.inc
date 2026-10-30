Johto_MrPokemonsHouse_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_MrPokemonsHouse_OnFrame_83EDB41
	.byte 0

Johto_MrPokemonsHouse_OnFrame_83EDB41:
	map_script_2 VAR_QZ_0x5005, 1, Johto_MrPokemonsHouse_EventScript_83EDB4B
	.2byte 0

Johto_MrPokemonsHouse_EventScript_83EDB4B::
	lock
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject 2, 1
	msgbox Johto_MrPokemonsHouse_Text_83EDCE1, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_MrPokemonsHouse_Movement_83EDCCD
	waitmovement 0
	msgbox Johto_MrPokemonsHouse_Text_83EDD23, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_MYSTERY_EGG
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	msgbox Johto_MrPokemonsHouse_Text_83EDD65, MSGBOX_DEFAULT
	msgbox Johto_MrPokemonsHouse_Text_83EDE0F, MSGBOX_DEFAULT
	turnobject 3, 3
	msgbox Johto_MrPokemonsHouse_Text_83EDE33, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_QZ_646, 1
	applymovement 3, Johto_MrPokemonsHouse_Movement_83EDCD3
	waitmovement 0
	turnobject LOCALID_PLAYER, 4
	goto_if_set FLAG_QZ_0xA79, Johto_MrPokemonsHouse_EventScript_83EDBD8
	msgbox Johto_MrPokemonsHouse_Text_83EDFA9, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Johto_MrPokemonsHouse_Text_83EE24C
	waitfanfare
	setflag FLAG_QZ_0xA79
	special EnableNationalPokedex
	goto Johto_MrPokemonsHouse_EventScript_83EDBE0

Johto_MrPokemonsHouse_EventScript_83EDBD8::
	msgbox Johto_MrPokemonsHouse_Text_83EDEFE, MSGBOX_DEFAULT

Johto_MrPokemonsHouse_EventScript_83EDBE0::
	msgbox Johto_MrPokemonsHouse_Text_83EE261, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_MrPokemonsHouse_Movement_83EDCD6
	waitmovement 0
	playse SE_EXIT
	removeobject 3
	setflag FLAG_QZ_0x107E
	turnobject LOCALID_PLAYER, 2
	fadedefaultbgm
	msgbox Johto_MrPokemonsHouse_Text_83EDE67, MSGBOX_DEFAULT
	call Quetzal_EventScript_83602D9
	msgbox Johto_MrPokemonsHouse_Text_83EDEB0, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Johto_MrPokemonsHouse_Movement_83EDCDD
	waitmovement 0
	removeobject 2
	clearflag FLAG_QZ_0x107F
	addobject 1
	setvar VAR_QZ_0x5004, 4
	setflag FLAG_QZ_0x106E
	clearflag FLAG_QZ_0x1074
	setvar VAR_QZ_0x5005, 2
	setrespawn 0x18
	clearflag FLAG_QZ_0x107B
	setflag FLAG_QZ_0x107C
	clearflag FLAG_QZ_0x108C
	release
	end

@ Johto_MrPokemonsHouse object 1 (local id 1, gfx 0x30) at 5,4
@ Johto_MrPokemonsHouse object 2 (local id 2, gfx 0x30) at 7,5
Johto_MrPokemonsHouse_EventScript_83EDC4F::
	lock
	faceplayer
	checkitem ITEM_RED_SCALE, 1
	goto_if_eq VAR_RESULT, 1, Johto_MrPokemonsHouse_EventScript_83EDC6C
	msgbox Johto_MrPokemonsHouse_Text_83EDEC6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MrPokemonsHouse_EventScript_83EDC6C::
	bufferitemname 0, ITEM_DRAGON_SCALE
	msgbox Johto_MrPokemonsHouse_Text_83EE301, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_MrPokemonsHouse_EventScript_83EDC8E
	msgbox Johto_MrPokemonsHouse_Text_83EE38E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MrPokemonsHouse_EventScript_83EDC8E::
	setorcopyvar VAR_0x8000, ITEM_DRAGON_SCALE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, 0, Quetzal_EventScript_83602AA
	removeitem ITEM_RED_SCALE, 1
	release
	end

@ Johto_MrPokemonsHouse sign / bg event at 1,1
@ Johto_MrPokemonsHouse sign / bg event at 2,1
@ Johto_MrPokemonsHouse sign / bg event at 3,1
@ Johto_MrPokemonsHouse sign / bg event at 4,1
Johto_MrPokemonsHouse_EventScript_83EDCB7::
	lock
	msgbox Johto_MrPokemonsHouse_Text_83EE3E6, MSGBOX_SIGN
	release
	end

@ Johto_MrPokemonsHouse sign / bg event at 3,4
Johto_MrPokemonsHouse_EventScript_83EDCC2::
	lock
	msgbox Johto_MrPokemonsHouse_Text_83EE44F, MSGBOX_SIGN
	release
	end

Johto_MrPokemonsHouse_Movement_83EDCCD:
	walk_up
	walk_up
	walk_right
	walk_right
	face_up
	step_end

Johto_MrPokemonsHouse_Movement_83EDCD3:
	walk_down
	face_left
	step_end

Johto_MrPokemonsHouse_Movement_83EDCD6:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_down
	walk_in_place_down
	step_end

Johto_MrPokemonsHouse_Movement_83EDCDD:
	walk_up
	walk_left
	walk_left
	step_end

Johto_MrPokemonsHouse_Text_83EDCE1::
	.string "Hello, hello! You must be\n"
	.string "{PLAYER}.\p"
	.string "PROF.ELM said that you would\n"
	.string "visit.$"

Johto_MrPokemonsHouse_Text_83EDD23::
	.string "This is what I want PROF.ELM\n"
	.string "to examine.$"

Johto_MrPokemonsHouse_Text_83EDD65::
	.string "I know a couple who run a\n"
	.string "POKéMON DAY-CARE service.\p"
	.string "They gave me that EGG.\p"
	.string "I was intrigued, so I sent\n"
	.string "mail to PROF.ELM.\p"
	.string "For POKéMON evolution,\n"
	.string "PROF.ELM is the authority.$"

Johto_MrPokemonsHouse_Text_83EDE0F::
	.string "Even PROF.OAK here recognizes\n"
	.string "that.$"

Johto_MrPokemonsHouse_Text_83EDE33::
	.string "If my assumption is correct,\n"
	.string "PROF.ELM will know it.$"

Johto_MrPokemonsHouse_Text_83EDE67::
	.string "You are returning to PROF.ELM?\p"
	.string "Here. Your POKéMON should have\n"
	.string "some rest.$"

Johto_MrPokemonsHouse_Text_83EDEB0::
	.string "I’m depending on you!$"

Johto_MrPokemonsHouse_Text_83EDEC6::
	.string "Life is delightful!\n"
	.string "Always, new discoveries to be made.$"

Johto_MrPokemonsHouse_Text_83EDEFE::
	.string "OAK: So you’re {PLAYER}!\p"
	.string "I’m OAK, a POKéMON researcher.\p"
	.string "I see ELM already gave you a\n"
	.string "POKéDEX. Your journey has begun!\p"
	.string "Meeting many POKéMON will help\n"
	.string "you understand them better.$"

Johto_MrPokemonsHouse_Text_83EDFA9::
	.string "OAK: Aha! So you’re {PLAYER}!\p"
	.string "I’m OAK! A POKéMON researcher.\p"
	.string "I was just visiting my old friend\n"
	.string "MR. POKéMON.\p"
	.string "I heard you were running an errand\n"
	.string "for PROF.ELM, so I waited here.\p"
	.string "Oh! What’s this? A rare POKéMON!\p"
	.string "Let’s see…\n"
	.string "Hm, I see!\p"
	.string "I understand why PROF.ELM gave\n"
	.string "you a POKéMON for this errand.\p"
	.string "To researchers like PROF.ELM and\n"
	.string "I, POKéMON are our friends.\p"
	.string "He saw that you would treat your\n"
	.string "POKéMON with love and care.\p"
	.string "…Ah!\p"
	.string "You seem to be dependable. How\n"
	.string "would you like to help me out?\p"
	.string "See? This is the latest version of\n"
	.string "the POKéDEX.\p"
	.string "It automatically records data on\n"
	.string "POKéMON you’ve seen or caught.\p"
	.string "It’s a hi-tech encyclopedia!\p"
	.string "It shows their habitats, how\n"
	.string "they evolve, and their moves.$"

Johto_MrPokemonsHouse_Text_83EE24C::
	.string "{PLAYER} received POKéDEX!$"

Johto_MrPokemonsHouse_Text_83EE261::
	.string "Go meet many kinds of POKéMON and\n"
	.string "complete that POKéDEX!\p"
	.string "But I’ve stayed too long.\p"
	.string "I have to get to GOLDENROD\n"
	.string "for my usual radio show.\p"
	.string "{PLAYER}, I’m counting on you!$"

Johto_MrPokemonsHouse_Text_83EE301::
	.string "Hm? That SCALE! What’s that?\n"
	.string "A red GYARADOS?\p"
	.string "That’s rare! I, I want it…\p"
	.string "{PLAYER}, would you care to\n"
	.string "trade it?\p"
	.string "I can offer this item in return:\n"
	.string "{STR_VAR_1}.$"

Johto_MrPokemonsHouse_Text_83EE38E::
	.string "That’s disappointing.\n"
	.string "That happens to be very rare.$"

Johto_MrPokemonsHouse_Text_83EE3E6::
	.string "It’s packed with foreign\n"
	.string "magazines.\p"
	.string "Can’t even read their titles…$"

Johto_MrPokemonsHouse_Text_83EE44F::
	.string "A whole pile of strange coins!\p"
	.string "Maybe they’re from another\n"
	.string "country…$"
