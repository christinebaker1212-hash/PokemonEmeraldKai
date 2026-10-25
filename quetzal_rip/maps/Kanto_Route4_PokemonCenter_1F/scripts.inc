Kanto_Route4_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route4_PokemonCenter_1F_EventScript_841F6BE
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_Route4_PokemonCenter_1F_EventScript_841F6BE::
	setflag FLAG_QZ_0x224E
	setrespawn 0x32
	end

@ Kanto_Route4_PokemonCenter_1F object 4 (local id 4, gfx 0x3EEE) at 5,4
Kanto_Route4_PokemonCenter_1F_EventScript_841F6C5::
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844DE76, MSGBOX_NPC
	end

@ Kanto_Route4_PokemonCenter_1F object 3 (local id 3, gfx 0x3F09) at 12,5
Kanto_Route4_PokemonCenter_1F_EventScript_841F6CE::
	lock
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844DEDC, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route4_PokemonCenter_1F object 2 (local id 2, gfx 0x3EE6) at 1,3
Kanto_Route4_PokemonCenter_1F_EventScript_841F6D9::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2111, Kanto_Route4_PokemonCenter_1F_EventScript_841F821
	call Quetzal_EventScript_8361A4D
	showmoneybox 0, 0, 0
	checkplayergender
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_PokemonCenter_1F_EventScript_841F705
	goto_if_eq VAR_RESULT, 1, Kanto_Route4_PokemonCenter_1F_EventScript_841F713
	end

Kanto_Route4_PokemonCenter_1F_EventScript_841F705::
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844DF3C, MSGBOX_YESNO
	goto Kanto_Route4_PokemonCenter_1F_EventScript_841F721

Kanto_Route4_PokemonCenter_1F_EventScript_841F713::
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844DFD6, MSGBOX_YESNO
	goto Kanto_Route4_PokemonCenter_1F_EventScript_841F721

Kanto_Route4_PokemonCenter_1F_EventScript_841F721::
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_PokemonCenter_1F_EventScript_841F807
	checkmoney 0x1F4, 0
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_PokemonCenter_1F_EventScript_841F814
	textcolor 3
	setvar VAR_TEMP_1, 0x81
	givemon SPECIES_MAGIKARP, 5, ITEM_NONE
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_PokemonCenter_1F_EventScript_841F775
	goto_if_eq VAR_RESULT, 1, Kanto_Route4_PokemonCenter_1F_EventScript_841F7A1
	goto_if_eq VAR_RESULT, 2, Kanto_Route4_PokemonCenter_1F_EventScript_841F82B
	end

Kanto_Route4_PokemonCenter_1F_EventScript_841F775::
	call Kanto_Route4_PokemonCenter_1F_EventScript_841F7EE
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_PokemonCenter_1F_EventScript_841F7E0
	call Quetzal_EventScript_84590EA
	fadescreen 1
	hidemoneybox
	special ChangePokemonNickname
	waitstate
	goto Kanto_Route4_PokemonCenter_1F_EventScript_841F7E9

Kanto_Route4_PokemonCenter_1F_EventScript_841F7A1::
	call Kanto_Route4_PokemonCenter_1F_EventScript_841F7EE
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_Route4_PokemonCenter_1F_EventScript_841F7CA
	fadescreen 1
	hidemoneybox
	special ChangeBoxPokemonNickname
	waitstate
	lock
	faceplayer
	goto Kanto_Route4_PokemonCenter_1F_EventScript_841F7D5

Kanto_Route4_PokemonCenter_1F_EventScript_841F7CA::
	call Quetzal_EventScript_84590FF
	goto Kanto_Route4_PokemonCenter_1F_EventScript_841F7E0

Kanto_Route4_PokemonCenter_1F_EventScript_841F7D5::
	call Quetzal_EventScript_84590FF
	goto Kanto_Route4_PokemonCenter_1F_EventScript_841F7E9

Kanto_Route4_PokemonCenter_1F_EventScript_841F7E0::
	hidemoneybox
	goto Kanto_Route4_PokemonCenter_1F_EventScript_841F7E9

