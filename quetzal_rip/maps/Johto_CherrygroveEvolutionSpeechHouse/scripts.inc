Johto_CherrygroveEvolutionSpeechHouse_MapScripts::
	.byte 0

@ Johto_CherrygroveEvolutionSpeechHouse object 1 (local id 1, gfx 0x23) at 1,5
Johto_CherrygroveEvolutionSpeechHouse_EventScript_83C98A8::
	lock
	faceplayer
	msgbox Johto_CherrygroveEvolutionSpeechHouse_Text_83C98C2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CherrygroveEvolutionSpeechHouse object 2 (local id 2, gfx 0x3E85) at 3,5
Johto_CherrygroveEvolutionSpeechHouse_EventScript_83C98B5::
	lock
	faceplayer
	msgbox Johto_CherrygroveEvolutionSpeechHouse_Text_83C98FB, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CherrygroveEvolutionSpeechHouse_Text_83C98C2::
	.string "POKéMON gain experience in\n"
	.string "battle and change their form.$"

Johto_CherrygroveEvolutionSpeechHouse_Text_83C98FB::
	.string "POKéMON change?\p"
	.string "I would be shocked if one did\n"
	.string "that!$"
