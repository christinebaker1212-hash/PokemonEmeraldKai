@ Moemon Kai: FireRed's ROUTE 5 DAY-CARE (pokefirered data/scripts/day_care.inc; audit case 47): one POKéMON, its own
@ slot (daycare.c sRoute5DayCareMon, kept in "<save>.region")
Kanto_Route5_PokemonDayCare_MapScripts::
	.byte 0

Kanto_Route5_PokemonDayCare_EventScript_DaycareMan::
	lock
	faceplayer
	showmoneybox 0, 0
	specialvar VAR_RESULT, IsThereMonInRoute5Daycare
	goto_if_eq VAR_RESULT, TRUE, Kanto_Route5_PokemonDayCare_EventScript_CheckOnMon
	msgbox Kanto_Route5_PokemonDayCare_Text_WantMeToRaiseMon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_Route5_PokemonDayCare_EventScript_TryGiveMon
	msgbox Kanto_Route5_PokemonDayCare_Text_ComeAgain, MSGBOX_DEFAULT
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_TryGiveMon::
	specialvar VAR_RESULT, CountPartyNonEggMons
	goto_if_eq VAR_RESULT, 1, Kanto_Route5_PokemonDayCare_EventScript_OnlyOneMonInParty
	msgbox Kanto_Route5_PokemonDayCare_Text_WhichMonShouldIRaise, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	hidemoneybox
	special ChooseSendDaycareMon
	showmoneybox 0, 0
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kanto_Route5_PokemonDayCare_EventScript_ComeAgain
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, Kanto_Route5_PokemonDayCare_EventScript_OnlyOneAliveMonInParty
	specialvar VAR_0x8005, GetSelectedMonNicknameAndSpecies
	msgbox Kanto_Route5_PokemonDayCare_Text_LookAfterMonForAWhile, MSGBOX_DEFAULT
	waitse
	playmoncry VAR_0x8005, CRY_MODE_NORMAL
	msgbox Kanto_Route5_PokemonDayCare_Text_ComeSeeMeInAWhile, MSGBOX_DEFAULT
	waitmoncry
	special PutMonInRoute5Daycare
	incrementgamestat GAME_STAT_USED_DAYCARE
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_ComeAgain::
	msgbox Kanto_Route5_PokemonDayCare_Text_ComeAnytimeYouLike, MSGBOX_DEFAULT
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_OnlyOneMonInParty::
	msgbox Kanto_Route5_PokemonDayCare_Text_OnlyHaveOneMonWithYou, MSGBOX_DEFAULT
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_OnlyOneAliveMonInParty::
	msgbox Kanto_Route5_PokemonDayCare_Text_WhatWillYouBattleWith, MSGBOX_DEFAULT
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_CheckOnMon::
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, GetNumLevelsGainedForRoute5DaycareMon
	call_if_ne VAR_RESULT, 0, Kanto_Route5_PokemonDayCare_EventScript_MonHasGrownByXLevels
	call_if_eq VAR_RESULT, 0, Kanto_Route5_PokemonDayCare_EventScript_NotEnoughTime
	special GetCostToWithdrawRoute5DaycareMon
	msgbox Kanto_Route5_PokemonDayCare_Text_OweMeXForMonsReturn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_Route5_PokemonDayCare_EventScript_TryRetrieveMon
	goto Kanto_Route5_PokemonDayCare_EventScript_ComeAgain
	end

Kanto_Route5_PokemonDayCare_EventScript_MonHasGrownByXLevels::
	msgbox Kanto_Route5_PokemonDayCare_Text_MonHasGrownByXLevels, MSGBOX_DEFAULT
	return

Kanto_Route5_PokemonDayCare_EventScript_NotEnoughTime::
	msgbox Kanto_Route5_PokemonDayCare_Text_MonNeedsToSpendMoreTime, MSGBOX_DEFAULT
	return

