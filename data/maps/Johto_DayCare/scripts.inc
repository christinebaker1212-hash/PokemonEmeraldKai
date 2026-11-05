@ Moemon Kai: Crystal's DAY-CARE (pokecrystal maps/DayCare.asm, engine/events/daycare.asm; audit case 47): the MAN and the
@ LADY each raise one POKéMON, on the game's DAY CARE pair (daycare.c MP_JohtoDaycare_*); the MAN gives the ODD EGG first
@ and waits on ROUTE 34 when there's an EGG.
Johto_DayCare_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_DayCare_EventScript_OnTransition
	.byte 0

Johto_DayCare_EventScript_OnTransition::
	special MP_JohtoDaycare_Flags
	end

Johto_DayCare_EventScript_Bookshelf::
	msgbox Johto_DayCare_Text_DifficultBookshelf, MSGBOX_SIGN
	end

@ Crystal's DayCareManScript_Inside
Johto_DayCare_EventScript_Man::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1069, Johto_DayCare_EventScript_ManDayCare
	msgbox Johto_DayCare_Text_DayCareManText_GiveOddEgg, MSGBOX_DEFAULT
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, Johto_DayCare_EventScript_PartyFull
	random 7
	addvar VAR_RESULT, 1
	qz_giveoddegg VAR_RESULT
	setflag FLAG_QZ_0x1069
	playfanfare MUS_OBTAIN_ITEM
	message Johto_DayCare_Text_DayCareText_GotOddEgg
	waitmessage
	waitfanfare
	msgbox Johto_DayCare_Text_DayCareText_DescribeOddEgg, MSGBOX_DEFAULT
	release
	end

Johto_DayCare_EventScript_PartyFull::
	msgbox Johto_DayCare_Text_DayCareText_PartyFull, MSGBOX_DEFAULT
	release
	end

Johto_DayCare_EventScript_ManDayCare::
	setvar VAR_0x8009, 1
	specialvar VAR_RESULT, MP_JohtoDaycare_Slot
	goto_if_ne VAR_RESULT, 0xFF, Johto_DayCare_EventScript_AskWithdraw
	msgbox Johto_DayCare_Text_DayCareManIntroText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_DayCare_EventScript_ComeAgain
	goto Johto_DayCare_EventScript_AskDeposit

@ Crystal's DayCareLadyScript
Johto_DayCare_EventScript_Lady::
	lock
	faceplayer
	specialvar VAR_RESULT, GetDaycareState
	goto_if_eq VAR_RESULT, DAYCARE_EGG_WAITING, Johto_DayCare_EventScript_GrampsLooking
	setvar VAR_0x8009, 0
	specialvar VAR_RESULT, MP_JohtoDaycare_Slot
	goto_if_ne VAR_RESULT, 0xFF, Johto_DayCare_EventScript_AskWithdraw
	goto_if_set FLAG_JOHTO_DAYCARE_LADY_MET, Johto_DayCare_EventScript_LadyIntro
	setflag FLAG_JOHTO_DAYCARE_LADY_MET
	msgbox Johto_DayCare_Text_DayCareLadyIntroEggText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_DayCare_EventScript_ComeAgain
	goto Johto_DayCare_EventScript_AskDeposit

Johto_DayCare_EventScript_LadyIntro::
	msgbox Johto_DayCare_Text_DayCareLadyIntroText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_DayCare_EventScript_ComeAgain
	goto Johto_DayCare_EventScript_AskDeposit

Johto_DayCare_EventScript_GrampsLooking::
	msgbox Johto_DayCare_Text_Text_GrampsLookingForYou, MSGBOX_DEFAULT
	release
	end

