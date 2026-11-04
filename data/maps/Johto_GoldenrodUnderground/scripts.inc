Johto_GoldenrodUnderground_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_LOAD, Johto_GoldenrodUnderground_EventScript_83DAF6C
	.byte 0

Johto_GoldenrodUnderground_EventScript_82E40D4::
	pokemart Johto_GoldenrodUnderground_Mart_82E40F0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodUnderground object 2 (local id 2, gfx 0x1E) at 6,24
Johto_GoldenrodUnderground_EventScript_83DAF41::
	@ Moemon Kai: Crystal's herb lady (BitterMerchantScript): Saturdays and Sundays (FLAG_JOHTO_HIDE_HERB_LADY), her
	@ MART_UNDERGROUND and her words (docs/REGIONS_AUDIT.md 145)
	lock
	faceplayer
	message Johto_GoldenrodUnderground_Text_82E4142
	waitmessage
	pokemart Johto_GoldenrodUnderground_Mart_82E40F0
	msgbox Johto_GoldenrodUnderground_Text_HerbComeAgain, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodUnderground_EventScript_83DAF6C::
	goto Johto_GoldenrodUnderground_EventScript_83DB290

@ Johto_GoldenrodUnderground trigger at 8,12 (VAR_QZ_0x500A == 7)
Johto_GoldenrodUnderground_EventScript_83DAF72::
	@ Moemon Kai: Quetzal's KIMONO GIRL KUNI scene is gone (Crystal has none here: docs/REGIONS_AUDIT.md 145); the story
	@ step it carried stays
	setvar VAR_QZ_0x500A, 8
	end

@ Johto_GoldenrodUnderground trigger at 8,11 (VAR_QZ_0x500A == 7)
Johto_GoldenrodUnderground_EventScript_83DAFD9::
	@ Moemon Kai: Quetzal's KIMONO GIRL KUNI scene is gone (Crystal has none here: docs/REGIONS_AUDIT.md 145); the story
	@ step it carried stays
	setvar VAR_QZ_0x500A, 8
	end

@ Johto_GoldenrodUnderground object 6 (local id 6, gfx 0x3EB7) at 4,7
Johto_GoldenrodUnderground_EventScript_83DB146::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4728, 0, Johto_GoldenrodUnderground_Text_83DB336, Johto_GoldenrodUnderground_Text_83DB383
	msgbox Johto_GoldenrodUnderground_Text_83DB38D, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 5 (local id 5, gfx 0x3EB7) at 4,10
Johto_GoldenrodUnderground_EventScript_83DB15D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4242, 0, Johto_GoldenrodUnderground_Text_83DB3BA, Johto_GoldenrodUnderground_Text_83DB423
	msgbox Johto_GoldenrodUnderground_Text_83DB42F, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 1 (local id 1, gfx 0x3EB7) at 4,30
Johto_GoldenrodUnderground_EventScript_83DB174::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4927, 0, Johto_GoldenrodUnderground_Text_83DB476, Johto_GoldenrodUnderground_Text_83DB4B5
	msgbox Johto_GoldenrodUnderground_Text_83DB4BE, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 7 (local id 7, gfx 0x3EB7) at 1,27
Johto_GoldenrodUnderground_EventScript_83DB18B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4320, 0, Johto_GoldenrodUnderground_Text_83DB4F9, Johto_GoldenrodUnderground_Text_83DB537
	msgbox Johto_GoldenrodUnderground_Text_83DB558, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 3 (local id 3, gfx 0x3EBF) at 6,20
Johto_GoldenrodUnderground_EventScript_83DB1A2::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodUnderground_Mart_83DB1BC
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodUnderground object 13 (local id 13, gfx 0x3E8E) at 1,20
Johto_GoldenrodUnderground_EventScript_83DB1E2::
	lock
	faceplayer
	message Johto_GoldenrodUnderground_Text_83DB208
	waitmessage
	pokemart Johto_GoldenrodUnderground_Mart_83DB1FC
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodUnderground object 4 (local id 4, gfx 0x3E8E) at 6,16
Johto_GoldenrodUnderground_EventScript_83DB260::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodUnderground_Mart_83DB278
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodUnderground_EventScript_83DB290::
	goto_if_set FLAG_QZ_0x109A, Quetzal_EventScript_83DB29A
	end

