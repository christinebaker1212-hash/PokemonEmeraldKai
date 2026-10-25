Kanto_Route18_EastEntrance_2F_MapScripts::
	.byte 0

@ Kanto_Route18_EastEntrance_2F sign / bg event at 4,1
Kanto_Route18_EastEntrance_2F_EventScript_842053C::
	msgbox Kanto_Route18_EastEntrance_2F_Text_845021B, MSGBOX_SIGN
	end

@ Kanto_Route18_EastEntrance_2F sign / bg event at 8,1
Kanto_Route18_EastEntrance_2F_EventScript_8420545::
	msgbox Kanto_Route18_EastEntrance_2F_Text_8450263, MSGBOX_SIGN
	end

@ Kanto_Route18_EastEntrance_2F object 1 (local id 1, gfx 0x3EEE) at 5,3
Kanto_Route18_EastEntrance_2F_EventScript_842054E::
	lock
	faceplayer
	setvar VAR_0x8008, 0xC
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x2129, Kanto_Route18_EastEntrance_2F_EventScript_84205CB
	goto_if_eq VAR_RESULT, 0, Kanto_Route18_EastEntrance_2F_EventScript_84205B3
	msgbox Quetzal_Text_8455C79, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_Route18_EastEntrance_2F_EventScript_84205B3
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_Route18_EastEntrance_2F_EventScript_84205B3
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_Route18_EastEntrance_2F_EventScript_84205BD
	call Quetzal_EventScript_845919C
	msgbox Quetzal_Text_8455CD8, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2129
	release
	end

Kanto_Route18_EastEntrance_2F_EventScript_84205B3::
	msgbox Quetzal_Text_8455CB4, MSGBOX_DEFAULT
	release
	end

Kanto_Route18_EastEntrance_2F_EventScript_84205BD::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455CC4, MSGBOX_DEFAULT
	release
	end

Kanto_Route18_EastEntrance_2F_EventScript_84205CB::
	msgbox Quetzal_Text_8455CE5, MSGBOX_DEFAULT
	release
	end

Kanto_Route18_EastEntrance_2F_Text_845021B::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "PALLET TOWN is in the west.$"

Kanto_Route18_EastEntrance_2F_Text_8450263::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "There are people swimming.$"
