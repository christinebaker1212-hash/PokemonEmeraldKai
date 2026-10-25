Kanto_FiveIsland_RocketWarehouse_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_FiveIsland_RocketWarehouse_EventScript_8413CEB
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FiveIsland_RocketWarehouse_EventScript_8413CE2
	.byte 0

Kanto_FiveIsland_RocketWarehouse_EventScript_8413CE2::
	setflag FLAG_QZ_0x223B
	call Quetzal_EventScript_840FBB5
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413CEB::
	setvar VAR_TEMP_F, 0
	checktrainerflag 0x221F
	call_if_eq Kanto_FiveIsland_RocketWarehouse_EventScript_8413CFA
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413CFA::
	setmetatile 0x1A, 0x14, 0x2A0, 0
	setmetatile 0x1B, 0x14, 0x2A0, 0
	setvar VAR_TEMP_F, 1
	return

@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 11,2
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 12,2
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 10,2
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 9,2
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 8,2
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 7,2
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 11,5
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 10,5
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 11,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 10,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 8,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 7,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 8,5
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 7,5
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 5,3
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 4,3
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 5,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 4,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 3,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 2,4
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 2,5
@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 3,5
Kanto_FiveIsland_RocketWarehouse_EventScript_8413D12::
	lockall
	goto_if_set FLAG_QZ_0x2124, Kanto_FiveIsland_RocketWarehouse_EventScript_8413D26
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BBEC, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413D26::
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BC1B, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_FiveIsland_RocketWarehouse sign / bg event at 26,11
Kanto_FiveIsland_RocketWarehouse_EventScript_8413D30::
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842C21E, MSGBOX_SIGN
	end

@ Kanto_FiveIsland_RocketWarehouse trigger at 22,11 (VAR_QZ_0x5123 == 0)
@ Kanto_FiveIsland_RocketWarehouse trigger at 23,11 (VAR_QZ_0x5123 == 0)
@ Kanto_FiveIsland_RocketWarehouse trigger at 24,11 (VAR_QZ_0x5123 == 0)
Kanto_FiveIsland_RocketWarehouse_EventScript_8413D39::
	lockall
	applymovement 4, Quetzal_Movement_8457AAA
	waitmovement 0
	setvar VAR_QZ_0x5123, 1
	releaseall
	end

@ Kanto_FiveIsland_RocketWarehouse object 6 (local id 6, gfx 0x3F43) at 27,4
Kanto_FiveIsland_RocketWarehouse_EventScript_8413D4B::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8737, 0, Kanto_FiveIsland_RocketWarehouse_Text_842C2B8, Kanto_FiveIsland_RocketWarehouse_Text_842C35E, Kanto_FiveIsland_RocketWarehouse_EventScript_8413D8E

Kanto_FiveIsland_RocketWarehouse_EventScript_8413D5D::
	goto_if_unset FLAG_QZ_0x21EB, Kanto_FiveIsland_RocketWarehouse_EventScript_8413D8E
	goto_if_set FLAG_QZ_0x220A, Kanto_FiveIsland_RocketWarehouse_EventScript_8413D78
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842C3AC, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413D78::
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842C3E0, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413D8E::
	goto_if_set FLAG_QZ_0x21EB, Kanto_FiveIsland_RocketWarehouse_EventScript_8413D5D
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842C375, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x2F5
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_84570B3
	setflag FLAG_QZ_0x21EB
	release
	end

@ Kanto_FiveIsland_RocketWarehouse object 1 (local id 1, gfx 0x3F3D) at 15,11
Kanto_FiveIsland_RocketWarehouse_EventScript_8413DBB::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8733, 0, Kanto_FiveIsland_RocketWarehouse_Text_842BC56, Kanto_FiveIsland_RocketWarehouse_Text_842BC8E
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BCAF, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_RocketWarehouse object 2 (local id 2, gfx 0x3F3D) at 17,15
Kanto_FiveIsland_RocketWarehouse_EventScript_8413DD2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8734, 0, Kanto_FiveIsland_RocketWarehouse_Text_842BD80, Kanto_FiveIsland_RocketWarehouse_Text_842BDE2
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BDF8, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_RocketWarehouse object 4 (local id 4, gfx 0x3F3D) at 25,11
Kanto_FiveIsland_RocketWarehouse_EventScript_8413DE9::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8736, 0, Kanto_FiveIsland_RocketWarehouse_Text_842BFE3, Kanto_FiveIsland_RocketWarehouse_Text_842C104, Kanto_FiveIsland_RocketWarehouse_EventScript_8413E04
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842C16D, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413E04::
	getplayerxy VAR_0x8004, VAR_0x8005
	call_if_le VAR_0x8004, 24, Kanto_FiveIsland_RocketWarehouse_EventScript_8413E3A
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842C16D, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	removeobject 1
	removeobject 2
	removeobject 4
	removeobject 3
	setflag FLAG_QZ_0x218A
	fadescreen 0
	setvar VAR_QZ_0x5123, 1
	setflag FLAG_QZ_0x2124
	release
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413E3A::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	return

