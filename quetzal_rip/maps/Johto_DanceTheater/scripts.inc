Johto_DanceTheater_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_DanceTheater_EventScript_83CF583
	.byte 0

Johto_DanceTheater_EventScript_83CF583::
	call Quetzal_EventScript_840EA62
	call_if_eq VAR_QZ_0x501A, 1, Johto_DanceTheater_EventScript_83CF59F
	call_if_eq VAR_QZ_0x501A, 8, Johto_DanceTheater_EventScript_83CF5AB
	end

Johto_DanceTheater_EventScript_83CF59F::
	setobjectxyperm 5, 0xA, 2
	setobjectmovementtype 11, 0x64
	return

Johto_DanceTheater_EventScript_83CF5AB::
	setobjectmovementtype 5, 0x18
	setobjectmovementtype 2, 0x18
	setobjectmovementtype 4, 0x18
	setobjectmovementtype 1, 0x17
	setobjectmovementtype 3, 0x17
	setobjectmovementtype 10, 0x18
	return

@ Johto_DanceTheater trigger at 10,16 (VAR_QZ_0x501A == 0)
@ Johto_DanceTheater trigger at 11,17 (VAR_QZ_0x501A == 0)
@ Johto_DanceTheater trigger at 9,17 (VAR_QZ_0x501A == 0)
Johto_DanceTheater_EventScript_83CF5C4::
	lockall
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	turnobject LOCALID_PLAYER, 2
	delay 0x3C
	turnobject 11, 2
	special SpawnCameraObject
	applymovement LOCALID_CAMERA, Johto_DanceTheater_Movement_83CF64D
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83CF66B, MSGBOX_DEFAULT
	closemessage
	msgbox Johto_DanceTheater_Text_83CF6BB, MSGBOX_DEFAULT
	closemessage
	msgbox Johto_DanceTheater_Text_83CF6F2, MSGBOX_DEFAULT
	applymovement 5, Johto_DanceTheater_Movement_83CF667
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83CF73D, MSGBOX_DEFAULT
	closemessage
	applymovement 11, Johto_DanceTheater_Movement_83CF63D
	delay 0x32
	applymovement LOCALID_CAMERA, Johto_DanceTheater_Movement_83CF65A
	waitmovement 0
	special RemoveCameraObject
	setobjectmovementtype 11, 0x64
	setobjectxyperm 5, 0xA, 2
	setvar VAR_QZ_0x501A, 1
	releaseall
	end

@ Johto_DanceTheater object 11 (local id 11, gfx 0x3EB0) at 10,4
Johto_DanceTheater_EventScript_83CF789::
	lock
	faceplayer
	msgbox Johto_DanceTheater_Text_83CF824, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4133, 0, Johto_DanceTheater_Text_83CF851
	msgbox Johto_DanceTheater_Text_83CF87A, MSGBOX_DEFAULT
	closemessage
	applymovement 11, Johto_DanceTheater_Movement_83CF81A
	waitmovement 0
	setflag FLAG_QZ_0x10AC
	removeobject 11
	call_if_eq VAR_FACING, 4, Johto_DanceTheater_EventScript_83CF7EB
	call_if_eq VAR_FACING, 3, Johto_DanceTheater_EventScript_83CF800
	call_if_eq VAR_FACING, 1, Johto_DanceTheater_EventScript_83CF815
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_DanceTheater_Text_83CF912, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501A, 2
	release
	end

Johto_DanceTheater_EventScript_83CF7EB::
	applymovement 5, Quetzal_Movement_836083D
	waitmovement 0
	applymovement 5, Quetzal_Movement_8360839
	waitmovement 0
	return

Johto_DanceTheater_EventScript_83CF800::
	applymovement 5, Quetzal_Movement_836083B
	waitmovement 0
	applymovement 5, Quetzal_Movement_8360839
	waitmovement 0
	return

Johto_DanceTheater_EventScript_83CF815::
	turnobject 5, 1
	return

