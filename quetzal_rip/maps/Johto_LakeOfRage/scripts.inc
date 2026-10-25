Johto_LakeOfRage_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_LakeOfRage_EventScript_83DD2C9
	.byte 0

Johto_LakeOfRage_EventScript_83DD2C9::
	call Quetzal_EventScript_840EA62
	goto_if_set FLAG_QZ_0x1318, Johto_LakeOfRage_EventScript_83DD2E3
	goto_if_set FLAG_QZ_0x1425, Johto_LakeOfRage_EventScript_83DD2E3
	setflag FLAG_QZ_0x11C4

Johto_LakeOfRage_EventScript_83DD2E3::
	call_if_lt VAR_QZ_0x500C, 2, Johto_LakeOfRage_EventScript_83DD30B
	setmaplayoutindex 0x21B
	goto_if_lt VAR_QZ_0x500C, 14, Johto_LakeOfRage_EventScript_83DD30F
	random 2
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRage_EventScript_83DD30F
	end

Johto_LakeOfRage_EventScript_83DD30B::
	setflag FLAG_TEMP_1
	return

Johto_LakeOfRage_EventScript_83DD30F::
	setweather 5
	end

@ Johto_LakeOfRage object 15 (local id 15, gfx 0x3EAD) at 9,38
Johto_LakeOfRage_EventScript_83DD313::
	lock
	faceplayer
	msgbox Johto_LakeOfRage_Text_83DD320, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_LakeOfRage object 4 (local id 4, gfx 0x3EA6) at 32,35
Johto_LakeOfRage_EventScript_83DD380::
	goto_if_ge VAR_QZ_0x500C, 14, Johto_LakeOfRage_EventScript_83DD394
	msgbox Johto_LakeOfRage_Text_83DD855, MSGBOX_NPC
	end

Johto_LakeOfRage_EventScript_83DD394::
	msgbox Johto_LakeOfRage_Text_83DD87E, MSGBOX_NPC
	end

@ Johto_LakeOfRage object 1 (local id 1, gfx 0x2F) at 37,36
Johto_LakeOfRage_EventScript_83DD39D::
	msgbox Johto_LakeOfRage_Text_83DD910, MSGBOX_NPC
	end

@ Johto_LakeOfRage object 13 (local id 13, gfx 0x3EB7) at 46,32
Johto_LakeOfRage_EventScript_83DD3A6::
	msgbox Johto_LakeOfRage_Text_83DD89E, MSGBOX_NPC
	end

@ Johto_LakeOfRage object 2 (local id 2, gfx 0x3E95) at 25,38
Johto_LakeOfRage_EventScript_83DD3AF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4459, 0, Johto_LakeOfRage_Text_83DD974, Johto_LakeOfRage_Text_83DD9A2
	msgbox Johto_LakeOfRage_Text_83DD9E1, MSGBOX_AUTOCLOSE
	end

@ Johto_LakeOfRage object 5 (local id 5, gfx 0x3E95) at 3,39
Johto_LakeOfRage_EventScript_83DD3C6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4142, 0, Johto_LakeOfRage_Text_83DDA15, Johto_LakeOfRage_Text_83DDA4C
	msgbox Johto_LakeOfRage_Text_83DDA66, MSGBOX_AUTOCLOSE
	end

@ Johto_LakeOfRage object 9 (local id 9, gfx 0x3E90) at 42,2
Johto_LakeOfRage_EventScript_83DD3DD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4493, 0, Johto_LakeOfRage_Text_83DDA8A, Johto_LakeOfRage_Text_83DDADF
	msgbox Johto_LakeOfRage_Text_83DDAF2, MSGBOX_AUTOCLOSE
	end

@ Johto_LakeOfRage object 7 (local id 7, gfx 0x3E8F) at 16,39
Johto_LakeOfRage_EventScript_83DD3F4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4500, 0, Johto_LakeOfRage_Text_83DDB37, Johto_LakeOfRage_Text_83DDB9E
	msgbox Johto_LakeOfRage_Text_83DDBAA, MSGBOX_AUTOCLOSE
	end

