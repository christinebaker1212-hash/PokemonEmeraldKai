Johto_CianwoodGym_MapScripts::
	.byte 0

@ Johto_CianwoodGym sign / bg event at 13,14
@ Johto_CianwoodGym sign / bg event at 7,14
Johto_CianwoodGym_EventScript_83CA487::
	lock
	goto_if_set FLAG_QZ_0x11AC, Johto_CianwoodGym_EventScript_83CA49C
	msgbox Johto_CianwoodGym_Text_83CA4A7, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodGym_EventScript_83CA49C::
	msgbox Johto_CianwoodGym_Text_83CA4C1, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodGym object 1 (local id 1, gfx 0x3E8C) at 10,2
Johto_CianwoodGym_EventScript_83CA4FA::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x14BB, Johto_CianwoodGym_EventScript_83CA5A7
	goto_if_set FLAG_QZ_0x11AC, Johto_CianwoodGym_EventScript_83CA6B1
	msgbox Johto_CianwoodGym_Text_83CA6F9, MSGBOX_DEFAULT
	applymovement 1, Johto_CianwoodGym_Movement_83CA6F5
	waitmovement 0
	message Johto_CianwoodGym_Text_83CA773
	applymovement 10, Quetzal_Movement_837EF9D
	playse SE_M_ROCK_THROW
	waitmovement 0
	delay 0x14
	applymovement 1, Johto_CianwoodGym_Movement_83CA6F7
	waitmovement 0
	applymovement 9, Quetzal_Movement_837EF9D
	playse SE_M_ROCK_THROW
	waitmovement 0
	waitmessage
	turnobject 1, 1
	msgbox Johto_CianwoodGym_Text_83CA78D, MSGBOX_DEFAULT
	goto Johto_CianwoodGym_EventScript_83CA683

Johto_CianwoodGym_EventScript_83CA55E::
	message Johto_CianwoodGym_Text_83CA83B
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_CianwoodGym_Text_83CA853, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B6, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B6
	setflag FLAG_QZ_0x11AC
	addvar VAR_QZ_0x502E, 1
	call Quetzal_EventScript_83DF68E
	setvar VAR_0x8008, 5
	call Quetzal_EventScript_83601F0
	setflag FLAG_QZ_0x14BB
	call Johto_CianwoodGym_EventScript_83CA5AE
	setvar VAR_QZ_0x501E, 1
	setrespawn 0x1E
	closemessage
	release
	end

Johto_CianwoodGym_EventScript_83CA5A7::
	call Johto_CianwoodGym_EventScript_83CA5AE
	release
	end

Johto_CianwoodGym_EventScript_83CA5AE::
	setorcopyvar VAR_0x8000, ITEM_LIFE_ORB
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602B4
	clearflag FLAG_QZ_0x14BB
	msgbox Johto_CianwoodGym_Text_83CA8E6, MSGBOX_DEFAULT
	return

Johto_CianwoodGym_EventScript_83CA683::
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_CianwoodGym_EventScript_83CA6A2
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4606, 0, Johto_CianwoodGym_Text_83CA773
	goto Johto_CianwoodGym_EventScript_83CA55E

Johto_CianwoodGym_EventScript_83CA6A2::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4967, 0, Johto_CianwoodGym_Text_83CA773
	goto Johto_CianwoodGym_EventScript_83CA55E

Johto_CianwoodGym_EventScript_83CA6B1::
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_CianwoodGym_EventScript_83CA6D8
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4606, 0, Johto_CianwoodGym_Text_83CA6F9, Johto_CianwoodGym_Text_83CA773, Johto_CianwoodGym_EventScript_83CA6EA
	goto Johto_CianwoodGym_EventScript_83CA6EA

Johto_CianwoodGym_EventScript_83CA6D8::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4967, 0, Johto_CianwoodGym_Text_83CA6F9, Johto_CianwoodGym_Text_83CA773, Johto_CianwoodGym_EventScript_83CA6EA

Johto_CianwoodGym_EventScript_83CA6EA::
	msgbox Johto_CianwoodGym_Text_83CA930, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CianwoodGym object 3 (local id 3, gfx 0x2C) at 3,8
Johto_CianwoodGym_EventScript_83CAA22::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4685, 0, Johto_CianwoodGym_Text_83CAA39, Johto_CianwoodGym_Text_83CAA8D
	msgbox Johto_CianwoodGym_Text_83CAA9E, MSGBOX_AUTOCLOSE
	end

