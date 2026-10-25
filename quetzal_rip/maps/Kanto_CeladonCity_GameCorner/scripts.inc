Kanto_CeladonCity_GameCorner_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_CeladonCity_GameCorner_EventScript_841C91F
	.byte 0

Kanto_CeladonCity_GameCorner_EventScript_841C91F::
	call_if_unset FLAG_QZ_0x21E6, Kanto_CeladonCity_GameCorner_EventScript_841C929
	end

Kanto_CeladonCity_GameCorner_EventScript_841C929::
	setmetatile 0xF, 2, 0x292, 0
	setmetatile 0x10, 2, 0x292, 0
	setmetatile 0x11, 2, 0x292, 0
	setmetatile 0x10, 3, 0x2CF, 1
	setmetatile 0x11, 3, 0x2CF, 1
	return

@ Kanto_CeladonCity_GameCorner object 1 (local id 1, gfx 0x3F5A) at 4,2
Kanto_CeladonCity_GameCorner_EventScript_841C957::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446ADE, MSGBOX_NPC
	end

@ Kanto_CeladonCity_GameCorner object 2 (local id 2, gfx 0x3F5B) at 6,2
Kanto_CeladonCity_GameCorner_EventScript_841C960::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	msgbox Kanto_CeladonCity_GameCorner_Text_8446B22, MSGBOX_DEFAULT
	goto Quetzal_EventScript_841C97A  @ falls through in the ROM

@ Kanto_CeladonCity_GameCorner object 3 (local id 3, gfx 0x3EE6) at 1,6
Kanto_CeladonCity_GameCorner_EventScript_841CAA2::
	lock
	faceplayer
	msgbox Kanto_CeladonCity_GameCorner_Text_8446C22, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CAB2::
	closemessage
	applymovement VAR_LAST_TALKED, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_CeladonCity_GameCorner object 4 (local id 4, gfx 0x3F58) at 1,8
Kanto_CeladonCity_GameCorner_EventScript_841CABF::
	lock
	faceplayer
	msgbox Kanto_CeladonCity_GameCorner_Text_8446C67, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

@ Kanto_CeladonCity_GameCorner object 5 (local id 5, gfx 0x3F06) at 4,7
Kanto_CeladonCity_GameCorner_EventScript_841CACF::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x213D, Kanto_CeladonCity_GameCorner_EventScript_841CB38
	msgbox Kanto_CeladonCity_GameCorner_Text_8446C93, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_EventScript_841CB28
	checkcoins VAR_TEMP_1
	goto_if_ge VAR_TEMP_1, 9990, Kanto_CeladonCity_GameCorner_EventScript_841CB1A
	addcoins 0xA
	textcolor 3
	msgbox Kanto_CeladonCity_GameCorner_Text_8446CAD, MSGBOX_DEFAULT
	playse SE_SHOP
	waitse
	setflag FLAG_QZ_0x213D
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CB1A::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446CD0, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CB28::
	textcolor 3
	msgbox Kanto_CeladonCity_GameCorner_Text_84470C6, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CB38::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446CE9, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

@ Kanto_CeladonCity_GameCorner object 6 (local id 6, gfx 0x3F0B) at 7,10
Kanto_CeladonCity_GameCorner_EventScript_841CB46::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x211D, Kanto_CeladonCity_GameCorner_EventScript_841CB5F
	msgbox Kanto_CeladonCity_GameCorner_Text_8446D4D, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CB5F::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446E41, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

@ Kanto_CeladonCity_GameCorner object 7 (local id 7, gfx 0x3F57) at 7,8
Kanto_CeladonCity_GameCorner_EventScript_841CB6D::
	lock
	faceplayer
	msgbox Kanto_CeladonCity_GameCorner_Text_8446D1B, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

@ Kanto_CeladonCity_GameCorner object 8 (local id 8, gfx 0x3F2D) at 10,9
Kanto_CeladonCity_GameCorner_EventScript_841CB7D::
	lock
	faceplayer
	msgbox Kanto_CeladonCity_GameCorner_Text_8446E9E, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

