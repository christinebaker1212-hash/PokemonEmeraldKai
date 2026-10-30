Kanto_TwoIsland_JoyfulGameCorner_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_TwoIsland_JoyfulGameCorner_EventScript_84212E7
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_TwoIsland_JoyfulGameCorner_OnWarp_8421306
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_TwoIsland_JoyfulGameCorner_OnFrame_842131D
	map_script MAP_SCRIPT_ON_LOAD, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364DFB
	.byte 0

Kanto_TwoIsland_JoyfulGameCorner_OnWarp_8421306:
	map_script_2 VAR_QZ_0x5134, 2, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421318
	map_script_2 VAR_QZ_0x5100, 8, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364DEB
	.2byte 0

Kanto_TwoIsland_JoyfulGameCorner_OnFrame_842131D:
	map_script_2 VAR_QZ_0x5134, 0, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421361
	map_script_2 VAR_QZ_0x5134, 2, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421337
	map_script_2 VAR_QZ_0x5100, 8, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364EDA
	.2byte 0

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364DEB::
	goto_if_eq VAR_0x8007, 0, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364DFA
	turnobject VAR_0x8007, 3

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364DFA::
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364DFB::
	goto_if_eq VAR_QZ_0x4087, 1, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E54
	goto_if_eq VAR_QZ_0x4087, 2, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E54
	goto_if_eq VAR_QZ_0x4087, 5, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E54
	goto_if_eq VAR_QZ_0x4087, 3, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E5A
	goto_if_eq VAR_QZ_0x4087, 4, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E60
	goto_if_eq VAR_QZ_0x4087, 6, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E66
	goto_if_eq VAR_QZ_0x4087, 7, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E6C
	goto_if_eq VAR_QZ_0x4087, 8, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E72
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E54::
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E00
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E5A::
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E00
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E60::
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E00
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E66::
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365DDA
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E6C::
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E00
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364E72::
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E26
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364EDA::
	lockall
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364EF2
	call Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E39
	special DrawWholeMapView
	playse SE_CLICK
	erasebox 0, 0, 0x1D, 0x13
	releaseall
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8364EF2::
	special CloseLink
	setvar VAR_QZ_0x4087, 0
	goto_if_eq VAR_0x8007, 0, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365012
	applymovement VAR_0x8007, Kanto_TwoIsland_JoyfulGameCorner_Movement_836558D
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_TwoIsland_JoyfulGameCorner_Movement_836558F
	waitmovement 0
	applymovement VAR_0x8007, Kanto_TwoIsland_JoyfulGameCorner_Movement_8365589
	waitmovement 0
	return

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365012::
	applymovement LOCALID_PLAYER, Kanto_TwoIsland_JoyfulGameCorner_Movement_836558F
	waitmovement 0
	return

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365DDA::
	setmetatile 5, 2, 0x2DC, 0
	setmetatile 5, 3, 0x2E4, 0
	return

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E00::
	setmetatile 9, 2, 0x2DC, 0
	setmetatile 9, 3, 0x2E4, 0
	return

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E26::
	setmetatile 5, 2, 0x22C, 0
	setmetatile 5, 3, 0x234, 0
	return

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8365E39::
	setmetatile 5, 2, 0x22A, 1
	setmetatile 5, 3, 0x232, 1
	return

@ Kanto_TwoIsland_JoyfulGameCorner sign / bg event at 1,1
Kanto_TwoIsland_JoyfulGameCorner_EventScript_8366028::
	lockall
	releaseall
	end

