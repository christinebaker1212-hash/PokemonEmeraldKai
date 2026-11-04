Kanto_PewterCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PewterCity_EventScript_841599D
	.byte 0

Kanto_PewterCity_EventScript_841599D::
	call Kanto_PewterCity_EventScript_AideShow   @ Moemon Kai: FireRed's RUNNING SHOES aide
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x2246
	setvar VAR_QZ_0x511E, 0
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_PewterCity object 5 (local id 5, gfx 0x3EEE) at 42,20
Kanto_PewterCity_EventScript_84159AE::
	lock
	faceplayer
	msgbox Kanto_PewterCity_Text_842E996, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_RG_FOLLOW_ME, 0
	call_if_eq VAR_FACING, 4, Kanto_PewterCity_EventScript_84159E4
	msgbox Kanto_PewterCity_Text_842E9DF, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Kanto_PewterCity_Movement_8415B94
	waitmovement 0
	fadedefaultbgm
	removeobject 5
	clearflag FLAG_QZ_0x21A5
	release
	end

Kanto_PewterCity_EventScript_84159E4::
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415A4D
	applymovement 5, Kanto_PewterCity_Movement_8415AEF
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415A52
	applymovement 5, Kanto_PewterCity_Movement_8415AF4
	waitmovement 0
	return

@ Kanto_PewterCity trigger at 42,21 (VAR_QZ_0x511D == 0)
Kanto_PewterCity_EventScript_8415B9F::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_PewterCity_EventScript_8415BCF

@ Kanto_PewterCity trigger at 42,22 (VAR_QZ_0x511D == 0)
Kanto_PewterCity_EventScript_8415BAB::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_PewterCity_EventScript_8415BCF

@ Kanto_PewterCity trigger at 42,23 (VAR_QZ_0x511D == 0)
Kanto_PewterCity_EventScript_8415BB7::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_PewterCity_EventScript_8415BCF

@ Kanto_PewterCity trigger at 43,23 (VAR_QZ_0x511D == 0)
Kanto_PewterCity_EventScript_8415BC3::
	lockall
	setvar VAR_TEMP_1, 3
	goto Kanto_PewterCity_EventScript_8415BCF

Kanto_PewterCity_EventScript_8415BCF::
	textcolor 0
	msgbox Kanto_PewterCity_Text_842E996, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_RG_FOLLOW_ME, 0
	call_if_eq VAR_TEMP_1, 0, Kanto_PewterCity_EventScript_8415C1B
	call_if_eq VAR_TEMP_1, 1, Kanto_PewterCity_EventScript_8415C37
	call_if_eq VAR_TEMP_1, 2, Kanto_PewterCity_EventScript_8415C53
	msgbox Kanto_PewterCity_Text_842E9DF, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Kanto_PewterCity_Movement_8415B94
	waitmovement 0
	fadedefaultbgm
	removeobject 5
	clearflag FLAG_QZ_0x21A5
	releaseall
	end

Kanto_PewterCity_EventScript_8415C1B::
	applymovement 5, Kanto_PewterCity_Movement_8415D7D
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415C8B
	applymovement 5, Kanto_PewterCity_Movement_8415D90
	waitmovement 0
	return

Kanto_PewterCity_EventScript_8415C37::
	applymovement 5, Kanto_PewterCity_Movement_8415D82
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415CCA
	applymovement 5, Kanto_PewterCity_Movement_8415DD0
	waitmovement 0
	return

Kanto_PewterCity_EventScript_8415C53::
	applymovement 5, Kanto_PewterCity_Movement_8415D85
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415D0A
	applymovement 5, Kanto_PewterCity_Movement_8415E11
	waitmovement 0
	return

@ Kanto_PewterCity object 1 (local id 1, gfx 0x3F16) at 6,15
Kanto_PewterCity_EventScript_8415E86::
	msgbox Kanto_PewterCity_Text_842E74E, MSGBOX_NPC
	end

