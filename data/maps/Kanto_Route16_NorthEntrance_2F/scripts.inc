Kanto_Route16_NorthEntrance_2F_MapScripts::
	.byte 0

@ Kanto_Route16_NorthEntrance_2F object 1 (local id 1, gfx 0x3F17) at 6,3
Kanto_Route16_NorthEntrance_2F_EventScript_8420360::
	msgbox Kanto_Route16_NorthEntrance_2F_Text_844FE7F, MSGBOX_NPC
	end

@ Kanto_Route16_NorthEntrance_2F object 2 (local id 2, gfx 0x3F18) at 4,8
Kanto_Route16_NorthEntrance_2F_EventScript_8420369::
	msgbox Kanto_Route16_NorthEntrance_2F_Text_844FEB9, MSGBOX_NPC
	end

@ Kanto_Route16_NorthEntrance_2F sign / bg event at 4,1
Kanto_Route16_NorthEntrance_2F_EventScript_8420372::
	msgbox Kanto_Route16_NorthEntrance_2F_Text_844FEE7, MSGBOX_SIGN
	end

@ Kanto_Route16_NorthEntrance_2F sign / bg event at 8,1
Kanto_Route16_NorthEntrance_2F_EventScript_842037B::
	msgbox Kanto_Route16_NorthEntrance_2F_Text_844FF31, MSGBOX_SIGN
	end

@ Kanto_Route16_NorthEntrance_2F object 3 (local id 3, gfx 0x3F43) at 10,6
Kanto_Route16_NorthEntrance_2F_EventScript_8420384::
	lock
	faceplayer
	call Kanto_Route16_NorthEntrance_2F_EventScript_8420413
	goto_if_set FLAG_QZ_0x2140, Kanto_Route16_NorthEntrance_2F_EventScript_8420409
	msgbox Kanto_Route16_NorthEntrance_2F_Text_844FF86, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F94
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, QzGetPokedexCount
	buffernumberstring 2, VAR_0x8006
	call Kanto_Route16_NorthEntrance_2F_EventScript_8420413
	goto_if_lt VAR_0x8006, 40, Quetzal_EventScript_8457F80
	msgbox Kanto_Route16_NorthEntrance_2F_Text_845008D, MSGBOX_DEFAULT
	checkitemspace ITEM_AMULET_COIN, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F8A
	additem ITEM_AMULET_COIN, 1
	loadword 0, Kanto_Route16_NorthEntrance_2F_Text_84500E0
	setorcopyvar VAR_0x8000, 0x1D2
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2140
	msgbox Kanto_Route16_NorthEntrance_2F_Text_845010B, MSGBOX_DEFAULT
	release
	end

Kanto_Route16_NorthEntrance_2F_EventScript_8420409::
	msgbox Kanto_Route16_NorthEntrance_2F_Text_845010B, MSGBOX_DEFAULT
	release
	end

Kanto_Route16_NorthEntrance_2F_EventScript_8420413::
	buffernumberstring 0, 0x28
	bufferitemname 1, ITEM_AMULET_COIN
	return

Kanto_Route16_NorthEntrance_2F_Text_844FE7F::
	.string "I’m on a relaxing ride on my new\n"
	.string "bike with my girlfriend.$"

Kanto_Route16_NorthEntrance_2F_Text_844FEB9::
	.string "We’re going riding together on our\n"
	.string "new bikes.$"

Kanto_Route16_NorthEntrance_2F_Text_844FEE7::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "It’s the CELADON DEPT. STORE!$"

Kanto_Route16_NorthEntrance_2F_Text_844FF31::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "There’s a long path over water far\n"
	.string "away.$"

Kanto_Route16_NorthEntrance_2F_Text_844FF86::
	.string "Hi! Remember me?\n"
	.string "I’m one of PROF. OAK’s AIDES.\p"
	.string "If your POKéDEX has complete data\n"
	.string "on 40 species, I’m supposed to\l"
	.string "give you a reward.\p"
	.string "PROF. OAK entrusted me with an\n"
	.string "AMULET COIN for you.\p"
	.string "So, {PLAYER}, let me ask you.\p"
	.string "Have you gathered data on at least\n"
	.string "40 kinds of POKéMON?$"

Kanto_Route16_NorthEntrance_2F_Text_845008D::
	.string "Great! You have caught or owned\n"
	.string "{STR_VAR_3} kinds of POKéMON!\p"
	.string "Congratulations!\n"
	.string "Here you go!$"

Kanto_Route16_NorthEntrance_2F_Text_84500E0::
	.string "{PLAYER} received the AMULET COIN\n"
	.string "from the AIDE.$"

Kanto_Route16_NorthEntrance_2F_Text_845010B::
	.string "An AMULET COIN is an item to be\n"
	.string "held by a POKéMON.\p"
	.string "If the POKéMON appears in a winning\n"
	.string "battle, you will earn more money.$"
