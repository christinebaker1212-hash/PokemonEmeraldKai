Kanto_Route21_South_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route21_South object 1 (local id 1, gfx 0x3F06) at 11,8
Kanto_Route21_South_EventScript_845BD84::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8422, 0, Kanto_Route21_South_Text_84388B9, Kanto_Route21_South_Text_84388D9
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_South_EventScript_845BDAB
	msgbox Kanto_Route21_South_Text_8438913, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_South_EventScript_845BDAB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8422, 0, Kanto_Route21_South_Text_84707C3, Kanto_Route21_South_Text_84388D9
	msgbox Kanto_Route21_South_Text_8438913, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_South object 2 (local id 2, gfx 0x3F06) at 14,15
Kanto_Route21_South_EventScript_845BE00::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8424, 0, Kanto_Route21_South_Text_843896D, Kanto_Route21_South_Text_843898F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_South_EventScript_845BE27
	msgbox Kanto_Route21_South_Text_84389A6, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_South_EventScript_845BE27::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8424, 0, Kanto_Route21_South_Text_84707FC, Kanto_Route21_South_Text_843898F
	msgbox Kanto_Route21_South_Text_84389A6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_South object 3 (local id 3, gfx 0x3F4F) at 15,24
Kanto_Route21_South_EventScript_845BE7C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8438, 0, Kanto_Route21_South_Text_84387B5, Kanto_Route21_South_Text_84387D1
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_South_EventScript_845BEA3
	msgbox Kanto_Route21_South_Text_84387E0, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_South_EventScript_845BEA3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8438, 0, Kanto_Route21_South_Text_847073E, Kanto_Route21_South_Text_84387D1
	msgbox Kanto_Route21_South_Text_84387E0, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_South object 4 (local id 4, gfx 0x3F4F) at 15,38
Kanto_Route21_South_EventScript_845BEBA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8439, 0, Kanto_Route21_South_Text_8438800, Kanto_Route21_South_Text_8438824
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_South_EventScript_845BEE1
	msgbox Kanto_Route21_South_Text_8438836, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_South_EventScript_845BEE1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8439, 0, Kanto_Route21_South_Text_8470779, Kanto_Route21_South_Text_8438824
	msgbox Kanto_Route21_South_Text_8438836, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_South object 5 (local id 5, gfx 0x3F4F) at 10,29
Kanto_Route21_South_EventScript_845BEF8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8440, 0, Kanto_Route21_South_Text_8438873, Kanto_Route21_South_Text_8438893
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_South_EventScript_845BF1F
	msgbox Kanto_Route21_South_Text_84388A0, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_South_EventScript_845BF1F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8440, 0, Kanto_Route21_South_Text_84707A3, Kanto_Route21_South_Text_8438893
	msgbox Kanto_Route21_South_Text_84388A0, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_South_Text_84387B5::
	.string "I caught my POKéMON at sea.$"

Kanto_Route21_South_Text_84387D1::
	.string "Diver!!\n"
	.string "Down!!$"

Kanto_Route21_South_Text_84387E0::
	.string "Where’d you catch your POKéMON?$"

Kanto_Route21_South_Text_8438800::
	.string "Right now, I’m in a triathlon meet.$"

Kanto_Route21_South_Text_8438824::
	.string "Pant…\n"
	.string "Pant… Pant…$"

Kanto_Route21_South_Text_8438836::
	.string "I’m beat! But, I still have the\n"
	.string "bike race and marathon left!$"

Kanto_Route21_South_Text_8438873::
	.string "Ahh!\n"
	.string "Feel the sun and the wind!$"

Kanto_Route21_South_Text_8438893::
	.string "Yow!\n"
	.string "I lost!$"

Kanto_Route21_South_Text_84388A0::
	.string "I’m sunburnt to a crisp!$"

Kanto_Route21_South_Text_84388B9::
	.string "Hey, don’t scare away the fish!$"

Kanto_Route21_South_Text_84388D9::
	.string "Sorry! I’m just so frustrated from\n"
	.string "not catching anything.$"

Kanto_Route21_South_Text_8438913::
	.string "Sheesh, I haven’t caught a thing.\n"
	.string "Could this place actually be a\l"
	.string "giant pool or something?$"

Kanto_Route21_South_Text_843896D::
	.string "Keep me company till I get a hit.$"

Kanto_Route21_South_Text_843898F::
	.string "That burned some time.$"

Kanto_Route21_South_Text_84389A6::
	.string "Oh wait!\n"
	.string "I got a bite! Yeah!$"

Kanto_Route21_South_Text_847073E::
	.string "I caught my POKéMON at sea.\n"
	.string "And that’s where I train them.$"

Kanto_Route21_South_Text_8470779::
	.string "Right now, I’m in another triathlon\n"
	.string "meet!$"

Kanto_Route21_South_Text_84707A3::
	.string "Ahh!\n"
	.string "Feel the sun and the wind!$"

Kanto_Route21_South_Text_84707C3::
	.string "Hey, cut it out already.\n"
	.string "You always scare away the fish!$"

Kanto_Route21_South_Text_84707FC::
	.string "Keep me company till I get a hit.$"
