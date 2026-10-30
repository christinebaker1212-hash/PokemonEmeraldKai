Kanto_ViridianCity_Gym_MapScripts::
	.byte 0

@ Kanto_ViridianCity_Gym object 8 (local id 8, gfx 0x3F0A) at 2,2
Kanto_ViridianCity_Gym_EventScript_8419EB8::
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8542, 0, Kanto_ViridianCity_Gym_Text_843F706, Kanto_ViridianCity_Gym_Text_843F7F3, Kanto_ViridianCity_Gym_EventScript_8419EEB

Kanto_ViridianCity_Gym_EventScript_8419ED7::
	goto_if_unset FLAG_QZ_0x216C, Kanto_ViridianCity_Gym_EventScript_8419F28
	msgbox Kanto_ViridianCity_Gym_Text_843F856, MSGBOX_DEFAULT
	closemessage
	release
	end

Kanto_ViridianCity_Gym_EventScript_8419EEB::
	goto_if_set FLAG_QZ_0x2107, Kanto_ViridianCity_Gym_EventScript_8419ED7
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	setflag FLAG_QZ_0x219E
	setflag FLAG_QZ_0x2120
	call_if_unset FLAG_QZ_0x2107, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2107
	setvar VAR_QZ_0x5125, 3
	setvar VAR_0x8008, 8
	call Quetzal_EventScript_8456FD2
	goto Kanto_ViridianCity_Gym_EventScript_8419F28

Kanto_ViridianCity_Gym_EventScript_8419F28::
	msgbox Kanto_ViridianCity_Gym_Text_843F93A, MSGBOX_DEFAULT
	checkitemspace ITEM_CHOICE_BAND, 1
	goto_if_eq VAR_RESULT, 0, Kanto_ViridianCity_Gym_EventScript_8419F70
	bufferitemname 0, ITEM_CHOICE_BAND
	additem ITEM_CHOICE_BAND, 1
	loadword 0, Kanto_ViridianCity_Gym_Text_843FA0F
	setorcopyvar VAR_0x8000, 0x1BA
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x216C
	msgbox Kanto_ViridianCity_Gym_Text_843FA1F, MSGBOX_DEFAULT
	release
	end

Kanto_ViridianCity_Gym_EventScript_8419F70::
	msgbox Kanto_ViridianCity_Gym_Text_843FA73, MSGBOX_DEFAULT
	release
	end

@ Kanto_ViridianCity_Gym object 4 (local id 4, gfx 0x3F1E) at 10,10
Kanto_ViridianCity_Gym_EventScript_8419F7A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8488, 0, Kanto_ViridianCity_Gym_Text_843FB4B, Kanto_ViridianCity_Gym_Text_843FB7C
	msgbox Kanto_ViridianCity_Gym_Text_843FB98, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 5 (local id 5, gfx 0x3F1E) at 2,21
Kanto_ViridianCity_Gym_EventScript_8419F91::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8489, 0, Kanto_ViridianCity_Gym_Text_843FCE2, Kanto_ViridianCity_Gym_Text_843FD13
	msgbox Kanto_ViridianCity_Gym_Text_843FD24, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 3 (local id 3, gfx 0x3EEB) at 11,14
Kanto_ViridianCity_Gym_EventScript_8419FA8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8514, 0, Kanto_ViridianCity_Gym_Text_843FB05, Kanto_ViridianCity_Gym_Text_843FB2E
	msgbox Kanto_ViridianCity_Gym_Text_843FB35, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 6 (local id 6, gfx 0x3EEB) at 3,11
Kanto_ViridianCity_Gym_EventScript_8419FBF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8515, 0, Kanto_ViridianCity_Gym_Text_843FBC4, Kanto_ViridianCity_Gym_Text_843FBF1
	msgbox Kanto_ViridianCity_Gym_Text_843FBF8, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 1 (local id 1, gfx 0x3EEB) at 10,2
Kanto_ViridianCity_Gym_EventScript_8419FD6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8516, 0, Kanto_ViridianCity_Gym_Text_843FC84, Kanto_ViridianCity_Gym_Text_843FCB2
	msgbox Kanto_ViridianCity_Gym_Text_843FCB8, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 7 (local id 7, gfx 0x3EFD) at 6,8
