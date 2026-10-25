Johto_EcruteakCityNorth_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_EcruteakCityNorth object 2 (local id 2, gfx 0x3EA3) at 54,59
Johto_EcruteakCityNorth_EventScript_83C6720::
	lock
	faceplayer
	msgbox Johto_EcruteakCityNorth_Text_83C672D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCityNorth object 1 (local id 1, gfx 0x3A9E) at 12,55
Johto_EcruteakCityNorth_EventScript_83C6817::
	setorcopyvar VAR_0x8000, 0x8A
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_EcruteakCityNorth_Text_83C672D::
	.string "MORTY: The legendary POKéMON\n"
	.string "did not choose me...\p"
	.string "But that does not mean that I\n"
	.string "have lost out on my future.\p"
	.string "I will continue my training until I\n"
	.string "I discover a new future for me.\p"
	.string "If you seek another battle, find\n"
	.string "me at the FIGHTING DOJO.$"