@ Kanto_PewterCity object 2 (local id 2, gfx 0x3F1E) at 33,17
Kanto_PewterCity_EventScript_8415E8F::
	lock
	faceplayer
	msgbox Kanto_PewterCity_Text_842E856, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_PewterCity_EventScript_8415F47
	msgbox Kanto_PewterCity_Text_842E8A1, MSGBOX_DEFAULT
	closemessage
	delay 0xA
	playbgm MUS_RG_FOLLOW_ME, 0
	call_if_eq VAR_FACING, 2, Kanto_PewterCity_EventScript_8415EFF
	call_if_eq VAR_FACING, 1, Kanto_PewterCity_EventScript_8415F11
	call_if_eq VAR_FACING, 3, Kanto_PewterCity_EventScript_8415F23
	call_if_eq VAR_FACING, 4, Kanto_PewterCity_EventScript_8415F35
	msgbox Kanto_PewterCity_Text_842E8C4, MSGBOX_DEFAULT
	closemessage
	delay 0xA
	applymovement 2, Kanto_PewterCity_Movement_841602D
	waitmovement 0
	fadedefaultbgm
	removeobject 2
	clearflag FLAG_QZ_0x21A7
	release
	end

Kanto_PewterCity_EventScript_8415EFF::
	applymovement 2, Kanto_PewterCity_Movement_8415F6D
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415F51
	waitmovement 0
	return

Kanto_PewterCity_EventScript_8415F11::
	applymovement 2, Kanto_PewterCity_Movement_8415FA4
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415F88
	waitmovement 0
	return

Kanto_PewterCity_EventScript_8415F23::
	applymovement 2, Kanto_PewterCity_Movement_8415FDB
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415FBF
	waitmovement 0
	return

Kanto_PewterCity_EventScript_8415F35::
	applymovement 2, Kanto_PewterCity_Movement_8416012
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_8415FF6
	waitmovement 0
	return

Kanto_PewterCity_EventScript_8415F47::
	msgbox Kanto_PewterCity_Text_842E874, MSGBOX_DEFAULT
	release
	end

@ Kanto_PewterCity object 3 (local id 3, gfx 0x3F04) at 21,28
Kanto_PewterCity_EventScript_841603C::
	lock
	faceplayer
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_PewterCity_Text_842E7B7, MSGBOX_DEFAULT
	release
	end

@ Kanto_PewterCity object 4 (local id 4, gfx 0x3EF1) at 33,27
Kanto_PewterCity_EventScript_8416055::
	lock
	faceplayer
	msgbox Kanto_PewterCity_Text_842E91A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_PewterCity_EventScript_8416074
	msgbox Kanto_PewterCity_Text_842E95C, MSGBOX_DEFAULT
	release
	end

Kanto_PewterCity_EventScript_8416074::
	msgbox Kanto_PewterCity_Text_842E93E, MSGBOX_DEFAULT
	release
	end

@ Kanto_PewterCity sign / bg event at 20,30
Kanto_PewterCity_EventScript_841607E::
	msgbox Kanto_PewterCity_Text_842EA0E, MSGBOX_SIGN
	end

@ Kanto_PewterCity sign / bg event at 39,19
Kanto_PewterCity_EventScript_8416087::
	msgbox Kanto_PewterCity_Text_842EA60, MSGBOX_SIGN
	end

@ Kanto_PewterCity sign / bg event at 19,7
Kanto_PewterCity_EventScript_8416090::
	msgbox Kanto_PewterCity_Text_842EADD, MSGBOX_SIGN
	end

@ Kanto_PewterCity sign / bg event at 11,16
Kanto_PewterCity_EventScript_8416099::
	lockall
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	msgbox Kanto_PewterCity_Text_842EAF6, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_PewterCity sign / bg event at 31,25
Kanto_PewterCity_EventScript_84160B1::
	msgbox Kanto_PewterCity_Text_842EB3C, MSGBOX_SIGN
	end

Kanto_PewterCity_Movement_8415A4D:
	delay_16
	delay_16
	delay_4
	walk_in_place_faster_down
	step_end

Kanto_PewterCity_Movement_8415A52:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PewterCity_Movement_8415AEF:
	walk_down
	walk_left
	walk_in_place_faster_up
	walk_in_place_up
	step_end

Kanto_PewterCity_Movement_8415AF4:
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_left
	step_end

Kanto_PewterCity_Movement_8415B94:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Kanto_PewterCity_Movement_8415C8B:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PewterCity_Movement_8415CCA:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PewterCity_Movement_8415D0A:
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PewterCity_Movement_8415D7D:
	walk_left
	walk_down
	walk_in_place_faster_right
	walk_in_place_right
	step_end