@ Kanto_TwoIsland_JoyfulGameCorner sign / bg event at 0,1
Kanto_TwoIsland_JoyfulGameCorner_EventScript_836602B::
	lockall
	releaseall
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_84212E7::
	call_if_set FLAG_QZ_0x2157, Kanto_TwoIsland_JoyfulGameCorner_EventScript_84212F1
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_84212F1::
	setobjectxyperm 1, 6, 2
	setobjectmovementtype 1, 8
	setobjectmovementtype 3, 1
	setvar VAR_QZ_0x5134, 4
	return

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421318::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421337::
	lockall
	textcolor 0
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452E6F, MSGBOX_DEFAULT
	textcolor 1
	special GetPlayerBigGuyGirlString
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452EEE, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	setvar VAR_QZ_0x5134, 3
	releaseall
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421361::
	lockall
	textcolor 0
	applymovement 1, Kanto_TwoIsland_JoyfulGameCorner_Movement_8421422
	waitmovement 0
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_84529A4, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AB0
	waitmovement 0
	playse SE_PIN
	applymovement 1, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 1, Quetzal_Movement_8457AA0
	waitmovement 0
	applymovement 1, Kanto_TwoIsland_JoyfulGameCorner_Movement_8421429
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452A01, MSGBOX_DEFAULT
	closemessage
	playse SE_DOOR
	waitse
	applymovement LOCALID_PLAYER, Kanto_TwoIsland_JoyfulGameCorner_Movement_842142E
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_BOY, 0
	addobject 2
	applymovement 2, Kanto_TwoIsland_JoyfulGameCorner_Movement_8421431
	waitmovement 0
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452B36, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AAE
	waitmovement 0
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452BF4, MSGBOX_DEFAULT
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452C71, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	playse SE_EXIT
	removeobject 2
	fadedefaultbgm
	delay 0x3C
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452CA8, MSGBOX_DEFAULT
	clearflag FLAG_QZ_0x21CF
	setvar VAR_QZ_0x5131, 2
	setvar VAR_QZ_0x5134, 1
	releaseall
	end

@ Kanto_TwoIsland_JoyfulGameCorner object 4 (local id 4, gfx 0x3F1E) at 4,2
Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421437::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5134, 4, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8473750
	goto_if_set FLAG_QZ_0x21EC, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421457
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452DF8, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421457::
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452E3D, MSGBOX_DEFAULT
	release
	end

@ Kanto_TwoIsland_JoyfulGameCorner object 1 (local id 1, gfx 0x3EE6) at 5,5
Kanto_TwoIsland_JoyfulGameCorner_EventScript_8421461::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5134, 4, Kanto_TwoIsland_JoyfulGameCorner_EventScript_8473750
	goto_if_set FLAG_QZ_0x2157, Kanto_TwoIsland_JoyfulGameCorner_EventScript_84214F5
	goto_if_eq VAR_QZ_0x5134, 3, Kanto_TwoIsland_JoyfulGameCorner_EventScript_842148C
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452CDF, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_842148C::
	goto_if_set FLAG_QZ_0x21E0, Kanto_TwoIsland_JoyfulGameCorner_EventScript_84214C1
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452F27, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5117, 2
	textcolor 3
	playfanfare MUS_OBTAIN_TMHM
	message Kanto_TwoIsland_JoyfulGameCorner_Text_8452FB4
	waitmessage
	waitfanfare
	call Quetzal_EventScript_8456AB7
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8452FE1, MSGBOX_DEFAULT
	goto Kanto_TwoIsland_JoyfulGameCorner_EventScript_84214C1

Kanto_TwoIsland_JoyfulGameCorner_EventScript_84214C1::
	checkitemspace ITEM_MOON_STONE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_TwoIsland_JoyfulGameCorner_EventScript_84214FF
	additem ITEM_MOON_STONE, 1
	setflag FLAG_QZ_0x2157
	loadword 0, Kanto_TwoIsland_JoyfulGameCorner_Text_8453057
	setorcopyvar VAR_0x8000, 0xD9
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	release
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_84214F5::
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_84530B2, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_84214FF::
	setflag FLAG_QZ_0x21E0
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8453087, MSGBOX_DEFAULT
	release
	end

@ Kanto_TwoIsland_JoyfulGameCorner object 3 (local id 3, gfx 0x3F18) at 6,5
Kanto_TwoIsland_JoyfulGameCorner_EventScript_842150C::
	lock
	faceplayer
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_84530EB, MSGBOX_DEFAULT
	release
	end

Kanto_TwoIsland_JoyfulGameCorner_EventScript_8473750::
	lockall
	msgbox Kanto_TwoIsland_JoyfulGameCorner_Text_8368568, MSGBOX_DEFAULT
	releaseall
	end

Kanto_TwoIsland_JoyfulGameCorner_Movement_8365589:
	face_down
	step_end

Kanto_TwoIsland_JoyfulGameCorner_Movement_836558D:
	face_left
	step_end

Kanto_TwoIsland_JoyfulGameCorner_Movement_836558F:
	walk_down
	walk_down
	step_end

Kanto_TwoIsland_JoyfulGameCorner_Movement_8421422:
	walk_left
	walk_right
	walk_right
	delay_8
	walk_in_place_faster_up
	delay_16
	step_end