@ Johto_GoldenrodUnderground sign / bg event at 12,10
Johto_GoldenrodUnderground_EventScript_83DB2A4::
	lock
	goto_if_set FLAG_QZ_0x109A, Johto_GoldenrodUnderground_EventScript_83DB2E7
	checkitem ITEM_BASEMENT_KEY, 1
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodUnderground_EventScript_83DB2C9
	msgbox Johto_GoldenrodUnderground_Text_83DB5BF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodUnderground_EventScript_83DB2C9::
	msgbox Johto_GoldenrodUnderground_Text_83DB5E4, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x109A
	setmetatile 0xC, 0xA, 0x2DD, 0
	playse SE_BANG
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUnderground_EventScript_83DB2E7::
	msgbox Johto_GoldenrodUnderground_Text_83DB5D2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodUnderground object 8 (local id 8, gfx 0x3A9E) at 13,13
Johto_GoldenrodUnderground_EventScript_83DB2F2::
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FD2E
	setorcopyvar VAR_0x8000, ITEM_COIN_CASE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUnderground object 11 (local id 11, gfx 0x3B) at 5,26
Johto_GoldenrodUnderground_EventScript_83DB30F::
	setorcopyvar VAR_0x8000, ITEM_SUPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUnderground object 9 (local id 9, gfx 0x3B) at 1,8
Johto_GoldenrodUnderground_EventScript_83DB31C::
	setorcopyvar VAR_0x8000, ITEM_PARALYZE_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUnderground object 10 (local id 10, gfx 0x3B) at 1,23
Johto_GoldenrodUnderground_EventScript_83DB329::
	setorcopyvar VAR_0x8000, ITEM_ANTIDOTE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_GoldenrodUnderground_Mart_82E40E4:
	.2byte ITEM_ENERGY_POWDER
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Mart_82E40F0:
	.2byte ITEM_ENERGY_POWDER
	.2byte ITEM_ENERGY_ROOT
	.2byte ITEM_HEAL_POWDER
	.2byte ITEM_REVIVAL_HERB
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Movement_83DB040:
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUnderground_Movement_83DB045:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUnderground_Movement_83DB04B:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUnderground_Movement_83DB053:
	face_down
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	step_end

Johto_GoldenrodUnderground_Movement_83DB05B:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_GoldenrodUnderground_Mart_83DB1BC:
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Mart_83DB1FC:
	.2byte ITEM_BLUE_FLUTE
	.2byte ITEM_YELLOW_FLUTE
	.2byte ITEM_RED_FLUTE
	.2byte ITEM_BLACK_FLUTE
	.2byte ITEM_WHITE_FLUTE
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Mart_83DB278:
	.2byte ITEM_METEORITE
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Text_82E4142::
	.string "Hello, dear.\p"
	.string "I sell inexpensive herbal\n"
	.string "medicine.\p"
	.string "They’re good, but a trifle bitter.\p"
	.string "Your MOéMON may not like them.\p"
	.string "Hehehehe…$"

Johto_GoldenrodUnderground_Text_83DB062::
	.string "Team Rocket members in black suits\n"
	.string "have been running around...\p"
	.string "I was wondering what was going on.\n"
	.string "So, you took on Team Rocket?\l"
	.string "You are impressive!\p"
	.string "You may be able to face the\n"
	.string "legendary MOé...$"

Johto_GoldenrodUnderground_Text_83DB123::
	.string "Hahaha! Never mind.\n"
	.string "I must go now.$"

Johto_GoldenrodUnderground_Text_83DB208::
	.string "My glass flutes produce different\n"
	.string "effects depending on their color.\p"
	.string "Would you like one?$"

Johto_GoldenrodUnderground_Text_83DB336::
	.string "I got booted out of the GAME CORNER.\p"
	.string "I was trying to cheat using my\n"
	.string "MOéMON…$"

Johto_GoldenrodUnderground_Text_83DB383::
	.string "…Grumble…$"

Johto_GoldenrodUnderground_Text_83DB38D::
	.string "I guess I have to do things fair and\n"
	.string "square…$"

Johto_GoldenrodUnderground_Text_83DB3BA::
	.string "Do you consider type alignments in\n"
	.string "battle?\p"
	.string "If you know your type advantages,\n"
	.string "you’ll do better in battle.$"

Johto_GoldenrodUnderground_Text_83DB423::
	.string "Ow, ow, ow!$"

Johto_GoldenrodUnderground_Text_83DB42F::
	.string "I know my MOéMON type alignments.\p"
	.string "But I only use one type of MOéMON.$"

