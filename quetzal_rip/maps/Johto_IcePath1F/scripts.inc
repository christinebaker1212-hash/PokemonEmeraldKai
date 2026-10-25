Johto_IcePath1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IcePath1F_EventScript_83DB627
	.byte 0

Johto_IcePath1F_EventScript_83DB627::
	end

@ Johto_IcePath1F object 1 (local id 1, gfx 0x3B) at 48,14
Johto_IcePath1F_EventScript_83DB628::
	setorcopyvar VAR_0x8000, 0x47
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IcePath1F object 2 (local id 2, gfx 0x3B) at 47,29
Johto_IcePath1F_EventScript_83DB635::
	setorcopyvar VAR_0x8000, 0x42
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IcePath1F object 3 (local id 3, gfx 0x3E9D) at 49,25
Johto_IcePath1F_EventScript_83DB642::
	lock
	playbgm MUS_QZ_655, 1
	goto_if_eq VAR_FACING, 1, Johto_IcePath1F_EventScript_83DB674
	msgbox Johto_IcePath1F_Text_83DB71F, MSGBOX_DEFAULT
	closemessage
	fadedefaultbgm
	release
	end

@ Johto_IcePath1F trigger at 49,29 (VAR_QZ_0x502B == 0)
Johto_IcePath1F_EventScript_83DB65E::
	lock
	msgbox Johto_IcePath1F_Text_83DB6FC, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	release
	end

Johto_IcePath1F_EventScript_83DB674::
	msgbox Johto_IcePath1F_Text_83DB829, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_IcePath1F_EventScript_83DB6CF

Johto_IcePath1F_EventScript_83DB687::
	msgbox Johto_IcePath1F_Text_83DB925, MSGBOX_AUTOCLOSE
	closemessage
	playse SE_LEDGE
	applymovement LOCALID_PLAYER, Johto_IcePath1F_Movement_83DB6ED
	waitmovement 0
	waitse
	playse SE_M_ICY_WIND
	applymovement 3, Johto_IcePath1F_Movement_83DB6EF
	waitmovement 0
	msgbox Johto_IcePath1F_Text_83DB969, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_IcePath1F_Movement_83DB6F8
	waitmovement 3
	setflag FLAG_QZ_0x10D2
	removeobject 3
	playse SE_EXIT
	fadedefaultbgm
	setvar VAR_QZ_0x502B, 1
	release
	end

Johto_IcePath1F_EventScript_83DB6CF::
	msgbox Johto_IcePath1F_Text_83DB934, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_IcePath1F_EventScript_83DB687
	goto_if_eq VAR_RESULT, 0, Johto_IcePath1F_EventScript_83DB6CF
	@ (the ROM bytes after this point are data used elsewhere; the script never reaches them)

Johto_IcePath1F_Movement_83DB6ED:
	jump_in_place_down
	step_end

Johto_IcePath1F_Movement_83DB6EF:
	slide_down
	slide_down
	slide_down
	slide_down
	walk_in_place_fast_down
	face_up
	walk_in_place_fast_up
	walk_in_place_fast_up
	step_end

Johto_IcePath1F_Movement_83DB6F8:
	walk_down
	walk_down
	walk_down
	step_end

Johto_IcePath1F_Text_83DB6FC::
	.string "KIMONO GIRL: .....................$"

Johto_IcePath1F_Text_83DB71F::
	.string "Me oh my... Is that you, {PLAYER}?\n"
	.string "Fancy meeting you here!\p"
	.string "To tell the truth... my sandals are\n"
	.string "frozen on the ice, and I am stuck.\p"
	.string "In other words, my flip-flops are\n"
	.string "quick-frozen to the ground.\p"
	.string "Tee hee.\p"
	.string "Well, it’s no laughing matter.\p"
	.string "Would you come around and push\n"
	.string "my back?$"

Johto_IcePath1F_Text_83DB829::
	.string "Me oh my... Is that you, {PLAYER}?\n"
	.string "Fancy meeting you here!\p"
	.string "To tell the truth... my sandals are\n"
	.string "frozen on the ice, and I am stuck.\p"
	.string "In other words, my flip-flops are\n"
	.string "quick-frozen with the ground.\p"
	.string "Tee hee.\p"
	.string "Well, it’s no laughing matter.\n"
	.string "Would you push my back?$"

Johto_IcePath1F_Text_83DB925::
	.string "....SHOVE!....$"

Johto_IcePath1F_Text_83DB934::
	.string "Don’t be so cruel.\n"
	.string "You will push my back, won’t you?$"

Johto_IcePath1F_Text_83DB969::
	.string "You really are kind.\n"
	.string "Thank you so much!$"