@ Johto_DanceTheater object 1 (local id 1, gfx 0x3E9D) at 7,2
@ Johto_DanceTheater object 2 (local id 2, gfx 0x3E9D) at 5,3
@ Johto_DanceTheater object 3 (local id 3, gfx 0x3E9D) at 13,2
@ Johto_DanceTheater object 4 (local id 4, gfx 0x3E9D) at 15,3
Johto_DanceTheater_EventScript_83CF9E7::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501A, 6, Johto_DanceTheater_EventScript_83CF9FF
	msgbox Johto_DanceTheater_Text_83D0AD9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CF9FF::
	msgbox Johto_DanceTheater_Text_83D0BCF, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater object 5 (local id 5, gfx 0x3E9D) at 10,3
Johto_DanceTheater_EventScript_83CFA0A::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x501A, 8, Johto_DanceTheater_EventScript_83CFA38
	goto_if_eq VAR_QZ_0x501A, 6, Johto_DanceTheater_EventScript_83D0224
	goto_if_eq VAR_QZ_0x501A, 1, Johto_DanceTheater_EventScript_83CFA43
	msgbox Johto_DanceTheater_Text_83CFA4E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CFA38::
	msgbox Johto_DanceTheater_Text_83D0AF4, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CFA43::
	msgbox Johto_DanceTheater_Text_83D04BA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater object 7 (local id 7, gfx 0x30) at 13,11
Johto_DanceTheater_EventScript_83CFAF5::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501A, 1, Johto_DanceTheater_EventScript_83CFB34
	goto_if_unset FLAG_QZ_0x1006, Johto_DanceTheater_EventScript_83CFB16
	msgbox Johto_DanceTheater_Text_83CFD5B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CFB16::
	msgbox Johto_DanceTheater_Text_83CFC97, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x1006
	msgbox Johto_DanceTheater_Text_83CFD13, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1006
	setvar VAR_QZ_0x501A, 3
	release
	end

Johto_DanceTheater_EventScript_83CFB34::
	msgbox Johto_DanceTheater_Text_83CFC20, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater trigger at 10,17 (VAR_QZ_0x501A == 2)
Johto_DanceTheater_EventScript_83CFB3F::
	lock
	applymovement 7, Johto_DanceTheater_Movement_83CFC0E
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_DanceTheater_Text_83CFC97, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x1006
	msgbox Johto_DanceTheater_Text_83CFD13, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1006
	setvar VAR_QZ_0x501A, 3
	applymovement 7, Johto_DanceTheater_Movement_83CFC17
	waitmovement 0
	release
	end

@ Johto_DanceTheater object 8 (local id 8, gfx 0x3EBF) at 17,9
Johto_DanceTheater_EventScript_83CFB76::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501A, 1, Johto_DanceTheater_EventScript_83CFB8E
	msgbox Johto_DanceTheater_Text_83CFE2C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CFB8E::
	msgbox Johto_DanceTheater_Text_83CFE24, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater object 6 (local id 6, gfx 0x1E) at 7,11
Johto_DanceTheater_EventScript_83CFB99::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501A, 1, Johto_DanceTheater_EventScript_83CFBB1
	msgbox Johto_DanceTheater_Text_83CFE7A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CFBB1::
	msgbox Johto_DanceTheater_Text_83CFF13, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater object 9 (local id 9, gfx 0x3EA6) at 3,9
Johto_DanceTheater_EventScript_83CFBBC::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501A, 1, Johto_DanceTheater_EventScript_83CFBD4
	msgbox Johto_DanceTheater_Text_83CFF73, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CFBD4::
	msgbox Johto_DanceTheater_Text_83D003D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater object 10 (local id 10, gfx 0x6) at 5,11
Johto_DanceTheater_EventScript_83CFBDF::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501A, 1, Johto_DanceTheater_EventScript_83CFBF7
	msgbox Johto_DanceTheater_Text_83D009B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_EventScript_83CFBF7::
	msgbox Johto_DanceTheater_Text_83D00DB, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater sign / bg event at 11,8
@ Johto_DanceTheater sign / bg event at 10,8
Johto_DanceTheater_EventScript_83CFC02::
	lock
	msgbox Johto_DanceTheater_Text_83D00F4, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DanceTheater trigger at 9,17 (VAR_QZ_0x501A == 5)
