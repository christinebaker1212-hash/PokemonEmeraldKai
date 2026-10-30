Johto_DragonShrine_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_DragonShrine_EventScript_83CC2F6
	.byte 0

Johto_DragonShrine_EventScript_83CC2F6::
	call Quetzal_EventScript_83CC300
	goto Quetzal_EventScript_840EA66

@ Johto_DragonShrine object 1 (local id 1, gfx 0x3EA6) at 6,9
Johto_DragonShrine_EventScript_83CC33E::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x14B9, Johto_DragonShrine_EventScript_83CC90C
	goto_if_ge VAR_QZ_0x5010, 4, Johto_DragonShrine_EventScript_83CCFD5
	msgbox Johto_DragonShrine_Text_83CCE5B, MSGBOX_DEFAULT
	release
	end

@ Johto_DragonShrine trigger at 5,16 (VAR_QZ_0x5010 == 2)
Johto_DragonShrine_EventScript_83CC35E::
	lock
	applymovement LOCALID_PLAYER, Johto_DragonShrine_Movement_83CC39A
	waitmovement LOCALID_PLAYER
	delay 0xA
	lockall
	goto Johto_DragonShrine_EventScript_83CC3C0

@ Johto_DragonShrine trigger at 6,15 (VAR_QZ_0x5010 == 2)
Johto_DragonShrine_EventScript_83CC372::
	lock
	applymovement LOCALID_PLAYER, Johto_DragonShrine_Movement_83CC3AA
	waitmovement LOCALID_PLAYER
	delay 0xA
	lockall
	goto Johto_DragonShrine_EventScript_83CC3C0

@ Johto_DragonShrine trigger at 7,16 (VAR_QZ_0x5010 == 2)
Johto_DragonShrine_EventScript_83CC386::
	lock
	applymovement LOCALID_PLAYER, Johto_DragonShrine_Movement_83CC3B0
	waitmovement LOCALID_PLAYER
	delay 0xA
	lockall
	goto Johto_DragonShrine_EventScript_83CC3C0

Johto_DragonShrine_EventScript_83CC3C0::
	msgbox Johto_DragonShrine_Text_83CC549, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5028, 0
	goto Johto_DragonShrine_EventScript_83CC3D2

Johto_DragonShrine_EventScript_83CC3D2::
	message Johto_DragonShrine_Text_83CC61B
	multichoice 0x14, 6, 0x4F, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_DragonShrine_EventScript_83CC40D
	goto_if_eq VAR_0x8000, 2, Johto_DragonShrine_EventScript_83CC40D
	goto_if_eq VAR_0x8000, 127, Johto_DragonShrine_EventScript_83CC3D2
	call Johto_DragonShrine_EventScript_83CC522
	goto Johto_DragonShrine_EventScript_83CC3D2

Johto_DragonShrine_EventScript_83CC40D::
	msgbox Johto_DragonShrine_Text_83CC718, MSGBOX_DEFAULT
	goto Johto_DragonShrine_EventScript_83CC41A

Johto_DragonShrine_EventScript_83CC41A::
	message Johto_DragonShrine_Text_83CC634
	multichoice 0x14, 6, 0x50, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_DragonShrine_EventScript_83CC455
	goto_if_eq VAR_0x8000, 1, Johto_DragonShrine_EventScript_83CC455
	goto_if_eq VAR_0x8000, 127, Johto_DragonShrine_EventScript_83CC41A
	call Johto_DragonShrine_EventScript_83CC522
	goto Johto_DragonShrine_EventScript_83CC41A

Johto_DragonShrine_EventScript_83CC455::
	msgbox Johto_DragonShrine_Text_83CC718, MSGBOX_DEFAULT
	goto Johto_DragonShrine_EventScript_83CC462

