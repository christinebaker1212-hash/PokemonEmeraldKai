Kanto_VermilionCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_VermilionCity_EventScript_841658F
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_VermilionCity_OnFrame_8416570
	.byte 0

Kanto_VermilionCity_OnFrame_8416570:
	map_script_2 VAR_QZ_0x5135, 2, Kanto_VermilionCity_EventScript_841657A
	.2byte 0

@ Kanto_VermilionCity object 13 (local id 13, gfx 0x2E) at 41,10
Kanto_VermilionCity_EventScript_82C0666::
	lock
	faceplayer
	message Kanto_VermilionCity_Text_82C1145
	waitmessage
	pokemart Kanto_VermilionCity_Mart_82C0680
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 12 (local id 12, gfx 0x2D) at 40,10
Kanto_VermilionCity_EventScript_82C06B6::
	lock
	faceplayer
	message Kanto_VermilionCity_Text_82C1185
	waitmessage
	pokemart Kanto_VermilionCity_Mart_82C06D0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 15 (local id 15, gfx 0x1E) at 36,14
Kanto_VermilionCity_EventScript_82C0B00::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Kanto_VermilionCity_Mart_82C0B18
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 17 (local id 17, gfx 0x7) at 40,14
Kanto_VermilionCity_EventScript_82C0B82::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Kanto_VermilionCity_Mart_82C0B9C
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 10 (local id 10, gfx 0xF) at 36,10
Kanto_VermilionCity_EventScript_82C1103::
	lock
	faceplayer
	message Kanto_VermilionCity_Text_82C2F40
	waitmessage
	special QzShowMegaStoneShop
	waitstate
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 11 (local id 11, gfx 0x2C) at 37,10
Kanto_VermilionCity_EventScript_82C1119::
	lock
	faceplayer
	message Kanto_VermilionCity_Text_82C2F7E
	waitmessage
	special QzShowZCrystalShop
	waitstate
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 16 (local id 16, gfx 0x30) at 37,14
Kanto_VermilionCity_EventScript_82C112F::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	special QzShowLegendaryItemShop
	waitstate
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_EventScript_840F931::
	specialvar VAR_RESULT, QzGetReachableRegionsMask
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_EventScript_840F94E
	message Kanto_VermilionCity_Text_840FAC7
	waitmessage
	multichoice 0x10, 2, 0x8D, 1
	nop
	return

Kanto_VermilionCity_EventScript_840F94E::
	setvar VAR_RESULT, 0
	return

@ Kanto_VermilionCity object 9 (local id 9, gfx 0x1B) at 32,10
Kanto_VermilionCity_EventScript_841652D::
	lock
	faceplayer
	message Kanto_VermilionCity_Text_82C1207
	waitmessage
	pokemart Kanto_VermilionCity_Mart_82C071C
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 14 (local id 14, gfx 0x20) at 32,14
Kanto_VermilionCity_EventScript_8416544::
	lock
	faceplayer
	message Kanto_VermilionCity_Text_82C11C6
	waitmessage
	pokemart Kanto_VermilionCity_Mart_82C0798
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_EventScript_841657A::
	lockall
	applymovement LOCALID_PLAYER, Kanto_VermilionCity_Movement_841658C
	waitmovement 0
	setvar VAR_QZ_0x5135, 3
	releaseall
	end

Kanto_VermilionCity_EventScript_841658F::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x2262
	call_if_set FLAG_QZ_0x2219, Kanto_VermilionCity_EventScript_84165A4
	call Quetzal_EventScript_840FBB5
	end

Kanto_VermilionCity_EventScript_84165A4::
	setflag FLAG_QZ_0x21D7
	return

@ Kanto_VermilionCity object 1 (local id 1, gfx 0x3F57) at 22,11
Kanto_VermilionCity_EventScript_84165A8::
	msgbox Kanto_VermilionCity_Text_842F7F7, MSGBOX_NPC
	end

@ Kanto_VermilionCity object 2 (local id 2, gfx 0x3F2C) at 17,9
Kanto_VermilionCity_EventScript_84165B1::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5135, 3, Kanto_VermilionCity_EventScript_84165C8
	msgbox Kanto_VermilionCity_Text_842F84A, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_EventScript_84165C8::
	msgbox Kanto_VermilionCity_Text_842F87A, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity object 4 (local id 4, gfx 0x3F41) at 24,33