@ Crystal's DayCareAskDepositPokemon and DayCare_DepositPokemonText
Johto_DayCare_EventScript_AskDeposit::
	getpartysize
	goto_if_lt VAR_RESULT, 2, Johto_DayCare_EventScript_OnlyOneMon
	msgbox Johto_DayCare_Text_WhatShouldIRaiseText, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	special ChooseSendDaycareMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Johto_DayCare_EventScript_OhFine
	specialvar VAR_RESULT, MP_JohtoDaycare_CheckPick
	goto_if_eq VAR_RESULT, 1, Johto_DayCare_EventScript_CantAcceptEgg
	goto_if_eq VAR_RESULT, 2, Johto_DayCare_EventScript_LastHealthyMon
	goto_if_eq VAR_RESULT, 3, Johto_DayCare_EventScript_RemoveMail
	specialvar VAR_0x8005, GetSelectedMonNicknameAndSpecies
	msgbox Johto_DayCare_Text_IllRaiseYourMonText, MSGBOX_DEFAULT
	waitse
	playmoncry VAR_0x8005, CRY_MODE_NORMAL
	msgbox Johto_DayCare_Text_ComeBackLaterText, MSGBOX_DEFAULT
	waitmoncry
	special MP_JohtoDaycare_Deposit
	incrementgamestat GAME_STAT_USED_DAYCARE
	release
	end

Johto_DayCare_EventScript_OnlyOneMon::
	msgbox Johto_DayCare_Text_OnlyOneMonText, MSGBOX_DEFAULT
	goto Johto_DayCare_EventScript_ComeAgain

Johto_DayCare_EventScript_CantAcceptEgg::
	msgbox Johto_DayCare_Text_CantAcceptEggText, MSGBOX_DEFAULT
	goto Johto_DayCare_EventScript_ComeAgain

Johto_DayCare_EventScript_LastHealthyMon::
	msgbox Johto_DayCare_Text_LastHealthyMonText, MSGBOX_DEFAULT
	goto Johto_DayCare_EventScript_ComeAgain

Johto_DayCare_EventScript_RemoveMail::
	msgbox Johto_DayCare_Text_RemoveMailText, MSGBOX_DEFAULT
	goto Johto_DayCare_EventScript_ComeAgain

@ Crystal's DayCare_AskWithdrawBreedMon and DayCare_GetBackMonForMoney
Johto_DayCare_EventScript_AskWithdraw::
	goto_if_eq VAR_0x8006, 0, Johto_DayCare_EventScript_TooSoon
	msgbox Johto_DayCare_Text_AreWeGeniusesText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_DayCare_EventScript_OhFine
	msgbox Johto_DayCare_Text_YourMonHasGrownText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_DayCare_EventScript_OhFine
	goto Johto_DayCare_EventScript_CheckMoney

Johto_DayCare_EventScript_TooSoon::
	msgbox Johto_DayCare_Text_BackAlreadyText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_DayCare_EventScript_OhFine
Johto_DayCare_EventScript_CheckMoney::
	specialvar VAR_RESULT, IsEnoughForCostInVar0x8005
	goto_if_eq VAR_RESULT, FALSE, Johto_DayCare_EventScript_NotEnoughMoney
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, Johto_DayCare_EventScript_HaveNoRoom
	special SubtractMoneyFromVar0x8005
	specialvar VAR_RESULT, MP_JohtoDaycare_Withdraw
	copyvar VAR_0x8005, VAR_RESULT
	playse SE_SHOP
	msgbox Johto_DayCare_Text_PerfectHeresYourMonText, MSGBOX_DEFAULT
	waitse
	playmoncry VAR_0x8005, CRY_MODE_NORMAL
	msgbox Johto_DayCare_Text_GotBackMonText, MSGBOX_DEFAULT
	waitmoncry
	goto Johto_DayCare_EventScript_ComeAgain

Johto_DayCare_EventScript_NotEnoughMoney::
	msgbox Johto_DayCare_Text_NotEnoughMoneyText, MSGBOX_DEFAULT
	goto Johto_DayCare_EventScript_ComeAgain

Johto_DayCare_EventScript_HaveNoRoom::
	msgbox Johto_DayCare_Text_HaveNoRoomText, MSGBOX_DEFAULT
	goto Johto_DayCare_EventScript_ComeAgain

Johto_DayCare_EventScript_OhFine::
	msgbox Johto_DayCare_Text_OhFineThenText, MSGBOX_DEFAULT
Johto_DayCare_EventScript_ComeAgain::
	msgbox Johto_DayCare_Text_ComeAgainText, MSGBOX_DEFAULT
	release
	end