Johto_DragonShrine_EventScript_83CC462::
	message Johto_DragonShrine_Text_83CC653
	multichoice 0x14, 6, 0x51, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 1, Johto_DragonShrine_EventScript_83CC49D
	goto_if_eq VAR_0x8000, 2, Johto_DragonShrine_EventScript_83CC49D
	goto_if_eq VAR_0x8000, 127, Johto_DragonShrine_EventScript_83CC462
	call Johto_DragonShrine_EventScript_83CC522
	goto Johto_DragonShrine_EventScript_83CC462

Johto_DragonShrine_EventScript_83CC49D::
	msgbox Johto_DragonShrine_Text_83CC718, MSGBOX_DEFAULT
	goto Johto_DragonShrine_EventScript_83CC4AA

Johto_DragonShrine_EventScript_83CC4AA::
	message Johto_DragonShrine_Text_83CC67F
	multichoice 0x14, 6, 0x52, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_DragonShrine_EventScript_83CC4E5
	goto_if_eq VAR_0x8000, 2, Johto_DragonShrine_EventScript_83CC4E5
	goto_if_eq VAR_0x8000, 127, Johto_DragonShrine_EventScript_83CC4AA
	call Johto_DragonShrine_EventScript_83CC522
	goto Johto_DragonShrine_EventScript_83CC4AA

Johto_DragonShrine_EventScript_83CC4E5::
	msgbox Johto_DragonShrine_Text_83CC718, MSGBOX_DEFAULT
	goto Johto_DragonShrine_EventScript_83CC4F2

Johto_DragonShrine_EventScript_83CC4F2::
	message Johto_DragonShrine_Text_83CC6AB
	multichoice 0x14, 6, 0x53, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 2, Johto_DragonShrine_EventScript_83CC530
	goto_if_eq VAR_0x8000, 127, Johto_DragonShrine_EventScript_83CC4F2
	call Johto_DragonShrine_EventScript_83CC522
	goto Johto_DragonShrine_EventScript_83CC4F2

Johto_DragonShrine_EventScript_83CC522::
	msgbox Johto_DragonShrine_Text_83CC6E2, MSGBOX_DEFAULT
	addvar VAR_QZ_0x5028, 1
	return

Johto_DragonShrine_EventScript_83CC530::
	msgbox Johto_DragonShrine_Text_83CC718, MSGBOX_DEFAULT
	delay 0x14
	msgbox Johto_DragonShrine_Text_83CC72A, MSGBOX_DEFAULT
	closemessage
	goto Johto_DragonShrine_EventScript_83CC7D5

Johto_DragonShrine_EventScript_83CC7D5::
	clearflag FLAG_QZ_0x1059
	addobject 4
	playse SE_DOOR
	delay 0x28
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	turnobject LOCALID_PLAYER, 1
	delay 0x14
	applymovement 4, Johto_DragonShrine_Movement_83CCB93
	waitmovement 0
	turnobject 1, 3
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_DragonShrine_Text_83CCBB9, MSGBOX_DEFAULT
	applymovement 4, Johto_DragonShrine_Movement_83CCB9B
	waitmovement 0
	msgbox Johto_DragonShrine_Text_83CCC33, MSGBOX_DEFAULT
	applymovement 4, Johto_DragonShrine_Movement_83CCB9F
	waitmovement 0
	msgbox Johto_DragonShrine_Text_83CCC42, MSGBOX_DEFAULT
	applymovement 1, Johto_DragonShrine_Movement_83CCBA3
	waitmovement 0
	turnobject 4, 2
	msgbox Johto_DragonShrine_Text_83CCC6E, MSGBOX_DEFAULT
	playse SE_PIN
	applymovement 4, Quetzal_Movement_83607F9
	msgbox Johto_DragonShrine_Text_83CCCEE, MSGBOX_DEFAULT
	applymovement 4, Johto_DragonShrine_Movement_83CCBA8
	waitmovement 0
	msgbox Johto_DragonShrine_Text_83CCCFE, MSGBOX_DEFAULT
	message Johto_DragonShrine_Text_83CCD30
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_DragonShrine_Text_83CCD49, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B9, Johto_DragonShrine_EventScript_83CC8F8
	applymovement 1, Johto_DragonShrine_Movement_83CCBAB
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	turnobject 4, 2
	msgbox Johto_DragonShrine_Text_83CCDF7, MSGBOX_DEFAULT
	closemessage
	delay 0x14
	msgbox Johto_DragonShrine_Text_83CCE38, MSGBOX_DEFAULT
	closemessage
	turnobject 4, 4
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_DragonShrine_Text_83CC950, MSGBOX_DEFAULT
	call_if_set FLAG_QZ_0x14B9, Johto_DragonShrine_EventScript_83CC91B
	call_if_set FLAG_QZ_0x14B9, Johto_DragonShrine_EventScript_83CC947
	msgbox Johto_DragonShrine_Text_83CCA59, MSGBOX_DEFAULT
	closemessage
	applymovement 4, Johto_DragonShrine_Movement_83CCBB0
	waitmovement 0
	playse SE_EXIT
	setflag FLAG_QZ_0x1059
	removeobject 4
	setflag FLAG_QZ_0x1057
	setvar VAR_QZ_0x5010, 4
	setrespawn 0x22
	release
	end