Kanto_TwoIsland_JoyfulGameCorner_Movement_8421429:
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_left
	step_end

Kanto_TwoIsland_JoyfulGameCorner_Movement_842142E:
	walk_left
	walk_in_place_faster_right
	step_end

Kanto_TwoIsland_JoyfulGameCorner_Movement_8421431:
	walk_in_place_faster_left
	delay_8
	walk_in_place_faster_right
	delay_8
	walk_in_place_faster_up
	step_end

Kanto_TwoIsland_JoyfulGameCorner_Text_8368568::
	.string "You can rest here and share stories\n"
	.string "with other TRAINERS.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_84529A4::
	.string "Now, where’s LOSTELLE gotten to\n"
	.string "today?\p"
	.string "She always brings me lunch every\n"
	.string "day right about now…$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452A01::
	.string "Hm? You, there!\n"
	.string "Are you a friend of LOSTELLE’s?\p"
	.string "Have you seen LOSTELLE around?\n"
	.string "I don’t know where she might be.\l"
	.string "She should’ve been here long ago.\p"
	.string "LOSTELLE’s a cutie - she got my\n"
	.string "looks - so what if someone…\p"
	.string "What if something’s happened to\n"
	.string "my LOSTELLE?!\p"
	.string "Please, help me find her!\n"
	.string "Please go search THREE ISLAND!$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452B36::
	.string "Hah? What is this GAME CORNER?\n"
	.string "How much sadder can this get?\p"
	.string "Those clowns…\p"
	.string "They said they’d be waiting on the\n"
	.string "island, so where are they?\p"
	.string "Hey, you! Is this the only THREE\n"
	.string "ISLAND around here?$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452BF4::
	.string "Try waking up before you crawl out\n"
	.string "of bed, you punk.\p"
	.string "This is TWO ISLAND!\p"
	.string "Move it! Get your filthy motorbike\n"
	.string "out of my place!$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452C71::
	.string "Huh…\n"
	.string "Oh, oh, gotcha.\p"
	.string "Tch…\n"
	.string "These islands are confusing…$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452CA8::
	.string "Would you look at that?\n"
	.string "That punk scuffed up my floor.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452CDF::
	.string "Hey, but listen! I’m begging you,\n"
	.string "you have to go to THREE ISLAND.\p"
	.string "If anything were to happen to my\n"
	.string "LOSTELLE…\p"
	.string "My place is the house with the red\n"
	.string "roof on THREE ISLAND.\p"
	.string "She may just show up late here,\n"
	.string "so I can’t run off anywhere.\p"
	.string "That’s why I have to get you to do\n"
	.string "me this big favor.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452DF8::
	.string "There is much to discover on these\n"
	.string "islands. You can rest here first.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452E3D::
	.string "I am glad LOSTELLE is safe!\n"
	.string "Now we can all relax.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452E6F::
	.string "So you rescued LOSTELLE?\n"
	.string "How can I thank you?\p"
	.string "LOSTELLE, darling, forgive me!\p"
	.string "Daddy didn’t know you were scared\n"
	.string "and in trouble!$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452EEE::
	.string "LOSTELLE: It’s okay, Daddy.\n"
	.string "I got to be friends with {PLAYER}!$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452F27::
	.string "Oh, what’s that?\n"
	.string "You’re saying that this is for me?\p"
	.string "How did you know that I love rare\n"
	.string "rocks and gems?\p"
	.string "You sure know how to make a guy\n"
	.string "happy.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452FB4::
	.string "{PLAYER} delivered the sample\n"
	.string "to LOSTELLE’s daddy.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8452FE1::
	.string "Oh, I see, this is from BILL!\n"
	.string "You have to thank him for me!\p"
	.string "You know, you’ve been fantastic.\n"
	.string "I want you to have this.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8453057::
	.string "{PLAYER} received a MOON STONE\n"
	.string "from LOSTELLE’s daddy.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_8453087::
	.string "Your BAG’s not going to fit another\n"
	.string "thing…$"

Kanto_TwoIsland_JoyfulGameCorner_Text_84530B2::
	.string "Thank you for helping us!\n"
	.string "Now we can take a little rest.$"

Kanto_TwoIsland_JoyfulGameCorner_Text_84530EB::
	.string "Listen, listen!\p"
	.string "Please show me you being cool\n"
	.string "again!$"
