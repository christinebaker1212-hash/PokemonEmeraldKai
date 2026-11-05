@ Moemon Kai: FireRed's FOUR ISLAND DAY CARE lady (audit case 47) on the game's DAY CARE: the save's one pair, the
@ same as ROUTE 117's (daycare.c)
Kanto_FourIsland_PokemonDayCare_MapScripts::
	.byte 0

Kanto_FourIsland_PokemonDayCare_EventScript_DaycareWoman::
	lock
	faceplayer
	specialvar VAR_RESULT, GetDaycareState
	goto_if_eq VAR_RESULT, DAYCARE_EGG_WAITING, Kanto_FourIsland_PokemonDayCare_EggWaiting
	goto_if_eq VAR_RESULT, DAYCARE_ONE_MON, Kanto_FourIsland_PokemonDayCare_OneMonInDaycare
	goto_if_eq VAR_RESULT, DAYCARE_TWO_MONS, Kanto_FourIsland_PokemonDayCare_TwoMonsInDaycare
	msgbox Kanto_FourIsland_PokemonDayCare_Text_WouldYouLikeUsToRaiseMon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FourIsland_PokemonDayCare_GiveMonToRaise
	msgbox Kanto_FourIsland_PokemonDayCare_Text_FineThenComeAgain, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_GiveMonToRaise::
	specialvar VAR_RESULT, CountPartyNonEggMons
	goto_if_eq VAR_RESULT, 1, Kanto_FourIsland_PokemonDayCare_OnlyOneMonInParty
	msgbox Kanto_FourIsland_PokemonDayCare_Text_WhichMonShouldWeRaise, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	special ChooseSendDaycareMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kanto_FourIsland_PokemonDayCare_ComeAgain
	specialvar VAR_RESULT, CountPartyAliveNonEggMons_IgnoreVar0x8004Slot
	goto_if_eq VAR_RESULT, 0, Kanto_FourIsland_PokemonDayCare_OnlyOneAliveMonInParty
	specialvar VAR_0x8005, GetSelectedMonNicknameAndSpecies
	waitse
	playmoncry VAR_0x8005, CRY_MODE_NORMAL
	msgbox Kanto_FourIsland_PokemonDayCare_Text_WellRaiseYourMon, MSGBOX_DEFAULT
	waitmoncry
	special StoreSelectedPokemonInDaycare
	incrementgamestat GAME_STAT_USED_DAYCARE
	specialvar VAR_RESULT, GetDaycareState
	goto_if_eq VAR_RESULT, DAYCARE_ONE_MON, Kanto_FourIsland_PokemonDayCare_CanRaiseOneMore
	release
	end

Kanto_FourIsland_PokemonDayCare_ComeAgain::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_ComeAgain, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_CanRaiseOneMore::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_WeCanRaiseOneMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FourIsland_PokemonDayCare_GiveMonToRaise
	goto Kanto_FourIsland_PokemonDayCare_ComeAgain
	end

Kanto_FourIsland_PokemonDayCare_OnlyOneMonInParty::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_YouHaveJustOneMon, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_OnlyOneAliveMonInParty::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_WhatWillYouBattleWith, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_EggWaiting::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_HusbandWasLookingForYou, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_MonHasGrownXLevels::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_YourMonHasGrownXLevels, MSGBOX_DEFAULT
	return

Kanto_FourIsland_PokemonDayCare_DisplayLevelsGained::
	specialvar VAR_RESULT, GetNumLevelsGainedFromDaycare
	call_if_ne VAR_RESULT, 0, Kanto_FourIsland_PokemonDayCare_MonHasGrownXLevels
	return

Kanto_FourIsland_PokemonDayCare_OneMonInDaycare::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_GoodToSeeYou, MSGBOX_DEFAULT
	setvar VAR_0x8004, 0
	call Kanto_FourIsland_PokemonDayCare_DisplayLevelsGained
	msgbox Kanto_FourIsland_PokemonDayCare_Text_WeCanRaiseOneMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FourIsland_PokemonDayCare_GiveMonToRaise
	msgbox Kanto_FourIsland_PokemonDayCare_Text_TakeYourMonBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FourIsland_PokemonDayCare_TryRetrieveMon
	goto Kanto_FourIsland_PokemonDayCare_ComeAgain
	end

Kanto_FourIsland_PokemonDayCare_TryRetrieveMon::
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, PARTY_SIZE, Kanto_FourIsland_PokemonDayCare_NoRoomInParty
	specialvar VAR_RESULT, GetDaycareState
	setvar VAR_0x8004, 0
	goto_if_eq VAR_RESULT, DAYCARE_ONE_MON, Kanto_FourIsland_PokemonDayCare_CostPrompt
	special ShowDaycareLevelMenu
	copyvar VAR_0x8004, VAR_RESULT
	goto_if_eq VAR_RESULT, DAYCARE_EXITED_LEVEL_MENU, Kanto_FourIsland_PokemonDayCare_ComeAgain
	goto Kanto_FourIsland_PokemonDayCare_CostPrompt
	end

Kanto_FourIsland_PokemonDayCare_CostPrompt::
	special GetDaycareCostAndPrepareString
	msgbox Kanto_FourIsland_PokemonDayCare_Text_ItWillCostX, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FourIsland_PokemonDayCare_CheckEnoughMoney
	goto Kanto_FourIsland_PokemonDayCare_ComeAgain
	end

