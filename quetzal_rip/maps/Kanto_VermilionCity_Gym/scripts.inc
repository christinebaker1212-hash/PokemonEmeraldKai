Kanto_VermilionCity_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_VermilionCity_Gym_EventScript_841B69D
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_VermilionCity_Gym_EventScript_841B766
	.byte 0

Kanto_VermilionCity_Gym_EventScript_841B69D::
	call_if_set FLAG_TEMP_1, Kanto_VermilionCity_Gym_EventScript_841B6B0
	call_if_set FLAG_QZ_0x213C, Kanto_VermilionCity_Gym_EventScript_841B70B
	end

Kanto_VermilionCity_Gym_EventScript_841B6B0::
	setmetatile 3, 6, 0x2BB, 1
	setmetatile 4, 6, 0x2BC, 1
	setmetatile 5, 6, 0x2BD, 1
	setmetatile 6, 6, 0x2BE, 1
	setmetatile 7, 6, 0x2BF, 1
	setmetatile 3, 7, 0x2C3, 1
	setmetatile 4, 7, 0x2C4, 1
	setmetatile 5, 7, 0x2C5, 1
	setmetatile 6, 7, 0x2C6, 1
	setmetatile 7, 7, 0x2C7, 1
	return

Kanto_VermilionCity_Gym_EventScript_841B70B::
	setmetatile 3, 6, 0x293, 1
	setmetatile 4, 6, 0x294, 0
	setmetatile 5, 6, 0x281, 0
	setmetatile 6, 6, 0x295, 0
	setmetatile 7, 6, 0x296, 1
	setmetatile 3, 7, 0x29B, 1
	setmetatile 4, 7, 0x29C, 0
	setmetatile 5, 7, 0x281, 0
	setmetatile 6, 7, 0x29D, 0
	setmetatile 7, 7, 0x29E, 1
	return

Kanto_VermilionCity_Gym_EventScript_841B766::
	call Kanto_VermilionCity_Gym_EventScript_841B76C
	end

Kanto_VermilionCity_Gym_EventScript_841B76C::
	goto_if_set FLAG_QZ_0x213C, Quetzal_EventScript_8457C6C
	special QzSetVermilionTrashCans_752
	copyvar VAR_TEMP_0, VAR_0x8004
	copyvar VAR_TEMP_1, VAR_0x8005
	return

@ Kanto_VermilionCity_Gym sign / bg event at 1,10
Kanto_VermilionCity_Gym_EventScript_841B783::
	lockall
	setvar VAR_0x8008, 1
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 3,10
Kanto_VermilionCity_Gym_EventScript_841B78F::
	lockall
	setvar VAR_0x8008, 2
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 5,10
Kanto_VermilionCity_Gym_EventScript_841B79B::
	lockall
	setvar VAR_0x8008, 3
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 7,10
Kanto_VermilionCity_Gym_EventScript_841B7A7::
	lockall
	setvar VAR_0x8008, 4
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 9,10
Kanto_VermilionCity_Gym_EventScript_841B7B3::
	lockall
	setvar VAR_0x8008, 5
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 1,12
Kanto_VermilionCity_Gym_EventScript_841B7BF::
	lockall
	setvar VAR_0x8008, 6
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 3,12
Kanto_VermilionCity_Gym_EventScript_841B7CB::
	lockall
	setvar VAR_0x8008, 7
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 5,12
Kanto_VermilionCity_Gym_EventScript_841B7D7::
	lockall
	setvar VAR_0x8008, 8
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 7,12
Kanto_VermilionCity_Gym_EventScript_841B7E3::
	lockall
	setvar VAR_0x8008, 9
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 9,12
Kanto_VermilionCity_Gym_EventScript_841B7EF::
	lockall
	setvar VAR_0x8008, 0xA
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 1,14
Kanto_VermilionCity_Gym_EventScript_841B7FB::
	lockall
	setvar VAR_0x8008, 0xB
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 3,14
Kanto_VermilionCity_Gym_EventScript_841B807::
	lockall
	setvar VAR_0x8008, 0xC
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 5,14
Kanto_VermilionCity_Gym_EventScript_841B813::
	lockall
	setvar VAR_0x8008, 0xD
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 7,14
Kanto_VermilionCity_Gym_EventScript_841B81F::
	lockall
	setvar VAR_0x8008, 0xE
	goto Kanto_VermilionCity_Gym_EventScript_841B837

