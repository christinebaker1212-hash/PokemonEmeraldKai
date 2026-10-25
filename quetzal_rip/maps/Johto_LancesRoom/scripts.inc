Johto_LancesRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_LancesRoom_OnFrame_83E73C6
	map_script MAP_SCRIPT_ON_RESUME, Johto_LancesRoom_EventScript_83E73BD
	.byte 0

Johto_LancesRoom_OnFrame_83E73C6:
	map_script_2 VAR_TEMP_0, 0, Johto_LancesRoom_EventScript_83E73D0
	.2byte 0

Johto_LancesRoom_EventScript_83E73BD::
	setvar VAR_0x8004, 4
	special QzDoPokemonLeagueLightingEffect_672
	end

Johto_LancesRoom_EventScript_83E73D0::
	lock
	setflag FLAG_TEMP_2
	special DrawWholeMapView
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E73E8
	waitmovement 0
	setvar VAR_TEMP_0, 1
	release
	end

@ Johto_LancesRoom trigger at 5,14 (VAR_TEMP_0 == 1)
@ Johto_LancesRoom trigger at 6,14 (VAR_TEMP_0 == 1)
@ Johto_LancesRoom trigger at 7,14 (VAR_TEMP_0 == 1)
Johto_LancesRoom_EventScript_83E73EF::
	lock
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	release
	end

@ Johto_LancesRoom object 1 (local id 1, gfx 0x3EA0) at 6,8
Johto_LancesRoom_EventScript_83E73FC::
	lockall
	call_if_eq VAR_FACING, 4, Johto_LancesRoom_EventScript_83E76CA
	call_if_eq VAR_FACING, 1, Johto_LancesRoom_EventScript_83E76D5
	call_if_eq VAR_FACING, 3, Johto_LancesRoom_EventScript_83E76E0
	faceplayer
	goto_if_set FLAG_QZ_0x11CA, Johto_LancesRoom_EventScript_83E7464
	msgbox Johto_LancesRoom_Text_83E77F9, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_LancesRoom_EventScript_83E7455
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4345, 0, Johto_LancesRoom_Text_83E7937
	goto Johto_LancesRoom_EventScript_83E74A0

Johto_LancesRoom_EventScript_83E7455::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4985, 0, Johto_LancesRoom_Text_83E7937
	goto Johto_LancesRoom_EventScript_83E74A0

Johto_LancesRoom_EventScript_83E7464::
	msgbox Johto_LancesRoom_Text_83E7736, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_LancesRoom_EventScript_83E7491
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4345, 0, Johto_LancesRoom_Text_83E7937
	goto Johto_LancesRoom_EventScript_83E75B5

Johto_LancesRoom_EventScript_83E7491::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4985, 0, Johto_LancesRoom_Text_83E7937
	goto Johto_LancesRoom_EventScript_83E75B5

Johto_LancesRoom_EventScript_83E74A0::
	clearflag FLAG_TEMP_5
	msgbox Johto_LancesRoom_Text_83E79C7, MSGBOX_DEFAULT
	closemessage
	setmetatile 6, 1, 0x3F2, 0
	setmetatile 6, 2, 0x296, 0
	playse SE_BANG
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 4
	setvar VAR_0x8007, 4
	special ShakeCamera
	waitstate
	special DrawWholeMapView
	setflag FLAG_TEMP_4
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x28
	turnobject LOCALID_PLAYER, 1
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E76F9
	applymovement 1, Johto_LancesRoom_Movement_83E76FD
	waitmovement LOCALID_PLAYER
	waitmovement 1
	addobject 2
	addobject 3
	applymovement 2, Johto_LancesRoom_Movement_83E7701
	waitmovement 0
	msgbox Johto_LancesRoom_Text_83E7A76, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_LancesRoom_Movement_83E770D
	delay 0x3C
	applymovement 2, Johto_LancesRoom_Movement_83E770A
	waitmovement 2
	waitmovement 3
	msgbox Johto_LancesRoom_Text_83E7AB5, MSGBOX_DEFAULT
	applymovement 2, Johto_LancesRoom_Movement_83E7715
	waitmovement 0
	msgbox Johto_LancesRoom_Text_83E7CE4, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_LancesRoom_Movement_83E7718
	waitmovement 0
	msgbox Johto_LancesRoom_Text_83E7D4D, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_LancesRoom_Movement_83E7720
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E7728
	waitmovement 1
	removeobject 1
	playse SE_EXIT
	waitmovement LOCALID_PLAYER
	playse SE_EXIT
	waitse
	delay 0x32
	applymovement 2, Johto_LancesRoom_Movement_83E7730
	waitmovement 0
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	delay 0x3C
	msgbox Johto_LancesRoom_Text_83E7D97, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x502A, 6
	warpsilent MAP_JOHTO_HALL_OF_FAME, 0, -1, -1
	waitstate
	end

