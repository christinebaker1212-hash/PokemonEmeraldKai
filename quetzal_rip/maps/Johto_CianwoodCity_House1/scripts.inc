Johto_CianwoodCity_House1_MapScripts::
	.byte 0

@ Johto_CianwoodCity_House1 object 1 (local id 1, gfx 0x1E) at 4,4
Johto_CianwoodCity_House1_EventScript_83CAC29::
	lock
	faceplayer
	msgbox Johto_CianwoodCity_House1_Text_83CAC79, MSGBOX_DEFAULT
	checkitem ITEM_TIDAL_BELL, 1
	goto_if_eq VAR_RESULT, 1, Johto_CianwoodCity_House1_EventScript_83CAC48
	goto Johto_CianwoodCity_House1_EventScript_83CAC53

Johto_CianwoodCity_House1_EventScript_83CAC48::
	msgbox Johto_CianwoodCity_House1_Text_83CAF9C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodCity_House1_EventScript_83CAC53::
	checkitem ITEM_CLEAR_BELL, 1
	goto_if_eq VAR_RESULT, 1, Johto_CianwoodCity_House1_EventScript_83CAC6E
	msgbox Johto_CianwoodCity_House1_Text_83CAD8C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodCity_House1_EventScript_83CAC6E::
	msgbox Johto_CianwoodCity_House1_Text_83CAED4, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CianwoodCity_House1_Text_83CAC79::
	.string "I don’t remember the last time\n"
	.string "someone came to visit me.\p"
	.string "When I was young, I was a\n"
	.string "POKéMON Trainer, too.\l"
	.string "I tried to reach the tower and\l"
	.string "dive into the ocean.\p"
	.string "I put a lot of effort into many\n"
	.string "things.\p"
	.string "Well, I take things easy now,\n"
	.string "listening to the waves and winds\l"
	.string "on this beach.$"

Johto_CianwoodCity_House1_Text_83CAD8C::
	.string "Have you heard of the TIN TOWER\n"
	.string "in ECRUTEAK?\p"
	.string "There are what they call Clear Bells\n"
	.string "on top of the tower.\p"
	.string "They don’t make a single noise\n"
	.string "in strong winds.\p"
	.string "There is only one way to make them\n"
	.string "ring.\p"
	.string "It is to obtain another CLEAR BELL\n"
	.string "just like the ones at the top of the\l"
	.string "tower.\p"
	.string "I went after it when I was young.\n"
	.string "But I never got one...$"

Johto_CianwoodCity_House1_Text_83CAED4::
	.string "Oh! You’ve managed to get a\n"
	.string "CLEAR BELL?\p"
	.string "The old-time dream of mine...\n"
	.string "I see...\p"
	.string "You’ve done it...\n"
	.string "...\l"
	.string "...I can hear it!\p"
	.string "The chime of the bells on the wind\n"
	.string "coming from the direction of the\l"
	.string "TIN TOWER...$"

Johto_CianwoodCity_House1_Text_83CAF9C::
	.string "Oh! You’ve managed to get a\n"
	.string "Tidal Bell?\p"
	.string "The old-time dream of mine...\n"
	.string "I see...\p"
	.string "You’ve done it...\n"
	.string "...\l"
	.string "...I can hear it!\p"
	.string "The chime of the bells on the sea\n"
	.string "breeze coming from the direction of\l"
	.string "the WHIRL ISLANDS...$"
