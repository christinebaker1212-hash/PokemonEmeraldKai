Kanto_OneIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B9B
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	map_script MAP_SCRIPT_ON_LOAD, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B5A
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_OneIsland_PokemonCenter_1F_OnFrame_8420BF4
	.byte 0

Kanto_OneIsland_PokemonCenter_1F_OnFrame_8420BF4:
	map_script_2 VAR_QZ_0x5117, 0, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BFE
	.2byte 0

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B5A::
	call_if_ge VAR_QZ_0x5117, 5, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B6F
	call_if_set FLAG_QZ_0x220A, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B83
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B6F::
	setmetatile 0xD, 4, 0x35D, 1
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B79::
	setmetatile 0x10, 4, 0x35F, 1
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B83::
	call Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B79
	setmetatile 0xE, 4, 0x35A, 1
	setmetatile 0xF, 4, 0x35B, 1
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B9B::
	setrespawn 0x2E
	call_if_eq VAR_QZ_0x5117, 6, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BC0
	call_if_eq VAR_QZ_0x5117, 0, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BC6
	call_if_eq VAR_QZ_0x5117, 2, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BDD
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BC0::
	setvar VAR_QZ_0x5117, 7
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BC6::
	setobjectxyperm 2, 9, 8
	setobjectmovementtype 2, 7
	setobjectxyperm 3, 0xF, 6
	setobjectmovementtype 3, 7
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BDD::
	setobjectxyperm 2, 0xE, 7
	setobjectmovementtype 2, 9
	setobjectxyperm 3, 0xE, 8
	setobjectmovementtype 3, 9
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420BFE::
	lockall
	textcolor 0
	applymovement LOCALID_PLAYER, Kanto_OneIsland_PokemonCenter_1F_Movement_8420D83
	applymovement 2, Kanto_OneIsland_PokemonCenter_1F_Movement_8420D8C
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_845127F, MSGBOX_DEFAULT
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	playse SE_PIN
	applymovement 3, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AAA
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AA0
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451298, MSGBOX_DEFAULT
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84512C8, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AB0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451313, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AAE
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	call_if_set FLAG_QZ_0x2108, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420D5F
	call_if_unset FLAG_QZ_0x2108, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420D68
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84513B8, MSGBOX_DEFAULT
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451431, MSGBOX_DEFAULT
	applymovement 3, Quetzal_Movement_8457AAA
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_845146D, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AAC
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84514E1, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451552, MSGBOX_DEFAULT
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451686, MSGBOX_DEFAULT
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84516AF, MSGBOX_DEFAULT
	additem ITEM_TRI_PASS, 1
	loadword 0, Kanto_OneIsland_PokemonCenter_1F_Text_84528B7
	setorcopyvar VAR_0x8000, 0x2F1
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84516EC, MSGBOX_DEFAULT
	call Kanto_OneIsland_PokemonCenter_1F_EventScript_8420D71
	setflag FLAG_QZ_0x2213
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84517CC, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AAC
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AAC
	waitmovement 0
	setflag FLAG_QZ_0x2211
	setvar VAR_QZ_0x5117, 1
	releaseall
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420D5F::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451389, MSGBOX_DEFAULT
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420D68::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_845134D, MSGBOX_DEFAULT
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420D71::
	textcolor 3
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Kanto_OneIsland_PokemonCenter_1F_Text_8451788
	waitmessage
	waitfanfare
	call Quetzal_EventScript_8456AB7
	return

@ Kanto_OneIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 5,2
Kanto_OneIsland_PokemonCenter_1F_EventScript_8420D9A::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_OneIsland_PokemonCenter_1F object 2 (local id 2, gfx 0x3EEA) at 14,6
Kanto_OneIsland_PokemonCenter_1F_EventScript_8420DA3::
	lock
	goto_if_set FLAG_QZ_0x21F1, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420DB7
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451802, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420DB7::
	applymovement 2, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451828, MSGBOX_DEFAULT
	release
	end

