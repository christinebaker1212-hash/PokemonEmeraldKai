Johto_GoldenrodDeptStoreB1F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_GoldenrodDeptStoreB1F_OnFrame_83D3830
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_GoldenrodDeptStoreB1F_OnWarp_83D383A
	map_script MAP_SCRIPT_ON_LOAD, Johto_GoldenrodDeptStoreB1F_EventScript_83D351B
	.byte 0

Johto_GoldenrodDeptStoreB1F_OnFrame_83D3830:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D384A
	.2byte 0

Johto_GoldenrodDeptStoreB1F_OnWarp_83D383A:
	map_script_2 VAR_QZ_0x5015, 1, Quetzal_EventScript_83D3844
	.2byte 0

Johto_GoldenrodDeptStoreB1F_EventScript_83D351B::
	call_if_set FLAG_QZ_0x103C, Johto_GoldenrodDeptStoreB1F_EventScript_83D3537
	call_if_set FLAG_QZ_0x103D, Johto_GoldenrodDeptStoreB1F_EventScript_83D354A
	call_if_set FLAG_QZ_0x103E, Johto_GoldenrodDeptStoreB1F_EventScript_83D355D
	end

Johto_GoldenrodDeptStoreB1F_EventScript_83D3537::
	setobjectxyperm 11, 1, 0xC
	setobjectxyperm 1, 1, 0xB
	setobjectmovementtype 11, 0xA
	return

Johto_GoldenrodDeptStoreB1F_EventScript_83D354A::
	setobjectxyperm 12, 0xA, 0xB
	setobjectxyperm 2, 9, 0xB
	setobjectmovementtype 12, 8
	return

Johto_GoldenrodDeptStoreB1F_EventScript_83D355D::
	setobjectxyperm 13, 0x11, 0x10
	setobjectxyperm 3, 0x12, 0x10
	setobjectmovementtype 13, 9
	return

@ Johto_GoldenrodDeptStoreB1F object 9 (local id 9, gfx 0x2C) at 5,9
Johto_GoldenrodDeptStoreB1F_EventScript_83D3570::
	lock
	goto_if_unset FLAG_QZ_0x103C, Johto_GoldenrodDeptStoreB1F_EventScript_83D35E0
	faceplayer
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D36F4, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStoreB1F object 8 (local id 8, gfx 0x2C) at 16,11
Johto_GoldenrodDeptStoreB1F_EventScript_83D3586::
	lock
	goto_if_unset FLAG_QZ_0x103D, Johto_GoldenrodDeptStoreB1F_EventScript_83D359C
	faceplayer
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D3755, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodDeptStoreB1F_EventScript_83D359C::
	checkitem ITEM_CARD_KEY, 1
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodDeptStoreB1F_EventScript_83D3613
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D3755, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStoreB1F object 10 (local id 10, gfx 0x2C) at 12,14
Johto_GoldenrodDeptStoreB1F_EventScript_83D35B7::
	lock
	goto_if_unset FLAG_QZ_0x103E, Johto_GoldenrodDeptStoreB1F_EventScript_83D3646
	faceplayer
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D37D2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodDeptStoreB1F object 11 (local id 11, gfx 0x143) at 5,11
@ Johto_GoldenrodDeptStoreB1F object 12 (local id 12, gfx 0x143) at 14,11
@ Johto_GoldenrodDeptStoreB1F object 13 (local id 13, gfx 0x143) at 12,16
Johto_GoldenrodDeptStoreB1F_EventScript_83D35CD::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MACHOKE, 0
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D3818, MSGBOX_DEFAULT
	waitmoncry
	release
	end

Johto_GoldenrodDeptStoreB1F_EventScript_83D35E0::
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D36D0, MSGBOX_DEFAULT
	closemessage
	playse SE_M_STRENGTH
	applymovement 1, Johto_GoldenrodDeptStoreB1F_Movement_83D36B4
	applymovement 11, Johto_GoldenrodDeptStoreB1F_Movement_83D36AD
	waitmovement 0
	waitmovement 0
	setobjectxyperm 11, 1, 0xC
	setobjectxyperm 1, 1, 0xB
	setflag FLAG_QZ_0x103C
	release
	end

