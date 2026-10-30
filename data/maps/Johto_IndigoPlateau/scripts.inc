Johto_IndigoPlateau_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IndigoPlateau_EventScript_83DCE12
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_IndigoPlateau_EventScript_83DCE12::
	special QzJohtoTravelingEventCheck
	special QzJohtoStaticEncountersRefresh
	end

@ Johto_IndigoPlateau sign / bg event at 13,7
Johto_IndigoPlateau_EventScript_83DCE19::
	msgbox Johto_IndigoPlateau_Text_83DCE22, MSGBOX_SIGN
	end

Johto_IndigoPlateau_Text_83DCE22::
	.string "INDIGO PLATEAU\p"
	.string "The Ultimate Goal for Trainers!\p"
	.string "POKéMON LEAGUE HQ$"
