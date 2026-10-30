Kanto_SixIsland_PatternBush_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SixIsland_PatternBush_EventScript_84142FE
	.byte 0

Kanto_SixIsland_PatternBush_EventScript_84142FE::
	setflag FLAG_QZ_0x2259
	getplayerxy VAR_TEMP_1, VAR_TEMP_2
	call_if_ge VAR_TEMP_1, 50, Kanto_SixIsland_PatternBush_EventScript_8414322
	call_if_le VAR_TEMP_1, 49, Kanto_SixIsland_PatternBush_EventScript_841432B
	call Quetzal_EventScript_840FBB5
	end

Kanto_SixIsland_PatternBush_EventScript_8414322::
	setescapewarp MAP_KANTO_SIX_ISLAND_GREEN_PATH, 255, 64, 10
	return

Kanto_SixIsland_PatternBush_EventScript_841432B::
	setescapewarp MAP_KANTO_SIX_ISLAND_GREEN_PATH, 255, 45, 10
	return

@ Kanto_SixIsland_PatternBush object 1 (local id 1, gfx 0x3F58) at 48,15
Kanto_SixIsland_PatternBush_EventScript_8414334::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8801, 0, Kanto_SixIsland_PatternBush_Text_842C643, Kanto_SixIsland_PatternBush_Text_842C66F
	msgbox Kanto_SixIsland_PatternBush_Text_842C6A0, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 2 (local id 2, gfx 0x3F58) at 10,5
Kanto_SixIsland_PatternBush_EventScript_841434B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8802, 0, Kanto_SixIsland_PatternBush_Text_842C6F4, Kanto_SixIsland_PatternBush_Text_842C731
	msgbox Kanto_SixIsland_PatternBush_Text_842C75F, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 3 (local id 3, gfx 0x3EF1) at 51,6
Kanto_SixIsland_PatternBush_EventScript_8414362::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8803, 0, Kanto_SixIsland_PatternBush_Text_842C7C7, Kanto_SixIsland_PatternBush_Text_842C7FB
	msgbox Kanto_SixIsland_PatternBush_Text_842C81C, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 4 (local id 4, gfx 0x3EF1) at 12,13
Kanto_SixIsland_PatternBush_EventScript_8414379::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8804, 0, Kanto_SixIsland_PatternBush_Text_842C85E, Kanto_SixIsland_PatternBush_Text_842C89F
	msgbox Kanto_SixIsland_PatternBush_Text_842C8D9, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 5 (local id 5, gfx 0x3EF1) at 37,23
Kanto_SixIsland_PatternBush_EventScript_8414390::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8805, 0, Kanto_SixIsland_PatternBush_Text_842C94F, Kanto_SixIsland_PatternBush_Text_842C97E
	msgbox Kanto_SixIsland_PatternBush_Text_842C9AD, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 6 (local id 6, gfx 0x3F5C) at 32,5
Kanto_SixIsland_PatternBush_EventScript_84143A7::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8806, 0, Kanto_SixIsland_PatternBush_Text_842C9E2, Kanto_SixIsland_PatternBush_Text_842CA16
	msgbox Kanto_SixIsland_PatternBush_Text_842CA25, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 7 (local id 7, gfx 0x3F5C) at 52,20
Kanto_SixIsland_PatternBush_EventScript_84143BE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8807, 0, Kanto_SixIsland_PatternBush_Text_842CA7C, Kanto_SixIsland_PatternBush_Text_842CAAE
	msgbox Kanto_SixIsland_PatternBush_Text_842CAD7, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 8 (local id 8, gfx 0x3F16) at 8,21
Kanto_SixIsland_PatternBush_EventScript_84143D5::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8808, 0, Kanto_SixIsland_PatternBush_Text_842CB34, Kanto_SixIsland_PatternBush_Text_842CB61
	msgbox Kanto_SixIsland_PatternBush_Text_842CB99, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 9 (local id 9, gfx 0x3F16) at 39,6
Kanto_SixIsland_PatternBush_EventScript_84143EC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8809, 0, Kanto_SixIsland_PatternBush_Text_842CC07, Kanto_SixIsland_PatternBush_Text_842CC5B
	msgbox Kanto_SixIsland_PatternBush_Text_842CC99, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 10 (local id 10, gfx 0x3EF3) at 3,9
Kanto_SixIsland_PatternBush_EventScript_8414403::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8810, 0, Kanto_SixIsland_PatternBush_Text_842CCE4, Kanto_SixIsland_PatternBush_Text_842CD12
	msgbox Kanto_SixIsland_PatternBush_Text_842CD1E, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 11 (local id 11, gfx 0x3F31) at 19,24
Kanto_SixIsland_PatternBush_EventScript_841441A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8811, 0, Kanto_SixIsland_PatternBush_Text_842CD47, Kanto_SixIsland_PatternBush_Text_842CD69
	msgbox Kanto_SixIsland_PatternBush_Text_842CD97, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_PatternBush object 12 (local id 12, gfx 0x3F0C) at 30,19
Kanto_SixIsland_PatternBush_EventScript_8414431::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8812, 0, Kanto_SixIsland_PatternBush_Text_842CDD2, Kanto_SixIsland_PatternBush_Text_842CE04
	msgbox Kanto_SixIsland_PatternBush_Text_842CE31, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_PatternBush_Text_842C643::
	.string "I’m curious, how do you raise your\n"
	.string "POKéMON?$"

Kanto_SixIsland_PatternBush_Text_842C66F::
	.string "You raise your POKéMON with a\n"
	.string "whole lot of love!$"