@ Kanto_VermilionCity_Gym sign / bg event at 9,14
Kanto_VermilionCity_Gym_EventScript_841B82B::
	lockall
	setvar VAR_0x8008, 0xF
	goto Kanto_VermilionCity_Gym_EventScript_841B837

Kanto_VermilionCity_Gym_EventScript_841B837::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop_737
	copyvar VAR_0x8004, VAR_TEMP_0
	copyvar VAR_0x8005, VAR_TEMP_1
	goto_if_set FLAG_QZ_0x213C, Kanto_VermilionCity_Gym_EventScript_841B8DB
	goto_if_set FLAG_TEMP_1, Kanto_VermilionCity_Gym_EventScript_841B894
	compare_var_to_var VAR_0x8004, VAR_0x8008
	goto_if_eq Kanto_VermilionCity_Gym_EventScript_841B87B
	msgbox Kanto_VermilionCity_Gym_Text_84452EB, MSGBOX_DEFAULT
	releaseall
	end

Kanto_VermilionCity_Gym_EventScript_841B87B::
	msgbox Kanto_VermilionCity_Gym_Text_844530A, MSGBOX_DEFAULT
	playse SE_PIN
	waitse
	call Kanto_VermilionCity_Gym_EventScript_841B6B0
	special DrawWholeMapView
	setflag FLAG_TEMP_1
	releaseall
	end

Kanto_VermilionCity_Gym_EventScript_841B894::
	compare_var_to_var VAR_0x8005, VAR_0x8008
	goto_if_eq Kanto_VermilionCity_Gym_EventScript_841B8C1
	msgbox Kanto_VermilionCity_Gym_Text_84453CE, MSGBOX_DEFAULT
	clearflag FLAG_TEMP_1
	special QzSetVermilionTrashCans_752
	copyvar VAR_TEMP_0, VAR_0x8004
	copyvar VAR_TEMP_1, VAR_0x8005
	call Kanto_VermilionCity_Gym_EventScript_841B8E5
	special DrawWholeMapView
	releaseall
	end

Kanto_VermilionCity_Gym_EventScript_841B8C1::
	msgbox Kanto_VermilionCity_Gym_Text_8445392, MSGBOX_DEFAULT
	closemessage
	call Kanto_VermilionCity_Gym_EventScript_841B70B
	special DrawWholeMapView
	playse SE_UNLOCK
	waitse
	setflag FLAG_QZ_0x213C
	releaseall
	end

Kanto_VermilionCity_Gym_EventScript_841B8DB::
	msgbox Kanto_VermilionCity_Gym_Text_84452EB, MSGBOX_DEFAULT
	releaseall
	end

Kanto_VermilionCity_Gym_EventScript_841B8E5::
	setmetatile 3, 6, 0x2A9, 1
	setmetatile 4, 6, 0x2AA, 1
	setmetatile 5, 6, 0x285, 1
	setmetatile 6, 6, 0x2AB, 1
	setmetatile 7, 6, 0x2AC, 1
	setmetatile 3, 7, 0x2B1, 1
	setmetatile 4, 7, 0x2B2, 1
	setmetatile 5, 7, 0x28D, 1
	setmetatile 6, 7, 0x2B3, 1
	setmetatile 7, 7, 0x2B4, 1
	return

@ Kanto_VermilionCity_Gym object 1 (local id 1, gfx 0x3F1A) at 5,2
Kanto_VermilionCity_Gym_EventScript_841B940::
	setvar VAR_0x8004, 4
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8608, 0, Kanto_VermilionCity_Gym_Text_8444BFB, Kanto_VermilionCity_Gym_Text_8444E61, Kanto_VermilionCity_Gym_EventScript_841B972

Kanto_VermilionCity_Gym_EventScript_841B95F::
	goto_if_unset FLAG_QZ_0x2171, Kanto_VermilionCity_Gym_EventScript_841B9B7
	msgbox Kanto_VermilionCity_Gym_Text_8444CE5, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_Gym_EventScript_841B972::
	goto_if_set FLAG_QZ_0x2102, Kanto_VermilionCity_Gym_EventScript_841B95F
	setvar VAR_0x8004, 4
	setvar VAR_0x8005, 1
	special QzQuestLogNop_737
	call_if_unset FLAG_QZ_0x2152, Kanto_VermilionCity_Gym_EventScript_841B9B3
	clearflag FLAG_QZ_0x2189
	setflag FLAG_QZ_0x2122
	call_if_unset FLAG_QZ_0x2102, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2102
	setvar VAR_0x8008, 3
	call Quetzal_EventScript_8456FD2
	goto Kanto_VermilionCity_Gym_EventScript_841B9B7