Kanto_Route5_PokemonDayCare_EventScript_TryRetrieveMon::
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, PARTY_SIZE, Kanto_Route5_PokemonDayCare_EventScript_NoRoomInParty
	specialvar VAR_RESULT, IsEnoughForCostInVar0x8005
	goto_if_eq VAR_RESULT, TRUE, Kanto_Route5_PokemonDayCare_EventScript_RetrieveMon
	msgbox Kanto_Route5_PokemonDayCare_Text_DontHaveEnoughMoney, MSGBOX_DEFAULT
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_RetrieveMon::
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, TakePokemonFromRoute5Daycare
	special SubtractMoneyFromVar0x8005
	updatemoneybox
	copyvar VAR_0x8008, VAR_RESULT
	getpartysize
	subvar VAR_RESULT, 1
	bufferpartymonnick STR_VAR_1, VAR_RESULT
	copyvar VAR_RESULT, VAR_0x8008
	msgbox Kanto_Route5_PokemonDayCare_Text_ThankYouHeresMon, MSGBOX_DEFAULT
	textcolor 3
	waitse
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	msgbox Kanto_Route5_PokemonDayCare_Text_PlayerGotMonBack, MSGBOX_DEFAULT
	textcolor 0
	waitmoncry
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_NoRoomInParty::
	msgbox Kanto_Route5_PokemonDayCare_Text_YouveGotNoRoomForIt, MSGBOX_DEFAULT
	goto Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox
	end

Kanto_Route5_PokemonDayCare_EventScript_CloseMoneyBox::
	hidemoneybox
	release
	end

Kanto_Route5_PokemonDayCare_Text_WantMeToRaiseMon::
	.string "I run the DAY-CARE service.\p"
	.string "Would you like me to raise one of\n"
	.string "your MOéMON?$"

Kanto_Route5_PokemonDayCare_Text_ComeAgain::
	.string "Come again.$"

Kanto_Route5_PokemonDayCare_Text_WhichMonShouldIRaise::
	.string "Which MOéMON should I raise?$"

Kanto_Route5_PokemonDayCare_Text_LookAfterMonForAWhile::
	.string "Fine, I’ll look after your\n"
	.string "{STR_VAR_1} for a while.$"

Kanto_Route5_PokemonDayCare_Text_ComeSeeMeInAWhile::
	.string "Come see me in a while.$"

Kanto_Route5_PokemonDayCare_Text_ComeAnytimeYouLike::
	.string "Fine.\n"
	.string "Come anytime you like.$"

Kanto_Route5_PokemonDayCare_Text_OnlyHaveOneMonWithYou::
	.string "Oh? You only have one MOéMON\n"
	.string "with you.$"

Kanto_Route5_PokemonDayCare_Text_WhatWillYouBattleWith::
	.string "If you leave me that MOéMON,\n"
	.string "what will you battle with?$"

Kanto_Route5_PokemonDayCare_Text_OweMeXForMonsReturn::
	.string "You owe me ¥{STR_VAR_2} for the return\n"
	.string "of this MOéMON.$"

Kanto_Route5_PokemonDayCare_Text_MonHasGrownByXLevels::
	.string "Your {STR_VAR_1} has grown a lot.\n"
	.string "Yes, quite a lot, I’d say.\p"
	.string "Let me see…\n"
	.string "By level, it’s grown by {STR_VAR_2}.\p"
	.string "Aren’t I great?$"

Kanto_Route5_PokemonDayCare_Text_MonNeedsToSpendMoreTime::
	.string "You’re back already?\p"
	.string "Your {STR_VAR_1} needs to spend\n"
	.string "some more time with me.$"

Kanto_Route5_PokemonDayCare_Text_DontHaveEnoughMoney::
	.string "You don’t have enough money.$"

Kanto_Route5_PokemonDayCare_Text_ThankYouHeresMon::
	.string "Thank you!\n"
	.string "Here’s your MOéMON.$"

Kanto_Route5_PokemonDayCare_Text_PlayerGotMonBack::
	.string "{PLAYER} got {STR_VAR_1} back\n"
	.string "from the DAY-CARE MAN.$"

Kanto_Route5_PokemonDayCare_Text_YouveGotNoRoomForIt::
	.string "You can’t take this MOéMON back\n"
	.string "if you’ve got no room for it.$"