Johto_GoldenrodDeptStoreB1F_EventScript_83D3613::
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D36D0, MSGBOX_DEFAULT
	closemessage
	playse SE_M_STRENGTH
	applymovement 2, Johto_GoldenrodDeptStoreB1F_Movement_83D36BF
	applymovement 12, Johto_GoldenrodDeptStoreB1F_Movement_83D36BA
	waitmovement 0
	waitmovement 0
	setobjectxyperm 12, 0xA, 0xB
	setobjectxyperm 2, 9, 0xB
	setflag FLAG_QZ_0x103D
	release
	end

Johto_GoldenrodDeptStoreB1F_EventScript_83D3646::
	msgbox Johto_GoldenrodDeptStoreB1F_Text_83D36D0, MSGBOX_DEFAULT
	closemessage
	playse SE_M_STRENGTH
	applymovement 3, Johto_GoldenrodDeptStoreB1F_Movement_83D36CA
	applymovement 13, Johto_GoldenrodDeptStoreB1F_Movement_83D36C4
	waitmovement 0
	waitmovement 0
	setobjectxyperm 13, 0x11, 0x10
	setobjectxyperm 3, 0x12, 0x10
	setflag FLAG_QZ_0x103E
	release
	end

@ Johto_GoldenrodDeptStoreB1F object 4 (local id 4, gfx 0x3A9E) at 16,3
Johto_GoldenrodDeptStoreB1F_EventScript_83D3679::
	setorcopyvar VAR_0x8000, 0x1D2
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodDeptStoreB1F object 7 (local id 7, gfx 0x3A9E) at 10,20
Johto_GoldenrodDeptStoreB1F_EventScript_83D3686::
	setorcopyvar VAR_0x8000, 0x31
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodDeptStoreB1F object 5 (local id 5, gfx 0x3A9E) at 7,3
Johto_GoldenrodDeptStoreB1F_EventScript_83D3693::
	setorcopyvar VAR_0x8000, 3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodDeptStoreB1F object 6 (local id 6, gfx 0x3A9E) at 17,19
Johto_GoldenrodDeptStoreB1F_EventScript_83D36A0::
	setorcopyvar VAR_0x8000, 0x66
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_GoldenrodDeptStoreB1F_Movement_83D36AD:
	walk_left
	walk_left
	walk_left
	walk_down
	walk_left
	face_up
	step_end

Johto_GoldenrodDeptStoreB1F_Movement_83D36B4:
	slide_left
	delay_8
	slide_left
	delay_8
	slide_left
	step_end

Johto_GoldenrodDeptStoreB1F_Movement_83D36BA:
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodDeptStoreB1F_Movement_83D36BF:
	slide_left
	slide_left
	slide_left
	slide_left
	step_end

Johto_GoldenrodDeptStoreB1F_Movement_83D36C4:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Johto_GoldenrodDeptStoreB1F_Movement_83D36CA:
	slide_right
	slide_right
	slide_right
	slide_right
	slide_right
	step_end

Johto_GoldenrodDeptStoreB1F_Text_83D36D0::
	.string "MACHOKE, help us move that boulder!$"

Johto_GoldenrodDeptStoreB1F_Text_83D36F4::
	.string "Hey, kid! You’re holding us up!\p"
	.string "Our policy is to work behind the\n"
	.string "scenes where no one can see us!$"

Johto_GoldenrodDeptStoreB1F_Text_83D3755::
	.string "I lose my passion for work if\n"
	.string "someone’s watching.\p"
	.string "Come on, kid, scoot!$"

Johto_GoldenrodDeptStoreB1F_Text_83D37D2::
	.string "Oohah! Oohah!\p"
	.string "The stuff on the ground’s junk.\p"
	.string "Take it if you want it!$"

Johto_GoldenrodDeptStoreB1F_Text_83D3818::
	.string "MACHOKE: Maaacho!$"