Kanto_SixIsland_PatternBush_Text_842C6A0::
	.string "In the same way your mother raised\n"
	.string "you full of love, you should raise\l"
	.string "your POKéMON.$"

Kanto_SixIsland_PatternBush_Text_842C6F4::
	.string "I’m working to preserve the natural\n"
	.string "ecology of POKéMON here.$"

Kanto_SixIsland_PatternBush_Text_842C731::
	.string "Oh, for someone so young, you are\n"
	.string "tremendous!$"

Kanto_SixIsland_PatternBush_Text_842C75F::
	.string "I’m not saying that you shouldn’t\n"
	.string "catch POKéMON.\p"
	.string "I just want people to raise their\n"
	.string "POKéMON responsibly.$"

Kanto_SixIsland_PatternBush_Text_842C7C7::
	.string "I caught a BUG POKéMON that lives\n"
	.string "only around here!$"

Kanto_SixIsland_PatternBush_Text_842C7FB::
	.string "Heheh…\n"
	.string "Isn’t my POKéMON awesome?$"

Kanto_SixIsland_PatternBush_Text_842C81C::
	.string "There’s a girl near the BUSH who\n"
	.string "measures HERACROSS for TRAINERS.$"

Kanto_SixIsland_PatternBush_Text_842C85E::
	.string "You know, it just doesn’t feel right\n"
	.string "if I don’t use BUG POKéMON.$"

Kanto_SixIsland_PatternBush_Text_842C89F::
	.string "It’s okay, losing is a fun part of\n"
	.string "POKéMON’s appeal, too.$"

Kanto_SixIsland_PatternBush_Text_842C8D9::
	.string "By the way, “appeal” is a weird\n"
	.string "word, if you think about it.\p"
	.string "Like, is it like an orange peel?\n"
	.string "Or like a ringing bell?$"

Kanto_SixIsland_PatternBush_Text_842C94F::
	.string "Yeah, yeah, yeah!\n"
	.string "Look at all the BUG POKéMON!$"

Kanto_SixIsland_PatternBush_Text_842C97E::
	.string "I got beat while I was still busy\n"
	.string "celebrating!$"

Kanto_SixIsland_PatternBush_Text_842C9AD::
	.string "I’m going to bring my little brother\n"
	.string "here next time.$"

Kanto_SixIsland_PatternBush_Text_842C9E2::
	.string "Look, look! There are funny\n"
	.string "patterns on the ground.$"

Kanto_SixIsland_PatternBush_Text_842CA16::
	.string "Super awesome!$"

Kanto_SixIsland_PatternBush_Text_842CA25::
	.string "The funny patterns on the ground…\p"
	.string "They look like the patterns on my\n"
	.string "grandpa’s clothes.$"

Kanto_SixIsland_PatternBush_Text_842CA7C::
	.string "I bet you think I’m just some guy,\n"
	.string "don’t you, eh?$"

Kanto_SixIsland_PatternBush_Text_842CAAE::
	.string "I bet you think I’m weak,\n"
	.string "don’t you, eh?$"

Kanto_SixIsland_PatternBush_Text_842CAD7::
	.string "Ayup, you townies come around all\n"
	.string "so dandy with a hat that flashy…\p"
	.string "How about giving me that?$"

Kanto_SixIsland_PatternBush_Text_842CB34::
	.string "Take a deep breath.\n"
	.string "Isn’t the air delicious?$"

Kanto_SixIsland_PatternBush_Text_842CB61::
	.string "If you’d like, I can teach you how\n"
	.string "to breathe properly.$"

Kanto_SixIsland_PatternBush_Text_842CB99::
	.string "First, exhale.\n"
	.string "Blow everything out.\p"
	.string "When you can’t exhale anymore,\n"
	.string "inhale the clean air!\p"
	.string "Isn’t it refreshing?$"

Kanto_SixIsland_PatternBush_Text_842CC07::
	.string "I love BUG POKéMON.\n"
	.string "That’s why I’m here all the time.\p"
	.string "Am I the only girl like that?$"

Kanto_SixIsland_PatternBush_Text_842CC5B::
	.string "I lost, but I’m still laughing.\n"
	.string "Am I the only girl like that?$"

Kanto_SixIsland_PatternBush_Text_842CC99::
	.string "I’m going to keep on collecting\n"
	.string "BUG POKéMON.\p"
	.string "Am I the only girl like that?$"

Kanto_SixIsland_PatternBush_Text_842CCE4::
	.string "This is a good spot.\n"
	.string "I’ll pitch my tent here.$"

Kanto_SixIsland_PatternBush_Text_842CD12::
	.string "Wimped out…$"

Kanto_SixIsland_PatternBush_Text_842CD1E::
	.string "I’m going to observe the night sky\n"
	.string "here.$"

Kanto_SixIsland_PatternBush_Text_842CD47::
	.string "Oh, yuck!\n"
	.string "I think a bug stung me!$"

Kanto_SixIsland_PatternBush_Text_842CD69::
	.string "It wasn’t a bug.\n"
	.string "I cut my shin on some grass.$"

Kanto_SixIsland_PatternBush_Text_842CD97::
	.string "A little cut like that…\n"
	.string "A little spit’s enough to cure it!$"

Kanto_SixIsland_PatternBush_Text_842CDD2::
	.string "Have you noticed something odd\n"
	.string "about these parts?$"

Kanto_SixIsland_PatternBush_Text_842CE04::
	.string "Have you taken a good look around\n"
	.string "your feet?$"

Kanto_SixIsland_PatternBush_Text_842CE31::
	.string "There are places here in PATTERN\n"
	.string "BUSH where grass won’t grow.\p"
	.string "What could be the cause of such\n"
	.string "a phenomenon?$"
