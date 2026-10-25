Johto_GoldenrodDeptStore5F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_GoldenrodDeptStore5F_OnFrame_83D3830
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_GoldenrodDeptStore5F_OnWarp_83D383A
	.byte 0

Johto_GoldenrodDeptStore5F_OnFrame_83D3830:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D384A
	.2byte 0

Johto_GoldenrodDeptStore5F_OnWarp_83D383A:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D3844
	.2byte 0

@ Johto_GoldenrodDeptStore5F object 5 (local id 5, gfx 0x1B) at 14,11
Johto_GoldenrodDeptStore5F_EventScript_83D3EBC::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodDeptStore5F_Mart_83D3ED4
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore5F object 1 (local id 1, gfx 0x15) at 17,11
Johto_GoldenrodDeptStore5F_EventScript_83D3F16::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	special QzShowMegaStoneShop_569
	waitstate
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore5F object 2 (local id 2, gfx 0x16) at 16,11
Johto_GoldenrodDeptStore5F_EventScript_83D3F2C::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	special QzShowZCrystalShop_570
	waitstate
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore5F object 3 (local id 3, gfx 0x2F) at 13,5
Johto_GoldenrodDeptStore5F_EventScript_83D4245::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore5F_Text_83D445B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore5F object 4 (local id 4, gfx 0x37) at 23,10
Johto_GoldenrodDeptStore5F_EventScript_83D4252::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore5F_Text_83D44A7, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore5F sign / bg event at 23,4
Johto_GoldenrodDeptStore5F_EventScript_83D426C::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore5F_Text_83D4553, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodDeptStore5F_Mart_83D3ED4:
	.2byte ITEM_GIGA_CANDY
	.2byte ITEM_RESET_TERA_SHARD
	.2byte ITEM_NORMAL_TERA_SHARD
	.2byte ITEM_FIGHTING_TERA_SHARD
	.2byte ITEM_FLYING_TERA_SHARD
	.2byte ITEM_POISON_TERA_SHARD
	.2byte ITEM_GROUND_TERA_SHARD
	.2byte ITEM_ROCK_TERA_SHARD
	.2byte ITEM_BUG_TERA_SHARD
	.2byte ITEM_GHOST_TERA_SHARD
	.2byte ITEM_STEEL_TERA_SHARD
	.2byte ITEM_FIRE_TERA_SHARD
	.2byte ITEM_WATER_TERA_SHARD
	.2byte ITEM_GRASS_TERA_SHARD
	.2byte ITEM_ELECTRIC_TERA_SHARD
	.2byte ITEM_PSYCHIC_TERA_SHARD
	.2byte ITEM_ICE_TERA_SHARD
	.2byte ITEM_DRAGON_TERA_SHARD
	.2byte ITEM_DARK_TERA_SHARD
	.2byte ITEM_FAIRY_TERA_SHARD
	.2byte ITEM_STELLAR_TERA_SHARD
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore5F_Text_83D445B::
	.string "During the day, a lady comes to check\n"
	.string "out POKéMON.\p"
	.string "She even gives away TMs!$"

Johto_GoldenrodDeptStore5F_Text_83D44A7::
	.string "You can’t rename a POKéMON you get\n"
	.string "in a trade.\p"
	.string "The name is a reflection of the\n"
	.string "original trainer’s feelings for it.$"

Johto_GoldenrodDeptStore5F_Text_83D4553::
	.string "Mega Stones, Z-Moves and\n"
	.string "Tera Shards.\p"
	.string "5F MEGA, Z & TERA$"
