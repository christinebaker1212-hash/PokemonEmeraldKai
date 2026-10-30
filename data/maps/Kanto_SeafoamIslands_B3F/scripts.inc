Kanto_SeafoamIslands_B3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SeafoamIslands_B3F_EventScript_8412C21
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_SeafoamIslands_B3F_OnFrame_8412C5F
	.byte 0

Kanto_SeafoamIslands_B3F_OnFrame_8412C5F:
	map_script_2 VAR_TEMP_1, 1, Kanto_SeafoamIslands_B3F_EventScript_8412C69
	.2byte 0

Kanto_SeafoamIslands_B3F_EventScript_8412C21::
	call_if_unset FLAG_QZ_0x2208, Kanto_SeafoamIslands_B3F_EventScript_8412C34
	call_if_set FLAG_QZ_0x2208, Kanto_SeafoamIslands_B3F_EventScript_8412C5B
	end

Kanto_SeafoamIslands_B3F_EventScript_8412C34::
	setvar VAR_TEMP_2, 0
	call_if_unset FLAG_QZ_0x21C1, Kanto_SeafoamIslands_B3F_EventScript_8412CB7
	call_if_unset FLAG_QZ_0x21C2, Kanto_SeafoamIslands_B3F_EventScript_8412CB7
	call_if_eq VAR_TEMP_2, 2, Kanto_SeafoamIslands_B3F_EventScript_8412C57
	return

Kanto_SeafoamIslands_B3F_EventScript_8412C57::
	setflag FLAG_QZ_0x2208
	return

Kanto_SeafoamIslands_B3F_EventScript_8412C5B::
	setmaplayoutindex 0x383
	return

Kanto_SeafoamIslands_B3F_EventScript_8412C69::
	lockall
	setvar VAR_TEMP_2, 0
	call_if_unset FLAG_QZ_0x21C1, Kanto_SeafoamIslands_B3F_EventScript_8412CB7
	call_if_unset FLAG_QZ_0x21C2, Kanto_SeafoamIslands_B3F_EventScript_8412CB7
	goto_if_eq VAR_TEMP_2, 2, Kanto_SeafoamIslands_B3F_EventScript_8412CD3
	getplayerxy VAR_0x8008, VAR_0x8009
	call_if_lt VAR_0x8008, 24, Kanto_SeafoamIslands_B3F_EventScript_8412CBD
	call_if_ge VAR_0x8008, 24, Kanto_SeafoamIslands_B3F_EventScript_8412CC8
	setvar VAR_QZ_0x512B, 1
	warp MAP_KANTO_SEAFOAM_ISLANDS_B4F, 255, 27, 21
	waitstate
	releaseall
	end

Kanto_SeafoamIslands_B3F_EventScript_8412CB7::
	addvar VAR_TEMP_2, 1
	return

Kanto_SeafoamIslands_B3F_EventScript_8412CBD::
	applymovement LOCALID_PLAYER, Kanto_SeafoamIslands_B3F_Movement_8412CDA
	waitmovement 0
	return

Kanto_SeafoamIslands_B3F_EventScript_8412CC8::
	applymovement LOCALID_PLAYER, Kanto_SeafoamIslands_B3F_Movement_8412CE8
	waitmovement 0
	return

Kanto_SeafoamIslands_B3F_EventScript_8412CD3::
	setvar VAR_TEMP_1, 0
	releaseall
	end

Kanto_SeafoamIslands_B3F_Movement_8412CDA:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Kanto_SeafoamIslands_B3F_Movement_8412CE8:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end