@ Kanto_OneIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3EF5) at 15,6
Kanto_OneIsland_PokemonCenter_1F_EventScript_8420DCB::
	lock
	faceplayer
	goto_if_lt VAR_QZ_0x5117, 3, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E24
	goto_if_unset FLAG_QZ_0x2108, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E24
	goto_if_eq VAR_QZ_0x5117, 7, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E44
	goto_if_eq VAR_QZ_0x5117, 6, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420EAA
	goto_if_set FLAG_QZ_0x21EB, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420EB4
	goto_if_eq VAR_QZ_0x5117, 5, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420F46
	goto_if_set FLAG_QZ_0x2161, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420F50
	goto_if_eq VAR_QZ_0x5117, 4, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FFE
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_8421008

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E24::
	goto_if_ge VAR_QZ_0x5117, 3, Kanto_OneIsland_PokemonCenter_1F_EventScript_8421040
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84518BD, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E44::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452459, MSGBOX_DEFAULT
	random 3
	goto_if_eq VAR_RESULT, 0, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E7C
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E93
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452612, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E7C::
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 3
	special QzQuestLogNop_737
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_845248F, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420E93::
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 4
	special QzQuestLogNop_737
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452518, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420EAA::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84523D5, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420EB4::
	textcolor 3
	playfanfare MUS_OBTAIN_TMHM
	message Kanto_OneIsland_PokemonCenter_1F_Text_84521C9
	waitmessage
	waitfanfare
	call Quetzal_EventScript_8456AB7
	removeitem ITEM_SAPPHIRE, 1
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84521EA, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Kanto_OneIsland_PokemonCenter_1F_Movement_842104E
	waitmovement 0
	playse SE_CLICK
	call Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B79
	special DrawWholeMapView
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84522E1, MSGBOX_DEFAULT
	closemessage
	delay 0x3C
	applymovement 3, Kanto_OneIsland_PokemonCenter_1F_Movement_842104A
	waitmovement 0
	delay 0x1E
	applymovement 3, Kanto_OneIsland_PokemonCenter_1F_Movement_842104A
	waitmovement 0
	delay 0x28
	playse SE_PC_ON
	call Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B83
	special DrawWholeMapView
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84522F5, MSGBOX_DEFAULT
	applymovement 3, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452319, MSGBOX_DEFAULT
	special QzClearSeagallopRestrictions_750
	setflag FLAG_QZ_0x220A
	setflag FLAG_QZ_0x217D
	setflag FLAG_QZ_0x2198
	setvar VAR_QZ_0x5117, 6
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420F46::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452153, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420F50::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451F1A, MSGBOX_DEFAULT
	textcolor 3
	playfanfare MUS_OBTAIN_TMHM
	message Kanto_OneIsland_PokemonCenter_1F_Text_8451F29
	waitmessage
	waitfanfare
	call Quetzal_EventScript_8456AB7
	closemessage
	applymovement 3, Kanto_OneIsland_PokemonCenter_1F_Movement_842104E
	waitmovement 0
	playse SE_CLICK
	call Kanto_OneIsland_PokemonCenter_1F_EventScript_8420B6F
	special DrawWholeMapView
	delay 0x1E
	applymovement 3, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451F46, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FA5
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FBE

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FA5::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451FA4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FA5
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FBE

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FBE::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451FDB, MSGBOX_DEFAULT
	removeitem ITEM_RUBY, 1
	removeitem ITEM_TRI_PASS, 1
	setvar VAR_QZ_0x5117, 5
	additem ITEM_RAINBOW_PASS, 1
	setflag FLAG_QZ_0x2214
	textcolor 3
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Kanto_OneIsland_PokemonCenter_1F_Text_84520F2
	waitmessage
	waitfanfare
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Kanto_OneIsland_PokemonCenter_1F_Text_845212A
	waitmessage
	waitfanfare
	call Quetzal_EventScript_8456AB7
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_8420F46

Kanto_OneIsland_PokemonCenter_1F_EventScript_8420FFE::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451E32, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8421008::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451C37, MSGBOX_DEFAULT
	applymovement 3, Quetzal_Movement_8457AAC
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451CBF, MSGBOX_DEFAULT
	applymovement 3, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451D7B, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5114, 1
	setvar VAR_QZ_0x5117, 4
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_8421040::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451B88, MSGBOX_DEFAULT
	release
	end

@ Kanto_OneIsland_PokemonCenter_1F object 4 (local id 4, gfx 0x3F17) at 6,8
Kanto_OneIsland_PokemonCenter_1F_EventScript_8421054::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452671, MSGBOX_NPC
	end

@ Kanto_OneIsland_PokemonCenter_1F object 5 (local id 5, gfx 0x3F0C) at 3,7
Kanto_OneIsland_PokemonCenter_1F_EventScript_842105D::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84526C9, MSGBOX_NPC
	end

