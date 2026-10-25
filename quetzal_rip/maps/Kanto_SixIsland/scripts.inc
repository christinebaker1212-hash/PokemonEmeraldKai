Kanto_SixIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SixIsland_EventScript_8417B83
	.byte 0

Kanto_SixIsland_EventScript_8417B83::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2256
	end

@ Kanto_SixIsland object 2 (local id 2, gfx 0x3F0C) at 15,13
Kanto_SixIsland_EventScript_8417B8A::
	msgbox Kanto_SixIsland_Text_84334B6, MSGBOX_NPC
	end

@ Kanto_SixIsland object 1 (local id 1, gfx 0x3EEE) at 10,15
Kanto_SixIsland_EventScript_8417B93::
	msgbox Kanto_SixIsland_Text_843351C, MSGBOX_NPC
	end

@ Kanto_SixIsland sign / bg event at 9,17
Kanto_SixIsland_EventScript_8417B9C::
	msgbox Kanto_SixIsland_Text_843348D, MSGBOX_SIGN
	end

Kanto_SixIsland_Text_843348D::
	.string "SIX ISLAND\n"
	.string "Fortune Island of Aged Wisdom$"

Kanto_SixIsland_Text_84334B6::
	.string "Keep going this way and you’ll get\n"
	.string "to the WATER PATH.\p"
	.string "Stay on that and you’ll get to\n"
	.string "the RUIN VALLEY.$"

Kanto_SixIsland_Text_843351C::
	.string "When you’re this far away from the\n"
	.string "city, the sky at night is fantastic.$"
