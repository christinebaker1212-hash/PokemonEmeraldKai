Kanto_SSAnne_Exterior_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SSAnne_Exterior_EventScript_84101C8
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SSAnne_Exterior_EventScript_84101BE
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_840FBB9
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_SSAnne_Exterior_OnFrame_84101CC
	.byte 0

Kanto_SSAnne_Exterior_OnFrame_84101CC:
	map_script_2 VAR_QZ_0x5135, 1, Kanto_SSAnne_Exterior_EventScript_84101D6
	.2byte 0

Kanto_SSAnne_Exterior_EventScript_84101BE::
	setflag FLAG_QZ_0x225A
	clearflag FLAG_QZ_0x2313
	special QzKantoStaticEncountersRefresh_757
	end

Kanto_SSAnne_Exterior_EventScript_84101C8::
	special QzSSAnneExteriorEventInit_765
	end

Kanto_SSAnne_Exterior_EventScript_84101D6::
	lockall
	getplayerxy VAR_0x8004, VAR_0x8005
	call_if_le VAR_0x8005, 6, Kanto_SSAnne_Exterior_EventScript_841021E
	call_if_ge VAR_0x8005, 7, Kanto_SSAnne_Exterior_EventScript_8410229
	fadenewbgm MUS_RG_SURF
	delay 0x32
	special QzDoSSAnneDepartureCutscene_724
	waitstate
	removeobject 1
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_8410234
	waitmovement 0
	setvar VAR_QZ_0x5135, 2
	setvar VAR_QZ_0x5145, 0
	warp MAP_KANTO_VERMILION_CITY, 255, 23, 34
	waitstate
	releaseall
	end

Kanto_SSAnne_Exterior_EventScript_841021E::
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_841023E
	waitmovement 0
	return

Kanto_SSAnne_Exterior_EventScript_8410229::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	return

@ Kanto_SSAnne_Exterior sign / bg event at 55,2
@ Kanto_SSAnne_Exterior sign / bg event at 56,2
@ Kanto_SSAnne_Exterior sign / bg event at 57,2
@ Kanto_SSAnne_Exterior sign / bg event at 55,3
@ Kanto_SSAnne_Exterior sign / bg event at 56,3
@ Kanto_SSAnne_Exterior sign / bg event at 57,3
Kanto_SSAnne_Exterior_EventScript_8410248::
	lockall
	goto_if_set FLAG_QZ_0x2931, Kanto_SSAnne_Exterior_EventScript_841029D
	special QzSSAnneExteriorEventCheck_764
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_Exterior_EventScript_8410295
	msgbox Kanto_SSAnne_Exterior_Text_84103DE, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_Exterior_EventScript_841029D
	goto Kanto_SSAnne_Exterior_EventScript_84102A0

Kanto_SSAnne_Exterior_EventScript_8410295::
	msgbox Kanto_SSAnne_Exterior_Text_8410417, MSGBOX_DEFAULT

Kanto_SSAnne_Exterior_EventScript_841029D::
	closemessage
	releaseall
	end

Kanto_SSAnne_Exterior_EventScript_84102A0::
	closemessage
	special QzResetFollowerObjects_572
	call Kanto_SSAnne_Exterior_EventScript_8410331
	msgbox Kanto_SSAnne_Exterior_Text_8410443, MSGBOX_DEFAULT
	closemessage
	call Kanto_SSAnne_Exterior_EventScript_8410318
	setvar VAR_0x8004, 1
	special QzSSAnneExteriorEventStep_766
	delay 8
	call Kanto_SSAnne_Exterior_EventScript_8410318
	setvar VAR_0x8004, 2
	special QzSSAnneExteriorEventStep_766
	setflag FLAG_QZ_0x2931
	playse SE_TRUCK_STOP
	waitse
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103DB
	waitmovement LOCALID_PLAYER
	delay 0xC
	clearflag FLAG_QZ_0x292E
	addobject 2
	applymovement 2, Kanto_SSAnne_Exterior_Movement_84103D7
	waitmovement 2
	setvar VAR_LAST_TALKED, 2
	special QzKantoStaticEncounterSetupByLocalId_758
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_Exterior_EventScript_841029D
	call Quetzal_EventScript_840FC01
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_840FC13
	releaseall
	end