Johto_DragonShrine_EventScript_83CC8F8::
	call Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B9
	setflag FLAG_QZ_0x11AF
	addvar VAR_QZ_0x502E, 1
	setflag FLAG_QZ_0x14B9
	return

Johto_DragonShrine_EventScript_83CC90C::
	msgbox Johto_DragonShrine_Text_83CCA3B, MSGBOX_DEFAULT
	call Johto_DragonShrine_EventScript_83CC91B
	release
	end

Johto_DragonShrine_EventScript_83CC91B::
	setorcopyvar VAR_0x8000, ITEM_DRAGON_SCALE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_DragonShrine_EventScript_83CC93E
	clearflag FLAG_QZ_0x14B9
	msgbox Johto_DragonShrine_Text_83CC9B2, MSGBOX_DEFAULT
	return

Johto_DragonShrine_EventScript_83CC93E::
	msgbox Quetzal_Text_8360DB1, MSGBOX_DEFAULT
	return

Johto_DragonShrine_EventScript_83CC947::
	msgbox Johto_DragonShrine_Text_83CC9FC, MSGBOX_DEFAULT
	return

@ Johto_DragonShrine object 2 (local id 2, gfx 0x3EB2) at 3,12
Johto_DragonShrine_EventScript_83CCEEC::
	msgbox Johto_DragonShrine_Text_83CCEFE, MSGBOX_NPC
	end

@ Johto_DragonShrine object 3 (local id 3, gfx 0x3EB2) at 9,12
Johto_DragonShrine_EventScript_83CCEF5::
	msgbox Johto_DragonShrine_Text_83CCF70, MSGBOX_NPC
	end

Johto_DragonShrine_EventScript_83CCFD5::
	goto_if_eq VAR_QZ_0x5028, 0, Johto_DragonShrine_EventScript_83CD025
	goto Johto_DragonShrine_EventScript_83CCFE5

Johto_DragonShrine_EventScript_83CCFE5::
	goto_if_set FLAG_QZ_0x105A, Johto_DragonShrine_EventScript_83CD059
	call Quetzal_EventScript_8361A4D
	msgbox Johto_DragonShrine_Text_83CD063, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, 0x93
	bufferspeciesname 0, SPECIES_DRATINI
	qz_givemon_ex SPECIES_DRATINI, 0x23, ITEM_NONE, 2, 0, 0
	call Johto_DragonShrine_EventScript_84017C6
	msgbox Johto_DragonShrine_Text_83CD0F5, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x105A
	release
	end

Johto_DragonShrine_EventScript_83CD025::
	goto_if_set FLAG_QZ_0x105A, Johto_DragonShrine_EventScript_83CD059
	call Quetzal_EventScript_8361A4D
	msgbox Johto_DragonShrine_Text_83CD063, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, 0x93
	bufferspeciesname 0, SPECIES_DRATINI
	qz_givestorymon 4
	call Quetzal_EventScript_840181E
	msgbox Johto_DragonShrine_Text_83CD0F5, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x105A
	release
	end

