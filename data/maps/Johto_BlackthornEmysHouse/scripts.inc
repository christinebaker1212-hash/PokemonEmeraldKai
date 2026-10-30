Johto_BlackthornEmysHouse_MapScripts::
	.byte 0

@ Johto_BlackthornEmysHouse object 1 (local id 1, gfx 0x10) at 2,4
Johto_BlackthornEmysHouse_EventScript_83C8009::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10D3, Johto_BlackthornEmysHouse_EventScript_83C80AF
	setvar VAR_0x8008, 7
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	goto_if_eq VAR_RESULT, 0, Johto_BlackthornEmysHouse_EventScript_83C8097
	copyvar VAR_0x8009, VAR_RESULT
	msgbox Johto_BlackthornEmysHouse_Text_83C80B9, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_BlackthornEmysHouse_EventScript_83C8097
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_BlackthornEmysHouse_EventScript_83C8097
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Johto_BlackthornEmysHouse_EventScript_83C80A1
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_BlackthornEmysHouse_Text_83C80FA, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x10D3
	releaseall
	end

Johto_BlackthornEmysHouse_EventScript_83C8097::
	msgbox Johto_BlackthornEmysHouse_Text_83C814E, MSGBOX_DEFAULT
	release
	end

Johto_BlackthornEmysHouse_EventScript_83C80A1::
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_BlackthornEmysHouse_Text_83C8123, MSGBOX_DEFAULT
	release
	end

Johto_BlackthornEmysHouse_EventScript_83C80AF::
	msgbox Johto_BlackthornEmysHouse_Text_83C816E, MSGBOX_DEFAULT
	release
	end

Johto_BlackthornEmysHouse_Text_83C80B9::
	.string "I collect POKéMON!\n"
	.string "Do you have a {STR_VAR_1}?\p"
	.string "Want to trade it for my\n"
	.string "{STR_VAR_2}?$"

Johto_BlackthornEmysHouse_Text_83C80FA::
	.string "Yay! I got myself {STR_VAR_1}!\p"
	.string "{STR_VAR_2}, you take care!$"

Johto_BlackthornEmysHouse_Text_83C8123::
	.string "Uh, no, I don’t think so.\n"
	.string "That isn’t a {STR_VAR_1}.$"

Johto_BlackthornEmysHouse_Text_83C814E::
	.string "You don’t want to trade? Aww...$"

Johto_BlackthornEmysHouse_Text_83C816E::
	.string "Hi, how’s my SLIGGOO doing?$"
