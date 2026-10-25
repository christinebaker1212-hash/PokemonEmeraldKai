Johto_VioletKylesHouse_MapScripts::
	.byte 0

@ Johto_VioletKylesHouse object 1 (local id 1, gfx 0x37) at 4,4
Johto_VioletKylesHouse_EventScript_83FF94E::
	lock
	faceplayer
	msgbox Johto_VioletKylesHouse_Text_83FFADB, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletKylesHouse object 2 (local id 2, gfx 0x3E87) at 7,5
Johto_VioletKylesHouse_EventScript_83FF95B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1027, Johto_VioletKylesHouse_EventScript_83FFA01
	setvar VAR_0x8008, 4
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	goto_if_eq VAR_RESULT, 0, Johto_VioletKylesHouse_EventScript_83FF9E9
	copyvar VAR_0x8009, VAR_RESULT
	msgbox Johto_VioletKylesHouse_Text_83FFA0B, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_VioletKylesHouse_EventScript_83FF9E9
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_VioletKylesHouse_EventScript_83FF9E9
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Johto_VioletKylesHouse_EventScript_83FF9F3
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_VioletKylesHouse_Text_83FFA4C, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1027
	release
	end

Johto_VioletKylesHouse_EventScript_83FF9E9::
	msgbox Johto_VioletKylesHouse_Text_83FFAA0, MSGBOX_DEFAULT
	release
	end

Johto_VioletKylesHouse_EventScript_83FF9F3::
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_VioletKylesHouse_Text_83FFA75, MSGBOX_DEFAULT
	release
	end

Johto_VioletKylesHouse_EventScript_83FFA01::
	msgbox Johto_VioletKylesHouse_Text_83FFAC0, MSGBOX_DEFAULT
	release
	end

Johto_VioletKylesHouse_Text_83FFA0B::
	.string "I collect POKéMON!\n"
	.string "Do you have a {STR_VAR_1}?\p"
	.string "Want to trade it for my\n"
	.string "{STR_VAR_2}?$"

Johto_VioletKylesHouse_Text_83FFA4C::
	.string "Yay! I got myself {STR_VAR_1}!\p"
	.string "{STR_VAR_2}, you take care!$"

Johto_VioletKylesHouse_Text_83FFA75::
	.string "Uh, no, I don’t think so.\n"
	.string "That isn’t a {STR_VAR_1}.$"

Johto_VioletKylesHouse_Text_83FFAA0::
	.string "You don’t want to trade? Aww...$"

Johto_VioletKylesHouse_Text_83FFAC0::
	.string "Hi, how’s my SPHEAL doing?$"

Johto_VioletKylesHouse_Text_83FFADB::
	.string "A POKéMON you get in a trade\n"
	.string "grows quickly.\p"
	.string "But if you don’t have the right\n"
	.string "GYM BADGE, they may disobey you.$"