Johto_DragonShrine_EventScript_83CD059::
	msgbox Johto_DragonShrine_Text_83CD151, MSGBOX_DEFAULT
	release
	end

Johto_DragonShrine_EventScript_84017C6::
	goto_if_eq VAR_RESULT, 0, Johto_DragonShrine_EventScript_84017E1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_840180E
	goto Quetzal_EventScript_8401844

Johto_DragonShrine_EventScript_84017E1::
	playfanfare MUS_OBTAIN_ITEM
	message Quetzal_Text_840184F
	waitmessage
	waitfanfare
	msgbox Johto_DragonShrine_Text_840185F, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, 1, Johto_DragonShrine_EventScript_8401808
	call Quetzal_EventScript_8361B12
	call Quetzal_EventScript_836063E

Johto_DragonShrine_EventScript_8401808::
	setvar VAR_RESULT, 0
	return

Johto_DragonShrine_Movement_83CC39A:
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_DragonShrine_Movement_83CC3AA:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_DragonShrine_Movement_83CC3B0:
	walk_up
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_DragonShrine_Movement_83CCB93:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	face_right
	step_end

Johto_DragonShrine_Movement_83CCB9B:
	lock_facing_direction
	walk_fast_left
	unlock_facing_direction
	step_end

Johto_DragonShrine_Movement_83CCB9F:
	lock_facing_direction
	walk_slow_left
	unlock_facing_direction
	step_end

Johto_DragonShrine_Movement_83CCBA3:
	walk_left
	walk_left
	walk_left
	face_down
	step_end

Johto_DragonShrine_Movement_83CCBA8:
	walk_right
	walk_right
	step_end

Johto_DragonShrine_Movement_83CCBAB:
	walk_slow_right
	walk_slow_right
	walk_slow_right
	face_down
	step_end

Johto_DragonShrine_Movement_83CCBB0:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_DragonShrine_Text_83CC549::
	.string "Hm… Good to see you here.\p"
	.string "No need to explain why you came.\n"
	.string "CLAIR sent you here, didn’t she?\p"
	.string "That girl is a handful…\n"
	.string "I am sorry, but I must test you.\p"
	.string "Not to worry, you are to answer\n"
	.string "only a few questions.\l"
	.string "Ready?$"

Johto_DragonShrine_Text_83CC61B::
	.string "What are POKéMON to you?$"

Johto_DragonShrine_Text_83CC634::
	.string "What helps you to win battles?$"

Johto_DragonShrine_Text_83CC653::
	.string "What kind of trainer do you wish\n"
	.string "to battle?$"

Johto_DragonShrine_Text_83CC67F::
	.string "What is most important for\n"
	.string "raising POKéMON?$"

Johto_DragonShrine_Text_83CC6AB::
	.string "Strong POKéMON. Weak POKéMON.\n"
	.string "Which is more important?$"

Johto_DragonShrine_Text_83CC6E2::
	.string "Hah? I didn’t quite catch that…$"

Johto_DragonShrine_Text_83CC718::
	.string "Oh, I understand…$"

Johto_DragonShrine_Text_83CC72A::
	.string "Hm… I see…\p"
	.string "You care deeply for POKéMON.\p"
	.string "Very commendable.\p"
	.string "That conviction is what is\n"
	.string "important!\p"
	.string "{PLAYER}, don’t lose that belief.\p"
	.string "It will see you through at the\n"
	.string "POKéMON LEAGUE.$"

Johto_DragonShrine_Text_83CC950::
	.string "CLAIR: I’m sorry for how I treated\n"
	.string "you. You earned this BADGE.\p"
	.string "I want you to have something else.$"

Johto_DragonShrine_Text_83CC9B2::
	.string "It’s a DRAGON SCALE.\p"
	.string "A rare treasure carried by some\n"
	.string "Dragon-type POKéMON.$"

