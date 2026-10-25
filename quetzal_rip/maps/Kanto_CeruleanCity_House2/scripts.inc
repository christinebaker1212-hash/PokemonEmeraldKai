Kanto_CeruleanCity_House2_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_House2 object 1 (local id 1, gfx 0x3F0C) at 1,2
Kanto_CeruleanCity_House2_EventScript_841A974::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x216E, Kanto_CeruleanCity_House2_EventScript_841A989
	msgbox Kanto_CeruleanCity_House2_Text_8442119, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_House2_EventScript_841A989::
	msgbox Kanto_CeruleanCity_House2_Text_84421C9, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeruleanCity_House2 object 2 (local id 2, gfx 0x3F16) at 7,6
Kanto_CeruleanCity_House2_EventScript_841A993::
	msgbox Kanto_CeruleanCity_House2_Text_844221C, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_House2 sign / bg event at 4,1
Kanto_CeruleanCity_House2_EventScript_841A99C::
	msgbox Kanto_CeruleanCity_House2_Text_8442256, MSGBOX_SIGN
	end

Kanto_CeruleanCity_House2_Text_8442119::
	.string "Those miserable ROCKETS!\p"
	.string "Look what they’ve done to my\n"
	.string "house!\p"
	.string "They stole a valuable SUN STONE!\p"
	.string "I was going to use it to evolve\n"
	.string "one of my POKéMON.\p"
	.string "That cost me a bundle, it did!$"

Kanto_CeruleanCity_House2_Text_84421C9::
	.string "I figure what’s lost is lost.\p"
	.string "I decided to train my DIGLETT the\n"
	.string "old-fashioned way.$"

Kanto_CeruleanCity_House2_Text_844221C::
	.string "TEAM ROCKET must be trying to DIG\n"
	.string "their way into no good!$"

Kanto_CeruleanCity_House2_Text_8442256::
	.string "TEAM ROCKET left a way out!$"
