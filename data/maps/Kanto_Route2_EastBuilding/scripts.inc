Kanto_Route2_EastBuilding_MapScripts::
	.byte 0

@ Kanto_Route2_EastBuilding object 1 (local id 1, gfx 0x3F43) at 4,6
Kanto_Route2_EastBuilding_EventScript_841F5F6::
	lock
	faceplayer
	call Kanto_Route2_EastBuilding_EventScript_841F685
	goto_if_set FLAG_QZ_0x2152, Kanto_Route2_EastBuilding_EventScript_841F67B
	msgbox Kanto_Route2_EastBuilding_Text_844DB4D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F94
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, QzGetPokedexCount
	buffernumberstring 2, VAR_0x8006
	call Kanto_Route2_EastBuilding_EventScript_841F685
	goto_if_lt VAR_0x8006, 10, Quetzal_EventScript_8457F80
	msgbox Kanto_Route2_EastBuilding_Text_844DC10, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_HM05   @ Moemon Kai: FireRed's HM (tools/quetzal_port/fix_hm_givers.py)
	call EventScript_GiveHmOnce
	setflag FLAG_QZ_0x2152
	msgbox Kanto_Route2_EastBuilding_Text_844DC8E, MSGBOX_DEFAULT
	release
	end

Kanto_Route2_EastBuilding_EventScript_841F67B::
	msgbox Kanto_Route2_EastBuilding_Text_844DC8E, MSGBOX_DEFAULT
	release
	end

Kanto_Route2_EastBuilding_EventScript_841F685::
	buffernumberstring 0, 0xA
	bufferitemname 1, ITEM_HM05
	return

@ Kanto_Route2_EastBuilding object 2 (local id 2, gfx 0x3F3B) at 10,3
Kanto_Route2_EastBuilding_EventScript_841F68E::
	msgbox Kanto_Route2_EastBuilding_Text_844DCC1, MSGBOX_NPC
	end

Kanto_Route2_EastBuilding_Text_844DB4D::
	.string "Hi! Remember me?\n"
	.string "I'm one of PROF. OAK's AIDES.\p"
	.string "If your MOéDEX has complete data\n"
	.string "on ten species, I'm supposed to\l"
	.string "give you a reward.\p"
	.string "PROF. OAK entrusted me with\n"
	.string "HM05 for you.\p"
	.string "So, {PLAYER}, let me ask you.\p"
	.string "Have you gathered data on at least\n"
	.string "ten kinds of MOéMON?$"

Kanto_Route2_EastBuilding_Text_844DC10::
	.string "Great! You have caught or owned\n"
	.string "{STR_VAR_3} kinds of MOéMON!\p"
	.string "Congratulations!\n"
	.string "Here you go!$"

Kanto_Route2_EastBuilding_Text_844DC63::
	.string "{PLAYER} received a GALARICA CUFF\n"
	.string "from the AIDE.$"

Kanto_Route2_EastBuilding_Text_844DC8E::
	.string "HM05 contains the hidden move\n"
	.string "FLASH.\p"
	.string "FLASH lights up even the darkest\n"
	.string "of caves and dungeons.$"

Kanto_Route2_EastBuilding_Text_844DCC1::
	.string "ROCK TUNNEL is very dark.\n"
	.string "Prepare carefully before entering.$"