Johto_LancesRoom_EventScript_83E75B5::
	clearflag FLAG_TEMP_5
	msgbox Johto_LancesRoom_Text_83E79C7, MSGBOX_DEFAULT
	closemessage
	setmetatile 6, 1, 0x3F2, 0
	setmetatile 6, 2, 0x296, 0
	playse SE_BANG
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 4
	setvar VAR_0x8007, 4
	special ShakeCamera
	waitstate
	special DrawWholeMapView
	setflag FLAG_TEMP_4
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x28
	turnobject LOCALID_PLAYER, 1
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E76F9
	applymovement 1, Johto_LancesRoom_Movement_83E76FD
	waitmovement LOCALID_PLAYER
	waitmovement 1
	addobject 2
	addobject 3
	applymovement 2, Johto_LancesRoom_Movement_83E7701
	waitmovement 0
	msgbox Johto_LancesRoom_Text_83E7A76, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_LancesRoom_Movement_83E770D
	delay 0x3C
	applymovement 2, Johto_LancesRoom_Movement_83E770A
	waitmovement 2
	waitmovement 3
	msgbox Johto_LancesRoom_Text_83E7BEF, MSGBOX_DEFAULT
	applymovement 2, Johto_LancesRoom_Movement_83E7715
	waitmovement 0
	msgbox Johto_LancesRoom_Text_83E7D18, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_LancesRoom_Movement_83E7718
	waitmovement 0
	msgbox Johto_LancesRoom_Text_83E7D4D, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_LancesRoom_Movement_83E7720
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E7728
	waitmovement 1
	removeobject 1
	playse SE_EXIT
	waitmovement LOCALID_PLAYER
	playse SE_EXIT
	waitse
	delay 0x32
	applymovement 2, Johto_LancesRoom_Movement_83E7730
	waitmovement 0
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	delay 0x3C
	msgbox Johto_LancesRoom_Text_83E7DC6, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x502A, 6
	warpsilent MAP_JOHTO_HALL_OF_FAME, 0, -1, -1
	waitstate
	end

Johto_LancesRoom_EventScript_83E76CA::
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E76EB
	waitmovement 0
	return

Johto_LancesRoom_EventScript_83E76D5::
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E76EF
	waitmovement 0
	return

Johto_LancesRoom_EventScript_83E76E0::
	applymovement LOCALID_PLAYER, Johto_LancesRoom_Movement_83E76F5
	waitmovement 0
	return

Johto_LancesRoom_Movement_83E73E8:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_LancesRoom_Movement_83E76EB:
	walk_down
	walk_right
	face_up
	step_end

Johto_LancesRoom_Movement_83E76EF:
	walk_left
	walk_down
	walk_down
	walk_right
	face_up
	step_end

Johto_LancesRoom_Movement_83E76F5:
	walk_down
	walk_left
	face_up
	step_end

Johto_LancesRoom_Movement_83E76F9:
	lock_facing_direction
	walk_up
	unlock_facing_direction
	step_end

Johto_LancesRoom_Movement_83E76FD:
	lock_facing_direction
	walk_right
	unlock_facing_direction
	step_end

Johto_LancesRoom_Movement_83E7701:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	delay_2
	walk_in_place_fast_down
	step_end

Johto_LancesRoom_Movement_83E770A:
	walk_left
	face_right
	step_end

Johto_LancesRoom_Movement_83E770D:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_LancesRoom_Movement_83E7715:
	walk_fast_up
	walk_in_place_fast_right
	step_end

