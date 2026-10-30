Kanto_FuchsiaCity_SafariZone_Entrance_MapScripts::
	.byte 0

@ Kanto_FuchsiaCity_SafariZone_Entrance object 2 (local id 2, gfx 0x3F5B) at 1,6
Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_841D640::
	lock
	faceplayer
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_8447F46, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_841D65F
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_844802B, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_SafariZone_Entrance_EventScript_841D65F::
	msgbox Kanto_FuchsiaCity_SafariZone_Entrance_Text_8447F79, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_SafariZone_Entrance_Text_8447F46::
	.string "Hi! Is it your first time here at\n"
	.string "the SAFARI ZONE?$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_8447F79::
	.string "The SAFARI ZONE has four areas.\p"
	.string "Each has different wild POKéMON,\n"
	.string "including some rare species.\p"
	.string "Battles work normally, so bring\n"
	.string "your own POKé BALLS.\p"
	.string "There is no fee or time limit.$"

Kanto_FuchsiaCity_SafariZone_Entrance_Text_844802B::
	.string "Then you know the way. Have fun!$"
