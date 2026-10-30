Kanto_ThreeIsland_BerryForest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ThreeIsland_BerryForest_EventScript_8413A05
	.byte 0

Kanto_ThreeIsland_BerryForest_EventScript_8413A05::
	setflag FLAG_QZ_0x225C
	setescapewarp MAP_KANTO_THREE_ISLAND_BOND_BRIDGE, 255, 12, 6
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_ThreeIsland_BerryForest object 1 (local id 1, gfx 0x3F18) at 4,8
Kanto_ThreeIsland_BerryForest_EventScript_8413A16::
	lock
	faceplayer
	msgbox Kanto_ThreeIsland_BerryForest_Text_842B4DF, MSGBOX_DEFAULT
	playse SE_PIN
	applymovement 1, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 1, Kanto_ThreeIsland_BerryForest_Movement_8413ABC
	waitmovement 0
	msgbox Kanto_ThreeIsland_BerryForest_Text_842B597, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	waitse
	playmoncry SPECIES_HYPNO, 2
	waitmoncry
	setwildbattle SPECIES_HYPNO, 0x1E, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	dowildbattle
	special QzQuestLogNop
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_ThreeIsland_BerryForest_Text_842B5EA, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_IAPAPA_BERRY
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	call_if_eq VAR_RESULT, 0, Kanto_ThreeIsland_BerryForest_EventScript_8413AB3
	special GetPlayerBigGuyGirlString
	msgbox Kanto_ThreeIsland_BerryForest_Text_842B644, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x21EC
	setflag FLAG_QZ_0x2199
	setvar VAR_QZ_0x5134, 2
	clearflag FLAG_QZ_0x21D4
	warp MAP_KANTO_TWO_ISLAND_JOYFUL_GAME_CORNER, 255, 6, 6
	waitstate
	release
	end

Kanto_ThreeIsland_BerryForest_EventScript_8413AB3::
	msgbox Kanto_ThreeIsland_BerryForest_Text_842B6A8, MSGBOX_DEFAULT
	return

@ Kanto_ThreeIsland_BerryForest sign / bg event at 39,35
Kanto_ThreeIsland_BerryForest_EventScript_8413AC2::
	msgbox Kanto_ThreeIsland_BerryForest_Text_842B6DF, MSGBOX_SIGN
	end

@ Kanto_ThreeIsland_BerryForest sign / bg event at 7,27
Kanto_ThreeIsland_BerryForest_EventScript_8413ACB::
	msgbox Kanto_ThreeIsland_BerryForest_Text_842B732, MSGBOX_SIGN
	end

@ Kanto_ThreeIsland_BerryForest object 12 (local id 12, gfx 0x3F0E) at 12,5
Kanto_ThreeIsland_BerryForest_EventScript_846C1EB::
	setorcopyvar VAR_0x8000, ITEM_MAX_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_ThreeIsland_BerryForest object 13 (local id 13, gfx 0x3F0E) at 27,11
Kanto_ThreeIsland_BerryForest_EventScript_846C1F8::
	setorcopyvar VAR_0x8000, ITEM_FULL_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_ThreeIsland_BerryForest object 14 (local id 14, gfx 0x3F0E) at 12,43
Kanto_ThreeIsland_BerryForest_EventScript_846C205::
	setorcopyvar VAR_0x8000, ITEM_MAX_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_ThreeIsland_BerryForest_Movement_8413ABC:
	walk_in_place_faster_left
	delay_16
	walk_in_place_faster_right
	delay_8
	walk_in_place_faster_left
	step_end

Kanto_ThreeIsland_BerryForest_Text_842B4DF::
	.string "LOSTELLE: Whimper… Sniff…\n"
	.string "Oh! Please, help!\p"
	.string "A scary POKéMON appeared there\n"
	.string "a little while ago.\p"
	.string "It kept scaring.\n"
	.string "It made LOSTELLE scared.\p"
	.string "I’m too scared to move!\n"
	.string "But I want to go home…$"

Kanto_ThreeIsland_BerryForest_Text_842B597::
	.string "Oh! Here it comes again!\n"
	.string "No! Go away! It’s scaring me!\p"
	.string "Waaaaaaah!\n"
	.string "I want my daddy!$"

Kanto_ThreeIsland_BerryForest_Text_842B5EA::
	.string "Ohh! That was so scary!\n"
	.string "Thank you!\p"
	.string "LOSTELLE came to pick some\n"
	.string "BERRIES.\p"
	.string "You can have this!$"

Kanto_ThreeIsland_BerryForest_Text_842B644::
	.string "What’s your name?\p"
	.string "LOSTELLE’s scared, so can I go\n"
	.string "with you to my daddy’s house?\p"
	.string "Okay!\n"
	.string "Let’s go home!$"

Kanto_ThreeIsland_BerryForest_Text_842B6A8::
	.string "Your BERRY POUCH is full.\n"
	.string "I guess you don’t want this.$"

Kanto_ThreeIsland_BerryForest_Text_842B6DF::
	.string "Welcome to the BERRY FOREST\p"
	.string "Be friendly and share BERRIES with\n"
	.string "others and POKéMON.$"

Kanto_ThreeIsland_BerryForest_Text_842B732::
	.string "Beware of wild, BERRY-loving\n"
	.string "POKéMON!$"
