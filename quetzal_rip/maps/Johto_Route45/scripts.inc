Johto_Route45_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_Route45 object 1 (local id 1, gfx 0x37) at 8,10
Johto_Route45_EventScript_83F5C81::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4198, 0, Johto_Route45_Text_83F5D76, Johto_Route45_Text_83F5DC6
	msgbox Johto_Route45_Text_83F5DD7, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 object 4 (local id 4, gfx 0x37) at 25,28
Johto_Route45_EventScript_83F5C98::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4445, 0, Johto_Route45_Text_83F5E14, Johto_Route45_Text_83F5E6A
	msgbox Johto_Route45_Text_83F5E8C, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 object 11 (local id 11, gfx 0x37) at 29,61
Johto_Route45_EventScript_83F5CAF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4929, 0, Johto_Route45_Text_83F5EDC, Johto_Route45_Text_83F5EF9
	msgbox Johto_Route45_Text_83F5F15, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 object 9 (local id 9, gfx 0x37) at 33,64
Johto_Route45_EventScript_83F5CC6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4121, 0, Johto_Route45_Text_83F5F5B, Johto_Route45_Text_83F5FBB
	msgbox Johto_Route45_Text_83F5FD6, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 object 7 (local id 7, gfx 0x2C) at 28,45
Johto_Route45_EventScript_83F5CDD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4513, 0, Johto_Route45_Text_83F6086, Johto_Route45_Text_83F60C0
	msgbox Johto_Route45_Text_83F60CA, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 object 2 (local id 2, gfx 0x3EAE) at 37,25
Johto_Route45_EventScript_83F5CF4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4582, 0, Johto_Route45_Text_83F61B3, Johto_Route45_Text_83F61DE
	msgbox Johto_Route45_Text_83F61F5, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 object 6 (local id 6, gfx 0x3E85) at 30,14
Johto_Route45_EventScript_83F5D0B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4574, 0, Johto_Route45_Text_83F6258, Johto_Route45_Text_83F62AA
	msgbox Johto_Route45_Text_83F62B8, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 object 12 (local id 12, gfx 0x1F) at 12,73
Johto_Route45_EventScript_83F5D22::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4878, 0, Johto_Route45_Text_83F6319, Johto_Route45_Text_83F6333
	msgbox Johto_Route45_Text_83F6354, MSGBOX_AUTOCLOSE
	end

@ Johto_Route45 sign / bg event at 31,4
Johto_Route45_EventScript_83F5D39::
	msgbox Johto_Route45_Text_83F6396, MSGBOX_SIGN
	end

@ Johto_Route45 object 8 (local id 8, gfx 0x3A9E) at 35,67
Johto_Route45_EventScript_83F5D42::
	setorcopyvar VAR_0x8000, 0x33
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route45 object 10 (local id 10, gfx 0x3A9E) at 12,46
Johto_Route45_EventScript_83F5D4F::
	setorcopyvar VAR_0x8000, 0x1F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route45 object 3 (local id 3, gfx 0x3A9E) at 21,17
Johto_Route45_EventScript_83F5D5C::
	setorcopyvar VAR_0x8000, 0x30
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route45 object 5 (local id 5, gfx 0x3A9E) at 37,32
Johto_Route45_EventScript_83F5D69::
	setorcopyvar VAR_0x8000, 0x87
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route45_Text_83F5D76::
	.string "Be prepared for anything!\p"
	.string "Let me see if your POKéMON have\n"
	.string "been raised properly!$"

Johto_Route45_Text_83F5DC6::
	.string "Oh, I lost that!$"

Johto_Route45_Text_83F5DD7::
	.string "I’ll head back to BLACKTHORN’s\n"
	.string "ICE PATH and train some more.$"

Johto_Route45_Text_83F5E14::
	.string "Yo! You’re spunky! But you know\n"
	.string "what?\p"
	.string "When it comes to sheer\n"
	.string "spunkiness, I’m the man!$"

Johto_Route45_Text_83F5E6A::
	.string "My POKéMON weren’t spunky enough!$"

Johto_Route45_Text_83F5E8C::
	.string "Boy, do I love HP UP! Mmmm, yum!\p"
	.string "I keep drinking my POKéMON’s!\p"
	.string "I can’t help it!$"

Johto_Route45_Text_83F5EDC::
	.string "My POKéMON are power packed!$"

Johto_Route45_Text_83F5EF9::
	.string "Wahahah! I’m the big loser!$"

Johto_Route45_Text_83F5F15::
	.string "I’m not much good at thinking,\n"
	.string "see?\p"
	.string "So, I just plow ahead with power!$"

Johto_Route45_Text_83F5F5B::
	.string "Why do I climb mountains?\p"
	.string "Because they’re there.\p"
	.string "Why do I train POKéMON?\p"
	.string "Because they’re there!$"

Johto_Route45_Text_83F5FBB::
	.string "Losses… They’re there too!$"

Johto_Route45_Text_83F5FD6::
	.string "The best thing to ever happen to\n"
	.string "me was discovering POKéMON.$"

Johto_Route45_Text_83F6086::
	.string "I was training here alone.\p"
	.string "Behold the fruits of my labor!$"

Johto_Route45_Text_83F60C0::
	.string "Waaaargh!$"

Johto_Route45_Text_83F60CA::
	.string "This calls for extreme measures.\p"
	.string "I must take to the hills and train\n"
	.string "in solitude.$"

Johto_Route45_Text_83F61B3::
	.string "What are your thoughts on\n"
	.string "raising POKéMON?$"

Johto_Route45_Text_83F61DE::
	.string "You’ve won my respect.$"

Johto_Route45_Text_83F61F5::
	.string "I see you’re raising your POKéMON\n"
	.string "with care.\p"
	.string "The bond you build will save you\n"
	.string "in tough situations.$"

Johto_Route45_Text_83F6258::
	.string "What is your battle strategy?\p"
	.string "It is foolish to use strong moves\n"
	.string "indiscriminately.$"

Johto_Route45_Text_83F62AA::
	.string "Fine. I lost.$"

Johto_Route45_Text_83F62B8::
	.string "I’m not in favor of overly\n"
	.string "powerful moves.\p"
	.string "I want to win, but I also don’t\n"
	.string "want to harm POKéMON.$"

Johto_Route45_Text_83F6319::
	.string "I’m really, really tough!$"

Johto_Route45_Text_83F6333::
	.string "I was tough at the TRAINER HILL…$"

Johto_Route45_Text_83F6354::
	.string "Have you been to the TRAINER HILL?\p"
	.string "I never, ever lose there, but…$"

Johto_Route45_Text_83F6396::
	.string "ROUTE 45\n"
	.string "MOUNTAIN RD. AHEAD$"