Kanto_VermilionCity_EventScript_84165D2::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5135, 3, Kanto_VermilionCity_EventScript_841662D
	msgbox Kanto_VermilionCity_Text_842FB5D, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_EventScript_84165E9::
	goto_if_unset FLAG_QZ_0x2108, Kanto_VermilionCity_EventScript_8457C73
	checkitem ITEM_MYSTIC_TICKET, 1
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_EventScript_8457C73
	setflag FLAG_QZ_0x2132
	goto Kanto_VermilionCity_EventScript_8457C6D

Kanto_VermilionCity_EventScript_841660B::
	goto_if_unset FLAG_QZ_0x2108, Kanto_VermilionCity_EventScript_8457C73
	checkitem ITEM_AURORA_TICKET, 1
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_EventScript_8457C73
	setflag FLAG_QZ_0x2131
	goto Kanto_VermilionCity_EventScript_8457C6D

Kanto_VermilionCity_EventScript_841662D::
	setvar VAR_0x8004, 0
	call Kanto_VermilionCity_EventScript_840F931
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_EventScript_841665C
	goto_if_ne VAR_RESULT, 0, Quetzal_EventScript_8459493
	goto_if_set FLAG_QZ_0x2108, Kanto_VermilionCity_EventScript_8416691
	goto Kanto_VermilionCity_EventScript_8416671

Kanto_VermilionCity_EventScript_841665C::
	call Quetzal_EventScript_840F954
	goto_if_eq VAR_RESULT, 127, Quetzal_EventScript_8459493
	goto Quetzal_EventScript_840FA9D

Kanto_VermilionCity_EventScript_8416671::
	goto_if_ge VAR_QZ_0x5117, 5, Kanto_VermilionCity_EventScript_84166CA
	goto_if_ge VAR_QZ_0x5117, 1, Kanto_VermilionCity_EventScript_841684C
	msgbox Kanto_VermilionCity_Text_842FC89, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_EventScript_8416691::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	call Kanto_VermilionCity_EventScript_84165E9
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_EventScript_84166EB
	call Kanto_VermilionCity_EventScript_841660B
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_EventScript_841674E
	goto Kanto_VermilionCity_EventScript_8416671

Kanto_VermilionCity_EventScript_84166CA::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	setvar VAR_0x8004, 0
	message Kanto_VermilionCity_Text_842FD8C
	waitmessage
	goto Quetzal_EventScript_84593E7

Kanto_VermilionCity_EventScript_84166EB::
	call Kanto_VermilionCity_EventScript_841660B
	goto_if_eq VAR_RESULT, 1, Kanto_VermilionCity_EventScript_84167A1
	call_if_unset FLAG_QZ_0x21F3, Kanto_VermilionCity_EventScript_8416742
	message Kanto_VermilionCity_Text_842FE9D
	waitmessage
	multichoice 0x11, 6, 0x76, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_VermilionCity_EventScript_8416671
	goto_if_eq VAR_0x8000, 1, Kanto_VermilionCity_EventScript_84167FC
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_8459493
	goto_if_eq VAR_0x8000, 127, Quetzal_EventScript_8459493
	end

Kanto_VermilionCity_EventScript_8416742::
	setflag FLAG_QZ_0x21F3
	msgbox Kanto_VermilionCity_Text_842FDDC, MSGBOX_DEFAULT
	return

Kanto_VermilionCity_EventScript_841674E::
	call_if_unset FLAG_QZ_0x21F2, Kanto_VermilionCity_EventScript_8416795
	message Kanto_VermilionCity_Text_842FE9D
	waitmessage
	multichoice 0xD, 6, 0x75, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_VermilionCity_EventScript_8416671
	goto_if_eq VAR_0x8000, 1, Kanto_VermilionCity_EventScript_8416824
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_8459493
	goto_if_eq VAR_0x8000, 127, Quetzal_EventScript_8459493
	end

Kanto_VermilionCity_EventScript_8416795::
	setflag FLAG_QZ_0x21F2
	msgbox Kanto_VermilionCity_Text_842FE3B, MSGBOX_DEFAULT
	return

Kanto_VermilionCity_EventScript_84167A1::
	call_if_unset FLAG_QZ_0x21F3, Kanto_VermilionCity_EventScript_8416742
	call_if_unset FLAG_QZ_0x21F2, Kanto_VermilionCity_EventScript_8416795
	message Kanto_VermilionCity_Text_842FE9D
	waitmessage
	multichoice 0xD, 4, 0x77, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_VermilionCity_EventScript_8416671
	goto_if_eq VAR_0x8000, 1, Kanto_VermilionCity_EventScript_84167FC
	goto_if_eq VAR_0x8000, 2, Kanto_VermilionCity_EventScript_8416824
	goto_if_eq VAR_0x8000, 3, Quetzal_EventScript_8459493
	goto_if_eq VAR_0x8000, 127, Quetzal_EventScript_8459493
	end

