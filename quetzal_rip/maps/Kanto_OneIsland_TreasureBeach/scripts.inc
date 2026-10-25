Kanto_OneIsland_TreasureBeach_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_OneIsland_TreasureBeach object 2 (local id 2, gfx 0x3EEE) at 13,20
Kanto_OneIsland_TreasureBeach_EventScript_841845B::
	msgbox Kanto_OneIsland_TreasureBeach_Text_843A0B7, MSGBOX_NPC
	end

@ Kanto_OneIsland_TreasureBeach object 1 (local id 1, gfx 0x3F4D) at 8,11
Kanto_OneIsland_TreasureBeach_EventScript_845C2F8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8738, 0, Kanto_OneIsland_TreasureBeach_Text_843A043, Kanto_OneIsland_TreasureBeach_Text_843A074
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_TreasureBeach_EventScript_845C31F
	msgbox Kanto_OneIsland_TreasureBeach_Text_843A08C, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_TreasureBeach_EventScript_845C31F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8738, 0, Kanto_OneIsland_TreasureBeach_Text_8470C19, Kanto_OneIsland_TreasureBeach_Text_843A074
	msgbox Kanto_OneIsland_TreasureBeach_Text_843A08C, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_TreasureBeach_Text_843A043::
	.string "Lying back, rocked by the waves…\n"
	.string "It’s exquisite.$"

Kanto_OneIsland_TreasureBeach_Text_843A074::
	.string "I lost…\n"
	.string "It feels awful.$"

Kanto_OneIsland_TreasureBeach_Text_843A08C::
	.string "The sea is pleasantly warm in these\n"
	.string "parts.$"

Kanto_OneIsland_TreasureBeach_Text_843A0B7::
	.string "Good things get washed up by the\n"
	.string "ocean current onto this beach.\p"
	.string "That’s why I often come here for\n"
	.string "some beachcombing.$"

Kanto_OneIsland_TreasureBeach_Text_8470C19::
	.string "Lying back, rocked by the waves…\n"
	.string "I don’t notice time slipping by…$"
