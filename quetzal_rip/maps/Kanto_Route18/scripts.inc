Kanto_Route18_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route18_EventScript_8417EF8
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_Route18_OnWarp_8417F0D
	.byte 0

Kanto_Route18_OnWarp_8417F0D:
	map_script_2 VAR_QZ_0x5124, 1, Kanto_Route18_EventScript_8417F17
	.2byte 0

Kanto_Route18_EventScript_8417EF8::
	call_if_eq VAR_QZ_0x5124, 1, Kanto_Route18_EventScript_8417F09
	call Quetzal_EventScript_840FBB5
	end

Kanto_Route18_EventScript_8417F09::
	setflag FLAG_QZ_0x2210
	return

Kanto_Route18_EventScript_8417F17::
	special QzForcePlayerOntoBike_728
	end

@ Kanto_Route18 sign / bg event at 52,7
Kanto_Route18_EventScript_8417F1B::
	msgbox Kanto_Route18_Text_8437A3F, MSGBOX_SIGN
	end

@ Kanto_Route18 sign / bg event at 37,7
Kanto_Route18_EventScript_8417F24::
	msgbox Kanto_Route18_Text_8437A64, MSGBOX_SIGN
	end

@ Kanto_Route18 object 3 (local id 3, gfx 0x3F3B) at 39,12
Kanto_Route18_EventScript_845B728::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8499, 0, Kanto_Route18_Text_8437928, Kanto_Route18_Text_843795A
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route18_EventScript_845B74F
	msgbox Kanto_Route18_Text_843795F, MSGBOX_AUTOCLOSE
	end

Kanto_Route18_EventScript_845B74F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8499, 0, Kanto_Route18_Text_84700D1, Kanto_Route18_Text_843795A
	msgbox Kanto_Route18_Text_843795F, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route18 object 2 (local id 2, gfx 0x3F3B) at 41,15
Kanto_Route18_EventScript_845B766::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8500, 0, Kanto_Route18_Text_8437974, Kanto_Route18_Text_843799B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route18_EventScript_845B78D
	msgbox Kanto_Route18_Text_84379AD, MSGBOX_AUTOCLOSE
	end

Kanto_Route18_EventScript_845B78D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8500, 0, Kanto_Route18_Text_8470121, Kanto_Route18_Text_843799B
	msgbox Kanto_Route18_Text_84379AD, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route18 object 1 (local id 1, gfx 0x3F3B) at 46,14
Kanto_Route18_EventScript_845B7A4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8501, 0, Kanto_Route18_Text_84379EC, Kanto_Route18_Text_8437A0E
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route18_EventScript_845B7CB
	msgbox Kanto_Route18_Text_8437A14, MSGBOX_AUTOCLOSE
	end

Kanto_Route18_EventScript_845B7CB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8501, 0, Kanto_Route18_Text_847014E, Kanto_Route18_Text_8437A0E
	msgbox Kanto_Route18_Text_8437A14, MSGBOX_AUTOCLOSE
	end

Kanto_Route18_Text_8437928::
	.string "I always check every grassy area\n"
	.string "for new POKéMON.$"

Kanto_Route18_Text_843795A::
	.string "Tch!$"

Kanto_Route18_Text_843795F::
	.string "I wish I had a BIKE!$"

Kanto_Route18_Text_8437974::
	.string "Kurukkoo!\n"
	.string "How do you like my birdcall?$"

Kanto_Route18_Text_843799B::
	.string "I had to bug you!$"

Kanto_Route18_Text_84379AD::
	.string "I collect sea POKéMON on weekends\n"
	.string "because the sea is so close.$"

Kanto_Route18_Text_84379EC::
	.string "This is my turf!\n"
	.string "Get out of here!$"

Kanto_Route18_Text_8437A0E::
	.string "Darn!$"

Kanto_Route18_Text_8437A14::
	.string "This is my fave area for catching\n"
	.string "POKéMON.$"

Kanto_Route18_Text_8437A3F::
	.string "ROUTE 18\n"
	.string "CELADON CITY - FUCHSIA CITY$"

Kanto_Route18_Text_8437A64::
	.string "CYCLING ROAD\p"
	.string "A BICYCLE is required to enter.\n"
	.string "You may explore on foot inside.$"

Kanto_Route18_Text_84700D1::
	.string "I’ve been checking every grassy\n"
	.string "area for new POKéMON.\p"
	.string "But it’s not always easy…$"

Kanto_Route18_Text_8470121::
	.string "Koorukukukoo!\n"
	.string "Is my birdcall getting better?$"

Kanto_Route18_Text_847014E::
	.string "I warned you before, this is my\n"
	.string "turf!\p"
	.string "I don’t want you coming around.$"
