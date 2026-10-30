Johto_WhirlIslandLugiaChamber_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_WhirlIslandLugiaChamber_EventScript_8400EB3
	.byte 0

Johto_WhirlIslandLugiaChamber_EventScript_8400EB3::
	special QzJohtoStaticEncountersRefresh
	goto_if_eq VAR_QZ_0x5006, 2, Johto_WhirlIslandLugiaChamber_EventScript_8400ECD
	goto_if_eq VAR_QZ_0x5006, 3, Johto_WhirlIslandLugiaChamber_EventScript_8400ED1
	end

Johto_WhirlIslandLugiaChamber_EventScript_8400ECD::
	clearflag FLAG_QZ_0x105F
	end

Johto_WhirlIslandLugiaChamber_EventScript_8400ED1::
	setflag FLAG_QZ_0x1013
	setflag FLAG_QZ_0x105F
	end

@ Johto_WhirlIslandLugiaChamber trigger at 29,22 (VAR_QZ_0x5006 == 2)
Johto_WhirlIslandLugiaChamber_EventScript_8400ED8::
	lockall
	playbgm MUS_QZ_654, 1
	msgbox Johto_WhirlIslandLugiaChamber_Text_840154B, MSGBOX_DEFAULT
	closemessage
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401427
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_WhirlIslandLugiaChamber_Movement_840142D
	waitmovement 0
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401433
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401437
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401444
	waitmovement 5
	playse SE_M_PETAL_DANCE
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401451
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401451
	waitmovement 0
	playse SE_M_PETAL_DANCE
	setweather 6
	doweather
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_840146B
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401472
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401479
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401482
	waitmovement 5
	delay 0x1E
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401451
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_840146B
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_840148D
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401479
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401495
	waitmovement 5
	delay 0x1E
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401451
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401472
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401479
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401482
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_840146B
	waitmovement 6
	delay 0x1E
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401451
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_840148D
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401479
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401495
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_840146B
	waitmovement 6
	delay 0x1E
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401451
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_84014A1
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_84014B2
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_84014C4
	waitmovement 3
	delay 0x1E
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_8401451
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_8401451
	waitmovement 0
	playse SE_M_PETAL_DANCE
	waitse
	setobjectxyperm 3, 0x1B, 0x12
	setobjectxyperm 2, 0x1C, 0x13
	setobjectxyperm 4, 0x1D, 0x14
	setobjectxyperm 5, 0x1E, 0x13
	setobjectxyperm 6, 0x1F, 0x12
	setobjectmovementtype 3, 7
	setobjectmovementtype 2, 7
	setobjectmovementtype 6, 7
	setobjectmovementtype 5, 7
	setobjectmovementtype 4, 7
	setvar VAR_TEMP_1, 1
	special SpawnCameraObject
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0x23
	special LoopWingFlapSE
	applymovement 1, Johto_WhirlIslandLugiaChamber_Movement_8401420
	applymovement LOCALID_CAMERA, Johto_WhirlIslandLugiaChamber_Movement_8401403
	waitmovement 0
	delay 0x32
	waitse
	playmoncry SPECIES_LUGIA, 2
	delay 0x1E
	waitmoncry
	delay 0x3C
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0x23
	special LoopWingFlapSE
	applymovement 1, Johto_WhirlIslandLugiaChamber_Movement_8401417
	waitmovement 0
	applymovement LOCALID_CAMERA, Johto_WhirlIslandLugiaChamber_Movement_840140C
	waitmovement 0
	special RemoveCameraObject
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_84014BF
	applymovement 3, Johto_WhirlIslandLugiaChamber_Movement_84014BF
	applymovement 2, Johto_WhirlIslandLugiaChamber_Movement_84014BF
	applymovement 6, Johto_WhirlIslandLugiaChamber_Movement_84014BF
	applymovement 5, Johto_WhirlIslandLugiaChamber_Movement_84014BF
	waitmovement 0
	msgbox Johto_WhirlIslandLugiaChamber_Text_840163A, MSGBOX_DEFAULT
	waitmessage
	setobjectxyperm 1, 0x1D, 0xC
	closemessage
	releaseall
	end

