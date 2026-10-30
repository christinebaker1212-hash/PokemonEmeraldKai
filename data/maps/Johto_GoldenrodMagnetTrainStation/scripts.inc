Johto_GoldenrodMagnetTrainStation_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_GoldenrodMagnetTrainStation_EventScript_83D9A13
	.byte 0

Johto_GoldenrodMagnetTrainStation_EventScript_83D9A13::
	setvar VAR_QZ_0x5016, 0
	goto Quetzal_EventScript_840EA66

@ Johto_GoldenrodMagnetTrainStation object 1 (local id 1, gfx 0x3EA9) at 16,20
Johto_GoldenrodMagnetTrainStation_EventScript_83D9A1D::
	lock
	faceplayer
	msgbox Johto_GoldenrodMagnetTrainStation_Text_83D9A4F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodMagnetTrainStation object 2 (local id 2, gfx 0x30) at 20,23
Johto_GoldenrodMagnetTrainStation_EventScript_83D9A2A::
	lock
	faceplayer
	msgbox Johto_GoldenrodMagnetTrainStation_Text_83D9AA1, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodMagnetTrainStation object 8 (local id 8, gfx 0xB) at 10,22
Johto_GoldenrodMagnetTrainStation_EventScript_83D9A37::
	lock
	msgbox Johto_GoldenrodMagnetTrainStation_Text_83D9B2E, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodMagnetTrainStation object 9 (local id 9, gfx 0x25) at 11,22
Johto_GoldenrodMagnetTrainStation_EventScript_83D9A43::
	lock
	msgbox Johto_GoldenrodMagnetTrainStation_Text_83D9B74, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodMagnetTrainStation_Text_83D9A4F::
	.string "The MAGNET TRAIN has broken down.\p"
	.string "Service is suspended indefinitely.\n"
	.string "We’re sorry.$"

Johto_GoldenrodMagnetTrainStation_Text_83D9AA1::
	.string "I’m the PRESIDENT.\p"
	.string "My dream was to link JOHTO and KANTO\n"
	.string "with a train faster than a POKéMON.\p"
	.string "But the service has been suspended\n"
	.string "indefinitely.$"

Johto_GoldenrodMagnetTrainStation_Text_83D9B2E::
	.string "I wanted to see the MAGNET TRAIN go!\n"
	.string "It’s a shame it has broken down.$"

Johto_GoldenrodMagnetTrainStation_Text_83D9B74::
	.string "They say I can see the future…\p"
	.string "But I didn’t see this breakdown coming!$"