Johto_GoldenrodUnderground_Text_83DB476::
	.string "My MOéMON just got a haircut!\p"
	.string "I’ll show you how strong it is!$"

Johto_GoldenrodUnderground_Text_83DB4B5::
	.string "Aiyeeee!$"

Johto_GoldenrodUnderground_Text_83DB4BE::
	.string "Your MOéMON will like you more if\n"
	.string "you give them haircuts.$"

Johto_GoldenrodUnderground_Text_83DB4F9::
	.string "I think you have some rare MOéMON\n"
	.string "with you.\p"
	.string "Let me see them!$"

Johto_GoldenrodUnderground_Text_83DB537::
	.string "Gaah! I lost! That makes me mad!$"

Johto_GoldenrodUnderground_Text_83DB558::
	.string "When I was walking near the BIKE\n"
	.string "SHOP last night, I felt like I was\l"
	.string "being watched from the rooftops...$"

Johto_GoldenrodUnderground_Text_83DB5BF::
	.string "The door’s locked…$"

Johto_GoldenrodUnderground_Text_83DB5D2::
	.string "The door is open.$"

Johto_GoldenrodUnderground_Text_83DB5E4::
	.string "The BASEMENT KEY opened the door.$"

@ Moemon Kai: Crystal's OLDER HAIRCUT BROTHER (OlderHaircutBrotherScript, docs/REGIONS_AUDIT.md 145): Tuesdays, Thursdays, Saturdays
@ (FLAG_JOHTO_HIDE_OLDER_CUT), one haircut a day for both (FLAG_DAILY_JOHTO_HAIRCUT), 500 yen
Johto_GoldenrodUnderground_EventScript_OlderCut::
	lock
	faceplayer
	goto_if_set FLAG_DAILY_JOHTO_HAIRCUT, Johto_GoldenrodUnderground_EventScript_OlderCutAlready
	showmoneybox 0, 0
	msgbox Johto_GoldenrodUnderground_Text_OlderCutOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodUnderground_EventScript_OlderCutRefused
	checkmoney 500
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_OlderCutNoMoney
	msgbox Johto_GoldenrodUnderground_Text_OlderCutWhich, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	setvar VAR_0x8005, 1
	specialvar VAR_RESULT, MP_HaircutBrother
	goto_if_le VAR_RESULT, 1, Johto_GoldenrodUnderground_EventScript_OlderCutRefused
	copyvar VAR_0x8006, VAR_RESULT
	setflag FLAG_DAILY_JOHTO_HAIRCUT
	removemoney 500
	updatemoneybox
	msgbox Johto_GoldenrodUnderground_Text_OlderCutDoing, MSGBOX_DEFAULT
	hidemoneybox
	fadescreen FADE_TO_WHITE
	playfanfare MUS_HEAL
	waitfanfare
	fadescreen FADE_FROM_WHITE
	msgbox Johto_GoldenrodUnderground_Text_OlderCutDone, MSGBOX_DEFAULT
	goto Johto_GoldenrodUnderground_EventScript_HaircutHappier
Johto_GoldenrodUnderground_EventScript_OlderCutRefused::
	hidemoneybox
	msgbox Johto_GoldenrodUnderground_Text_OlderCutRefused, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodUnderground_EventScript_OlderCutNoMoney::
	hidemoneybox
	msgbox Johto_GoldenrodUnderground_Text_OlderCutNoMoney, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodUnderground_EventScript_OlderCutAlready::
	msgbox Johto_GoldenrodUnderground_Text_OlderCutAlready, MSGBOX_DEFAULT
	release
	end

