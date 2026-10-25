Kanto_FiveIsland_WaterLabyrinth_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_FiveIsland_WaterLabyrinth object 1 (local id 1, gfx 0x3F09) at 14,11
Kanto_FiveIsland_WaterLabyrinth_EventScript_8418552::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2175, Kanto_FiveIsland_WaterLabyrinth_EventScript_84185BF
	call Quetzal_EventScript_8361A4D
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_84185D3, MSGBOX_DEFAULT
	qz_givemon_ex SPECIES_TOGEPI, 5, ITEM_NONE, 1, 3, 0
	goto_if_eq VAR_RESULT, 2, Kanto_FiveIsland_WaterLabyrinth_EventScript_84185C9
	setflag FLAG_QZ_0x2175
	clearflag FLAG_QZ_0x21E2
	playfanfare MUS_LEVEL_UP
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_WaterLabyrinth_EventScript_84185B2
	message Kanto_FiveIsland_WaterLabyrinth_Text_841861F
	goto Kanto_FiveIsland_WaterLabyrinth_EventScript_84185B7

Kanto_FiveIsland_WaterLabyrinth_EventScript_84185B2::
	message Kanto_FiveIsland_WaterLabyrinth_Text_841863F

Kanto_FiveIsland_WaterLabyrinth_EventScript_84185B7::
	waitfanfare
	waitmessage
	waitbuttonpress
	goto Kanto_FiveIsland_WaterLabyrinth_EventScript_84185BF

Kanto_FiveIsland_WaterLabyrinth_EventScript_84185BF::
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_8418678, MSGBOX_DEFAULT
	release
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_84185C9::
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_8418698, MSGBOX_DEFAULT
	release
	end

@ Kanto_FiveIsland_WaterLabyrinth object 2 (local id 2, gfx 0x3F58) at 9,7
Kanto_FiveIsland_WaterLabyrinth_EventScript_845C6AA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8712, 0, Kanto_FiveIsland_WaterLabyrinth_Text_843ADBE, Kanto_FiveIsland_WaterLabyrinth_Text_843ADF4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_WaterLabyrinth_EventScript_845C6D1
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_843AE20, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_WaterLabyrinth_EventScript_845C6D1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8712, 0, Kanto_FiveIsland_WaterLabyrinth_Text_847105D, Kanto_FiveIsland_WaterLabyrinth_Text_843ADF4
	msgbox Kanto_FiveIsland_WaterLabyrinth_Text_843AE20, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_WaterLabyrinth_Text_84185D3::
	.string "This little POKéMON wants to\n"
	.string "see the world.\p"
	.string "I’d like it to travel with you!$"

Kanto_FiveIsland_WaterLabyrinth_Text_841861F::
	.string "You received a special POKéMON!$"

Kanto_FiveIsland_WaterLabyrinth_Text_841863F::
	.string "You received a special POKéMON!\p"
	.string "It was sent to a PC Box.$"

Kanto_FiveIsland_WaterLabyrinth_Text_8418678::
	.string "Enjoy many adventures together!$"

Kanto_FiveIsland_WaterLabyrinth_Text_8418698::
	.string "You can’t receive it right now.\n"
	.string "I’ll care for it until you can.$"

Kanto_FiveIsland_WaterLabyrinth_Text_843ADBE::
	.string "This is an excellent environment\n"
	.string "for raising POKéMON.$"

Kanto_FiveIsland_WaterLabyrinth_Text_843ADF4::
	.string "Your POKéMON are growing up\n"
	.string "admirably well!$"

Kanto_FiveIsland_WaterLabyrinth_Text_843AE20::
	.string "You know, I think you have the\n"
	.string "talent to be a good BREEDER.$"

Kanto_FiveIsland_WaterLabyrinth_Text_847105D::
	.string "Oh, hello!\p"
	.string "Are you raising your POKéMON in a\n"
	.string "good environment?$"