@ Kanto_CeladonCity_GameCorner object 9 (local id 9, gfx 0x3F43) at 13,7
Kanto_CeladonCity_GameCorner_EventScript_841CB8D::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x213E, Kanto_CeladonCity_GameCorner_EventScript_841CBE6
	msgbox Kanto_CeladonCity_GameCorner_Text_8446ECB, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_EventScript_841CB28
	checkcoins VAR_TEMP_1
	goto_if_ge VAR_TEMP_1, 9980, Kanto_CeladonCity_GameCorner_EventScript_841CBD8
	addcoins 0x14
	textcolor 3
	msgbox Kanto_CeladonCity_GameCorner_Text_8446EE7, MSGBOX_DEFAULT
	playse SE_SHOP
	waitse
	setflag FLAG_QZ_0x213E
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CBD8::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446F0F, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CBE6::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446F27, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

@ Kanto_CeladonCity_GameCorner object 10 (local id 10, gfx 0x3F09) at 16,10
Kanto_CeladonCity_GameCorner_EventScript_841CBF4::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x213F, Kanto_CeladonCity_GameCorner_EventScript_841CC4D
	msgbox Kanto_CeladonCity_GameCorner_Text_8446F57, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_EventScript_841CB28
	checkcoins VAR_TEMP_1
	goto_if_ge VAR_TEMP_1, 9980, Kanto_CeladonCity_GameCorner_EventScript_841CC3F
	addcoins 0x14
	textcolor 3
	msgbox Kanto_CeladonCity_GameCorner_Text_8446F98, MSGBOX_DEFAULT
	playse SE_SHOP
	waitse
	setflag FLAG_QZ_0x213F
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CC3F::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446FBB, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

Kanto_CeladonCity_GameCorner_EventScript_841CC4D::
	msgbox Kanto_CeladonCity_GameCorner_Text_8446FE0, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_EventScript_841CAB2

@ Kanto_CeladonCity_GameCorner sign / bg event at 0,7
Kanto_CeladonCity_GameCorner_EventScript_841CC5B::
	lockall
	setvar VAR_0x8004, 0
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

Kanto_CeladonCity_GameCorner_EventScript_841CC67::
	releaseall
	end

Kanto_CeladonCity_GameCorner_EventScript_841CC69::
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_EventScript_841CDA4
	msgbox Kanto_CeladonCity_GameCorner_Text_84470E6, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_EventScript_841CC67
	setfieldeffectargument 0, 0xFF
	setfieldeffectargument 1, 0xA
	setfieldeffectargument 2, 0xE
	dofieldeffect 0x2E
	waitfieldeffect 0x2E
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

@ Kanto_CeladonCity_GameCorner sign / bg event at 0,9
Kanto_CeladonCity_GameCorner_EventScript_841CCA8::
	lockall
	setvar VAR_0x8004, 1
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 0,10
Kanto_CeladonCity_GameCorner_EventScript_841CCB4::
	lockall
	setvar VAR_0x8004, 2
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 5,6
Kanto_CeladonCity_GameCorner_EventScript_841CCC0::
	lockall
	setvar VAR_0x8004, 3
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 5,9
Kanto_CeladonCity_GameCorner_EventScript_841CCD8::
	lockall
	setvar VAR_0x8004, 5
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 5,10
Kanto_CeladonCity_GameCorner_EventScript_841CCE4::
	lockall
	setvar VAR_0x8004, 6
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 6,6
Kanto_CeladonCity_GameCorner_EventScript_841CCF0::
	lockall
	setvar VAR_0x8004, 7
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 6,7
Kanto_CeladonCity_GameCorner_EventScript_841CCFC::
	lockall
	setvar VAR_0x8004, 8
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 6,9
Kanto_CeladonCity_GameCorner_EventScript_841CD08::
	lockall
	setvar VAR_0x8004, 9
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 11,6
Kanto_CeladonCity_GameCorner_EventScript_841CD14::
	lockall
	setvar VAR_0x8004, 0xA
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 11,7
Kanto_CeladonCity_GameCorner_EventScript_841CD20::
	lockall
	setvar VAR_0x8004, 0xB
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 11,8
Kanto_CeladonCity_GameCorner_EventScript_841CD2C::
	lockall
	setvar VAR_0x8004, 0xC
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 11,10
Kanto_CeladonCity_GameCorner_EventScript_841CD38::
	lockall
	setvar VAR_0x8004, 0xD
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 12,6
Kanto_CeladonCity_GameCorner_EventScript_841CD44::
	lockall
	setvar VAR_0x8004, 0xE
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 12,9
Kanto_CeladonCity_GameCorner_EventScript_841CD5C::
	lockall
	setvar VAR_0x8004, 0x10
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 12,10
Kanto_CeladonCity_GameCorner_EventScript_841CD68::
	lockall
	setvar VAR_0x8004, 0x11
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 17,7
Kanto_CeladonCity_GameCorner_EventScript_841CD80::
	lockall
	setvar VAR_0x8004, 0x13
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 17,8
Kanto_CeladonCity_GameCorner_EventScript_841CD8C::
	lockall
	setvar VAR_0x8004, 0x14
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

