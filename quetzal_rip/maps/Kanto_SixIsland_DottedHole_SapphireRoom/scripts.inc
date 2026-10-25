Kanto_SixIsland_DottedHole_SapphireRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SixIsland_DottedHole_SapphireRoom object 1 (local id 1, gfx 0x3F42) at 7,7
Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8413F9E::
	lock
	faceplayer
	textcolor 3
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Kanto_SixIsland_DottedHole_SapphireRoom_Text_842C48B
	waitmessage
	waitfanfare
	closemessage
	setobjectxyperm 2, 5, 0
	addobject 2
	playse SE_FALL
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141F8
	waitmovement 0
	playse SE_M_STRENGTH
	call_if_eq VAR_FACING, 2, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140F8
	call_if_eq VAR_FACING, 1, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140F8
	call_if_eq VAR_FACING, 4, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414103
	call_if_eq VAR_FACING, 3, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414103
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0
	setvar VAR_0x8006, 0xC
	setvar VAR_0x8007, 3
	special ShakeCamera
	delay 0x3C
	call_if_eq VAR_FACING, 2, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841410E
	call_if_eq VAR_FACING, 1, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414119
	call_if_eq VAR_FACING, 4, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414124
	call_if_eq VAR_FACING, 3, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841412F
	textcolor 0
	msgbox Kanto_SixIsland_DottedHole_SapphireRoom_Text_842C4A0, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 2, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841413A
	call_if_eq VAR_FACING, 1, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841414F
	call_if_eq VAR_FACING, 4, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414164
	call_if_eq VAR_FACING, 3, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414179
	removeobject 1
	call_if_eq VAR_FACING, 2, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140E2
	call_if_eq VAR_FACING, 1, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140E2
	call_if_eq VAR_FACING, 4, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140ED
	call_if_eq VAR_FACING, 3, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140ED
	msgbox Kanto_SixIsland_DottedHole_SapphireRoom_Text_842C4DD, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 2, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841418E
	call_if_eq VAR_FACING, 1, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84141A0
	call_if_eq VAR_FACING, 4, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84141B2
	call_if_eq VAR_FACING, 3, Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84141C4
	playse SE_EXIT
	delay 0x23
	removeobject 2
	setflag FLAG_QZ_0x21DD
	release
	end

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140E2::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140ED::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84140F8::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414103::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841410E::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141D6
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414119::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141D6
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414124::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141DA
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841412F::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141DA
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841413A::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141DE
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841414F::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141DE
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414164::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141E0
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414179::
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141E0
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_841418E::
	applymovement LOCALID_PLAYER, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_8414205
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141E2
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84141A0::
	applymovement LOCALID_PLAYER, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_8414205
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141EA
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84141B2::
	applymovement LOCALID_PLAYER, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_8414205
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141F2
	waitmovement 0
	return

Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_84141C4::
	applymovement LOCALID_PLAYER, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_8414205
	applymovement 2, Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141F2
	waitmovement 0
	return

@ Kanto_SixIsland_DottedHole_SapphireRoom sign / bg event at 7,2
Kanto_SixIsland_DottedHole_SapphireRoom_EventScript_8414208::
	lockall
	setvar VAR_0x8005, 0x82
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459692
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596A4
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596B5
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596C2
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596D3
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596E5
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596F7
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459703
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459715
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459723
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459734
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459746
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459758
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_845976A
	call Quetzal_EventScript_8459552
	setvar VAR_0x8006, 0
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459779
	call Quetzal_EventScript_8459552
	braillemessage Kanto_SixIsland_DottedHole_SapphireRoom_Braille_845978B
	waitbuttonpress
	closebraillemessage
	releaseall
	end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141D6:
	walk_up
	walk_up
	walk_in_place_faster_right
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141DA:
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141DE:
	walk_right
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141E0:
	walk_up
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141E2:
	walk_fast_up
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_right
	walk_fast_right
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141EA:
	walk_fast_down
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_right
	walk_fast_right
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141F2:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_84141F8:
	face_down
	disable_anim
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	slide_down
	restore_anim
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Movement_8414205:
	delay_16
	walk_in_place_faster_right
	step_end

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459692:
	@ braille
	.string "      ßÌì ìÒÌ ì+ç$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596A4:
	@ braille
	.string "      ÔßÉììÌëÉéÔ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596B5:
	@ braille
	.string "      àìçéÌà$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596C2:
	@ braille
	.string "      çéÌ Éé ëÌÎ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596D3:
	@ braille
	.string "      çéÌ Éé ÈßぅÌ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596E5:
	@ braille
	.string "      ÂçééÌÂì ìÒÌ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_84596F7:
	@ braille
	.string "      áÀàìわ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459703:
	@ braille
	.string "      ì+ç ÊëÉÌéÎà$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459715:
	@ braille
	.string "      àÒÀëÉéÔ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459723:
	@ braille
	.string "      áç+Ìë çáÌé$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459734:
	@ braille
	.string "      À +ÉéÎç+ ìç$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459746:
	@ braille
	.string "      À éÌ+ +çëßÎ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459758:
	@ braille
	.string "      ìÒÀì Ôßç+àわ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_845976A:
	@ braille
	.string "      ìÒÌ éÌぉì$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_8459779:
	@ braille
	.string "      +çëßÎ +ÀÉìà$"

Kanto_SixIsland_DottedHole_SapphireRoom_Braille_845978B:
	@ braille
	.string "      Êçë ごçぅわ$"

Kanto_SixIsland_DottedHole_SapphireRoom_Text_842C48B::
	.string "{PLAYER} found a SAPPHIRE!$"

Kanto_SixIsland_DottedHole_SapphireRoom_Text_842C4A0::
	.string "Fufu… Fufufufu…\n"
	.string "I guessed right.\p"
	.string "I was right in tailing you!$"

Kanto_SixIsland_DottedHole_SapphireRoom_Text_842C4DD::
	.string "I knew that there was a SAPPHIRE\n"
	.string "here, so it belongs to me!\p"
	.string "I’ll sell it to TEAM ROCKET for\n"
	.string "serious money.\p"
	.string "…D-don’t glare at me like that!\p"
	.string "If you want it back, why don’t you\n"
	.string "go get it after I sell it?\p"
	.string "I’ll even tell you one of the\n"
	.string "passwords to TEAM ROCKET’s\l"
	.string "WAREHOUSE.\p"
	.string "The WAREHOUSE password I know is\n"
	.string "“Yes, nah, CHANSEY.”\p"
	.string "I’m done.\n"
	.string "Don’t think badly of me!$"
