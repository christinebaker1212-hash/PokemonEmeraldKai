Johto_EcruteakPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_EcruteakPokecenter1F_EventScript_83CEC9C
	.byte 0

Johto_EcruteakPokecenter1F_EventScript_83CEC9C::
	setrespawn 0x1C
	end

@ Johto_EcruteakPokecenter1F object 6 (local id 6, gfx 0x3E86) at 11,2
Johto_EcruteakPokecenter1F_EventScript_83CECA0::
	lock
	delay 0x1E
	playse SE_PC_OFF
	delay 0x37
	faceplayer
	msgbox Johto_EcruteakPokecenter1F_Text_83CED42, MSGBOX_DEFAULT
	msgbox Johto_EcruteakPokecenter1F_Text_83CED8E, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 2, Johto_EcruteakPokecenter1F_EventScript_83CECF8
	call_if_eq VAR_FACING, 3, Johto_EcruteakPokecenter1F_EventScript_83CECED
	playse SE_EXIT
	setflag FLAG_QZ_0x10AB
	removeobject 6
	setflag FLAG_QZ_0x11CB
	clearflag FLAG_QZ_0x103A
	release
	end

Johto_EcruteakPokecenter1F_EventScript_83CECED::
	applymovement 6, Johto_EcruteakPokecenter1F_Movement_83CED2A
	waitmovement 0
	return

Johto_EcruteakPokecenter1F_EventScript_83CECF8::
	applymovement 6, Johto_EcruteakPokecenter1F_Movement_83CED35
	waitmovement 0
	return

@ Johto_EcruteakPokecenter1F object 4 (local id 4, gfx 0x37) at 11,5
Johto_EcruteakPokecenter1F_EventScript_83CED03::
	lock
	faceplayer
	msgbox Johto_EcruteakPokecenter1F_Text_83CEECF, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakPokecenter1F object 5 (local id 5, gfx 0x14) at 2,5
Johto_EcruteakPokecenter1F_EventScript_83CED10::
	lock
	faceplayer
	msgbox Johto_EcruteakPokecenter1F_Text_83CEFBD, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakPokecenter1F object 3 (local id 3, gfx 0x3EB7) at 14,6
Johto_EcruteakPokecenter1F_EventScript_83CED1D::
	lock
	faceplayer
	msgbox Johto_EcruteakPokecenter1F_Text_83CF005, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakPokecenter1F_Movement_83CED2A:
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_EcruteakPokecenter1F_Movement_83CED35:
	walk_right
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_EcruteakPokecenter1F_Text_83CED42::
	.string "Hi, I’m BILL. And who are you?\p"
	.string "Hmm, {PLAYER}, huh? You’ve come at\n"
	.string "the right time.$"

Johto_EcruteakPokecenter1F_Text_83CED8E::
	.string "I just finished adjustments on my\n"
	.string "TIME CAPSULE.\p"
	.string "You know that MOéMON can be\n"
	.string "traded, right?\p"
	.string "My TIME CAPSULE was developed to\n"
	.string "enable trades with the past.\p"
	.string "But you can’t send anything that\n"
	.string "didn’t exist in the past.\p"
	.string "If you did, the PC in the past\n"
	.string "would have a breakdown.\p"
	.string "So you have to remove anything\n"
	.string "that wasn’t around in the past.\p"
	.string "Put simply, no sending new moves\n"
	.string "or new MOéMON in the TIME CAPSULE.\p"
	.string "Don’t you worry. I’m done with the\n"
	.string "adjustments.\p"
	.string "Tomorrow, TIME CAPSULES will be\n"
	.string "running at all MOéMON CENTERS.\p"
	.string "I have to hurry on back to\n"
	.string "GOLDENROD and see my folks.\p"
	.string "Buh-bye!$"

Johto_EcruteakPokecenter1F_Text_83CEECF::
	.string "The way the KIMONO GIRLS dance is\n"
	.string "marvelous.\p"
	.string "Just like the way they use their\n"
	.string "MOéMON.$"

Johto_EcruteakPokecenter1F_Text_83CEFBD::
	.string "MORTY, the GYM LEADER, is soooo\n"
	.string "cool.\p"
	.string "His MOéMON are really tough too.$"

Johto_EcruteakPokecenter1F_Text_83CF005::
	.string "LAKE OF RAGE…\p"
	.string "The appearance of a GYARADOS\n"
	.string "swarm…\p"
	.string "I smell a conspiracy. I know it!$"
