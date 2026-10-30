Kanto_Route2_House_MapScripts::
	.byte 0

@ Kanto_Route2_House object 1 (local id 1, gfx 0x3F43) at 4,5
Kanto_Route2_House_EventScript_841F565::
	msgbox Kanto_Route2_House_Text_844DAE5, MSGBOX_NPC
	end

@ Kanto_Route2_House object 2 (local id 2, gfx 0x3F08) at 7,2
Kanto_Route2_House_EventScript_841F56E::
	lock
	faceplayer
	setvar VAR_0x8008, 0x10
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x212A, Kanto_Route2_House_EventScript_841F5EB
	goto_if_eq VAR_RESULT, 0, Kanto_Route2_House_EventScript_841F5D3
	msgbox Quetzal_Text_8455C79, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_Route2_House_EventScript_841F5D3
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_Route2_House_EventScript_841F5D3
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_Route2_House_EventScript_841F5DD
	call Quetzal_EventScript_845919C
	msgbox Quetzal_Text_8455CD8, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x212A
	release
	end

Kanto_Route2_House_EventScript_841F5D3::
	msgbox Quetzal_Text_8455CB4, MSGBOX_DEFAULT
	release
	end

Kanto_Route2_House_EventScript_841F5DD::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455CC4, MSGBOX_DEFAULT
	release
	end

Kanto_Route2_House_EventScript_841F5EB::
	msgbox Quetzal_Text_8455CE5, MSGBOX_DEFAULT
	release
	end

Kanto_Route2_House_Text_844DAE5::
	.string "A fainted POKéMON just has no\n"
	.string "energy left to battle.\p"
	.string "It can still use moves like CUT\n"
	.string "outside of battle.$"
