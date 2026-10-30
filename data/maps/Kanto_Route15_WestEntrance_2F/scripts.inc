Kanto_Route15_WestEntrance_2F_MapScripts::
	.byte 0

@ Kanto_Route15_WestEntrance_2F sign / bg event at 4,1
Kanto_Route15_WestEntrance_2F_EventScript_84200F7::
	lockall
	msgbox Kanto_Route15_WestEntrance_2F_Text_844FBF8, MSGBOX_DEFAULT
	showmonpic SPECIES_ARTICUNO, 0xA, 3
	delay 0x14
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, 0x90
	special QzSetSeenMon
	releaseall
	end

@ Kanto_Route15_WestEntrance_2F sign / bg event at 8,1
Kanto_Route15_WestEntrance_2F_EventScript_8420114::
	msgbox Kanto_Route15_WestEntrance_2F_Text_844FC54, MSGBOX_SIGN
	end

@ Kanto_Route15_WestEntrance_2F object 1 (local id 1, gfx 0x3F43) at 5,5
Kanto_Route15_WestEntrance_2F_EventScript_842011D::
	lock
	faceplayer
	call Kanto_Route15_WestEntrance_2F_EventScript_84201B7
	goto_if_set FLAG_QZ_0x2147, Kanto_Route15_WestEntrance_2F_EventScript_84201AD
	msgbox Kanto_Route15_WestEntrance_2F_Text_844FA45, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F94
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, QzGetPokedexCount
	buffernumberstring 2, VAR_0x8006
	call Kanto_Route15_WestEntrance_2F_EventScript_84201B7
	goto_if_lt VAR_0x8006, 50, Quetzal_EventScript_8457F80
	msgbox Kanto_Route15_WestEntrance_2F_Text_844FB44, MSGBOX_DEFAULT
	checkitemspace ITEM_LUCKY_EGG, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F8A
	additem ITEM_LUCKY_EGG, 1
	loadword 0, Kanto_Route15_WestEntrance_2F_Text_844FB97
	setorcopyvar VAR_0x8000, 0x1D6
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	goto_if_eq VAR_0x8007, 0, Quetzal_EventScript_8457F8A
	setflag FLAG_QZ_0x2147
	msgbox Kanto_Route15_WestEntrance_2F_Text_844FBB9, MSGBOX_DEFAULT
	release
	end

Kanto_Route15_WestEntrance_2F_EventScript_84201AD::
	msgbox Kanto_Route15_WestEntrance_2F_Text_844FBB9, MSGBOX_DEFAULT
	release
	end

Kanto_Route15_WestEntrance_2F_EventScript_84201B7::
	buffernumberstring 0, 0x32
	bufferitemname 1, ITEM_LUCKY_EGG
	return

Kanto_Route15_WestEntrance_2F_Text_844FA45::
	.string "Hi! Remember me?\n"
	.string "I’m one of PROF. OAK’s AIDES.\p"
	.string "If your POKéDEX has complete data\n"
	.string "on {STR_VAR_1} species, I’m supposed to\l"
	.string "give you a reward.\p"
	.string "PROF. OAK entrusted me with the\n"
	.string "{STR_VAR_2} for you.\p"
	.string "So, {PLAYER}, let me ask you.\p"
	.string "Have you gathered data on at least\n"
	.string "{STR_VAR_1} kinds of POKéMON?$"

Kanto_Route15_WestEntrance_2F_Text_844FB44::
	.string "Great! You have caught or owned\n"
	.string "{STR_VAR_3} kinds of POKéMON!\p"
	.string "Congratulations!\n"
	.string "Here you go!$"

Kanto_Route15_WestEntrance_2F_Text_844FB97::
	.string "{PLAYER} received the {STR_VAR_2}\n"
	.string "from the AIDE.$"

Kanto_Route15_WestEntrance_2F_Text_844FBB9::
	.string "A POKéMON holding this item gains\n"
	.string "extra EXP. Points in battle.$"

Kanto_Route15_WestEntrance_2F_Text_844FBF8::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "A large, shining bird is flying\n"
	.string "toward the sea.$"

Kanto_Route15_WestEntrance_2F_Text_844FC54::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "It looks like a small island on\n"
	.string "the horizon!$"
