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
	special QzShowMegaStoneShop
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
	special QzShowZCrystalShop
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
	.2byte ITEM_NONE

Johto_GoldenrodDeptStore5F_Text_83D445B::
	.string "During the day, a lady comes to check\n"
	.string "out MOéMON.\p"
	.string "She even gives away TMs!$"

Johto_GoldenrodDeptStore5F_Text_83D44A7::
	.string "You can’t rename a MOéMON you get\n"
	.string "in a trade.\p"
	.string "The name is a reflection of the\n"
	.string "original trainer’s feelings for it.$"

Johto_GoldenrodDeptStore5F_Text_83D4553::
	.string "Mega Stones, Z-Moves and\n"
	.string "Tera Shards.\p"
	.string "5F MEGA, Z & TERA$"

@ Moemon Kai: Crystal's Sunday receptionist (pokecrystal GoldenrodDeptStore5F.asm ReceptionistScript, docs/REGIONS_AUDIT.md
@ 143): she rates the lead's friendship once a day (Sundays only) and gives TM27 RETURN or TM21 FRUSTRATION
Johto_GoldenrodDeptStore5F_EventScript_SundayLady::
	lock
	faceplayer
	specialvar VAR_RESULT, MP_JohtoWeekday
	goto_if_ne VAR_RESULT, 0, Johto_GoldenrodDeptStore5F_EventScript_SundayLadyOver
	goto_if_set FLAG_DAILY_JOHTO_DEPT_TM, Johto_GoldenrodDeptStore5F_EventScript_SundayLadyOver
	specialvar VAR_0x8004, MP_LeadFriendship
	msgbox Johto_GoldenrodDeptStore5F_Text_SundayLadyLook, MSGBOX_DEFAULT
	goto_if_ge VAR_0x8004, 150, Johto_GoldenrodDeptStore5F_EventScript_SundayLadyHappy
	goto_if_ge VAR_0x8004, 50, Johto_GoldenrodDeptStore5F_EventScript_SundayLadyAdorable
	msgbox Johto_GoldenrodDeptStore5F_Text_SundayLadyEvil, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_TM21
	goto Johto_GoldenrodDeptStore5F_EventScript_SundayLadyGive
Johto_GoldenrodDeptStore5F_EventScript_SundayLadyHappy::
	msgbox Johto_GoldenrodDeptStore5F_Text_SundayLadyPerfect, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_TM27
Johto_GoldenrodDeptStore5F_EventScript_SundayLadyGive::
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodDeptStore5F_EventScript_SundayLadyEnd
	setflag FLAG_DAILY_JOHTO_DEPT_TM
	release
	end
Johto_GoldenrodDeptStore5F_EventScript_SundayLadyAdorable::
	msgbox Johto_GoldenrodDeptStore5F_Text_SundayLadyAdorable, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodDeptStore5F_EventScript_SundayLadyOver::
	msgbox Johto_GoldenrodDeptStore5F_Text_SundayLadyOver, MSGBOX_DEFAULT
Johto_GoldenrodDeptStore5F_EventScript_SundayLadyEnd::
	release
	end

Johto_GoldenrodDeptStore5F_Text_SundayLadyLook::
	.string "Hello. Oh, your MOéMON…$"

Johto_GoldenrodDeptStore5F_Text_SundayLadyPerfect::
	.string "It’s very attached to you.\p"
	.string "This move should be perfect for a\n"
	.string "pair like you.$"

Johto_GoldenrodDeptStore5F_Text_SundayLadyAdorable::
	.string "It’s adorable!\p"
	.string "You should teach it good TM moves.$"

Johto_GoldenrodDeptStore5F_Text_SundayLadyEvil::
	.string "It looks evil. How about this TM\n"
	.string "for it?$"

Johto_GoldenrodDeptStore5F_Text_SundayLadyOver::
	.string "There are sure to be TMs that are\n"
	.string "just perfect for your MOéMON.$"
