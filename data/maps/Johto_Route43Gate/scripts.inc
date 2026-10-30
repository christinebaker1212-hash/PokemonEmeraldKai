Johto_Route43Gate_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_Route43Gate_OnFrame_83D1F0B
	.byte 0

Johto_Route43Gate_OnFrame_83D1F0B:
	map_script_2 VAR_TEMP_0, 0, Johto_Route43Gate_EventScript_83D1F15
	.2byte 0

Johto_Route43Gate_EventScript_83D1F15::
	setvar VAR_TEMP_0, 1
	goto_if_eq VAR_QZ_0x500C, 1, Johto_Route43Gate_EventScript_83D1F52
	goto_if_eq VAR_QZ_0x500C, 2, Johto_Route43Gate_EventScript_83D1F52
	goto_if_eq VAR_QZ_0x500C, 3, Johto_Route43Gate_EventScript_83D1F52
	goto_if_eq VAR_QZ_0x500C, 4, Johto_Route43Gate_EventScript_83D1F52
	goto_if_eq VAR_QZ_0x500C, 5, Johto_Route43Gate_EventScript_83D1F52
	end

Johto_Route43Gate_EventScript_83D1F52::
	lock
	showmoneybox 0, 0, 0
	msgbox Johto_Route43Gate_Text_83D200F, MSGBOX_DEFAULT
	checkmoney 0x1F4, 0
	goto_if_eq VAR_RESULT, 0, Johto_Route43Gate_EventScript_83D1F8A
	removemoney 0x1F4, 0
	playse SE_SHOP
	updatemoneybox 0
	msgbox Johto_Route43Gate_Text_83D2045, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

Johto_Route43Gate_EventScript_83D1F8A::
	checkmoney 1, 0
	goto_if_eq VAR_RESULT, 0, Johto_Route43Gate_EventScript_83D1FBD
	msgbox Johto_Route43Gate_Text_83D205A, MSGBOX_DEFAULT
	removemoney 0x1F4, 0
	playse SE_SHOP
	updatemoneybox 0
	msgbox Johto_Route43Gate_Text_83D2045, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

Johto_Route43Gate_EventScript_83D1FBD::
	msgbox Johto_Route43Gate_Text_83D207A, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	release
	end

@ Johto_Route43Gate object 3 (local id 3, gfx 0x3EA9) at 3,5
Johto_Route43Gate_EventScript_83D1FCB::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1052, Johto_Route43Gate_EventScript_83D1FFB
	msgbox Johto_Route43Gate_Text_83D214A, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_BLACK_SLUDGE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1052
	closemessage
	release
	end

Johto_Route43Gate_EventScript_83D1FFB::
	msgbox Johto_Route43Gate_Text_83D21D9, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route43Gate object 1 (local id 1, gfx 0x3EAF) at 7,3
@ Johto_Route43Gate object 2 (local id 2, gfx 0x3EB0) at 7,8
Johto_Route43Gate_EventScript_83D2006::
	msgbox Johto_Route43Gate_Text_83D20EF, MSGBOX_NPC
	end

Johto_Route43Gate_Text_83D200F::
	.string "Hold it there, kiddo!\p"
	.string "The toll is ¥500 to go\n"
	.string "through.$"

Johto_Route43Gate_Text_83D2045::
	.string "Thank you very much!$"

Johto_Route43Gate_Text_83D205A::
	.string "Then pay what you have, please.$"

Johto_Route43Gate_Text_83D207A::
	.string "Wow!\p"
	.string "You’re totally broke!\p"
	.string "I almost feel sorry for you.\p"
	.string "Okay, you can pass...\p"
	.string "Just don’t tell the other guard,\n"
	.string "okay?$"

Johto_Route43Gate_Text_83D20EF::
	.string "He-he-he. We’re making a bundle.\p"
	.string "Everyone wants to see what’s going\n"
	.string "on up at LAKE OF RAGE.$"

Johto_Route43Gate_Text_83D214A::
	.string "I got chased from my post by these\n"
	.string "thugs in black.\p"
	.string "They left this behind.\p"
	.string "It makes me uncomfortable. Could\n"
	.string "you take it away?$"

Johto_Route43Gate_Text_83D21D9::
	.string "Use this gate to avoid walking in\n"
	.string "the grass.$"
