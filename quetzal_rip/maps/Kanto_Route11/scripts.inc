Kanto_Route11_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route11 sign / bg event at 3,7
Kanto_Route11_EventScript_8417CCC::
	msgbox Kanto_Route11_Text_84355CF, MSGBOX_SIGN
	end

@ Kanto_Route11 object 1 (local id 1, gfx 0x3F5C) at 19,8
Kanto_Route11_EventScript_8459EDE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8289, 0, Kanto_Route11_Text_8435284, Kanto_Route11_Text_843529F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_8459F05
	msgbox Kanto_Route11_Text_84352B6, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_8459F05::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8289, 0, Kanto_Route11_Text_846F25B, Kanto_Route11_Text_843529F
	msgbox Kanto_Route11_Text_84352B6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 3 (local id 3, gfx 0x3F5C) at 31,8
Kanto_Route11_EventScript_8459F1C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8290, 0, Kanto_Route11_Text_8435331, Kanto_Route11_Text_8435362
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_8459F43
	msgbox Kanto_Route11_Text_8435396, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_8459F43::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8290, 0, Kanto_Route11_Text_846F2C5, Kanto_Route11_Text_8435362
	msgbox Kanto_Route11_Text_8435396, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 8 (local id 8, gfx 0x3F5C) at 50,14
Kanto_Route11_EventScript_8459F5A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8291, 0, Kanto_Route11_Text_8435442, Kanto_Route11_Text_8435473
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_8459F81
	msgbox Kanto_Route11_Text_8435499, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_8459F81::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8291, 0, Kanto_Route11_Text_846F388, Kanto_Route11_Text_8435473
	msgbox Kanto_Route11_Text_8435499, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 4 (local id 4, gfx 0x3F5C) at 28,10
Kanto_Route11_EventScript_8459F98::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8292, 0, Kanto_Route11_Text_843554C, Kanto_Route11_Text_8435588
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_8459FBF
	msgbox Kanto_Route11_Text_84355AA, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_8459FBF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8292, 0, Kanto_Route11_Text_846F3E6, Kanto_Route11_Text_8435588
	msgbox Kanto_Route11_Text_84355AA, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 7 (local id 7, gfx 0x3EE6) at 42,8
Kanto_Route11_EventScript_8459FD6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8413, 0, Kanto_Route11_Text_84352D8, Kanto_Route11_Text_84352FE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_8459FFD
	msgbox Kanto_Route11_Text_8435311, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_8459FFD::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8413, 0, Kanto_Route11_Text_846F287, Kanto_Route11_Text_84352FE
	msgbox Kanto_Route11_Text_8435311, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 10 (local id 10, gfx 0x3EE6) at 32,18
Kanto_Route11_EventScript_845A014::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8414, 0, Kanto_Route11_Text_84354FD, Kanto_Route11_Text_8435517
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_845A03B
	msgbox Kanto_Route11_Text_843552D, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_845A03B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8414, 0, Kanto_Route11_Text_846F3C3, Kanto_Route11_Text_8435517
	msgbox Kanto_Route11_Text_843552D, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 2 (local id 2, gfx 0x3F2C) at 16,17
Kanto_Route11_EventScript_845A052::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8450, 0, Kanto_Route11_Text_84351C8, Kanto_Route11_Text_84351DC
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_845A079
	msgbox Kanto_Route11_Text_84351F5, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_845A079::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8450, 0, Kanto_Route11_Text_846F1F6, Kanto_Route11_Text_84351DC
	msgbox Kanto_Route11_Text_84351F5, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 5 (local id 5, gfx 0x3F2C) at 38,15
Kanto_Route11_EventScript_845A090::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8451, 0, Kanto_Route11_Text_8435224, Kanto_Route11_Text_8435245
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_845A0B7
	msgbox Kanto_Route11_Text_8435255, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_845A0B7::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8451, 0, Kanto_Route11_Text_846F21F, Kanto_Route11_Text_8435245
	msgbox Kanto_Route11_Text_8435255, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 9 (local id 9, gfx 0x3F2C) at 57,16
Kanto_Route11_EventScript_845A0CE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8452, 0, Kanto_Route11_Text_84353B0, Kanto_Route11_Text_84353CC
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_845A0F5
	msgbox Kanto_Route11_Text_84353DB, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_845A0F5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8452, 0, Kanto_Route11_Text_846F2FD, Kanto_Route11_Text_84353CC
	msgbox Kanto_Route11_Text_84353DB, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 6 (local id 6, gfx 0x3F2C) at 50,4
Kanto_Route11_EventScript_845A10C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8453, 0, Kanto_Route11_Text_84353FC, Kanto_Route11_Text_8435415
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EventScript_845A133
	msgbox Kanto_Route11_Text_8435428, MSGBOX_AUTOCLOSE
	end

Kanto_Route11_EventScript_845A133::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8453, 0, Kanto_Route11_Text_846F345, Kanto_Route11_Text_8435415
	msgbox Kanto_Route11_Text_8435428, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route11 object 11 (local id 11, gfx 0x3F0E) at 42,12