@ Kanto_OneIsland_PokemonCenter_1F object 6 (local id 6, gfx 0x3EE7) at 7,4
Kanto_OneIsland_PokemonCenter_1F_EventScript_8421066::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x220A, Kanto_OneIsland_PokemonCenter_1F_EventScript_842107B
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452729, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_842107B::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_845276C, MSGBOX_DEFAULT
	release
	end

@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 12,2
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 12,3
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 12,4
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 12,5
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 13,5
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 15,5
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 14,5
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 16,5
@ Kanto_OneIsland_PokemonCenter_1F sign / bg event at 17,5
Kanto_OneIsland_PokemonCenter_1F_EventScript_8421085::
	lockall
	goto_if_set FLAG_QZ_0x220A, Kanto_OneIsland_PokemonCenter_1F_EventScript_84210AC
	goto_if_set FLAG_QZ_0x21F1, Kanto_OneIsland_PokemonCenter_1F_EventScript_84210A2
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84527DB, MSGBOX_DEFAULT
	releaseall
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_84210A2::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452822, MSGBOX_DEFAULT
	releaseall
	end

Kanto_OneIsland_PokemonCenter_1F_EventScript_84210AC::
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8452867, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_OneIsland_PokemonCenter_1F trigger at 12,6 (VAR_QZ_0x5117 == 2)
Kanto_OneIsland_PokemonCenter_1F_EventScript_84210B6::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_84210E6

@ Kanto_OneIsland_PokemonCenter_1F trigger at 12,7 (VAR_QZ_0x5117 == 2)
Kanto_OneIsland_PokemonCenter_1F_EventScript_84210C2::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_84210E6

@ Kanto_OneIsland_PokemonCenter_1F trigger at 12,8 (VAR_QZ_0x5117 == 2)
Kanto_OneIsland_PokemonCenter_1F_EventScript_84210CE::
	lockall
	setvar VAR_TEMP_1, 3
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_84210E6

@ Kanto_OneIsland_PokemonCenter_1F trigger at 12,9 (VAR_QZ_0x5117 == 2)
Kanto_OneIsland_PokemonCenter_1F_EventScript_84210DA::
	lockall
	setvar VAR_TEMP_1, 4
	goto Kanto_OneIsland_PokemonCenter_1F_EventScript_84210E6

Kanto_OneIsland_PokemonCenter_1F_EventScript_84210E6::
	textcolor 0
	playse SE_PIN
	applymovement 2, Quetzal_Movement_8457A9E
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451954, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 1, Kanto_OneIsland_PokemonCenter_1F_EventScript_84211F0
	call_if_eq VAR_TEMP_1, 2, Kanto_OneIsland_PokemonCenter_1F_EventScript_84211FB
	call_if_eq VAR_TEMP_1, 3, Kanto_OneIsland_PokemonCenter_1F_EventScript_8421206
	call_if_eq VAR_TEMP_1, 4, Kanto_OneIsland_PokemonCenter_1F_EventScript_8421211
	delay 0xA
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451967, MSGBOX_DEFAULT
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_84519CD, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AAC
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451A0C, MSGBOX_DEFAULT
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451A7D, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AAA
	waitmovement 0
	delay 0xA
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	delay 0xF
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451A97, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AB0
	waitmovement 0
	delay 0xF
	applymovement 3, Quetzal_Movement_8457AAA
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	msgbox Kanto_OneIsland_PokemonCenter_1F_Text_8451B10, MSGBOX_DEFAULT
	closemessage
	delay 0x19
	applymovement LOCALID_PLAYER, Kanto_OneIsland_PokemonCenter_1F_Movement_8421232
	applymovement 2, Kanto_OneIsland_PokemonCenter_1F_Movement_8421239
	waitmovement 0
	playse SE_EXIT
	setflag FLAG_QZ_0x21A4
	setvar VAR_QZ_0x5117, 3
	clearflag FLAG_QZ_0x217F
	setvar VAR_QZ_0x510A, 3
	setvar VAR_0x8004, 1
	setvar VAR_0x8006, 8
	goto Quetzal_EventScript_8459475

Kanto_OneIsland_PokemonCenter_1F_EventScript_84211F0::
	applymovement LOCALID_PLAYER, Kanto_OneIsland_PokemonCenter_1F_Movement_842121C
	waitmovement 0
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_84211FB::
	applymovement LOCALID_PLAYER, Kanto_OneIsland_PokemonCenter_1F_Movement_8421220
	waitmovement 0
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8421206::
	applymovement LOCALID_PLAYER, Kanto_OneIsland_PokemonCenter_1F_Movement_8421222
	waitmovement 0
	return