@ Kanto_FiveIsland_RocketWarehouse object 5 (local id 5, gfx 0x3F3C) at 6,6
Kanto_FiveIsland_RocketWarehouse_EventScript_8413E45::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8708, 0, Kanto_FiveIsland_RocketWarehouse_Text_842BCDF, Kanto_FiveIsland_RocketWarehouse_Text_842BD38
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BD4B, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_RocketWarehouse object 3 (local id 3, gfx 0x3F3C) at 27,16
Kanto_FiveIsland_RocketWarehouse_EventScript_8413E5C::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8735, 0, Kanto_FiveIsland_RocketWarehouse_Text_842BE36, Kanto_FiveIsland_RocketWarehouse_Text_842BEA5, Kanto_FiveIsland_RocketWarehouse_EventScript_8413E77

Kanto_FiveIsland_RocketWarehouse_EventScript_8413E6E::
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BF03, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413E77::
	goto_if_eq VAR_TEMP_F, 1, Kanto_FiveIsland_RocketWarehouse_EventScript_8413E6E
	getplayerxy VAR_0x8004, VAR_0x8005
	specialvar VAR_RESULT, GetPlayerFacingDirection
	call_if_eq VAR_RESULT, 2, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F0E
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BEB7, MSGBOX_DEFAULT
	closemessage
	call_if_le VAR_0x8004, 24, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F24
	call_if_eq VAR_0x8004, 25, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F2F
	call_if_eq VAR_0x8004, 26, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F3A
	call_if_eq VAR_0x8004, 27, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F3A
	playse SE_PIN
	call Kanto_FiveIsland_RocketWarehouse_EventScript_8413CFA
	special DrawWholeMapView
	waitse
	call_if_le VAR_0x8004, 24, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F45
	call_if_eq VAR_0x8004, 25, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F45
	call_if_eq VAR_0x8004, 26, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F45
	call_if_eq VAR_0x8004, 27, Kanto_FiveIsland_RocketWarehouse_EventScript_8413F50
	msgbox Kanto_FiveIsland_RocketWarehouse_Text_842BF03, MSGBOX_DEFAULT
	release
	end

Kanto_FiveIsland_RocketWarehouse_EventScript_8413F0E::
	goto_if_eq VAR_0x8004, 27, Quetzal_EventScript_8457C6C
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	return

Kanto_FiveIsland_RocketWarehouse_EventScript_8413F24::
	applymovement 3, Kanto_FiveIsland_RocketWarehouse_Movement_8413F5B
	waitmovement 0
	return

Kanto_FiveIsland_RocketWarehouse_EventScript_8413F2F::
	applymovement 3, Kanto_FiveIsland_RocketWarehouse_Movement_8413F5F
	waitmovement 0
	return

Kanto_FiveIsland_RocketWarehouse_EventScript_8413F3A::
	applymovement 3, Quetzal_Movement_8457AAC
	waitmovement 0
	return

Kanto_FiveIsland_RocketWarehouse_EventScript_8413F45::
	applymovement 3, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_FiveIsland_RocketWarehouse_EventScript_8413F50::
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	return

@ Kanto_FiveIsland_RocketWarehouse object 7 (local id 7, gfx 0x3F0E) at 8,25
Kanto_FiveIsland_RocketWarehouse_EventScript_846C26D::
	setorcopyvar VAR_0x8000, 0x8D
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_FiveIsland_RocketWarehouse object 8 (local id 8, gfx 0x3F0E) at 17,3
Kanto_FiveIsland_RocketWarehouse_EventScript_846C27A::
	setorcopyvar VAR_0x8000, 0x1BE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_FiveIsland_RocketWarehouse object 9 (local id 9, gfx 0x3F0E) at 1,16
Kanto_FiveIsland_RocketWarehouse_EventScript_846C287::
	setorcopyvar VAR_0x8000, 0x8C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_FiveIsland_RocketWarehouse object 10 (local id 10, gfx 0x3F0E) at 4,5
Kanto_FiveIsland_RocketWarehouse_EventScript_846C294::
	setorcopyvar VAR_0x8000, 0xE4
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_FiveIsland_RocketWarehouse_Movement_8413F5B:
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

Kanto_FiveIsland_RocketWarehouse_Movement_8413F5F:
	walk_right
	walk_in_place_faster_up
	step_end

Kanto_FiveIsland_RocketWarehouse_Text_842BBEC::
	.string "There are many POKéMON locked\n"
	.string "away in the pen…$"

Kanto_FiveIsland_RocketWarehouse_Text_842BC1B::
	.string "The pen door is unlocked.\n"
	.string "The POKéMON appear to have fled.$"

Kanto_FiveIsland_RocketWarehouse_Text_842BC56::
	.string "Did you come here knowing it’s\n"
	.string "TEAM ROCKET’s WAREHOUSE?$"

Kanto_FiveIsland_RocketWarehouse_Text_842BC8E::
	.string "What do you think you’re doing?!$"

Kanto_FiveIsland_RocketWarehouse_Text_842BCAF::
	.string "TEAM ROCKET broke up?\n"
	.string "What planet are you from?$"