Kanto_PewterCity_Movement_8415D82:
	walk_down
	walk_in_place_down
	step_end

Kanto_PewterCity_Movement_8415D85:
	walk_down
	walk_down
	walk_in_place_down
	step_end

Kanto_PewterCity_Movement_8415D90:
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_left
	step_end

Kanto_PewterCity_Movement_8415DD0:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_left
	step_end

Kanto_PewterCity_Movement_8415E11:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_left
	step_end

Kanto_PewterCity_Movement_8415F51:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	delay_8
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_8415F6D:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_8415F88:
	delay_16
	delay_16
	walk_left
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	delay_8
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_8415FA4:
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_8415FBF:
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	delay_8
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_8415FDB:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_8415FF6:
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	delay_8
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_8416012:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Movement_841602D:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	step_end

Kanto_PewterCity_Text_842E74E::
	.string "CLEFAIRY came from the moon.\n"
	.string "That’s what the rumor is.\p"
	.string "They appeared after MOON STONES\n"
	.string "fell on MT. MOON.$"

Kanto_PewterCity_Text_842E7B7::
	.string "There aren’t many serious MOéMON\n"
	.string "TRAINERS here.\p"
	.string "They’re all like BUG CATCHERS,\n"
	.string "you know, just hobbyists.\p"
	.string "But PEWTER GYM’s BROCK isn’t like\n"
	.string "that, not one bit.$"

Kanto_PewterCity_Text_842E856::
	.string "Did you check out the MUSEUM?$"

Kanto_PewterCity_Text_842E874::
	.string "Weren’t those fossils from MT. MOON\n"
	.string "amazing?$"

Kanto_PewterCity_Text_842E8A1::
	.string "Really?\n"
	.string "You absolutely have to go!$"

Kanto_PewterCity_Text_842E8C4::
	.string "This is it, the MUSEUM.\p"
	.string "You have to pay to get in, but it’s\n"
	.string "worth it. See you around!$"

Kanto_PewterCity_Text_842E91A::
	.string "Psssst!\n"
	.string "Do you know what I’m doing?$"

Kanto_PewterCity_Text_842E93E::
	.string "That’s right!\n"
	.string "It’s hard work!$"

Kanto_PewterCity_Text_842E95C::
	.string "I’m spraying REPEL to keep wild\n"
	.string "MOéMON out of my garden!$"

Kanto_PewterCity_Text_842E996::
	.string "You’re a TRAINER, right?\p"
	.string "BROCK’s looking for new\n"
	.string "challengers. Follow me!$"

Kanto_PewterCity_Text_842E9DF::
	.string "If you have the right stuff,\n"
	.string "go take on BROCK!$"

Kanto_PewterCity_Text_842EA0E::
	.string "TRAINER TIPS\p"
	.string "All MOéMON that appear in battle,\n"
	.string "however briefly, earn EXP Points.$"

Kanto_PewterCity_Text_842EA60::
	.string "NOTICE!\p"
	.string "Thieves have been stealing MOéMON\n"
	.string "fossils from MT. MOON.\p"
	.string "Please call the PEWTER POLICE if\n"
	.string "you have any information.$"

Kanto_PewterCity_Text_842EADD::
	.string "PEWTER MUSEUM OF SCIENCE$"

Kanto_PewterCity_Text_842EAF6::
	.string "PEWTER CITY MOéMON GYM\n"
	.string "LEADER: BROCK\l"
	.string "The Rock-Solid MOéMON TRAINER!$"

Kanto_PewterCity_Text_842EB3C::
	.string "PEWTER CITY\n"
	.string "A Stone Gray City$"

@ Moemon Kai: FireRed's RUNNING SHOES aide (pokefirered data/maps/PewterCity), removed by Quetzal
Kanto_PewterCity_EventScript_RunningShoesAide::
	lock
	faceplayer
	setvar VAR_TEMP_1, 0
	call Kanto_PewterCity_EventScript_AideGiveRunningShoes
	release
	end

Kanto_PewterCity_EventScript_RunningShoesAideTriggerTop::
	lockall
	setvar VAR_TEMP_1, 1
	call Kanto_PewterCity_EventScript_AideGiveRunningShoes
	releaseall
	end