Kanto_VermilionCity_EventScript_84167FC::
	call Kanto_VermilionCity_EventScript_84165E9
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8459493
	msgbox Kanto_VermilionCity_Text_842FEC8, MSGBOX_DEFAULT
	setvar VAR_0x8004, 0
	setvar VAR_0x8006, 9
	goto Quetzal_EventScript_8459475

Kanto_VermilionCity_EventScript_8416824::
	call Kanto_VermilionCity_EventScript_841660B
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8459493
	msgbox Kanto_VermilionCity_Text_842FF3B, MSGBOX_DEFAULT
	setvar VAR_0x8004, 0
	setvar VAR_0x8006, 0xA
	goto Quetzal_EventScript_8459475

Kanto_VermilionCity_EventScript_841684C::
	message Kanto_VermilionCity_Text_842FCEF
	waitmessage
	setvar VAR_0x8004, 0
	goto Quetzal_EventScript_84593E7

@ Kanto_VermilionCity trigger at 22,33 (VAR_QZ_0x5145 == 0)
Kanto_VermilionCity_EventScript_841685F::
	lockall
	goto Kanto_VermilionCity_EventScript_8416875

@ Kanto_VermilionCity trigger at 23,33 (VAR_QZ_0x5145 == 0)
Kanto_VermilionCity_EventScript_8416866::
	lockall
	goto Kanto_VermilionCity_EventScript_8416875

@ Kanto_VermilionCity trigger at 22,32 (VAR_TEMP_1 == 0)
@ Kanto_VermilionCity trigger at 23,32 (VAR_TEMP_1 == 0)
Kanto_VermilionCity_EventScript_841686D::
	lockall
	setvar VAR_QZ_0x5145, 0
	releaseall
	end

Kanto_VermilionCity_EventScript_8416875::
	textcolor 0
	applymovement 4, Quetzal_Movement_8457AAA
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	msgbox Kanto_VermilionCity_Text_842FBC1, MSGBOX_DEFAULT
	goto_if_unset FLAG_QZ_0x2162, Kanto_VermilionCity_EventScript_84168A8
	msgbox Kanto_VermilionCity_Text_842FBFC, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5145, 1
	releaseall
	end

Kanto_VermilionCity_EventScript_84168A8::
	msgbox Kanto_VermilionCity_Text_842FC39, MSGBOX_DEFAULT
	closemessage
	goto Quetzal_EventScript_84594A5

@ Kanto_VermilionCity object 3 (local id 3, gfx 0x3F41) at 31,30
Kanto_VermilionCity_EventScript_84168B7::
	msgbox Kanto_VermilionCity_Text_842F96D, MSGBOX_NPC
	end

@ Kanto_VermilionCity object 6 (local id 6, gfx 0x3F43) at 25,7
Kanto_VermilionCity_EventScript_84168C0::
	lock
	faceplayer
	setflag FLAG_QZ_0x2219
	msgbox Kanto_VermilionCity_Text_842FFAE, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity sign / bg event at 33,6
Kanto_VermilionCity_EventScript_84168F3::
	msgbox Kanto_VermilionCity_Text_842F9D9, MSGBOX_SIGN
	end

@ Kanto_VermilionCity sign / bg event at 45,17
Kanto_VermilionCity_EventScript_84168FC::
	msgbox Kanto_VermilionCity_Text_842FA06, MSGBOX_SIGN
	end

@ Kanto_VermilionCity sign / bg event at 10,17
Kanto_VermilionCity_EventScript_8416905::
	msgbox Kanto_VermilionCity_Text_842FA7F, MSGBOX_SIGN
	end

@ Kanto_VermilionCity sign / bg event at 10,24
Kanto_VermilionCity_EventScript_841690E::
	lockall
	setvar VAR_0x8004, 4
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	msgbox Kanto_VermilionCity_Text_842FAAA, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_VermilionCity sign / bg event at 34,18
Kanto_VermilionCity_EventScript_8416926::
	msgbox Kanto_VermilionCity_Text_842FAEF, MSGBOX_SIGN
	end

Kanto_VermilionCity_EventScript_8457C6D::
	setvar VAR_RESULT, 1
	return