@ Moemon Kai: Crystal's YOUNGER HAIRCUT BROTHER (YoungerHaircutBrotherScript, docs/REGIONS_AUDIT.md 145): Sundays, Wednesdays, Fridays
@ (FLAG_JOHTO_HIDE_YOUNGER_CUT), one haircut a day for both (FLAG_DAILY_JOHTO_HAIRCUT), 300 yen
Johto_GoldenrodUnderground_EventScript_YoungerCut::
	lock
	faceplayer
	goto_if_set FLAG_DAILY_JOHTO_HAIRCUT, Johto_GoldenrodUnderground_EventScript_YoungerCutAlready
	showmoneybox 0, 0
	msgbox Johto_GoldenrodUnderground_Text_YoungerCutOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodUnderground_EventScript_YoungerCutRefused
	checkmoney 300
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_YoungerCutNoMoney
	msgbox Johto_GoldenrodUnderground_Text_YoungerCutWhich, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, MP_HaircutBrother
	goto_if_le VAR_RESULT, 1, Johto_GoldenrodUnderground_EventScript_YoungerCutRefused
	copyvar VAR_0x8006, VAR_RESULT
	setflag FLAG_DAILY_JOHTO_HAIRCUT
	removemoney 300
	updatemoneybox
	msgbox Johto_GoldenrodUnderground_Text_YoungerCutDoing, MSGBOX_DEFAULT
	hidemoneybox
	fadescreen FADE_TO_WHITE
	playfanfare MUS_HEAL
	waitfanfare
	fadescreen FADE_FROM_WHITE
	msgbox Johto_GoldenrodUnderground_Text_YoungerCutDone, MSGBOX_DEFAULT
	goto Johto_GoldenrodUnderground_EventScript_HaircutHappier
Johto_GoldenrodUnderground_EventScript_YoungerCutRefused::
	hidemoneybox
	msgbox Johto_GoldenrodUnderground_Text_YoungerCutRefused, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodUnderground_EventScript_YoungerCutNoMoney::
	hidemoneybox
	msgbox Johto_GoldenrodUnderground_Text_YoungerCutNoMoney, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodUnderground_EventScript_YoungerCutAlready::
	msgbox Johto_GoldenrodUnderground_Text_YoungerCutAlready, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodUnderground_EventScript_HaircutHappier::
	switch VAR_0x8006
	case 2, Johto_GoldenrodUnderground_EventScript_HaircutSlightly
	case 3, Johto_GoldenrodUnderground_EventScript_HaircutHappy
	msgbox Johto_GoldenrodUnderground_Text_HaircutDelighted, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodUnderground_EventScript_HaircutSlightly::
	msgbox Johto_GoldenrodUnderground_Text_HaircutSlightly, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodUnderground_EventScript_HaircutHappy::
	msgbox Johto_GoldenrodUnderground_Text_HaircutHappy, MSGBOX_DEFAULT
	release
	end

@ Moemon Kai: Crystal's bargain merchant (BargainMerchantScript, MARTTYPE_BARGAIN; docs/REGIONS_AUDIT.md 145): Monday
@ mornings (FLAG_JOHTO_HIDE_BARGAIN), Crystal's bargain prices, one of each (FLAG_DAILY_JOHTO_BARGAIN_*)
Johto_GoldenrodUnderground_EventScript_Bargain::
	lock
	faceplayer
	msgbox Johto_GoldenrodUnderground_Text_BargainIntro, MSGBOX_DEFAULT
Johto_GoldenrodUnderground_EventScript_BargainMenu::
	showmoneybox 0, 0
	message Johto_GoldenrodUnderground_Text_BargainWhich
	waitmessage
	multichoice 0x11, 0, MULTI_KAI_GOLDENROD_BARGAIN, FALSE
	switch VAR_RESULT
	case 0, Johto_GoldenrodUnderground_EventScript_Bargain_NUGGET
	case 1, Johto_GoldenrodUnderground_EventScript_Bargain_PEARL
	case 2, Johto_GoldenrodUnderground_EventScript_Bargain_BIG_PEARL
	case 3, Johto_GoldenrodUnderground_EventScript_Bargain_STARDUST
	case 4, Johto_GoldenrodUnderground_EventScript_Bargain_STAR_PIECE
	goto Johto_GoldenrodUnderground_EventScript_BargainBye
Johto_GoldenrodUnderground_EventScript_Bargain_NUGGET::
	goto_if_set FLAG_DAILY_JOHTO_BARGAIN_NUGGET, Johto_GoldenrodUnderground_EventScript_BargainSoldOut
	bufferitemname STR_VAR_1, ITEM_NUGGET
	buffernumberstring STR_VAR_2, 4500
	msgbox Johto_GoldenrodUnderground_Text_BargainPrice, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodUnderground_EventScript_BargainMenu
	checkmoney 4500
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainNoFunds
	checkitemspace ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainFull
	additem ITEM_NUGGET, 1
	removemoney 4500
	updatemoneybox
	setflag FLAG_DAILY_JOHTO_BARGAIN_NUGGET
	goto Johto_GoldenrodUnderground_EventScript_BargainThanks