Johto_DanceTheater_EventScript_83D0126::
	lock
	applymovement LOCALID_PLAYER, Johto_DanceTheater_Movement_83D0162
	waitmovement LOCALID_PLAYER
	delay 0xA
	lockall
	goto Johto_DanceTheater_EventScript_83D01FC

@ Johto_DanceTheater trigger at 10,16 (VAR_QZ_0x501A == 5)
Johto_DanceTheater_EventScript_83D013A::
	lock
	applymovement LOCALID_PLAYER, Johto_DanceTheater_Movement_83D018D
	waitmovement LOCALID_PLAYER
	delay 0xA
	lockall
	goto Johto_DanceTheater_EventScript_83D01FC

@ Johto_DanceTheater trigger at 11,17 (VAR_QZ_0x501A == 5)
Johto_DanceTheater_EventScript_83D014E::
	lock
	applymovement LOCALID_PLAYER, Johto_DanceTheater_Movement_83D01B8
	waitmovement LOCALID_PLAYER
	delay 0xA
	lockall
	goto Johto_DanceTheater_EventScript_83D01FC

Johto_DanceTheater_EventScript_83D01F7::
	bufferspeciesname 0, SPECIES_HO_OH
	return

Johto_DanceTheater_EventScript_83D01FC::
	setvar VAR_QZ_0x5017, 2
	call Johto_DanceTheater_EventScript_83D01F7
	msgbox Johto_DanceTheater_Text_83D04BE, MSGBOX_DEFAULT
	closemessage
	delay 0x14
	playse SE_M_PETAL_DANCE
	applymovement 5, Johto_DanceTheater_Movement_83D01E7
	waitmovement 0
	setvar VAR_QZ_0x501A, 6

Johto_DanceTheater_EventScript_83D0224::
	msgbox Johto_DanceTheater_Text_83D060C, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_DanceTheater_EventScript_83D0477
	call_if_eq VAR_FACING, 1, Johto_DanceTheater_EventScript_83D034F
	call_if_eq VAR_FACING, 3, Johto_DanceTheater_EventScript_83D0380
	call_if_eq VAR_FACING, 4, Johto_DanceTheater_EventScript_83D039D
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4114, 0, Johto_DanceTheater_Text_83D04BA
	applymovement 5, Johto_DanceTheater_Movement_83D0482
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 2, Johto_DanceTheater_Movement_83D01E7
	waitmovement 0
	applymovement 2, Johto_DanceTheater_Movement_83D0486
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83D0683, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4312, 0, Johto_DanceTheater_Text_83D04BA
	applymovement 2, Johto_DanceTheater_Movement_83D048D
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 4, Johto_DanceTheater_Movement_83D01E7
	waitmovement 0
	applymovement 4, Johto_DanceTheater_Movement_83D0494
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83D0756, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4594, 0, Johto_DanceTheater_Text_83D04BA
	applymovement 4, Johto_DanceTheater_Movement_83D049B
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 1, Johto_DanceTheater_Movement_83D01E7
	waitmovement 0
	applymovement 1, Johto_DanceTheater_Movement_83D04A2
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83D0848, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4098, 0, Johto_DanceTheater_Text_83D04BA
	applymovement 1, Johto_DanceTheater_Movement_83D04A8
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Johto_DanceTheater_Movement_83D01E7
	waitmovement 0
	applymovement 3, Johto_DanceTheater_Movement_83D04AE
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83D0918, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4143, 0, Johto_DanceTheater_Text_83D04BA
	applymovement 3, Johto_DanceTheater_Movement_83D04B4
	waitmovement 0
	applymovement 5, Quetzal_Movement_8360839
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83D0A00, MSGBOX_DEFAULT
	goto Johto_DanceTheater_EventScript_83D03BA

Johto_DanceTheater_EventScript_83D034F::
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	turnobject 5, 1
	return

Johto_DanceTheater_EventScript_83D0380::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	turnobject 5, 1
	return

Johto_DanceTheater_EventScript_83D039D::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	turnobject 5, 1
	return

Johto_DanceTheater_EventScript_83D03BA::
	checkitem ITEM_RAINBOW_WING, 1
	goto_if_eq VAR_RESULT, 1, Johto_DanceTheater_EventScript_83D03D6
	setorcopyvar VAR_0x8000, 0x376
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM

