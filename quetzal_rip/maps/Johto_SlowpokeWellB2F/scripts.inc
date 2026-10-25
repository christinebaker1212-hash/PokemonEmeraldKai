Johto_SlowpokeWellB2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_SlowpokeWellB2F object 2 (local id 2, gfx 0x3A9E) at 22,10
Johto_SlowpokeWellB2F_EventScript_83FA2BB::
	setorcopyvar VAR_0x8000, 0x1D1
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SlowpokeWellB2F object 1 (local id 1, gfx 0x1C7) at 13,9
Johto_SlowpokeWellB2F_EventScript_83FA2C8::
	lock
	faceplayer
	playmoncry SPECIES_SLOWKING, 0
	msgbox Johto_SlowpokeWellB2F_Text_83FA36F, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	goto_if_set FLAG_QZ_0x117A, Johto_SlowpokeWellB2F_EventScript_83FA350
	playmoncry SPECIES_SLOWKING, 0
	applymovement 1, Johto_SlowpokeWellB2F_Movement_83FA356
	waitmovement 0
	delay 0x37
	playmoncry SPECIES_SLOWKING, 0
	applymovement 1, Johto_SlowpokeWellB2F_Movement_83FA35D
	waitmovement 0
	delay 0x37
	playmoncry SPECIES_SLOWKING, 0
	applymovement 1, Johto_SlowpokeWellB2F_Movement_83FA363
	waitmovement 0
	delay 0x37
	playmoncry SPECIES_SLOWKING, 0
	applymovement 1, Johto_SlowpokeWellB2F_Movement_83FA36A
	waitmovement 0
	delay 0x37
	playmoncry SPECIES_SLOWKING, 0
	setweather 3
	doweather
	waitmoncry
	setorcopyvar VAR_0x8000, 0x1BF
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x117A
	release
	end

Johto_SlowpokeWellB2F_EventScript_83FA350::
	setweather 3
	doweather
	release
	end

Johto_SlowpokeWellB2F_Movement_83FA356:
	face_right
	delay_8
	walk_in_place_fast_right
	walk_in_place_fast_right
	walk_in_place_fast_right
	delay_8
	step_end

Johto_SlowpokeWellB2F_Movement_83FA35D:
	face_left
	walk_in_place_fast_left
	walk_in_place_fast_left
	walk_in_place_fast_left
	delay_8
	step_end

Johto_SlowpokeWellB2F_Movement_83FA363:
	face_down
	lock_facing_direction
	walk_fast_right
	walk_fast_left
	unlock_facing_direction
	delay_8
	step_end

Johto_SlowpokeWellB2F_Movement_83FA36A:
	lock_facing_direction
	walk_fast_left
	walk_fast_right
	unlock_facing_direction
	step_end

Johto_SlowpokeWellB2F_Text_83FA36F::
	.string "SLOWKING: ........................$"
