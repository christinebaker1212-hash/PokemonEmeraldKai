Johto_Route44_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route44_EventScript_83F5641
	.byte 0

Johto_Route44_EventScript_83F5641::
	call Quetzal_EventScript_840EA62
	end

@ Johto_Route44 object 3 (local id 3, gfx 0x3E95) at 40,5
Johto_Route44_EventScript_83F5647::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4684, 0, Johto_Route44_Text_83F5721, Johto_Route44_Text_83F5766
	msgbox Johto_Route44_Text_83F577C, MSGBOX_AUTOCLOSE
	end

@ Johto_Route44 object 2 (local id 2, gfx 0x3E95) at 28,23
Johto_Route44_EventScript_83F565E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4175, 0, Johto_Route44_Text_83F57CB, Johto_Route44_Text_83F582D
	msgbox Johto_Route44_Text_83F5846, MSGBOX_AUTOCLOSE
	end

@ Johto_Route44 object 11 (local id 11, gfx 0x3EAE) at 17,20
Johto_Route44_EventScript_83F5675::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4663, 0, Johto_Route44_Text_83F5887, Johto_Route44_Text_83F58B5
	msgbox Johto_Route44_Text_83F58CD, MSGBOX_AUTOCLOSE
	end

@ Johto_Route44 object 1 (local id 1, gfx 0x25) at 11,5
Johto_Route44_EventScript_83F568C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4496, 0, Johto_Route44_Text_83F5941, Johto_Route44_Text_83F595A
	msgbox Johto_Route44_Text_83F5979, MSGBOX_AUTOCLOSE
	end

@ Johto_Route44 object 4 (local id 4, gfx 0x3EB7) at 49,3
Johto_Route44_EventScript_83F56A3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4686, 0, Johto_Route44_Text_83F59A9, Johto_Route44_Text_83F59DB
	msgbox Johto_Route44_Text_83F5A00, MSGBOX_AUTOCLOSE
	end

@ Johto_Route44 object 5 (local id 5, gfx 0x3E90) at 64,13
Johto_Route44_EventScript_83F56BA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4429, 0, Johto_Route44_Text_83F5A62, Johto_Route44_Text_83F5ACB
	msgbox Johto_Route44_Text_83F5AEE, MSGBOX_AUTOCLOSE
	end

@ Johto_Route44 object 6 (local id 6, gfx 0x3E8F) at 46,22
Johto_Route44_EventScript_83F56D1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4439, 0, Johto_Route44_Text_83F5B76, Johto_Route44_Text_83F5BB9
	msgbox Johto_Route44_Text_83F5BD5, MSGBOX_AUTOCLOSE
	end

@ Johto_Route44 sign / bg event at 66,13
Johto_Route44_EventScript_83F56E8::
	msgbox Johto_Route44_Text_83F5C3A, MSGBOX_SIGN
	end

@ Johto_Route44 sign / bg event at 5,17
Johto_Route44_EventScript_83F56F1::
	msgbox Johto_Route44_Text_83F5C52, MSGBOX_SIGN
	end

@ Johto_Route44 object 8 (local id 8, gfx 0x3A9E) at 69,18
Johto_Route44_EventScript_83F56FA::
	setorcopyvar VAR_0x8000, ITEM_MAX_REPEL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route44 object 9 (local id 9, gfx 0x3A9E) at 13,3
Johto_Route44_EventScript_83F5707::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route44 object 7 (local id 7, gfx 0x3A9E) at 41,12
Johto_Route44_EventScript_83F5714::
	setorcopyvar VAR_0x8000, ITEM_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route44_Text_83F5721::
	.string "Aack!\p"
	.string "You made me lose a POLIWAG!\n"
	.string "What are you going to do about it?$"

Johto_Route44_Text_83F5766::
	.string "Just forget about it.$"

Johto_Route44_Text_83F577C::
	.string "That POLIWAG that got away…\n"
	.string "it was huge.\p"
	.string "I swear it must’ve been\n"
	.string "16 feet long!$"

Johto_Route44_Text_83F57CB::
	.string "I fish until I can’t anymore.\p"
	.string "I also battle until I drop.\p"
	.string "That’s my relationship with my\n"
	.string "POKéMON.$"

Johto_Route44_Text_83F582D::
	.string "Hmmmm… This isn’t right.$"

Johto_Route44_Text_83F5846::
	.string "That’s 100 wins to 101 losses.\p"
	.string "I won’t battle again for a while.$"

Johto_Route44_Text_83F5887::
	.string "Do you know about the legendary\n"
	.string "bird POKéMON?$"

Johto_Route44_Text_83F58B5::
	.string "Whew! You’re hot stuff.$"

Johto_Route44_Text_83F58CD::
	.string "ARTICUNO, ZAPDOS and MOLTRES are\n"
	.string "the three legendary bird POKéMON.\p"
	.string "I heard there are other legendary\n"
	.string "birds, though.$"

Johto_Route44_Text_83F5941::
	.string "I’m gonna win, for sure!$"

Johto_Route44_Text_83F595A::
	.string "Arrgh… That’s a shocking loss…$"

Johto_Route44_Text_83F5979::
	.string "It’s important to have conviction\n"
	.string "on your side.$"

Johto_Route44_Text_83F59A9::
	.string "I’ll do anything to get my hands\n"
	.string "on rare POKéMON!$"

Johto_Route44_Text_83F59DB::
	.string "Oooh, your POKéMON are so\n"
	.string "appealing.$"

Johto_Route44_Text_83F5A00::
	.string "If a POKéMON has different colors\n"
	.string "from usual, it’s more valuable.\p"
	.string "What? You mean that’s not true?$"

Johto_Route44_Text_83F5A62::
	.string "I can tell you’re a good trainer\n"
	.string "by looking at you.\p"
	.string "I’m good at POKéMON, and I can\n"
	.string "see that you are too.$"

Johto_Route44_Text_83F5ACB::
	.string "Tch! It’s a total loss on my part.$"

Johto_Route44_Text_83F5AEE::
	.string "Wow. You have a lot of BADGES.\p"
	.string "No wonder you’re so tough.\p"
	.string "I wonder if ECRUTEAK GYM’s MORTY\n"
	.string "is still in training?\p"
	.string "He really hammered me.$"

Johto_Route44_Text_83F5B76::
	.string "You look strong.\p"
	.string "Good trainers seek tough\n"
	.string "opponents instinctively.$"

Johto_Route44_Text_83F5BB9::
	.string "Nope! This won’t do at all.$"

Johto_Route44_Text_83F5BD5::
	.string "We all get better by experiencing\n"
	.string "many battles.\p"
	.string "I battled a lot to become what I\n"
	.string "am today--an elite.$"

Johto_Route44_Text_83F5C3A::
	.string "ROUTE 44\n"
	.string "ICE PATH AHEAD$"

Johto_Route44_Text_83F5C52::
	.string "ROUTE 44\p"
	.string "MAHOGANY TOWN - BLACKTHORN CITY$"
