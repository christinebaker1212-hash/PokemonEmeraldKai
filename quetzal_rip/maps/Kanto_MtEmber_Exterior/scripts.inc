Kanto_MtEmber_Exterior_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_MtEmber_Exterior_EventScript_84136A5
	map_script MAP_SCRIPT_ON_LOAD, Kanto_MtEmber_Exterior_EventScript_84136C2
	.byte 0

Kanto_MtEmber_Exterior_EventScript_84136A5::
	setflag FLAG_QZ_0x2241
	call_if_eq VAR_QZ_0x5114, 2, Kanto_MtEmber_Exterior_EventScript_84136B9
	call Quetzal_EventScript_840FBB5
	end

Kanto_MtEmber_Exterior_EventScript_84136B9::
	setobjectmovementtype 2, 8
	setobjectmovementtype 3, 8
	return

Kanto_MtEmber_Exterior_EventScript_84136C2::
	call_if_ge VAR_QZ_0x5117, 4, Kanto_MtEmber_Exterior_EventScript_84136CE
	end

Kanto_MtEmber_Exterior_EventScript_84136CE::
	goto_if_unset FLAG_QZ_0x2108, Quetzal_EventScript_8457C6C
	setmetatile 0x2A, 0x27, 0x346, 0
	return

@ Kanto_MtEmber_Exterior object 2 (local id 2, gfx 0x3F3D) at 41,40
Kanto_MtEmber_Exterior_EventScript_84136E1::
	lock
	goto_if_unset FLAG_QZ_0x2108, Kanto_MtEmber_Exterior_EventScript_84136FF
	checktrainerflag 0x2219
	goto_if_eq Kanto_MtEmber_Exterior_EventScript_8413709
	goto_if_eq VAR_QZ_0x5117, 4, Kanto_MtEmber_Exterior_EventScript_8413713

Kanto_MtEmber_Exterior_EventScript_84136FF::
	msgbox Kanto_MtEmber_Exterior_Text_842AF33, MSGBOX_DEFAULT
	release
	end

Kanto_MtEmber_Exterior_EventScript_8413709::
	msgbox Kanto_MtEmber_Exterior_Text_842B148, MSGBOX_DEFAULT
	release
	end

Kanto_MtEmber_Exterior_EventScript_8413713::
	applymovement 2, Quetzal_Movement_8457AA4
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_ROCKET, 0
	msgbox Kanto_MtEmber_Exterior_Text_842B10F, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8729, 0, Kanto_MtEmber_Exterior_Text_842B13D
	msgbox Kanto_MtEmber_Exterior_Text_842B148, MSGBOX_DEFAULT
	checktrainerflag 0x221A
	goto_if_eq Kanto_MtEmber_Exterior_EventScript_8413746
	release
	end

Kanto_MtEmber_Exterior_EventScript_8413746::
	msgbox Kanto_MtEmber_Exterior_Text_842B1E0, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	removeobject 2
	removeobject 3
	fadescreen 0
	release
	end

@ Kanto_MtEmber_Exterior object 3 (local id 3, gfx 0x3F3D) at 42,40
Kanto_MtEmber_Exterior_EventScript_841375B::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x2108, Kanto_MtEmber_Exterior_EventScript_841377A
	checktrainerflag 0x221A
	goto_if_eq Kanto_MtEmber_Exterior_EventScript_841378F
	goto_if_eq VAR_QZ_0x5117, 4, Kanto_MtEmber_Exterior_EventScript_8413799

Kanto_MtEmber_Exterior_EventScript_841377A::
	msgbox Kanto_MtEmber_Exterior_Text_842AF95, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_MtEmber_Exterior_EventScript_841378F::
	msgbox Kanto_MtEmber_Exterior_Text_842B1A2, MSGBOX_DEFAULT
	release
	end

Kanto_MtEmber_Exterior_EventScript_8413799::
	playbgm MUS_RG_ENCOUNTER_ROCKET, 0
	msgbox Kanto_MtEmber_Exterior_Text_842B15F, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8730, 0, Kanto_MtEmber_Exterior_Text_842B193
	msgbox Kanto_MtEmber_Exterior_Text_842B1A2, MSGBOX_DEFAULT
	checktrainerflag 0x2219
	goto_if_eq Kanto_MtEmber_Exterior_EventScript_8413746
	release
	end

@ Kanto_MtEmber_Exterior trigger at 37,43 (VAR_QZ_0x5114 == 1)
@ Kanto_MtEmber_Exterior trigger at 38,43 (VAR_QZ_0x5114 == 1)
@ Kanto_MtEmber_Exterior trigger at 39,43 (VAR_QZ_0x5114 == 1)
Kanto_MtEmber_Exterior_EventScript_84137C2::
	lockall
	textcolor 0
	msgbox Kanto_MtEmber_Exterior_Text_842AFBD, MSGBOX_DEFAULT
	message Kanto_MtEmber_Exterior_Text_842AFF7
	waitmessage
	playse SE_PIN
	applymovement 3, Quetzal_Movement_8457A9C
	waitmovement 0
	waitbuttonpress
	msgbox Kanto_MtEmber_Exterior_Text_842B079, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	playse SE_PIN
	applymovement 2, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 2, Quetzal_Movement_8457AA0
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	call Kanto_MtEmber_Exterior_EventScript_84136B9
	msgbox Kanto_MtEmber_Exterior_Text_842B0EA, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5114, 2
	releaseall
	end

