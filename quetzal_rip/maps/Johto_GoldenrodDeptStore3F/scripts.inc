Johto_GoldenrodDeptStore3F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_GoldenrodDeptStore3F_OnFrame_83D3830
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_GoldenrodDeptStore3F_OnWarp_83D383A
	.byte 0

Johto_GoldenrodDeptStore3F_OnFrame_83D3830:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D384A
	.2byte 0

Johto_GoldenrodDeptStore3F_OnWarp_83D383A:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D3844
	.2byte 0

@ Johto_GoldenrodDeptStore3F object 1 (local id 1, gfx 0x19) at 16,5
Johto_GoldenrodDeptStore3F_EventScript_83D3D78::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodDeptStore3F_Mart_83D3D90
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore3F object 2 (local id 2, gfx 0x22) at 15,5
Johto_GoldenrodDeptStore3F_EventScript_83D3DC4::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodDeptStore3F_Mart_83D3DDC
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore3F object 3 (local id 3, gfx 0x8) at 13,5
Johto_GoldenrodDeptStore3F_EventScript_83D3DEE::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodDeptStore3F_Mart_83D3E08
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodDeptStore3F sign / bg event at 23,4
Johto_GoldenrodDeptStore3F_EventScript_83D3F5A::
	lock
	faceplayer
	msgbox Johto_GoldenrodDeptStore3F_Text_83D401D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodDeptStore3F_Mart_83D3D90:
	.2byte ITEM_HP_UP
	.2byte ITEM_PROTEIN
	.2byte ITEM_IRON
	.2byte ITEM_CALCIUM
	.2byte ITEM_ZINC
	.2byte ITEM_CARBOS
	.2byte ITEM_HEALTH_MOCHI
	.2byte ITEM_MUSCLE_MOCHI
	.2byte ITEM_RESIST_MOCHI
	.2byte ITEM_GENIUS_MOCHI
	.2byte ITEM_CLEVER_MOCHI
	.2byte ITEM_SWIFT_MOCHI
	.2byte ITEM_FRESH_START_MOCHI
	.2byte ITEM_HEALTH_FEATHER
	.2byte ITEM_MUSCLE_FEATHER
	.2byte ITEM_RESIST_FEATHER
	.2byte ITEM_GENIUS_FEATHER
	.2byte ITEM_CLEVER_FEATHER
	.2byte ITEM_SWIFT_FEATHER
	.2byte ITEM_POMEG_BERRY
	.2byte ITEM_KELPSY_BERRY
	.2byte ITEM_QUALOT_BERRY
	.2byte ITEM_HONDEW_BERRY
	.2byte ITEM_GREPA_BERRY
	.2byte ITEM_TAMATO_BERRY
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore3F_Mart_83D3DDC:
	.2byte ITEM_MACHO_BRACE
	.2byte ITEM_POWER_WEIGHT
	.2byte ITEM_POWER_BRACER
	.2byte ITEM_POWER_BELT
	.2byte ITEM_POWER_LENS
	.2byte ITEM_POWER_BAND
	.2byte ITEM_POWER_ANKLET
	.2byte ITEM_DESTINY_KNOT
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore3F_Mart_83D3E08:
	.2byte ITEM_GENDER_PILL
	.2byte ITEM_ABILITY_CAPSULE
	.2byte ITEM_ABILITY_PATCH
	.2byte ITEM_LONELY_MINT
	.2byte ITEM_ADAMANT_MINT
	.2byte ITEM_NAUGHTY_MINT
	.2byte ITEM_BRAVE_MINT
	.2byte ITEM_BOLD_MINT
	.2byte ITEM_IMPISH_MINT
	.2byte ITEM_LAX_MINT
	.2byte ITEM_RELAXED_MINT
	.2byte ITEM_MODEST_MINT
	.2byte ITEM_MILD_MINT
	.2byte ITEM_RASH_MINT
	.2byte ITEM_QUIET_MINT
	.2byte ITEM_CALM_MINT
	.2byte ITEM_GENTLE_MINT
	.2byte ITEM_CAREFUL_MINT
	.2byte ITEM_SASSY_MINT
	.2byte ITEM_TIMID_MINT
	.2byte ITEM_HASTY_MINT
	.2byte ITEM_JOLLY_MINT
	.2byte ITEM_NAIVE_MINT
	.2byte ITEM_SERIOUS_MINT
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore3F_Text_83D401D::
	.string "Vitamins, Power Items and Mints.\p"
	.string "3F TRAINING & MINTS$"
