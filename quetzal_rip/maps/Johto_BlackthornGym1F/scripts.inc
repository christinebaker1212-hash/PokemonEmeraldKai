Johto_BlackthornGym1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Johto_BlackthornGym1F_EventScript_83C6F31
	.byte 0

Johto_BlackthornGym1F_EventScript_83C6F31::
	call_if_eq VAR_QZ_0x5027, 1, Johto_BlackthornGym1F_EventScript_83C6F5E
	call_if_eq VAR_QZ_0x5027, 2, Johto_BlackthornGym1F_EventScript_83C6F64
	call_if_eq VAR_QZ_0x5027, 3, Johto_BlackthornGym1F_EventScript_83C6F6F
	call_if_eq VAR_QZ_0x5027, 4, Johto_BlackthornGym1F_EventScript_83C6F7F
	end

Johto_BlackthornGym1F_EventScript_83C6F5E::
	call Quetzal_EventScript_83C6FA5
	end

Johto_BlackthornGym1F_EventScript_83C6F64::
	call Quetzal_EventScript_83C6FA5
	call Quetzal_EventScript_83C708F
	end

Johto_BlackthornGym1F_EventScript_83C6F6F::
	call Quetzal_EventScript_83C6FA5
	call Quetzal_EventScript_83C708F
	call Quetzal_EventScript_83C71AF
	end

Johto_BlackthornGym1F_EventScript_83C6F7F::
	call Quetzal_EventScript_83C6FA5
	call Quetzal_EventScript_83C708F
	call Quetzal_EventScript_83C71AF
	call Quetzal_EventScript_83C72BD
	end

@ Johto_BlackthornGym1F trigger at 19,55 (VAR_QZ_0x5027 == 0)
@ Johto_BlackthornGym1F trigger at 20,55 (VAR_QZ_0x5027 == 0)
@ Johto_BlackthornGym1F trigger at 21,55 (VAR_QZ_0x5027 == 0)
Johto_BlackthornGym1F_EventScript_83C6F94::
	call Quetzal_EventScript_83C6FA5
	playse SE_BANG
	special DrawWholeMapView
	setvar VAR_QZ_0x5027, 1
	end

@ Johto_BlackthornGym1F trigger at 17,41 (VAR_QZ_0x5027 == 1)
@ Johto_BlackthornGym1F trigger at 18,41 (VAR_QZ_0x5027 == 1)
@ Johto_BlackthornGym1F trigger at 19,41 (VAR_QZ_0x5027 == 1)
Johto_BlackthornGym1F_EventScript_83C707E::
	call Quetzal_EventScript_83C708F
	playse SE_BANG
	special DrawWholeMapView
	setvar VAR_QZ_0x5027, 2
	end

@ Johto_BlackthornGym1F trigger at 25,26 (VAR_QZ_0x5027 == 2)
@ Johto_BlackthornGym1F trigger at 25,27 (VAR_QZ_0x5027 == 2)
@ Johto_BlackthornGym1F trigger at 25,28 (VAR_QZ_0x5027 == 2)
Johto_BlackthornGym1F_EventScript_83C719E::
	call Quetzal_EventScript_83C71AF
	playse SE_BANG
	special DrawWholeMapView
	setvar VAR_QZ_0x5027, 3
	end

@ Johto_BlackthornGym1F trigger at 19,16 (VAR_QZ_0x5027 == 3)
@ Johto_BlackthornGym1F trigger at 20,16 (VAR_QZ_0x5027 == 3)
@ Johto_BlackthornGym1F trigger at 21,16 (VAR_QZ_0x5027 == 3)
Johto_BlackthornGym1F_EventScript_83C72AC::
	call Quetzal_EventScript_83C72BD
	playse SE_BANG
	special DrawWholeMapView
	setvar VAR_QZ_0x5027, 4
	end

@ Johto_BlackthornGym1F object 7 (local id 7, gfx 0x3E90) at 30,30
Johto_BlackthornGym1F_EventScript_83C7345::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4154, 0, Johto_BlackthornGym1F_Text_83C79FB, Johto_BlackthornGym1F_Text_83C7A2A
	msgbox Johto_BlackthornGym1F_Text_83C7A4B, MSGBOX_AUTOCLOSE
	end

@ Johto_BlackthornGym1F object 6 (local id 6, gfx 0x3E8F) at 21,42
Johto_BlackthornGym1F_EventScript_83C735C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4425, 0, Johto_BlackthornGym1F_Text_83C7AA3, Johto_BlackthornGym1F_Text_83C7AEC
	msgbox Johto_BlackthornGym1F_Text_83C7AFA, MSGBOX_AUTOCLOSE
	end

@ Johto_BlackthornGym1F object 2 (local id 2, gfx 0x3E90) at 15,42
Johto_BlackthornGym1F_EventScript_83C7373::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4371, 0, Johto_BlackthornGym1F_Text_83C7B1B, Johto_BlackthornGym1F_Text_83C7B60
	msgbox Johto_BlackthornGym1F_Text_83C7B78, MSGBOX_AUTOCLOSE
	end

