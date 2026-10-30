Johto_BlackthornCity_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_BlackthornCity_OnFrame_83C6860
	map_script MAP_SCRIPT_ON_LOAD, Johto_BlackthornCity_EventScript_83C6839
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_BlackthornCity_EventScript_83C686A
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_BlackthornCity_OnFrame_83C6860:
	map_script_2 VAR_QZ_0x5010, 4, Johto_BlackthornCity_EventScript_83C6888
	.2byte 0

Johto_BlackthornCity_EventScript_83C6839::
	special QzJohtoTravelingEventCheck
	goto_if_unset FLAG_QZ_0x11B6, Johto_BlackthornCity_EventScript_83C6858
	goto_if_unset FLAG_QZ_0x11B7, Johto_BlackthornCity_EventScript_83C6858
	goto_if_unset FLAG_QZ_0x11B8, Johto_BlackthornCity_EventScript_83C6858
	end

Johto_BlackthornCity_EventScript_83C6858::
	setobjectxyperm 2, 0x18, 0x1B
	end

Johto_BlackthornCity_EventScript_83C686A::
	special QzJohtoTravelingEventCheck
	goto_if_set FLAG_QZ_0x1318, Johto_BlackthornCity_EventScript_83C6887
	call Quetzal_EventScript_83CC300
	goto_if_set FLAG_QZ_0x1425, Johto_BlackthornCity_EventScript_83C6887
	setflag FLAG_QZ_0x11C5

Johto_BlackthornCity_EventScript_83C6887::
	end

Johto_BlackthornCity_EventScript_83C6888::
	lock
	msgbox Johto_BlackthornCity_Text_83C68ED, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5010, 5
	setvar VAR_QZ_0x5004, 7
	release
	end

@ Johto_BlackthornCity object 7 (local id 7, gfx 0x3EB2) at 25,13
Johto_BlackthornCity_EventScript_83C694D::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5010, 1, Johto_BlackthornCity_EventScript_83C6965
	msgbox Johto_BlackthornCity_Text_83C6C68, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornCity_EventScript_83C6965::
	msgbox Johto_BlackthornCity_Text_83C6C31, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BlackthornCity trigger at 26,13 (VAR_QZ_0x5010 == 1)
Johto_BlackthornCity_EventScript_83C6970::
	lock
	turnobject 7, 4
	msgbox Johto_BlackthornCity_Text_83C6C31, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	turnobject 7, 1
	release
	end

@ Johto_BlackthornCity object 2 (local id 2, gfx 0x3E87) at 25,27
Johto_BlackthornCity_EventScript_83C698E::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x11B6, Johto_BlackthornCity_EventScript_83C69C1
	goto_if_unset FLAG_QZ_0x11B7, Johto_BlackthornCity_EventScript_83C69C1
	goto_if_unset FLAG_QZ_0x11B8, Johto_BlackthornCity_EventScript_83C69C1
	goto_if_eq VAR_QZ_0x5010, 1, Johto_BlackthornCity_EventScript_83C69CC
	msgbox Johto_BlackthornCity_Text_83C6BD2, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornCity_EventScript_83C69C1::
	msgbox Johto_BlackthornCity_Text_83C6AEA, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornCity_EventScript_83C69CC::
	msgbox Johto_BlackthornCity_Text_83C6B61, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BlackthornCity object 6 (local id 6, gfx 0x2C) at 33,44
Johto_BlackthornCity_EventScript_83C69D7::
	msgbox Johto_BlackthornCity_Text_83C6CB5, MSGBOX_NPC
	end

@ Johto_BlackthornCity object 5 (local id 5, gfx 0x3E85) at 12,40
Johto_BlackthornCity_EventScript_83C69E0::
	msgbox Johto_BlackthornCity_Text_83C6CEF, MSGBOX_NPC
	end

@ Johto_BlackthornCity object 1 (local id 1, gfx 0x23) at 16,28
Johto_BlackthornCity_EventScript_83C69E9::
	msgbox Johto_BlackthornCity_Text_83C6D27, MSGBOX_NPC
	end

@ Johto_BlackthornCity object 4 (local id 4, gfx 0x3E8F) at 44,31
Johto_BlackthornCity_EventScript_83C69F2::
	msgbox Johto_BlackthornCity_Text_83C6E49, MSGBOX_NPC
	end

@ Johto_BlackthornCity sign / bg event at 42,42
Johto_BlackthornCity_EventScript_83C69FB::
	msgbox Johto_BlackthornCity_Text_83C6E91, MSGBOX_SIGN
	end

@ Johto_BlackthornCity sign / bg event at 26,26
Johto_BlackthornCity_EventScript_83C6A04::
	msgbox Johto_BlackthornCity_Text_83C6EBA, MSGBOX_SIGN
	end

@ Johto_BlackthornCity sign / bg event at 13,49
Johto_BlackthornCity_EventScript_83C6A0D::
	msgbox Johto_BlackthornCity_Text_83C6F07, MSGBOX_SIGN
	end

@ Johto_BlackthornCity sign / bg event at 28,13
Johto_BlackthornCity_EventScript_83C6A16::
	msgbox Johto_BlackthornCity_Text_83C6F18, MSGBOX_SIGN
	end

