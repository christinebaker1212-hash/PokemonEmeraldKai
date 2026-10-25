Kanto_FourIsland_IcefallCave_Back_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FourIsland_IcefallCave_Back_EventScript_8413B31
	.byte 0

Kanto_FourIsland_IcefallCave_Back_EventScript_8413B31::
	call_if_eq VAR_QZ_0x5112, 1, Kanto_FourIsland_IcefallCave_Back_EventScript_8413B3D
	end

Kanto_FourIsland_IcefallCave_Back_EventScript_8413B3D::
	setflag FLAG_QZ_0x218E
	return

@ Kanto_FourIsland_IcefallCave_Back trigger at 11,18 (VAR_QZ_0x5112 == 0)
@ Kanto_FourIsland_IcefallCave_Back trigger at 12,18 (VAR_QZ_0x5112 == 0)
@ Kanto_FourIsland_IcefallCave_Back trigger at 13,18 (VAR_QZ_0x5112 == 0)
Kanto_FourIsland_IcefallCave_Back_EventScript_8413B41::
	lockall
	textcolor 1
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842B758, MSGBOX_DEFAULT
	applymovement 1, Kanto_FourIsland_IcefallCave_Back_Movement_8413C91
	waitmovement 0
	textcolor 0
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842B7C2, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 3, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AA0
	waitmovement 0
	textcolor 1
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842B812, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Quetzal_Movement_8457AAE
	waitmovement 0
	delay 0x12
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	delay 0x23
	applymovement 4, Kanto_FourIsland_IcefallCave_Back_Movement_8413CBA
	applymovement 3, Kanto_FourIsland_IcefallCave_Back_Movement_8413CC1
	applymovement LOCALID_PLAYER, Kanto_FourIsland_IcefallCave_Back_Movement_8413C86
	waitmovement 0
	applymovement 2, Kanto_FourIsland_IcefallCave_Back_Movement_8413C91
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_ROCKET, 0
	textcolor 0
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842B914, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 4
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8731, 0, Kanto_FourIsland_IcefallCave_Back_Text_842B95A
	applymovement 3, Kanto_FourIsland_IcefallCave_Back_Movement_8413CC4
	waitmovement 0
	textcolor 1
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842B972, MSGBOX_DEFAULT
	textcolor 0
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842B9F8, MSGBOX_DEFAULT
	textcolor 1
	message Kanto_FourIsland_IcefallCave_Back_Text_842BA1C
	waitmessage
	waitse
	playmoncry SPECIES_LAPRAS, 2
	waitbuttonpress
	waitmoncry
	applymovement 1, Kanto_FourIsland_IcefallCave_Back_Movement_8413C97
	waitmovement 0
	textcolor 0
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842BAA2, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Kanto_FourIsland_IcefallCave_Back_Movement_8413CC6
	applymovement LOCALID_PLAYER, Kanto_FourIsland_IcefallCave_Back_Movement_8413C8B
	applymovement 1, Kanto_FourIsland_IcefallCave_Back_Movement_8413C99
	applymovement 2, Kanto_FourIsland_IcefallCave_Back_Movement_8413CA3
	applymovement 4, Kanto_FourIsland_IcefallCave_Back_Movement_8413CAE
	waitmovement 0
	removeobject 1
	removeobject 2
	removeobject 4
	delay 0x32
	applymovement 3, Kanto_FourIsland_IcefallCave_Back_Movement_8413CCC
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	textcolor 1
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842BB60, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x21B2
	clearflag FLAG_QZ_0x2198
	setvar VAR_QZ_0x5112, 1
	releaseall
	end

@ Kanto_FourIsland_IcefallCave_Back object 3 (local id 3, gfx 0x3F19) at 12,16
Kanto_FourIsland_IcefallCave_Back_EventScript_8413CCE::
	msgbox Kanto_FourIsland_IcefallCave_Back_Text_842BB60, MSGBOX_NPC
	end

