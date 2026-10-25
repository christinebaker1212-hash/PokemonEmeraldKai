Kanto_CinnabarIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CinnabarIsland_EventScript_8416BAA
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_CinnabarIsland_OnFrame_8416C3A
	.byte 0

Kanto_CinnabarIsland_OnFrame_8416C3A:
	map_script_2 VAR_QZ_0x510B, 1, Kanto_CinnabarIsland_EventScript_8416C54
	map_script_2 VAR_QZ_0x510A, 1, Kanto_CinnabarIsland_EventScript_8416C99
	map_script_2 VAR_QZ_0x510A, 3, Kanto_CinnabarIsland_EventScript_8416C6C
	.2byte 0

Kanto_CinnabarIsland_EventScript_8416BAA::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2237
	call Kanto_CinnabarIsland_EventScript_8416C2A
	call_if_eq VAR_QZ_0x510A, 1, Kanto_CinnabarIsland_EventScript_8416BF0
	call_if_eq VAR_QZ_0x510A, 3, Kanto_CinnabarIsland_EventScript_8416C1E
	call_if_eq VAR_QZ_0x510B, 1, Kanto_CinnabarIsland_EventScript_8416BD7
	end

Kanto_CinnabarIsland_EventScript_8416BD7::
	savebgm MUS_RG_FOLLOW_ME
	setflag FLAG_TEMP_2
	setobjectxyperm 3, 0xF, 0xC
	setobjectmovementtype 3, 9
	setobjectxyperm 4, 0x1E, 0xC
	return

Kanto_CinnabarIsland_EventScript_8416BF0::
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ge VAR_0x8004, 18, Kanto_CinnabarIsland_EventScript_8416C16
	setflag FLAG_TEMP_2
	setobjectxyperm 3, 0x12, 0xC
	setobjectmovementtype 3, 7
	setobjectxyperm 4, 0x1E, 0xC
	return

Kanto_CinnabarIsland_EventScript_8416C16::
	setobjectxyperm 4, 0x1E, 8
	return

Kanto_CinnabarIsland_EventScript_8416C1E::
	setobjectxyperm 3, 0x15, 8
	setobjectmovementtype 3, 7
	return

Kanto_CinnabarIsland_EventScript_8416C2A::
	goto_if_set FLAG_QZ_0x21A9, Kanto_CinnabarIsland_EventScript_8416C34
	return

Kanto_CinnabarIsland_EventScript_8416C34::
	setvar VAR_TEMP_1, 1
	return

Kanto_CinnabarIsland_EventScript_8416C54::
	lockall
	clearflag FLAG_QZ_0x2130
	savebgm MUS_DUMMY
	textcolor 0
	setvar VAR_QZ_0x510B, 2
	delay 0x14
	call Kanto_CinnabarIsland_EventScript_8416DB2
	releaseall
	end

Kanto_CinnabarIsland_EventScript_8416C6C::
	lockall
	textcolor 0
	msgbox Kanto_CinnabarIsland_Text_84310E5, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Kanto_CinnabarIsland_Movement_8416C92
	waitmovement 0
	removeobject 3
	setvar VAR_QZ_0x510A, 4
	setflag FLAG_QZ_0x21D4
	clearflag FLAG_QZ_0x219A
	releaseall
	end

Kanto_CinnabarIsland_EventScript_8416C99::
	lockall
	textcolor 0
	call_if_unset FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416CFE
	call_if_set FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416D09
	playse SE_PIN
	applymovement 3, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 3, Quetzal_Movement_8457AA0
	waitmovement 0
	call_if_unset FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416D14
	call_if_set FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416D1F
	msgbox Kanto_CinnabarIsland_Text_8430E6C, MSGBOX_DEFAULT
	msgbox Kanto_CinnabarIsland_Text_8430E86, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_EventScript_8416D34
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_EventScript_8416D44
	end

Kanto_CinnabarIsland_EventScript_8416CFE::
	applymovement 3, Quetzal_Movement_8457AAC
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416D09::
	applymovement 3, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416D14::
	applymovement 3, Kanto_CinnabarIsland_Movement_8416E70
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416D1F::
	applymovement 3, Kanto_CinnabarIsland_Movement_8416E72
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416D34::
	msgbox Kanto_CinnabarIsland_Text_8430FB9, MSGBOX_DEFAULT
	closemessage
	call Kanto_CinnabarIsland_EventScript_8416DB2
	releaseall
	end