@ Johto_WhirlIslandLugiaChamber object 1 (local id 1, gfx 0x1F9) at 29,7
Johto_WhirlIslandLugiaChamber_EventScript_840118A::
	lockall
	waitse
	playmoncry SPECIES_LUGIA, 2
	delay 0x1E
	waitmoncry
	playbgm MUS_QZ_653, 0
	setwildbattle SPECIES_LUGIA, 0x32, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	setflag FLAG_QZ_0xAD9
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0xAD9
	setvar VAR_LAST_TALKED, 1
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 4, Johto_WhirlIslandLugiaChamber_EventScript_8401238
	goto_if_eq VAR_RESULT, 5, Johto_WhirlIslandLugiaChamber_EventScript_8401238
	call_if_eq VAR_RESULT, 1, Johto_WhirlIslandLugiaChamber_EventScript_840120B
	call_if_eq VAR_RESULT, 7, Johto_WhirlIslandLugiaChamber_EventScript_8401226
	setflag FLAG_QZ_0x1013
	applymovement 4, Johto_WhirlIslandLugiaChamber_Movement_84014CF
	waitmovement 0
	clearflag FLAG_QZ_0x10AD
	clearflag FLAG_QZ_0x10D7
	clearflag FLAG_QZ_0x10D6
	setvar VAR_QZ_0x501A, 8
	setvar VAR_QZ_0x5004, 9
	setvar VAR_QZ_0x5006, 3
	releaseall
	end

Johto_WhirlIslandLugiaChamber_EventScript_840120B::
	fadescreenswapbuffers 1
	removeobject 1
	setweather 0
	doweather
	setflag FLAG_QZ_0x105F
	fadescreenswapbuffers 0
	bufferspeciesname 0, SPECIES_LUGIA
	msgbox Quetzal_Text_83614E5, MSGBOX_DEFAULT
	return

Johto_WhirlIslandLugiaChamber_EventScript_8401226::
	setflag FLAG_QZ_0x1009
	fadescreenswapbuffers 1
	removeobject 1
	setweather 0
	doweather
	setflag FLAG_QZ_0x105F
	fadescreenswapbuffers 0
	return

Johto_WhirlIslandLugiaChamber_EventScript_8401238::
	releaseall
	end

@ Johto_WhirlIslandLugiaChamber object 4 (local id 4, gfx 0x3E9D) at 29,21
Johto_WhirlIslandLugiaChamber_EventScript_840123A::
	goto_if_eq VAR_QZ_0x5006, 3, Johto_WhirlIslandLugiaChamber_EventScript_840124E
	msgbox Johto_WhirlIslandLugiaChamber_Text_84014D7, MSGBOX_NPC
	end

Johto_WhirlIslandLugiaChamber_EventScript_840124E::
	goto_if_unset FLAG_QZ_0x1009, Johto_WhirlIslandLugiaChamber_EventScript_8401260
	msgbox Johto_WhirlIslandLugiaChamber_Text_8401371, MSGBOX_NPC
	end

Johto_WhirlIslandLugiaChamber_EventScript_8401260::
	msgbox Johto_WhirlIslandLugiaChamber_Text_84012DD, MSGBOX_NPC
	end

@ Johto_WhirlIslandLugiaChamber object 3 (local id 3, gfx 0x3E9D) at 28,20
Johto_WhirlIslandLugiaChamber_EventScript_8401269::
	goto_if_eq VAR_QZ_0x5006, 3, Johto_WhirlIslandLugiaChamber_EventScript_840127D
	msgbox Johto_WhirlIslandLugiaChamber_Text_84014EE, MSGBOX_NPC
	end

Johto_WhirlIslandLugiaChamber_EventScript_840127D::
	msgbox Johto_WhirlIslandLugiaChamber_Text_8401785, MSGBOX_NPC
	end

@ Johto_WhirlIslandLugiaChamber object 5 (local id 5, gfx 0x3E9D) at 30,20
Johto_WhirlIslandLugiaChamber_EventScript_8401286::
	goto_if_eq VAR_QZ_0x5006, 3, Johto_WhirlIslandLugiaChamber_EventScript_840129A
	msgbox Johto_WhirlIslandLugiaChamber_Text_8401506, MSGBOX_NPC
	end

Johto_WhirlIslandLugiaChamber_EventScript_840129A::
	msgbox Johto_WhirlIslandLugiaChamber_Text_8401785, MSGBOX_NPC
	end