Kanto_FourIsland_PokemonDayCare_CheckEnoughMoney::
	specialvar VAR_RESULT, IsEnoughForCostInVar0x8005
	goto_if_eq VAR_RESULT, TRUE, Kanto_FourIsland_PokemonDayCare_RetrieveMon
	msgbox Kanto_FourIsland_PokemonDayCare_Text_NotEnoughMoney, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_RetrieveMon::
	applymovement 1, Kanto_FourIsland_PokemonDayCare_Movement_RetrieveMon
	waitmovement 0
	specialvar VAR_RESULT, TakePokemonFromDaycare
	special SubtractMoneyFromVar0x8005
	playse SE_SHOP
	msgbox Kanto_FourIsland_PokemonDayCare_Text_HeresYourMon, MSGBOX_DEFAULT
	waitse
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	textcolor 3
	msgbox Kanto_FourIsland_PokemonDayCare_Text_TookBackMon, MSGBOX_DEFAULT
	textcolor 1
	waitmoncry
	specialvar VAR_RESULT, GetDaycareState
	goto_if_eq VAR_RESULT, DAYCARE_ONE_MON, Kanto_FourIsland_PokemonDayCare_AskRetrieveOtherMon
	goto Kanto_FourIsland_PokemonDayCare_ComeAgain
	end

Kanto_FourIsland_PokemonDayCare_AskRetrieveOtherMon::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_TakeOtherOneBackToo, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FourIsland_PokemonDayCare_TryRetrieveMon
	goto Kanto_FourIsland_PokemonDayCare_ComeAgain
	end

Kanto_FourIsland_PokemonDayCare_NoRoomInParty::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_YourPartyIsFull, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_Movement_RetrieveMon::
	delay_16
	delay_16
	face_right
	delay_16
	delay_16
	face_left
	delay_16
	delay_16
	face_up
	walk_slow_up
	set_invisible
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	face_down
	set_visible
	walk_slow_down
	step_end

Kanto_FourIsland_PokemonDayCare_TwoMonsInDaycare::
	msgbox Kanto_FourIsland_PokemonDayCare_Text_GoodToSeeYou, MSGBOX_DEFAULT
	setvar VAR_0x8004, 0
	call Kanto_FourIsland_PokemonDayCare_DisplayLevelsGained
	setvar VAR_0x8004, 1
	call Kanto_FourIsland_PokemonDayCare_DisplayLevelsGained
	msgbox Kanto_FourIsland_PokemonDayCare_Text_TakeYourMonBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_FourIsland_PokemonDayCare_TryRetrieveMon
	msgbox Kanto_FourIsland_PokemonDayCare_Text_ComeAgain, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_PokemonDayCare_Text_WouldYouLikeUsToRaiseMon::
	.string "I’m the DAY-CARE LADY.\p"
	.string "We can raise MOéMON for you.\p"
	.string "Would you like us to raise one?$"

Kanto_FourIsland_PokemonDayCare_Text_FineThenComeAgain::
	.string "Oh, fine, then.\n"
	.string "Come again.$"

Kanto_FourIsland_PokemonDayCare_Text_WhichMonShouldWeRaise::
	.string "Which MOéMON should we raise for\n"
	.string "you?$"

Kanto_FourIsland_PokemonDayCare_Text_WellRaiseYourMon::
	.string "Fine, we’ll raise your {STR_VAR_1}\n"
	.string "for a while.\p"
	.string "Come back for it later.$"

Kanto_FourIsland_PokemonDayCare_Text_ComeAgain::
	.string "Fine.\n"
	.string "Come again.$"

Kanto_FourIsland_PokemonDayCare_Text_WeCanRaiseOneMore::
	.string "We can raise two of your MOéMON.\p"
	.string "Would you like us to raise one\n"
	.string "more MOéMON for you?$"

Kanto_FourIsland_PokemonDayCare_Text_YouHaveJustOneMon::
	.string "Oh? But you have just one\n"
	.string "MOéMON.\p"
	.string "Come back another time.$"

Kanto_FourIsland_PokemonDayCare_Text_WhatWillYouBattleWith::
	.string "If you leave me that MOéMON,\n"
	.string "what will you battle with?\p"
	.string "Come back another time.$"

Kanto_FourIsland_PokemonDayCare_Text_HusbandWasLookingForYou::
	.string "My husband was looking for you.$"

Kanto_FourIsland_PokemonDayCare_Text_YourMonHasGrownXLevels::
	.string "By level, your {STR_VAR_1} has\n"
	.string "grown by {STR_VAR_2}.$"

Kanto_FourIsland_PokemonDayCare_Text_GoodToSeeYou::
	.string "Ah, it’s you!\n"
	.string "Good to see you.\p"
	.string "Your MOéMON can only be doing\n"
	.string "good!$"

Kanto_FourIsland_PokemonDayCare_Text_TakeYourMonBack::
	.string "Will you take your MOéMON back?$"

Kanto_FourIsland_PokemonDayCare_Text_ItWillCostX::
	.string "If you want your {STR_VAR_1} back,\n"
	.string "it will cost ¥{STR_VAR_2}.$"

Kanto_FourIsland_PokemonDayCare_Text_NotEnoughMoney::
	.string "You don’t have enough money…$"

Kanto_FourIsland_PokemonDayCare_Text_HeresYourMon::
	.string "Perfect!\n"
	.string "Here’s your MOéMON.$"

Kanto_FourIsland_PokemonDayCare_Text_TookBackMon::
	.string "{PLAYER} took back {STR_VAR_1} from\n"
	.string "the DAY-CARE LADY.$"

Kanto_FourIsland_PokemonDayCare_Text_TakeOtherOneBackToo::
	.string "Will you take back the other one,\n"
	.string "too?$"

Kanto_FourIsland_PokemonDayCare_Text_YourPartyIsFull::
	.string "Your MOéMON party is full.\n"
	.string "Make room, then come see me.$"
