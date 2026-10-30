Kanto_Route7_EastEntrance_MapScripts::
	.byte 0

@ Kanto_Route7_EastEntrance object 1 (local id 1, gfx 0x3F36) at 6,2
Kanto_Route7_EastEntrance_EventScript_841FA02::
	msgbox Kanto_Route7_EastEntrance_Text_844E87B, MSGBOX_NPC
	end

@ Kanto_Route7_EastEntrance trigger at 6,4 (VAR_QZ_0x5128 == 0)
Kanto_Route7_EastEntrance_EventScript_841FA0B::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_Route7_EastEntrance_EventScript_841FA2F

@ Kanto_Route7_EastEntrance trigger at 6,5 (VAR_QZ_0x5128 == 0)
Kanto_Route7_EastEntrance_EventScript_841FA17::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_Route7_EastEntrance_EventScript_841FA2F

@ Kanto_Route7_EastEntrance trigger at 6,6 (VAR_QZ_0x5128 == 0)
Kanto_Route7_EastEntrance_EventScript_841FA23::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_Route7_EastEntrance_EventScript_841FA2F

Kanto_Route7_EastEntrance_EventScript_841FA2F::
	textcolor 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	goto_if_set FLAG_QZ_0x2165, Kanto_Route7_EastEntrance_EventScript_841FA59
	msgbox Kanto_Route7_EastEntrance_Text_844E759, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_Route7_EastEntrance_Movement_841FAC5
	waitmovement 0
	releaseall
	end

Kanto_Route7_EastEntrance_EventScript_841FA59::
	removeitem ITEM_TEA, 1
	goto Kanto_Route7_EastEntrance_EventScript_841FA64

Kanto_Route7_EastEntrance_EventScript_841FA64::
	msgbox Kanto_Route7_EastEntrance_Text_844E7A9, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_Route7_EastEntrance_EventScript_841FA9D
	call_if_eq VAR_TEMP_1, 1, Kanto_Route7_EastEntrance_EventScript_841FAA8
	call_if_eq VAR_TEMP_1, 2, Kanto_Route7_EastEntrance_EventScript_841FAB3
	msgbox Kanto_Route7_EastEntrance_Text_844E7CF, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5128, 1
	releaseall
	end

Kanto_Route7_EastEntrance_EventScript_841FA9D::
	applymovement LOCALID_PLAYER, Kanto_Route7_EastEntrance_Movement_841FAC3
	waitmovement 0
	return

Kanto_Route7_EastEntrance_EventScript_841FAA8::
	applymovement LOCALID_PLAYER, Kanto_Route7_EastEntrance_Movement_841FABE
	waitmovement 0
	return

Kanto_Route7_EastEntrance_EventScript_841FAB3::
	applymovement LOCALID_PLAYER, Kanto_Route7_EastEntrance_Movement_841FAC0
	waitmovement 0
	return

Kanto_Route7_EastEntrance_Movement_841FABE:
	walk_up
	step_end

Kanto_Route7_EastEntrance_Movement_841FAC0:
	walk_up
	walk_up
	step_end

Kanto_Route7_EastEntrance_Movement_841FAC3:
	walk_in_place_up
	step_end

Kanto_Route7_EastEntrance_Movement_841FAC5:
	walk_left
	step_end

Kanto_Route7_EastEntrance_Text_844E759::
	.string "I’m on guard duty.\n"
	.string "Gee, I’m thirsty, though!\p"
	.string "Oh, wait there, the road’s closed.$"

Kanto_Route7_EastEntrance_Text_844E7A9::
	.string "Oh, that TEA…\n"
	.string "It looks awfully tasty…$"

Kanto_Route7_EastEntrance_Text_844E7CF::
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

Kanto_Route7_EastEntrance_Text_844E87B::
	.string "Hi, how’s it going?$"