Johto_LancesRoom_Movement_83E7718:
	face_up
	delay_16
	delay_16
	face_down
	delay_16
	delay_16
	face_left
	step_end

Johto_LancesRoom_Movement_83E7720:
	walk_up
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_LancesRoom_Movement_83E7728:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	set_invisible
	step_end

Johto_LancesRoom_Movement_83E7730:
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Johto_LancesRoom_Text_83E7736::
	.string "LANCE: Welcome back!\p"
	.string "You conquered the JOHTO LEAGUE once,\n"
	.string "but a CHAMPION never stops testing\l"
	.string "their strength.\p"
	.string "My team is stronger than before.\p"
	.string "Show me how much you’ve grown since\n"
	.string "our last battle!$"

Johto_LancesRoom_Text_83E77F9::
	.string "LANCE: I’ve been waiting for you.\p"
	.string "{PLAYER}!\p"
	.string "I knew that you, with your skills,\n"
	.string "would eventually reach me here.\p"
	.string "There’s no need for words now.\p"
	.string "We will battle to determine who is\n"
	.string "the stronger of the two of us.\p"
	.string "As the most powerful trainer and as\n"
	.string "the POKéMON LEAGUE CHAMPION…\p"
	.string "I, LANCE the dragon master, accept\n"
	.string "your challenge!$"

Johto_LancesRoom_Text_83E7937::
	.string "…It’s over.\p"
	.string "But it’s an odd feeling.\p"
	.string "I’m not angry that I lost. In fact,\n"
	.string "I feel happy.\p"
	.string "Happy that I witnessed the rise\n"
	.string "of a great new CHAMPION!$"

Johto_LancesRoom_Text_83E79C7::
	.string "…Whew.\p"
	.string "You have become truly powerful,\n"
	.string "{PLAYER}.\p"
	.string "Your POKéMON have responded to your\n"
	.string "strong and upstanding nature.\p"
	.string "As a trainer, you will continue to\n"
	.string "grow strong with your POKéMON.$"

Johto_LancesRoom_Text_83E7A76::
	.string "MARY: Oh, no! It’s all over!\p"
	.string "PROF.OAK, if you weren’t so slow…$"

Johto_LancesRoom_Text_83E7AB5::
	.string "PROF.OAK: Ah, {PLAYER}!\p"
	.string "It’s been a long while.\p"
	.string "You certainly look more impressive.\p"
	.string "Your conquest of the JOHTO LEAGUE\n"
	.string "is just fantastic!\p"
	.string "Your dedication, trust and love for\n"
	.string "your POKéMON made this happen.\p"
	.string "Your POKéMON were outstanding too.\p"
	.string "Because they believed in you as a\n"
	.string "trainer, they persevered.\p"
	.string "Congratulations, {PLAYER}!$"

Johto_LancesRoom_Text_83E7BEF::
	.string "PROF.OAK: Ah, {PLAYER}!\p"
	.string "I’ve followed your progress since\n"
	.string "PROF.ELM sent you on your way.\p"
	.string "Conquering the JOHTO LEAGUE again\n"
	.string "is no small feat.\p"
	.string "It’s clear that your POKéMON place\n"
	.string "their complete trust in you.\p"
	.string "Congratulations on another great\n"
	.string "victory, {PLAYER}!$"

Johto_LancesRoom_Text_83E7CE4::
	.string "MARY: Let’s interview the brand new\n"
	.string "JOHTO CHAMPION!$"

Johto_LancesRoom_Text_83E7D18::
	.string "MARY: Let’s interview the JOHTO\n"
	.string "CHAMPION once again!$"

Johto_LancesRoom_Text_83E7D4D::
	.string "LANCE: This is getting to be a bit\n"
	.string "too noisy…\p"
	.string "{PLAYER}, could you come with me?$"

Johto_LancesRoom_Text_83E7D97::
	.string "MARY: Oh, wait! We haven’t done\n"
	.string "the interview!$"

Johto_LancesRoom_Text_83E7DC6::
	.string "MARY: Come on! You can’t blow off\n"
	.string "the interview AGAIN!$"
