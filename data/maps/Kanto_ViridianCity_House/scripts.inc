Kanto_ViridianCity_House_MapScripts::
	.byte 0

@ Kanto_ViridianCity_House object 1 (local id 1, gfx 0x3EE6) at 7,4
Kanto_ViridianCity_House_EventScript_8419E89::
	msgbox Kanto_ViridianCity_House_Text_843F646, MSGBOX_NPC
	end

@ Kanto_ViridianCity_House object 2 (local id 2, gfx 0x3F18) at 2,5
Kanto_ViridianCity_House_EventScript_8419E92::
	msgbox Kanto_ViridianCity_House_Text_843F6C3, MSGBOX_NPC
	end

@ Kanto_ViridianCity_House object 3 (local id 3, gfx 0x3F49) at 6,6
Kanto_ViridianCity_House_EventScript_8419E9B::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_SPEAROW, 0
	msgbox Kanto_ViridianCity_House_Text_843F6E0, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_ViridianCity_House sign / bg event at 7,1
Kanto_ViridianCity_House_EventScript_8419EAE::
	msgbox Kanto_ViridianCity_House_Text_843F6F1, MSGBOX_SIGN
	end

Kanto_ViridianCity_House_Text_843F646::
	.string "Coming up with nicknames is fun,\n"
	.string "but it’s not so easy to do.\p"
	.string "Clever names are nice, but simple\n"
	.string "names are easier to remember.$"

Kanto_ViridianCity_House_Text_843F6C3::
	.string "My daddy loves POKéMON, too.$"

Kanto_ViridianCity_House_Text_843F6E0::
	.string "SPEARY: Tetweet!$"

Kanto_ViridianCity_House_Text_843F6F1::
	.string "SPEAROW\n"
	.string "Name: SPEARY$"
