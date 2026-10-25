Kanto_SilphCo_7F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_7F_EventScript_84116E5
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SilphCo_7F_EventScript_8411701
	.byte 0

Kanto_SilphCo_7F_EventScript_84116E5::
	call_if_unset FLAG_QZ_0x2200, Kanto_SilphCo_7F_EventScript_845880E
	call_if_unset FLAG_QZ_0x2201, Kanto_SilphCo_7F_EventScript_845883C
	call_if_unset FLAG_QZ_0x2202, Kanto_SilphCo_7F_EventScript_845886A
	end

Kanto_SilphCo_7F_EventScript_8411701::
	call_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_7F_EventScript_841170D
	end

Kanto_SilphCo_7F_EventScript_841170D::
	setobjectmovementtype 8, 7
	return

@ Kanto_SilphCo_7F trigger at 2,4 (VAR_QZ_0x512E == 0)
Kanto_SilphCo_7F_EventScript_8411712::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_SilphCo_7F_EventScript_841172A

@ Kanto_SilphCo_7F trigger at 2,5 (VAR_QZ_0x512E == 0)
Kanto_SilphCo_7F_EventScript_841171E::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_SilphCo_7F_EventScript_841172A

Kanto_SilphCo_7F_EventScript_841172A::
	textcolor 0
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	applymovement 1, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	applymovement 1, Quetzal_Movement_8457AA0
	waitmovement 0
	msgbox Kanto_SilphCo_7F_Text_8426DF1, MSGBOX_DEFAULT
	call_if_eq VAR_TEMP_1, 0, Kanto_SilphCo_7F_EventScript_84117AD
	call_if_eq VAR_TEMP_1, 1, Kanto_SilphCo_7F_EventScript_84117B9
	msgbox Kanto_SilphCo_7F_Text_8426E0A, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 1
	call Kanto_SilphCo_7F_EventScript_84117BA
	msgbox Kanto_SilphCo_7F_Text_8426F0E, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_RG_RIVAL_EXIT, 0
	call_if_eq VAR_TEMP_1, 0, Kanto_SilphCo_7F_EventScript_84117DB
	call_if_eq VAR_TEMP_1, 1, Kanto_SilphCo_7F_EventScript_84117E6
	playse SE_WARP_IN
	fadedefaultbgm
	removeobject 1
	waitse
	setvar VAR_QZ_0x512E, 1
	releaseall
	end

Kanto_SilphCo_7F_EventScript_84117AD::
	closemessage
	applymovement 1, Kanto_SilphCo_7F_Movement_84117F1
	waitmovement 0
	return

Kanto_SilphCo_7F_EventScript_84117B9::
	return

Kanto_SilphCo_7F_EventScript_84117BA::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8624, 0, Kanto_SilphCo_7F_Text_8426EB5
	return

Kanto_SilphCo_7F_EventScript_84117DB::
	applymovement 1, Kanto_SilphCo_7F_Movement_84117F3
	waitmovement 0
	return

Kanto_SilphCo_7F_EventScript_84117E6::
	applymovement 1, Kanto_SilphCo_7F_Movement_84117F9
	waitmovement 0
	return

@ Kanto_SilphCo_7F object 2 (local id 2, gfx 0x3F5B) at 0,7
Kanto_SilphCo_7F_EventScript_8411802::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2155, Kanto_SilphCo_7F_EventScript_84118CC
	call Quetzal_EventScript_8361A4D
	msgbox Kanto_SilphCo_7F_Text_8426947, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, 0x83
	givemon SPECIES_LAPRAS, 0x19, ITEM_NONE
	goto_if_eq VAR_RESULT, 0, Kanto_SilphCo_7F_EventScript_8411851
	goto_if_eq VAR_RESULT, 1, Kanto_SilphCo_7F_EventScript_8411884
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8459563
	release
	end