Johto_GoldenrodUnderground_EventScript_Bargain_PEARL::
	goto_if_set FLAG_DAILY_JOHTO_BARGAIN_PEARL, Johto_GoldenrodUnderground_EventScript_BargainSoldOut
	bufferitemname STR_VAR_1, ITEM_PEARL
	buffernumberstring STR_VAR_2, 650
	msgbox Johto_GoldenrodUnderground_Text_BargainPrice, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodUnderground_EventScript_BargainMenu
	checkmoney 650
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainNoFunds
	checkitemspace ITEM_PEARL, 1
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainFull
	additem ITEM_PEARL, 1
	removemoney 650
	updatemoneybox
	setflag FLAG_DAILY_JOHTO_BARGAIN_PEARL
	goto Johto_GoldenrodUnderground_EventScript_BargainThanks
Johto_GoldenrodUnderground_EventScript_Bargain_BIG_PEARL::
	goto_if_set FLAG_DAILY_JOHTO_BARGAIN_BIG_PEARL, Johto_GoldenrodUnderground_EventScript_BargainSoldOut
	bufferitemname STR_VAR_1, ITEM_BIG_PEARL
	buffernumberstring STR_VAR_2, 3500
	msgbox Johto_GoldenrodUnderground_Text_BargainPrice, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodUnderground_EventScript_BargainMenu
	checkmoney 3500
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainNoFunds
	checkitemspace ITEM_BIG_PEARL, 1
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainFull
	additem ITEM_BIG_PEARL, 1
	removemoney 3500
	updatemoneybox
	setflag FLAG_DAILY_JOHTO_BARGAIN_BIG_PEARL
	goto Johto_GoldenrodUnderground_EventScript_BargainThanks
Johto_GoldenrodUnderground_EventScript_Bargain_STARDUST::
	goto_if_set FLAG_DAILY_JOHTO_BARGAIN_STARDUST, Johto_GoldenrodUnderground_EventScript_BargainSoldOut
	bufferitemname STR_VAR_1, ITEM_STARDUST
	buffernumberstring STR_VAR_2, 900
	msgbox Johto_GoldenrodUnderground_Text_BargainPrice, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodUnderground_EventScript_BargainMenu
	checkmoney 900
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainNoFunds
	checkitemspace ITEM_STARDUST, 1
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainFull
	additem ITEM_STARDUST, 1
	removemoney 900
	updatemoneybox
	setflag FLAG_DAILY_JOHTO_BARGAIN_STARDUST
	goto Johto_GoldenrodUnderground_EventScript_BargainThanks
Johto_GoldenrodUnderground_EventScript_Bargain_STAR_PIECE::
	goto_if_set FLAG_DAILY_JOHTO_BARGAIN_STAR_PIECE, Johto_GoldenrodUnderground_EventScript_BargainSoldOut
	bufferitemname STR_VAR_1, ITEM_STAR_PIECE
	buffernumberstring STR_VAR_2, 4600
	msgbox Johto_GoldenrodUnderground_Text_BargainPrice, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodUnderground_EventScript_BargainMenu
	checkmoney 4600
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainNoFunds
	checkitemspace ITEM_STAR_PIECE, 1
	goto_if_eq VAR_RESULT, FALSE, Johto_GoldenrodUnderground_EventScript_BargainFull
	additem ITEM_STAR_PIECE, 1
	removemoney 4600
	updatemoneybox
	setflag FLAG_DAILY_JOHTO_BARGAIN_STAR_PIECE
	goto Johto_GoldenrodUnderground_EventScript_BargainThanks
Johto_GoldenrodUnderground_EventScript_BargainThanks::
	playse SE_SHOP
	msgbox Johto_GoldenrodUnderground_Text_BargainThanks, MSGBOX_DEFAULT
	goto Johto_GoldenrodUnderground_EventScript_BargainMenu
Johto_GoldenrodUnderground_EventScript_BargainSoldOut::
	msgbox Johto_GoldenrodUnderground_Text_BargainSoldOut, MSGBOX_DEFAULT
	goto Johto_GoldenrodUnderground_EventScript_BargainMenu
Johto_GoldenrodUnderground_EventScript_BargainNoFunds::
	msgbox Johto_GoldenrodUnderground_Text_BargainNoFunds, MSGBOX_DEFAULT
	goto Johto_GoldenrodUnderground_EventScript_BargainMenu
