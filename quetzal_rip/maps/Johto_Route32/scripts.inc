Johto_Route32_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route32_EventScript_83EE9D4
	.byte 0

Johto_Route32_EventScript_83EE9D4::
	end

@ Johto_Route32 object 12 (local id 12, gfx 0x3E95) at 20,56
Johto_Route32_EventScript_83EE9D5::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4311, 0, Johto_Route32_Text_83EF193, Johto_Route32_Text_83EF1B5
	msgbox Johto_Route32_Text_83EF1BE, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 10 (local id 10, gfx 0x3E95) at 25,57
Johto_Route32_EventScript_83EE9EC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4881, 0, Johto_Route32_Text_83EF1FF, Johto_Route32_Text_83EF24E
	msgbox Johto_Route32_Text_83EF26B, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 11 (local id 11, gfx 0x3E95) at 28,63
Johto_Route32_EventScript_83EEA03::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4764, 0, Johto_Route32_Text_83EF2A8, Johto_Route32_Text_83EF2C4
	msgbox Johto_Route32_Text_83EF2CC, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 6 (local id 6, gfx 0x23) at 22,23
Johto_Route32_EventScript_83EEA1A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4176, 0, Johto_Route32_Text_83EF30A, Johto_Route32_Text_83EF34F
	msgbox Johto_Route32_Text_83EF35E, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 13 (local id 13, gfx 0x23) at 15,63
Johto_Route32_EventScript_83EEA31::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4337, 0, Johto_Route32_Text_83EF3C8, Johto_Route32_Text_83EF40E
	msgbox Johto_Route32_Text_83EF42B, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 9 (local id 9, gfx 0x1F) at 14,45
Johto_Route32_EventScript_83EEA48::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4256, 0, Johto_Route32_Text_83EF44F, Johto_Route32_Text_83EF46D
	msgbox Johto_Route32_Text_83EF48A, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 7 (local id 7, gfx 0x20) at 20,31
Johto_Route32_EventScript_83EEA5F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4463, 0, Johto_Route32_Text_83EF4BF, Johto_Route32_Text_83EF51E
	msgbox Johto_Route32_Text_83EF540, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 16 (local id 16, gfx 0x3EAE) at 24,95
Johto_Route32_EventScript_83EEA76::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4931, 0, Johto_Route32_Text_83EF55E, Johto_Route32_Text_83EF593
	msgbox Johto_Route32_Text_83EF5B2, MSGBOX_AUTOCLOSE
	end

@ Johto_Route32 object 2 (local id 2, gfx 0x11) at 22,13
Johto_Route32_EventScript_83EEA8D::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1499, Johto_Route32_EventScript_83EEAC5
	msgbox Johto_Route32_Text_83EF5E2, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1F2
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1499
	msgbox Johto_Route32_Text_83EF63A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route32_EventScript_83EEAC5::
	msgbox Johto_Route32_Text_83EF63A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route32 trigger at 15,80 (VAR_QZ_0x5013 == 0)
Johto_Route32_EventScript_83EEAD0::
	lock
	turnobject 14, 4
	turnobject LOCALID_PLAYER, 3
	playse SE_PIN
	applymovement 14, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 14, Johto_Route32_Movement_83EEE2C
	waitmovement 0
	goto Johto_Route32_EventScript_83EEB3F

@ Johto_Route32 trigger at 16,80 (VAR_QZ_0x5013 == 0)
Johto_Route32_EventScript_83EEAF5::
	lock
	turnobject 14, 4
	turnobject LOCALID_PLAYER, 3
	playse SE_PIN
	applymovement 14, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 14, Johto_Route32_Movement_83EEE2D
	waitmovement 0
	goto Johto_Route32_EventScript_83EEB3F

@ Johto_Route32 trigger at 17,80 (VAR_QZ_0x5013 == 0)
Johto_Route32_EventScript_83EEB1A::
	lock
	turnobject 14, 4
	turnobject LOCALID_PLAYER, 3
	playse SE_PIN
	applymovement 14, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 14, Johto_Route32_Movement_83EEE2F
	waitmovement 0
	goto Johto_Route32_EventScript_83EEB3F