@ Johto_BlackthornGym1F object 1 (local id 1, gfx 0x3E90) at 18,17
Johto_BlackthornGym1F_EventScript_83C738A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4588, 0, Johto_BlackthornGym1F_Text_83C7BB6, Johto_BlackthornGym1F_Text_83C7BE1
	msgbox Johto_BlackthornGym1F_Text_83C7BED, MSGBOX_AUTOCLOSE
	end

@ Johto_BlackthornGym1F object 3 (local id 3, gfx 0x3E8F) at 22,17
Johto_BlackthornGym1F_EventScript_83C73A1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4608, 0, Johto_BlackthornGym1F_Text_83C7C21, Johto_BlackthornGym1F_Text_83C7C92
	msgbox Johto_BlackthornGym1F_Text_83C7C9D, MSGBOX_AUTOCLOSE
	end

@ Johto_BlackthornGym1F object 4 (local id 4, gfx 0x3E8D) at 20,5
Johto_BlackthornGym1F_EventScript_83C73B8::
	lock
	faceplayer
	call Quetzal_EventScript_83CC300
	goto_if_set FLAG_QZ_0x11B9, Johto_BlackthornGym1F_EventScript_83C7453
	goto_if_set FLAG_QZ_0x11CA, Johto_BlackthornGym1F_EventScript_83C7448
	goto_if_ge VAR_QZ_0x5010, 4, Johto_BlackthornGym1F_EventScript_83C743D
	goto_if_eq VAR_QZ_0x5010, 2, Johto_BlackthornGym1F_EventScript_83C7432
	msgbox Johto_BlackthornGym1F_Text_83C749A, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_BlackthornGym1F_EventScript_83C740E
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4637, 0, Johto_BlackthornGym1F_Text_83C757B
	goto Johto_BlackthornGym1F_EventScript_83C7418

Johto_BlackthornGym1F_EventScript_83C740E::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4976, 0, Johto_BlackthornGym1F_Text_83C757B

Johto_BlackthornGym1F_EventScript_83C7418::
	msgbox Johto_BlackthornGym1F_Text_83C75B3, MSGBOX_DEFAULT
	setvar VAR_0x8008, 8
	call Quetzal_EventScript_83601F0
	setvar VAR_QZ_0x5010, 2
	closemessage
	release
	end

Johto_BlackthornGym1F_EventScript_83C7432::
	msgbox Johto_BlackthornGym1F_Text_83C770D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornGym1F_EventScript_83C743D::
	msgbox Johto_BlackthornGym1F_Text_83C77A6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornGym1F_EventScript_83C7448::
	msgbox Johto_BlackthornGym1F_Text_83C78D1, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornGym1F_EventScript_83C7453::
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_BlackthornGym1F_EventScript_83C747A
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4637, 0, Johto_BlackthornGym1F_Text_83C749A, Johto_BlackthornGym1F_Text_83C757B, Johto_BlackthornGym1F_EventScript_83C748C
	goto Johto_BlackthornGym1F_EventScript_83C748C

Johto_BlackthornGym1F_EventScript_83C747A::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4976, 0, Johto_BlackthornGym1F_Text_83C749A, Johto_BlackthornGym1F_Text_83C757B, Johto_BlackthornGym1F_EventScript_83C748C

Johto_BlackthornGym1F_EventScript_83C748C::
	goto_if_set FLAG_QZ_0x11CA, Johto_BlackthornGym1F_EventScript_83C7448
	goto Johto_BlackthornGym1F_EventScript_83C743D

@ Johto_BlackthornGym1F object 5 (local id 5, gfx 0x3E98) at 22,55
Johto_BlackthornGym1F_EventScript_83C7CCA::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AF, Johto_BlackthornGym1F_EventScript_83C7CE0
	msgbox Johto_BlackthornGym1F_Text_83C7CEA, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornGym1F_EventScript_83C7CE0::
	msgbox Johto_BlackthornGym1F_Text_83C7E64, MSGBOX_DEFAULT
	release
	end

@ Johto_BlackthornGym1F sign / bg event at 17,55
@ Johto_BlackthornGym1F sign / bg event at 23,55
Johto_BlackthornGym1F_EventScript_83C7EE7::
	lock
	goto_if_set FLAG_QZ_0x11AF, Johto_BlackthornGym1F_EventScript_83C7EFC
	msgbox Johto_BlackthornGym1F_Text_83C7F07, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornGym1F_EventScript_83C7EFC::
	msgbox Johto_BlackthornGym1F_Text_83C7F23, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BlackthornGym1F_Text_83C749A::
	.string "I am CLAIR.\p"
	.string "The world’s best dragon master.\p"
	.string "I can hold my own against even the\n"
	.string "POKéMON LEAGUE’s ELITE FOUR.\p"
	.string "Do you still want to take me on?\p"
	.string "…Fine. Let’s do it!\p"
	.string "As a GYM LEADER, I will use my full\n"
	.string "power against any opponent!$"

Johto_BlackthornGym1F_Text_83C757B::
	.string "I lost?\p"
	.string "I don’t believe it. There must be\n"
	.string "some mistake…$"

