Johto_AzaleaPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_AzaleaPokecenter1F_EventScript_83C6545
	.byte 0

Johto_AzaleaPokecenter1F_EventScript_83C6545::
	setrespawn 0x1A
	end

@ Johto_AzaleaPokecenter1F object 5 (local id 5, gfx 0x30) at 12,5
Johto_AzaleaPokecenter1F_EventScript_83C6549::
	lock
	faceplayer
	msgbox Johto_AzaleaPokecenter1F_Text_83C6570, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaPokecenter1F object 4 (local id 4, gfx 0x3E95) at 13,3
Johto_AzaleaPokecenter1F_EventScript_83C6556::
	lock
	faceplayer
	msgbox Johto_AzaleaPokecenter1F_Text_83C6635, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaPokecenter1F object 3 (local id 3, gfx 0x18) at 3,6
Johto_AzaleaPokecenter1F_EventScript_83C6563::
	lock
	faceplayer
	msgbox Johto_AzaleaPokecenter1F_Text_83C6663, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_AzaleaPokecenter1F_Text_83C6570::
	.string "Do your POKéMON know HM moves?\p"
	.string "Those moves can be used even if\n"
	.string "your POKéMON has fainted.$"

Johto_AzaleaPokecenter1F_Text_83C6635::
	.string "BILL’s PC can store up to 30\n"
	.string "POKéMON per BOX.$"

Johto_AzaleaPokecenter1F_Text_83C6663::
	.string "Do you know about BERRIES?\p"
	.string "Crack one open, hollow it out and\n"
	.string "fit it with a special device.\p"
	.string "Then you can catch POKéMON with it.\p"
	.string "Before POKé BALLS were invented,\n"
	.string "everyone used BERRIES.$"
