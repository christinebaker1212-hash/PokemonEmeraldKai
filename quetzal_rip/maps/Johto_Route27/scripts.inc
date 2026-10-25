Johto_Route27_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route27_EventScript_83EC35F
	.byte 0

Johto_Route27_EventScript_83EC35F::
	call Quetzal_EventScript_840EA62
	end

@ Johto_Route27 trigger at 25,25 (VAR_QZ_0x5029 == 0)
@ Johto_Route27 trigger at 25,24 (VAR_QZ_0x5029 == 0)
Johto_Route27_EventScript_83EC378::
	lock
	playse SE_PIN
	setflag FLAG_QZ_0x1068
	applymovement 1, Quetzal_Movement_83607F9
	turnobject 1, 3
	delay 0x1E
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_1, 24, Johto_Route27_EventScript_83EC3C4
	call_if_eq VAR_TEMP_1, 25, Johto_Route27_EventScript_83EC3CF
	msgbox Johto_Route27_Text_83EC4C7, MSGBOX_DEFAULT
	msgbox Johto_Route27_Text_83EC4CC, MSGBOX_DEFAULT
	closemessage
	turnobject 1, 1
	setvar VAR_QZ_0x5029, 1
	release
	end

Johto_Route27_EventScript_83EC3C4::
	applymovement 1, Johto_Route27_Movement_83EC3DA
	waitmovement 0
	return

Johto_Route27_EventScript_83EC3CF::
	applymovement 1, Johto_Route27_Movement_83EC3DD
	waitmovement 0
	return

@ Johto_Route27 object 7 (local id 7, gfx 0x3B) at 69,14
Johto_Route27_EventScript_83EC3E0::
	setorcopyvar VAR_0x8000, 0xD9
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route27 object 6 (local id 6, gfx 0x3B) at 76,26
Johto_Route27_EventScript_83EC3ED::
	setorcopyvar VAR_0x8000, 0x66
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route27 object 11 (local id 11, gfx 0x3B) at 133,17
Johto_Route27_EventScript_83EC3FA::
	setorcopyvar VAR_0x8000, 0xDA
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route27 object 12 (local id 12, gfx 0x3B) at 93,14
Johto_Route27_EventScript_83EC407::
	setorcopyvar VAR_0x8000, 0x21
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route27 object 1 (local id 1, gfx 0x11) at 27,24
Johto_Route27_EventScript_83EC414::
	msgbox Johto_Route27_Text_83EC4CC, MSGBOX_NPC
	end

@ Johto_Route27 object 5 (local id 5, gfx 0x3E90) at 89,18
Johto_Route27_EventScript_83EC41D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4331, 0, Johto_Route27_Text_83EC51C, Johto_Route27_Text_83EC547
	msgbox Johto_Route27_Text_83EC54C, MSGBOX_AUTOCLOSE
	end

@ Johto_Route27 object 3 (local id 3, gfx 0x3E90) at 66,19
Johto_Route27_EventScript_83EC434::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4709, 0, Johto_Route27_Text_83EC5BC, Johto_Route27_Text_83EC5D9
	msgbox Johto_Route27_Text_83EC5EC, MSGBOX_AUTOCLOSE
	end

@ Johto_Route27 object 9 (local id 9, gfx 0x3E8F) at 127,12
Johto_Route27_EventScript_83EC44B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4147, 0, Johto_Route27_Text_83EC61E, Johto_Route27_Text_83EC65B
	msgbox Johto_Route27_Text_83EC677, MSGBOX_AUTOCLOSE
	end

@ Johto_Route27 object 2 (local id 2, gfx 0x3E8F) at 44,22
Johto_Route27_EventScript_83EC462::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4444, 0, Johto_Route27_Text_83EC6B6, Johto_Route27_Text_83EC6F7
	msgbox Johto_Route27_Text_83EC711, MSGBOX_AUTOCLOSE
	end

@ Johto_Route27 object 8 (local id 8, gfx 0x25) at 100,18
Johto_Route27_EventScript_83EC479::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4265, 0, Johto_Route27_Text_83EC79E, Johto_Route27_Text_83EC80F
	msgbox Johto_Route27_Text_83EC820, MSGBOX_AUTOCLOSE
	end

