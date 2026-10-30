Kanto_Route17_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route17 sign / bg event at 12,53
Kanto_Route17_EventScript_8417EB7::
	msgbox Kanto_Route17_Text_84377AF, MSGBOX_SIGN
	end

@ Kanto_Route17 sign / bg event at 12,68
Kanto_Route17_EventScript_8417EC0::
	msgbox Kanto_Route17_Text_84377DD, MSGBOX_SIGN
	end

@ Kanto_Route17 sign / bg event at 12,83
Kanto_Route17_EventScript_8417EC9::
	msgbox Kanto_Route17_Text_8437846, MSGBOX_SIGN
	end

@ Kanto_Route17 sign / bg event at 12,97
Kanto_Route17_EventScript_8417ED2::
	msgbox Kanto_Route17_Text_8437898, MSGBOX_SIGN
	end

@ Kanto_Route17 sign / bg event at 12,125
Kanto_Route17_EventScript_8417EDB::
	msgbox Kanto_Route17_Text_84378BD, MSGBOX_SIGN
	end

@ Kanto_Route17 sign / bg event at 11,157
Kanto_Route17_EventScript_8417EE4::
	msgbox Kanto_Route17_Text_84378FC, MSGBOX_SIGN
	end

@ Kanto_Route17 object 5 (local id 5, gfx 0x3EE9) at 7,38
Kanto_Route17_EventScript_845B4BC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8395, 0, Kanto_Route17_Text_8437588, Kanto_Route17_Text_84375AE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B4E3
	msgbox Kanto_Route17_Text_84375B6, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B4E3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8395, 0, Kanto_Route17_Text_846FFAC, Kanto_Route17_Text_84375AE
	msgbox Kanto_Route17_Text_84375B6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 4 (local id 4, gfx 0x3EE9) at 18,41
Kanto_Route17_EventScript_845B4FA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8396, 0, Kanto_Route17_Text_84375D5, Kanto_Route17_Text_84375F0
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B521
	msgbox Kanto_Route17_Text_84375FE, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B521::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8396, 0, Kanto_Route17_Text_846FFD2, Kanto_Route17_Text_84375F0
	msgbox Kanto_Route17_Text_84375FE, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 10 (local id 10, gfx 0x3EE9) at 4,116
Kanto_Route17_EventScript_845B538::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8397, 0, Kanto_Route17_Text_8437721, Kanto_Route17_Text_843773D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B55F
	msgbox Kanto_Route17_Text_8437744, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B55F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8397, 0, Kanto_Route17_Text_847008C, Kanto_Route17_Text_843773D
	msgbox Kanto_Route17_Text_8437744, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 9 (local id 9, gfx 0x3EE9) at 16,134
Kanto_Route17_EventScript_845B576::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8398, 0, Kanto_Route17_Text_8437766, Kanto_Route17_Text_8437781
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B59D
	msgbox Kanto_Route17_Text_8437794, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B59D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8398, 0, Kanto_Route17_Text_84700B6, Kanto_Route17_Text_8437781
	msgbox Kanto_Route17_Text_8437794, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 3 (local id 3, gfx 0x3EE9) at 15,24
Kanto_Route17_EventScript_845B5B4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8444, 0, Kanto_Route17_Text_8437449, Kanto_Route17_Text_8437479
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B5DB
	msgbox Kanto_Route17_Text_8437485, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B5DB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8444, 0, Kanto_Route17_Text_846FF2E, Kanto_Route17_Text_8437479
	msgbox Kanto_Route17_Text_8437485, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 2 (local id 2, gfx 0x3EE9) at 11,15
Kanto_Route17_EventScript_845B5F2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8445, 0, Kanto_Route17_Text_84374F3, Kanto_Route17_Text_843751F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B619
	msgbox Kanto_Route17_Text_8437525, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B619::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8445, 0, Kanto_Route17_Text_846FF6A, Kanto_Route17_Text_843751F
	msgbox Kanto_Route17_Text_8437525, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 7 (local id 7, gfx 0x3EE9) at 21,61
Kanto_Route17_EventScript_845B630::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8446, 0, Kanto_Route17_Text_843762D, Kanto_Route17_Text_8437660
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B657
	msgbox Kanto_Route17_Text_843766A, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B657::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8446, 0, Kanto_Route17_Text_8470004, Kanto_Route17_Text_8437660
	msgbox Kanto_Route17_Text_843766A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 6 (local id 6, gfx 0x3EE9) at 2,61
Kanto_Route17_EventScript_845B66E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8447, 0, Kanto_Route17_Text_843769C, Kanto_Route17_Text_84376B6
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B695
	msgbox Kanto_Route17_Text_84376CA, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B695::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8447, 0, Kanto_Route17_Text_8470042, Kanto_Route17_Text_84376B6
	msgbox Kanto_Route17_Text_84376CA, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 8 (local id 8, gfx 0x3EE9) at 18,94
Kanto_Route17_EventScript_845B6AC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8448, 0, Kanto_Route17_Text_84376E8, Kanto_Route17_Text_84376F4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B6D3
	msgbox Kanto_Route17_Text_84376FD, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B6D3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8448, 0, Kanto_Route17_Text_8470080, Kanto_Route17_Text_84376F4
	msgbox Kanto_Route17_Text_84376FD, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route17 object 1 (local id 1, gfx 0x3EE9) at 4,18