Johto_DanceTheater_EventScript_83D03D6::
	setorcopyvar VAR_0x8000, 0x371
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	closemessage
	playse SE_M_RAZOR_WIND
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	addobject 10
	applymovement 10, Johto_DanceTheater_Movement_83D046A
	waitmovement 0
	msgbox Johto_DanceTheater_Text_83D0A4E, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 5, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject 5, 1
	msgbox Johto_DanceTheater_Text_83D0AA5, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers 1
	removeobject 5
	removeobject 4
	removeobject 2
	removeobject 3
	removeobject 1
	removeobject 4
	removeobject 10
	playse SE_EXIT
	delay 0x78
	fadescreenswapbuffers 0
	clearflag FLAG_QZ_0x10D8
	clearflag FLAG_QZ_0x1014
	setvar VAR_QZ_0x5007, 2
	setvar VAR_QZ_0x5006, 0
	setflag FLAG_QZ_0x10D7
	setflag FLAG_QZ_0x10AD
	setvar VAR_QZ_0x501A, 7
	release
	end

Johto_DanceTheater_EventScript_83D0477::
	msgbox Johto_DanceTheater_Text_83D0651, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DanceTheater_Movement_83CF63D:
	walk_in_place_down
	delay_2
	walk_in_place_right
	delay_2
	face_up
	delay_2
	face_left
	delay_8
	walk_in_place_down
	delay_2
	walk_in_place_left
	delay_2
	face_up
	delay_8
	face_right
	step_end

Johto_DanceTheater_Movement_83CF64D:
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	slide_up
	step_end

Johto_DanceTheater_Movement_83CF65A:
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	step_end

Johto_DanceTheater_Movement_83CF667:
	lock_facing_direction
	walk_up
	unlock_facing_direction
	step_end

Johto_DanceTheater_Movement_83CF81A:
	jump_2_down
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_DanceTheater_Movement_83CFC0E:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_down
	step_end

Johto_DanceTheater_Movement_83CFC17:
	walk_up
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_DanceTheater_Movement_83D0162:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	face_up
	step_end

Johto_DanceTheater_Movement_83D018D:
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	face_up
	step_end

Johto_DanceTheater_Movement_83D01B8:
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	face_up
	step_end

Johto_DanceTheater_Movement_83D01E7:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	delay_8
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	step_end

Johto_DanceTheater_Movement_83D046A:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_right
	step_end

Johto_DanceTheater_Movement_83D0482:
	lock_facing_direction
	walk_up
	unlock_facing_direction
	step_end

Johto_DanceTheater_Movement_83D0486:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_down
	step_end

Johto_DanceTheater_Movement_83D048D:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	face_down
	step_end

Johto_DanceTheater_Movement_83D0494:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	face_down
	step_end

Johto_DanceTheater_Movement_83D049B:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_down
	step_end

Johto_DanceTheater_Movement_83D04A2:
	walk_down
	walk_right
	walk_right
	walk_right
	face_down
	step_end

Johto_DanceTheater_Movement_83D04A8:
	walk_left
	walk_left
	walk_left
	walk_up
	face_down
	step_end

Johto_DanceTheater_Movement_83D04AE:
	walk_down
	walk_left
	walk_left
	walk_left
	face_down
	step_end

Johto_DanceTheater_Movement_83D04B4:
	walk_right
	walk_right
	walk_right
	walk_up
	face_down
	step_end

Johto_DanceTheater_Text_83CF66B::
	.string "ROCKET: Stop dancing such a serious\n"
	.string "dance! Show me something like\l"
	.string "a hula dance!$"

Johto_DanceTheater_Text_83CF6BB::
	.string "KIMONO GIRL: ...You mustn’t push\n"
	.string "such a request on me.$"

Johto_DanceTheater_Text_83CF6F2::
	.string "ROCKET: Huh?\p"
	.string "Are you telling me you don’t\n"
	.string "respect what the customer\l"
	.string "wants?$"

Johto_DanceTheater_Text_83CF73D::
	.string "Well, then I’ll show you how\n"
	.string "to dance. I’ll show you a\l"
	.string "great one.\p"
	.string "Wahahaha!$"

Johto_DanceTheater_Text_83CF824::
	.string "Huh? Who are you?\n"
	.string "You dare to get in my way?$"

