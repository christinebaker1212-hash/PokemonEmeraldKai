Kanto_Route21_North_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route21_North object 1 (local id 1, gfx 0x3F06) at 7,27
Kanto_Route21_North_EventScript_845BD46::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8421, 0, Kanto_Route21_North_Text_8438651, Kanto_Route21_North_Text_843867A
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_North_EventScript_845BD6D
	msgbox Kanto_Route21_North_Text_8438680, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_North_EventScript_845BD6D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8421, 0, Kanto_Route21_North_Text_84706AE, Kanto_Route21_North_Text_843867A
	msgbox Kanto_Route21_North_Text_8438680, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_North object 2 (local id 2, gfx 0x3F06) at 16,26
Kanto_Route21_North_EventScript_845BDC2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8423, 0, Kanto_Route21_North_Text_84386BD, Kanto_Route21_North_Text_84386E0
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_North_EventScript_845BDE9
	msgbox Kanto_Route21_North_Text_84386FD, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_North_EventScript_845BDE9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8423, 0, Kanto_Route21_North_Text_84706D7, Kanto_Route21_North_Text_84386E0
	msgbox Kanto_Route21_North_Text_84386FD, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_North object 3 (local id 3, gfx 0x3F4F) at 12,44
Kanto_Route21_North_EventScript_845BE3E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8437, 0, Kanto_Route21_North_Text_843871D, Kanto_Route21_North_Text_8438740
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_North_EventScript_845BE65
	msgbox Kanto_Route21_North_Text_8438746, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_North_EventScript_845BE65::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8437, 0, Kanto_Route21_North_Text_847070A, Kanto_Route21_North_Text_8438740
	msgbox Kanto_Route21_North_Text_8438746, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_North object 4 (local id 4, gfx 0x3F4D) at 14,35
Kanto_Route21_North_EventScript_845BF36::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8683, 0, Kanto_Route21_North_Text_84389C3, Kanto_Route21_North_Text_84389F3, Kanto_Route21_North_Text_8438A3F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_North_EventScript_845BF61
	msgbox Kanto_Route21_North_Text_8438A15, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_North_EventScript_845BF61::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8683, 0, Kanto_Route21_North_Text_847081E, Kanto_Route21_North_Text_84389F3, Kanto_Route21_North_Text_8438A3F
	msgbox Kanto_Route21_North_Text_8438A15, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route21_North object 5 (local id 5, gfx 0x3F53) at 15,35
Kanto_Route21_North_EventScript_845BF7C::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8683, 0, Kanto_Route21_North_Text_8438A95, Kanto_Route21_North_Text_8438AD2, Kanto_Route21_North_Text_8438B31
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route21_North_EventScript_845BFA7
	msgbox Kanto_Route21_North_Text_8438AF3, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_North_EventScript_845BFA7::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8683, 0, Kanto_Route21_North_Text_8470854, Kanto_Route21_North_Text_8438AD2, Kanto_Route21_North_Text_8438B31
	msgbox Kanto_Route21_North_Text_8438AF3, MSGBOX_AUTOCLOSE
	end

Kanto_Route21_North_Text_8438651::
	.string "You want to know if the fish are\n"
	.string "biting?$"

Kanto_Route21_North_Text_843867A::
	.string "Dang!$"

Kanto_Route21_North_Text_8438680::
	.string "I can’t catch anything good.\n"
	.string "Not one good POKéMON to be had!$"

Kanto_Route21_North_Text_84386BD::
	.string "I got a big haul!\n"
	.string "Wanna go for it?$"

Kanto_Route21_North_Text_84386E0::
	.string "A big haul isn’t everything!$"

Kanto_Route21_North_Text_84386FD::
	.string "I keep catching different fish!$"

Kanto_Route21_North_Text_843871D::
	.string "The sea cleanses my body and soul!$"

Kanto_Route21_North_Text_8438740::
	.string "Ayah!$"

Kanto_Route21_North_Text_8438746::
	.string "The sea is great and all, but I\n"
	.string "like the mountains, too.$"

Kanto_Route21_North_Text_84389C3::
	.string "LIL: Huh? A battle?\n"
	.string "IAN, can’t you do it alone?$"

Kanto_Route21_North_Text_84389F3::
	.string "LIL: Oh, see?\n"
	.string "We lost. Happy now?$"

Kanto_Route21_North_Text_8438A15::
	.string "LIL: I’m tired.\n"
	.string "Can’t we go home already?$"

Kanto_Route21_North_Text_8438A3F::
	.string "LIL: Huh? A battle?\n"
	.string "I can’t be bothered to do it alone.\l"
	.string "Bring two POKéMON, won’t you?$"

Kanto_Route21_North_Text_8438A95::
	.string "IAN: My sis doesn’t get enough\n"
	.string "exercise, so I made her come.$"

Kanto_Route21_North_Text_8438AD2::
	.string "IAN: Awww, Sis!\n"
	.string "Get it together!$"

Kanto_Route21_North_Text_8438AF3::
	.string "IAN: Come on, Sis!\p"
	.string "You’re not going to lose weight\n"
	.string "like this!$"

Kanto_Route21_North_Text_8438B31::
	.string "IAN: We want a two-on-two battle.\n"
	.string "So can you bring two POKéMON?$"

Kanto_Route21_North_Text_84706AE::
	.string "You want to know if the fish are\n"
	.string "biting?$"

Kanto_Route21_North_Text_84706D7::
	.string "I got a big haul again!\n"
	.string "Wanna go for it once more?$"

Kanto_Route21_North_Text_847070A::
	.string "The sea cleanses my body and soul!$"

Kanto_Route21_North_Text_847081E::
	.string "LIL: Huh? A battle again?\n"
	.string "IAN, can’t you do it alone?$"

Kanto_Route21_North_Text_8470854::
	.string "IAN: My sis is still a slob.\n"
	.string "Help me get her into shape!$"
