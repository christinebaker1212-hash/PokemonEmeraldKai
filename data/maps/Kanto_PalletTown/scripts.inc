Kanto_PalletTown_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PalletTown_EventScript_84152CB
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_PalletTown_OnFrame_8415337
	.byte 0

Kanto_PalletTown_OnFrame_8415337:
	map_script_2 VAR_QZ_0x5118, 2, Kanto_PalletTown_EventScript_8415341
	.2byte 0

Kanto_PalletTown_EventScript_84152CB::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x2245
	call_if_set FLAG_QZ_0x21E8, Kanto_PalletTown_EventScript_84152F1
	call_if_eq VAR_QZ_0x511C, 0, Kanto_PalletTown_EventScript_8415311
	call_if_eq VAR_QZ_0x511C, 1, Kanto_PalletTown_EventScript_841530B
	end

Kanto_PalletTown_EventScript_84152F1::
	goto_if_unset FLAG_QZ_0x21E7, Quetzal_EventScript_8457C6C
	goto_if_ge VAR_QZ_0x511C, 1, Quetzal_EventScript_8457C6C
	setvar VAR_QZ_0x511C, 1
	return

Kanto_PalletTown_EventScript_841530B::
	setvar VAR_QZ_0x511C, 2
	return

Kanto_PalletTown_EventScript_8415311::
	goto_if_set FLAG_QZ_0x21E8, Kanto_PalletTown_EventScript_8415326
	setobjectxyperm 1, 5, 0xF
	setobjectmovementtype 1, 7
	return

Kanto_PalletTown_EventScript_8415326::
	setobjectxyperm 1, 0xC, 2
	setobjectmovementtype 1, 8
	setvar VAR_TEMP_2, 1
	return

Kanto_PalletTown_EventScript_8415341::
	specialvar VAR_RESULT, QzIsNationalPokedexEnabled
	goto_if_eq VAR_RESULT, 1, Kanto_PalletTown_EventScript_84153F7
	lockall
	textcolor 0
	setobjectxyperm 3, 0xE, 0xE
	setobjectmovementtype 3, 9
	addobject 3
	applymovement 3, Kanto_PalletTown_Movement_841542E
	waitmovement 0
	goto_if_unset FLAG_QZ_0x2244, Kanto_PalletTown_EventScript_8415420
	msgbox Kanto_PalletTown_Text_842DF5F, MSGBOX_DEFAULT
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, QzGetPokedexCount
	copyvar VAR_0x8008, VAR_0x8005
	copyvar VAR_0x8009, VAR_0x8006
	buffernumberstring 0, VAR_0x8008
	buffernumberstring 1, VAR_0x8009
	goto_if_lt VAR_0x8009, 60, Kanto_PalletTown_EventScript_8415412
	msgbox Kanto_PalletTown_Text_842E04A, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_RG_FOLLOW_ME, 0
	applymovement LOCALID_PLAYER, Kanto_PalletTown_Movement_841545C
	applymovement 3, Kanto_PalletTown_Movement_841544B
	waitmovement 0
	opendoor 0x10, 0xD
	waitdooranim
	applymovement 3, Kanto_PalletTown_Movement_84155AD
	applymovement LOCALID_PLAYER, Kanto_PalletTown_Movement_84155D7
	waitmovement 0
	closedoor 0x10, 0xD
	waitdooranim
	setvar VAR_QZ_0x511A, 7
	setvar VAR_QZ_0x5118, 3
	setflag FLAG_QZ_0x21A3
	warp MAP_KANTO_PALLET_TOWN_PROFESSOR_OAKS_LAB, 255, 6, 12
	waitstate
	releaseall
	end

Kanto_PalletTown_EventScript_84153F7::
	setvar VAR_QZ_0x5118, 3
	end

Kanto_PalletTown_EventScript_84153FD::
	closemessage
	applymovement 3, Kanto_PalletTown_Movement_841543D
	waitmovement 0
	removeobject 3
	setvar VAR_QZ_0x5118, 3
	releaseall
	end