Kanto_PewterCity_EventScript_RunningShoesAideTriggerMid::
	lockall
	setvar VAR_TEMP_1, 2
	call Kanto_PewterCity_EventScript_AideGiveRunningShoes
	releaseall
	end

Kanto_PewterCity_EventScript_RunningShoesAideTriggerBottom::
	lockall
	setvar VAR_TEMP_1, 3
	call Kanto_PewterCity_EventScript_AideGiveRunningShoes
	releaseall
	end

Kanto_PewterCity_EventScript_AideGiveRunningShoes::
	textcolor 0
	call_if_eq VAR_TEMP_1, 1, Kanto_PewterCity_EventScript_AideNoticePlayer
	call_if_eq VAR_TEMP_1, 2, Kanto_PewterCity_EventScript_AideNoticePlayer
	call_if_eq VAR_TEMP_1, 3, Kanto_PewterCity_EventScript_AideNoticePlayer
	waitse
	playse SE_PIN
	applymovement 12, Kanto_PewterCity_Movement_AideExclamation
	waitmovement 0
	applymovement 12, Kanto_PewterCity_Movement_AideDelay48
	waitmovement 0
	msgbox Kanto_PewterCity_Text_OhPlayer, MSGBOX_DEFAULT
	call_if_eq VAR_TEMP_1, 0, Kanto_PewterCity_EventScript_AideApproachPlayer0
	call_if_eq VAR_TEMP_1, 1, Kanto_PewterCity_EventScript_AideApproachPlayer1
	call_if_eq VAR_TEMP_1, 2, Kanto_PewterCity_EventScript_AideApproachPlayer2
	call_if_eq VAR_TEMP_1, 3, Kanto_PewterCity_EventScript_AideApproachPlayer3
	msgbox Kanto_PewterCity_Text_AskedToDeliverThis, MSGBOX_DEFAULT
	goto_if_set FLAG_SYS_B_DASH, Kanto_PewterCity_EventScript_AideHadShoes
	textcolor 3
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Kanto_PewterCity_Text_ReceivedRunningShoesFromAide
	waitmessage
	waitfanfare
	msgbox Kanto_PewterCity_Text_SwitchedShoesWithRunningShoes, MSGBOX_DEFAULT
	msgbox Kanto_PewterCity_Text_ExplainRunningShoes, MSGBOX_DEFAULT
	call Quetzal_EventScript_8456AB7
	msgbox Kanto_PewterCity_Text_MustBeGoingBackToLab, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_PewterCity_EventScript_AideExit0
	call_if_eq VAR_TEMP_1, 1, Kanto_PewterCity_EventScript_AideExit1
	call_if_eq VAR_TEMP_1, 2, Kanto_PewterCity_EventScript_AideExit2
	call_if_eq VAR_TEMP_1, 3, Kanto_PewterCity_EventScript_AideExit3
	delay 30
	textcolor 3
	msgbox Kanto_PewterCity_Text_RunningShoesLetterFromMom, MSGBOX_DEFAULT
	closemessage
Kanto_PewterCity_EventScript_AideGone:
	removeobject 12
	setflag FLAG_SYS_B_DASH
	setflag 0x2223
	setvar VAR_TEMP_2, 0
	return

Kanto_PewterCity_EventScript_AideNoticePlayer::
	applymovement 12, Kanto_PewterCity_Movement_AideFaceDown
	waitmovement 0
	return

@ Aide already at player
Kanto_PewterCity_EventScript_AideApproachPlayer0::
	closemessage
	delay 20
	return

Kanto_PewterCity_EventScript_AideApproachPlayer1::
	closemessage
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_PlayerFaceUp
	waitmovement 0
	return

Kanto_PewterCity_EventScript_AideApproachPlayer2::
	closemessage
	applymovement 12, Kanto_PewterCity_Movement_AideApproachPlayerMid
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_PlayerFaceUp
	waitmovement 0
	return

Kanto_PewterCity_EventScript_AideApproachPlayer3::
	closemessage
	applymovement 12, Kanto_PewterCity_Movement_AideApproachPlayerBottom
	waitmovement 0
	applymovement LOCALID_PLAYER, Kanto_PewterCity_Movement_PlayerFaceUp
	waitmovement 0
	return

Kanto_PewterCity_EventScript_AideExit0::
	applymovement 12, Kanto_PewterCity_Movement_AideExit0
	waitmovement 0
	return

