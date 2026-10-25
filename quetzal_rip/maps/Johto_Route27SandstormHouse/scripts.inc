Johto_Route27SandstormHouse_MapScripts::
	.byte 0

@ Johto_Route27SandstormHouse object 1 (local id 1, gfx 0x1E) at 4,4
Johto_Route27SandstormHouse_EventScript_83EC99A::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x105C, Johto_Route27SandstormHouse_EventScript_83EC9B0
	msgbox Johto_Route27SandstormHouse_Text_83ECA2A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route27SandstormHouse_EventScript_83EC9B0::
	msgbox Johto_Route27SandstormHouse_Text_83EC9DD, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1C1
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x105C
	msgbox Johto_Route27SandstormHouse_Text_83ECA2A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route27SandstormHouse_Text_83EC9DD::
	.string "Heading to the POKéMON LEAGUE?\p"
	.string "Here. This gift may help you\n"
	.string "on your journey.$"

Johto_Route27SandstormHouse_Text_83ECA2A::
	.string "A SMOOTH ROCK extends the\n"
	.string "sandstorm its holder creates.$"
