Johto_GoldenrodDeptStore4F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_GoldenrodDeptStore4F_OnFrame_83D3830
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_GoldenrodDeptStore4F_OnWarp_83D383A
	.byte 0

Johto_GoldenrodDeptStore4F_OnFrame_83D3830:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D384A
	.2byte 0

Johto_GoldenrodDeptStore4F_OnWarp_83D383A:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D3844
	.2byte 0

@ Johto_GoldenrodDeptStore4F object 1 (local id 1, gfx 0x53) at 24,11
Johto_GoldenrodDeptStore4F_EventScript_83D3E3A::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodDeptStore4F_Mart_83D3E54
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore4F object 4 (local id 4, gfx 0x30) at 22,11
Johto_GoldenrodDeptStore4F_EventScript_83D3F00::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	special QzShowLegendaryItemShop_571
	waitstate
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore4F object 2 (local id 2, gfx 0x3EBF) at 13,6
Johto_GoldenrodDeptStore4F_EventScript_83D4074::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore4F_Text_83D40A8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore4F object 3 (local id 3, gfx 0x3E96) at 11,5
Johto_GoldenrodDeptStore4F_EventScript_83D408E::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore4F_Text_83D4117, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore4F sign / bg event at 24,4
Johto_GoldenrodDeptStore4F_EventScript_83D409B::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore4F_Text_83D4204, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodDeptStore4F_Mart_83D3E54:
	.2byte ITEM_WATER_STONE
	.2byte ITEM_THUNDER_STONE
	.2byte ITEM_FIRE_STONE
	.2byte ITEM_LEAF_STONE
	.2byte ITEM_SUN_STONE
	.2byte ITEM_MOON_STONE
	.2byte ITEM_SHINY_STONE
	.2byte ITEM_DUSK_STONE
	.2byte ITEM_DAWN_STONE
	.2byte ITEM_ICE_STONE
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
	.2byte ITEM_PRISM_SCALE
	.2byte ITEM_WHIPPED_DREAM
	.2byte ITEM_SACHET
	.2byte ITEM_SWEET_APPLE
	.2byte ITEM_TART_APPLE
	.2byte ITEM_SYRUPY_APPLE
	.2byte ITEM_CRACKED_POT
	.2byte ITEM_CHIPPED_POT
	.2byte ITEM_UNREMARKABLE_TEACUP
	.2byte ITEM_MASTERPIECE_TEACUP
	.2byte ITEM_GALARICA_CUFF
	.2byte ITEM_GALARICA_WREATH
	.2byte ITEM_STRAWBERRY_SWEET
	.2byte ITEM_BERRY_SWEET
	.2byte ITEM_LOVE_SWEET
	.2byte ITEM_STAR_SWEET
	.2byte ITEM_CLOVER_SWEET
	.2byte ITEM_FLOWER_SWEET
	.2byte ITEM_RIBBON_SWEET
	.2byte ITEM_LINKING_CORD
	.2byte ITEM_BLACK_AUGURITE
	.2byte ITEM_PEAT_BLOCK
	.2byte ITEM_AUSPICIOUS_ARMOR
	.2byte ITEM_MALICIOUS_ARMOR
	.2byte ITEM_LEADERS_CREST
	.2byte ITEM_GIMMIGHOUL_COIN
	.2byte ITEM_METAL_ALLOY
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore4F_Text_83D40A8::
	.string "Hey. I love strong POKéMON.\p"
	.string "I feed them PROTEIN to crank up\n"
	.string "their ATTACK.$"

Johto_GoldenrodDeptStore4F_Text_83D4117::
	.string "There were rumours that some\n"
	.string "POKéMON evolve only by\l"
	.string "being traded via a Game Link\l"
	.string "cable.\p"
	.string "Pfft...\p"
	.string "Such misinformation.\p"
	.string "I know some of the rumoured\n"
	.string "POKéMON:\l"
	.string "MACHOKE, KADABRA, HAUNTER and,\l"
	.string "um, GRAVELER,\l"
	.string "but they all evolve through\l"
	.string "level up.$"

Johto_GoldenrodDeptStore4F_Text_83D4204::
	.string "Evolution and form-change items.\p"
	.string "4F EVOLUTION & FORMS$"