Kanto_ViridianCity_Gym_EventScript_8419FED::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8584, 0, Kanto_ViridianCity_Gym_Text_843FD3F, Kanto_ViridianCity_Gym_Text_843FD85
	msgbox Kanto_ViridianCity_Gym_Text_843FD93, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 2 (local id 2, gfx 0x3EFD) at 12,10
Kanto_ViridianCity_Gym_EventScript_841A004::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8592, 0, Kanto_ViridianCity_Gym_Text_843FA93, Kanto_ViridianCity_Gym_Text_843FAC1
	msgbox Kanto_ViridianCity_Gym_Text_843FAD3, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 9 (local id 9, gfx 0x3EFD) at 13,7
Kanto_ViridianCity_Gym_EventScript_841A01B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8593, 0, Kanto_ViridianCity_Gym_Text_843FC23, Kanto_ViridianCity_Gym_Text_843FC46
	msgbox Kanto_ViridianCity_Gym_Text_843FC56, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianCity_Gym object 10 (local id 10, gfx 0x3F0B) at 16,20
Kanto_ViridianCity_Gym_EventScript_841A032::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2120, Kanto_ViridianCity_Gym_EventScript_841A047
	msgbox Kanto_ViridianCity_Gym_Text_843FDD9, MSGBOX_DEFAULT
	release
	end

Kanto_ViridianCity_Gym_EventScript_841A047::
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	msgbox Kanto_ViridianCity_Gym_Text_843FEAE, MSGBOX_DEFAULT
	release
	end

@ Kanto_ViridianCity_Gym sign / bg event at 15,20
@ Kanto_ViridianCity_Gym sign / bg event at 19,20
Kanto_ViridianCity_Gym_EventScript_841A05E::
	lockall
	goto_if_set FLAG_QZ_0x2107, Kanto_ViridianCity_Gym_EventScript_841A072
	msgbox Kanto_ViridianCity_Gym_Text_843FEE5, MSGBOX_DEFAULT
	releaseall
	end

Kanto_ViridianCity_Gym_EventScript_841A072::
	msgbox Kanto_ViridianCity_Gym_Text_843FF1B, MSGBOX_DEFAULT
	releaseall
	end

Kanto_ViridianCity_Gym_Text_843F706::
	.string "Fwahahaha!\n"
	.string "Welcome to my hideout!\p"
	.string "It shall be so until I can restore\n"
	.string "TEAM ROCKET to its former glory.\p"
	.string "But, you have found me again.\n"
	.string "So be it.\l"
	.string "This time, I’m not holding back!\p"
	.string "Once more, you shall face\n"
	.string "GIOVANNI, the greatest TRAINER!{PLAY_BGM 0x01EF}$"

Kanto_ViridianCity_Gym_Text_843F7F3::
	.string "Ha!\n"
	.string "That was a truly intense fight.\l"
	.string "You have won!\p"
	.string "As proof, here is the EARTHBADGE!\n"
	.string "{PAUSE_MUSIC}{PLAY_BGM 0x0171}{PAUSE 0xFE}{PAUSE 0x56}{RESUME_MUSIC}$"

Kanto_ViridianCity_Gym_Text_843F856::
	.string "Having lost in this fashion, \n"
	.string "I can’t face my followers.\l"
	.string "I have betrayed their trust.\p"
	.string "As of today, TEAM ROCKET is\n"
	.string "finished forever!\p"
	.string "As for myself, I shall dedicate\n"
	.string "my life to training again.\p"
	.string "I will keep training here at my GYM.$"

Kanto_ViridianCity_Gym_Text_843F93A::
	.string "The EARTHBADGE makes POKéMON of\n"
	.string "any level obey without question.\p"
	.string "It is evidence of your mastery as\n"
	.string "a POKéMON TRAINER.\p"
	.string "With it, you can challenge the\n"
	.string "POKéMON LEAGUE.\p"
	.string "Also, take this gift for\n"
	.string "your LEAGUE challenge.$"

Kanto_ViridianCity_Gym_Text_843FA0F::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_ViridianCity_Gym_Text_843FA1F::
	.string "It boosts the holder’s Attack,\n"
	.string "but locks it into one move until\l"
	.string "you switch POKéMON.$"

