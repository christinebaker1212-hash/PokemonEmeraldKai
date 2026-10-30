Kanto_IndigoPlateau_Exterior_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_IndigoPlateau_Exterior_EventScript_8416F1C
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_IndigoPlateau_Exterior_OnFrame_8416F33
	.byte 0

Kanto_IndigoPlateau_Exterior_OnFrame_8416F33:
	map_script_2 VAR_QZ_0x5113, 1, Kanto_IndigoPlateau_Exterior_EventScript_8416F3D
	.2byte 0

Kanto_IndigoPlateau_Exterior_EventScript_8416F1C::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x223F
	call_if_eq VAR_QZ_0x5113, 1, Kanto_IndigoPlateau_Exterior_EventScript_8416F2E
	end

Kanto_IndigoPlateau_Exterior_EventScript_8416F2E::
	playbgm MUS_RG_CREDITS, 0
	return

Kanto_IndigoPlateau_Exterior_EventScript_8416F3D::
	lockall
	delay 0x69
	opendoor 0xB, 6
	waitdooranim
	special SpawnCameraObject
	addobject 1
	applymovement 1, Kanto_IndigoPlateau_Exterior_Movement_841701F
	applymovement LOCALID_PLAYER, Kanto_IndigoPlateau_Exterior_Movement_8417010
	waitmovement 0
	closedoor 0xB, 6
	waitdooranim
	applymovement LOCALID_PLAYER, Kanto_IndigoPlateau_Exterior_Movement_8416FF4
	applymovement 1, Kanto_IndigoPlateau_Exterior_Movement_8417017
	waitmovement 0
	delay 0x64
	opendoor 0xB, 6
	waitdooranim
	addobject 2
	applymovement 2, Kanto_IndigoPlateau_Exterior_Movement_8417036
	waitmovement 0
	closedoor 0xB, 6
	waitdooranim
	delay 0xA0
	applymovement LOCALID_PLAYER, Kanto_IndigoPlateau_Exterior_Movement_8416FF8
	applymovement 2, Kanto_IndigoPlateau_Exterior_Movement_8417021
	waitmovement 0
	delay 0x64
	applymovement LOCALID_PLAYER, Kanto_IndigoPlateau_Exterior_Movement_8417007
	waitmovement 0
	delay 0x9B
	applymovement LOCALID_PLAYER, Kanto_IndigoPlateau_Exterior_Movement_8417015
	waitmovement 0
	delay 0x5A
	applymovement LOCALID_PLAYER, Kanto_IndigoPlateau_Exterior_Movement_841700E
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_IndigoPlateau_Exterior_Movement_8416FEA
	waitmovement 0
	special RemoveCameraObject
	setvar VAR_QZ_0x5113, 0
	removeobject 1
	removeobject 2
	special QzDoCredits
	waitstate
	releaseall
	end

Kanto_IndigoPlateau_Exterior_Movement_8416FEA:
	player_run_down
	player_run_down
	player_run_down
	player_run_down
	player_run_down
	player_run_down
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8416FF4:
	delay_16
	delay_4
	walk_in_place_faster_down
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8416FF8:
	delay_8
	walk_in_place_faster_right
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_down
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8417007:
	walk_right
	delay_16
	walk_down
	walk_down
	delay_16
	delay_16
	step_end

Kanto_IndigoPlateau_Exterior_Movement_841700E:
	walk_in_place_faster_down
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8417010:
	face_right
	lock_facing_direction
	walk_left
	unlock_facing_direction
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8417015:
	walk_in_place_faster_up
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8417017:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Kanto_IndigoPlateau_Exterior_Movement_841701F:
	walk_fast_down
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8417021:
	walk_in_place_faster_left
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_down
	delay_16
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_IndigoPlateau_Exterior_Movement_8417036:
	walk_down
	step_end
