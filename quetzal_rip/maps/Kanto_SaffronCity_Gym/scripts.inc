Kanto_SaffronCity_Gym_MapScripts::
	.byte 0

@ Kanto_SaffronCity_Gym object 7 (local id 7, gfx 0x3F40) at 14,11
Kanto_SaffronCity_Gym_EventScript_841EE6B::
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8612, 0, Kanto_SaffronCity_Gym_Text_844B497, Kanto_SaffronCity_Gym_Text_844B55D, Kanto_SaffronCity_Gym_EventScript_841EE9D

Kanto_SaffronCity_Gym_EventScript_841EE8A::
	goto_if_unset FLAG_QZ_0x2167, Kanto_SaffronCity_Gym_EventScript_841EEE2
	msgbox Kanto_SaffronCity_Gym_Text_844B736, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_Gym_EventScript_841EE9D::
	goto_if_set FLAG_QZ_0x2105, Kanto_SaffronCity_Gym_EventScript_841EE8A
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 3
	special QzQuestLogNop_737
	clearflag FLAG_QZ_0x21B4
	setflag FLAG_QZ_0x2125
	call_if_unset FLAG_QZ_0x2105, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2105
	setvar VAR_0x8008, 6
	call Quetzal_EventScript_8456FD2
	goto Kanto_SaffronCity_Gym_EventScript_841EEE2

Kanto_SaffronCity_Gym_EventScript_841EEE2::
	msgbox Kanto_SaffronCity_Gym_Text_844B655, MSGBOX_DEFAULT
	checkitemspace ITEM_CHOICE_SPECS, 1
	goto_if_eq VAR_RESULT, 0, Kanto_SaffronCity_Gym_EventScript_841EF2A
	bufferitemname 0, ITEM_CHOICE_SPECS
	additem ITEM_CHOICE_SPECS, 1
	loadword 0, Kanto_SaffronCity_Gym_Text_844B726
	setorcopyvar VAR_0x8000, 0x1BB
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2167
	msgbox Kanto_SaffronCity_Gym_Text_844B5E0, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_Gym_EventScript_841EF2A::
	msgbox Kanto_SaffronCity_Gym_Text_844B792, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_Gym object 2 (local id 2, gfx 0x3EFD) at 24,2
Kanto_SaffronCity_Gym_EventScript_841EF34::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8472, 0, Kanto_SaffronCity_Gym_Text_844B87F, Kanto_SaffronCity_Gym_Text_844B8A0
	msgbox Kanto_SaffronCity_Gym_Text_844B8B6, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Gym object 8 (local id 8, gfx 0x3EFD) at 24,10
Kanto_SaffronCity_Gym_EventScript_841EF4B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8473, 0, Kanto_SaffronCity_Gym_Text_844B99C, Kanto_SaffronCity_Gym_Text_844B9F5
	setvar VAR_0x8004, 7
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	msgbox Kanto_SaffronCity_Gym_Text_844BA0B, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Gym object 1 (local id 1, gfx 0x3EFD) at 24,18
Kanto_SaffronCity_Gym_EventScript_841EF6F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8474, 0, Kanto_SaffronCity_Gym_Text_844BAA0, Kanto_SaffronCity_Gym_Text_844BAF8
	msgbox Kanto_SaffronCity_Gym_Text_844BB0A, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Gym object 3 (local id 3, gfx 0x3EFD) at 4,2
Kanto_SaffronCity_Gym_EventScript_841EF86::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8475, 0, Kanto_SaffronCity_Gym_Text_844BBAE, Kanto_SaffronCity_Gym_Text_844BC13
	msgbox Kanto_SaffronCity_Gym_Text_844BC1B, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Gym object 4 (local id 4, gfx 0x3EF6) at 14,2
Kanto_SaffronCity_Gym_EventScript_841EF9D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8654, 0, Kanto_SaffronCity_Gym_Text_844B7B3, Kanto_SaffronCity_Gym_Text_844B7F1
	msgbox Kanto_SaffronCity_Gym_Text_844B802, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Gym object 5 (local id 5, gfx 0x3EF6) at 4,10