Johto_DanceTheater_Text_83CF851::
	.string "Oh, no!\n"
	.string "You make me look like a\l"
	.string "villain!$"

Johto_DanceTheater_Text_83CF87A::
	.string "Oops! I have an important mission!\n"
	.string "If they find out I was wasting\l"
	.string "time here, they’ll make me start\l"
	.string "over as the lowest ROCKET GRUNT!\p"
	.string "I better leave now!$"

Johto_DanceTheater_Text_83CF912::
	.string "You must be {PLAYER},\n"
	.string "correct? That was indeed\l"
	.string "excellent.\p"
	.string "Kind and strong... Good at\n"
	.string "raising POKéMON as well...\p"
	.string "That person does know what\n"
	.string "to look for in people...\p"
	.string "Ohoho! That was just me\n"
	.string "talking to myself.\l"
	.string "Never mind.$"

Johto_DanceTheater_Text_83CFA4E::
	.string "For KIMONO GIRLS like me,\n"
	.string "dancing is everything.\p"
	.string "I have never gone a day\n"
	.string "without a dance practice.\p"
	.string "POKéMON are the same.\n"
	.string "They grow with you little\l"
	.string "by little each day.$"

Johto_DanceTheater_Text_83CFC20::
	.string "What’s the deal with that\n"
	.string "Team Rocket Grunt, messing\l"
	.string "with that KIMONO GIRL?\p"
	.string "You are a Trainer, right?\n"
	.string "Please help her!$"

Johto_DanceTheater_Text_83CFC97::
	.string "Wonderful! You were so\n"
	.string "courageous for your age!\p"
	.string "It was a rare sight to see!\n"
	.string "I want you to have this.\p"
	.string "Don’t be shy--take it!$"

Johto_DanceTheater_Text_83CFD13::
	.string "That’s SURF.\n"
	.string "It’s a move that lets POKéMON\l"
	.string "swim across water with ease.$"

Johto_DanceTheater_Text_83CFD5B::
	.string "Not only are the KIMONO GIRLS\n"
	.string "great dancers, they’re also\l"
	.string "skilled at POKéMON.\p"
	.string "But they battle only for good\n"
	.string "reasons.\p"
	.string "I don’t know if you could beat\n"
	.string "them if they showed their skill...$"

Johto_DanceTheater_Text_83CFE24::
	.string "Eeeeek!$"

Johto_DanceTheater_Text_83CFE2C::
	.string "I was supposed to be the hero...!\n"
	.string "So much for getting close to\l"
	.string "a KIMONO GIRL!$"

Johto_DanceTheater_Text_83CFE7A::
	.string "That man’s always with his\n"
	.string "PSYDUCK.\p"
	.string "Says he wants a POKéMON that\n"
	.string "can SURF and dance.\p"
	.string "Is he trying to make a POKéMON\n"
	.string "that stars in synchronized\l"
	.string "swimming?$"

Johto_DanceTheater_Text_83CFF13::
	.string "Oh... Ouch...!\n"
	.string "This stomachache!\p"
	.string "I wouldn’t let this guy act like\n"
	.string "that if not for this pain...!$"

Johto_DanceTheater_Text_83CFF73::
	.string "The KIMONO GIRLS are so\n"
	.string "beautiful...\p"
	.string "But they have to go through\n"
	.string "rigorous training.\p"
	.string "And they have to learn to follow\n"
	.string "customs before appearing in\l"
	.string "public.\p"
	.string "But if you love something,\n"
	.string "anything is possible.$"

Johto_DanceTheater_Text_83D003D::
	.string "Oh... Ouch...!\n"
	.string "This back pain!\p"
	.string "I wouldn’t let this guy act like\n"
	.string "that if not for this pain...!$"

Johto_DanceTheater_Text_83D009B::
	.string "The KIMONO GIRLS are my big\n"
	.string "sisters! I’ll be like them one\l"
	.string "day!$"

Johto_DanceTheater_Text_83D00DB::
	.string "I like that funny dance!$"

Johto_DanceTheater_Text_83D00F4::
	.string "It’s a fancy panel that’s\n"
	.string "decorated with flowers.$"