Kanto_SSAnne_Exterior_EventScript_8410318::
	setvar VAR_0x8004, 0
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 6
	setvar VAR_0x8007, 2
	special ShakeCamera
	waitstate
	return

Kanto_SSAnne_Exterior_EventScript_8410331::
	getplayerxy VAR_0x8008, VAR_0x8009
	goto_if_eq VAR_0x8008, 54, Kanto_SSAnne_Exterior_EventScript_8410379
	goto_if_eq VAR_0x8008, 55, Kanto_SSAnne_Exterior_EventScript_841039C
	goto_if_eq VAR_0x8008, 56, Kanto_SSAnne_Exterior_EventScript_84103A8
	goto_if_eq VAR_0x8008, 57, Kanto_SSAnne_Exterior_EventScript_84103B4
	goto_if_eq VAR_0x8009, 2, Kanto_SSAnne_Exterior_EventScript_84103C0
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103D5
	goto Kanto_SSAnne_Exterior_EventScript_84103C7

Kanto_SSAnne_Exterior_EventScript_8410379::
	goto_if_eq VAR_0x8009, 2, Kanto_SSAnne_Exterior_EventScript_8410390
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103CC
	goto Kanto_SSAnne_Exterior_EventScript_84103C7

Kanto_SSAnne_Exterior_EventScript_8410390::
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103CB
	goto Kanto_SSAnne_Exterior_EventScript_84103C7

Kanto_SSAnne_Exterior_EventScript_841039C::
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103CE
	goto Kanto_SSAnne_Exterior_EventScript_84103C7

Kanto_SSAnne_Exterior_EventScript_84103A8::
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103CF
	goto Kanto_SSAnne_Exterior_EventScript_84103C7

Kanto_SSAnne_Exterior_EventScript_84103B4::
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103D0
	goto Kanto_SSAnne_Exterior_EventScript_84103C7

Kanto_SSAnne_Exterior_EventScript_84103C0::
	applymovement LOCALID_PLAYER, Kanto_SSAnne_Exterior_Movement_84103D4

Kanto_SSAnne_Exterior_EventScript_84103C7::
	waitmovement LOCALID_PLAYER
	return

Kanto_SSAnne_Exterior_Movement_8410234:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_SSAnne_Exterior_Movement_841023E:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_SSAnne_Exterior_Movement_84103CB:
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	face_left
	step_end

Kanto_SSAnne_Exterior_Movement_84103CC:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	face_left
	step_end

Kanto_SSAnne_Exterior_Movement_84103CE:
	walk_right
	walk_right
	walk_right
	walk_up
	face_left
	step_end

Kanto_SSAnne_Exterior_Movement_84103CF:
	walk_right
	walk_right
	walk_up
	face_left
	step_end

Kanto_SSAnne_Exterior_Movement_84103D0:
	walk_right
	walk_up
	face_left
	step_end

Kanto_SSAnne_Exterior_Movement_84103D4:
	walk_down
	face_left
	step_end

Kanto_SSAnne_Exterior_Movement_84103D5:
	face_left
	step_end

Kanto_SSAnne_Exterior_Movement_84103D7:
	jump_in_place_left_right
	face_down
	delay_16
	step_end

Kanto_SSAnne_Exterior_Movement_84103DB:
	walk_down
	face_up
	step_end

Kanto_SSAnne_Exterior_Text_84103DE::
	.string "It’s a very heavy truck.\n"
	.string "Would you like to use Strength?$"

Kanto_SSAnne_Exterior_Text_8410417::
	.string "It’s a very heavy truck.\n"
	.string "You can’t move it.$"

Kanto_SSAnne_Exterior_Text_8410443::
	.string "The truck starts to move!$"