Johto_GoldenrodUnderground_EventScript_BargainFull::
	msgbox Johto_GoldenrodUnderground_Text_BargainFull, MSGBOX_DEFAULT
	goto Johto_GoldenrodUnderground_EventScript_BargainMenu
Johto_GoldenrodUnderground_EventScript_BargainBye::
	hidemoneybox
	msgbox Johto_GoldenrodUnderground_Text_BargainBye, MSGBOX_DEFAULT
	release
	end

@ (Crystal's texts that start with a name buffer, the buffer as STR_VAR_1)
Johto_GoldenrodUnderground_Text_HaircutSlightly::
	.string "{STR_VAR_1} looks a\n"
	.string "little happier.$"

Johto_GoldenrodUnderground_Text_HaircutHappy::
	.string "{STR_VAR_1} looks happy.$"

Johto_GoldenrodUnderground_Text_HaircutDelighted::
	.string "{STR_VAR_1} looks delighted!$"

Johto_GoldenrodUnderground_Text_BargainPrice::
	.string "{STR_VAR_1} costs\n"
	.string "¥{STR_VAR_2}. Want it?$"

Johto_GoldenrodUnderground_Text_BargainWhich::
	.string "Which would you like?$"

Johto_GoldenrodUnderground_Text_OlderCutOffer::
	.string "Welcome!\p"
	.string "I run the MOéMON SALON!\p"
	.string "I’m the older and better of the\n"
	.string "two HAIRCUT BROTHERS.\p"
	.string "I can make your MOéMON beautiful\n"
	.string "for just ¥500.\p"
	.string "Would you like me to do that?$"

Johto_GoldenrodUnderground_Text_OlderCutWhich::
	.string "Which MOéMON should I work on?$"

Johto_GoldenrodUnderground_Text_OlderCutDoing::
	.string "OK! Watch it become beautiful!$"

Johto_GoldenrodUnderground_Text_OlderCutDone::
	.string "There! All done!$"

Johto_GoldenrodUnderground_Text_OlderCutRefused::
	.string "Is that right? That’s a shame!$"

Johto_GoldenrodUnderground_Text_OlderCutNoMoney::
	.string "You’ll need more money than that.$"

Johto_GoldenrodUnderground_Text_OlderCutAlready::
	.string "I do only one haircut a day. I’m\n"
	.string "done for today.$"

Johto_GoldenrodUnderground_Text_YoungerCutOffer::
	.string "Welcome to the MOéMON SALON!\p"
	.string "I’m the younger and less expensive\n"
	.string "of the two HAIRCUT BROTHERS.\p"
	.string "I’ll spiff up your MOéMON for just\n"
	.string "¥300.\p"
	.string "So? How about it?$"

Johto_GoldenrodUnderground_Text_YoungerCutWhich::
	.string "OK, which MOéMON should I do?$"

Johto_GoldenrodUnderground_Text_YoungerCutDoing::
	.string "OK! I’ll make it look cool!$"

Johto_GoldenrodUnderground_Text_YoungerCutDone::
	.string "There we go! All done!$"

Johto_GoldenrodUnderground_Text_YoungerCutRefused::
	.string "No? How disappointing!$"

Johto_GoldenrodUnderground_Text_YoungerCutNoMoney::
	.string "You’re a little short on funds.$"

Johto_GoldenrodUnderground_Text_YoungerCutAlready::
	.string "I can do only one haircut a day.\p"
	.string "Sorry, but I’m all done for today.$"

Johto_GoldenrodUnderground_Text_HerbComeAgain::
	.string "Come again, dear. Hehehehe…$"

Johto_GoldenrodUnderground_Text_BargainIntro::
	.string "Hiya! Care to see some bargains?\p"
	.string "I sell rare items that nobody else\n"
	.string "carries--but only one of each\l"
	.string "item.$"

Johto_GoldenrodUnderground_Text_BargainThanks::
	.string "Thanks.$"

Johto_GoldenrodUnderground_Text_BargainSoldOut::
	.string "You bought that already. I’m all\n"
	.string "sold out of it.$"

Johto_GoldenrodUnderground_Text_BargainNoFunds::
	.string "Uh-oh, you’re short on funds.$"

Johto_GoldenrodUnderground_Text_BargainFull::
	.string "Uh-oh, your PACK is chock-full.$"

Johto_GoldenrodUnderground_Text_BargainBye::
	.string "Come by again sometime.$"