Kanto_PalletTown_EventScript_8415412::
	msgbox Kanto_PalletTown_Text_842DFC5, MSGBOX_DEFAULT
	goto Kanto_PalletTown_EventScript_84153FD

Kanto_PalletTown_EventScript_8415420::
	msgbox Kanto_PalletTown_Text_842E0B6, MSGBOX_DEFAULT
	goto Kanto_PalletTown_EventScript_84153FD

@ Kanto_PalletTown trigger at 12,1 (VAR_QZ_0x5118 == 0)
Kanto_PalletTown_EventScript_841546C::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_PalletTown_EventScript_8415484

@ Kanto_PalletTown trigger at 13,1 (VAR_QZ_0x5118 == 0)
Kanto_PalletTown_EventScript_8415478::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_PalletTown_EventScript_8415484

Kanto_PalletTown_EventScript_8415484::
	setvar VAR_0x8004, 0
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	textcolor 0
	delay 0x1E
	playbgm MUS_RG_OAK, 0
	message Kanto_PalletTown_Text_842DDCA
	waitmessage
	delay 0x55
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457A9E
	waitmovement 0
	delay 0x1E
	addobject 3
	call_if_eq VAR_TEMP_1, 0, Kanto_PalletTown_EventScript_8415537
	call_if_eq VAR_TEMP_1, 1, Kanto_PalletTown_EventScript_8415542
	delay 0x1E
	msgbox Kanto_PalletTown_Text_842DDE8, MSGBOX_DEFAULT
	closemessage
	delay 0x1E
	call_if_eq VAR_TEMP_1, 0, Kanto_PalletTown_EventScript_841554D
	call_if_eq VAR_TEMP_1, 1, Kanto_PalletTown_EventScript_841555F
	opendoor 0x10, 0xD
	waitdooranim
	applymovement 3, Kanto_PalletTown_Movement_84155AD
	applymovement LOCALID_PLAYER, Kanto_PalletTown_Movement_84155D7
	waitmovement 0
	closedoor 0x10, 0xD
	waitdooranim
	setvar VAR_QZ_0x511A, 1
	clearflag FLAG_QZ_0x21A2
	setvar VAR_QZ_0x5118, 1
	setflag FLAG_QZ_0x21A3
	setflag FLAG_QZ_0x2130
	warp MAP_KANTO_PALLET_TOWN_PROFESSOR_OAKS_LAB, 255, 6, 12
	waitstate
	releaseall
	end

Kanto_PalletTown_EventScript_8415537::
	applymovement 3, Kanto_PalletTown_Movement_8415571
	waitmovement 0
	return

Kanto_PalletTown_EventScript_8415542::
	applymovement 3, Kanto_PalletTown_Movement_841557A
	waitmovement 0
	return

Kanto_PalletTown_EventScript_841554D::
	applymovement 3, Kanto_PalletTown_Movement_8415584
	applymovement LOCALID_PLAYER, Kanto_PalletTown_Movement_84155B0
	waitmovement 0
	return

Kanto_PalletTown_EventScript_841555F::
	applymovement 3, Kanto_PalletTown_Movement_8415598
	applymovement LOCALID_PLAYER, Kanto_PalletTown_Movement_84155C3
	waitmovement 0
	return

@ Kanto_PalletTown object 1 (local id 1, gfx 0x3F57) at 3,10
Kanto_PalletTown_EventScript_84155DB::
	lock
	goto_if_eq VAR_QZ_0x511C, 2, Kanto_PalletTown_EventScript_841566C
	goto_if_eq VAR_QZ_0x511C, 1, Kanto_PalletTown_EventScript_8415694
	goto_if_eq VAR_TEMP_2, 1, Kanto_PalletTown_EventScript_8415741
	goto_if_set FLAG_TEMP_2, Kanto_PalletTown_EventScript_8415680
	msgbox Kanto_PalletTown_Text_8462064, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	playse SE_PIN
	applymovement 1, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 1, Quetzal_Movement_8457AA0
	waitmovement 0
	msgbox Kanto_PalletTown_Text_8462078, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 4, Kanto_PalletTown_EventScript_8415656
	call_if_ne VAR_FACING, 4, Kanto_PalletTown_EventScript_8415661
	copyobjectxytoperm 1
	setflag FLAG_TEMP_2
	release
	end

