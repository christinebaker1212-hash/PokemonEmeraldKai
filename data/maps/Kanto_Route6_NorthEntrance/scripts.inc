Kanto_Route6_NorthEntrance_MapScripts::
	.byte 0

@ Kanto_Route6_NorthEntrance object 1 (local id 1, gfx 0x3F36) at 7,5
Kanto_Route6_NorthEntrance_EventScript_841F93C::
	msgbox Kanto_Route6_NorthEntrance_Text_844E745, MSGBOX_NPC
	end

@ Kanto_Route6_NorthEntrance trigger at 3,5 (VAR_QZ_0x5128 == 0)
Kanto_Route6_NorthEntrance_EventScript_841F945::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_Route6_NorthEntrance_EventScript_841F969

@ Kanto_Route6_NorthEntrance trigger at 4,5 (VAR_QZ_0x5128 == 0)
Kanto_Route6_NorthEntrance_EventScript_841F951::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_Route6_NorthEntrance_EventScript_841F969

@ Kanto_Route6_NorthEntrance trigger at 5,5 (VAR_QZ_0x5128 == 0)
Kanto_Route6_NorthEntrance_EventScript_841F95D::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_Route6_NorthEntrance_EventScript_841F969

Kanto_Route6_NorthEntrance_EventScript_841F969::
	textcolor 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	goto_if_set FLAG_QZ_0x2165, Kanto_Route6_NorthEntrance_EventScript_841F993
	msgbox Kanto_Route6_NorthEntrance_Text_844E623, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_Route6_NorthEntrance_Movement_841F9FF
	waitmovement 0
	releaseall
	end

Kanto_Route6_NorthEntrance_EventScript_841F993::
	removeitem ITEM_TEA, 1
	goto Kanto_Route6_NorthEntrance_EventScript_841F99E

Kanto_Route6_NorthEntrance_EventScript_841F99E::
	msgbox Kanto_Route6_NorthEntrance_Text_844E673, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_Route6_NorthEntrance_EventScript_841F9D7
	call_if_eq VAR_TEMP_1, 1, Kanto_Route6_NorthEntrance_EventScript_841F9E2
	call_if_eq VAR_TEMP_1, 2, Kanto_Route6_NorthEntrance_EventScript_841F9ED
	msgbox Kanto_Route6_NorthEntrance_Text_844E699, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5128, 1
	releaseall
	end

Kanto_Route6_NorthEntrance_EventScript_841F9D7::
	applymovement LOCALID_PLAYER, Kanto_Route6_NorthEntrance_Movement_841F9F8
	waitmovement 0
	return

Kanto_Route6_NorthEntrance_EventScript_841F9E2::
	applymovement LOCALID_PLAYER, Kanto_Route6_NorthEntrance_Movement_841F9FB
	waitmovement 0
	return

Kanto_Route6_NorthEntrance_EventScript_841F9ED::
	applymovement LOCALID_PLAYER, Kanto_Route6_NorthEntrance_Movement_841F9FD
	waitmovement 0
	return

Kanto_Route6_NorthEntrance_Movement_841F9F8:
	walk_right
	walk_right
	step_end

Kanto_Route6_NorthEntrance_Movement_841F9FB:
	walk_right
	step_end

Kanto_Route6_NorthEntrance_Movement_841F9FD:
	walk_in_place_right
	step_end

Kanto_Route6_NorthEntrance_Movement_841F9FF:
	walk_down
	step_end

Kanto_Route6_NorthEntrance_Text_844E623::
	.string "I’m on guard duty.\n"
	.string "Gee, I’m thirsty, though!\p"
	.string "Oh, wait there, the road’s closed.$"

Kanto_Route6_NorthEntrance_Text_844E673::
	.string "Oh, that TEA…\n"
	.string "It looks awfully tasty…$"

Kanto_Route6_NorthEntrance_Text_844E699::
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

Kanto_Route6_NorthEntrance_Text_844E745::
	.string "Hi, how’s it going?$"