@ Kanto_MtEmber_Exterior object 5 (local id 5, gfx 0x3EF3) at 15,28
Kanto_MtEmber_Exterior_EventScript_8413828::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8787, 0, Kanto_MtEmber_Exterior_Text_842B27C, Kanto_MtEmber_Exterior_Text_842B2B3
	msgbox Kanto_MtEmber_Exterior_Text_842B2E1, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtEmber_Exterior object 6 (local id 6, gfx 0x3F31) at 14,42
Kanto_MtEmber_Exterior_EventScript_841383F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8789, 0, Kanto_MtEmber_Exterior_Text_842B35E, Kanto_MtEmber_Exterior_Text_842B38B
	msgbox Kanto_MtEmber_Exterior_Text_842B3AB, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtEmber_Exterior object 4 (local id 4, gfx 0x3EE7) at 22,35
Kanto_MtEmber_Exterior_EventScript_8413856::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8784, 0, Kanto_MtEmber_Exterior_Text_842B40F, Kanto_MtEmber_Exterior_Text_842B474
	msgbox Kanto_MtEmber_Exterior_Text_842B492, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtEmber_Exterior object 18 (local id 18, gfx 0x3F0E) at 13,6
Kanto_MtEmber_Exterior_EventScript_846C212::
	setorcopyvar VAR_0x8000, 3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtEmber_Exterior object 19 (local id 19, gfx 0x3F0E) at 38,10
Kanto_MtEmber_Exterior_EventScript_846C21F::
	setorcopyvar VAR_0x8000, 0xD3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtEmber_Exterior object 20 (local id 20, gfx 0x3F0E) at 48,20
Kanto_MtEmber_Exterior_EventScript_846C22C::
	setorcopyvar VAR_0x8000, 0x7F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtEmber_Exterior object 1 (local id 1, gfx 0x3F0C) at 29,45
Kanto_MtEmber_Exterior_EventScript_8471C6F::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_MtEmber_Exterior_Text_842AF33::
	.string "Over here.\n"
	.string "We’ll try digging here.\p"
	.string "That treasure the ADMINS’ve been\n"
	.string "talking about should be here.$"

Kanto_MtEmber_Exterior_Text_842AF95::
	.string "What the…?\n"
	.string "You’re in the way. Get lost.$"

Kanto_MtEmber_Exterior_Text_842AFBD::
	.string "Whew, punched through at last.\n"
	.string "Let’s go treasure hunting!$"

Kanto_MtEmber_Exterior_Text_842AFF7::
	.string "Any treasure we find, we haul back\n"
	.string "to the WAREHOUSE, understood?\p"
	.string "…Oh, and what’re the passwords?\n"
	.string "At the ROCKET WAREHOUSE, I mean.$"

Kanto_MtEmber_Exterior_Text_842B079::
	.string "What, you forgot the password?\n"
	.string "There’re actually two.\p"
	.string "The first one’s “GOLDEEN need\n"
	.string "log.”\p"
	.string "And the second one is…$"

Kanto_MtEmber_Exterior_Text_842B0EA::
	.string "Hey!\n"
	.string "This snoop’s been listening in!$"

Kanto_MtEmber_Exterior_Text_842B10F::
	.string "You’ve been eavesdropping on us,\n"
	.string "haven’t you?$"

Kanto_MtEmber_Exterior_Text_842B13D::
	.string "Huh, what?$"

Kanto_MtEmber_Exterior_Text_842B148::
	.string "Why’d you have to win?$"

Kanto_MtEmber_Exterior_Text_842B15F::
	.string "Trying to horn in on our treasure?\n"
	.string "Don’t bet on it!$"

Kanto_MtEmber_Exterior_Text_842B193::
	.string "Wait!\n"
	.string "But how?$"

Kanto_MtEmber_Exterior_Text_842B1A2::
	.string "Develop amnesia conveniently and\n"
	.string "forget everything you heard!$"

Kanto_MtEmber_Exterior_Text_842B1E0::
	.string "What a setback…\n"
	.string "We’ll have to regroup.\p"
	.string "You! Don’t even think about taking\n"
	.string "a step inside here!\p"
	.string "If you know what’s good for you,\n"
	.string "don’t even think about this!$"

Kanto_MtEmber_Exterior_Text_842B27C::
	.string "An active volcano is one that’s\n"
	.string "erupting, or about to.$"

Kanto_MtEmber_Exterior_Text_842B2B3::
	.string "Whew… That was hot!\n"
	.string "Sweat’s erupting from me!$"

Kanto_MtEmber_Exterior_Text_842B2E1::
	.string "They say there’re over 1,500\n"
	.string "active volcanoes in the world.\p"
	.string "Just thinking about them makes me\n"
	.string "feel all hot and gooey inside!$"

Kanto_MtEmber_Exterior_Text_842B35E::
	.string "Do you know how caves form inside\n"
	.string "volcanoes?$"

Kanto_MtEmber_Exterior_Text_842B38B::
	.string "Atchah!\n"
	.string "You’re shockingly good!$"

Kanto_MtEmber_Exterior_Text_842B3AB::
	.string "When lava at the surface of magma\n"
	.string "hardens, and the magma inside flows\l"
	.string "out, caverns are left behind.$"

Kanto_MtEmber_Exterior_Text_842B40F::
	.string "I’ve been in training so I can mend\n"
	.string "my broken heart…\p"
	.string "It toughened me up physically at\n"
	.string "least! Hi-yah!$"

Kanto_MtEmber_Exterior_Text_842B474::
	.string "I think I tried to overdo it…$"

Kanto_MtEmber_Exterior_Text_842B492::
	.string "The way you battle, it reminds me\n"
	.string "of the guy who dumped me…$"
