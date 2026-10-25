Kanto_SSAnne_2F_Room2_MapScripts::
	.byte 0

@ Kanto_SSAnne_2F_Room2 object 1 (local id 1, gfx 0x3F06) at 5,5
Kanto_SSAnne_2F_Room2_EventScript_841082A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8415, 0, Kanto_SSAnne_2F_Room2_Text_8424390, Kanto_SSAnne_2F_Room2_Text_84243AC
	msgbox Kanto_SSAnne_2F_Room2_Text_84243B9, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_2F_Room2 object 2 (local id 2, gfx 0x3F09) at 1,4
Kanto_SSAnne_2F_Room2_EventScript_8410841::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8674, 0, Kanto_SSAnne_2F_Room2_Text_8424318, Kanto_SSAnne_2F_Room2_Text_8424347
	msgbox Kanto_SSAnne_2F_Room2_Text_842436B, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_2F_Room2 object 3 (local id 3, gfx 0x3F0E) at 3,3
Kanto_SSAnne_2F_Room2_EventScript_846BB21::
	setorcopyvar VAR_0x8000, 0x8F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SSAnne_2F_Room2_Text_8424318::
	.string "Competing against the young keeps\n"
	.string "me youthful.$"

Kanto_SSAnne_2F_Room2_Text_8424347::
	.string "Good match!\n"
	.string "Ah, I feel young again!$"

Kanto_SSAnne_2F_Room2_Text_842436B::
	.string "Fifteen years ago, I would\n"
	.string "have won!$"

Kanto_SSAnne_2F_Room2_Text_8424390::
	.string "Check out what I fished up!$"

Kanto_SSAnne_2F_Room2_Text_84243AC::
	.string "I’m all out!$"

Kanto_SSAnne_2F_Room2_Text_84243B9::
	.string "Party?\p"
	.string "The cruise ship’s party should be\n"
	.string "over by now.$"