Johto_DragonShrine_Text_83CC9FC::
	.string "My grandfather will keep the gift.\n"
	.string "Ask him when you have room.$"

Johto_DragonShrine_Text_83CCA3B::
	.string "CLAIR left this gift for you.$"

Johto_DragonShrine_Text_83CCA59::
	.string "CLAIR: You have all eight BADGES.\p"
	.string "The way to the LEAGUE starts east\n"
	.string "of NEW BARK TOWN. Use SURF there.\p"
	.string "Once you become CHAMPION, you can\n"
	.string "challenge the GYM LEADERS again.\p"
	.string "When you reach the POKéMON LEAGUE,\n"
	.string "don’t you dare lose!\p"
	.string "If you do, I’ll feel even worse\n"
	.string "about having lost to you.\p"
	.string "Give it everything you’ve got.$"

Johto_DragonShrine_Text_83CCBB9::
	.string "So how did it go?\p"
	.string "I guess there’s no point\n"
	.string "in asking. You did fail?\p"
	.string "..................................\p"
	.string "…What? You passed?$"

Johto_DragonShrine_Text_83CCC33::
	.string "That can’t be!$"

Johto_DragonShrine_Text_83CCC42::
	.string "You’re lying!\p"
	.string "Even I haven’t been approved!$"

Johto_DragonShrine_Text_83CCC6E::
	.string "CLAIR!\p"
	.string "This child is impeccable,\n"
	.string "in skill and spirit!\p"
	.string "Admit defeat and confer the\n"
	.string "RISINGBADGE!\p"
	.string "…Or must I inform LANCE of this?$"

Johto_DragonShrine_Text_83CCCEE::
	.string "I-I understand…$"

Johto_DragonShrine_Text_83CCCFE::
	.string "Here, this is the RISINGBADGE…\p"
	.string "Hurry up! Take it!$"

Johto_DragonShrine_Text_83CCD30::
	.string "{PLAYER} received RISINGBADGE.$"

Johto_DragonShrine_Text_83CCD49::
	.string "RISINGBADGE will enable your\n"
	.string "POKéMON to use the move for\l"
	.string "climbing waterfalls.\p"
	.string "Also, all POKéMON will recognize\n"
	.string "you as a trainer and obey your\l"
	.string "every command without question.$"

Johto_DragonShrine_Text_83CCDF7::
	.string "CLAIR…\p"
	.string "Reflect upon what it is that you\n"
	.string "lack and this child has.$"

Johto_DragonShrine_Text_83CCE38::
	.string "..................................$"

Johto_DragonShrine_Text_83CCE5B::
	.string "Come again, if you so desire.$"

Johto_DragonShrine_Text_83CCEFE::
	.string "It’s been quite some time since a\n"
	.string "trainer has gained our MASTER’s\l"
	.string "rare approval.\p"
	.string "In fact, not since Master LANCE.$"

Johto_DragonShrine_Text_83CCF70::
	.string "You know young Master LANCE?\p"
	.string "He looks so much like our MASTER\n"
	.string "did in his youth.\p"
	.string "It’s in their blood.$"

Johto_DragonShrine_Text_83CD063::
	.string "Hm… Good to see you here.\p"
	.string "Your arrival is most fortunate.\p"
	.string "I have something for you.\p"
	.string "Take this DRATINI as proof that I\n"
	.string "have recognized your worth.$"

Johto_DragonShrine_Text_83CD0F5::
	.string "Dragon POKéMON are symbolic of our\n"
	.string "clan.\p"
	.string "You have shown that you can be\n"
	.string "entrusted with one.$"

Johto_DragonShrine_Text_83CD151::
	.string "CLAIR appears to have learned an\n"
	.string "invaluable lesson from you.\p"
	.string "I thank you as her grandfather.$"

Johto_DragonShrine_Text_840185F::
	.string "Would you like to give a nickname\n"
	.string "to this POKéMON?$"
