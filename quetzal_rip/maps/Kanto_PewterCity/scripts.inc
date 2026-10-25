Kanto_PewterCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PewterCity_EventScript_841599D
	.byte 0

Kanto_PewterCity_EventScript_841599D::
	special QzKantoTravelingEventCheck_773
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
	special QzQuestLogNop_737
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
	special QzQuestLogNop_737
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
	.string "There aren’t many serious POKéMON\n"
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
	.string "POKéMON out of my garden!$"

Kanto_PewterCity_Text_842E996::
	.string "You’re a TRAINER, right?\p"
	.string "BROCK’s looking for new\n"
	.string "challengers. Follow me!$"

Kanto_PewterCity_Text_842E9DF::
	.string "If you have the right stuff,\n"
	.string "go take on BROCK!$"

Kanto_PewterCity_Text_842EA0E::
	.string "TRAINER TIPS\p"
	.string "All POKéMON that appear in battle,\n"
	.string "however briefly, earn EXP Points.$"

Kanto_PewterCity_Text_842EA60::
	.string "NOTICE!\p"
	.string "Thieves have been stealing POKéMON\n"
	.string "fossils from MT. MOON.\p"
	.string "Please call the PEWTER POLICE if\n"
	.string "you have any information.$"

Kanto_PewterCity_Text_842EADD::
	.string "PEWTER MUSEUM OF SCIENCE$"

Kanto_PewterCity_Text_842EAF6::
	.string "PEWTER CITY POKéMON GYM\n"
	.string "LEADER: BROCK\l"
	.string "The Rock-Solid POKéMON TRAINER!$"

Kanto_PewterCity_Text_842EB3C::
	.string "PEWTER CITY\n"
	.string "A Stone Gray City$"
