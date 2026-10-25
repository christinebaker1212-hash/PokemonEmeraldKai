Kanto_SSAnne_1F_Room4_MapScripts::
	.byte 0

@ Kanto_SSAnne_1F_Room4 object 1 (local id 1, gfx 0x3F57) at 2,4
Kanto_SSAnne_1F_Room4_EventScript_84107BD::
	lock
	faceplayer
	checkplayergender
	goto_if_eq VAR_RESULT, 0, Kanto_SSAnne_1F_Room4_EventScript_84107D5
	msgbox Kanto_SSAnne_1F_Room4_Text_8424159, MSGBOX_DEFAULT
	release
	end

Kanto_SSAnne_1F_Room4_EventScript_84107D5::
	msgbox Kanto_SSAnne_1F_Room4_Text_842412E, MSGBOX_DEFAULT
	release
	end

Kanto_SSAnne_1F_Room4_Text_842412E::
	.string "Waiter, I would like a cherry pie,\n"
	.string "please!$"

Kanto_SSAnne_1F_Room4_Text_8424159::
	.string "Waitress, I would like a cherry\n"
	.string "pie, please!$"