Johto_DayCare_Text_DifficultBookshelf::
	.string "It’s full of difficult books.$"

Johto_DayCare_Text_DayCareManIntroText::
	.string "I’m the DAY-CARE MAN. Want me to\n"
	.string "raise a MOéMON?$"

Johto_DayCare_Text_DayCareLadyIntroText::
	.string "I’m the DAY-CARE LADY.\p"
	.string "Should I raise a MOéMON for you?$"

Johto_DayCare_Text_DayCareLadyIntroEggText::
	.string "I’m the DAY-CARE LADY. Do you know\n"
	.string "about EGGS?\p"
	.string "My husband and I were raising some\n"
	.string "MOéMON, you see.\p"
	.string "We were shocked to find an EGG!\p"
	.string "How incredible could that be?\p"
	.string "Should I raise a MOéMON for you?$"

Johto_DayCare_Text_WhatShouldIRaiseText::
	.string "What should I raise for you?$"

Johto_DayCare_Text_OnlyOneMonText::
	.string "Oh? But you have just one MOéMON.$"

Johto_DayCare_Text_CantAcceptEggText::
	.string "Sorry, but I can’t accept an EGG.$"

Johto_DayCare_Text_RemoveMailText::
	.string "Remove MAIL before you come see\n"
	.string "me.$"

Johto_DayCare_Text_LastHealthyMonText::
	.string "If you give me that, what will you\n"
	.string "battle with?$"

Johto_DayCare_Text_IllRaiseYourMonText::
	.string "OK. I’ll raise your {STR_VAR_1}.$"

Johto_DayCare_Text_ComeBackLaterText::
	.string "Come back for it later.$"

Johto_DayCare_Text_AreWeGeniusesText::
	.string "Are we geniuses or what? Want to\n"
	.string "see your {STR_VAR_1}?$"

Johto_DayCare_Text_YourMonHasGrownText::
	.string "Your {STR_VAR_1} has grown a lot.\p"
	.string "By level, it’s grown by\n"
	.string "{STR_VAR_2}.\p"
	.string "If you want your MOéMON back, it\n"
	.string "will cost ¥{STR_VAR_3}.$"

Johto_DayCare_Text_PerfectHeresYourMonText::
	.string "Perfect! Here’s your MOéMON.$"

Johto_DayCare_Text_GotBackMonText::
	.string "{PLAYER} got back {STR_VAR_1}.$"

Johto_DayCare_Text_BackAlreadyText::
	.string "Huh? Back already? Your\n"
	.string "{STR_VAR_1} needs a little more\l"
	.string "time with us.\p"
	.string "If you want your MOéMON back, it\n"
	.string "will cost ¥100.$"

Johto_DayCare_Text_HaveNoRoomText::
	.string "You have no room for it.$"

Johto_DayCare_Text_NotEnoughMoneyText::
	.string "You don’t have enough money.$"

Johto_DayCare_Text_OhFineThenText::
	.string "Oh, fine then.$"

Johto_DayCare_Text_ComeAgainText::
	.string "Come again.$"

Johto_DayCare_Text_DayCareManText_GiveOddEgg::
	.string "I’m the DAY-CARE MAN.\p"
	.string "Do you know about EGGS?\p"
	.string "I was raising MOéMON with my wife,\n"
	.string "you see.\p"
	.string "We were shocked to find an EGG!\p"
	.string "How incredible is that?\p"
	.string "Well, wouldn’t you like this EGG?\p"
	.string "Then fine, this is yours to keep!$"

Johto_DayCare_Text_DayCareText_GotOddEgg::
	.string "{PLAYER} received ODD EGG!$"

Johto_DayCare_Text_DayCareText_DescribeOddEgg::
	.string "I found that when I was caring for\n"
	.string "someone’s MOéMON before.\p"
	.string "But the trainer didn’t want the\n"
	.string "EGG, so I’d kept it around.$"

Johto_DayCare_Text_DayCareText_PartyFull::
	.string "You’ve no room for this.$"

Johto_DayCare_Text_Text_GrampsLookingForYou::
	.string "Gramps was looking for you.$"
