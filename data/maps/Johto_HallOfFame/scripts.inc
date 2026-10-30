Johto_HallOfFame_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_HallOfFame_OnFrame_83E7E03
	.byte 0

Johto_HallOfFame_OnFrame_83E7E03:
	map_script_2 VAR_QZ_0x502A, 6, Johto_HallOfFame_EventScript_83E7E0D
	.2byte 0

Johto_HallOfFame_EventScript_83E7E0D::
	lock
	goto_if_set FLAG_QZ_0x11CA, Johto_HallOfFame_EventScript_83E7E65
	applymovement 1, Johto_HallOfFame_Movement_83E7EDB
	applymovement LOCALID_PLAYER, Johto_HallOfFame_Movement_83E7EE5
	waitmovement LOCALID_PLAYER
	waitmovement 1
	msgbox Johto_HallOfFame_Text_83E7EEF, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360831
	waitmovement 0
	delay 0x14
	dofieldeffect 0x3E
	waitfieldeffect 0x3E
	delay 0x28
	call Quetzal_EventScript_835F6E4
	setvar VAR_TEMP_1, 1
	call Johto_HallOfFame_EventScript_83E7EB3
	setrespawn 0x17
	fadescreenspeed 1, 0x18
	special GameClear
	waitstate
	releaseall
	end

Johto_HallOfFame_EventScript_83E7E65::
	applymovement 1, Johto_HallOfFame_Movement_83E7EDB
	applymovement LOCALID_PLAYER, Johto_HallOfFame_Movement_83E7EE5
	waitmovement LOCALID_PLAYER
	waitmovement 1
	msgbox Johto_HallOfFame_Text_83E80C8, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360831
	waitmovement 0
	delay 0x14
	dofieldeffect 0x3E
	waitfieldeffect 0x3E
	delay 0x28
	call Quetzal_EventScript_835F6E4
	setvar VAR_TEMP_1, 1
	call Johto_HallOfFame_EventScript_83E7ECF
	setrespawn 0x17
	fadescreenspeed 1, 0x18
	special GameClear
	waitstate
	releaseall
	end

Johto_HallOfFame_EventScript_83E7EB3::
	setflag FLAG_QZ_0x1061
	setflag FLAG_QZ_0x11B1
	setflag FLAG_QZ_0x11CA
	setflag FLAG_QZ_0x1046
	setvar VAR_QZ_0x502A, 1
	setvar VAR_QZ_0x5003, 6
	setvar VAR_QZ_0x502C, 0
	return

Johto_HallOfFame_EventScript_83E7ECF::
	setflag FLAG_QZ_0x11B1
	setflag FLAG_QZ_0x11CA
	setvar VAR_QZ_0x502A, 1
	return

Johto_HallOfFame_Movement_83E7EDB:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	face_left
	step_end

Johto_HallOfFame_Movement_83E7EE5:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	face_right
	step_end

Johto_HallOfFame_Text_83E7EEF::
	.string "LANCE: It’s been a long time since I\n"
	.string "last came here.\p"
	.string "This is where we honor the LEAGUE\n"
	.string "CHAMPIONS for all eternity.\p"
	.string "Their courageous POKéMON are also\n"
	.string "inducted.\p"
	.string "Here today, we witnessed the rise\n"
	.string "of a new LEAGUE CHAMPION--a\p"
	.string "trainer who feels compassion for,\n"
	.string "and trust toward, all POKéMON.\p"
	.string "A trainer who succeeded through\n"
	.string "perseverance and determination.\p"
	.string "The new LEAGUE CHAMPION who has\n"
	.string "all the makings of greatness!\p"
	.string "{PLAYER}, allow me to register you\n"
	.string "and your partners as CHAMPIONS!$"

Johto_HallOfFame_Text_83E80C8::
	.string "LANCE: Once again, you have bested\n"
	.string "JOHTO’s finest trainers.\p"
	.string "The bond with your POKéMON remains\n"
	.string "the foundation of every victory.\p"
	.string "{PLAYER}, it is my honor to register\n"
	.string "you as CHAMPIONS once again.$"
