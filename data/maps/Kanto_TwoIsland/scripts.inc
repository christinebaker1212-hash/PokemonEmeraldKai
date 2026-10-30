Kanto_TwoIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_TwoIsland_EventScript_841726C
	.byte 0

Kanto_TwoIsland_EventScript_841726C::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x225F
	setflag FLAG_QZ_0x2231
	call Kanto_TwoIsland_EventScript_8417280
	call Quetzal_EventScript_840FBB5
	end

Kanto_TwoIsland_EventScript_8417280::
	goto_if_set FLAG_QZ_0x220A, Kanto_TwoIsland_EventScript_84172A1
	goto_if_set FLAG_QZ_0x2108, Kanto_TwoIsland_EventScript_84172C2
	goto_if_set FLAG_QZ_0x21EC, Kanto_TwoIsland_EventScript_84172E3
	goto Kanto_TwoIsland_EventScript_84172F2

Kanto_TwoIsland_EventScript_84172A1::
	goto_if_unset FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_84172F8
	goto_if_unset FLAG_QZ_0x222A, Kanto_TwoIsland_EventScript_84172FE
	goto_if_unset FLAG_QZ_0x222B, Kanto_TwoIsland_EventScript_8417307
	goto Kanto_TwoIsland_EventScript_8417310

Kanto_TwoIsland_EventScript_84172C2::
	goto_if_unset FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_84172F8
	goto_if_unset FLAG_QZ_0x21EC, Kanto_TwoIsland_EventScript_84172F8
	goto_if_unset FLAG_QZ_0x222A, Kanto_TwoIsland_EventScript_84172FE
	goto Kanto_TwoIsland_EventScript_8417307

Kanto_TwoIsland_EventScript_84172E3::
	goto_if_unset FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_84172F8
	goto Kanto_TwoIsland_EventScript_84172FE

Kanto_TwoIsland_EventScript_84172F2::
	goto Kanto_TwoIsland_EventScript_84172F8

Kanto_TwoIsland_EventScript_84172F8::
	setvar VAR_QZ_0x5133, 1
	return

Kanto_TwoIsland_EventScript_84172FE::
	setvar VAR_QZ_0x5133, 2
	clearflag FLAG_QZ_0x21D6
	return

Kanto_TwoIsland_EventScript_8417307::
	setvar VAR_QZ_0x5133, 3
	clearflag FLAG_QZ_0x21D2
	return

Kanto_TwoIsland_EventScript_8417310::
	setvar VAR_QZ_0x5133, 4
	clearflag FLAG_QZ_0x21D5
	return

@ Kanto_TwoIsland object 1 (local id 1, gfx 0x3EFA) at 28,4
Kanto_TwoIsland_EventScript_8417319::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5133, 4, Kanto_TwoIsland_EventScript_8417352
	goto_if_eq VAR_QZ_0x5133, 3, Kanto_TwoIsland_EventScript_8417364
	goto_if_eq VAR_QZ_0x5133, 2, Kanto_TwoIsland_EventScript_8417376
	goto Kanto_TwoIsland_EventScript_8417388

Kanto_TwoIsland_EventScript_8417352::
	goto_if_set FLAG_QZ_0x222C, Kanto_TwoIsland_EventScript_841739A
	setflag FLAG_QZ_0x222C
	goto Kanto_TwoIsland_EventScript_841739A

Kanto_TwoIsland_EventScript_8417364::
	goto_if_set FLAG_QZ_0x222B, Kanto_TwoIsland_EventScript_841739A
	setflag FLAG_QZ_0x222B
	goto Kanto_TwoIsland_EventScript_841739A

Kanto_TwoIsland_EventScript_8417376::
	goto_if_set FLAG_QZ_0x222A, Kanto_TwoIsland_EventScript_841739A
	setflag FLAG_QZ_0x222A
	goto Kanto_TwoIsland_EventScript_841739A

Kanto_TwoIsland_EventScript_8417388::
	goto_if_set FLAG_QZ_0x222D, Kanto_TwoIsland_EventScript_841739A
	setflag FLAG_QZ_0x222D
	goto Kanto_TwoIsland_EventScript_841739A

Kanto_TwoIsland_EventScript_841739A::
	goto_if_unset FLAG_QZ_0x2108, Kanto_TwoIsland_EventScript_84173C3
	goto_if_lt VAR_QZ_0x5117, 5, Kanto_TwoIsland_EventScript_84173C3
	message Kanto_TwoIsland_Text_8431CD2
	waitmessage
	pokemart Kanto_TwoIsland_Mart_84173D0
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_EventScript_84173C3::
	msgbox Kanto_TwoIsland_Text_8431C72, MSGBOX_DEFAULT
	release
	end

@ Kanto_TwoIsland object 4 (local id 4, gfx 0x3F41) at 26,5
Kanto_TwoIsland_EventScript_84173D6::
	msgbox Kanto_TwoIsland_Text_8431FF4, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 2 (local id 2, gfx 0x3F57) at 31,6
