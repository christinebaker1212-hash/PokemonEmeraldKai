Kanto_NavelRock_Harbor_MapScripts::
	.byte 0

@ Kanto_NavelRock_Harbor object 2 (local id 2, gfx 0x3F41) at 8,6
Kanto_NavelRock_Harbor_EventScript_841527C::
	lock
	faceplayer
	message Quetzal_Text_8431905
	waitmessage
	setvar VAR_0x8004, 9
	multichoice 0x14, 8, 0x74, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Quetzal_EventScript_84594BB
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_8459493
	goto_if_eq VAR_0x8000, 127, Quetzal_EventScript_8459493
	end