@ Johto_LakeOfRage object 8 (local id 8, gfx 0x3EC0) at 32,28
Johto_LakeOfRage_EventScript_83DD40B::
	lock
	faceplayer
	msgbox Johto_LakeOfRage_Text_83DD842, MSGBOX_DEFAULT
	waitse
	playmoncry SPECIES_GYARADOS, 2
	delay 0x28
	waitmoncry
	qz_setstaticwildmon SPECIES_GYARADOS, 0x1E, ITEM_NONE
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRage_EventScript_83DD44C
	goto_if_eq VAR_RESULT, 4, Johto_LakeOfRage_EventScript_83DD478
	goto_if_eq VAR_RESULT, 5, Johto_LakeOfRage_EventScript_83DD478

Johto_LakeOfRage_EventScript_83DD44C::
	setflag FLAG_QZ_0x104C
	clearflag FLAG_TEMP_1
	removeobject 8
	clearflag FLAG_QZ_0x104D
	addobject 3
	addobject 14
	setorcopyvar VAR_0x8000, 0x378
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setobjectxyperm 4, 0x20, 0x26
	setvar VAR_QZ_0x500C, 2
	release
	end

Johto_LakeOfRage_EventScript_83DD478::
	release
	end

@ Johto_LakeOfRage object 10 (local id 10, gfx 0x3A9E) at 21,3
Johto_LakeOfRage_EventScript_83DD47A::
	setorcopyvar VAR_0x8000, 0x20
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_LakeOfRage object 11 (local id 11, gfx 0x3A9E) at 37,1
Johto_LakeOfRage_EventScript_83DD487::
	setorcopyvar VAR_0x8000, 0x22
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_LakeOfRage object 12 (local id 12, gfx 0x3A9E) at 54,9
Johto_LakeOfRage_EventScript_83DD494::
	setorcopyvar VAR_0x8000, 0x6F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_LakeOfRage object 14 (local id 14, gfx 0x195) at 31,37
Johto_LakeOfRage_EventScript_83DD4A1::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_DRAGONITE, 0
	msgbox Johto_LakeOfRage_Text_83DD5A0, MSGBOX_DEFAULT
	waitmoncry
	turnobject 14, 3
	release
	end

@ Johto_LakeOfRage object 3 (local id 3, gfx 0x3EA0) at 30,37
Johto_LakeOfRage_EventScript_83DD4B8::
	lock
	goto_if_eq VAR_QZ_0x500C, 3, Johto_LakeOfRage_EventScript_83DD531
	msgbox Johto_LakeOfRage_Text_83DD5B5, MSGBOX_DEFAULT
	closemessage
	delay 0x14
	faceplayer
	turnobject 4, 3
	msgbox Johto_LakeOfRage_Text_83DD664, MSGBOX_DEFAULT
	setvar VAR_QZ_0x500C, 3
	msgbox Johto_LakeOfRage_Text_83DD6A9, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRage_EventScript_83DD551

Johto_LakeOfRage_EventScript_83DD4F5::
	lock
	msgbox Johto_LakeOfRage_Text_83DD73C, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_FACING, 2, Johto_LakeOfRage_EventScript_83DD55C
	applymovement 3, Johto_LakeOfRage_Movement_83DD58B
	applymovement 14, Johto_LakeOfRage_Movement_83DD595
	waitmovement 3
	waitmovement 14

Johto_LakeOfRage_EventScript_83DD51E::
	setflag FLAG_QZ_0x104D
	removeobject 3
	removeobject 14
	setvar VAR_QZ_0x500C, 4
	clearflag FLAG_QZ_0x104F
	release
	end

Johto_LakeOfRage_EventScript_83DD531::
	lock
	faceplayer
	msgbox Johto_LakeOfRage_Text_83DD81D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRage_EventScript_83DD4F5
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRage_EventScript_83DD551

Johto_LakeOfRage_EventScript_83DD551::
	msgbox Johto_LakeOfRage_Text_83DD7EA, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_LakeOfRage_EventScript_83DD55C::
	applymovement 3, Johto_LakeOfRage_Movement_83DD575
	applymovement 14, Johto_LakeOfRage_Movement_83DD57F
	waitmovement 3
	waitmovement 14
	goto Johto_LakeOfRage_EventScript_83DD51E