Kanto_OneIsland_PokemonCenter_1F_EventScript_8421211::
	applymovement LOCALID_PLAYER, Kanto_OneIsland_PokemonCenter_1F_Movement_8421226
	waitmovement 0
	return

Kanto_OneIsland_PokemonCenter_1F_Movement_8420D83:
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_8420D8C:
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_in_place_faster_right
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_842104A:
	walk_in_place_up
	delay_8
	walk_in_place_up
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_842104E:
	walk_in_place_faster_up
	walk_in_place_up
	delay_16
	walk_in_place_up
	walk_in_place_up
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_842121C:
	walk_right
	walk_down
	walk_in_place_faster_right
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_8421220:
	walk_right
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_8421222:
	walk_right
	walk_up
	walk_in_place_faster_right
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_8421226:
	walk_right
	walk_up
	walk_up
	walk_in_place_faster_right
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_8421232:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	step_end

Kanto_OneIsland_PokemonCenter_1F_Movement_8421239:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	step_end

Kanto_OneIsland_PokemonCenter_1F_Text_845127F::
	.string "BILL: Hey, there!\n"
	.string "CELIO!$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451298::
	.string "CELIO: BILL!\n"
	.string "I can’t believe you came out here.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84512C8::
	.string "BILL: Well, absolutely!\n"
	.string "How’s your research coming along?\p"
	.string "…Oh, wait a sec.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451313::
	.string "{PLAYER}, this is my buddy CELIO.\n"
	.string "He’s one dedicated PC MANIAC!$"

Kanto_OneIsland_PokemonCenter_1F_Text_845134D::
	.string "CELIO, this is {PLAYER}, a rising\n"
	.string "contender as the POKéMON CHAMP!$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451389::
	.string "CELIO, this is {PLAYER}, the\n"
	.string "reigning POKéMON CHAMP!$"

Kanto_OneIsland_PokemonCenter_1F_Text_84513B8::
	.string "CELIO: That’s really impressive.\p"
	.string "I hate to say it, but I have zero\n"
	.string "aptitude for battling.\p"
	.string "Anyways, I’m glad to meet you.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451431::
	.string "BILL: So, bring me up to speed.\n"
	.string "How’s your machine running?$"

Kanto_OneIsland_PokemonCenter_1F_Text_845146D::
	.string "CELIO: It’s running fine, but we’re\n"
	.string "too remote out here.\p"
	.string "The PCs on this island just can’t\n"
	.string "link with your PC, BILL.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84514E1::
	.string "BILL: Oh, yeah?\n"
	.string "Okay, let me take a look-see.\p"
	.string "…Hang on here…\n"
	.string "I think we can make it work.\l"
	.string "Let me help you, okay?$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451552::
	.string "{PLAYER}, can I get you to wait for\n"
	.string "me just a bit?\p"
	.string "…Actually, can I get you to do\n"
	.string "me a favor?\p"
	.string "The island next to this one’s\n"
	.string "called TWO ISLAND.\p"
	.string "There’s a guy there that runs\n"
	.string "a GAME CORNER.\p"
	.string "He has this thing for rare rocks\n"
	.string "and gems.\p"
	.string "We keep in touch, being fellow\n"
	.string "maniacs.\p"
	.string "Could you deliver this rock sample\n"
	.string "to him?$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451686::
	.string "{PLAYER} agrees to deliver\n"
	.string "BILL’s rock sample.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84516AF::
	.string "CELIO: {PLAYER}, if you are going\n"
	.string "to TWO ISLAND, please take this.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84516EC::
	.string "It’s a PASS for the ferry service\n"
	.string "serving the local islands.\p"
	.string "It will let you travel between the\n"
	.string "ISLANDS ONE, TWO, and THREE.\p"
	.string "Oh, you should have this, too.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451788::
	.string "{PLAYER} received an extra page\n"
	.string "for the TOWN MAP!$"

Kanto_OneIsland_PokemonCenter_1F_Text_84517CC::
	.string "BILL: I’ll catch you later!\n"
	.string "Say hi to the guy for me!$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451802::
	.string "Hmm…\p"
	.string "How about we try this like this…$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451828::
	.string "Oh, hey, {PLAYER}!\p"
	.string "Did you see?\n"
	.string "We got the PC working!\p"
	.string "I’ve got a few things to show\n"
	.string "CELIO here.\p"
	.string "Can you go out on a stroll or\n"
	.string "something for a while more?$"