Johto_Route32_EventScript_83EEB3F::
	msgbox Johto_Route32_Text_83EF0CC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route32_EventScript_83EEB62
	msgbox Johto_Route32_Text_83EF16E, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5013, 1
	release
	end

Johto_Route32_EventScript_83EEB62::
	msgbox Johto_Route32_Text_83EF142, MSGBOX_DEFAULT
	closemessage
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 16, Johto_Route32_EventScript_83EEB8D
	call_if_eq VAR_TEMP_0, 17, Johto_Route32_EventScript_83EEB9C
	setvar VAR_QZ_0x5013, 1
	release
	end

Johto_Route32_EventScript_83EEB8D::
	applymovement 14, Quetzal_Movement_836083D
	waitmovement 0
	turnobject 14, 4
	return

Johto_Route32_EventScript_83EEB9C::
	applymovement 14, Quetzal_Movement_836083D
	waitmovement 0
	applymovement 14, Quetzal_Movement_836083D
	waitmovement 0
	turnobject 14, 4
	return

@ Johto_Route32 object 14 (local id 14, gfx 0x11) at 14,80
Johto_Route32_EventScript_83EEBB5::
	lock
	faceplayer
	msgbox Johto_Route32_Text_83EF0CC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route32_EventScript_83EEBD5
	msgbox Johto_Route32_Text_83EF16E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route32_EventScript_83EEBD5::
	msgbox Johto_Route32_Text_83EF142, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route32 object 17 (local id 17, gfx 0x3E85) at 27,78
Johto_Route32_EventScript_83EEBE0::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102D, Johto_Route32_EventScript_83EECA0
	msgbox Johto_Route32_Text_83EF6AB, MSGBOX_DEFAULT
	closemessage
	multichoice 0x14, 0, 0x4A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_Route32_EventScript_83EEC92
	goto_if_eq VAR_0x8000, 1, Johto_Route32_EventScript_83EEC92
	goto_if_eq VAR_0x8000, 2, Johto_Route32_EventScript_83EEC92
	goto_if_eq VAR_0x8000, 3, Johto_Route32_EventScript_83EEC92
	goto_if_eq VAR_0x8000, 4, Johto_Route32_EventScript_83EEC92
	goto_if_eq VAR_0x8000, 5, Johto_Route32_EventScript_83EEC65
	goto_if_eq VAR_0x8000, 6, Johto_Route32_EventScript_83EEC92
	goto_if_eq VAR_0x8000, 7, Johto_Route32_EventScript_83EEC9D
	goto_if_eq VAR_0x8000, 127, Johto_Route32_EventScript_83EEC9D
	closemessage
	release
	end

Johto_Route32_EventScript_83EEC65::
	msgbox Johto_Route32_Text_83EF728, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1B0
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x102D
	msgbox Johto_Route32_Text_83EF746, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route32_EventScript_83EEC92::
	msgbox Johto_Route32_Text_83EF828, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route32_EventScript_83EEC9D::
	closemessage
	release
	end

Johto_Route32_EventScript_83EECA0::
	msgbox Johto_Route32_Text_83EF7C2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route32 object 1 (local id 1, gfx 0x3E84) at 26,10
Johto_Route32_EventScript_83EECAB::
	lock
	faceplayer
	msgbox Johto_Route32_Text_83EF089, MSGBOX_DEFAULT
	closemessage
	turnobject 1, 4
	release
	end

@ Johto_Route32 trigger at 27,10 (VAR_QZ_0x5008 == 1)
@ Johto_Route32 trigger at 27,10 (VAR_QZ_0x5008 == 2)
@ Johto_Route32 trigger at 27,10 (VAR_QZ_0x5008 == 4)
Johto_Route32_EventScript_83EECBC::
	lock
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject LOCALID_PLAYER, 3
	goto Johto_Route32_EventScript_83EED10