@ Johto_LakeOfRage object 6 (local id 6, gfx 0x2C) at 55,15
Johto_LakeOfRage_EventScript_83DDBDC::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1053, Johto_LakeOfRage_EventScript_83DDC9C
	msgbox Johto_LakeOfRage_Text_83DDCA7, MSGBOX_DEFAULT
	closemessage
	multichoice 0x14, 0, 0x4A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_LakeOfRage_EventScript_83DDC8E
	goto_if_eq VAR_0x8000, 1, Johto_LakeOfRage_EventScript_83DDC8E
	goto_if_eq VAR_0x8000, 2, Johto_LakeOfRage_EventScript_83DDC8E
	goto_if_eq VAR_0x8000, 3, Johto_LakeOfRage_EventScript_83DDC61
	goto_if_eq VAR_0x8000, 4, Johto_LakeOfRage_EventScript_83DDC8E
	goto_if_eq VAR_0x8000, 5, Johto_LakeOfRage_EventScript_83DDC8E
	goto_if_eq VAR_0x8000, 6, Johto_LakeOfRage_EventScript_83DDC8E
	goto_if_eq VAR_0x8000, 7, Johto_LakeOfRage_EventScript_83DDC99
	goto_if_eq VAR_0x8000, 127, Johto_LakeOfRage_EventScript_83DDC99
	closemessage
	release
	end

Johto_LakeOfRage_EventScript_83DDC61::
	msgbox Johto_LakeOfRage_Text_83DDD07, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1AF
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1053
	msgbox Johto_LakeOfRage_Text_83DDD34, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_LakeOfRage_EventScript_83DDC8E::
	msgbox Johto_LakeOfRage_Text_83DDDCF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_LakeOfRage_EventScript_83DDC99::
	closemessage
	release
	end

Johto_LakeOfRage_EventScript_83DDC9C::
	msgbox Johto_LakeOfRage_Text_83DDD6D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_LakeOfRage sign / bg event at 30,36
Johto_LakeOfRage_EventScript_83DDDE6::
	msgbox Johto_LakeOfRage_Text_83DDDF8, MSGBOX_SIGN
	end

@ Johto_LakeOfRage sign / bg event at 37,41
Johto_LakeOfRage_EventScript_83DDDEF::
	msgbox Johto_LakeOfRage_Text_83DDE23, MSGBOX_SIGN
	end

Johto_LakeOfRage_Movement_83DD575:
	walk_left
	walk_down
	delay_8
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_LakeOfRage_Movement_83DD57F:
	delay_16
	delay_16
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_LakeOfRage_Movement_83DD58B:
	walk_down
	delay_8
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_LakeOfRage_Movement_83DD595:
	delay_16
	delay_16
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_LakeOfRage_Text_83DD320::
	.string "{PLAYER}.\p"
	.string "Even here, the cold keeps me sharp.\p"
	.string "If you wish to battle again, come\n"
	.string "to the FIGHTING DOJO.$"

Johto_LakeOfRage_Text_83DD5A0::
	.string "DRAGONITE: Gyafooha!$"

Johto_LakeOfRage_Text_83DD5B5::
	.string "This lake is full of GYARADOS\n"
	.string "but nothing else…\p"
	.string "So the MAGIKARP are being forced\n"
	.string "to evolve…$"

Johto_LakeOfRage_Text_83DD664::
	.string "This man here informed me of\n"
	.string "some rumors.\p"
	.string "So we came to investigate…$"

Johto_LakeOfRage_Text_83DD6A9::
	.string "I saw the way you battled\n"
	.string "earlier, {PLAYER}.\p"
	.string "I can tell that you’re a trainer\n"
	.string "with considerable skill.\p"
	.string "If you don’t mind, could you help\n"
	.string "us investigate?$"

Johto_LakeOfRage_Text_83DD73C::
	.string "LANCE: Excellent!\p"
	.string "It seems that the LAKE’s MAGIKARP\n"
	.string "are being forced to evolve.\p"
	.string "A mysterious radio broadcast\n"
	.string "coming from MAHOGANY is the cause.\p"
	.string "We’ll be waiting for you,\n"
	.string "{PLAYER}.$"

