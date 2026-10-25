Kanto_Route11_EastEntrance_2F_MapScripts::
	.byte 0

@ Kanto_Route11_EastEntrance_2F sign / bg event at 4,1
Kanto_Route11_EastEntrance_2F_EventScript_841FCA2::
	lockall
	goto_if_set FLAG_QZ_0x2232, Kanto_Route11_EastEntrance_2F_EventScript_841FCB6
	msgbox Kanto_Route11_EastEntrance_2F_Text_844F12C, MSGBOX_DEFAULT
	releaseall
	end

Kanto_Route11_EastEntrance_2F_EventScript_841FCB6::
	msgbox Kanto_Route11_EastEntrance_2F_Text_844F17B, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_Route11_EastEntrance_2F sign / bg event at 8,1
Kanto_Route11_EastEntrance_2F_EventScript_841FCC0::
	msgbox Kanto_Route11_EastEntrance_2F_Text_844F1C1, MSGBOX_SIGN
	end

@ Kanto_Route11_EastEntrance_2F object 1 (local id 1, gfx 0x3F5C) at 7,3
Kanto_Route11_EastEntrance_2F_EventScript_841FCC9::
	lock
	faceplayer
	setvar VAR_0x8008, 0xB
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x212C, Kanto_Route11_EastEntrance_2F_EventScript_841FD46
	goto_if_eq VAR_RESULT, 0, Kanto_Route11_EastEntrance_2F_EventScript_841FD2E
	msgbox Quetzal_Text_8455C79, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_Route11_EastEntrance_2F_EventScript_841FD2E
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_Route11_EastEntrance_2F_EventScript_841FD2E
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_Route11_EastEntrance_2F_EventScript_841FD38
	call Quetzal_EventScript_845919C
	msgbox Quetzal_Text_8455CD8, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x212C
	release
	end

Kanto_Route11_EastEntrance_2F_EventScript_841FD2E::
	msgbox Quetzal_Text_8455CB4, MSGBOX_DEFAULT
	release
	end

Kanto_Route11_EastEntrance_2F_EventScript_841FD38::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455CC4, MSGBOX_DEFAULT
	release
	end

Kanto_Route11_EastEntrance_2F_EventScript_841FD46::
	msgbox Quetzal_Text_8455CE5, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route11_EastEntrance_2F object 2 (local id 2, gfx 0x3F43) at 2,6
Kanto_Route11_EastEntrance_2F_EventScript_841FD50::
	lock
	faceplayer
	call Kanto_Route11_EastEntrance_2F_EventScript_841FDEA
	goto_if_set FLAG_QZ_0x2154, Kanto_Route11_EastEntrance_2F_EventScript_841FDE0
	msgbox Kanto_Route11_EastEntrance_2F_Text_844EE87, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F94
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, QzGetPokedexCount_731
	buffernumberstring 2, VAR_0x8006
	call Kanto_Route11_EastEntrance_2F_EventScript_841FDEA
	goto_if_lt VAR_0x8006, 30, Quetzal_EventScript_8457F80
	checkitem ITEM_DOWSING_MACHINE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route11_EastEntrance_2F_EventScript_841FDD3
	msgbox Kanto_Route11_EastEntrance_2F_Text_844EF86, MSGBOX_DEFAULT
	additem ITEM_DOWSING_MACHINE, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F8A
	loadword 0, Kanto_Route11_EastEntrance_2F_Text_844EFD9
	setorcopyvar VAR_0x8000, 0x2C8
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E

Kanto_Route11_EastEntrance_2F_EventScript_841FDD3::
	setflag FLAG_QZ_0x2154
	msgbox Kanto_Route11_EastEntrance_2F_Text_844EFFB, MSGBOX_DEFAULT
	release
	end

Kanto_Route11_EastEntrance_2F_EventScript_841FDE0::
	msgbox Kanto_Route11_EastEntrance_2F_Text_844EFFB, MSGBOX_DEFAULT
	release
	end

Kanto_Route11_EastEntrance_2F_EventScript_841FDEA::
	buffernumberstring 0, 0x1E
	bufferitemname 1, ITEM_DOWSING_MACHINE
	return

Kanto_Route11_EastEntrance_2F_Text_844EE87::
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

Kanto_Route11_EastEntrance_2F_Text_844EF86::
	.string "Great! You have caught or owned\n"
	.string "{STR_VAR_3} kinds of POKéMON!\p"
	.string "Congratulations!\n"
	.string "Here you go!$"

Kanto_Route11_EastEntrance_2F_Text_844EFD9::
	.string "{PLAYER} received the {STR_VAR_2}\n"
	.string "from the AIDE.$"

Kanto_Route11_EastEntrance_2F_Text_844EFFB::
	.string "There are items on the ground that\n"
	.string "may be hidden from view.\p"
	.string "Use the ITEMFINDER to detect any\n"
	.string "hidden items close to you.\p"
	.string "The machine is a bit limited.\n"
	.string "It can’t pinpoint item locations.\p"
	.string "What it does is show the direction\n"
	.string "where the item is.\p"
	.string "Use it to get your bearings, then\n"
	.string "search the suspect area by hand.$"

Kanto_Route11_EastEntrance_2F_Text_844F12C::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "A big POKéMON is asleep on a road!$"

Kanto_Route11_EastEntrance_2F_Text_844F17B::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "What a breathtaking view!$"

Kanto_Route11_EastEntrance_2F_Text_844F1C1::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "To get to LAVENDER TOWN from\n"
	.string "CERULEAN CITY…\p"
	.string "ROCK TUNNEL appears to be a good\n"
	.string "route to take.$"