Kanto_CinnabarIsland_EventScript_8416D44::
	msgbox Kanto_CinnabarIsland_Text_8430FD4, MSGBOX_DEFAULT
	closemessage
	call_if_unset FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416D6C
	call_if_set FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416D7F
	removeobject 3
	setvar VAR_QZ_0x510A, 2
	clearflag FLAG_QZ_0x2180
	releaseall
	end

Kanto_CinnabarIsland_EventScript_8416D6C::
	applymovement 3, Kanto_CinnabarIsland_Movement_8416D77
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416D7F::
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_Movement_8416DA7
	applymovement 3, Kanto_CinnabarIsland_Movement_8416DAA
	waitmovement 0
	opendoor 0xE, 0xB
	waitdooranim
	applymovement 3, Kanto_CinnabarIsland_Movement_8416DAE
	waitmovement 0
	closedoor 0xE, 0xB
	waitdooranim
	return

Kanto_CinnabarIsland_EventScript_8416DB2::
	call_if_set FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416E0E
	addobject 4
	call_if_unset FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416E20
	call_if_set FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416E32
	msgbox Kanto_CinnabarIsland_Text_8431089, MSGBOX_DEFAULT
	closemessage
	call_if_unset FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416E3D
	call_if_set FLAG_TEMP_2, Kanto_CinnabarIsland_EventScript_8416E4F
	setvar VAR_QZ_0x510A, 2
	removeobject 3
	delay 0x28
	setflag FLAG_QZ_0x2181
	setvar VAR_QZ_0x5116, 1
	setvar VAR_0x8004, 8
	setvar VAR_0x8006, 1
	goto Quetzal_EventScript_8459475

Kanto_CinnabarIsland_EventScript_8416E0E::
	applymovement 3, Kanto_CinnabarIsland_Movement_8416E7B
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_Movement_8416E7B
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416E20::
	applymovement 3, Kanto_CinnabarIsland_Movement_8416E76
	applymovement 4, Kanto_CinnabarIsland_Movement_8416E61
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416E32::
	applymovement 4, Kanto_CinnabarIsland_Movement_8416E61
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416E3D::
	applymovement 3, Kanto_CinnabarIsland_Movement_8416E69
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_Movement_8416E86
	waitmovement 0
	return

Kanto_CinnabarIsland_EventScript_8416E4F::
	applymovement 3, Kanto_CinnabarIsland_Movement_8416E82
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_Movement_8416E90
	waitmovement 0
	return

@ Kanto_CinnabarIsland trigger at 20,5 (VAR_TEMP_1 == 0)
Kanto_CinnabarIsland_EventScript_8416E96::
	lockall
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	delay 0x14
	msgbox Kanto_CinnabarIsland_Text_8430CFC, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_Movement_8416EB9
	waitmovement 0
	releaseall
	end

@ Kanto_CinnabarIsland object 1 (local id 1, gfx 0x3F58) at 14,6
Kanto_CinnabarIsland_EventScript_8416EBB::
	lock
	faceplayer
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 3
	special QzQuestLogNop_737
	msgbox Kanto_CinnabarIsland_Text_8430D10, MSGBOX_DEFAULT
	release
	end

@ Kanto_CinnabarIsland object 2 (local id 2, gfx 0x3F2C) at 11,11
Kanto_CinnabarIsland_EventScript_8416ED4::
	msgbox Kanto_CinnabarIsland_Text_8430D7C, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland sign / bg event at 12,3
Kanto_CinnabarIsland_EventScript_8416EDD::
	msgbox Kanto_CinnabarIsland_Text_8430DE9, MSGBOX_SIGN
	end

@ Kanto_CinnabarIsland sign / bg event at 9,9
@ Kanto_CinnabarIsland sign / bg event at 10,9
Kanto_CinnabarIsland_EventScript_8416EE6::
	msgbox Kanto_CinnabarIsland_Text_8430E1A, MSGBOX_SIGN
	end

@ Kanto_CinnabarIsland sign / bg event at 22,5
Kanto_CinnabarIsland_EventScript_8416EEF::
	lockall
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 0
	special QzQuestLogNop_737
	msgbox Kanto_CinnabarIsland_Text_8430E26, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CinnabarIsland_Movement_8416C92:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_CinnabarIsland_Movement_8416D77:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_CinnabarIsland_Movement_8416DA7:
	walk_left
	walk_in_place_faster_right
	step_end

Kanto_CinnabarIsland_Movement_8416DAA:
	delay_16
	walk_left
	walk_in_place_faster_up
	step_end

