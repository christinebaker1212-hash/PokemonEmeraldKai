Kanto_PalletTown_PlayersHouse_1F_MapScripts::
	.byte 0

@ Kanto_PalletTown_PlayersHouse_1F object 1 (local id 1, gfx 0x3F25) at 8,4
Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A17::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2110, Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A58
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A46
	call_if_eq VAR_RESULT, 1, Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A4F
	closemessage
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A46::
	msgbox Kanto_PalletTown_PlayersHouse_1F_Text_843D608, MSGBOX_DEFAULT
	return

Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A4F::
	msgbox Kanto_PalletTown_PlayersHouse_1F_Text_843D678, MSGBOX_DEFAULT
	return

Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A58::
	msgbox Kanto_PalletTown_PlayersHouse_1F_Text_843D6E9, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_84570E0
	msgbox Kanto_PalletTown_PlayersHouse_1F_Text_843D710, MSGBOX_DEFAULT
	release
	end

@ Kanto_PalletTown_PlayersHouse_1F sign / bg event at 6,1
Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A70::
	lockall
	goto_if_eq VAR_FACING, 2, Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A86
	msgbox Kanto_PalletTown_PlayersHouse_1F_Text_843D811, MSGBOX_DEFAULT
	releaseall
	end

Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A86::
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A9F
	call_if_eq VAR_RESULT, 1, Kanto_PalletTown_PlayersHouse_1F_EventScript_8418AA8
	releaseall
	end

Kanto_PalletTown_PlayersHouse_1F_EventScript_8418A9F::
	msgbox Kanto_PalletTown_PlayersHouse_1F_Text_843D756, MSGBOX_DEFAULT
	return

Kanto_PalletTown_PlayersHouse_1F_EventScript_8418AA8::
	msgbox Kanto_PalletTown_PlayersHouse_1F_Text_843D7AA, MSGBOX_DEFAULT
	return

Kanto_PalletTown_PlayersHouse_1F_Text_843D608::
	.string "MOM: …Right.\n"
	.string "All boys leave home someday.\l"
	.string "It said so on TV.\p"
	.string "Oh, yes. PROF. OAK, next door, was\n"
	.string "looking for you.$"

Kanto_PalletTown_PlayersHouse_1F_Text_843D678::
	.string "MOM: …Right.\n"
	.string "All girls dream of traveling.\l"
	.string "It said so on TV.\p"
	.string "Oh, yes. PROF. OAK, next door, was\n"
	.string "looking for you.$"

Kanto_PalletTown_PlayersHouse_1F_Text_843D6E9::
	.string "MOM: {PLAYER}!\n"
	.string "You should take a quick rest.$"

Kanto_PalletTown_PlayersHouse_1F_Text_843D710::
	.string "MOM: Oh, good! You and your\n"
	.string "POKéMON are looking great.\l"
	.string "Take care now!$"

Kanto_PalletTown_PlayersHouse_1F_Text_843D756::
	.string "There’s a movie on TV.\n"
	.string "Four boys are walking on railroad\l"
	.string "tracks.\p"
	.string "…I better go, too.$"

Kanto_PalletTown_PlayersHouse_1F_Text_843D7AA::
	.string "There’s a movie on TV.\n"
	.string "A girl with her hair in pigtails is\l"
	.string "walking up a brick road.\p"
	.string "…I better go, too.$"

Kanto_PalletTown_PlayersHouse_1F_Text_843D811::
	.string "Oops, wrong side…$"