@ Johto_Route32 trigger at 28,10 (VAR_QZ_0x5008 == 1)
@ Johto_Route32 trigger at 28,10 (VAR_QZ_0x5008 == 2)
@ Johto_Route32 trigger at 28,10 (VAR_QZ_0x5008 == 4)
Johto_Route32_EventScript_83EECD4::
	lock
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement LOCALID_PLAYER, Johto_Route32_Movement_83EEE27
	waitmovement 0
	goto Johto_Route32_EventScript_83EED10

@ Johto_Route32 trigger at 29,10 (VAR_QZ_0x5008 == 1)
@ Johto_Route32 trigger at 29,10 (VAR_QZ_0x5008 == 2)
@ Johto_Route32 trigger at 29,10 (VAR_QZ_0x5008 == 4)
Johto_Route32_EventScript_83EECF2::
	lock
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement LOCALID_PLAYER, Johto_Route32_Movement_83EEE29
	waitmovement 0
	goto Johto_Route32_EventScript_83EED10

Johto_Route32_EventScript_83EED10::
	goto_if_unset FLAG_QZ_0x1082, Johto_Route32_EventScript_83EED76
	goto_if_unset FLAG_QZ_0x1004, Johto_Route32_EventScript_83EED76
	goto_if_unset FLAG_QZ_0x11A8, Johto_Route32_EventScript_83EED93
	goto_if_set FLAG_QZ_0x1080, Johto_Route32_EventScript_83EED4F
	specialvar VAR_RESULT, QzIsNuzlockeActiveInRegion_779
	goto_if_eq VAR_RESULT, 0, Johto_Route32_EventScript_83EEDB0
	goto_if_eq VAR_QZ_0x5012, 0, Johto_Route32_EventScript_83EEDB0

Johto_Route32_EventScript_83EED4F::
	msgbox Johto_Route32_Text_83EEF97, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1AD
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, 0, Quetzal_EventScript_83602AA
	closemessage
	setvar VAR_QZ_0x5008, 5
	release
	end

Johto_Route32_EventScript_83EED76::
	msgbox Johto_Route32_Text_83EEE32, MSGBOX_DEFAULT
	msgbox Johto_Route32_Text_83EEEC4, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_Route32_Movement_83EEE25
	waitmovement 0
	release
	end

Johto_Route32_EventScript_83EED93::
	msgbox Johto_Route32_Text_83EEE32, MSGBOX_DEFAULT
	msgbox Johto_Route32_Text_83EEF1E, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_Route32_Movement_83EEE25
	waitmovement 0
	release
	end

Johto_Route32_EventScript_83EEDB0::
	msgbox Johto_Route32_Text_83EEE32, MSGBOX_DEFAULT
	msgbox Johto_Route32_Text_83EEE4D, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_Route32_Movement_83EEE25
	waitmovement 0
	release
	end

@ Johto_Route32 object 5 (local id 5, gfx 0x3A9E) at 16,59
Johto_Route32_EventScript_83EEDCD::
	setorcopyvar VAR_0x8000, 0x1C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route32 object 4 (local id 4, gfx 0x3A9E) at 10,30
Johto_Route32_EventScript_83EEDDA::
	setorcopyvar VAR_0x8000, 2
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route32 object 15 (local id 15, gfx 0x3A9E) at 8,70
Johto_Route32_EventScript_83EEDE7::
	setorcopyvar VAR_0x8000, 2
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route32 object 3 (local id 3, gfx 0x3A9E) at 18,11
Johto_Route32_EventScript_83EEDF4::
	setorcopyvar VAR_0x8000, 0x1D
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route32 sign / bg event at 22,5
Johto_Route32_EventScript_83EEE01::
	lock
	msgbox Johto_Route32_Text_83EF83E, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route32 sign / bg event at 17,1
Johto_Route32_EventScript_83EEE0D::
	lock
	msgbox Johto_Route32_Text_83EF861, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route32 sign / bg event at 20,96