Kanto_VermilionCity_Gym_EventScript_841B9B3::
	clearflag FLAG_QZ_0x21D7
	return

Kanto_VermilionCity_Gym_EventScript_841B9B7::
	msgbox Kanto_VermilionCity_Gym_Text_8444D51, MSGBOX_DEFAULT
	checkitemspace ITEM_CHOICE_SCARF, 1
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_Gym_EventScript_841B9FF
	bufferitemname 0, ITEM_CHOICE_SCARF
	additem ITEM_CHOICE_SCARF, 1
	loadword 0, Kanto_VermilionCity_Gym_Text_8444DDE
	setorcopyvar VAR_0x8000, 0x1BC
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2171
	msgbox Kanto_VermilionCity_Gym_Text_8444DEE, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_Gym_EventScript_841B9FF::
	msgbox Kanto_VermilionCity_Gym_Text_8444E41, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity_Gym object 3 (local id 3, gfx 0x3F41) at 8,13
Kanto_VermilionCity_Gym_EventScript_841BA09::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8333, 0, Kanto_VermilionCity_Gym_Text_844501B, Kanto_VermilionCity_Gym_Text_844504F
	setvar VAR_0x8004, 4
	setvar VAR_0x8005, 4
	special QzQuestLogNop_737
	msgbox Kanto_VermilionCity_Gym_Text_8445062, MSGBOX_AUTOCLOSE
	end

@ Kanto_VermilionCity_Gym object 2 (local id 2, gfx 0x3EE6) at 2,11
Kanto_VermilionCity_Gym_EventScript_841BA2D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8412, 0, Kanto_VermilionCity_Gym_Text_8444F7D, Kanto_VermilionCity_Gym_Text_8444FCD
	msgbox Kanto_VermilionCity_Gym_Text_8444FD4, MSGBOX_AUTOCLOSE
	end

@ Kanto_VermilionCity_Gym object 5 (local id 5, gfx 0x3F09) at 7,8
Kanto_VermilionCity_Gym_EventScript_841BA44::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8615, 0, Kanto_VermilionCity_Gym_Text_8444EB5, Kanto_VermilionCity_Gym_Text_8444F03, Kanto_VermilionCity_Gym_EventScript_841BA6C
	setvar VAR_0x8004, 4
	setvar VAR_0x8005, 3
	special QzQuestLogNop_737
	msgbox Kanto_VermilionCity_Gym_Text_8444F1B, MSGBOX_AUTOCLOSE
	end

Kanto_VermilionCity_Gym_EventScript_841BA6C::
	setvar VAR_0x8004, 4
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	release
	end

@ Kanto_VermilionCity_Gym object 4 (local id 4, gfx 0x3F0B) at 4,17
Kanto_VermilionCity_Gym_EventScript_841BA7B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2122, Kanto_VermilionCity_Gym_EventScript_841BA90
	msgbox Kanto_VermilionCity_Gym_Text_8445112, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_Gym_EventScript_841BA90::
	msgbox Kanto_VermilionCity_Gym_Text_844524A, MSGBOX_DEFAULT
	release
	end

@ Kanto_VermilionCity_Gym sign / bg event at 3,17
@ Kanto_VermilionCity_Gym sign / bg event at 7,17
Kanto_VermilionCity_Gym_EventScript_841BA9A::
	lockall
	goto_if_set FLAG_QZ_0x2102, Kanto_VermilionCity_Gym_EventScript_841BAAE
	msgbox Kanto_VermilionCity_Gym_Text_8445269, MSGBOX_DEFAULT
	releaseall
	end

Kanto_VermilionCity_Gym_EventScript_841BAAE::
	msgbox Kanto_VermilionCity_Gym_Text_84452A8, MSGBOX_DEFAULT
	releaseall
	end

Kanto_VermilionCity_Gym_Text_8444BFB::
	.string "Hey, kid! What do you think you’re\n"
	.string "doing here?\p"
	.string "You won’t live long in combat!\n"
	.string "Not with your puny power!\p"
	.string "I tell you, kid, electric POKéMON\n"
	.string "saved me during the war!\p"
	.string "They zapped my enemies into\n"
	.string "paralysis!\p"
	.string "The same as I’ll do to you!{PLAY_BGM 0x022A}$"