Kanto_SilphCo_7F_EventScript_8411851::
	textcolor 3
	playfanfare MUS_LEVEL_UP
	message Kanto_SilphCo_7F_Text_84269B8
	waitmessage
	waitfanfare
	bufferspeciesname 0, SPECIES_LAPRAS
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SilphCo_7F_EventScript_84118BD
	call Quetzal_EventScript_84590EA
	call Quetzal_EventScript_84579AE
	goto Kanto_SilphCo_7F_EventScript_84118BD

Kanto_SilphCo_7F_EventScript_8411884::
	textcolor 3
	playfanfare MUS_LEVEL_UP
	message Kanto_SilphCo_7F_Text_84269B8
	waitmessage
	waitfanfare
	bufferspeciesname 0, SPECIES_LAPRAS
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SilphCo_7F_EventScript_84118B2
	call Quetzal_EventScript_84590F6
	goto Kanto_SilphCo_7F_EventScript_84118B2

Kanto_SilphCo_7F_EventScript_84118B2::
	call Quetzal_EventScript_84590FF
	goto Kanto_SilphCo_7F_EventScript_84118BD

Kanto_SilphCo_7F_EventScript_84118BD::
	textcolor 0
	msgbox Kanto_SilphCo_7F_Text_84269E6, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2155
	release
	end

Kanto_SilphCo_7F_EventScript_84118CC::
	msgbox Kanto_SilphCo_7F_Text_8426ABE, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_7F object 7 (local id 7, gfx 0x3F5B) at 13,14
Kanto_SilphCo_7F_EventScript_84118D6::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_7F_EventScript_84118ED
	msgbox Kanto_SilphCo_7F_Text_8426B25, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_7F_EventScript_84118ED::
	msgbox Kanto_SilphCo_7F_Text_8426B67, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_7F object 8 (local id 8, gfx 0x3F5B) at 9,14
Kanto_SilphCo_7F_EventScript_84118F7::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_7F_EventScript_841190E
	msgbox Kanto_SilphCo_7F_Text_8426BA3, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_7F_EventScript_841190E::
	msgbox Kanto_SilphCo_7F_Text_8426BE2, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_7F object 6 (local id 6, gfx 0x3F5A) at 10,10
Kanto_SilphCo_7F_EventScript_8411918::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x512D, 1, Kanto_SilphCo_7F_EventScript_841192F
	msgbox Kanto_SilphCo_7F_Text_8426C13, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_7F_EventScript_841192F::
	msgbox Kanto_SilphCo_7F_Text_8426C54, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_7F sign / bg event at 20,1
Kanto_SilphCo_7F_EventScript_8411939::
	msgbox Kanto_SilphCo_7F_Text_842703B, MSGBOX_SIGN
	end

@ Kanto_SilphCo_7F object 9 (local id 9, gfx 0x3F43) at 3,13
Kanto_SilphCo_7F_EventScript_8411942::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8534, 0, Kanto_SilphCo_7F_Text_8426CC3, Kanto_SilphCo_7F_Text_8426CED
	msgbox Kanto_SilphCo_7F_Text_8426CF7, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_7F object 3 (local id 3, gfx 0x3F3D) at 24,5
Kanto_SilphCo_7F_EventScript_8411959::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8575, 0, Kanto_SilphCo_7F_Text_8426D24, Kanto_SilphCo_7F_Text_8426D4A
	msgbox Kanto_SilphCo_7F_Text_8426D62, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_7F object 4 (local id 4, gfx 0x3F3D) at 24,15
Kanto_SilphCo_7F_EventScript_8411970::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8576, 0, Kanto_SilphCo_7F_Text_8426D94, Kanto_SilphCo_7F_Text_8426DB8
	msgbox Kanto_SilphCo_7F_Text_8426DC6, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_7F object 5 (local id 5, gfx 0x3F3D) at 13,4
Kanto_SilphCo_7F_EventScript_8411987::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8577, 0, Kanto_SilphCo_7F_Text_8426C67, Kanto_SilphCo_7F_Text_8426C86
	msgbox Kanto_SilphCo_7F_Text_8426C92, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_7F_EventScript_845880E::
	setmetatile 0xB, 8, 0x3B0, 1
	setmetatile 0xC, 8, 0x3B1, 1
	setmetatile 0xB, 9, 0x3B8, 1
	setmetatile 0xC, 9, 0x3B9, 1
	setmetatile 0xC, 0xA, 0x335, 0
	return