Kanto_FourIsland_IcefallCave_Back_Movement_8413C86:
	walk_up
	walk_up
	walk_right
	walk_up
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413C8B:
	delay_16
	walk_in_place_faster_left
	delay_16
	delay_16
	walk_in_place_faster_down
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413C91:
	walk_in_place_down
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413C97:
	walk_in_place_fast_down
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413C99:
	walk_right
	walk_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413CA3:
	delay_16
	walk_left
	walk_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413CAE:
	delay_16
	walk_down
	delay_16
	walk_down
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413CBA:
	delay_16
	delay_8
	walk_in_place_faster_right
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413CC1:
	walk_left
	walk_up
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413CC4:
	walk_in_place_up
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413CC6:
	delay_8
	walk_in_place_faster_right
	delay_16
	delay_16
	walk_in_place_faster_down
	step_end

Kanto_FourIsland_IcefallCave_Back_Movement_8413CCC:
	walk_right
	step_end

Kanto_FourIsland_IcefallCave_Back_Text_842B758::
	.string "LORELEI: Keep your filthy hands\n"
	.string "off the POKéMON in the cave!\p"
	.string "Do as I say, or you’ll have me to\n"
	.string "answer to!$"

Kanto_FourIsland_IcefallCave_Back_Text_842B7C2::
	.string "Aww, shut it, lady, and leave\n"
	.string "us be.\p"
	.string "Don’t let your glasses get all\n"
	.string "steamed up!$"

Kanto_FourIsland_IcefallCave_Back_Text_842B812::
	.string "LORELEI: {PLAYER}?!\n"
	.string "What are you doing here?\p"
	.string "No, we can catch up later.\n"
	.string "Right now, I need your help.\p"
	.string "Help me kick these poachers out\n"
	.string "before they do anything else.\p"
	.string "They’ve been catching POKéMON\n"
	.string "here, then selling them off!\p"
	.string "Are you ready?\n"
	.string "You take that one, please!$"

Kanto_FourIsland_IcefallCave_Back_Text_842B914::
	.string "W-what?!\p"
	.string "Who says we can’t do what we want\n"
	.string "with the POKéMON we catch?$"

Kanto_FourIsland_IcefallCave_Back_Text_842B95A::
	.string "We didn’t plan on this!$"

Kanto_FourIsland_IcefallCave_Back_Text_842B972::
	.string "LORELEI: Humph.\n"
	.string "So despicably weak.\p"
	.string "You!\n"
	.string "Tell me!\p"
	.string "Where have you taken the captured\n"
	.string "POKéMON?\p"
	.string "I’m smashing your ring once and\n"
	.string "for all!$"

Kanto_FourIsland_IcefallCave_Back_Text_842B9F8::
	.string "N-no way!\n"
	.string "I’m not telling you that!$"

Kanto_FourIsland_IcefallCave_Back_Text_842BA1C::
	.string "LORELEI: If you won’t confess,\n"
	.string "we’ll deep-freeze you.\p"
	.string "My LAPRAS is furious for what\n"
	.string "you’ve done to its friends.\p"
	.string "Go, LAPRAS!\n"
	.string "ICE BEAM…$"

Kanto_FourIsland_IcefallCave_Back_Text_842BAA2::
	.string "Wawaah! Okay!\n"
	.string "I’ll talk!\p"
	.string "The POKéMON are in the ROCKET\n"
	.string "WAREHOUSE on FIVE ISLAND.\p"
	.string "There! I said it!\n"
	.string "We’ll be going now!\p"
	.string "…But I doubt you’ll ever make it\n"
	.string "into the ROCKET WAREHOUSE!\p"
	.string "Heheheheh!$"

Kanto_FourIsland_IcefallCave_Back_Text_842BB60::
	.string "{PLAYER}, thank you.\n"
	.string "But this is awful…\p"
	.string "I was born and raised here on\n"
	.string "these islands.\p"
	.string "I had no idea that those horrible\n"
	.string "criminals were loose here…$"
