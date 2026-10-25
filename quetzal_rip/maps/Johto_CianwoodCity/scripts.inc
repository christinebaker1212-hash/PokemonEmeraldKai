Johto_CianwoodCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_CianwoodCity_EventScript_83C9BDF
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_CianwoodCity_OnFrame_83C9BCD
	map_script MAP_SCRIPT_ON_LOAD, Johto_CianwoodCity_EventScript_83C9C08
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_CianwoodCity_OnFrame_83C9BCD:
	map_script_2 VAR_QZ_0x501E, 1, Johto_CianwoodCity_EventScript_83C9C28
	map_script_2 VAR_QZ_0x500C, 15, Quetzal_EventScript_83DF6B4
	.2byte 0

Johto_CianwoodCity_EventScript_83C9BDF::
	special QzJohtoTravelingEventCheck_770
	call Quetzal_EventScript_840EA62
	goto_if_set FLAG_QZ_0x1318, Johto_CianwoodCity_EventScript_83C9BFC
	goto_if_set FLAG_QZ_0x1425, Johto_CianwoodCity_EventScript_83C9BFC
	setflag FLAG_QZ_0x11C1

Johto_CianwoodCity_EventScript_83C9BFC::
	goto_if_ge VAR_QZ_0x500D, 5, Quetzal_EventScript_83E3F8B
	end

Johto_CianwoodCity_EventScript_83C9C08::
	special QzJohtoTravelingEventCheck_770
	goto_if_ge VAR_QZ_0x502D, 2, Johto_CianwoodCity_EventScript_83C9C22
	clearflag FLAG_QZ_0x10BE
	setflag FLAG_QZ_0x10BD
	setvar VAR_TEMP_0, 1
	end

Johto_CianwoodCity_EventScript_83C9C22::
	setvar VAR_TEMP_0, 1
	end

Johto_CianwoodCity_EventScript_83C9C28::
	lock
	playse SE_PIN
	applymovement 6, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 6, Johto_CianwoodCity_Movement_83C9C58
	waitmovement 0
	msgbox Johto_CianwoodCity_Text_83C9DB3, MSGBOX_DEFAULT
	msgbox Johto_CianwoodCity_Text_83C9DEE, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501E, 2
	release
	end

@ Johto_CianwoodCity object 6 (local id 6, gfx 0x18) at 18,47
Johto_CianwoodCity_EventScript_83C9C6F::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x501E, 2, Johto_CianwoodCity_EventScript_83C9CB0
	msgbox Johto_CianwoodCity_Text_83C9CBB, MSGBOX_DEFAULT
	goto_if_set FLAG_QZ_0x11B6, Johto_CianwoodCity_EventScript_83C9C98
	msgbox Johto_CianwoodCity_Text_83C9D31, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodCity_EventScript_83C9C98::
	msgbox Johto_CianwoodCity_Text_83C9DB3, MSGBOX_DEFAULT
	setvar VAR_QZ_0x501E, 2
	msgbox Johto_CianwoodCity_Text_83C9DEE, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodCity_EventScript_83C9CB0::
	msgbox Johto_CianwoodCity_Text_83C9E49, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodCity object 4 (local id 4, gfx 0x23) at 27,41
Johto_CianwoodCity_EventScript_83C9EB1::
	lock
	faceplayer
	msgbox Johto_CianwoodCity_Text_83C9EBE, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodCity object 3 (local id 3, gfx 0x11) at 20,36
Johto_CianwoodCity_EventScript_83C9EF5::
	lock
	faceplayer
	msgbox Johto_CianwoodCity_Text_83C9F02, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodCity object 5 (local id 5, gfx 0x3E85) at 20,54
Johto_CianwoodCity_EventScript_83C9F7F::
	lock
	faceplayer
	msgbox Johto_CianwoodCity_Text_83C9F8C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodCity trigger at 18,22 (VAR_QZ_0x502D == 1)
Johto_CianwoodCity_EventScript_83C9FC4::
	lock
	playmoncry SPECIES_SUICUNE, 0
	applymovement 1, Johto_CianwoodCity_Movement_83CA0AD
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	waitmovement 0
	waitmoncry
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 1, Johto_CianwoodCity_Movement_83CA0B1
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	playmoncry SPECIES_SUICUNE, 0
	waitmoncry
	applymovement 1, Johto_CianwoodCity_Movement_83CA0B6
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	waitmovement 0
	delay 0x1E
	playbgm MUS_QZ_651, 1
	clearflag FLAG_QZ_0x10BD
	addobject 2
	applymovement 2, Johto_CianwoodCity_Movement_83CA0BD
	waitmovement 0
	delay 0x37
	applymovement 2, Johto_CianwoodCity_Movement_83CA0C3
	waitmovement 0
	msgbox Johto_CianwoodCity_Text_83CA0D0, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4656, 0, Johto_CianwoodCity_Text_83CA243
	playbgm MUS_QZ_651, 1
	msgbox Johto_CianwoodCity_Text_83CA264, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Johto_CianwoodCity_Movement_83CA0C7
	waitmovement 0
	setflag FLAG_QZ_0x10BD
	setflag FLAG_QZ_0x10BE
	removeobject 1
	removeobject 2
	setvar VAR_QZ_0x502D, 2
	setvar VAR_QZ_0x500E, 2
	fadedefaultbgm
	call_if_set FLAG_QZ_0x10B9, Johto_CianwoodCity_EventScript_83CA0A7
	release
	end

Johto_CianwoodCity_EventScript_83CA0A7::
	setvar VAR_QZ_0x502D, 3
	return

