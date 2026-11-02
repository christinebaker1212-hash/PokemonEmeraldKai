Kanto_Route4_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route4 object 1 (local id 1, gfx 0x3F57) at 9,8
Kanto_Route4_EventScript_8417C3E::
	msgbox Kanto_Route4_Text_8433B9E, MSGBOX_NPC
	end

@ Kanto_Route4 sign / bg event at 18,7
Kanto_Route4_EventScript_8417C47::
	msgbox Kanto_Route4_Text_8433C62, MSGBOX_SIGN
	end

@ Kanto_Route4 sign / bg event at 34,7
Kanto_Route4_EventScript_8417C50::
	msgbox Kanto_Route4_Text_8433C7B, MSGBOX_SIGN
	end

@ Kanto_Route4 object 2 (local id 2, gfx 0x3F16) at 75,3
Kanto_Route4_EventScript_845998A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8311, 0, Kanto_Route4_Text_8433BCD, Kanto_Route4_Text_8433BFF
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route4_EventScript_84599B1
	msgbox Kanto_Route4_Text_8433C1E, MSGBOX_AUTOCLOSE
	end

Kanto_Route4_EventScript_84599B1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8311, 0, Kanto_Route4_Text_846EAB5, Kanto_Route4_Text_8433BFF
	msgbox Kanto_Route4_Text_8433C1E, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route4 object 4 (local id 4, gfx 0x3EEE) at 15,14
Kanto_Route4_EventScript_845D1BB::
	lock
	faceplayer
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	msgbox Kanto_Route4_Text_8433FED, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route4 object 3 (local id 3, gfx 0x3F0E) at 67,5
Kanto_Route4_EventScript_846BAED::
	setorcopyvar VAR_0x8000, ITEM_LIFE_ORB
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_Route4 object 5 (local id 5, gfx 0x3EEB) at 47,3
Kanto_Route4_EventScript_8471C78::
	@ Moemon Kai: FireRed's MEGAPUNCH tutor (pokefirered data/scripts/move_tutors.inc), on Kai's move tutors
	lock
	faceplayer
	goto_if_set 0x21DF, Kanto_Route4_EventScript_MegaPunchTaught
	msgbox Kanto_Route4_Text_MegaPunchTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_Route4_EventScript_MegaPunchDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_Route4_EventScript_MegaPunchDeclined
	msgbox Kanto_Route4_Text_MegaPunchWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_MEGA_PUNCH
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_EventScript_MegaPunchDeclined
	setflag 0x21DF
	goto Kanto_Route4_EventScript_MegaPunchTaught
	end

Kanto_Route4_EventScript_MegaPunchDeclined::
	msgbox Kanto_Route4_Text_MegaPunchDeclined, MSGBOX_DEFAULT
	release
	end

Kanto_Route4_EventScript_MegaPunchTaught::
	msgbox Kanto_Route4_Text_MegaPunchTaught, MSGBOX_DEFAULT
	release
	end

Kanto_Route4_Text_MegaPunchTeach:
	.string "A punch of roaring ferocity!\p"
	.string "Packed with destructive power!\p"
	.string "When the chips are down,\n"
	.string "MEGA PUNCH is the ultimate attack!\l"
	.string "You agree, yes?\p"
	.string "Now!\n"
	.string "Let me teach it to your MOéMON!$"

Kanto_Route4_Text_MegaPunchDeclined:
	.string "You'll be back when you understand\n"
	.string "the worth of MEGA PUNCH.$"

Kanto_Route4_Text_MegaPunchWhichMon:
	.string "Fine!\n"
	.string "Which MOéMON will learn it?$"

Kanto_Route4_Text_MegaPunchTaught:
	.string "Now, we are comrades in the way of\n"
	.string "punching!\p"
	.string "You should go before you're seen\n"
	.string "by the misguided fool who trains\l"
	.string "only his silly kicking over there.$"

@ Kanto_Route4 object 6 (local id 6, gfx 0x3EEB) at 50,3
Kanto_Route4_EventScript_8471C81::
	@ Moemon Kai: FireRed's MEGAKICK tutor (pokefirered data/scripts/move_tutors.inc), on Kai's move tutors
	lock
	faceplayer
	goto_if_set 0x21DC, Kanto_Route4_EventScript_MegaKickTaught
	msgbox Kanto_Route4_Text_MegaKickTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_Route4_EventScript_MegaKickDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_Route4_EventScript_MegaKickDeclined
	msgbox Kanto_Route4_Text_MegaKickWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_MEGA_KICK
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_EventScript_MegaKickDeclined
	setflag 0x21DC
	goto Kanto_Route4_EventScript_MegaKickTaught
	end

Kanto_Route4_EventScript_MegaKickDeclined::
	msgbox Kanto_Route4_Text_MegaKickDeclined, MSGBOX_DEFAULT
	release
	end

Kanto_Route4_EventScript_MegaKickTaught::
	msgbox Kanto_Route4_Text_MegaKickTaught, MSGBOX_DEFAULT
	release
	end

Kanto_Route4_Text_MegaKickTeach:
	.string "A kick of brutal ferocity!\p"
	.string "Packed with destructive power!\p"
	.string "When you get right down to it,\n"
	.string "MEGA KICK is the ultimate attack!\l"
	.string "Don't you agree?\p"
	.string "Okay!\n"
	.string "I'll teach it to your MOéMON!$"

Kanto_Route4_Text_MegaKickDeclined:
	.string "You'll come crawling back when you\n"
	.string "realize the value of MEGA KICK.$"

Kanto_Route4_Text_MegaKickWhichMon:
	.string "All right!\n"
	.string "Which MOéMON wants to learn it?$"

Kanto_Route4_Text_MegaKickTaught:
	.string "Now, we are soul mates in the way\n"
	.string "of kicking!\p"
	.string "You should run before you're seen\n"
	.string "by the deluded nitwit who trains\l"
	.string "only simple punching over there.$"

Kanto_Route4_Text_8433B9E::
	.string "Ouch! I tripped over a rocky\n"
	.string "MOéMON, GEODUDE!$"

Kanto_Route4_Text_8433BCD::
	.string "I came to MT. MOON in search of\n"
	.string "mushroom MOéMON.$"

Kanto_Route4_Text_8433BFF::
	.string "After all I did to catch them!$"

Kanto_Route4_Text_8433C1E::
	.string "There might not be any more\n"
	.string "mushrooms here.\p"
	.string "I think I got them all.$"

Kanto_Route4_Text_8433C62::
	.string "MT. MOON\n"
	.string "Tunnel Entrance$"

Kanto_Route4_Text_8433C7B::
	.string "ROUTE 4\n"
	.string "MT. MOON - CERULEAN CITY$"

Kanto_Route4_Text_8433FED::
	.string "Oh, wow, that’s the BOULDERBADGE!\n"
	.string "You got it from BROCK, didn’t you?\p"
	.string "BROCK is cool. He’s not just tough.\n"
	.string "People like and respect him.\p"
	.string "I want to become a GYM LEADER\n"
	.string "like him.$"

Kanto_Route4_Text_846EAB5::
	.string "I always catch mushroom MOéMON\n"
	.string "on MT. MOON.$"