Johto_DanceTheater_Text_83D04BA::
	.string "...$"

Johto_DanceTheater_Text_83D04BE::
	.string "Welcome. My name is ZUKI.\n"
	.string "I met you previously in VIOLET\l"
	.string "CITY.\p"
	.string "To bring back {STR_VAR_1}, we\n"
	.string "needed someone with the right bond\l"
	.string "with POKéMON.\p"
	.string "We asked MR. POKéMON to give this\n"
	.string "MYSTERY EGG to\l"
	.string "whoever showed potential.\p"
	.string "The EGG went from MR. POKéMON\n"
	.string "to PROF.ELM, then to you.\p"
	.string "But being pure alone is not enough\n"
	.string "to face the power of this POKéMON.$"

Johto_DanceTheater_Text_83D060C::
	.string "I will test you to see if you are\n"
	.string "a good Trainer.\p"
	.string "Are you ready, {PLAYER}?$"

Johto_DanceTheater_Text_83D0651::
	.string "That is fine...\n"
	.string "I will wait for you to get\l"
	.string "ready.$"

Johto_DanceTheater_Text_83D0683::
	.string "You are quite strong. You helped\n"
	.string "me in the ILEX FOREST.\p"
	.string "I’m NAOKO--not so good with\n"
	.string "directions.\p"
	.string "I saw you defeat Team Rocket\n"
	.string "when KURT was in trouble at\l"
	.string "the SLOWPOKE WELL.\p"
	.string "Let me challenge you and your\n"
	.string "POKéMON!$"

Johto_DanceTheater_Text_83D0756::
	.string "You’re quite the Trainer. I\n"
	.string "remember you chased off that\l"
	.string "Rocket Grunt from right here.\p"
	.string "Thank you for that.\p"
	.string "Though, I’m a Trainer too.\n"
	.string "She who knows the most speaks\l"
	.string "the least!\p"
	.string "I am MIKI--the KIMONO GIRL!\p"
	.string "Let me challenge you and your\n"
	.string "POKéMON!$"

Johto_DanceTheater_Text_83D0848::
	.string "Excellent spirit! You got me\n"
	.string "off the ice by pushing me.\p"
	.string "I skate in sandals.\n"
	.string "I am SAYO--the KIMONO GIRL!\p"
	.string "I listened in the DRAGON’S DEN\n"
	.string "when you passed the Elder’s test.\p"
	.string "Let me challenge you and your\n"
	.string "POKéMON!$"

Johto_DanceTheater_Text_83D0918::
	.string "I’m the last one.\p"
	.string "You were dependable at the\n"
	.string "GOLDENROD TUNNEL.\p"
	.string "I’m the one who twirls even\n"
	.string "underground--KUNI!\p"
	.string "I saw you thwart Rocket’s\n"
	.string "plan to take the Radio Tower.\p"
	.string "Now show me your strength!\n"
	.string "Let me challenge you and your\l"
	.string "POKéMON!$"

Johto_DanceTheater_Text_83D0A00::
	.string "We were not wrong about you.\n"
	.string "You are indeed worthy of this...\p"
	.string "Please take it.$"

Johto_DanceTheater_Text_83D0A4E::
	.string "Big sisters! I just saw the\n"
	.string "big shadow of a POKéMON over\l"
	.string "the TIN TOWER!\p"
	.string "It could be...$"

Johto_DanceTheater_Text_83D0AA5::
	.string "ZUKI: {PLAYER}!\n"
	.string "We’ll be at the TIN TOWER.\n"
	.string "See you there!$"

Johto_DanceTheater_Text_83D0AD9::
	.string "We are rehearsing a dance.$"

Johto_DanceTheater_Text_83D0AF4::
	.string "Thank you for your help the\n"
	.string "other day.\p"
	.string "You’ve shown the bond between\n"
	.string "people can grow with POKéMON...\p"
	.string "We appreciate how you lived up\n"
	.string "to our hopes, {PLAYER}!\p"
	.string "If you learned anything from\n"
	.string "this, thank MR. POKéMON and\l"
	.string "PROF.ELM...$"

Johto_DanceTheater_Text_83D0BCF::
	.string "Talk to ZUKI in the middle\n"
	.string "when you are ready.$"