Kanto_OneIsland_PokemonCenter_1F_Text_84518BD::
	.string "I’m sorry for taking up so much of\n"
	.string "BILL’s time.\p"
	.string "I’m also sorry for being such a \n"
	.string "poor host on your visit here.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451954::
	.string "BILL: Oh, hey!\n"
	.string "{PLAYER}!$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451967::
	.string "BILL: What kept you so long?\n"
	.string "Been out having a good time?\p"
	.string "We got it done.\n"
	.string "The PCs are up and running!$"

Kanto_OneIsland_PokemonCenter_1F_Text_84519CD::
	.string "CELIO: The job went incredibly\n"
	.string "quick.\p"
	.string "BILL is one amazing guy…$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451A0C::
	.string "BILL: No, no! There was almost\n"
	.string "nothing left for me to do.\p"
	.string "CELIO, I have to hand it to you.\n"
	.string "You’ve learned a lot.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451A7D::
	.string "CELIO: Oh, really?\n"
	.string "Ehehe…$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451A97::
	.string "BILL: Well, there you have it.\n"
	.string "I’m finished with the job.\l"
	.string "We should head back to KANTO.\p"
	.string "CELIO, I’ll be seeing you again.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451B10::
	.string "CELIO: {PLAYER}, I’m really sorry\n"
	.string "that we sent you off alone today.\p"
	.string "I promise, I will show you around\n"
	.string "these islands sometime.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451B88::
	.string "CELIO: Hello!\n"
	.string "You look awfully busy as always.\p"
	.string "How am I doing?\p"
	.string "Well, I’m modifying my Network\n"
	.string "Machine.\p"
	.string "When I get done with the machine,\n"
	.string "I hope you’ll be first to use it,\l"
	.string "{PLAYER}.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451C37::
	.string "CELIO: {PLAYER}, how have things\n"
	.string "been for you?\p"
	.string "Oh, is that right?\n"
	.string "You’ve caught more POKéMON.\p"
	.string "Do you know what?\n"
	.string "Maybe I can be useful to you.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451CBF::
	.string "I’m modifying the Network Machine\n"
	.string "right now.\p"
	.string "I’m changing it so it can handle\n"
	.string "trades over long distances.\p"
	.string "When I get finished, you’ll be\n"
	.string "trading for exotic POKéMON from\l"
	.string "TRAINERS far away.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451D7B::
	.string "But, there is a slight catch.\p"
	.string "For the link to work, the Machine\n"
	.string "needs a special gemstone.\p"
	.string "It’s supposed to be on ONE ISLAND,\n"
	.string "but I haven’t found one yet.\p"
	.string "Who knows where it could be.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451E32::
	.string "I was trying to find the gem\n"
	.string "even while I was studying.\p"
	.string "As a result, I’ve made no headway\n"
	.string "in both my search and studies…\p"
	.string "If I relied on BILL, I’m sure my\n"
	.string "research would progress.\p"
	.string "But this time, I want to try to do\n"
	.string "things by myself.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451F1A::
	.string "Oh!\n"
	.string "Th-that’s…$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451F29::
	.string "{PLAYER} handed the RUBY\n"
	.string "to CELIO.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451F46::
	.string "Thank you!\n"
	.string "{PLAYER}, you’re simply amazing.\p"
	.string "… … …  … … …\p"
	.string "Um… May I ask one more giant favor\n"
	.string "of you?$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451FA4::
	.string "It… It’s not anything weird.\n"
	.string "Please, I need your help.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8451FDB::
	.string "While I was studying gemstones,\n"
	.string "I discovered something important.\p"
	.string "There is another gem that forms\n"
	.string "a pair with this RUBY.\p"
	.string "That other gemstone is supposed to\n"
	.string "be in the SEVII ISLANDS.\p"
	.string "{PLAYER}, please, I need you to go\n"
	.string "find the other gem.\p"
	.string "{PLAYER}, may I have your ferry\n"
	.string "PASS and the TOWN MAP?$"

Kanto_OneIsland_PokemonCenter_1F_Text_84520F2::
	.string "{PLAYER} returned the TRI-PASS and\n"
	.string "received the RAINBOW PASS.$"

