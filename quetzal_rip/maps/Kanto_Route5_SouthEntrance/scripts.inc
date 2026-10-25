Kanto_Route5_SouthEntrance_MapScripts::
	.byte 0

@ Kanto_Route5_SouthEntrance object 1 (local id 1, gfx 0x3F36) at 1,5
Kanto_Route5_SouthEntrance_EventScript_841F876::
	msgbox Kanto_Route5_SouthEntrance_Text_844E60F, MSGBOX_NPC
	end

@ Kanto_Route5_SouthEntrance trigger at 3,5 (VAR_QZ_0x5128 == 0)
Kanto_Route5_SouthEntrance_EventScript_841F87F::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_Route5_SouthEntrance_EventScript_841F8A3

@ Kanto_Route5_SouthEntrance trigger at 4,5 (VAR_QZ_0x5128 == 0)
Kanto_Route5_SouthEntrance_EventScript_841F88B::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_Route5_SouthEntrance_EventScript_841F8A3

@ Kanto_Route5_SouthEntrance trigger at 5,5 (VAR_QZ_0x5128 == 0)
Kanto_Route5_SouthEntrance_EventScript_841F897::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_Route5_SouthEntrance_EventScript_841F8A3

Kanto_Route5_SouthEntrance_EventScript_841F8A3::
	textcolor 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	goto_if_set FLAG_QZ_0x2165, Kanto_Route5_SouthEntrance_EventScript_841F8CD
	msgbox Kanto_Route5_SouthEntrance_Text_844E4ED, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_Route5_SouthEntrance_Movement_841F939
	waitmovement 0
	releaseall
	end

Kanto_Route5_SouthEntrance_EventScript_841F8CD::
	removeitem ITEM_TEA, 1
	goto Kanto_Route5_SouthEntrance_EventScript_841F8D8

Kanto_Route5_SouthEntrance_EventScript_841F8D8::
	msgbox Kanto_Route5_SouthEntrance_Text_844E53D, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_Route5_SouthEntrance_EventScript_841F911
	call_if_eq VAR_TEMP_1, 1, Kanto_Route5_SouthEntrance_EventScript_841F91C
	call_if_eq VAR_TEMP_1, 2, Kanto_Route5_SouthEntrance_EventScript_841F927
	msgbox Kanto_Route5_SouthEntrance_Text_844E563, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5128, 1
	releaseall
	end

Kanto_Route5_SouthEntrance_EventScript_841F911::
	applymovement LOCALID_PLAYER, Kanto_Route5_SouthEntrance_Movement_841F937
	waitmovement 0
	return

Kanto_Route5_SouthEntrance_EventScript_841F91C::
	applymovement LOCALID_PLAYER, Kanto_Route5_SouthEntrance_Movement_841F932
	waitmovement 0
	return

Kanto_Route5_SouthEntrance_EventScript_841F927::
	applymovement LOCALID_PLAYER, Kanto_Route5_SouthEntrance_Movement_841F934
	waitmovement 0
	return

Kanto_Route5_SouthEntrance_Movement_841F932:
	walk_left
	step_end

Kanto_Route5_SouthEntrance_Movement_841F934:
	walk_left
	walk_left
	step_end

Kanto_Route5_SouthEntrance_Movement_841F937:
	walk_in_place_left
	step_end

Kanto_Route5_SouthEntrance_Movement_841F939:
	walk_up
	step_end

Kanto_Route5_SouthEntrance_Text_844E4ED::
	.string "I’m on guard duty.\n"
	.string "Gee, I’m thirsty, though!\p"
	.string "Oh, wait there, the road’s closed.$"

Kanto_Route5_SouthEntrance_Text_844E53D::
	.string "Oh, that TEA…\n"
	.string "It looks awfully tasty…$"

Kanto_Route5_SouthEntrance_Text_844E563::
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

Kanto_Route5_SouthEntrance_Text_844E60F::
	.string "Hi, how’s it going?$"