Kanto_SaffronCity_Gym_EventScript_841EFB4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8655, 0, Kanto_SaffronCity_Gym_Text_844B8EF, Kanto_SaffronCity_Gym_Text_844B943
	msgbox Kanto_SaffronCity_Gym_Text_844B94E, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Gym object 6 (local id 6, gfx 0x3EF6) at 4,18
Kanto_SaffronCity_Gym_EventScript_841EFCB::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8656, 0, Kanto_SaffronCity_Gym_Text_844BA3F, Kanto_SaffronCity_Gym_Text_844BA64
	msgbox Kanto_SaffronCity_Gym_Text_844BA76, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Gym object 9 (local id 9, gfx 0x3F0B) at 15,20
Kanto_SaffronCity_Gym_EventScript_841EFE2::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2125, Kanto_SaffronCity_Gym_EventScript_841EFF7
	msgbox Kanto_SaffronCity_Gym_Text_844BC4D, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_Gym_EventScript_841EFF7::
	msgbox Kanto_SaffronCity_Gym_Text_844BD12, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_Gym sign / bg event at 12,20
@ Kanto_SaffronCity_Gym sign / bg event at 16,20
Kanto_SaffronCity_Gym_EventScript_841F001::
	lockall
	goto_if_set FLAG_QZ_0x2105, Kanto_SaffronCity_Gym_EventScript_841F015
	msgbox Kanto_SaffronCity_Gym_Text_844BD55, MSGBOX_DEFAULT
	releaseall
	end

Kanto_SaffronCity_Gym_EventScript_841F015::
	msgbox Kanto_SaffronCity_Gym_Text_844BD90, MSGBOX_DEFAULT
	releaseall
	end

Kanto_SaffronCity_Gym_Text_844B497::
	.string "I had a vision of your arrival.\p"
	.string "I have had psychic powers since\n"
	.string "I was a child.\p"
	.string "It started when a spoon I\n"
	.string "carelessly tossed, bent.\p"
	.string "I dislike battling, but if you wish,\n"
	.string "I will show you my powers!{PLAY_BGM 0x022A}$"

Kanto_SaffronCity_Gym_Text_844B55D::
	.string "This loss shocks me!\n"
	.string "But, a loss is a loss.\p"
	.string "I admit, I didn’t work hard enough\n"
	.string "to win.\p"
	.string "Your victory has earned you the\n"
	.string "MARSHBADGE.$"

Kanto_SaffronCity_Gym_Text_844B5E0::
	.string "Psychic power isn’t something that\n"
	.string "only a few people have.\p"
	.string "Everyone has psychic power.\n"
	.string "People just don’t realize it.$"

Kanto_SaffronCity_Gym_Text_844B655::
	.string "The MARSHBADGE makes POKéMON up\n"
	.string "to Lv. 70 obey you.\p"
	.string "Stronger POKéMON will become wild,\n"
	.string "ignoring your orders in battle.\p"
	.string "Just don’t raise your POKéMON too\n"
	.string "much to avoid that problem.\p"
	.string "Please take this gift, too.$"

Kanto_SaffronCity_Gym_Text_844B726::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_SaffronCity_Gym_Text_844B736::
	.string "It boosts the holder’s Special\n"
	.string "Attack, but locks it into one move\l"
	.string "until you switch POKéMON.$"

Kanto_SaffronCity_Gym_Text_844B792::
	.string "Your BAG is full of other items.$"

Kanto_SaffronCity_Gym_Text_844B7B3::
	.string "SABRINA is far younger than I,\n"
	.string "but she has earned my respect.$"

Kanto_SaffronCity_Gym_Text_844B7F1::
	.string "Not good enough!$"