Kanto_OneIsland_PokemonCenter_1F_Text_845212A::
	.string "Obtained an extra page for the\n"
	.string "TOWN MAP!$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452153::
	.string "This is my own ferry PASS.\p"
	.string "It will let you get to all the\n"
	.string "SEVII ISLANDS.\p"
	.string "{PLAYER}, please, I can’t do\n"
	.string "it without your help.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84521C9::
	.string "{PLAYER} handed the SAPPHIRE\n"
	.string "to CELIO.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84521EA::
	.string "CELIO: So this is the gem that\n"
	.string "forms a pair with the RUBY…\p"
	.string "{PLAYER}, you’ve gone through a lot\n"
	.string "to get this, didn’t you?\p"
	.string "You don’t have to tell me. I know\n"
	.string "it wasn’t easy.\p"
	.string "Thank you so much!\p"
	.string "Now it’s my turn to work for you!\n"
	.string "Please give me a little time.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84522E1::
	.string "Okay, this is good…$"

Kanto_OneIsland_PokemonCenter_1F_Text_84522F5::
	.string "I did it!\n"
	.string "I linked up with LANETTE!$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452319::
	.string "{PLAYER}…\n"
	.string "{PLAYER}, I did it!\p"
	.string "I’ve managed to link up with\n"
	.string "TRAINERS in the HOENN region!\p"
	.string "Finally, the Network Machine is\n"
	.string "fully operational!\p"
	.string "{PLAYER}, I owe it all to you!\p"
	.string "Thanks to you, my dream came\n"
	.string "true…$"

Kanto_OneIsland_PokemonCenter_1F_Text_84523D5::
	.string "I…\n"
	.string "I’m not crying.\p"
	.string "That’s enough about me!\p"
	.string "{PLAYER}, you’re going to keep\n"
	.string "looking for exotic POKéMON, right?\p"
	.string "I wish you the best of luck!$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452459::
	.string "CELIO: Hello!\p"
	.string "{PLAYER}, I’ve been hearing rumors\n"
	.string "about you.$"

Kanto_OneIsland_PokemonCenter_1F_Text_845248F::
	.string "{PLAYER}, what’s your favorite kind\n"
	.string "of POKéMON?\p"
	.string "BILL is a POKéMANIAC, so he loves\n"
	.string "every kind.\p"
	.string "Apparently, the first one he caught\n"
	.string "was an ABRA.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452518::
	.string "By the way, {PLAYER}, you’re from\n"
	.string "PALLET TOWN, aren’t you?\p"
	.string "I’ve heard that it’s a quiet and\n"
	.string "pleasant place.\p"
	.string "BILL’s hometown is GOLDENROD CITY,\n"
	.string "where his folks still live.\p"
	.string "I’ve heard that it’s quite the\n"
	.string "festive, bustling city.\p"
	.string "I’d like to go there one day.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452612::
	.string "{PLAYER}, is there anything that\n"
	.string "you can’t stand?\p"
	.string "Apparently, BILL simply can’t\n"
	.string "stomach milk at all.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452671::
	.string "Oh, you’re a stranger here!\n"
	.string "Hi! Where did you come from?\p"
	.string "…PALLET TOWN?\n"
	.string "I don’t know it!$"

Kanto_OneIsland_PokemonCenter_1F_Text_84526C9::
	.string "On this island, there’s an enormous\n"
	.string "volcano.\p"
	.string "It hasn’t erupted lately, so why\n"
	.string "not enjoy a hike?$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452729::
	.string "I wish I could trade POKéMON with\n"
	.string "my boyfriend who lives far away…$"

Kanto_OneIsland_PokemonCenter_1F_Text_845276C::
	.string "I traded POKéMON with my boyfriend\n"
	.string "far away!\p"
	.string "Everyone’s saying that we can\n"
	.string "thank you and CELIO.\p"
	.string "So, thank you!$"

Kanto_OneIsland_PokemonCenter_1F_Text_84527DB::
	.string "Network Machine\n"
	.string "Link Level 0\p"
	.string "…POKéMON Storage System under\n"
	.string "adjustment…$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452822::
	.string "Network Machine\n"
	.string "Link Level 1\p"
	.string "Link established with the KANTO\n"
	.string "region.$"

Kanto_OneIsland_PokemonCenter_1F_Text_8452867::
	.string "Network Machine\n"
	.string "Link Level 2\p"
	.string "Link established with the KANTO\n"
	.string "and HOENN regions.$"

Kanto_OneIsland_PokemonCenter_1F_Text_84528B7::
	.string "Obtained the TRI-PASS!$"
