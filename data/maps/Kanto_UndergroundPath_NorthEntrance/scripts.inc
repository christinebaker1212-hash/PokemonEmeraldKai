Kanto_UndergroundPath_NorthEntrance_MapScripts::
	.byte 0

@ Kanto_UndergroundPath_NorthEntrance object 1 (local id 1, gfx 0x3F18) at 5,6
Kanto_UndergroundPath_NorthEntrance_EventScript_8410AC6::
	lock
	faceplayer
	setvar VAR_0x8008, 9
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x212B, Kanto_UndergroundPath_NorthEntrance_EventScript_8410B43
	goto_if_eq VAR_RESULT, 0, Kanto_UndergroundPath_NorthEntrance_EventScript_8410B2B
	msgbox Quetzal_Text_8455DD4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_UndergroundPath_NorthEntrance_EventScript_8410B2B
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_UndergroundPath_NorthEntrance_EventScript_8410B2B
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_UndergroundPath_NorthEntrance_EventScript_8410B35
	call Quetzal_EventScript_845919C
	msgbox Quetzal_Text_8455E48, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x212B
	release
	end

Kanto_UndergroundPath_NorthEntrance_EventScript_8410B2B::
	msgbox Quetzal_Text_8455E06, MSGBOX_DEFAULT
	release
	end

Kanto_UndergroundPath_NorthEntrance_EventScript_8410B35::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455E16, MSGBOX_DEFAULT
	release
	end

Kanto_UndergroundPath_NorthEntrance_EventScript_8410B43::
	msgbox Quetzal_Text_8455E5E, MSGBOX_DEFAULT
	release
	end
