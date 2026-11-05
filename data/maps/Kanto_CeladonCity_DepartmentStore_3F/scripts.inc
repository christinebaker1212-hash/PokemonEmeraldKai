Kanto_CeladonCity_DepartmentStore_3F_MapScripts::
	.byte 0

@ Kanto_CeladonCity_DepartmentStore_3F object 1 (local id 1, gfx 0x3EFA) at 10,9
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BBC1::
	@ Moemon Kai: FireRed's COUNTER tutor (pokefirered data/scripts/move_tutors.inc EventScript_CounterTutor), on Kai's
	@ move tutors (Quetzal's battle-supplies counter is gone)
	lock
	faceplayer
	goto_if_set 0x221C, Kanto_CeladonCity_DepartmentStore_3F_EventScript_CounterTaught
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_CounterTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_CeladonCity_DepartmentStore_3F_EventScript_CounterDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_CeladonCity_DepartmentStore_3F_EventScript_CounterDeclined
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_CounterWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_COUNTER
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_DepartmentStore_3F_EventScript_CounterDeclined
	setflag 0x221C
	goto Kanto_CeladonCity_DepartmentStore_3F_EventScript_CounterTaught
	end

Kanto_CeladonCity_DepartmentStore_3F_EventScript_CounterDeclined::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_CounterDeclined, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_3F_EventScript_CounterTaught::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_CounterTaught, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_DepartmentStore_3F object 2 (local id 2, gfx 0x3F08) at 11,6
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC5C::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_844582E, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F object 4 (local id 4, gfx 0x3F08) at 1,9
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC65::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84458AE, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F object 3 (local id 3, gfx 0x3F08) at 4,9
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC6E::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84458F5, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F object 5 (local id 5, gfx 0x3F18) at 9,14
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC77::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445968, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 1,8
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 1,11
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 4,8
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 4,11
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC80::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459A8, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 2,8
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC89::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459BA, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 5,8
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC92::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459DC, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 2,11
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BC9B::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_84459FD, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 5,11
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BCA4::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445A1D, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 8,1
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BCAD::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445A3B, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 2,4
@ Kanto_CeladonCity_DepartmentStore_3F sign / bg event at 0,4
Kanto_CeladonCity_DepartmentStore_3F_EventScript_841BCB6::
	msgbox Kanto_CeladonCity_DepartmentStore_3F_Text_8445ABF, MSGBOX_SIGN
	end

Kanto_CeladonCity_DepartmentStore_3F_Text_844582E::
	.string "Captured MOéMON are registered\n"
	.string "with an ID No. and the OT.\p"
	.string "OT stands for “Original TRAINER” -\n"
	.string "the TRAINER that caught it first.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84458AE::
	.string "All right!\p"
	.string "My buddy’s going to trade me his\n"
	.string "KANGASKHAN for my HAUNTER!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84458F5::
	.string "Come on, HAUNTER!\p"
	.string "I love HAUNTER!\n"
	.string "I collect them!\p"
	.string "Huh?\p"
	.string "HAUNTER turned into a different\n"
	.string "MOéMON while being traded!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445968::
	.string "You can identify MOéMON you get\n"
	.string "in trades by their ID Numbers.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459A8::
	.string "It’s a Super NES.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459BA::
	.string "An RPG!\n"
	.string "There’s no time for that!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459DC::
	.string "A sports game!\n"
	.string "Dad’ll like that!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_84459FD::
	.string "A puzzle game!\n"
	.string "Looks addictive!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445A1D::
	.string "A fighting game!\n"
	.string "Looks tough!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445A3B::
	.string "3F: TV GAME SHOP$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445A81::
	.string "Get your team ready for battle.\n"
	.string "Which section would you like?$"

Kanto_CeladonCity_DepartmentStore_3F_Text_8445ABF::
	.string "Red and Green!\n"
	.string "Both are MOéMON!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_CounterTeach::
	.string "Oh, hi!\n"
	.string "I finally finished MOéMON.\p"
	.string "Not done yet? How about I teach\n"
	.string "you a good move?\p"
	.string "The move I have in mind is\n"
	.string "COUNTER.\p"
	.string "Not like the one I’m leaning on,\n"
	.string "mind you!$"

Kanto_CeladonCity_DepartmentStore_3F_Text_CounterWhichMon::
	.string "Which MOéMON should I teach\n"
	.string "COUNTER to?$"

Kanto_CeladonCity_DepartmentStore_3F_Text_CounterDeclined::
	.string "You’re not interested? Come see\n"
	.string "me if you change your mind.$"

Kanto_CeladonCity_DepartmentStore_3F_Text_CounterTaught::
	.string "Are you using that COUNTER move\n"
	.string "I taught your MOéMON?$"