Kanto_ViridianCity_Gym_Text_843FA73::
	.string "You do not have space for this!$"

Kanto_ViridianCity_Gym_Text_843FA93::
	.string "Heh!\n"
	.string "You must be running out of steam\l"
	.string "by now!$"

Kanto_ViridianCity_Gym_Text_843FAC1::
	.string "I ran out of gas!$"

Kanto_ViridianCity_Gym_Text_843FAD3::
	.string "You’ll need power to keep up with\n"
	.string "our GYM LEADER.$"

Kanto_ViridianCity_Gym_Text_843FB05::
	.string "Rrrroar!\n"
	.string "I’m working myself into a rage!$"

Kanto_ViridianCity_Gym_Text_843FB2E::
	.string "Wargh!$"

Kanto_ViridianCity_Gym_Text_843FB35::
	.string "I’m still not worthy!$"

Kanto_ViridianCity_Gym_Text_843FB4B::
	.string "POKéMON and I, we make wonderful\n"
	.string "music together!$"

Kanto_ViridianCity_Gym_Text_843FB7C::
	.string "You are in perfect harmony!$"

Kanto_ViridianCity_Gym_Text_843FB98::
	.string "Do you know the identity of our\n"
	.string "GYM LEADER?$"

Kanto_ViridianCity_Gym_Text_843FBC4::
	.string "Karate is the ultimate form of\n"
	.string "martial arts!$"

Kanto_ViridianCity_Gym_Text_843FBF1::
	.string "Aiyah!$"

Kanto_ViridianCity_Gym_Text_843FBF8::
	.string "If my POKéMON were as good at\n"
	.string "karate as I…$"

Kanto_ViridianCity_Gym_Text_843FC23::
	.string "The truly talented win with style.$"

Kanto_ViridianCity_Gym_Text_843FC46::
	.string "I lost my grip!$"

Kanto_ViridianCity_Gym_Text_843FC56::
	.string "The LEADER will scold me for\n"
	.string "losing this way…$"

Kanto_ViridianCity_Gym_Text_843FC84::
	.string "I’m the KARATE KING!\n"
	.string "Your fate rests with me!$"

Kanto_ViridianCity_Gym_Text_843FCB2::
	.string "Ayah!$"

Kanto_ViridianCity_Gym_Text_843FCB8::
	.string "The POKéMON LEAGUE?\n"
	.string "You? Don’t get cocky!$"

Kanto_ViridianCity_Gym_Text_843FCE2::
	.string "Your POKéMON will cower at the\n"
	.string "crack of my whip!$"

Kanto_ViridianCity_Gym_Text_843FD13::
	.string "Yowch!\n"
	.string "Whiplash!$"

Kanto_ViridianCity_Gym_Text_843FD24::
	.string "Wait!\n"
	.string "I was just careless!$"

Kanto_ViridianCity_Gym_Text_843FD3F::
	.string "VIRIDIAN GYM was closed for a\n"
	.string "long time.\p"
	.string "But now, our LEADER is back!$"

Kanto_ViridianCity_Gym_Text_843FD85::
	.string "I was beaten?$"

Kanto_ViridianCity_Gym_Text_843FD93::
	.string "You can go on to the POKéMON\n"
	.string "LEAGUE only by defeating our GYM\l"
	.string "LEADER!$"

Kanto_ViridianCity_Gym_Text_843FDD9::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "Even I don’t know the VIRIDIAN\n"
	.string "LEADER’s identity.\p"
	.string "But one thing’s certain.\n"
	.string "This will be the toughest of all\l"
	.string "the GYM LEADERS.\p"
	.string "Also, I heard that the TRAINERS\n"
	.string "here like GROUND-type POKéMON.$"

Kanto_ViridianCity_Gym_Text_843FEAE::
	.string "Blow me away! GIOVANNI was the\n"
	.string "GYM LEADER of VIRIDIAN?$"

Kanto_ViridianCity_Gym_Text_843FEE5::
	.string "VIRIDIAN POKéMON GYM\n"
	.string "LEADER: ?\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_ViridianCity_Gym_Text_843FF1B::
	.string "VIRIDIAN POKéMON GYM\n"
	.string "LEADER: GIOVANNI\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"
