Johto_OlivineTimsHouse_MapScripts::
	.byte 0

@ Johto_OlivineTimsHouse object 1 (local id 1, gfx 0x3E91) at 4,4
Johto_OlivineTimsHouse_EventScript_83E5513::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10BC, Johto_OlivineTimsHouse_EventScript_83E55B9
	setvar VAR_0x8008, 6
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	goto_if_eq VAR_RESULT, 0, Johto_OlivineTimsHouse_EventScript_83E55A1
	copyvar VAR_0x8009, VAR_RESULT
	msgbox Johto_OlivineTimsHouse_Text_83E55C3, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_OlivineTimsHouse_EventScript_83E55A1
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, 255, Johto_OlivineTimsHouse_EventScript_83E55A1
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Johto_OlivineTimsHouse_EventScript_83E55AB
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_OlivineTimsHouse_Text_83E5604, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x10BC
	release
	end

Johto_OlivineTimsHouse_EventScript_83E55A1::
	msgbox Johto_OlivineTimsHouse_Text_83E5658, MSGBOX_DEFAULT
	release
	end

Johto_OlivineTimsHouse_EventScript_83E55AB::
	bufferspeciesname 0, VAR_0x8009
	msgbox Johto_OlivineTimsHouse_Text_83E562D, MSGBOX_DEFAULT
	release
	end

Johto_OlivineTimsHouse_EventScript_83E55B9::
	setvar VAR_0x8004, 6
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo   @ (the names Crystal's words use: audit case 58)
	msgbox Johto_OlivineTimsHouse_Text_83E5678, MSGBOX_DEFAULT
	release
	end

Johto_OlivineTimsHouse_Text_83E55C3::
	.string "Hi, I’m looking for this MOéMON.\p"
	.string "If you have {STR_VAR_1}, would you\n"
	.string "trade it for my {STR_VAR_2}?$"

Johto_OlivineTimsHouse_Text_83E5604::
	.string "Great! Thank you!\p"
	.string "I finally got {STR_VAR_1}.$"

Johto_OlivineTimsHouse_Text_83E562D::
	.string "You don’t have {STR_VAR_1}? That’s\n"
	.string "too bad, then.$"

Johto_OlivineTimsHouse_Text_83E5658::
	.string "You don’t have one either?\p"
	.string "Gee, that’s really disappointing…$"

Johto_OlivineTimsHouse_Text_83E5678::
	.string "Hi! The {STR_VAR_1} you traded me\n"
	.string "is doing great!$"