Johto_BlackthornGym1F_Text_83C75B3::
	.string "I won’t admit this.\p"
	.string "I may have lost, but you’re still\n"
	.string "not ready for the POKéMON LEAGUE.\p"
	.string "I know. You should take the dragon-\n"
	.string "user challenge.\p"
	.string "Behind this GYM is a place called\n"
	.string "DRAGON’S DEN.\p"
	.string "There is a small shrine at its\n"
	.string "center. Go there.\p"
	.string "If you can prove that you’ve lost\n"
	.string "your lazy ideals, I will recognize\l"
	.string "you as a trainer worthy of a GYM\l"
	.string "BADGE!$"

Johto_BlackthornGym1F_Text_83C770D::
	.string "What’s the matter?\p"
	.string "Is it too much to expect of you?$"

Johto_BlackthornGym1F_Text_83C77A6::
	.string "What’s the matter?\p"
	.string "Aren’t you headed to the POKéMON\n"
	.string "LEAGUE?\p"
	.string "Don’t you know where it is?\p"
	.string "From here, go to NEW BARK TOWN.\p"
	.string "Then SURF east. The route there is\n"
	.string "very tough.\p"
	.string "Don’t you dare lose at the POKéMON\n"
	.string "LEAGUE!\p"
	.string "If you do, I’ll feel even worse\n"
	.string "about having lost to you!\p"
	.string "Give it everything you’ve got.$"

Johto_BlackthornGym1F_Text_83C78D1::
	.string "You did it, huh?\p"
	.string "You are now the JOHTO CHAMP.$"

Johto_BlackthornGym1F_Text_83C79FB::
	.string "I just got access to use DRAGON-\n"
	.string "type POKéMON!$"

Johto_BlackthornGym1F_Text_83C7A2A::
	.string "Rats! If only I had a DRAGONITE!$"

Johto_BlackthornGym1F_Text_83C7A4B::
	.string "Members of our dragon-user clan\n"
	.string "can use dragon POKéMON only after\l"
	.string "our MASTER allows it.$"

Johto_BlackthornGym1F_Text_83C7AA3::
	.string "I can’t allow a nameless trainer\n"
	.string "past me!\p"
	.string "CLAIR would be livid if I did!$"

Johto_BlackthornGym1F_Text_83C7AEC::
	.string "Awww… I lost…$"

Johto_BlackthornGym1F_Text_83C7AFA::
	.string "Uh-oh… CLAIR is going to be mad…$"

Johto_BlackthornGym1F_Text_83C7B1B::
	.string "Your first battle against dragons?\p"
	.string "I’ll show you how tough they are!$"

Johto_BlackthornGym1F_Text_83C7B60::
	.string "My dragon POKéMON lost?$"

Johto_BlackthornGym1F_Text_83C7B78::
	.string "LANCE told you that he’d like to\n"
	.string "see you again?\p"
	.string "Not a chance!$"

Johto_BlackthornGym1F_Text_83C7BB6::
	.string "My chance of losing? Not even one\n"
	.string "percent!$"

Johto_BlackthornGym1F_Text_83C7BE1::
	.string "That’s odd.$"

Johto_BlackthornGym1F_Text_83C7BED::
	.string "I know my short-comings now.\p"
	.string "Thanks for showing me!$"

Johto_BlackthornGym1F_Text_83C7C21::
	.string "Dragons are sacred POKéMON.\p"
	.string "They are full of life energy.\p"
	.string "If you’re not serious, you won’t\n"
	.string "be able to beat them.$"

Johto_BlackthornGym1F_Text_83C7C92::
	.string "Way to go!$"

Johto_BlackthornGym1F_Text_83C7C9D::
	.string "Dragons are weak against DRAGON-\n"
	.string "type moves.$"

Johto_BlackthornGym1F_Text_83C7CEA::
	.string "Yo! CHAMP in making!\p"
	.string "It’s been a long journey, but we\n"
	.string "are almost done! Count on me!\p"
	.string "While CLAIR looks up to the Dragon\n"
	.string "Master, LANCE, she’s managed to\l"
	.string "create her own identity with a\l"
	.string "team consisting of dragons from\l"
	.string "the sea.\p"
	.string "Dragons are supposed to be\n"
	.string "weak against ICE-type moves,\l"
	.string "but you’ll need some DRAGON-type\l"
	.string "or FAIRY-type moves of your own if\l"
	.string "you want to win.$"

Johto_BlackthornGym1F_Text_83C7E64::
	.string "You were great to beat CLAIR!\p"
	.string "All that’s left is the POKéMON\n"
	.string "LEAGUE challenge.\p"
	.string "You’re on the way to becoming the\n"
	.string "POKéMON CHAMPION!$"

Johto_BlackthornGym1F_Text_83C7F07::
	.string "BLACKTHORN CITY POKéMON GYM$"

Johto_BlackthornGym1F_Text_83C7F23::
	.string "BLACKTHORN CITY POKéMON GYM\p"
	.string "CLAIR’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"