@ Kanto_CeladonCity_GameCorner sign / bg event at 17,9
Kanto_CeladonCity_GameCorner_EventScript_841CD98::
	lockall
	setvar VAR_0x8004, 0x15
	goto Kanto_CeladonCity_GameCorner_EventScript_841CC69

Kanto_CeladonCity_GameCorner_EventScript_841CDA4::
	msgbox Kanto_CeladonCity_GameCorner_Text_84470AD, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_CeladonCity_GameCorner sign / bg event at 11,1
Kanto_CeladonCity_GameCorner_EventScript_841CDAE::
	lockall
	msgbox Kanto_CeladonCity_GameCorner_Text_844707C, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x21E6, Kanto_CeladonCity_GameCorner_EventScript_841CDC2
	releaseall
	end

Kanto_CeladonCity_GameCorner_EventScript_841CDC2::
	playse SE_UNLOCK
	setmetatile 0xF, 2, 0x29D, 0
	setmetatile 0x10, 2, 0x29E, 1
	setmetatile 0x11, 2, 0x29F, 1
	setmetatile 0x10, 3, 0x2A6, 1
	setmetatile 0x11, 3, 0x2A7, 1
	special DrawWholeMapView
	setflag FLAG_QZ_0x21E6
	return

@ Kanto_CeladonCity_GameCorner object 11 (local id 11, gfx 0x3F3D) at 11,2
Kanto_CeladonCity_GameCorner_EventScript_841CDF9::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8549, 0, Kanto_CeladonCity_GameCorner_Text_8447009, Kanto_CeladonCity_GameCorner_Text_8447035, Kanto_CeladonCity_GameCorner_EventScript_841CE14
	msgbox Kanto_CeladonCity_GameCorner_Text_844703B, MSGBOX_AUTOCLOSE
	end

Kanto_CeladonCity_GameCorner_EventScript_841CE14::
	msgbox Kanto_CeladonCity_GameCorner_Text_844703B, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 3, Kanto_CeladonCity_GameCorner_EventScript_841CE38
	call_if_ne VAR_FACING, 3, Kanto_CeladonCity_GameCorner_EventScript_841CE43
	removeobject 11
	release
	end

Kanto_CeladonCity_GameCorner_EventScript_841CE38::
	applymovement 11, Kanto_CeladonCity_GameCorner_Movement_841CE4E
	waitmovement 0
	return

Kanto_CeladonCity_GameCorner_EventScript_841CE43::
	applymovement 11, Kanto_CeladonCity_GameCorner_Movement_841CE57
	waitmovement 0
	return

@ Kanto_CeladonCity_GameCorner sign / bg event at 5,8
Kanto_CeladonCity_GameCorner_EventScript_841CE5E::
	msgbox Kanto_CeladonCity_GameCorner_Text_8447104, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_GameCorner sign / bg event at 12,8
Kanto_CeladonCity_GameCorner_EventScript_841CE67::
	msgbox Kanto_CeladonCity_GameCorner_Text_8447121, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_GameCorner sign / bg event at 17,6
Kanto_CeladonCity_GameCorner_EventScript_841CE70::
	msgbox Kanto_CeladonCity_GameCorner_Text_8447140, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_GameCorner sign / bg event at 17,13
Kanto_CeladonCity_GameCorner_EventScript_8462B2D::
	msgbox Kanto_CeladonCity_GameCorner_Text_8462156, MSGBOX_SIGN
	end

Kanto_CeladonCity_GameCorner_Movement_841CE4E:
	walk_down
	walk_right
	walk_right
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_CeladonCity_GameCorner_Movement_841CE57:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_CeladonCity_GameCorner_Text_8446ADE::
	.string "Welcome!\p"
	.string "You can exchange your COINS for\n"
	.string "fabulous prizes next door.$"

Kanto_CeladonCity_GameCorner_Text_8446B22::
	.string "Welcome to ROCKET GAME CORNER!\p"
	.string "Do you need some game COINS?\n"
	.string "Would you like to buy some?$"

Kanto_CeladonCity_GameCorner_Text_8446C22::
	.string "Keep this quiet.\p"
	.string "It’s rumored that this place is run\n"
	.string "by TEAM ROCKET.$"