Kanto_PalletTown_EventScript_8415656::
	applymovement 1, Kanto_PalletTown_Movement_84156A8
	waitmovement 0
	return

Kanto_PalletTown_EventScript_8415661::
	applymovement 1, Kanto_PalletTown_Movement_84156AB
	waitmovement 0
	return

Kanto_PalletTown_EventScript_841566C::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_PalletTown_Text_842DE66, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_EventScript_8415680::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_PalletTown_Text_8462088, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_EventScript_8415694::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_PalletTown_Text_84620C5, MSGBOX_DEFAULT
	release
	end

@ Kanto_PalletTown object 2 (local id 2, gfx 0x3F04) at 13,17
Kanto_PalletTown_EventScript_84156AE::
	msgbox Kanto_PalletTown_Text_842DEAB, MSGBOX_NPC
	end

@ Kanto_PalletTown sign / bg event at 16,16
Kanto_PalletTown_EventScript_84156B7::
	lockall
	setvar VAR_0x8004, 0
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	msgbox Kanto_PalletTown_Text_842DF04, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_PalletTown sign / bg event at 4,7
Kanto_PalletTown_EventScript_84156CF::
	msgbox Kanto_PalletTown_Text_842DF1D, MSGBOX_SIGN
	end

@ Kanto_PalletTown sign / bg event at 13,7
Kanto_PalletTown_EventScript_84156D8::
	msgbox Kanto_PalletTown_Text_842DF28, MSGBOX_SIGN
	end

@ Kanto_PalletTown sign / bg event at 9,11
Kanto_PalletTown_EventScript_84156E1::
	msgbox Kanto_PalletTown_Text_842DF35, MSGBOX_SIGN
	end

@ Kanto_PalletTown sign / bg event at 5,14
Kanto_PalletTown_EventScript_84156EA::
	lockall
	msgbox Kanto_PalletTown_Text_846209A, MSGBOX_DEFAULT
	setvar VAR_QZ_0x511C, 1
	releaseall
	end

@ Kanto_PalletTown trigger at 13,2 (VAR_TEMP_2 == 1)
Kanto_PalletTown_EventScript_84156FA::
	lockall
	applymovement 1, Quetzal_Movement_8457AAE
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	call Kanto_PalletTown_EventScript_8415713
	releaseall
	end

Kanto_PalletTown_EventScript_8415713::
	textcolor 1
	msgbox Kanto_PalletTown_Text_84620E4, MSGBOX_DEFAULT
	closemessage
	delay 0x14
	textcolor 3
	setflag FLAG_QZ_0x21E7
	setvar VAR_QZ_0x511C, 1
	setvar VAR_TEMP_2, 0
	special QzQuestLogNop
	special QzQuestLogNop
	signmsg
	msgbox Kanto_PalletTown_Text_846212A, MSGBOX_DEFAULT
	normalmsg
	return

Kanto_PalletTown_EventScript_8415741::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	call Kanto_PalletTown_EventScript_8415713
	release
	end

Kanto_PalletTown_Movement_841542E:
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

Kanto_PalletTown_Movement_841543D:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PalletTown_Movement_841544B:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

Kanto_PalletTown_Movement_841545C:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PalletTown_Movement_8415571:
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	step_end

Kanto_PalletTown_Movement_841557A:
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	step_end

Kanto_PalletTown_Movement_8415584:
	walk_down
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

Kanto_PalletTown_Movement_8415598:
	walk_down
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