Johto_LakeOfRage_Text_83DD7EA::
	.string "Oh… Well, if you change your mind,\n"
	.string "please help us.$"

Johto_LakeOfRage_Text_83DD81D::
	.string "LANCE: Hm? Are you going to help\n"
	.string "us?$"

Johto_LakeOfRage_Text_83DD842::
	.string "GYARADOS: Gyashaa!$"

Johto_LakeOfRage_Text_83DD855::
	.string "The GYARADOS are angry!\p"
	.string "It’s a bad omen!$"

Johto_LakeOfRage_Text_83DD87E::
	.string "Hahah! The MAGIKARP are biting!$"

Johto_LakeOfRage_Text_83DD89E::
	.string "I heard this lake was made by\n"
	.string "rampaging GYARADOS.\p"
	.string "I wonder if there is any\n"
	.string "connection to their mass outbreak\l"
	.string "now?$"

Johto_LakeOfRage_Text_83DD910::
	.string "Did my eyes deceive me? I saw a\n"
	.string "red GYARADOS in the LAKE…\p"
	.string "But I thought GYARADOS were\n"
	.string "usually blue?$"

Johto_LakeOfRage_Text_83DD974::
	.string "Let me battle with the POKéMON I\n"
	.string "just caught!$"

Johto_LakeOfRage_Text_83DD9A2::
	.string "I might be an expert angler, but\n"
	.string "I stink as a POKéMON trainer…$"

Johto_LakeOfRage_Text_83DD9E1::
	.string "I won’t lose as an angler! I catch\n"
	.string "POKéMON all day.$"

Johto_LakeOfRage_Text_83DDA15::
	.string "No matter what I do, all I catch\n"
	.string "are the same POKéMON…$"

Johto_LakeOfRage_Text_83DDA4C::
	.string "My line’s all tangled up…$"

Johto_LakeOfRage_Text_83DDA66::
	.string "Why can’t I catch any good\n"
	.string "POKéMON?$"

Johto_LakeOfRage_Text_83DDA8A::
	.string "If a trainer spots another trainer,\n"
	.string "he has to make a challenge.\p"
	.string "That is our destiny.$"

Johto_LakeOfRage_Text_83DDADF::
	.string "Whew… Good battle.$"

Johto_LakeOfRage_Text_83DDAF2::
	.string "POKéMON and their trainer become\n"
	.string "powerful through constant battling.$"

Johto_LakeOfRage_Text_83DDB37::
	.string "What happened to the red\n"
	.string "GYARADOS?\p"
	.string "It’s gone?\p"
	.string "Oh, darn. I came here for\n"
	.string "nothing?\p"
	.string "I know--let’s battle!$"

Johto_LakeOfRage_Text_83DDB9E::
	.string "Good going!$"

Johto_LakeOfRage_Text_83DDBAA::
	.string "Come to think of it, I’ve seen a\n"
	.string "pink BUTTERFREE.$"

Johto_LakeOfRage_Text_83DDCA7::
	.string "WESLEY: Well, how do you do?\p"
	.string "I’m a member of the WEEKDAY family!\n"
	.string "Can you guess my favorite day?$"

Johto_LakeOfRage_Text_83DDD07::
	.string "Pleased to meet you. Please take a\n"
	.string "souvenir.$"

Johto_LakeOfRage_Text_83DDD34::
	.string "WESLEY: BLACK BELT beefs up the\n"
	.string "power of fighting moves.$"

Johto_LakeOfRage_Text_83DDD6D::
	.string "WESLEY: Since you found me, you\n"
	.string "must have met my brothers and\l"
	.string "sisters.\p"
	.string "Or did you just get lucky?$"

Johto_LakeOfRage_Text_83DDDCF::
	.string "WESLEY: Not exactly...$"

Johto_LakeOfRage_Text_83DDDF8::
	.string "LAKE OF RAGE,\n"
	.string "also known as GYARADOS LAKE.$"

Johto_LakeOfRage_Text_83DDE23::
	.string "FISHING GURU’S HOUSE$"
