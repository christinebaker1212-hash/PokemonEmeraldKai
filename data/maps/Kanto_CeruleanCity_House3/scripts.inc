Kanto_CeruleanCity_House3_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_House3 object 2 (local id 2, gfx 0x3F2F) at 7,5
Kanto_CeruleanCity_House3_EventScript_841A9A6::
	msgbox Kanto_CeruleanCity_House3_Text_8442272, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_House3 object 1 (local id 1, gfx 0x3F2C) at 2,2
Kanto_CeruleanCity_House3_EventScript_841A9AF::
	lock
	faceplayer
	setvar VAR_0x8008, 8
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x212F, Kanto_CeruleanCity_House3_EventScript_841AA2C
	goto_if_eq VAR_RESULT, 0, Kanto_CeruleanCity_House3_EventScript_841AA14
	msgbox Quetzal_Text_8455CFC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeruleanCity_House3_EventScript_841AA14
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_CeruleanCity_House3_EventScript_841AA14
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_CeruleanCity_House3_EventScript_841AA1E
	call Quetzal_EventScript_845919C
	msgbox Quetzal_Text_8455D9B, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x212F
	release
	end

Kanto_CeruleanCity_House3_EventScript_841AA14::
	msgbox Quetzal_Text_8455D4C, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_House3_EventScript_841AA1E::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455D68, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_House3_EventScript_841AA2C::
	msgbox Quetzal_Text_8455DA3, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_House3_Text_8442272::
	.string "My husband likes trading POKéMON.\p"
	.string "You’re collecting POKéMON for your\n"
	.string "POKéDEX, aren’t you?\p"
	.string "Would you please trade with him?$"