Kanto_Route17_EventScript_845B6EA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8662, 0, Kanto_Route17_Text_8437548, Kanto_Route17_Text_8437560
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route17_EventScript_845B711
	msgbox Kanto_Route17_Text_8437570, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_EventScript_845B711::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8662, 0, Kanto_Route17_Text_846FF96, Kanto_Route17_Text_8437560
	msgbox Kanto_Route17_Text_8437570, MSGBOX_AUTOCLOSE
	end

Kanto_Route17_Text_8437449::
	.string "There’s no getting rich quick in\n"
	.string "battling kids.$"

Kanto_Route17_Text_8437479::
	.string "Burned out!$"

Kanto_Route17_Text_8437485::
	.string "You can find good stuff lying\n"
	.string "around on CYCLING ROAD.\p"
	.string "There’s good money in picking them\n"
	.string "up and selling them.$"

Kanto_Route17_Text_84374F3::
	.string "I’m mighty proud of my bod, kiddo.\n"
	.string "Come on!$"

Kanto_Route17_Text_843751F::
	.string "Whoo!$"

Kanto_Route17_Text_8437525::
	.string "I could belly-bump you outta\n"
	.string "here!$"

Kanto_Route17_Text_8437548::
	.string "You heading to FUCHSIA?$"

Kanto_Route17_Text_8437560::
	.string "Crash and burn!$"

Kanto_Route17_Text_8437570::
	.string "I love racing downhill!$"

Kanto_Route17_Text_8437588::
	.string "We’re BIKERS!\n"
	.string "We rule the roads, man!$"

Kanto_Route17_Text_84375AE::
	.string "Smoked!$"

Kanto_Route17_Text_84375B6::
	.string "Are you looking for adventure?$"

Kanto_Route17_Text_84375D5::
	.string "Let VOLTORB electrify you!$"

Kanto_Route17_Text_84375F0::
	.string "Grounded out!$"

Kanto_Route17_Text_84375FE::
	.string "I got my VOLTORB at the abandoned\n"
	.string "POWER PLANT.$"

Kanto_Route17_Text_843762D::
	.string "I leveled up my POKéMON, but it\n"
	.string "won’t evolve. Why?$"

Kanto_Route17_Text_8437660::
	.string "Why, you!$"

Kanto_Route17_Text_843766A::
	.string "Maybe some POKéMON need element\n"
	.string "STONES to evolve.$"

Kanto_Route17_Text_843769C::
	.string "I need a little exercise!$"

Kanto_Route17_Text_84376B6::
	.string "Whew!\n"
	.string "Good workout!$"

Kanto_Route17_Text_84376CA::
	.string "I’m sure I lost weight there!$"

Kanto_Route17_Text_84376E8::
	.string "Be a rebel!$"

Kanto_Route17_Text_84376F4::
	.string "Aaaargh!$"

Kanto_Route17_Text_84376FD::
	.string "Be ready to fight for your beliefs!$"

Kanto_Route17_Text_8437721::
	.string "Nice BIKE!\n"
	.string "How’s it handle?$"

Kanto_Route17_Text_843773D::
	.string "Shoot!$"

Kanto_Route17_Text_8437744::
	.string "The slope makes it hard to steer.$"

Kanto_Route17_Text_8437766::
	.string "Get lost, kid!\n"
	.string "I’m bushed!$"

Kanto_Route17_Text_8437781::
	.string "Are you satisfied?$"

Kanto_Route17_Text_8437794::
	.string "I need to catch a few z’s!$"

Kanto_Route17_Text_84377AF::
	.string "It’s a notice.\p"
	.string "Watch out for discarded items.$"

Kanto_Route17_Text_84377DD::
	.string "TRAINER TIPS\p"
	.string "All POKéMON are unique.\p"
	.string "Even POKéMON of the same species\n"
	.string "and level grow at different rates.$"

Kanto_Route17_Text_8437846::
	.string "TRAINER TIPS\p"
	.string "Get off your bike and move slowly\n"
	.string "when approaching a DexNav POKéMON.$"

Kanto_Route17_Text_8437898::
	.string "ROUTE 17\n"
	.string "CELADON CITY - FUCHSIA CITY$"

Kanto_Route17_Text_84378BD::
	.string "It’s a notice!\p"
	.string "Don’t throw the game, throw POKé\n"
	.string "BALLS instead!$"

Kanto_Route17_Text_84378FC::
	.string "CYCLING ROAD\n"
	.string "Continue towards FUCHSIA CITY.$"

Kanto_Route17_Text_846FF2E::
	.string "I told you, there’s no getting rich\n"
	.string "quick in battling kids.$"

Kanto_Route17_Text_846FF6A::
	.string "I’m mighty proud of my bod, kiddo.\n"
	.string "Come on!$"

Kanto_Route17_Text_846FF96::
	.string "You out for a stroll?$"

Kanto_Route17_Text_846FFAC::
	.string "We’re BIKERS!\n"
	.string "We rule the roads, man!$"

Kanto_Route17_Text_846FFD2::
	.string "VOLTORB’s going to seriously\n"
	.string "electrify you today!$"

Kanto_Route17_Text_8470004::
	.string "I leveled up my POKéMON, but it\n"
	.string "absolutely won’t evolve. Why?$"

Kanto_Route17_Text_8470042::
	.string "Gaah! I really need to exercise\n"
	.string "and seriously trim some flab!$"

Kanto_Route17_Text_8470080::
	.string "Be a rebel!$"

Kanto_Route17_Text_847008C::
	.string "Yep, that’s a nice BIKE!\n"
	.string "How’s it handle?$"

Kanto_Route17_Text_84700B6::
	.string "Get lost, kid!\n"
	.string "I’m bushed!$"
