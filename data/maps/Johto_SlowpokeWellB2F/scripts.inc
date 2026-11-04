Johto_SlowpokeWellB2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_SlowpokeWellB2F object 2 (local id 2, gfx 0x3A9E) at 22,10
Johto_SlowpokeWellB2F_EventScript_83FA2BB::
	setorcopyvar VAR_0x8000, ITEM_KINGS_ROCK
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SlowpokeWellB2F object 1 (local id 1) at 13,9
@ Moemon Kai: Crystal's man waiting for SLOWPOKE to evolve gives the KING'S ROCK (pokecrystal SlowpokeWellB2F.asm;
@ Quetzal's SLOWKING made it rain for a DAMP ROCK; docs/REGIONS_AUDIT.md 92)
Johto_SlowpokeWellB2F_EventScript_83FA2C8::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x117A, Johto_SlowpokeWellB2F_EventScript_GotKingsRock
	msgbox Johto_SlowpokeWellB2F_Text_GymGuide, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_KINGS_ROCK
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x117A
	release
	end

Johto_SlowpokeWellB2F_EventScript_GotKingsRock::
	msgbox Johto_SlowpokeWellB2F_Text_GotKingsRock, MSGBOX_DEFAULT
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

Johto_SlowpokeWellB2F_Text_GymGuide::
	.string "I’m waiting to see SLOWPOKE’s\n"
	.string "moment of evolution.\p"
	.string "Through observation, I made a new\n"
	.string "discovery.\p"
	.string "A SLOWPOKE with a KING’S ROCK\n"
	.string "often gets bitten by a SHELLDER.\p"
	.string "Here, I’ll share a KING’S ROCK\n"
	.string "with you.$"

Johto_SlowpokeWellB2F_Text_GotKingsRock::
	.string "I’m going to be like SLOWPOKE.\p"
	.string "I’ll wait patiently, so I can see\n"
	.string "one evolve.$"