@ Johto_CianwoodGym object 4 (local id 4, gfx 0x2C) at 14,8
Johto_CianwoodGym_EventScript_83CAAD4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4145, 0, Johto_CianwoodGym_Text_83CAAEB, Johto_CianwoodGym_Text_83CAB0C
	msgbox Johto_CianwoodGym_Text_83CAB1D, MSGBOX_AUTOCLOSE
	end

@ Johto_CianwoodGym object 2 (local id 2, gfx 0x2C) at 4,4
Johto_CianwoodGym_EventScript_83CAB46::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4855, 0, Johto_CianwoodGym_Text_83CAB5D, Johto_CianwoodGym_Text_83CAB8F
	msgbox Johto_CianwoodGym_Text_83CAB91, MSGBOX_AUTOCLOSE
	end

@ Johto_CianwoodGym object 5 (local id 5, gfx 0x2C) at 15,6
Johto_CianwoodGym_EventScript_83CABA9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4516, 0, Johto_CianwoodGym_Text_83CABC0, Johto_CianwoodGym_Text_83CABEB
	msgbox Johto_CianwoodGym_Text_83CABFC, MSGBOX_AUTOCLOSE
	end

Johto_CianwoodGym_Movement_83CA6F5:
	jump_in_place_left
	step_end

Johto_CianwoodGym_Movement_83CA6F7:
	jump_in_place_right
	step_end

Johto_CianwoodGym_Text_83CA4A7::
	.string "CIANWOOD CITY POKéMON GYM$"

Johto_CianwoodGym_Text_83CA4C1::
	.string "CIANWOOD CITY POKéMON GYM\p"
	.string "CHUCK’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

Johto_CianwoodGym_Text_83CA6F9::
	.string "WAHAHAH!\p"
	.string "So you’ve come this far!\p"
	.string "Let me tell you, I’m tough!\p"
	.string "My POKéMON will crush stones and\n"
	.string "shatter bones!\p"
	.string "Watch this!$"

Johto_CianwoodGym_Text_83CA773::
	.string "CHUCK: Urggh!\n"
	.string "…\p"
	.string "Oooarrgh!$"

Johto_CianwoodGym_Text_83CA78D::
	.string "There! Scared now, are you?\p"
	.string "What? It has nothing to do with\n"
	.string "POKéMON?\p"
	.string "That’s true!\p"
	.string "Come on. We shall do battle!$"

Johto_CianwoodGym_Text_83CA83B::
	.string "{PLAYER} received STORMBADGE.$"

Johto_CianwoodGym_Text_83CA853::
	.string "STORMBADGE makes all POKéMON up to\n"
	.string "L70 obey, even traded ones.\p"
	.string "It also lets your POKéMON use FLY\n"
	.string "when you’re not in a battle.\p"
	.string "Here, take this too!$"

Johto_CianwoodGym_Text_83CA8E6::
	.string "That’s a PUNCHING GLOVE.\p"
	.string "Give it to a POKéMON that fights\n"
	.string "with its fists!$"

Johto_CianwoodGym_Text_83CA930::
	.string "WAHAHAH! I enjoyed battling you!\p"
	.string "But a loss is a loss!\p"
	.string "From now on, I’m going to train\n"
	.string "24 hours a day!$"

Johto_CianwoodGym_Text_83CAA39::
	.string "My POKéMON and I are bound togeth-\n"
	.string "er by friendship.\p"
	.string "Our bond will never be broken!$"

Johto_CianwoodGym_Text_83CAA8D::
	.string "This isn’t real!$"

Johto_CianwoodGym_Text_83CAA9E::
	.string "You seem to have a strong bond\n"
	.string "with your POKéMON too!$"

Johto_CianwoodGym_Text_83CAAEB::
	.string "We martial artists fear nothing!$"

Johto_CianwoodGym_Text_83CAB0C::
	.string "That’s shocking!$"

Johto_CianwoodGym_Text_83CAB1D::
	.string "Fighting POKéMON are afraid of\n"
	.string "psychics…$"

Johto_CianwoodGym_Text_83CAB5D::
	.string "Words are useless.\n"
	.string "Let your fists do the talking!$"

Johto_CianwoodGym_Text_83CAB8F::
	.string "…$"

Johto_CianwoodGym_Text_83CAB91::
	.string "I lost! I’m speechless!$"

Johto_CianwoodGym_Text_83CABC0::
	.string "My raging fists will shatter your\n"
	.string "POKéMON!$"

Johto_CianwoodGym_Text_83CABEB::
	.string "I got shattered!$"

Johto_CianwoodGym_Text_83CABFC::
	.string "My POKéMON lost…\n"
	.string "My… my pride is shattered…$"
