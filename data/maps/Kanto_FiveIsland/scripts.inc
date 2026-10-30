Kanto_FiveIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FiveIsland_EventScript_8417B1F
	.byte 0

Kanto_FiveIsland_EventScript_8417B1F::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x2239
	end

@ Kanto_FiveIsland sign / bg event at 20,8
Kanto_FiveIsland_EventScript_8417B26::
	msgbox Kanto_FiveIsland_Text_84330C2, MSGBOX_SIGN
	end

@ Kanto_FiveIsland object 2 (local id 2, gfx 0x3EF1) at 16,10
Kanto_FiveIsland_EventScript_8417B2F::
	msgbox Kanto_FiveIsland_Text_84330ED, MSGBOX_NPC
	end

@ Kanto_FiveIsland object 1 (local id 1, gfx 0x3F06) at 8,5
Kanto_FiveIsland_EventScript_8417B38::
	msgbox Kanto_FiveIsland_Text_843315D, MSGBOX_NPC
	end

Kanto_FiveIsland_Text_84330C2::
	.string "FIVE ISLAND\n"
	.string "Chrono Island: Where Time Goes$"

Kanto_FiveIsland_Text_84330ED::
	.string "Oh, this stinks. I wanted to play in\n"
	.string "the MEADOW.\p"
	.string "But they put up a weird building\n"
	.string "on it, so I can’t play there.$"

Kanto_FiveIsland_Text_843315D::
	.string "It’s been getting busier around\n"
	.string "these parts lately.\p"
	.string "I haven’t been catching many good\n"
	.string "fish lately, either.$"
