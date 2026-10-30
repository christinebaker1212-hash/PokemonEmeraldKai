Kanto_Route3_MapScripts::
	.byte 0

@ Kanto_Route3 object 1 (local id 1, gfx 0x3F5C) at 70,13
Kanto_Route3_EventScript_8417C25::
	msgbox Kanto_Route3_Text_8433758, MSGBOX_NPC
	end

@ Kanto_Route3 sign / bg event at 72,11
Kanto_Route3_EventScript_8417C2E::
	msgbox Kanto_Route3_Text_8433B87, MSGBOX_SIGN
	end

@ Kanto_Route3 object 9 (local id 9, gfx 0x3F5C) at 17,4
Kanto_Route3_EventScript_845979A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8281, 0, Kanto_Route3_Text_8433827, Kanto_Route3_Text_8433867
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_84597C1
	msgbox Kanto_Route3_Text_843387B, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_84597C1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8281, 0, Kanto_Route3_Text_846E943, Kanto_Route3_Text_8433867
	msgbox Kanto_Route3_Text_843387B, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route3 object 6 (local id 6, gfx 0x3F5C) at 29,10
Kanto_Route3_EventScript_84597D8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8282, 0, Kanto_Route3_Text_8433A4D, Kanto_Route3_Text_8433A84
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_84597FF
	msgbox Kanto_Route3_Text_8433A96, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_84597FF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8282, 0, Kanto_Route3_Text_846EA2E, Kanto_Route3_Text_8433A84
	msgbox Kanto_Route3_Text_8433A96, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route3 object 8 (local id 8, gfx 0x3EF1) at 12,6
Kanto_Route3_EventScript_8459816::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8297, 0, Kanto_Route3_Text_84337AD, Kanto_Route3_Text_84337D0
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_845983D
	msgbox Kanto_Route3_Text_84337E3, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_845983D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8297, 0, Kanto_Route3_Text_846E920, Kanto_Route3_Text_84337D0
	msgbox Kanto_Route3_Text_84337E3, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route3 object 5 (local id 5, gfx 0x3EF1) at 25,4
Kanto_Route3_EventScript_8459854::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8298, 0, Kanto_Route3_Text_8433949, Kanto_Route3_Text_843397A
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_845987B
	msgbox Kanto_Route3_Text_84339A0, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_845987B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8298, 0, Kanto_Route3_Text_846E9C7, Kanto_Route3_Text_843397A
	msgbox Kanto_Route3_Text_84339A0, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route3 object 3 (local id 3, gfx 0x3EF1) at 32,6
Kanto_Route3_EventScript_8459892::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8299, 0, Kanto_Route3_Text_8433ACE, Kanto_Route3_Text_8433AFE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_84598B9
	msgbox Kanto_Route3_Text_8433B10, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_84598B9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8299, 0, Kanto_Route3_Text_846EA6B, Kanto_Route3_Text_8433AFE
	msgbox Kanto_Route3_Text_8433B10, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route3 object 7 (local id 7, gfx 0x3F16) at 19,9
Kanto_Route3_EventScript_84598D0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8308, 0, Kanto_Route3_Text_84338DD, Kanto_Route3_Text_8433906
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_84598F7
	msgbox Kanto_Route3_Text_8433913, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_84598F7::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8308, 0, Kanto_Route3_Text_846E999, Kanto_Route3_Text_8433906
	msgbox Kanto_Route3_Text_8433913, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route3 object 4 (local id 4, gfx 0x3F16) at 30,3
Kanto_Route3_EventScript_845990E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8309, 0, Kanto_Route3_Text_84339E2, Kanto_Route3_Text_8433A0D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_8459935
	msgbox Kanto_Route3_Text_8433A16, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_8459935::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8309, 0, Kanto_Route3_Text_846EA03, Kanto_Route3_Text_8433A0D
	msgbox Kanto_Route3_Text_8433A16, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route3 object 2 (local id 2, gfx 0x3F16) at 40,11
Kanto_Route3_EventScript_845994C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8310, 0, Kanto_Route3_Text_8433B41, Kanto_Route3_Text_8433B58
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route3_EventScript_8459973
	msgbox Kanto_Route3_Text_8433B63, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_EventScript_8459973::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8310, 0, Kanto_Route3_Text_846EA9F, Kanto_Route3_Text_8433B58
	msgbox Kanto_Route3_Text_8433B63, MSGBOX_AUTOCLOSE
	end