Kanto_VermilionCity_EventScript_8457C73::
	setvar VAR_RESULT, 0
	return

Kanto_VermilionCity_Mart_82C0680:
	.2byte ITEM_HP_UP
	.2byte ITEM_PROTEIN
	.2byte ITEM_IRON
	.2byte ITEM_CALCIUM
	.2byte ITEM_ZINC
	.2byte ITEM_CARBOS
	.2byte ITEM_POMEG_BERRY
	.2byte ITEM_KELPSY_BERRY
	.2byte ITEM_QUALOT_BERRY
	.2byte ITEM_HONDEW_BERRY
	.2byte ITEM_GREPA_BERRY
	.2byte ITEM_TAMATO_BERRY
	.2byte ITEM_NONE

Kanto_VermilionCity_Mart_82C06D0:
	.2byte ITEM_MACHO_BRACE
	.2byte ITEM_POWER_WEIGHT
	.2byte ITEM_POWER_BRACER
	.2byte ITEM_POWER_BELT
	.2byte ITEM_POWER_LENS
	.2byte ITEM_POWER_BAND
	.2byte ITEM_POWER_ANKLET
	.2byte ITEM_DESTINY_KNOT
	.2byte ITEM_NONE

Kanto_VermilionCity_Mart_82C071C:
	.2byte ITEM_NONE

Kanto_VermilionCity_Mart_82C0798:
	.2byte ITEM_NONE

Kanto_VermilionCity_Mart_82C0B18:
	.2byte ITEM_WATER_STONE
	.2byte ITEM_THUNDER_STONE
	.2byte ITEM_FIRE_STONE
	.2byte ITEM_LEAF_STONE
	.2byte ITEM_SUN_STONE
	.2byte ITEM_MOON_STONE
	.2byte ITEM_SHINY_STONE
	.2byte ITEM_DUSK_STONE
	.2byte ITEM_DAWN_STONE
	.2byte ITEM_DRAGON_SCALE
	.2byte ITEM_UPGRADE
	.2byte ITEM_KINGS_ROCK
	.2byte ITEM_METAL_COAT
	.2byte ITEM_DEEP_SEA_SCALE
	.2byte ITEM_DEEP_SEA_TOOTH
	.2byte ITEM_OVAL_STONE
	.2byte ITEM_PROTECTOR
	.2byte ITEM_ELECTIRIZER
	.2byte ITEM_MAGMARIZER
	.2byte ITEM_DUBIOUS_DISC
	.2byte ITEM_REAPER_CLOTH
	.2byte ITEM_RAZOR_CLAW
	.2byte ITEM_RAZOR_FANG
	.2byte ITEM_NONE

Kanto_VermilionCity_Mart_82C0B9C:
	.2byte ITEM_RARE_CANDY
	.2byte ITEM_NONE

Kanto_VermilionCity_Movement_841658C:
	walk_up
	walk_up
	step_end

Kanto_VermilionCity_Text_82C1145::
	.string "I’m the ENERGY GURU! I’ll go for it and\n"
	.string "sell you what you need!$"

Kanto_VermilionCity_Text_82C1185::
	.string "I’m the EFFORT WOMAN! I’ll go for it and\n"
	.string "sell you what you need!$"

Kanto_VermilionCity_Text_82C11C6::
	.string "I’m the NATURE WOMAN! I’ll go for it and\n"
	.string "sell you what you need!$"

Kanto_VermilionCity_Text_82C1207::
	.string "I’m the GIGA COOK! I’ll go for it and\n"
	.string "sell you what you need!$"

Kanto_VermilionCity_Text_82C2F40::
	.string "I’m the MEGA GURU! I’ll go for it and\n"
	.string "sell you what you need!$"

Kanto_VermilionCity_Text_82C2F7E::
	.string "I’m the Z-MOVE GURU! I’ll go for it and\n"
	.string "sell you what you need!$"

Kanto_VermilionCity_Text_840FAC7::
	.string "Which service would you like?$"

Kanto_VermilionCity_Text_842F7F7::
	.string "We’re careful about pollution here.\p"
	.string "We’ve heard GRIMER multiplies in\n"
	.string "toxic sludge.$"

Kanto_VermilionCity_Text_842F84A::
	.string "Did you see the S.S. ANNE moored\n"
	.string "in the harbor?$"

Kanto_VermilionCity_Text_842F87A::
	.string "The S.S. ANNE returns to this pier.\n"
	.string "You can visit with your TICKET.$"

