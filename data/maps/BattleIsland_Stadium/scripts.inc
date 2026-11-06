BattleIsland_Stadium_MapScripts::
	.byte 0

@ The NURSE: heals, then the STADIUM's news (the player's results, the cups' champions: mp_stadium.c)
BattleIsland_Stadium_EventScript_Nurse::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	special MP_Stadium_BufferNews
	msgbox gStringVar4, MSGBOX_DEFAULT
	release
	end

@ The MART's clerk: what the STADIUM is and how it works
BattleIsland_Stadium_EventScript_Clerk::
	lock
	faceplayer
	msgbox BattleIsland_Stadium_Text_Clerk1, MSGBOX_DEFAULT
	msgbox BattleIsland_Stadium_Text_Clerk2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, BattleIsland_Stadium_EventScript_ClerkDone
	msgbox BattleIsland_Stadium_Text_Rules1, MSGBOX_DEFAULT
	msgbox BattleIsland_Stadium_Text_Rules2, MSGBOX_DEFAULT
	msgbox BattleIsland_Stadium_Text_Rules3, MSGBOX_DEFAULT
BattleIsland_Stadium_EventScript_ClerkDone::
	msgbox BattleIsland_Stadium_Text_ClerkBye, MSGBOX_DEFAULT
	release
	end

@ The doors: into POKéMON STADIUM (its screens: mp_stadium_ui.c)
BattleIsland_Stadium_EventScript_Attendant::
	lock
	faceplayer
	msgbox BattleIsland_Stadium_Text_Attendant, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, BattleIsland_Stadium_EventScript_AttendantNo
	closemessage
	special MP_Stadium_Open
	waitstate
	release
	end

BattleIsland_Stadium_EventScript_AttendantNo::
	msgbox BattleIsland_Stadium_Text_AttendantNo, MSGBOX_DEFAULT
	release
	end

BattleIsland_Stadium_Text_Clerk1:
	.string "Welcome to POKéMON STADIUM!\p"
	.string "Trainers from every region come here\n"
	.string "to battle in its cups.$"

BattleIsland_Stadium_Text_Clerk2:
	.string "Would you like to hear how the\n"
	.string "STADIUM works?$"

BattleIsland_Stadium_Text_Rules1:
	.string "First, register a team of six. Take\n"
	.string "them from your PC, the SHARED PC or\l"
	.string "the GLOBAL PC, or borrow RENTALS.\p"
	.string "Before each battle you see your\n"
	.string "opponent's six, then pick three.$"

BattleIsland_Stadium_Text_Rules2:
	.string "Each cup has its own rules. The PIKA,\n"
	.string "PETIT and POKé CUPS limit levels, and\l"
	.string "the PETIT CUP height and weight too.\p"
	.string "The STADIUM's cups use the first 151\n"
	.string "POKéMON and their own moves only.$"

BattleIsland_Stadium_Text_Rules3:
	.string "Win eight battles in a row to take a\n"
	.string "cup. The GYM LEADER CASTLE has every\l"
	.string "LEADER, the ELITE FOUR and a RIVAL.\p"
	.string "The CHALLENGE CUP lends you six\n"
	.string "random POKéMON. Good luck!$"

BattleIsland_Stadium_Text_ClerkBye:
	.string "Teams you register stay here, and\n"
	.string "your friends on the server can see\l"
	.string "them too. Enjoy the STADIUM!$"

BattleIsland_Stadium_Text_Attendant:
	.string "This way to POKéMON STADIUM.\n"
	.string "Will you go in?$"

BattleIsland_Stadium_Text_AttendantNo:
	.string "Come back when you're ready.$"
