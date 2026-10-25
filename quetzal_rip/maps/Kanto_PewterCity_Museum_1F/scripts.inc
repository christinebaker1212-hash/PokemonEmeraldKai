Kanto_PewterCity_Museum_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_PewterCity_Museum_1F object 1 (local id 1, gfx 0x3F5B) at 16,5
Kanto_PewterCity_Museum_1F_EventScript_841A31E::
	lock
	faceplayer
	goto_if_eq VAR_FACING, 3, Kanto_PewterCity_Museum_1F_EventScript_841A34B
	goto_if_eq VAR_FACING, 1, Kanto_PewterCity_Museum_1F_EventScript_841A34B
	goto_if_eq VAR_FACING, 2, Kanto_PewterCity_Museum_1F_EventScript_841A34B
	msgbox Kanto_PewterCity_Museum_1F_Text_8440895, MSGBOX_DEFAULT
	release
	end

Kanto_PewterCity_Museum_1F_EventScript_841A34B::
	msgbox Kanto_PewterCity_Museum_1F_Text_84408AC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_PewterCity_Museum_1F_EventScript_841A36A
	goto_if_eq VAR_RESULT, 0, Kanto_PewterCity_Museum_1F_EventScript_841A37E
	end

Kanto_PewterCity_Museum_1F_EventScript_841A36A::
	msgbox Kanto_PewterCity_Museum_1F_Text_844090D, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_PewterCity_Museum_1F_EventScript_841A37E::
	msgbox Kanto_PewterCity_Museum_1F_Text_8440983, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_PewterCity_Museum_1F trigger at 12,5 (VAR_QZ_0x511E == 0)
Kanto_PewterCity_Museum_1F_EventScript_841A392::
	lockall
	setvar VAR_TEMP_1, 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	goto Kanto_PewterCity_Museum_1F_EventScript_841A3D4

@ Kanto_PewterCity_Museum_1F trigger at 13,5 (VAR_QZ_0x511E == 0)
Kanto_PewterCity_Museum_1F_EventScript_841A3A8::
	lockall
	setvar VAR_TEMP_1, 1
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	goto Kanto_PewterCity_Museum_1F_EventScript_841A3D4

@ Kanto_PewterCity_Museum_1F trigger at 14,5 (VAR_QZ_0x511E == 0)
Kanto_PewterCity_Museum_1F_EventScript_841A3BE::
	lockall
	setvar VAR_TEMP_1, 2
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	goto Kanto_PewterCity_Museum_1F_EventScript_841A3D4

Kanto_PewterCity_Museum_1F_EventScript_841A3D4::
	textcolor 0
	showmoneybox 0, 0, 0
	msgbox Kanto_PewterCity_Museum_1F_Text_8440811, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_PewterCity_Museum_1F_EventScript_841A405
	msgbox Kanto_PewterCity_Museum_1F_Text_8440855, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Museum_1F_Movement_841A491
	waitmovement 0
	releaseall
	end

Kanto_PewterCity_Museum_1F_EventScript_841A405::
	checkmoney 0x32, 0
	goto_if_eq VAR_RESULT, 0, Kanto_PewterCity_Museum_1F_EventScript_841A479
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_PewterCity_Museum_1F_EventScript_841A458
	call_if_eq VAR_TEMP_1, 1, Kanto_PewterCity_Museum_1F_EventScript_841A463
	call_if_eq VAR_TEMP_1, 2, Kanto_PewterCity_Museum_1F_EventScript_841A46E
	playse SE_SHOP
	removemoney 0x32, 0
	updatemoneybox 0
	waitse
	msgbox Kanto_PewterCity_Museum_1F_Text_8440861, MSGBOX_DEFAULT
	setvar VAR_QZ_0x511E, 1
	hidemoneybox
	releaseall
	end

Kanto_PewterCity_Museum_1F_EventScript_841A458::
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Museum_1F_Movement_841A493
	waitmovement 0
	return

Kanto_PewterCity_Museum_1F_EventScript_841A463::
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Museum_1F_Movement_841A498
	waitmovement 0
	return

