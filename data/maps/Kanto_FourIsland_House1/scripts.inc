Kanto_FourIsland_House1_MapScripts::
	.byte 0

@ Kanto_FourIsland_House1 object 2 (local id 2, gfx 0x3F04) at 9,3
Kanto_FourIsland_House1_EventScript_8421958::
	msgbox Kanto_FourIsland_House1_Text_8453D9A, MSGBOX_NPC
	end

@ Kanto_FourIsland_House1 object 1 (local id 1, gfx 0x3F04) at 8,3
Kanto_FourIsland_House1_EventScript_8471CD2::
	@ Moemon Kai: FireRed's BODY SLAM tutor (tools/quetzal_port/fix_move_tutors.py), on Kai's move tutors
	lock
	faceplayer
	goto_if_set 0x2222, Kanto_FourIsland_House1_EventScript_8471CD2_Taught
	msgbox Kanto_FourIsland_House1_EventScript_8471CD2_Teach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_FourIsland_House1_EventScript_8471CD2_Declined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_FourIsland_House1_EventScript_8471CD2_Declined
	msgbox Kanto_FourIsland_House1_EventScript_8471CD2_WhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_BODY_SLAM
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_House1_EventScript_8471CD2_Declined
	setflag 0x2222
	goto Kanto_FourIsland_House1_EventScript_8471CD2_Taught
	end

Kanto_FourIsland_House1_EventScript_8471CD2_Declined:
	msgbox Kanto_FourIsland_House1_EventScript_8471CD2_DeclinedText, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_House1_EventScript_8471CD2_Taught:
	msgbox Kanto_FourIsland_House1_EventScript_8471CD2_TaughtText, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_House1_EventScript_8471CD2_Teach:
	.string "Ready?\n"
	.string "Boing!\p"
	.string "We're having a BODY SLAM match\n"
	.string "to see who wimps out first.\p"
	.string "If you were to join us, you'd be\n"
	.string "squashed like a bug, though…\p"
	.string "How about I teach BODY SLAM to\n"
	.string "a MOéMON of yours instead?$"

Kanto_FourIsland_House1_EventScript_8471CD2_DeclinedText:
	.string "Come again!\n"
	.string "Boing!$"

Kanto_FourIsland_House1_EventScript_8471CD2_WhichMon:
	.string "Which MOéMON wants to learn how\n"
	.string "to BODY SLAM?$"

Kanto_FourIsland_House1_EventScript_8471CD2_TaughtText:
	.string "Boioing!$"

Kanto_FourIsland_House1_Text_8453D9A::
	.string "There! Boing!\n"
	.string "You’re awfully heavy!$"