@ Johto_WhirlIslandLugiaChamber object 2 (local id 2, gfx 0x3E9D) at 27,20
Johto_WhirlIslandLugiaChamber_EventScript_84012A3::
	goto_if_eq VAR_QZ_0x5006, 3, Johto_WhirlIslandLugiaChamber_EventScript_84012B7
	msgbox Johto_WhirlIslandLugiaChamber_Text_840151D, MSGBOX_NPC
	end

Johto_WhirlIslandLugiaChamber_EventScript_84012B7::
	msgbox Johto_WhirlIslandLugiaChamber_Text_8401785, MSGBOX_NPC
	end

@ Johto_WhirlIslandLugiaChamber object 6 (local id 6, gfx 0x3E9D) at 31,20
Johto_WhirlIslandLugiaChamber_EventScript_84012C0::
	goto_if_eq VAR_QZ_0x5006, 3, Johto_WhirlIslandLugiaChamber_EventScript_84012D4
	msgbox Johto_WhirlIslandLugiaChamber_Text_8401534, MSGBOX_NPC
	end

Johto_WhirlIslandLugiaChamber_EventScript_84012D4::
	msgbox Johto_WhirlIslandLugiaChamber_Text_8401785, MSGBOX_NPC
	end

@ Johto_WhirlIslandLugiaChamber object 9 (local id 9, gfx 0x29B3) at 28,18
Johto_WhirlIslandLugiaChamber_EventScript_840EC7C::
	goto_if_set FLAG_QZ_0x1312, Johto_WhirlIslandLugiaChamber_EventScript_840ECED
	goto_if_unset FLAG_QZ_0x1317, Johto_WhirlIslandLugiaChamber_EventScript_840EC95
	lock
	faceplayer
	goto Johto_WhirlIslandLugiaChamber_EventScript_840ECB8

Johto_WhirlIslandLugiaChamber_EventScript_840EC95::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_4961, 0, Johto_WhirlIslandLugiaChamber_Text_840EF30, Johto_WhirlIslandLugiaChamber_Text_840EF92, Johto_WhirlIslandLugiaChamber_EventScript_840ECB0
	msgbox Johto_WhirlIslandLugiaChamber_Text_840EFBB, MSGBOX_AUTOCLOSE
	end

Johto_WhirlIslandLugiaChamber_EventScript_840ECB0::
	setflag FLAG_QZ_0x1317
	goto Johto_WhirlIslandLugiaChamber_EventScript_840ECB8

Johto_WhirlIslandLugiaChamber_EventScript_840ECB8::
	checkitem ITEM_BLUE_ORB, 1
	goto_if_eq VAR_RESULT, 1, Johto_WhirlIslandLugiaChamber_EventScript_840ECDF
	setorcopyvar VAR_0x8000, ITEM_BLUE_ORB
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840ED18

Johto_WhirlIslandLugiaChamber_EventScript_840ECDF::
	msgbox Johto_WhirlIslandLugiaChamber_Text_840EFBB, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1312
	release
	end

Johto_WhirlIslandLugiaChamber_EventScript_840ECED::
	lock
	faceplayer
	msgbox Johto_WhirlIslandLugiaChamber_Text_840F021, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Quetzal_EventScript_840ED15
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4961, 0, Johto_WhirlIslandLugiaChamber_Text_840EF92
	msgbox Johto_WhirlIslandLugiaChamber_Text_840F051, MSGBOX_DEFAULT
	goto Quetzal_EventScript_840ED15  @ falls through in the ROM

Johto_WhirlIslandLugiaChamber_Movement_8401403:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_WhirlIslandLugiaChamber_Movement_840140C:
	delay_16
	delay_16
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401417:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_down
	delay_16
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401420:
	delay_16
	delay_16
	walk_in_place_down
	walk_in_place_down
	walk_in_place_down
	walk_in_place_down
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401427:
	face_right
	delay_16
	walk_right
	delay_16
	face_left
	step_end