Kanto_SilphCo_7F_EventScript_845883C::
	setmetatile 0x18, 7, 0x3B0, 1
	setmetatile 0x19, 7, 0x3B1, 1
	setmetatile 0x18, 8, 0x3B8, 1
	setmetatile 0x19, 8, 0x3B9, 1
	setmetatile 0x19, 9, 0x335, 0
	return

Kanto_SilphCo_7F_EventScript_845886A::
	setmetatile 0x19, 0xD, 0x3B0, 1
	setmetatile 0x1A, 0xD, 0x3B1, 1
	setmetatile 0x19, 0xE, 0x3B8, 1
	setmetatile 0x1A, 0xE, 0x3B9, 1
	setmetatile 0x1A, 0xF, 0x335, 0
	return

@ Kanto_SilphCo_7F sign / bg event at 11,8
@ Kanto_SilphCo_7F sign / bg event at 12,8
@ Kanto_SilphCo_7F sign / bg event at 11,9
@ Kanto_SilphCo_7F sign / bg event at 12,9
Kanto_SilphCo_7F_EventScript_8458EE2::
	lockall
	setvar VAR_TEMP_1, 0xB
	setvar VAR_0x8004, 0x2200
	goto_if_set FLAG_QZ_0x2200, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_7F sign / bg event at 24,7
@ Kanto_SilphCo_7F sign / bg event at 25,7
@ Kanto_SilphCo_7F sign / bg event at 25,8
@ Kanto_SilphCo_7F sign / bg event at 24,8
Kanto_SilphCo_7F_EventScript_8458EFC::
	lockall
	setvar VAR_TEMP_1, 0xC
	setvar VAR_0x8004, 0x2201
	goto_if_set FLAG_QZ_0x2201, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_7F sign / bg event at 25,13
@ Kanto_SilphCo_7F sign / bg event at 26,13
@ Kanto_SilphCo_7F sign / bg event at 25,14
@ Kanto_SilphCo_7F sign / bg event at 26,14
Kanto_SilphCo_7F_EventScript_8458F16::
	lockall
	setvar VAR_TEMP_1, 0xD
	setvar VAR_0x8004, 0x2202
	goto_if_set FLAG_QZ_0x2202, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_7F object 10 (local id 10, gfx 0x3F0E) at 0,12
Kanto_SilphCo_7F_EventScript_846BDB1::
	setorcopyvar VAR_0x8000, 0x44
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SilphCo_7F object 11 (local id 11, gfx 0x3F0E) at 30,11
Kanto_SilphCo_7F_EventScript_846BDBE::
	setorcopyvar VAR_0x8000, 0x2FA
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_7F_Movement_84117F1:
	walk_up
	step_end

Kanto_SilphCo_7F_Movement_84117F3:
	walk_right
	walk_right
	walk_right
	walk_up
	delay_16
	step_end

Kanto_SilphCo_7F_Movement_84117F9:
	walk_left
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	delay_16
	step_end

Kanto_SilphCo_7F_Text_8426947::
	.string "Oh! Hi! You’re not a ROCKET!\n"
	.string "You came to save us?\l"
	.string "Why, thank you!\p"
	.string "I want you to have this POKéMON\n"
	.string "for saving us.$"

Kanto_SilphCo_7F_Text_84269B8::
	.string "{PLAYER} obtained a LAPRAS from\n"
	.string "the SILPH employee!$"

Kanto_SilphCo_7F_Text_84269E6::
	.string "It’s a LAPRAS.\n"
	.string "It’s a very intelligent POKéMON.\p"
	.string "We kept it in our lab, but it will\n"
	.string "be much better off with you.\p"
	.string "I think you will be a good TRAINER\n"
	.string "for LAPRAS!\p"
	.string "It’s a good swimmer.\n"
	.string "It’ll give you a lift across water!$"