Kanto_CeladonCity_GameCorner_Text_8446C67::
	.string "I think these machines have\n"
	.string "different odds.$"

Kanto_CeladonCity_GameCorner_Text_8446C93::
	.string "Kid, do you want to play?$"

Kanto_CeladonCity_GameCorner_Text_8446CAD::
	.string "{PLAYER} received 10 COINS\n"
	.string "from the man.$"

Kanto_CeladonCity_GameCorner_Text_8446CD0::
	.string "You don’t need my COINS!$"

Kanto_CeladonCity_GameCorner_Text_8446CE9::
	.string "Wins seem to come and go.\n"
	.string "Nothing’s a sure thing.$"

Kanto_CeladonCity_GameCorner_Text_8446D1B::
	.string "These slot games…\n"
	.string "Win or lose, it’s only by luck.$"

Kanto_CeladonCity_GameCorner_Text_8446D4D::
	.string "Hey!\p"
	.string "You have better things to do,\n"
	.string "champ in the making!\p"
	.string "CELADON GYM’s LEADER is ERIKA.\p"
	.string "She is a user of GRASS-type\n"
	.string "POKéMON, and at one with nature.\p"
	.string "She might appear docile because of\n"
	.string "her flower arranging…\p"
	.string "But she’s not one to be taken\n"
	.string "lightly!$"

Kanto_CeladonCity_GameCorner_Text_8446E41::
	.string "They offer rare POKéMON that can\n"
	.string "be exchanged for your COINS.\p"
	.string "But, I just can’t seem to win!$"

Kanto_CeladonCity_GameCorner_Text_8446E9E::
	.string "Games are scary!\n"
	.string "It’s so easy to get hooked!$"

Kanto_CeladonCity_GameCorner_Text_8446ECB::
	.string "What’s up?\n"
	.string "Want some COINS?$"

Kanto_CeladonCity_GameCorner_Text_8446EE7::
	.string "{PLAYER} received 20 COINS\n"
	.string "from the nice guy.$"

Kanto_CeladonCity_GameCorner_Text_8446F0F::
	.string "You have lots of COINS!$"

Kanto_CeladonCity_GameCorner_Text_8446F27::
	.string "Darn! I need more COINS for the\n"
	.string "POKéMON I want!$"

Kanto_CeladonCity_GameCorner_Text_8446F57::
	.string "Hey, what? You’re throwing me off!\n"
	.string "Here are some COINS, so shoo!$"

Kanto_CeladonCity_GameCorner_Text_8446F98::
	.string "{PLAYER} received 20 COINS\n"
	.string "from the man.$"

Kanto_CeladonCity_GameCorner_Text_8446FBB::
	.string "You’ve got plenty of your own\n"
	.string "COINS!$"

Kanto_CeladonCity_GameCorner_Text_8446FE0::
	.string "The trick is to watch the reels\n"
	.string "closely.$"

Kanto_CeladonCity_GameCorner_Text_8447009::
	.string "I’m guarding this poster!\n"
	.string "Go away, or else!$"

Kanto_CeladonCity_GameCorner_Text_8447035::
	.string "Dang!$"

Kanto_CeladonCity_GameCorner_Text_844703B::
	.string "The TEAM ROCKET HIDEOUT might\n"
	.string "be discovered!\p"
	.string "I better tell BOSS!$"

Kanto_CeladonCity_GameCorner_Text_844707C::
	.string "Hey!\p"
	.string "A switch behind the poster!?\n"
	.string "Let’s push it!$"

Kanto_CeladonCity_GameCorner_Text_84470AD::
	.string "A COIN CASE is required…$"

Kanto_CeladonCity_GameCorner_Text_84470C6::
	.string "Oops!\n"
	.string "Don’t have the COIN CASE!$"

Kanto_CeladonCity_GameCorner_Text_84470E6::
	.string "A slot machine!\n"
	.string "Want to play?$"

Kanto_CeladonCity_GameCorner_Text_8447104::
	.string "OUT OF ORDER\n"
	.string "This is broken.$"

Kanto_CeladonCity_GameCorner_Text_8447121::
	.string "OUT TO LUNCH\n"
	.string "This is reserved.$"

Kanto_CeladonCity_GameCorner_Text_8447140::
	.string "Someone’s keys!\n"
	.string "They’ll be back.$"

Kanto_CeladonCity_GameCorner_Text_8462156::
	.string "The photo booth is\n"
	.string "out of service.$"
