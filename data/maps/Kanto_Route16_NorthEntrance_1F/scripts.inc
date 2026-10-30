Kanto_Route16_NorthEntrance_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route16_NorthEntrance_1F_EventScript_842023D
	.byte 0

Kanto_Route16_NorthEntrance_1F_EventScript_842023D::
	clearflag FLAG_QZ_0x2210
	setvar VAR_TEMP_1, 0
	checkitem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_NorthEntrance_1F_EventScript_8420276
	checkitem ITEM_MACH_BIKE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_NorthEntrance_1F_EventScript_8420276
	checkitem ITEM_ACRO_BIKE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_NorthEntrance_1F_EventScript_8420276
	end

Kanto_Route16_NorthEntrance_1F_EventScript_8420276::
	setvar VAR_TEMP_1, 1
	end

@ Kanto_Route16_NorthEntrance_1F object 1 (local id 1, gfx 0x3F36) at 6,8
Kanto_Route16_NorthEntrance_1F_EventScript_842027C::
	msgbox Kanto_Route16_NorthEntrance_1F_Text_844FDEB, MSGBOX_NPC
	end

@ Kanto_Route16_NorthEntrance_1F trigger at 6,10 (VAR_TEMP_1 == 0)
Kanto_Route16_NorthEntrance_1F_EventScript_8420285::
	lockall
	setvar VAR_0x8008, 0
	goto Kanto_Route16_NorthEntrance_1F_EventScript_84202C1

@ Kanto_Route16_NorthEntrance_1F trigger at 6,11 (VAR_TEMP_1 == 0)
Kanto_Route16_NorthEntrance_1F_EventScript_8420291::
	lockall
	setvar VAR_0x8008, 1
	goto Kanto_Route16_NorthEntrance_1F_EventScript_84202C1

@ Kanto_Route16_NorthEntrance_1F trigger at 6,12 (VAR_TEMP_1 == 0)
Kanto_Route16_NorthEntrance_1F_EventScript_842029D::
	lockall
	setvar VAR_0x8008, 2
	goto Kanto_Route16_NorthEntrance_1F_EventScript_84202C1

@ Kanto_Route16_NorthEntrance_1F trigger at 6,13 (VAR_TEMP_1 == 0)
Kanto_Route16_NorthEntrance_1F_EventScript_84202A9::
	lockall
	setvar VAR_0x8008, 3
	goto Kanto_Route16_NorthEntrance_1F_EventScript_84202C1

@ Kanto_Route16_NorthEntrance_1F trigger at 6,14 (VAR_TEMP_1 == 0)
Kanto_Route16_NorthEntrance_1F_EventScript_84202B5::
	lockall
	setvar VAR_0x8008, 4
	goto Kanto_Route16_NorthEntrance_1F_EventScript_84202C1

Kanto_Route16_NorthEntrance_1F_EventScript_84202C1::
	textcolor 0
	msgbox Kanto_Route16_NorthEntrance_1F_Text_844FE44, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	delay 0x14
	call_if_eq VAR_0x8008, 1, Kanto_Route16_NorthEntrance_1F_EventScript_842031A
	call_if_eq VAR_0x8008, 2, Kanto_Route16_NorthEntrance_1F_EventScript_8420325
	call_if_eq VAR_0x8008, 3, Kanto_Route16_NorthEntrance_1F_EventScript_8420330
	call_if_eq VAR_0x8008, 4, Kanto_Route16_NorthEntrance_1F_EventScript_842033B
	msgbox Kanto_Route16_NorthEntrance_1F_Text_844FDC1, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_Route16_NorthEntrance_1F_Movement_8420354
	waitmovement 0
	releaseall
	end

Kanto_Route16_NorthEntrance_1F_EventScript_842031A::
	applymovement LOCALID_PLAYER, Kanto_Route16_NorthEntrance_1F_Movement_8420346
	waitmovement 0
	return

Kanto_Route16_NorthEntrance_1F_EventScript_8420325::
	applymovement LOCALID_PLAYER, Kanto_Route16_NorthEntrance_1F_Movement_8420348
	waitmovement 0
	return

Kanto_Route16_NorthEntrance_1F_EventScript_8420330::
	applymovement LOCALID_PLAYER, Kanto_Route16_NorthEntrance_1F_Movement_842034B
	waitmovement 0
	return

Kanto_Route16_NorthEntrance_1F_EventScript_842033B::
	applymovement LOCALID_PLAYER, Kanto_Route16_NorthEntrance_1F_Movement_842034F
	waitmovement 0
	return

@ Kanto_Route16_NorthEntrance_1F object 2 (local id 2, gfx 0x3F2C) at 7,4
Kanto_Route16_NorthEntrance_1F_EventScript_8420356::
	msgbox Kanto_Route16_NorthEntrance_1F_Text_844FE60, MSGBOX_NPC
	end

Kanto_Route16_NorthEntrance_1F_Movement_8420346:
	walk_up
	step_end

Kanto_Route16_NorthEntrance_1F_Movement_8420348:
	walk_up
	walk_up
	step_end

Kanto_Route16_NorthEntrance_1F_Movement_842034B:
	walk_up
	walk_up
	walk_up
	step_end

Kanto_Route16_NorthEntrance_1F_Movement_842034F:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_Route16_NorthEntrance_1F_Movement_8420354:
	walk_right
	step_end

Kanto_Route16_NorthEntrance_1F_Text_844FDC1::
	.string "You need a BICYCLE to enter\n"
	.string "CYCLING ROAD.$"

Kanto_Route16_NorthEntrance_1F_Text_844FDEB::
	.string "You need a BICYCLE to enter.\p"
	.string "Once inside, you can get off\n"
	.string "your bike and explore on foot.$"

Kanto_Route16_NorthEntrance_1F_Text_844FE44::
	.string "Excuse me!\n"
	.string "Wait up, please!$"

Kanto_Route16_NorthEntrance_1F_Text_844FE60::
	.string "How’d you get in?\n"
	.string "Good effort!$"
