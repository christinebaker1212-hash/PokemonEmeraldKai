Johto_Route31VioletGate_MapScripts::
	.byte 0

@ Johto_Route31VioletGate object 1 (local id 1, gfx 0x3EA9) at 6,2
Johto_Route31VioletGate_EventScript_83D1E82::
	lock
	faceplayer
	msgbox Johto_Route31VioletGate_Text_83D1E9C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route31VioletGate object 2 (local id 2, gfx 0x3E85) at 8,4
Johto_Route31VioletGate_EventScript_83D1E8F::
	lock
	faceplayer
	msgbox Johto_Route31VioletGate_Text_83D1EC2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route31VioletGate trigger at 1,5 (VAR_QZ_0x5008 == 0)
Johto_Route31VioletGate_EventScript_83FE8CE::
	setflag FLAG_QZ_0x11BC
	setvar VAR_QZ_0x5008, 1
	end

Johto_Route31VioletGate_Text_83D1E9C::
	.string "Hi there! Did you visit\n"
	.string "SPROUT TOWER?$"

Johto_Route31VioletGate_Text_83D1EC2::
	.string "I saw the cutest POKéMON out\n"
	.string "there.\p"
	.string "It looked a lot like PIKACHU!$"