Kanto_TwoIsland_EventScript_84173DF::
	msgbox Kanto_TwoIsland_Text_843208F, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 3 (local id 3, gfx 0x3EE8) at 30,11
Kanto_TwoIsland_EventScript_84173E8::
	lock
	faceplayer
	setvar VAR_0x8004, 0xA
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	msgbox Kanto_TwoIsland_Text_843210C, MSGBOX_DEFAULT
	release
	end

@ Kanto_TwoIsland object 5 (local id 5, gfx 0x3F4B) at 27,6
Kanto_TwoIsland_EventScript_8417401::
	msgbox Kanto_TwoIsland_Text_84321ED, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 9 (local id 9, gfx 0x3EEE) at 24,12
Kanto_TwoIsland_EventScript_841740A::
	msgbox Kanto_TwoIsland_Text_8432222, MSGBOX_NPC
	end

@ Kanto_TwoIsland object 8 (local id 8, gfx 0x3F17) at 37,12
Kanto_TwoIsland_EventScript_8417413::
	msgbox Kanto_TwoIsland_Text_8432293, MSGBOX_NPC
	end

@ Kanto_TwoIsland sign / bg event at 29,8
Kanto_TwoIsland_EventScript_841741C::
	msgbox Kanto_TwoIsland_Text_8431D4D, MSGBOX_SIGN
	end

@ Kanto_TwoIsland sign / bg event at 40,10
Kanto_TwoIsland_EventScript_8417425::
	msgbox Kanto_TwoIsland_Text_8431D6C, MSGBOX_SIGN
	end

@ Kanto_TwoIsland sign / bg event at 8,3
Kanto_TwoIsland_EventScript_841742E::
	msgbox Kanto_TwoIsland_Text_8431D8D, MSGBOX_SIGN
	end

@ Kanto_TwoIsland object 7 (local id 7, gfx 0x3F0E) at 39,16
Kanto_TwoIsland_EventScript_846BF95::
	setorcopyvar VAR_0x8000, ITEM_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_TwoIsland_Mart_84173D0:
	.2byte ITEM_AURORA_TICKET
	.2byte ITEM_MYSTIC_TICKET
	.2byte ITEM_NONE

Kanto_TwoIsland_Text_8431C72::
	.string "I’m preparing new voyages.\p"
	.string "Come back after winning KANTO’s\n"
	.string "League and getting the Rainbow Pass.$"

Kanto_TwoIsland_Text_8431CD2::
	.string "We sell tickets for special\n"
	.string "voyages from VERMILION.\p"
	.string "Tickets are reusable. If you have\n"
	.string "one, you don’t need to buy it again.$"

Kanto_TwoIsland_Text_8431D4D::
	.string "TWO ISLAND\n"
	.string "Boon Island for Two$"

Kanto_TwoIsland_Text_8431D6C::
	.string "Skip & Chomp!\n"
	.string "Joyful GAME CORNER$"

Kanto_TwoIsland_Text_8431D8D::
	.string "DANGER! FAST CURRENT!\n"
	.string "Fishing and surfing prohibited!$"

Kanto_TwoIsland_Text_8431FF4::
	.string "That fellow there, he moved here\n"
	.string "from CELADON CITY.\p"
	.string "He started up a shop with his\n"
	.string "younger brother.\p"
	.string "You have to wonder if he knew what\n"
	.string "he was getting into.$"

Kanto_TwoIsland_Text_843208F::
	.string "The shopkeeper’s brother took\n"
	.string "a job at the GAME CORNER.\p"
	.string "He’s trying to pull together money\n"
	.string "so they can buy more merchandise.$"

Kanto_TwoIsland_Text_843210C::
	.string "Listen, listen.\n"
	.string "Did you know?\p"
	.string "A famous TRAINER came to the\n"
	.string "ISLAND a little while back.\p"
	.string "He’s one of the ELITE FOUR.\n"
	.string "His name is BRUNO.\p"
	.string "He went away disappointed when he\n"
	.string "found out that they were all sold\l"
	.string "out of Rage Candybars.$"

Kanto_TwoIsland_Text_84321ED::
	.string "The discerning TRAINER knows to\n"
	.string "buy rare items here!$"

Kanto_TwoIsland_Text_8432222::
	.string "There’s an old woman who lives in\n"
	.string "a cottage out on the cape.\p"
	.string "She gives the eye to POKéMON.\n"
	.string "It’s a bit unnerving.$"

Kanto_TwoIsland_Text_8432293::
	.string "Boing! Boing!\n"
	.string "Have you tried the jumping game?\p"
	.string "The POKéMON you can enter…\n"
	.string "Boing!\p"
	.string "Some go boing!\p"
	.string "And some go boooooooing!\p"
	.string "Did you know that?\n"
	.string "Boing!$"