Kanto_SaffronCity_Gym_Text_844B802::
	.string "In a battle of equals, the one\n"
	.string "with the stronger will wins.\p"
	.string "If you wish to beat SABRINA, focus\n"
	.string "and will yourself to victory.$"

Kanto_SaffronCity_Gym_Text_844B87F::
	.string "Does our unseen power scare you?$"

Kanto_SaffronCity_Gym_Text_844B8A0::
	.string "I never foresaw this!$"

Kanto_SaffronCity_Gym_Text_844B8B6::
	.string "BUG, GHOST, and DARK moves can\n"
	.string "threaten PSYCHIC POKéMON.$"

Kanto_SaffronCity_Gym_Text_844B8EF::
	.string "POKéMON take on the appearance of\n"
	.string "their TRAINERS.\p"
	.string "Your POKéMON must be tough, then!$"

Kanto_SaffronCity_Gym_Text_844B943::
	.string "I knew it!$"

Kanto_SaffronCity_Gym_Text_844B94E::
	.string "I have much to learn still…\p"
	.string "I must master PSYCHIC and teach it\n"
	.string "to my POKéMON…$"

Kanto_SaffronCity_Gym_Text_844B99C::
	.string "You understand, don’t you?\p"
	.string "That power alone isn’t enough to\n"
	.string "win in the world of POKéMON?$"

Kanto_SaffronCity_Gym_Text_844B9F5::
	.string "I don’t believe this!$"

Kanto_SaffronCity_Gym_Text_844BA0B::
	.string "SABRINA just wiped out the KARATE\n"
	.string "MASTER next door.$"

Kanto_SaffronCity_Gym_Text_844BA3F::
	.string "You and I, our POKéMON shall\n"
	.string "battle!$"

Kanto_SaffronCity_Gym_Text_844BA64::
	.string "I lost after all!$"

Kanto_SaffronCity_Gym_Text_844BA76::
	.string "I knew that this was going to take\n"
	.string "place.$"

Kanto_SaffronCity_Gym_Text_844BAA0::
	.string "SABRINA is young, but she’s also\n"
	.string "our highly skilled LEADER.\p"
	.string "You won’t reach her easily!$"

Kanto_SaffronCity_Gym_Text_844BAF8::
	.string "Ow!\n"
	.string "Down and out!$"

Kanto_SaffronCity_Gym_Text_844BB0A::
	.string "There used to be two POKéMON GYMS\n"
	.string "in SAFFRON.\p"
	.string "The FIGHTING DOJO next door lost\n"
	.string "its GYM status, though.\p"
	.string "We simply creamed them when we\n"
	.string "decided to have just one GYM.$"

Kanto_SaffronCity_Gym_Text_844BBAE::
	.string "SAFFRON POKéMON GYM is famous for\n"
	.string "training psychics.\p"
	.string "You want to see SABRINA, don’t\n"
	.string "you? I can tell!$"

Kanto_SaffronCity_Gym_Text_844BC13::
	.string "Arrrgh!$"

Kanto_SaffronCity_Gym_Text_844BC1B::
	.string "That’s right! I used telepathy to\n"
	.string "read your mind!$"

Kanto_SaffronCity_Gym_Text_844BC4D::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "SABRINA’s POKéMON use psychic\n"
	.string "power instead of force.\p"
	.string "FIGHTING POKéMON are especially\n"
	.string "weak against PSYCHIC POKéMON.\p"
	.string "They’ll get destroyed before they\n"
	.string "can even aim a punch!$"

Kanto_SaffronCity_Gym_Text_844BD12::
	.string "Psychic power, huh?\p"
	.string "If I had that, I’d make a bundle\n"
	.string "at the slots!$"

Kanto_SaffronCity_Gym_Text_844BD55::
	.string "SAFFRON POKéMON GYM\n"
	.string "LEADER: SABRINA\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_SaffronCity_Gym_Text_844BD90::
	.string "SAFFRON POKéMON GYM\n"
	.string "LEADER: SABRINA\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"