Kanto_CinnabarIsland_Movement_8416DAE:
	walk_up
	delay_4
	set_invisible
	step_end

Kanto_CinnabarIsland_Movement_8416E61:
	delay_16
	delay_16
	walk_fast_left
	walk_fast_left
	walk_left
	walk_slow_left
	walk_slow_left
	step_end

Kanto_CinnabarIsland_Movement_8416E69:
	walk_right
	walk_down
	walk_down
	walk_right
	delay_4
	set_invisible
	step_end

Kanto_CinnabarIsland_Movement_8416E70:
	walk_up
	step_end

Kanto_CinnabarIsland_Movement_8416E72:
	walk_left
	walk_left
	walk_left
	step_end

Kanto_CinnabarIsland_Movement_8416E76:
	delay_16
	delay_16
	delay_8
	walk_in_place_faster_right
	step_end

Kanto_CinnabarIsland_Movement_8416E7B:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_CinnabarIsland_Movement_8416E82:
	walk_right
	delay_4
	set_invisible
	step_end

Kanto_CinnabarIsland_Movement_8416E86:
	walk_down
	walk_right
	walk_down
	walk_down
	walk_in_place_faster_right
	delay_4
	walk_right
	delay_4
	set_invisible
	step_end

Kanto_CinnabarIsland_Movement_8416E90:
	walk_right
	delay_4
	walk_right
	delay_4
	set_invisible
	step_end

Kanto_CinnabarIsland_Movement_8416EB9:
	walk_down
	step_end

Kanto_CinnabarIsland_Text_8430CFC::
	.string "The door is locked…$"

Kanto_CinnabarIsland_Text_8430D10::
	.string "CINNABAR GYM’s BLAINE is quite the\n"
	.string "odd fellow.\p"
	.string "He’s lived on the island since way\n"
	.string "before the LAB was built.$"

Kanto_CinnabarIsland_Text_8430D7C::
	.string "Scientists conduct experiments \n"
	.string "in the burned-out building.\p"
	.string "You know, the one they call the\n"
	.string "POKéMON MANSION.$"

Kanto_CinnabarIsland_Text_8430DE9::
	.string "CINNABAR ISLAND\n"
	.string "The Fiery Town of Burning Desire$"

Kanto_CinnabarIsland_Text_8430E1A::
	.string "POKéMON LAB$"

Kanto_CinnabarIsland_Text_8430E26::
	.string "CINNABAR ISLAND POKéMON GYM\n"
	.string "LEADER: BLAINE\l"
	.string "The Hotheaded Quiz Master!$"

Kanto_CinnabarIsland_Text_8430E6C::
	.string "Huh?\n"
	.string "Hey, if it isn’t {PLAYER}!$"

Kanto_CinnabarIsland_Text_8430E86::
	.string "Look, it’s me, BILL.\n"
	.string "Long time no see!\p"
	.string "I hope you’re still using my\n"
	.string "PC system.\p"
	.string "Well, listen, since we met up here,\n"
	.string "how about spending time with me?\p"
	.string "There’s this little island in the far\n"
	.string "south called ONE ISLAND.\p"
	.string "A friend invited me, so I’m on my\n"
	.string "way out there.\p"
	.string "How about it?\n"
	.string "Do you feel like coming with me?$"

Kanto_CinnabarIsland_Text_8430FB9::
	.string "All right, then.\n"
	.string "Let’s go!$"

Kanto_CinnabarIsland_Text_8430FD4::
	.string "What, are you too busy?\p"
	.string "Well, all right.\n"
	.string "The boat hasn’t arrived yet anyway.\p"
	.string "I’ll be waiting at the POKéMON\n"
	.string "CENTER over there.\p"
	.string "Come see me when you’re done with\n"
	.string "your business here.$"

Kanto_CinnabarIsland_Text_8431089::
	.string "Looks like my pal’s boat arrived,\n"
	.string "too.\p"
	.string "He sent it specially here to\n"
	.string "CINNABAR to pick me up.$"

Kanto_CinnabarIsland_Text_84310E5::
	.string "Hey, wasn’t that a long cruise?\p"
	.string "My buddy CELIO seemed to enjoy\n"
	.string "your company.\p"
	.string "I’m sure he’d welcome you if you\n"
	.string "were to visit him again.\p"
	.string "If you have a TRI-PASS, you can\n"
	.string "always take a ferry there from\l"
	.string "VERMILION PORT.\p"
	.string "All right, thanks for your company!$"