Johto_Route32_EventScript_83EEE19::
	lock
	msgbox Johto_Route32_Text_83EF87D, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route32 object 19 (local id 19, gfx 0x3E90) at 8,58
Johto_Route32_EventScript_83FEB21::
	msgbox Johto_Route32_Text_83FEB33, MSGBOX_NPC
	end

Johto_Route32_Movement_83EEE25:
	walk_up
	step_end

Johto_Route32_Movement_83EEE27:
	walk_left
	step_end

Johto_Route32_Movement_83EEE29:
	walk_left
	walk_left
	step_end

Johto_Route32_Movement_83EEE2C:
	step_end

Johto_Route32_Movement_83EEE2D:
	walk_right
	step_end

Johto_Route32_Movement_83EEE2F:
	walk_right
	walk_right
	step_end

Johto_Route32_Text_83EEE32::
	.string "Wait up!\n"
	.string "What’s the hurry?$"

Johto_Route32_Text_83EEE4D::
	.string "{PLAYER}, right?\n"
	.string "Some guy wearing glasses\p"
	.string "was looking for you.\p"
	.string "See for yourself. He’s waiting for\n"
	.string "you at the POKéMON CENTER.$"

Johto_Route32_Text_83EEEC4::
	.string "Before going on, you must defeat\n"
	.string "the elder, LI.\p"
	.string "He awaits you at the top of\n"
	.string "SPROUT TOWER.$"

Johto_Route32_Text_83EEF1E::
	.string "Have you gone to the POKéMON GYM?\n"
	.string "You can test your POKéMON and\p"
	.string "yourself there.\n"
	.string "It’s a rite of passage for all\p"
	.string "trainers!$"

Johto_Route32_Text_83EEF97::
	.string "You have some good POKéMON there.\n"
	.string "It must be from the training you\p"
	.string "gave them around VIOLET CITY.\n"
	.string "The training at the GYM must have\p"
	.string "been especially helpful. As a\n"
	.string "souvenir of VIOLET CITY, take this.\p"
	.string "It increases the power of GRASS-\n"
	.string "type moves.$"

Johto_Route32_Text_83EF089::
	.string "Your experiences in VIOLET CITY\n"
	.string "should be useful for your journey.$"

Johto_Route32_Text_83EF0CC::
	.string "How would you like to have this\n"
	.string "tasty, nutritious SLOWPOKETAIL?\p"
	.string "For you right now, just\n"
	.string "¥1,000,000!\p"
	.string "You’ll want this!$"

Johto_Route32_Text_83EF142::
	.string "Tch! I thought kids\n"
	.string "these days were loaded…$"

Johto_Route32_Text_83EF16E::
	.string "You don’t want it?\n"
	.string "Then scram. Shoo!$"

Johto_Route32_Text_83EF193::
	.string "Whoa!\p"
	.string "You made me lose that fish!$"

Johto_Route32_Text_83EF1B5::
	.string "Sploosh!$"

Johto_Route32_Text_83EF1BE::
	.string "Calm, collected… The essence of\n"
	.string "fishing and POKéMON is the same.$"

Johto_Route32_Text_83EF1FF::
	.string "I’m really good at both\n"
	.string "fishing and POKéMON.\p"
	.string "I’m not about to lose to any kid!$"

Johto_Route32_Text_83EF24E::
	.string "Tch! I tried to rush\n"
	.string "things…$"

Johto_Route32_Text_83EF26B::
	.string "Fishing is a lifelong\n"
	.string "passion.\p"
	.string "POKéMON are lifelong friends!$"

Johto_Route32_Text_83EF2A8::
	.string "My POKéMON? Freshly caught!$"

Johto_Route32_Text_83EF2C4::
	.string "SPLASH?$"

Johto_Route32_Text_83EF2CC::
	.string "Freshly caught POKéMON are\n"
	.string "no match for properly raised ones.$"

Johto_Route32_Text_83EF30A::
	.string "I haven’t seen you around before.\n"
	.string "So you think you are pretty tough?$"