Kanto_Route11_EventScript_846BFD6::
	setorcopyvar VAR_0x8000, 0x7A
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_Route11 object 12 (local id 12, gfx 0x3F0E) at 63,13
Kanto_Route11_EventScript_846C07F::
	setorcopyvar VAR_0x8000, 2
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_Route11 object 13 (local id 13, gfx 0x3F0E) at 13,3
Kanto_Route11_EventScript_846C08C::
	setorcopyvar VAR_0x8000, 0x2F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_Route11_Text_84351C8::
	.string "Win, lose, or draw!$"

Kanto_Route11_Text_84351DC::
	.string "Atcha!\n"
	.string "Didn’t go my way!$"

Kanto_Route11_Text_84351F5::
	.string "POKéMON is life!\n"
	.string "And to live is to play games!$"

Kanto_Route11_Text_8435224::
	.string "Competition!\n"
	.string "I can’t get enough!$"

Kanto_Route11_Text_8435245::
	.string "I had a chance!$"

Kanto_Route11_Text_8435255::
	.string "You can’t be a coward in the world\n"
	.string "of POKéMON!$"

Kanto_Route11_Text_8435284::
	.string "Let’s go, but don’t cheat!$"

Kanto_Route11_Text_843529F::
	.string "Huh?\n"
	.string "That’s not right!$"

Kanto_Route11_Text_84352B6::
	.string "I did my best.\n"
	.string "I have no regrets.$"

Kanto_Route11_Text_84352D8::
	.string "Careful!\n"
	.string "I’m laying down some cables!$"

Kanto_Route11_Text_84352FE::
	.string "That was electric!$"

Kanto_Route11_Text_8435311::
	.string "Spread the word to save energy!$"

Kanto_Route11_Text_8435331::
	.string "I just became a TRAINER.\n"
	.string "But, I think I can win.$"

Kanto_Route11_Text_8435362::
	.string "My POKéMON couldn’t win…\n"
	.string "Haven’t they grown enough?$"

Kanto_Route11_Text_8435396::
	.string "What now?\n"
	.string "Leave me alone!$"

Kanto_Route11_Text_84353B0::
	.string "Fwahaha!\n"
	.string "I have never lost!$"

Kanto_Route11_Text_84353CC::
	.string "My first loss!$"

Kanto_Route11_Text_84353DB::
	.string "You were just lucky, that’s all.$"

Kanto_Route11_Text_84353FC::
	.string "I have never won before…$"

Kanto_Route11_Text_8435415::
	.string "I saw this coming…$"

Kanto_Route11_Text_8435428::
	.string "I was unlucky, as always.$"

Kanto_Route11_Text_8435442::
	.string "I’m the best in my class.\n"
	.string "I train every morning.$"

Kanto_Route11_Text_8435473::
	.string "Darn!\n"
	.string "My POKéMON need to be stronger!$"

Kanto_Route11_Text_8435499::
	.string "There’s a fat POKéMON that comes\n"
	.string "down from the mountains.\p"
	.string "I bet it’d be strong if you can\n"
	.string "catch it.$"

Kanto_Route11_Text_84354FD::
	.string "Watch out for live wires!$"

Kanto_Route11_Text_8435517::
	.string "Whoa!\n"
	.string "You spark plug!$"

Kanto_Route11_Text_843552D::
	.string "Well, better get back to work.$"

Kanto_Route11_Text_843554C::
	.string "I raised my POKéMON carefully.\n"
	.string "They should be ready by now!$"

Kanto_Route11_Text_8435588::
	.string "Bye-bye!\n"
	.string "Thank you, and good-bye!$"

Kanto_Route11_Text_84355AA::
	.string "Tch…\n"
	.string "I better go find stronger ones!$"

Kanto_Route11_Text_84355CF::
	.string "DIGLETT’S CAVE$"

Kanto_Route11_Text_846F1F6::
	.string "Win, lose, or draw!\n"
	.string "Now for our rematch!$"

Kanto_Route11_Text_846F21F::
	.string "Competing is the ultimate thrill.\n"
	.string "I still can’t get enough!$"

Kanto_Route11_Text_846F25B::
	.string "You know, right?\n"
	.string "Let’s go, but don’t cheat!$"

Kanto_Route11_Text_846F287::
	.string "Hi, there!\p"
	.string "But be careful!\n"
	.string "I’m still laying down some cables!$"

Kanto_Route11_Text_846F2C5::
	.string "I became a TRAINER a while ago.\n"
	.string "But, I think I can win.$"

Kanto_Route11_Text_846F2FD::
	.string "Fwahaha!\n"
	.string "I have never lost!\p"
	.string "…And if I did, I’ve forgotten all\n"
	.string "about it!$"

Kanto_Route11_Text_846F345::
	.string "I have never won before…\p"
	.string "Perhaps I am destined to remain\n"
	.string "that way…$"

Kanto_Route11_Text_846F388::
	.string "I’m the best in my class.\n"
	.string "I train every morning and night!$"

Kanto_Route11_Text_846F3C3::
	.string "Keep your eyes out for live wires!$"

Kanto_Route11_Text_846F3E6::
	.string "I raised my POKéMON carefully.\n"
	.string "They should be ready by now.\l"
	.string "This time, they should win, too.$"