Kanto_PewterCity_Museum_1F_EventScript_841A46E::
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Museum_1F_Movement_841A49C
	waitmovement 0
	return

Kanto_PewterCity_Museum_1F_EventScript_841A479::
	msgbox Kanto_PewterCity_Museum_1F_Text_8440878, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Museum_1F_Movement_841A491
	waitmovement 0
	releaseall
	end

@ Kanto_PewterCity_Museum_1F object 5 (local id 5, gfx 0x3F43) at 26,4
Kanto_PewterCity_Museum_1F_EventScript_841A49F::
	msgbox Kanto_PewterCity_Museum_1F_Text_8440C29, MSGBOX_NPC
	end

@ Kanto_PewterCity_Museum_1F object 2 (local id 2, gfx 0x3F2C) at 2,4
Kanto_PewterCity_Museum_1F_EventScript_841A4A8::
	msgbox Kanto_PewterCity_Museum_1F_Text_8440A10, MSGBOX_NPC
	end

@ Kanto_PewterCity_Museum_1F object 3 (local id 3, gfx 0x3F43) at 21,3
Kanto_PewterCity_Museum_1F_EventScript_841A4B1::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x215A, Kanto_PewterCity_Museum_1F_EventScript_841A505
	msgbox Kanto_PewterCity_Museum_1F_Text_8440A73, MSGBOX_DEFAULT
	checkitemspace ITEM_OLD_AMBER, 1
	goto_if_eq VAR_RESULT, 0, Kanto_PewterCity_Museum_1F_EventScript_841A4FB
	setflag FLAG_QZ_0x215A
	removeobject 4
	additem ITEM_OLD_AMBER, 1
	loadword 0, Kanto_PewterCity_Museum_1F_Text_8440BC2
	setorcopyvar VAR_0x8000, 0xA7
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	release
	end

Kanto_PewterCity_Museum_1F_EventScript_841A4FB::
	msgbox Quetzal_Text_8440C0A, MSGBOX_DEFAULT
	release
	end

Kanto_PewterCity_Museum_1F_EventScript_841A505::
	msgbox Kanto_PewterCity_Museum_1F_Text_8440BEA, MSGBOX_DEFAULT
	release
	end

@ Kanto_PewterCity_Museum_1F object 4 (local id 4, gfx 0x3F2B) at 22,3
Kanto_PewterCity_Museum_1F_EventScript_841A50F::
	msgbox Kanto_PewterCity_Museum_1F_Text_8440C66, MSGBOX_NPC
	end

@ Kanto_PewterCity_Museum_1F sign / bg event at 4,4
Kanto_PewterCity_Museum_1F_EventScript_841A518::
	lockall
	specialvar VAR_0x8007, QzOpenMuseumFossilPicKabutops_775
	msgbox Kanto_PewterCity_Museum_1F_Text_8440CA1, MSGBOX_DEFAULT
	special QzCloseMuseumFossilPic_777
	releaseall
	end

@ Kanto_PewterCity_Museum_1F sign / bg event at 4,7
Kanto_PewterCity_Museum_1F_EventScript_841A52B::
	lockall
	specialvar VAR_0x8007, QzOpenMuseumFossilPicAerodactyl_776
	msgbox Kanto_PewterCity_Museum_1F_Text_8440D12, MSGBOX_DEFAULT
	special QzCloseMuseumFossilPic_777
	releaseall
	end

@ Kanto_PewterCity_Museum_1F sign / bg event at 27,1
@ Kanto_PewterCity_Museum_1F sign / bg event at 26,1
Kanto_PewterCity_Museum_1F_EventScript_845D1ED::
	lockall
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	textcolor 3
	msgbox Kanto_PewterCity_Museum_1F_Text_8461841, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_PewterCity_Museum_1F object 6 (local id 6, gfx 0x3F43) at 26,7
Kanto_PewterCity_Museum_1F_EventScript_8471CAE::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_PewterCity_Museum_1F_Movement_841A491:
	walk_down
	step_end

Kanto_PewterCity_Museum_1F_Movement_841A493:
	walk_right
	walk_right
	delay_8
	walk_in_place_right
	step_end

