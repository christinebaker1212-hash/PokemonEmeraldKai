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

@ Moemon Kai: the STADIUM's fans in the lobby, each crazy about one of its cups (as the BATTLE FRONTIER's trainers
@ are about its facilities)
BattleIsland_Stadium_EventScript_PetitFan::
	msgbox BattleIsland_Stadium_Text_PetitFan, MSGBOX_NPC
	end

BattleIsland_Stadium_EventScript_PikaFan::
	msgbox BattleIsland_Stadium_Text_PikaFan, MSGBOX_NPC
	end

BattleIsland_Stadium_EventScript_PokeFan::
	msgbox BattleIsland_Stadium_Text_PokeFan, MSGBOX_NPC
	end

BattleIsland_Stadium_EventScript_PrimeFan::
	msgbox BattleIsland_Stadium_Text_PrimeFan, MSGBOX_NPC
	end

BattleIsland_Stadium_EventScript_ChallengeFan::
	msgbox BattleIsland_Stadium_Text_ChallengeFan, MSGBOX_NPC
	end

BattleIsland_Stadium_EventScript_CastleFan::
	msgbox BattleIsland_Stadium_Text_CastleFan, MSGBOX_NPC
	end

BattleIsland_Stadium_EventScript_Reporter::
	msgbox BattleIsland_Stadium_Text_Reporter, MSGBOX_NPC
	end

BattleIsland_Stadium_Text_PetitFan:
	.string "The PETIT CUP is the cutest cup in\n"
	.string "the whole world!\p"
	.string "Only little POKéMON, levels 25 to 30,\n"
	.string "no taller than 6 feet 8 inches, and\l"
	.string "no heavier than 44 pounds!\p"
	.string "Small doesn't mean weak, though.\n"
	.string "My PIKACHU is a lightning bolt!$"

BattleIsland_Stadium_Text_PikaFan:
	.string "PIKA CUP! PIKA CUP! PIKA CUP!\p"
	.string "Levels 15 to 20 only, and three of\n"
	.string "them can't add up past 50!\p"
	.string "Every turn counts when nobody hits\n"
	.string "that hard. I love it so much!$"

BattleIsland_Stadium_Text_PokeFan:
	.string "The POKé CUP is where real TRAINERS\n"
	.string "prove themselves!\p"
	.string "Levels 50 to 55, and your three\n"
	.string "can't add up past 155.\p"
	.string "I've cleared the POKé BALL and GREAT\n"
	.string "BALL classes. The MASTER BALL class\l"
	.string "is next! I can feel it!$"

BattleIsland_Stadium_Text_PrimeFan:
	.string "Ahh, the PRIME CUP… No level limits\n"
	.string "at all. Bring your strongest!\p"
	.string "Every POKéMON at full strength, even\n"
	.string "MEWTWO and MEW!\p"
	.string "I've watched STADIUM battles for forty\n"
	.string "years, and the PRIME CUP still gives\l"
	.string "me goosebumps!$"

BattleIsland_Stadium_Text_ChallengeFan:
	.string "In the CHALLENGE CUP, you don't bring\n"
	.string "a team. They hand you six random\l"
	.string "POKéMON at the door!\p"
	.string "You never know what you'll get, so\n"
	.string "only true skill wins.\p"
	.string "Last time I got a MAGIKARP and won\n"
	.string "anyway! Okay, it took a while…$"

BattleIsland_Stadium_Text_CastleFan:
	.string "The GYM LEADER CASTLE! Every LEADER\n"
	.string "of KANTO, one after another!\p"
	.string "Then the ELITE FOUR, and then the\n"
	.string "RIVAL! Hoo-hah!\p"
	.string "My muscles are shaking just from\n"
	.string "thinking about it!$"

BattleIsland_Stadium_Text_Reporter:
	.string "I'm covering the STADIUM for the\n"
	.string "evening news!\p"
	.string "Cups for every level, a CASTLE full\n"
	.string "of LEADERS, and MEWTWO at the end…\p"
	.string "Win a cup with nothing but RENTALS,\n"
	.string "and you'll be my top story!$"