@ Johto_CianwoodCity sign / bg event at 33,48
Johto_CianwoodCity_EventScript_83CA308::
	lock
	msgbox Johto_CianwoodCity_Text_83CA314, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_CianwoodCity sign / bg event at 17,44
Johto_CianwoodCity_EventScript_83CA342::
	lock
	msgbox Johto_CianwoodCity_Text_83CA34E, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_CianwoodCity sign / bg event at 21,51
Johto_CianwoodCity_EventScript_83CA397::
	lock
	msgbox Johto_CianwoodCity_Text_83CA3A3, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_CianwoodCity sign / bg event at 14,33
Johto_CianwoodCity_EventScript_83CA3F1::
	lock
	msgbox Johto_CianwoodCity_Text_83CA40A, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_CianwoodCity object 13 (local id 13, gfx 0x3E91) at 0,34
Johto_CianwoodCity_EventScript_83CA3FD::
	lock
	faceplayer
	msgbox Johto_CianwoodCity_Text_83CA433, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodCity_Movement_83C9C58:
	walk_left
	walk_left
	walk_up
	walk_left
	face_up
	step_end

Johto_CianwoodCity_Movement_83CA0AD:
	jump_in_place_right_left
	jump_in_place_right_left
	face_down
	step_end

Johto_CianwoodCity_Movement_83CA0B1:
	jump_down
	jump_2_down
	jump_right
	face_up
	step_end

Johto_CianwoodCity_Movement_83CA0B6:
	jump_2_down
	jump_2_right
	jump_2_right
	jump_2_right
	jump_2_right
	jump_2_right
	step_end

Johto_CianwoodCity_Movement_83CA0BD:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	face_right
	step_end

Johto_CianwoodCity_Movement_83CA0C3:
	face_up
	walk_up
	walk_up
	step_end

Johto_CianwoodCity_Movement_83CA0C7:
	face_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_CianwoodCity_Text_83C9CBB::
	.string "You crossed the sea to get here?\p"
	.string "That must have been hard.\p"
	.string "It would be much easier if your\n"
	.string "POKéMON knew how to FLY...$"

Johto_CianwoodCity_Text_83C9D31::
	.string "But you can’t use FLY without this\n"
	.string "city’s GYM BADGE.\p"
	.string "If you beat the GYM LEADER here,\n"
	.string "come see me.\p"
	.string "I’ll have a nice gift for you.$"

Johto_CianwoodCity_Text_83C9DB3::
	.string "That’s CIANWOOD’s GYM BADGE!\p"
	.string "Then you should take this HM.$"

Johto_CianwoodCity_Text_83C9DEE::
	.string "Teach FLY to your POKéMON.\p"
	.string "You will be able to FLY instantly\n"
	.string "to anywhere you have visited.$"

Johto_CianwoodCity_Text_83C9E49::
	.string "My husband lost to you, so he needs\n"
	.string "to train harder.\p"
	.string "That’s good, since he was getting\n"
	.string "a little chubby.$"

Johto_CianwoodCity_Text_83C9EBE::
	.string "If you use FLY, you can get back\n"
	.string "to OLIVINE instantly.$"

Johto_CianwoodCity_Text_83C9F02::
	.string "Boulders to the north of town can\n"
	.string "be crushed.\p"
	.string "They may be hiding something.\p"
	.string "Your POKéMON could use ROCK SMASH\n"
	.string "to break them.$"

Johto_CianwoodCity_Text_83C9F8C::
	.string "CHUCK, the GYM LEADER, spars with\n"
	.string "his fighting POKéMON.$"

Johto_CianwoodCity_Text_83CA0D0::
	.string "EUSINE: Yo, {PLAYER}.\p"
	.string "Wasn’t that SUICUNE just now?\p"
	.string "I only caught a quick glimpse, but\n"
	.string "I thought I saw SUICUNE running on\l"
	.string "the waves.\p"
	.string "SUICUNE is beautiful and grand.\p"
	.string "And it races through towns and\n"
	.string "roads at simply awesome speeds.\p"
	.string "It’s wonderful…\p"
	.string "I want to see SUICUNE up close…\p"
	.string "I’ve decided.\p"
	.string "I’ll battle you as a trainer to\n"
	.string "earn SUICUNE’s respect!\p"
	.string "Come on, {PLAYER}. Let’s battle now!$"

Johto_CianwoodCity_Text_83CA243::
	.string "I hate to admit it, but you win.$"

Johto_CianwoodCity_Text_83CA264::
	.string "You’re amazing, {PLAYER}!\p"
	.string "No wonder POKéMON gravitate to you.\p"
	.string "I get it now.\p"
	.string "I’m going to keep searching for\n"
	.string "SUICUNE.\p"
	.string "I’m sure we’ll see each other again.\p"
	.string "See you around!$"

Johto_CianwoodCity_Text_83CA314::
	.string "CIANWOOD CITY\p"
	.string "A Port Surrounded by Rough Seas$"

Johto_CianwoodCity_Text_83CA34E::
	.string "CIANWOOD CITY POKéMON GYM\p"
	.string "LEADER: CHUCK\p"
	.string "His Roaring Fists Do the Talking$"

Johto_CianwoodCity_Text_83CA3A3::
	.string "500 Years of Tradition\p"
	.string "CIANWOOD CITY PHARMACY\p"
	.string "We Await Your Medicinal Queries$"

Johto_CianwoodCity_Text_83CA40A::
	.string "CLIFF EDGE CAVE\n"
	.string "Road to the SAFARI ZONE.$"

Johto_CianwoodCity_Text_83CA433::
	.string "This path is under construction.\n"
	.string "We’re getting the new SAFARI ZONE\l"
	.string "up and running!$"
