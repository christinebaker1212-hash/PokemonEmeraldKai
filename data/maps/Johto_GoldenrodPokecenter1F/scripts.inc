Johto_GoldenrodPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_GoldenrodPokecenter1F_EventScript_83D6887
	.byte 0

Johto_GoldenrodPokecenter1F_EventScript_83D6887::
	setrespawn 0x1B
	setflag FLAG_JOHTO_HIDE_GS_RECEPTIONIST
	specialvar VAR_TEMP_9, MP_JohtoGsBallReady
	end

@ Moemon Kai: Crystal's GS BALL (GoldenrodPokecenter1F.asm GoldenrodPokecenter1F_GSBallScene*): once JOHTO's LEAGUE is
@ beaten, a receptionist brings it to the player at the door (Quetzal left it in the RUINS OF ALPH)
Johto_GoldenrodPokecenter1F_EventScript_GsBall::
	lockall
	setvar VAR_TEMP_9, 0
	clearflag FLAG_JOHTO_HIDE_GS_RECEPTIONIST
	addobject 10
	playse SE_EXIT
	applymovement 10, Johto_GoldenrodPokecenter1F_Movement_GsBallApproach
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_EAST
	msgbox Johto_GoldenrodPokecenter1F_Text_GsBallPleaseAccept, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_GS_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setflag FLAG_JOHTO_GOT_GS_BALL
	msgbox Johto_GoldenrodPokecenter1F_Text_GsBallComeAgain, MSGBOX_DEFAULT
	closemessage
	applymovement 10, Johto_GoldenrodPokecenter1F_Movement_GsBallLeave
	waitmovement 0
	removeobject 10
	setflag FLAG_JOHTO_HIDE_GS_RECEPTIONIST
	releaseall
	end

Johto_GoldenrodPokecenter1F_Movement_GsBallApproach:
	walk_left
	walk_left
	walk_left
	face_left
	step_end

Johto_GoldenrodPokecenter1F_Movement_GsBallLeave:
	walk_right
	walk_right
	walk_right
	step_end

Johto_GoldenrodPokecenter1F_Text_GsBallPleaseAccept::
	.string "{PLAYER}, isn’t it?\p"
	.string "Congratulations!\p"
	.string "As a special deal, a GS BALL has\n"
	.string "been sent just for you!\p"
	.string "Please accept it!$"

Johto_GoldenrodPokecenter1F_Text_GsBallComeAgain::
	.string "Please do come again!$"


@ Johto_GoldenrodPokecenter1F object 4 (local id 4, gfx 0x3E96) at 2,5
Johto_GoldenrodPokecenter1F_EventScript_83D688B::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D6916, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodPokecenter1F object 6 (local id 6, gfx 0x3E96) at 1,5
Johto_GoldenrodPokecenter1F_EventScript_83D6898::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D6952, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodPokecenter1F object 5 (local id 5, gfx 0x1E) at 14,6
Johto_GoldenrodPokecenter1F_EventScript_83D68A5::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D68BF, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodPokecenter1F object 3 (local id 3, gfx 0x2F) at 11,5
Johto_GoldenrodPokecenter1F_EventScript_83D68B2::
	lock
	faceplayer
	msgbox Johto_GoldenrodPokecenter1F_Text_83D69A0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodPokecenter1F_Text_83D68BF::
	.string "I just love living in the big city!\n"
	.string "I moved here when I was a young\l"
	.string "trainer, like you!$"

Johto_GoldenrodPokecenter1F_Text_83D6916::
	.string "This is a special MOéMON CENTER.\n"
	.string "The roof is made of gold!$"

Johto_GoldenrodPokecenter1F_Text_83D6952::
	.string "I told my brother that the roof is\n"
	.string "made of real gold. He’ll believe\l"
	.string "anything!$"

Johto_GoldenrodPokecenter1F_Text_83D69A0::
	.string "A higher level MOéMON doesn’t\n"
	.string "always win.\p"
	.string "After all, it may have a type\n"
	.string "disadvantage.\p"
	.string "I don’t think there is a single\n"
	.string "MOéMON that is the toughest.$"