Kanto_VermilionCity_Text_842F96D::
	.string "The S.S. ANNE is a famous luxury\n"
	.string "cruise ship.\p"
	.string "Even after its voyages, you can\n"
	.string "always come back for a visit.$"

Kanto_VermilionCity_Text_842F9D9::
	.string "VERMILION CITY\n"
	.string "The Port of Exquisite Sunsets$"

Kanto_VermilionCity_Text_842FA06::
	.string "NOTICE!\p"
	.string "ROUTE 12 may be blocked off by a\n"
	.string "sleeping POKéMON.\p"
	.string "Detour through ROCK TUNNEL to\n"
	.string "LAVENDER TOWN.\p"
	.string "VERMILION POLICE$"

Kanto_VermilionCity_Text_842FA7F::
	.string "POKéMON FAN CLUB\n"
	.string "All POKéMON fans welcome!$"

Kanto_VermilionCity_Text_842FAAA::
	.string "VERMILION CITY POKéMON GYM\n"
	.string "LEADER: LT. SURGE\l"
	.string "The Lightning American!$"

Kanto_VermilionCity_Text_842FAEF::
	.string "VERMILION HARBOR\n"
	.string "S.S. ANNE: at the end of the pier.\p"
	.string "Island ferry: speak to the sailor\n"
	.string "at the harbor entrance.$"

Kanto_VermilionCity_Text_842FB5D::
	.string "The S.S. ANNE is at the end of the pier.\p"
	.string "To travel to the islands, talk to me\n"
	.string "once you have a pass.$"

Kanto_VermilionCity_Text_842FBC1::
	.string "Welcome to the S.S. ANNE!\p"
	.string "Excuse me, do you have a ticket?$"

Kanto_VermilionCity_Text_842FBFC::
	.string "{PLAYER} flashed the S.S. TICKET!\p"
	.string "Great!\n"
	.string "Welcome to the S.S. ANNE!$"

Kanto_VermilionCity_Text_842FC39::
	.string "{PLAYER} doesn’t have the needed\n"
	.string "S.S. TICKET.\p"
	.string "Sorry!\p"
	.string "You need a ticket to get aboard.$"

Kanto_VermilionCity_Text_842FC89::
	.string "There is no ferry available yet.\p"
	.string "You can visit the S.S. ANNE at the\n"
	.string "end of the pier with your TICKET.$"

Kanto_VermilionCity_Text_842FCEF::
	.string "Ah, you have a TRI-PASS.\p"
	.string "Choose a destination (up/down).\n"
	.string "A: Sail  B: Cancel$"

Kanto_VermilionCity_Text_842FD8C::
	.string "Ah, you have a RAINBOW PASS.\p"
	.string "Choose a destination (up/down).\n"
	.string "A: Sail  B: Cancel$"

Kanto_VermilionCity_Text_842FDDC::
	.string "Oh! That’s a MYSTICTICKET!\n"
	.string "Now that is rare.\p"
	.string "We’ll be happy to take you to\n"
	.string "NAVEL ROCK anytime.$"

Kanto_VermilionCity_Text_842FE3B::
	.string "Oh! That’s an AURORATICKET!\n"
	.string "Now that is rare.\p"
	.string "We’ll be happy to take you to\n"
	.string "BIRTH ISLAND anytime.$"

Kanto_VermilionCity_Text_842FE9D::
	.string "Would you like to board\n"
	.string "a SEAGALLOP ferry?$"

Kanto_VermilionCity_Text_842FEC8::
	.string "Okay, everything’s in order for you\n"
	.string "to board a special ferry.\p"
	.string "SEAGALLOP HI-SPEED 10 will be\n"
	.string "departing immediately.$"

Kanto_VermilionCity_Text_842FF3B::
	.string "Okay, everything’s in order for you\n"
	.string "to board a special ferry.\p"
	.string "SEAGALLOP HI-SPEED 12 will be\n"
	.string "departing immediately.$"

Kanto_VermilionCity_Text_842FFAE::
	.string "Oh, hello, {PLAYER}!\n"
	.string "How are you doing?\p"
	.string "It’s me, one of PROF. OAK’s AIDES.\p"
	.string "Did you meet the other AIDE?\p"
	.string "He had a package from PROF. OAK\n"
	.string "for you, {PLAYER}.\p"
	.string "He said he’d look for you around\n"
	.string "ROUTE 2, {PLAYER}.\p"
	.string "If you’re in the ROUTE 2 area,\n"
	.string "please look for him.$"