Kanto_PalletTown_Movement_84155AD:
	walk_up
	set_invisible
	step_end

Kanto_PalletTown_Movement_84155B0:
	walk_down
	walk_down
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PalletTown_Movement_84155C3:
	walk_down
	walk_down
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PalletTown_Movement_84155D7:
	walk_right
	walk_up
	set_invisible
	step_end

Kanto_PalletTown_Movement_84156A8:
	walk_right
	walk_in_place_faster_left
	step_end

Kanto_PalletTown_Movement_84156AB:
	walk_left
	walk_in_place_faster_right
	step_end

Kanto_PalletTown_Text_842DDCA::
	.string "OAK: Hey! Wait!\n"
	.string "Don’t go out!$"

Kanto_PalletTown_Text_842DDE8::
	.string "OAK: It’s unsafe!\n"
	.string "Wild POKéMON live in tall grass!\p"
	.string "You need your own POKéMON for\n"
	.string "your protection.\p"
	.string "I know!\n"
	.string "Here, come with me!$"

Kanto_PalletTown_Text_842DE66::
	.string "I’m raising POKéMON, too.\p"
	.string "When they get strong, they can\n"
	.string "protect me.$"

Kanto_PalletTown_Text_842DEAB::
	.string "Technology is incredible!\p"
	.string "You can now store and recall items\n"
	.string "and POKéMON as data via PC.$"

Kanto_PalletTown_Text_842DF04::
	.string "OAK POKéMON RESEARCH LAB$"

Kanto_PalletTown_Text_842DF1D::
	.string "{PLAYER}’s house$"

Kanto_PalletTown_Text_842DF28::
	.string "BLUE’s house$"

Kanto_PalletTown_Text_842DF35::
	.string "PALLET TOWN\n"
	.string "Shades of your journey await!$"

Kanto_PalletTown_Text_842DF5F::
	.string "OAK: Ah, {PLAYER}!\n"
	.string "You’re back, are you?\p"
	.string "How much have you filled in your\n"
	.string "POKéDEX?\p"
	.string "May I see it?\p"
	.string "Let’s see…$"

Kanto_PalletTown_Text_842DFC5::
	.string "You’ve caught {STR_VAR_2}…\p"
	.string "Hm, it looks as if you’re putting\n"
	.string "in an honest effort.\p"
	.string "When you manage to fill it some\n"
	.string "more, come show me, please.$"

Kanto_PalletTown_Text_842E04A::
	.string "You’ve caught… {STR_VAR_2}!?\n"
	.string "Now, this is impressive!\p"
	.string "There’s something I wanted to ask\n"
	.string "of you, {PLAYER}.\p"
	.string "Come.\n"
	.string "Follow me.$"

Kanto_PalletTown_Text_842E0B6::
	.string "OAK: Ah, {PLAYER}!\n"
	.string "You seem to be enjoying traveling.\p"
	.string "Knowing you, {PLAYER}, I can easily\n"
	.string "imagine you going out to even more\l"
	.string "exotic locales.\p"
	.string "Good for you, good for you.\n"
	.string "Hohoho.$"

Kanto_PalletTown_Text_8462064::
	.string "Hmm…\n"
	.string "Is that right…$"

Kanto_PalletTown_Text_8462078::
	.string "Oh!\n"
	.string "Look, look!$"

Kanto_PalletTown_Text_8462088::
	.string "Read it, read it!$"

Kanto_PalletTown_Text_846209A::
	.string "TRAINER TIPS\p"
	.string "Press START to open the MENU!$"

Kanto_PalletTown_Text_84620C5::
	.string "Signs are useful, aren’t they?$"

Kanto_PalletTown_Text_84620E4::
	.string "Look, look!\p"
	.string "I copied what it said on one of\n"
	.string "those TRAINER TIPS signs!$"

Kanto_PalletTown_Text_846212A::
	.string "TRAINER TIPS!\p"
	.string "Press START to open the MENU!$"