Kanto_Route3_Text_8433758::
	.string "Whew… I better take a rest…\n"
	.string "Groan…\p"
	.string "That tunnel from CERULEAN takes a\n"
	.string "lot out of you!$"

Kanto_Route3_Text_84337AD::
	.string "Hey!\n"
	.string "I saw you in VIRIDIAN FOREST!$"

Kanto_Route3_Text_84337D0::
	.string "You beat me again!$"

Kanto_Route3_Text_84337E3::
	.string "There are other kinds of POKéMON\n"
	.string "than the ones you find in forests.$"

Kanto_Route3_Text_8433827::
	.string "Hi!\n"
	.string "I like shorts!\p"
	.string "They’re delightfully comfy and\n"
	.string "easy to wear!$"

Kanto_Route3_Text_8433867::
	.string "I don’t believe it!$"

Kanto_Route3_Text_843387B::
	.string "Are you using a POKéMON CENTER’s\n"
	.string "PC for storing your POKéMON?\p"
	.string "Each BOX can hold up to\n"
	.string "30 POKéMON.$"

Kanto_Route3_Text_84338DD::
	.string "Excuse me!\n"
	.string "You looked at me, didn’t you?$"

Kanto_Route3_Text_8433906::
	.string "You’re mean!$"

Kanto_Route3_Text_8433913::
	.string "You shouldn’t be staring if you\n"
	.string "don’t want to battle!$"

Kanto_Route3_Text_8433949::
	.string "Are you a TRAINER?\n"
	.string "Let’s get with it right away!$"

Kanto_Route3_Text_843397A::
	.string "If I had new POKéMON, I would’ve\n"
	.string "won!$"

Kanto_Route3_Text_84339A0::
	.string "If a POKéMON BOX on the PC gets\n"
	.string "full, just switch to another BOX.$"

Kanto_Route3_Text_84339E2::
	.string "That look you gave me…\n"
	.string "It’s so intriguing!$"

Kanto_Route3_Text_8433A0D::
	.string "Be nice!$"

Kanto_Route3_Text_8433A16::
	.string "You can avoid battles by not\n"
	.string "letting TRAINERS see you.$"

Kanto_Route3_Text_8433A4D::
	.string "Hey! You’re not wearing shorts!\n"
	.string "What’s wrong with you?$"

Kanto_Route3_Text_8433A84::
	.string "Lost!\n"
	.string "Lost! Lost!$"

Kanto_Route3_Text_8433A96::
	.string "I always wear shorts, even in\n"
	.string "winter. That’s my policy.$"

Kanto_Route3_Text_8433ACE::
	.string "I’ll battle you with the POKéMON\n"
	.string "I just caught.$"

Kanto_Route3_Text_8433AFE::
	.string "Done like dinner!$"

Kanto_Route3_Text_8433B10::
	.string "Trained POKéMON are stronger than\n"
	.string "the wild ones.$"

Kanto_Route3_Text_8433B41::
	.string "Eek!\n"
	.string "Did you touch me?$"

Kanto_Route3_Text_8433B58::
	.string "That’s it?$"

Kanto_Route3_Text_8433B63::
	.string "ROUTE 4 is at the foot of\n"
	.string "MT. MOON.$"

Kanto_Route3_Text_8433B87::
	.string "ROUTE 3\n"
	.string "MT. MOON AHEAD$"

Kanto_Route3_Text_846E920::
	.string "Hey!\n"
	.string "I saw you in VIRIDIAN FOREST!$"

Kanto_Route3_Text_846E943::
	.string "Hi! I like shorts!\n"
	.string "They’re comfy and easy to wear!\p"
	.string "You should be wearing shorts, too!$"

Kanto_Route3_Text_846E999::
	.string "Excuse me!\n"
	.string "You keep looking at me, don’t you?$"

Kanto_Route3_Text_846E9C7::
	.string "You’re a TRAINER, aren’t you?\n"
	.string "Let’s get with it right away!$"

Kanto_Route3_Text_846EA03::
	.string "That look you give me…\n"
	.string "It intrigues me so!$"

Kanto_Route3_Text_846EA2E::
	.string "Hey! What’s wrong with you?\n"
	.string "You’re still not wearing shorts!$"

Kanto_Route3_Text_846EA6B::
	.string "I’ll battle you with the POKéMON\n"
	.string "I started raising.$"

Kanto_Route3_Text_846EA9F::
	.string "Eek!\n"
	.string "Did you push me?$"