Kanto_FiveIsland_RocketWarehouse_Text_842BCDF::
	.string "Oh, now your POKéMON look strong.\n"
	.string "You’re willing to sell them?\p"
	.string "Oh?\n"
	.string "You wanted to battle?$"

Kanto_FiveIsland_RocketWarehouse_Text_842BD38::
	.string "Oh, but…\n"
	.string "Too much!$"

Kanto_FiveIsland_RocketWarehouse_Text_842BD4B::
	.string "We can give you a great price.\n"
	.string "Sell us your POKéMON!$"

Kanto_FiveIsland_RocketWarehouse_Text_842BD80::
	.string "I got word about you from the\n"
	.string "others.\p"
	.string "But there’s no way a kid like you’d\n"
	.string "know our BOSS GIOVANNI!$"

Kanto_FiveIsland_RocketWarehouse_Text_842BDE2::
	.string "Don’t…you…dare…laugh…$"

Kanto_FiveIsland_RocketWarehouse_Text_842BDF8::
	.string "Don’t mess in the doings of adults,\n"
	.string "you jumped-up pip-squeak!$"

Kanto_FiveIsland_RocketWarehouse_Text_842BE36::
	.string "I don’t know or care if what I’m\n"
	.string "doing is right or wrong…\p"
	.string "I just put my faith in GIOVANNI\n"
	.string "and do as I am told.$"

Kanto_FiveIsland_RocketWarehouse_Text_842BEA5::
	.string "I…\n"
	.string "I’m shattered…$"

Kanto_FiveIsland_RocketWarehouse_Text_842BEB7::
	.string "You’re doing things according to\n"
	.string "your beliefs and morals.\p"
	.string "I understand now…$"

Kanto_FiveIsland_RocketWarehouse_Text_842BF03::
	.string "I’ve made it so you can come back\n"
	.string "in through there.\p"
	.string "Why don’t you go get rested and\n"
	.string "refreshed for what lies ahead?\p"
	.string "The ADMIN after me outranks me\n"
	.string "thoroughly as a TRAINER.\p"
	.string "You’d better steel yourself for one\n"
	.string "harsh challenge.$"

Kanto_FiveIsland_RocketWarehouse_Text_842BFE3::
	.string "That’s quite enough of you \n"
	.string "playing the hero, kid.\p"
	.string "Spreading lies about how TEAM\n"
	.string "ROCKET has disbanded…\p"
	.string "It’s such an obvious attempt to\n"
	.string "cause confusion in our ranks.\p"
	.string "Fortunately, we’re not so ignorant\n"
	.string "to fall for the lies of a child!\p"
	.string "And now, I’ll show you how scary\n"
	.string "an angry adult can be!$"

Kanto_FiveIsland_RocketWarehouse_Text_842C104::
	.string "Urgh…\n"
	.string "You were too strong…\p"
	.string "…\n"
	.string "Th-that’s GIOVANNI’s BADGE!\p"
	.string "So it’s true?\n"
	.string "TEAM ROCKET really has disbanded?$"

Kanto_FiveIsland_RocketWarehouse_Text_842C16D::
	.string "We will abandon this WAREHOUSE…\p"
	.string "But don’t think this is over.\n"
	.string "I won’t let this be the end.\p"
	.string "I will find GIOVANNI.\n"
	.string "And I will resurrect TEAM ROCKET!\l"
	.string "I will…\p"
	.string "Until then, farewell!$"

Kanto_FiveIsland_RocketWarehouse_Text_842C21E::
	.string "A report with graphs and text is on\n"
	.string "the PC monitor.\p"
	.string "“The Influence of Magnetic Waves\n"
	.string "on POKéMON Evolution”…\p"
	.string "… … … … … …\p"
	.string "It’s too difficult to understand…$"

Kanto_FiveIsland_RocketWarehouse_Text_842C2B8::
	.string "I almost managed to sell the\n"
	.string "SAPPHIRE for huge money…\p"
	.string "Then, you just had to come along\n"
	.string "and blow the whole deal apart!\p"
	.string "Fufu… Fufufufu…\n"
	.string "There’s no forgiveness for you!$"

Kanto_FiveIsland_RocketWarehouse_Text_842C35E::
	.string "Gah! Darn!\n"
	.string "Darn! Darn!$"

Kanto_FiveIsland_RocketWarehouse_Text_842C375::
	.string "If I can’t sell it, it’s worthless!\n"
	.string "Go ahead, take it!$"

Kanto_FiveIsland_RocketWarehouse_Text_842C3AC::
	.string "What do you want now?\n"
	.string "Go on, get lost, leave me be.$"

Kanto_FiveIsland_RocketWarehouse_Text_842C3E0::
	.string "What, you again?\p"
	.string "You… You’re not GIOVANNI’s kid,\n"
	.string "are you?\p"
	.string "No, that can’t be right!\n"
	.string "GIOVANNI’s kid has red hair.\p"
	.string "You don’t have any reasons for\n"
	.string "bugging me. Leave me alone!$"