@ Johto_Route27 object 4 (local id 4, gfx 0x3EAE) at 77,27
Johto_Route27_EventScript_83EC490::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4713, 0, Johto_Route27_Text_83EC871, Johto_Route27_Text_83EC888
	msgbox Johto_Route27_Text_83EC88F, MSGBOX_AUTOCLOSE
	end

@ Johto_Route27 object 10 (local id 10, gfx 0x3E95) at 145,25
Johto_Route27_EventScript_83EC4A7::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4099, 0, Johto_Route27_Text_83EC8F9, Johto_Route27_Text_83EC942
	msgbox Johto_Route27_Text_83EC95A, MSGBOX_AUTOCLOSE
	end

@ Johto_Route27 sign / bg event at 32,20
@ Johto_Route27 sign / bg event at 44,18
Johto_Route27_EventScript_83EC4BE::
	msgbox Johto_Route27_Text_83EC8CC, MSGBOX_SIGN
	end

Johto_Route27_Movement_83EC3DA:
	face_left
	walk_left
	step_end

Johto_Route27_Movement_83EC3DD:
	walk_down
	walk_left
	step_end

Johto_Route27_Text_83EC4C7::
	.string "Hey!$"

Johto_Route27_Text_83EC4CC::
	.string "Do you know what you just did?\p"
	.string "You’ve crossed the border from\n"
	.string "JOHTO into KANTO.$"

Johto_Route27_Text_83EC51C::
	.string "You look pretty strong.\n"
	.string "Let me battle you!$"

Johto_Route27_Text_83EC547::
	.string "Yow!$"

Johto_Route27_Text_83EC54C::
	.string "If you prevail on this harsh trek,\n"
	.string "the truth will be revealed!\p"
	.string "Heh, sorry, I just wanted to say\n"
	.string "something cool.$"

Johto_Route27_Text_83EC5BC::
	.string "Hm? You’re good, aren’t you?$"

Johto_Route27_Text_83EC5D9::
	.string "Just as I thought!$"

Johto_Route27_Text_83EC5EC::
	.string "A good trainer can recognize other\n"
	.string "good trainers.$"

Johto_Route27_Text_83EC61E::
	.string "You shouldn’t underestimate the\n"
	.string "wild POKéMON in these parts.$"

Johto_Route27_Text_83EC65B::
	.string "Oh! You’re much too strong!$"

Johto_Route27_Text_83EC677::
	.string "You’re just a kid, but you’re not\n"
	.string "to be underestimated either.$"

Johto_Route27_Text_83EC6B6::
	.string "It’s rare to see anyone come here.\p"
	.string "Are you training on your own?$"

Johto_Route27_Text_83EC6F7::
	.string "Oh! You’re really strong!$"

Johto_Route27_Text_83EC711::
	.string "I’m checking out pre- and post-\n"
	.string "evolution POKéMON.\p"
	.string "Evolution really does make\n"
	.string "POKéMON stronger.\p"
	.string "But evolved forms also learn\n"
	.string "moves later on.$"

Johto_Route27_Text_83EC79E::
	.string "Don’t say a thing!\p"
	.string "Let me guess what you’re thinking.\p"
	.string "Mmmmmmm…\p"
	.string "I got it! You’re on the POKéMON\n"
	.string "LEAGUE challenge!$"

Johto_Route27_Text_83EC80F::
	.string "You’re too much!$"

Johto_Route27_Text_83EC820::
	.string "With your skills, you’ll do well at\n"
	.string "the LEAGUE.\p"
	.string "That’s what my premonition says.$"

Johto_Route27_Text_83EC871::
	.string "Tweet! Tweet! Tetweet!$"

Johto_Route27_Text_83EC888::
	.string "Tweet!$"

Johto_Route27_Text_83EC88F::
	.string "BIRD KEEPERS like me mimic bird\n"
	.string "whistles to command POKéMON.$"

Johto_Route27_Text_83EC8CC::
	.string "TOHJO FALLS\p"
	.string "THE LINK BETWEEN KANTO AND JOHTO$"

Johto_Route27_Text_83EC8F9::
	.string "I’m feeling great today!\p"
	.string "I feel like I could boot even the\n"
	.string "LEAGUE CHAMP!$"

Johto_Route27_Text_83EC942::
	.string "No! Not in this battle!$"

Johto_Route27_Text_83EC95A::
	.string "Just like in fishing, it’s all over\n"
	.string "in POKéMON if you give up.$"