Kanto_PewterCity_EventScript_AideExit1::
	applymovement 12, Kanto_PewterCity_Movement_AideExit1
	waitmovement 0
	return

Kanto_PewterCity_EventScript_AideExit2::
	applymovement 12, Kanto_PewterCity_Movement_AideExit2
	waitmovement 0
	return

Kanto_PewterCity_EventScript_AideExit3::
	applymovement 12, Kanto_PewterCity_Movement_AideExit3
	waitmovement 0
	return

Kanto_PewterCity_Movement_AideApproachPlayerMid::
	delay_4
	walk_down
	step_end

Kanto_PewterCity_Movement_AideApproachPlayerBottom::
	delay_4
	walk_down
	walk_down
	step_end

Kanto_PewterCity_Movement_AideExit0::
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Kanto_PewterCity_Movement_AideExit1::
	walk_left
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Kanto_PewterCity_Movement_AideExit2::
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Kanto_PewterCity_Movement_AideExit3::
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Kanto_PewterCity_EventScript_AideHadShoes:
	msgbox Kanto_PewterCity_Text_AideHadShoes, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_PewterCity_EventScript_AideExit0
	call_if_eq VAR_TEMP_1, 1, Kanto_PewterCity_EventScript_AideExit1
	call_if_eq VAR_TEMP_1, 2, Kanto_PewterCity_EventScript_AideExit2
	call_if_eq VAR_TEMP_1, 3, Kanto_PewterCity_EventScript_AideExit3
	goto Kanto_PewterCity_EventScript_AideGone

@ (shown from BROCK's BOULDERBADGE until the RUNNING SHOES are delivered: FireRed's VAR_MAP_SCENE_PEWTER_CITY 1)
Kanto_PewterCity_EventScript_AideShow::
	setflag 0x2224
	goto_if_set 0x2223, Kanto_PewterCity_EventScript_AideShowEnd
	goto_if_unset FLAG_QZ_0x2100, Kanto_PewterCity_EventScript_AideShowEnd
	clearflag 0x2224
	setvar VAR_TEMP_2, 1
Kanto_PewterCity_EventScript_AideShowEnd:
	return

Kanto_PewterCity_Movement_AideExclamation:
	emote_exclamation_mark
	step_end

Kanto_PewterCity_Movement_AideDelay48:
	delay_16
	delay_16
	delay_16
	step_end

Kanto_PewterCity_Movement_AideFaceDown:
	walk_in_place_faster_down
	step_end

Kanto_PewterCity_Movement_PlayerFaceUp:
	walk_in_place_faster_up
	step_end

Kanto_PewterCity_Text_OhPlayer:
	.string "Oh, {PLAYER}{KUN}!$"

Kanto_PewterCity_Text_AskedToDeliverThis:
	.string "I'm glad I caught up to you.\n"
	.string "I'm PROF. OAK's AIDE.\p"
	.string "I've been asked to deliver this,\n"
	.string "so here you go.$"

Kanto_PewterCity_Text_ReceivedRunningShoesFromAide:
	.string "{PLAYER} received the\n"
	.string "RUNNING SHOES from the AIDE.$"

Kanto_PewterCity_Text_SwitchedShoesWithRunningShoes::
	.string "{PLAYER} switched shoes with the\n"
	.string "RUNNING SHOES.$"

Kanto_PewterCity_Text_ExplainRunningShoes::
	.string "Press the B Button to run.\n"
	.string "But only where there's room to run!$"

Kanto_PewterCity_Text_MustBeGoingBackToLab:
	.string "Well, I must be going back to\n"
	.string "the LAB.\p"
	.string "Bye-bye!$"

Kanto_PewterCity_Text_RunningShoesLetterFromMom:
	.string "There's a letter attached…\p"
	.string "Dear {PLAYER},\p"
	.string "Here is a pair of RUNNING SHOES\n"
	.string "for my beloved challenger.\p"
	.string "Remember, I'll always cheer for\n"
	.string "you! Don't ever give up!\p"
	.string "From Mom$"

Kanto_PewterCity_Text_AideHadShoes:
	.string "…Oh! You're already wearing a pair\n"
	.string "of RUNNING SHOES!\p"
	.string "Then I've come all this way for\n"
	.string "nothing. Well, keep up the running!$"