Kanto_VermilionCity_Gym_Text_8444CE5::
	.string "A little word of advice, kid!\p"
	.string "Electricity is sure powerful!\p"
	.string "But, it’s useless against GROUND-\n"
	.string "type POKéMON!$"

Kanto_VermilionCity_Gym_Text_8444D51::
	.string "The THUNDERBADGE cranks up your\n"
	.string "POKéMON’s SPEED!\p"
	.string "It also lets your POKéMON FLY\n"
	.string "lightning-quick anytime, kid!\p"
	.string "You’re special, kid!\n"
	.string "Take this!$"

Kanto_VermilionCity_Gym_Text_8444DDE::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_VermilionCity_Gym_Text_8444DEE::
	.string "It boosts the holder’s Speed,\n"
	.string "but locks it into one move until\l"
	.string "you switch POKéMON.$"

Kanto_VermilionCity_Gym_Text_8444E41::
	.string "Yo, kid, make room in your BAG!$"

Kanto_VermilionCity_Gym_Text_8444E61::
	.string "Now that’s a shocker!\p"
	.string "You’re the real deal, kid!\p"
	.string "Fine, then, take the THUNDERBADGE!$"

Kanto_VermilionCity_Gym_Text_8444EB5::
	.string "When I was in the Army, LT. SURGE\n"
	.string "was my strict CO.\p"
	.string "He was a hard taskmaster.$"

Kanto_VermilionCity_Gym_Text_8444F03::
	.string "Stop!\n"
	.string "You’re very good!$"

Kanto_VermilionCity_Gym_Text_8444F1B::
	.string "It’s not easy opening that door.\p"
	.string "LT. SURGE was always famous for\n"
	.string "his cautious nature in the Army.$"

Kanto_VermilionCity_Gym_Text_8444F7D::
	.string "I’m a lightweight, but I’m good\n"
	.string "with electricity!\p"
	.string "That’s why I joined this GYM.$"

Kanto_VermilionCity_Gym_Text_8444FCD::
	.string "Fried!$"

Kanto_VermilionCity_Gym_Text_8444FD4::
	.string "Okay, I’ll talk!\p"
	.string "LT. SURGE said he hid door\n"
	.string "switches inside something.$"

Kanto_VermilionCity_Gym_Text_844501B::
	.string "This is no place for kids!\n"
	.string "Not even if you’re good!$"

Kanto_VermilionCity_Gym_Text_844504F::
	.string "Wow!\n"
	.string "Surprised me!$"

Kanto_VermilionCity_Gym_Text_8445062::
	.string "LT. SURGE installed the traps in\n"
	.string "the GYM himself.\p"
	.string "He set up double locks everywhere.\n"
	.string "Let me give you a hint.\p"
	.string "When you open the first lock, the\n"
	.string "second lock is right next to it.$"

Kanto_VermilionCity_Gym_Text_8445112::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "LT. SURGE has a nickname.\p"
	.string "People refer to him as the\n"
	.string "Lightning American!\p"
	.string "He’s an expert on electric\n"
	.string "POKéMON.\p"
	.string "BIRD/WATER-type POKéMON match\n"
	.string "poorly against the ELECTRIC type.\p"
	.string "Beware of paralysis, too.\p"
	.string "LT. SURGE is very cautious.\p"
	.string "He’s locked himself in, so it won’t\n"
	.string "be easy getting to him.$"

Kanto_VermilionCity_Gym_Text_844524A::
	.string "Whew!\n"
	.string "That match was electric!$"

Kanto_VermilionCity_Gym_Text_8445269::
	.string "VERMILION POKéMON GYM\n"
	.string "LEADER: LT. SURGE\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_VermilionCity_Gym_Text_84452A8::
	.string "VERMILION POKéMON GYM\n"
	.string "LEADER: LT. SURGE\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"

Kanto_VermilionCity_Gym_Text_84452EB::
	.string "Nope!\n"
	.string "There’s only trash here.$"

Kanto_VermilionCity_Gym_Text_844530A::
	.string "Hey! There’s a switch under the\n"
	.string "trash! Turn it on!\p"
	.string "The first electric lock opened!$"

Kanto_VermilionCity_Gym_Text_8445392::
	.string "The second electric lock opened!\n"
	.string "The motorized door opened!$"

Kanto_VermilionCity_Gym_Text_84453CE::
	.string "Nope!\n"
	.string "There’s only trash here.\p"
	.string "Hey!\n"
	.string "The electric locks were reset!$"
