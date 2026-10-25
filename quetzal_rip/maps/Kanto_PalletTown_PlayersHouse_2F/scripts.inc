Kanto_PalletTown_PlayersHouse_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PalletTown_PlayersHouse_2F_EventScript_8418ABC
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_PalletTown_PlayersHouse_2F_OnWarp_8418ACF
	.byte 0

Kanto_PalletTown_PlayersHouse_2F_OnWarp_8418ACF:
	map_script_2 VAR_QZ_0x5119, 0, Kanto_PalletTown_PlayersHouse_2F_EventScript_8418AD9
	.2byte 0

Kanto_PalletTown_PlayersHouse_2F_EventScript_8418ABC::
	setflag FLAG_QZ_0x2E14
	call_if_eq VAR_QZ_0x5119, 0, Kanto_PalletTown_PlayersHouse_2F_EventScript_8418ACB
	end

Kanto_PalletTown_PlayersHouse_2F_EventScript_8418ACB::
	setrespawn 0x2F
	return

Kanto_PalletTown_PlayersHouse_2F_EventScript_8418AD9::
	turnobject LOCALID_PLAYER, 2
	setvar VAR_QZ_0x5119, 1
	end

@ Kanto_PalletTown_PlayersHouse_2F sign / bg event at 6,5
Kanto_PalletTown_PlayersHouse_2F_EventScript_8418AE3::
	msgbox Kanto_PalletTown_PlayersHouse_2F_Text_843D823, MSGBOX_SIGN
	end

@ Kanto_PalletTown_PlayersHouse_2F sign / bg event at 11,1
Kanto_PalletTown_PlayersHouse_2F_EventScript_8418AEC::
	msgbox Kanto_PalletTown_PlayersHouse_2F_Text_843D853, MSGBOX_SIGN
	end

@ Kanto_PalletTown_PlayersHouse_2F sign / bg event at 1,1
Kanto_PalletTown_PlayersHouse_2F_EventScript_8418AF5::
	end

@ Kanto_PalletTown_PlayersHouse_2F sign / bg event at 2,5
@ Kanto_PalletTown_PlayersHouse_2F sign / bg event at 2,6
Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B06::
	lockall
	msgbox Kanto_PalletTown_PlayersHouse_2F_Text_8418B77, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B74
	closemessage
	special QzPalletPostgameEventEligibility_768
	goto_if_eq VAR_RESULT, 1, Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B3D
	fadescreen 1
	special QzHealPlayerPartyEx_0
	fadescreen 0
	msgbox Kanto_PalletTown_PlayersHouse_2F_Text_8418B90, MSGBOX_DEFAULT
	goto Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B74

Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B3D::
	fadescreen 1
	special QzHealPlayerPartyEx_0
	special QzResetFollowerObjects_572
	setobjectxy LOCALID_PLAYER, 3, 6
	turnobject LOCALID_PLAYER, 4
	clearflag FLAG_QZ_0x2E14
	addobject 1
	fadescreen 0
	playmoncry SPECIES_ARCEUS, 2
	waitmoncry
	msgbox Kanto_PalletTown_PlayersHouse_2F_Text_8418BA1, MSGBOX_DEFAULT
	closemessage
	fadescreen 3
	removeobject 1
	setflag FLAG_QZ_0x2E14
	setflag FLAG_QZ_0x2E13
	fadescreen 2

Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B74::
	closemessage
	releaseall
	end

Kanto_PalletTown_PlayersHouse_2F_Text_8418B77::
	.string "Would you like to sleep?$"

Kanto_PalletTown_PlayersHouse_2F_Text_8418B90::
	.string "You feel rested.$"

Kanto_PalletTown_PlayersHouse_2F_Text_8418BA1::
	.string "I have witnessed the bond\n"
	.string "between you and your Pokémon.\p"
	.string "Your journey is not yet over.\p"
	.string "We shall meet again.$"

Kanto_PalletTown_PlayersHouse_2F_Text_843D823::
	.string "{PLAYER} played with the NES.\p"
	.string "…Okay!\n"
	.string "It’s time to go!$"

Kanto_PalletTown_PlayersHouse_2F_Text_843D853::
	.string "It’s a posted notice…\p"
	.string "If you’re confused, ask for HELP!\n"
	.string "Press the L or R Button!$"