Johto_Route32_Text_83EF34F::
	.string "You’re strong!$"

Johto_Route32_Text_83EF35E::
	.string "I’m going to try to be the\n"
	.string "best with my favorites.\p"
	.string "I’m not using the same tough\n"
	.string "POKéMON as everyone else.$"

Johto_Route32_Text_83EF3C8::
	.string "I found some good POKéMON\n"
	.string "in the grass!\p"
	.string "I think they’ll do it for me!$"

Johto_Route32_Text_83EF40E::
	.string "Darn. I thought I could win.$"

Johto_Route32_Text_83EF42B::
	.string "The grass is full of\n"
	.string "clingy things.$"

Johto_Route32_Text_83EF44F::
	.string "That glance… It’s intriguing.$"

Johto_Route32_Text_83EF46D::
	.string "Hmmm. This is disappointing.$"

Johto_Route32_Text_83EF48A::
	.string "If you don’t want to battle,\n"
	.string "just avoid eye contact.$"

Johto_Route32_Text_83EF4BF::
	.string "Uh-huh. Yeah, and you know…\p"
	.string "Pardon? Battle? I’m on the phone.\n"
	.string "Oh, all right. But make it fast.$"

Johto_Route32_Text_83EF51E::
	.string "Oh! I’ve got to relieve\n"
	.string "my anger!$"

Johto_Route32_Text_83EF540::
	.string "I was having a nice chat too.$"

Johto_Route32_Text_83EF55E::
	.string "That BADGE! It’s from VIOLET CITY!\n"
	.string "You beat FALKNER?$"

Johto_Route32_Text_83EF593::
	.string "I know what my weaknesses are.$"

Johto_Route32_Text_83EF5B2::
	.string "I should train again at the\n"
	.string "GYM in VIOLET CITY.$"

Johto_Route32_Text_83EF5E2::
	.string "WROOOOAR! PEOPLE RUN WHEN I\n"
	.string "ROAR! BUT YOU CAME LOOKING!\p"
	.string "THAT PLEASES ME! NOW TAKE THIS!$"

Johto_Route32_Text_83EF63A::
	.string "WROOOOAR! A RED CARD can force\n"
	.string "an attacker out of battle\l"
	.string "after it hits the holder!\p"
	.string "It is used up in the process!$"

Johto_Route32_Text_83EF6AB::
	.string "FRIEDA: Yahoo!\p"
	.string "I’m FRIEDA of the WEEKDAY FAMILY!\n"
	.string "Nice to meet you!\p"
	.string "If you can guess my favorite day,\n"
	.string "I’ll give you a reward.$"

Johto_Route32_Text_83EF728::
	.string "Here’s a POISON BARB for you!$"

Johto_Route32_Text_83EF746::
	.string "FRIEDA: Give it to a POKéMON\n"
	.string "that has POISON-type moves.\p"
	.string "Oh!\p"
	.string "It’s wicked!\p"
	.string "You’ll be shocked how good it\n"
	.string "makes poison moves!$"

Johto_Route32_Text_83EF7C2::
	.string "FRIEDA: Hiya! What day do\n"
	.string "you like?\p"
	.string "I love Friday. No doubt about\n"
	.string "it!\p"
	.string "Don’t you think it’s great too?$"

Johto_Route32_Text_83EF828::
	.string "FRIEDA: Not that one!$"

Johto_Route32_Text_83EF83E::
	.string "ROUTE 32\p"
	.string "VIOLET CITY - AZALEA TOWN$"

Johto_Route32_Text_83EF861::
	.string "RUINS OF ALPH\n"
	.string "EAST ENTRANCE$"

Johto_Route32_Text_83EF87D::
	.string "UNION CAVE\n"
	.string "AHEAD$"

Johto_Route32_Text_83FEB33::
	.string "Look around!\p"
	.string "It’s easy to see what POKéMON\n"
	.string "you can catch in an area without\l"
	.string "having to run around in tall\l"
	.string "grass all day!$"
