Kanto_ViridianCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ViridianCity_EventScript_8415762
	.byte 0

Kanto_ViridianCity_EventScript_8415762::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2264
	call_if_eq VAR_QZ_0x513C, 0, Kanto_ViridianCity_EventScript_84157AC
	call_if_eq VAR_QZ_0x513C, 1, Kanto_ViridianCity_EventScript_841579B
	call_if_ge VAR_QZ_0x513C, 2, Kanto_ViridianCity_EventScript_8415795
	call_if_eq VAR_QZ_0x513A, 0, Kanto_ViridianCity_EventScript_84157BD
	end

Kanto_ViridianCity_EventScript_8415795::
	setvar VAR_QZ_0x513E, 0x3F2C
	return

Kanto_ViridianCity_EventScript_841579B::
	setvar VAR_QZ_0x513E, 0x3F2C
	setobjectxyperm 4, 0x15, 8
	setobjectmovementtype 4, 1
	return

Kanto_ViridianCity_EventScript_84157AC::
	setvar VAR_QZ_0x513E, 0x3F2E
	setobjectxyperm 4, 0x15, 0xB
	setobjectmovementtype 4, 8
	return

Kanto_ViridianCity_EventScript_84157BD::
	goto_if_unset FLAG_QZ_0x2101, Quetzal_EventScript_8457C6C
	goto_if_unset FLAG_QZ_0x2102, Quetzal_EventScript_8457C6C
	goto_if_unset FLAG_QZ_0x2103, Quetzal_EventScript_8457C6C
	goto_if_unset FLAG_QZ_0x2104, Quetzal_EventScript_8457C6C
	goto_if_unset FLAG_QZ_0x2105, Quetzal_EventScript_8457C6C
	goto_if_unset FLAG_QZ_0x2106, Quetzal_EventScript_8457C6C
	setvar VAR_QZ_0x513A, 1
	return

@ Kanto_ViridianCity trigger at 36,11 (VAR_QZ_0x513A == 0)
Kanto_ViridianCity_EventScript_84157F9::
	lockall
	textcolor 3
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	delay 0x14
	msgbox Kanto_ViridianCity_Text_842E60D, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_ViridianCity_Movement_841581E
	waitmovement 0
	releaseall
	end

@ Kanto_ViridianCity sign / bg event at 20,16
Kanto_ViridianCity_EventScript_8415820::
	msgbox Kanto_ViridianCity_Text_842E4D3, MSGBOX_SIGN
	end

@ Kanto_ViridianCity sign / bg event at 23,1
Kanto_ViridianCity_EventScript_8415829::
	msgbox Kanto_ViridianCity_Text_842E4FF, MSGBOX_SIGN
	end

@ Kanto_ViridianCity sign / bg event at 20,31
Kanto_ViridianCity_EventScript_8415832::
	msgbox Kanto_ViridianCity_Text_842E565, MSGBOX_SIGN
	end

@ Kanto_ViridianCity sign / bg event at 32,10
Kanto_ViridianCity_EventScript_841583B::
	msgbox Kanto_ViridianCity_Text_842E5F3, MSGBOX_SIGN
	end

@ Kanto_ViridianCity sign / bg event at 36,10
Kanto_ViridianCity_EventScript_8415844::
	msgbox Kanto_ViridianCity_Text_842E60D, MSGBOX_SIGN
	end

@ Kanto_ViridianCity object 7 (local id 7, gfx 0x3EEE) at 16,22
Kanto_ViridianCity_EventScript_841584D::
	msgbox Kanto_ViridianCity_Text_842E15B, MSGBOX_NPC
	end

@ Kanto_ViridianCity object 3 (local id 3, gfx 0x3F2C) at 34,11
Kanto_ViridianCity_EventScript_8415856::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x513A, 1, Kanto_ViridianCity_EventScript_8415878
	msgbox Kanto_ViridianCity_Text_842E1D9, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_ViridianCity_EventScript_8415878::
	msgbox Kanto_ViridianCity_Text_842E218, MSGBOX_DEFAULT
	release
	end

@ Kanto_ViridianCity object 4 (local id 4, gfx 0xF0) at 21,6
Kanto_ViridianCity_EventScript_8415882::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x513C, 2, Kanto_ViridianCity_EventScript_84158A6
	goto_if_eq VAR_QZ_0x513C, 1, Kanto_ViridianCity_EventScript_84158B0
	goto_if_eq VAR_QZ_0x513C, 0, Kanto_ViridianCity_EventScript_84158B7
	end

Kanto_ViridianCity_EventScript_84158A6::
	msgbox Kanto_ViridianCity_Text_842E3EE, MSGBOX_DEFAULT
	release
	end

Kanto_ViridianCity_EventScript_84158B0::
	call Kanto_ViridianCity_EventScript_841597F
	release
	end

Kanto_ViridianCity_EventScript_84158B7::
	msgbox Kanto_ViridianCity_Text_842E3A3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Kanto_ViridianCity object 6 (local id 6, gfx 0x3F5C) at 33,26
Kanto_ViridianCity_EventScript_84158C2::
	lock
	faceplayer
	msgbox Kanto_ViridianCity_Text_842E238, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_ViridianCity_EventScript_84158E3
	goto_if_eq VAR_RESULT, 0, Kanto_ViridianCity_EventScript_84158ED
	end

Kanto_ViridianCity_EventScript_84158E3::
	msgbox Kanto_ViridianCity_Text_842E284, MSGBOX_DEFAULT
	release
	end

