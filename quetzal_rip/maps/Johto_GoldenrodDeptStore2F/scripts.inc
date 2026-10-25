Johto_GoldenrodDeptStore2F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_GoldenrodDeptStore2F_OnFrame_83D3830
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_GoldenrodDeptStore2F_OnWarp_83D383A
	.byte 0

Johto_GoldenrodDeptStore2F_OnFrame_83D3830:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D384A
	.2byte 0

Johto_GoldenrodDeptStore2F_OnWarp_83D383A:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D3844
	.2byte 0

@ Johto_GoldenrodDeptStore2F object 4 (local id 4, gfx 0x10) at 20,9
Johto_GoldenrodDeptStore2F_EventScript_83D3BAA::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore2F_Text_83D3C24, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore2F sign / bg event at 24,4
Johto_GoldenrodDeptStore2F_EventScript_83D3BC4::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore2F_Text_83D3CCE, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStore2F object 1 (local id 1, gfx 0x3E8E) at 14,11
Johto_GoldenrodDeptStore2F_EventScript_83D3D09::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore2F object 2 (local id 2, gfx 0x18) at 16,11
Johto_GoldenrodDeptStore2F_EventScript_83D3D20::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodDeptStore2F_Mart_83D3D38
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore2F object 3 (local id 3, gfx 0x11) at 12,11
Johto_GoldenrodDeptStore2F_EventScript_83D3D4A::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodDeptStore2F_Mart_83D3D64
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodDeptStore2F_Mart_83D3D38:
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_SP_ATK
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_X_SP_DEF
	.2byte ITEM_DIRE_HIT
	.2byte ITEM_GUARD_SPEC
	.2byte ITEM_X_ACCURACY
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore2F_Mart_83D3D64:
	.2byte ITEM_IV_UP
	.2byte ITEM_IV_MAX
	.2byte ITEM_EXP_CANDY_XS
	.2byte ITEM_EXP_CANDY_S
	.2byte ITEM_EXP_CANDY_M
	.2byte ITEM_EXP_CANDY_L
	.2byte ITEM_EXP_CANDY_XL
	.2byte ITEM_RARE_CANDY
	.2byte ITEM_MAX_CANDY
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore2F_Text_83D3C24::
	.string "I got my ABRA at the GAME CORNER.\p"
	.string "Now it’s my best partner.$"

Johto_GoldenrodDeptStore2F_Text_83D3CCE::
	.string "Items, battle boosts and\n"
	.string "upgrades.\p"
	.string "2F ITEMS & BATTLE TOOLS$"
