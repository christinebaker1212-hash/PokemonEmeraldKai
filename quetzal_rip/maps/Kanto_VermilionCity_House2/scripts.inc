Kanto_VermilionCity_House2_MapScripts::
	.byte 0

@ Kanto_VermilionCity_House2 object 1 (local id 1, gfx 0x3F18) at 4,4
Kanto_VermilionCity_House2_EventScript_841B5BD::
	lock
	faceplayer
	setvar VAR_0x8008, 0xA
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x2127, Kanto_VermilionCity_House2_EventScript_841B63A
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_House2_EventScript_841B622
	msgbox Kanto_VermilionCity_House2_Text_8444A22, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_VermilionCity_House2_EventScript_841B622
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_VermilionCity_House2_EventScript_841B622
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_VermilionCity_House2_EventScript_841B62C
	call Quetzal_EventScript_845919C
	msgbox Kanto_VermilionCity_House2_Text_8444A9B, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2127
	release
	end

Kanto_VermilionCity_House2_EventScript_841B622::
	msgbox Kanto_VermilionCity_House2_Text_8444A54, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_House2_EventScript_841B62C::
	bufferspeciesname 0, VAR_0x8009
	msgbox Kanto_VermilionCity_House2_Text_8444A64, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_House2_EventScript_841B63A::
	msgbox Kanto_VermilionCity_House2_Text_8444AA6, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_House2_Text_8444A22::
	.string "Hi!\n"
	.string "Do you have a {STR_VAR_1}?\p"
	.string "Want to trade it for my\n"
	.string "{STR_VAR_2}?$"

Kanto_VermilionCity_House2_Text_8444A54::
	.string "That’s too bad.$"

Kanto_VermilionCity_House2_Text_8444A64::
	.string "…This is no {STR_VAR_1}.\p"
	.string "If you get one, please trade it\n"
	.string "to me!$"

Kanto_VermilionCity_House2_Text_8444A9B::
	.string "Thank you!$"

Kanto_VermilionCity_House2_Text_8444AA6::
	.string "How is my old {STR_VAR_2}?\p"
	.string "My {STR_VAR_1} is doing great!$"
