Kanto_CeruleanCity_House1_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_House1 object 1 (local id 1, gfx 0x3F09) at 6,5
Kanto_CeruleanCity_House1_EventScript_841A7EE::
	lock
	faceplayer
	msgbox Kanto_CeruleanCity_House1_Text_8441BED, MSGBOX_DEFAULT
	message Kanto_CeruleanCity_House1_Text_8441C6D
	waitmessage
	setvar VAR_0x8004, 0
	special QzListMenu
	waitstate
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeruleanCity_House1_EventScript_841A8F9
	goto_if_eq VAR_0x8000, 1, Kanto_CeruleanCity_House1_EventScript_841A907
	goto_if_eq VAR_0x8000, 2, Kanto_CeruleanCity_House1_EventScript_841A915
	goto_if_eq VAR_0x8000, 3, Kanto_CeruleanCity_House1_EventScript_841A923
	goto_if_eq VAR_0x8000, 4, Kanto_CeruleanCity_House1_EventScript_841A931
	goto_if_eq VAR_0x8000, 5, Kanto_CeruleanCity_House1_EventScript_841A93F
	goto_if_eq VAR_0x8000, 6, Kanto_CeruleanCity_House1_EventScript_841A94D
	goto_if_eq VAR_0x8000, 7, Kanto_CeruleanCity_House1_EventScript_841A95B
	goto_if_eq VAR_0x8000, 8, Kanto_CeruleanCity_House1_EventScript_841A969
	goto_if_eq VAR_0x8000, 127, Kanto_CeruleanCity_House1_EventScript_841A969
	end

Kanto_CeruleanCity_House1_EventScript_841A87B::
	message Kanto_CeruleanCity_House1_Text_8441C6D
	waitmessage
	special QzReturnToListMenu
	waitstate
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeruleanCity_House1_EventScript_841A8F9
	goto_if_eq VAR_0x8000, 1, Kanto_CeruleanCity_House1_EventScript_841A907
	goto_if_eq VAR_0x8000, 2, Kanto_CeruleanCity_House1_EventScript_841A915
	goto_if_eq VAR_0x8000, 3, Kanto_CeruleanCity_House1_EventScript_841A923
	goto_if_eq VAR_0x8000, 4, Kanto_CeruleanCity_House1_EventScript_841A931
	goto_if_eq VAR_0x8000, 5, Kanto_CeruleanCity_House1_EventScript_841A93F
	goto_if_eq VAR_0x8000, 6, Kanto_CeruleanCity_House1_EventScript_841A94D
	goto_if_eq VAR_0x8000, 7, Kanto_CeruleanCity_House1_EventScript_841A95B
	goto_if_eq VAR_0x8000, 8, Kanto_CeruleanCity_House1_EventScript_841A969
	goto_if_eq VAR_0x8000, 127, Kanto_CeruleanCity_House1_EventScript_841A969
	end

Kanto_CeruleanCity_House1_EventScript_841A8F9::
	msgbox Kanto_CeruleanCity_House1_Text_8441CC4, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A907::
	msgbox Kanto_CeruleanCity_House1_Text_8441D37, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A915::
	msgbox Kanto_CeruleanCity_House1_Text_8441DFF, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A923::
	msgbox Kanto_CeruleanCity_House1_Text_8441E6F, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A931::
	msgbox Kanto_CeruleanCity_House1_Text_8441F3C, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A93F::
	msgbox Kanto_CeruleanCity_House1_Text_8441FAF, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A94D::
	msgbox Kanto_CeruleanCity_House1_Text_844207E, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A95B::
	msgbox Kanto_CeruleanCity_House1_Text_84420FE, MSGBOX_DEFAULT
	goto Kanto_CeruleanCity_House1_EventScript_841A87B

Kanto_CeruleanCity_House1_EventScript_841A969::
	msgbox Kanto_CeruleanCity_House1_Text_8441CA4, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_House1_Text_8441BED::
	.string "Only skilled TRAINERS can collect\n"
	.string "POKéMON BADGES.\p"
	.string "I see you have at least one.\p"
	.string "Those BADGES have amazing secrets,\n"
	.string "did you know?$"

Kanto_CeruleanCity_House1_Text_8441C6D::
	.string "Now then…\p"
	.string "Which of the eight BADGES should\n"
	.string "I describe?$"

Kanto_CeruleanCity_House1_Text_8441CA4::
	.string "Come visit me anytime you wish.$"

Kanto_CeruleanCity_House1_Text_8441CC4::
	.string "The ATTACK stat of all your\n"
	.string "POKéMON increases a little bit.\p"
	.string "It also lets you use the move\n"
	.string "FLASH outside of battle.$"

Kanto_CeruleanCity_House1_Text_8441D37::
	.string "POKéMON up to Lv. 30 will obey\n"
	.string "you.\p"
	.string "That includes even outsiders you\n"
	.string "obtain in trades.\p"
	.string "POKéMON of higher levels will\n"
	.string "be unruly in battle, however.\p"
	.string "It also lets you use the move\n"
	.string "CUT outside of battle.$"

Kanto_CeruleanCity_House1_Text_8441DFF::
	.string "The SPEED stat of all your\n"
	.string "POKéMON increases a little bit.\p"
	.string "It also lets you use the move\n"
	.string "FLY outside of battle.$"

Kanto_CeruleanCity_House1_Text_8441E6F::
	.string "POKéMON up to Lv. 50 will obey\n"
	.string "you.\p"
	.string "That includes even outsiders you\n"
	.string "obtain in trades.\p"
	.string "POKéMON of higher levels will\n"
	.string "be unruly in battle, however.\p"
	.string "It also lets you use the move\n"
	.string "STRENGTH outside of battle.$"

Kanto_CeruleanCity_House1_Text_8441F3C::
	.string "The DEFENSE stat of all your\n"
	.string "POKéMON increases a little bit.\p"
	.string "It also lets you use the move\n"
	.string "SURF outside of battle.$"

Kanto_CeruleanCity_House1_Text_8441FAF::
	.string "POKéMON up to Lv. 70 will obey\n"
	.string "you.\p"
	.string "That includes even outsiders you\n"
	.string "obtain in trades.\p"
	.string "POKéMON of higher levels will\n"
	.string "be unruly in battle, however.\p"
	.string "It also lets you use the move\n"
	.string "ROCK SMASH outside of battle.$"

Kanto_CeruleanCity_House1_Text_844207E::
	.string "The SP. ATK and SP. DEF stats of\n"
	.string "all your POKéMON rise a little bit.\p"
	.string "It also lets you use the move\n"
	.string "WATERFALL outside of battle.$"

Kanto_CeruleanCity_House1_Text_84420FE::
	.string "All POKéMON will obey you!$"
