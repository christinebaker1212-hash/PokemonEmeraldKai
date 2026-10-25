Johto_TohjoFalls_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_TohjoFalls object 1 (local id 1, gfx 0x3B) at 15,11
Johto_TohjoFalls_EventScript_83FBFD7::
	setorcopyvar VAR_0x8000, 0x98
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TohjoFalls object 4 (local id 4, gfx 0x3E95) at 33,22
Johto_TohjoFalls_EventScript_83FBFE4::
	lock
	faceplayer
	checkitem ITEM_SUPER_ROD, 1
	goto_if_eq VAR_RESULT, 1, Johto_TohjoFalls_EventScript_83FC01F
	msgbox Johto_TohjoFalls_Text_83FC029, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x2C7
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_TohjoFalls_Text_83FC0A9, MSGBOX_DEFAULT
	release
	end

Johto_TohjoFalls_EventScript_83FC01F::
	msgbox Johto_TohjoFalls_Text_83FC0EB, MSGBOX_DEFAULT
	release
	end

Johto_TohjoFalls_Text_83FC029::
	.string "You’ve made it to TOHJO FALLS.\p"
	.string "To fish in waters this challenging,\n"
	.string "you need a ROD for the challenge.\p"
	.string "Here, take this SUPER ROD!$"

Johto_TohjoFalls_Text_83FC0A9::
	.string "With the SUPER ROD, you can hook\n"
	.string "POKéMON from the deepest waters.$"

Johto_TohjoFalls_Text_83FC0EB::
	.string "Use the SUPER ROD in deep water.\p"
	.string "You never know what POKéMON\n"
	.string "might bite!$"