Johto_WhirlIslandLugiaChamber_Movement_840142D:
	walk_up
	walk_up
	walk_up
	delay_16
	face_down
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401433:
	walk_left
	delay_16
	face_down
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401437:
	face_right
	delay_16
	face_up
	delay_16
	walk_up
	walk_up
	delay_16
	face_left
	delay_16
	walk_left
	delay_16
	face_down
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401444:
	face_right
	delay_16
	face_up
	delay_16
	walk_up
	walk_up
	delay_16
	face_right
	delay_16
	walk_right
	delay_16
	face_down
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401451:
	face_down
	delay_8
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	delay_8
	delay_8
	delay_8
	delay_8
	delay_8
	delay_16
	face_down
	delay_8
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_840146B:
	face_down
	delay_8
	walk_down
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401472:
	face_right
	delay_8
	walk_right
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401479:
	face_right
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401482:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	walk_left
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_840148D:
	face_right
	delay_8
	walk_right
	walk_right
	walk_right
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_8401495:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	walk_left
	walk_left
	walk_left
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_84014A1:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	walk_left
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_left
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_84014B2:
	face_right
	delay_8
	walk_right
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_right
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_84014BF:
	face_right
	delay_8
	face_up
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_84014C4:
	face_right
	delay_8
	face_up
	delay_8
	walk_up
	delay_8
	face_left
	delay_8
	face_down
	delay_16
	step_end

Johto_WhirlIslandLugiaChamber_Movement_84014CF:
	face_right
	delay_8
	walk_right
	delay_8
	face_down
	delay_16
	face_left
	step_end

Johto_WhirlIslandLugiaChamber_Text_84012DD::
	.string "You have not only made LUGIA\n"
	.string "appear, but you have also defeated\l"
	.string "it...\p"
	.string "There is nothing more for us to\n"
	.string "say. Take care of yourself in your\l"
	.string "journey...$"

Johto_WhirlIslandLugiaChamber_Text_8401371::
	.string "You have not only made LUGIA\n"
	.string "appear, but you have also caught\l"
	.string "it...\p"
	.string "There is nothing more for us to\n"
	.string "say. Take care of yourself in your\l"
	.string "journey...$"

Johto_WhirlIslandLugiaChamber_Text_84014D7::
	.string "ZUKI: Go, face LUGIA. $"

Johto_WhirlIslandLugiaChamber_Text_84014EE::
	.string "NAOKO: Go, face LUGIA. $"

Johto_WhirlIslandLugiaChamber_Text_8401506::
	.string "MIKI: Go, face LUGIA. $"

Johto_WhirlIslandLugiaChamber_Text_840151D::
	.string "SAYO: Go, face LUGIA. $"

Johto_WhirlIslandLugiaChamber_Text_8401534::
	.string "KUNI: Go, face LUGIA. $"

Johto_WhirlIslandLugiaChamber_Text_840154B::
	.string "KIMONO GIRL: That’s right.\n"
	.string "This is where we welcome LUGIA.\p"
	.string "When the dance we practiced many\n"
	.string "days becomes one with the sound of the\l"
	.string "Tidal Bell we entrusted to you, then\l"
	.string "LUGIA shall come out from deep within\l"
	.string "the waterfall basin once again. $"

Johto_WhirlIslandLugiaChamber_Text_840163A::
	.string "That is indeed LUGIA...\n"
	.string "The guardian of these islands from\l"
	.string "ancient times... \p"
	.string "So many have tried again and again,\n"
	.string "only to fail...\p"
	.string "{PLAYER}, your heart in complete\n"
	.string "harmony with the Tidal Bell has finally\l"
	.string "allowed it to appear...\n"
	.string "{PLAYER}, don’t you see?\p"
	.string "LUGIA... It must have been waiting for\n"
	.string "someone like you all this time.\p"
	.string "Wouldn’t you agree? $"

Johto_WhirlIslandLugiaChamber_Text_8401785::
	.string "KIMONO GIRL: You’ve proven yourself\n"
	.string "today, {PLAYER}.$"

Johto_WhirlIslandLugiaChamber_Text_840EF30::
	.string "LUGIA’s sanctuary...\n"
	.string "You can feel the sea’s power here!\p"
	.string "Let’s see if you can withstand\n"
	.string "this tide!$"

Johto_WhirlIslandLugiaChamber_Text_840EF92::
	.string "Even the ocean could not\n"
	.string "sweep you away.$"

Johto_WhirlIslandLugiaChamber_Text_840EFBB::
	.string "This orb responds to your courage.\p"
	.string "KYOGRE’s power is like the sea:\n"
	.string "dangerous if you don’t respect it.$"

Johto_WhirlIslandLugiaChamber_Text_840F021::
	.string "The tide never stops!\n"
	.string "Ready for another battle?$"

Johto_WhirlIslandLugiaChamber_Text_840F051::
	.string "What strength you’ve got!\n"
	.string "Come back whenever you want a battle!$"