Kanto_SilphCo_7F_Text_8426ABE::
	.string "TEAM ROCKET’s BOSS went to the\n"
	.string "boardroom!\p"
	.string "Is our PRESIDENT okay?\n"
	.string "I’m worried.$"

Kanto_SilphCo_7F_Text_8426B25::
	.string "TEAM ROCKET was after the MASTER\n"
	.string "BALL, which catches any POKéMON.$"

Kanto_SilphCo_7F_Text_8426B67::
	.string "We canceled the MASTER BALL\n"
	.string "project because of TEAM ROCKET.$"

Kanto_SilphCo_7F_Text_8426BA3::
	.string "It would be bad if TEAM ROCKET\n"
	.string "took over SILPH or our POKéMON.$"

Kanto_SilphCo_7F_Text_8426BE2::
	.string "Wow!\p"
	.string "You chased off TEAM ROCKET all by\n"
	.string "yourself?$"

Kanto_SilphCo_7F_Text_8426C13::
	.string "You!\n"
	.string "It’s really dangerous here!\p"
	.string "You came to save me?\n"
	.string "You can’t!$"

Kanto_SilphCo_7F_Text_8426C54::
	.string "Thank you so much!$"

Kanto_SilphCo_7F_Text_8426C67::
	.string "Uh-oh!\n"
	.string "I smell a little mouse!$"

Kanto_SilphCo_7F_Text_8426C86::
	.string "Lights out!$"

Kanto_SilphCo_7F_Text_8426C92::
	.string "You won’t find my BOSS by just\n"
	.string "scurrying around!$"

Kanto_SilphCo_7F_Text_8426CC3::
	.string "Heheh!\p"
	.string "You mistook me for a SILPH worker?$"

Kanto_SilphCo_7F_Text_8426CED::
	.string "I’m done!$"

Kanto_SilphCo_7F_Text_8426CF7::
	.string "Despite your age, you are a\n"
	.string "skilled TRAINER!$"

Kanto_SilphCo_7F_Text_8426D24::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

Kanto_SilphCo_7F_Text_8426D4A::
	.string "Aack!\n"
	.string "Brothers, I lost!$"

Kanto_SilphCo_7F_Text_8426D62::
	.string "Doesn’t matter.\n"
	.string "My brothers will repay the favor!$"

Kanto_SilphCo_7F_Text_8426D94::
	.string "A child intruder?\n"
	.string "That must be you!$"

Kanto_SilphCo_7F_Text_8426DB8::
	.string "Fine!\n"
	.string "I lost!$"

Kanto_SilphCo_7F_Text_8426DC6::
	.string "Go on home before my BOSS gets\n"
	.string "ticked off!$"

Kanto_SilphCo_7F_Text_8426DF1::
	.string "BLUE: What kept you, {PLAYER}?$"

Kanto_SilphCo_7F_Text_8426E0A::
	.string "BLUE: Hahaha! I thought you’d\n"
	.string "turn up if I waited here!\p"
	.string "I guess TEAM ROCKET slowed you\n"
	.string "down! Not that I care!\p"
	.string "I saw you in SAFFRON, so I decided\n"
	.string "to see if you got better!$"

Kanto_SilphCo_7F_Text_8426EB5::
	.string "Oh, man!\n"
	.string "So, you are ready for BOSS\l"
	.string "ROCKET!$"

Kanto_SilphCo_7F_Text_8426F0E::
	.string "Well, {PLAYER}!\n"
	.string "I’m moving on up and ahead!\p"
	.string "By checking my POKéDEX, I’m\n"
	.string "starting to see what’s strong and\l"
	.string "how they evolve!\p"
	.string "Am I a genius or what?\p"
	.string "I’m going to the POKéMON LEAGUE to\n"
	.string "boot out the ELITE FOUR.\p"
	.string "I’ll become the world’s most\n"
	.string "powerful TRAINER!\p"
	.string "{PLAYER}, well, good luck to you!\n"
	.string "Don’t sweat it!\l"
	.string "Smell ya!$"

Kanto_SilphCo_7F_Text_842703B::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "7F$"