Kanto_ViridianCity_EventScript_84158ED::
	msgbox Kanto_ViridianCity_Text_842E275, MSGBOX_DEFAULT
	release
	end

@ Kanto_ViridianCity object 5 (local id 5, gfx 0x3F59) at 20,12
Kanto_ViridianCity_EventScript_84158F7::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x513C, 0, Kanto_ViridianCity_EventScript_841590E
	msgbox Kanto_ViridianCity_Text_842E339, MSGBOX_DEFAULT
	release
	end

Kanto_ViridianCity_EventScript_841590E::
	msgbox Kanto_ViridianCity_Text_842E2EE, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_ViridianCity object 1 (local id 1, gfx 0x3F04) at 8,26
Kanto_ViridianCity_EventScript_8415923::
	goto Kanto_ViridianCity_EventScript_8471C8A

@ Kanto_ViridianCity trigger at 22,11 (VAR_QZ_0x513C == 0)
Kanto_ViridianCity_EventScript_8415929::
	lockall
	textcolor 0
	msgbox Kanto_ViridianCity_Text_842E3A3, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_ViridianCity_Movement_8415941
	waitmovement 0
	releaseall
	end

@ Kanto_ViridianCity trigger at 20,8 (VAR_QZ_0x513C == 1)
Kanto_ViridianCity_EventScript_8415943::
	lockall
	textcolor 0
	applymovement 4, Quetzal_Movement_8457AAA
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	call Kanto_ViridianCity_EventScript_841597F
	release
	end

@ Kanto_ViridianCity trigger at 22,8 (VAR_QZ_0x513C == 1)
Kanto_ViridianCity_EventScript_8415961::
	lockall
	textcolor 0
	applymovement 4, Quetzal_Movement_8457AAE
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	call Kanto_ViridianCity_EventScript_841597F
	release
	end

Kanto_ViridianCity_EventScript_841597F::
	msgbox Kanto_ViridianCity_Text_842E3EE, MSGBOX_DEFAULT
	setvar VAR_QZ_0x513C, 2
	return

@ Kanto_ViridianCity object 9 (local id 9, gfx 0x3F0E) at 17,5
Kanto_ViridianCity_EventScript_846C072::
	setorcopyvar VAR_0x8000, 0x1C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_ViridianCity_EventScript_8471C8A::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_ViridianCity_Movement_841581E:
	jump_2_down
	step_end

Kanto_ViridianCity_Movement_8415941:
	walk_down
	step_end

Kanto_ViridianCity_Text_842E15B::
	.string "Those POKé BALLS at your waist!\n"
	.string "You have POKéMON, don’t you?\p"
	.string "It’s great that you can carry and\n"
	.string "use POKéMON anytime, anywhere.$"

Kanto_ViridianCity_Text_842E1D9::
	.string "This POKéMON GYM is always closed.\p"
	.string "I wonder who the LEADER is?$"

Kanto_ViridianCity_Text_842E218::
	.string "VIRIDIAN GYM’s LEADER returned!$"

Kanto_ViridianCity_Text_842E238::
	.string "You want to know about the two\n"
	.string "kinds of caterpillar POKéMON?$"

Kanto_ViridianCity_Text_842E275::
	.string "Oh, okay then!$"

Kanto_ViridianCity_Text_842E284::
	.string "CATERPIE has no poison,\n"
	.string "but WEEDLE does.\p"
	.string "Watch that your POKéMON aren’t\n"
	.string "stabbed by WEEDLE’s POISON STING.$"

Kanto_ViridianCity_Text_842E2EE::
	.string "Oh, Grandpa!\n"
	.string "Don’t be so mean!\p"
	.string "I’m so sorry.\n"
	.string "He hasn’t had his coffee yet.$"

Kanto_ViridianCity_Text_842E339::
	.string "I go shopping in PEWTER CITY\n"
	.string "occasionally.\p"
	.string "I have to take the winding trail in\n"
	.string "VIRIDIAN FOREST when I go.$"

Kanto_ViridianCity_Text_842E3A3::
	.string "I absolutely forbid you from\n"
	.string "going through here!\p"
	.string "This is private property!$"

Kanto_ViridianCity_Text_842E3EE::
	.string "Well, now, I’ve had my coffee, and\n"
	.string "that’s what I need to get going!\p"
	.string "But I made it too strong.\n"
	.string "It gave me a headache…\p"
	.string "Incidentally, are you filling your\n"
	.string "POKéDEX?\p"
	.string "At first, focus on weakening the\n"
	.string "POKéMON before trying to catch it.$"

Kanto_ViridianCity_Text_842E4D3::
	.string "VIRIDIAN CITY \n"
	.string "The Eternally Green Paradise$"

Kanto_ViridianCity_Text_842E4FF::
	.string "TRAINER TIPS\p"
	.string "Catch POKéMON and expand your\n"
	.string "collection.\p"
	.string "The more you have, the easier it\n"
	.string "is to battle.$"

Kanto_ViridianCity_Text_842E565::
	.string "TRAINER TIPS\p"
	.string "The battle moves of POKéMON are\n"
	.string "limited by their POWER POINTS, PP.\p"
	.string "To replenish PP, rest your tired\n"
	.string "POKéMON at a POKéMON CENTER.$"

Kanto_ViridianCity_Text_842E5F3::
	.string "VIRIDIAN CITY POKéMON GYM$"

Kanto_ViridianCity_Text_842E60D::
	.string "VIRIDIAN GYM’s doors are locked…$"
