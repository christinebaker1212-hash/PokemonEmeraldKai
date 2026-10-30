Kanto_Route8_WestEntrance_MapScripts::
	.byte 0

@ Kanto_Route8_WestEntrance object 1 (local id 1, gfx 0x3F36) at 6,2
Kanto_Route8_WestEntrance_EventScript_841FAC8::
	msgbox Kanto_Route8_WestEntrance_Text_844E9B1, MSGBOX_NPC
	end

@ Kanto_Route8_WestEntrance trigger at 6,4 (VAR_QZ_0x5128 == 0)
Kanto_Route8_WestEntrance_EventScript_841FAD1::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_Route8_WestEntrance_EventScript_841FAF5

@ Kanto_Route8_WestEntrance trigger at 6,5 (VAR_QZ_0x5128 == 0)
Kanto_Route8_WestEntrance_EventScript_841FADD::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_Route8_WestEntrance_EventScript_841FAF5

@ Kanto_Route8_WestEntrance trigger at 6,6 (VAR_QZ_0x5128 == 0)
Kanto_Route8_WestEntrance_EventScript_841FAE9::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_Route8_WestEntrance_EventScript_841FAF5

Kanto_Route8_WestEntrance_EventScript_841FAF5::
	textcolor 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	goto_if_set FLAG_QZ_0x2165, Kanto_Route8_WestEntrance_EventScript_841FB1F
	msgbox Kanto_Route8_WestEntrance_Text_844E88F, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_Route8_WestEntrance_Movement_841FBA1
	waitmovement 0
	releaseall
	end

Kanto_Route8_WestEntrance_EventScript_841FB1F::
	removeitem ITEM_TEA, 1
	goto Kanto_Route8_WestEntrance_EventScript_841FB40

Kanto_Route8_WestEntrance_EventScript_841FB40::
	msgbox Kanto_Route8_WestEntrance_Text_844E8DF, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_Route8_WestEntrance_EventScript_841FB79
	call_if_eq VAR_TEMP_1, 1, Kanto_Route8_WestEntrance_EventScript_841FB84
	call_if_eq VAR_TEMP_1, 2, Kanto_Route8_WestEntrance_EventScript_841FB8F
	msgbox Kanto_Route8_WestEntrance_Text_844E905, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5128, 1
	releaseall
	end

Kanto_Route8_WestEntrance_EventScript_841FB79::
	applymovement LOCALID_PLAYER, Kanto_Route8_WestEntrance_Movement_841FB9F
	waitmovement 0
	return

Kanto_Route8_WestEntrance_EventScript_841FB84::
	applymovement LOCALID_PLAYER, Kanto_Route8_WestEntrance_Movement_841FB9A
	waitmovement 0
	return

Kanto_Route8_WestEntrance_EventScript_841FB8F::
	applymovement LOCALID_PLAYER, Kanto_Route8_WestEntrance_Movement_841FB9C
	waitmovement 0
	return

Kanto_Route8_WestEntrance_Movement_841FB9A:
	walk_up
	step_end

Kanto_Route8_WestEntrance_Movement_841FB9C:
	walk_up
	walk_up
	step_end

Kanto_Route8_WestEntrance_Movement_841FB9F:
	walk_in_place_up
	step_end

Kanto_Route8_WestEntrance_Movement_841FBA1:
	walk_right
	step_end

Kanto_Route8_WestEntrance_Text_844E88F::
	.string "I’m on guard duty.\n"
	.string "Gee, I’m thirsty, though!\p"
	.string "Oh, wait there, the road’s closed.$"

Kanto_Route8_WestEntrance_Text_844E8DF::
	.string "Oh, that TEA…\n"
	.string "It looks awfully tasty…$"

Kanto_Route8_WestEntrance_Text_844E905::
	.string "Huh? I can have this drink?\n"
	.string "Gee, thanks!\l"
	.string "… …\l"
	.string "Glug, glug…\l"
	.string "… …\l"
	.string "Gulp…\l"
	.string "If you want to go to SAFFRON\l"
	.string "CITY…\l"
	.string "… …\l"
	.string "You can go on through.\p"
	.string "I’ll share this TEA with the other\n"
	.string "guards!$"

Kanto_Route8_WestEntrance_Text_844E9B1::
	.string "Hi, how’s it going?$"