Kanto_Route4_PokemonCenter_1F_EventScript_841F7E9::
	setflag FLAG_QZ_0x2111
	release
	end

Kanto_Route4_PokemonCenter_1F_EventScript_841F7EE::
	removemoney 0x1F4, 0
	updatemoneybox 0
	playfanfare MUS_LEVEL_UP
	message Kanto_Route4_PokemonCenter_1F_Text_844E075
	waitmessage
	waitfanfare
	bufferspeciesname 0, SPECIES_MAGIKARP
	return

Kanto_Route4_PokemonCenter_1F_EventScript_841F807::
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844E0A9, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

Kanto_Route4_PokemonCenter_1F_EventScript_841F814::
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844E11A, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

Kanto_Route4_PokemonCenter_1F_EventScript_841F821::
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844E13C, MSGBOX_DEFAULT
	release
	end

Kanto_Route4_PokemonCenter_1F_EventScript_841F82B::
	textcolor 0
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844E0E0, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

@ Kanto_Route4_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_Route4_PokemonCenter_1F_EventScript_841F83A::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_Route4_PokemonCenter_1F object 5 (local id 5, gfx 0x3F5C) at 14,4
Kanto_Route4_PokemonCenter_1F_EventScript_841F843::
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844E17D, MSGBOX_NPC
	end

@ Kanto_Route4_PokemonCenter_1F object 6 (local id 6, gfx 0x3EFB) at 12,6
Kanto_Route4_PokemonCenter_1F_EventScript_841F84C::
	lock
	msgbox Kanto_Route4_PokemonCenter_1F_Text_844E1F6, MSGBOX_DEFAULT
	release
	end

Kanto_Route4_PokemonCenter_1F_Text_844DE76::
	.string "Okay, set six POKé BALLS in my\n"
	.string "belt…\p"
	.string "Yeah, that’ll do it. At most, you\n"
	.string "can have six POKéMON with you.$"

Kanto_Route4_PokemonCenter_1F_Text_844DEDC::
	.string "TEAM ROCKET attacks CERULEAN\n"
	.string "citizens…\p"
	.string "Not a day goes by without TEAM\n"
	.string "ROCKET being in the news.$"

Kanto_Route4_PokemonCenter_1F_Text_844DF3C::
	.string "MAN: Hello there, laddie!\n"
	.string "Have I got a deal just for you!\p"
	.string "I’ll let you have a secret POKéMON\n"
	.string "- a MAGIKARP - for just ¥500!\p"
	.string "So, you’ll buy it, am I right?$"

Kanto_Route4_PokemonCenter_1F_Text_844DFD6::
	.string "MAN: Hello there, sweetie pie!\n"
	.string "Have I got a deal just for you!\p"
	.string "I’ll let you have a secret POKéMON\n"
	.string "- a MAGIKARP - for just ¥500!\p"
	.string "So, you’ll buy it, am I right?$"

Kanto_Route4_PokemonCenter_1F_Text_844E075::
	.string "{PLAYER} paid an outrageous ¥500\n"
	.string "and bought the MAGIKARP…$"

Kanto_Route4_PokemonCenter_1F_Text_844E0A9::
	.string "No? You say no? I’m only doing this\n"
	.string "as a favor to you!$"

Kanto_Route4_PokemonCenter_1F_Text_844E0E0::
	.string "There’s no more room for any more\n"
	.string "POKéMON, it looks like.$"

Kanto_Route4_PokemonCenter_1F_Text_844E11A::
	.string "You’ll need more money than that!$"

Kanto_Route4_PokemonCenter_1F_Text_844E13C::
	.string "MAN: Well, I don’t give refunds.\n"
	.string "You knew what you were getting!$"

Kanto_Route4_PokemonCenter_1F_Text_844E17D::
	.string "Sometimes, you’ll have too many\n"
	.string "POKéMON with you to add any more.\p"
	.string "In that case, you should just\n"
	.string "store some using any PC.$"

Kanto_Route4_PokemonCenter_1F_Text_844E1F6::
	.string "It’s a newspaper.$"