@ Johto_BlackthornCity object 3 (local id 3, gfx 0x11) at 27,39
Johto_BlackthornCity_EventScript_83C6A1F::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1055, Johto_BlackthornCity_EventScript_83C6ADF
	msgbox Johto_BlackthornCity_Text_83C6D5C, MSGBOX_DEFAULT
	closemessage
	multichoice 0x14, 0, 0x4A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_BlackthornCity_EventScript_83C6AD1
	goto_if_eq VAR_0x8000, 1, Johto_BlackthornCity_EventScript_83C6AD1
	goto_if_eq VAR_0x8000, 2, Johto_BlackthornCity_EventScript_83C6AD1
	goto_if_eq VAR_0x8000, 3, Johto_BlackthornCity_EventScript_83C6AD1
	goto_if_eq VAR_0x8000, 4, Johto_BlackthornCity_EventScript_83C6AD1
	goto_if_eq VAR_0x8000, 5, Johto_BlackthornCity_EventScript_83C6AD1
	goto_if_eq VAR_0x8000, 6, Johto_BlackthornCity_EventScript_83C6AA4
	goto_if_eq VAR_0x8000, 7, Johto_BlackthornCity_EventScript_83C6ADC
	goto_if_eq VAR_0x8000, 127, Johto_BlackthornCity_EventScript_83C6ADC
	closemessage
	release
	end

Johto_BlackthornCity_EventScript_83C6AA4::
	msgbox Johto_BlackthornCity_Text_83C6D96, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_SPELL_TAG
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1055
	msgbox Johto_BlackthornCity_Text_83C6DA9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornCity_EventScript_83C6AD1::
	msgbox Johto_BlackthornCity_Text_83C6E3D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornCity_EventScript_83C6ADC::
	closemessage
	release
	end

Johto_BlackthornCity_EventScript_83C6ADF::
	msgbox Johto_BlackthornCity_Text_83C6DF3, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornCity_Text_83C68ED::
	.string "A message from PROF. ELM arrived:\p"
	.string "I have something for you.\n"
	.string "Please come to my LAB when you can.$"

Johto_BlackthornCity_Text_83C6AEA::
	.string "CLAIR, our GYM LEADER, has been\n"
	.string "following your progress.\p"
	.string "However, she only battles trainers\n"
	.string "who already have 7 badges.$"

Johto_BlackthornCity_Text_83C6B61::
	.string "CLAIR, our GYM LEADER, is waiting\n"
	.string "for you.\p"
	.string "However, it would be impossible for\n"
	.string "a run-of-the-mill trainer to win.$"

Johto_BlackthornCity_Text_83C6BD2::
	.string "You defeated CLAIR?\p"
	.string "That’s amazing!\p"
	.string "I’ve never heard of her losing to\n"
	.string "anyone other than LANCE.$"

Johto_BlackthornCity_Text_83C6C31::
	.string "No. Only chosen trainers may train\n"
	.string "here.\p"
	.string "Please leave.$"

Johto_BlackthornCity_Text_83C6C68::
	.string "If CLAIR allows it, her grand-\n"
	.string "father--our MASTER--will also.\p"
	.string "You may enter.$"

Johto_BlackthornCity_Text_83C6CB5::
	.string "Arooo! Voices in my head!\p"
	.string "Huh? I’m listening to my radio!$"

Johto_BlackthornCity_Text_83C6CEF::
	.string "Are you going to make your POKéMON\n"
	.string "remember some moves?$"

Johto_BlackthornCity_Text_83C6D27::
	.string "Dragon masters all come from the city\n"
	.string "of BLACKTHORN.$"

Johto_BlackthornCity_Text_83C6D5C::
	.string "SANTOS: …\p"
	.string "I’m SANTOS of the weekday family.\p"
	.string "Take a guess.$"

Johto_BlackthornCity_Text_83C6D96::
	.string "You can have this…$"

Johto_BlackthornCity_Text_83C6DA9::
	.string "SANTOS: …\p"
	.string "SPELL TAG…\p"
	.string "GHOST-type moves get stronger…\p"
	.string "It will frighten you…$"

Johto_BlackthornCity_Text_83C6DF3::
	.string "SANTOS: …\p"
	.string "See you again on another Saturday…\p"
	.string "I won’t have any more gifts…$"

Johto_BlackthornCity_Text_83C6E3D::
	.string "SANTOS: No.$"

Johto_BlackthornCity_Text_83C6E49::
	.string "Wow, you came through the ICE PATH?\p"
	.string "You must be a real hotshot trainer!$"

Johto_BlackthornCity_Text_83C6E91::
	.string "BLACKTHORN CITY\p"
	.string "A Quiet Mountain Retreat$"

Johto_BlackthornCity_Text_83C6EBA::
	.string "BLACKTHORN CITY POKéMON GYM\n"
	.string "LEADER: CLAIR\p"
	.string "The Blessed User of Dragon POKéMON$"

Johto_BlackthornCity_Text_83C6F07::
	.string "MOVE BROS’ HOUSE$"

Johto_BlackthornCity_Text_83C6F18::
	.string "DRAGON’S DEN AHEAD$"
