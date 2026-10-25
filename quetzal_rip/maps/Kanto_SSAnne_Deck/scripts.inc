Kanto_SSAnne_Deck_MapScripts::
	.byte 0

@ Kanto_SSAnne_Deck object 5 (local id 5, gfx 0x3F5C) at 11,5
Kanto_SSAnne_Deck_EventScript_84105DF::
	msgbox Kanto_SSAnne_Deck_Text_842385A, MSGBOX_NPC
	end

@ Kanto_SSAnne_Deck object 2 (local id 2, gfx 0x3EE6) at 14,11
Kanto_SSAnne_Deck_EventScript_84105E8::
	msgbox Kanto_SSAnne_Deck_Text_84238B1, MSGBOX_NPC
	end

@ Kanto_SSAnne_Deck object 3 (local id 3, gfx 0x3F41) at 13,13
Kanto_SSAnne_Deck_EventScript_84105F1::
	msgbox Kanto_SSAnne_Deck_Text_842388D, MSGBOX_NPC
	end

@ Kanto_SSAnne_Deck object 4 (local id 4, gfx 0x3F41) at 6,9
Kanto_SSAnne_Deck_EventScript_84105FA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8326, 0, Kanto_SSAnne_Deck_Text_84238F3, Kanto_SSAnne_Deck_Text_8423916
	msgbox Kanto_SSAnne_Deck_Text_8423929, MSGBOX_AUTOCLOSE
	end

@ Kanto_SSAnne_Deck object 1 (local id 1, gfx 0x3F41) at 12,10
Kanto_SSAnne_Deck_EventScript_8410611::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8327, 0, Kanto_SSAnne_Deck_Text_842396D, Kanto_SSAnne_Deck_Text_842398B
	msgbox Kanto_SSAnne_Deck_Text_84239A0, MSGBOX_AUTOCLOSE
	end

Kanto_SSAnne_Deck_Text_842385A::
	.string "The party’s over.\n"
	.string "The ship will be departing soon.$"

Kanto_SSAnne_Deck_Text_842388D::
	.string "Whew!\n"
	.string "Scrubbing decks is hard work!$"

Kanto_SSAnne_Deck_Text_84238B1::
	.string "Urf… I feel ill…\p"
	.string "I got seasick, so I stepped out to\n"
	.string "get some air…$"

Kanto_SSAnne_Deck_Text_84238F3::
	.string "Hey, matey!\p"
	.string "Let’s do a little jig!$"

Kanto_SSAnne_Deck_Text_8423916::
	.string "You’re impressive!$"

Kanto_SSAnne_Deck_Text_8423929::
	.string "How many kinds of POKéMON do you\n"
	.string "think there are in this big world?$"

Kanto_SSAnne_Deck_Text_842396D::
	.string "Ahoy, there!\n"
	.string "Are you seasick?$"

Kanto_SSAnne_Deck_Text_842398B::
	.string "I was just careless!$"

Kanto_SSAnne_Deck_Text_84239A0::
	.string "My pa said there are 100 kinds of\n"
	.string "POKéMON. I think there are more.$"