Kanto_PewterCity_Museum_1F_Movement_841A498:
	walk_right
	delay_8
	walk_in_place_right
	step_end

Kanto_PewterCity_Museum_1F_Movement_841A49C:
	delay_8
	walk_in_place_right
	step_end

Kanto_PewterCity_Museum_1F_Text_8440811::
	.string "Yes, yes.\n"
	.string "It’s ¥50 for a child’s ticket.\p"
	.string "Would you like to come in?$"

Kanto_PewterCity_Museum_1F_Text_8440855::
	.string "Come again!$"

Kanto_PewterCity_Museum_1F_Text_8440861::
	.string "Right, ¥50!\n"
	.string "Thank you!$"

Kanto_PewterCity_Museum_1F_Text_8440878::
	.string "You don’t have enough money.$"

Kanto_PewterCity_Museum_1F_Text_8440895::
	.string "Please enjoy yourself.$"

Kanto_PewterCity_Museum_1F_Text_84408AC::
	.string "You can’t sneak in the back way!\n"
	.string "Nice try, kid, but no.\p"
	.string "Oh, whatever!\n"
	.string "Do you know what AMBER is?$"

Kanto_PewterCity_Museum_1F_Text_844090D::
	.string "AMBER contains the genetic matter\n"
	.string "of ancient POKéMON.\p"
	.string "There’s a LAB somewhere trying to\n"
	.string "resurrect POKéMON from AMBER.$"

Kanto_PewterCity_Museum_1F_Text_8440983::
	.string "AMBER is actually sap, that gooey\n"
	.string "stuff that oozes from trees.\p"
	.string "The ancient sap fossilized over\n"
	.string "time to become rock-hard AMBER.$"

Kanto_PewterCity_Museum_1F_Text_8440A10::
	.string "I should be grateful for my long\n"
	.string "life.\p"
	.string "Never did I think I would get to\n"
	.string "see the bones of a dragon!$"

Kanto_PewterCity_Museum_1F_Text_8440A73::
	.string "Ssh! Listen, I need to share a\n"
	.string "secret with someone.\p"
	.string "I think that this chunk of AMBER\n"
	.string "contains POKéMON DNA!\p"
	.string "It would be a shattering scientific\n"
	.string "breakthrough if POKéMON could be\l"
	.string "resurrected from it.\p"
	.string "But, my colleagues just ignore\n"
	.string "what I have to say.\p"
	.string "So, I have a favor to ask!\p"
	.string "I want you to get this examined at\n"
	.string "a POKéMON LAB somewhere.$"

Kanto_PewterCity_Museum_1F_Text_8440BC2::
	.string "{PLAYER} received the OLD AMBER\n"
	.string "from the man.$"

Kanto_PewterCity_Museum_1F_Text_8440BEA::
	.string "Ssh!\n"
	.string "Get the OLD AMBER checked!$"

Kanto_PewterCity_Museum_1F_Text_8440C29::
	.string "We have two fossils of rare,\n"
	.string "prehistoric POKéMON on exhibit.$"

Kanto_PewterCity_Museum_1F_Text_8440C66::
	.string "There is a beautiful piece of\n"
	.string "AMBER in a clear gold color.$"

Kanto_PewterCity_Museum_1F_Text_8440CA1::
	.string "A reconstruction of Aerodactyl’s\n"
	.string "skeleton.\p"
	.string "This Pokémon soared through the skies\n"
	.string "long before our cities existed.$"

Kanto_PewterCity_Museum_1F_Text_8440D12::
	.string "A reconstruction of Kabutops’s\n"
	.string "skeleton.\p"
	.string "Its sharp limbs reveal the\n"
	.string "adaptations of an ancient predator.$"

Kanto_PewterCity_Museum_1F_Text_8461841::
	.string "POKéMON JOURNAL\p"
	.string "Special Feature: PEWTER GYM\n"
	.string "LEADER BROCK!\p"
	.string "BROCK rarely laughs, but is said to\n"
	.string "be unable to stop if he starts.$"
