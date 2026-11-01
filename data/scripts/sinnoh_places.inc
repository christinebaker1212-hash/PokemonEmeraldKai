@ Moemon Kai: SINNOH places of this game's own (words written for this game)

@ VEILSTONE's GAME CORNER: the coins counter and 12 slot machines (Emerald's, as MAUVILLE's)
Sinnoh_EventScript_GameCornerClerk::
	lock
	faceplayer
	msgbox Sinnoh_Text_GameCornerClerk, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NeedCoinCase
	message Sinnoh_Text_GameCornerCoins
	waitmessage
	showmoneybox 0, 0
	showcoinsbox 1, 6
	goto MauvilleCity_GameCorner_EventScript_ChooseCoinsDefault50
	end

Sinnoh_EventScript_SlotMachine0::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine1::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine2::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 2
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine3::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 3
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine4::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 4
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine5::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 5
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine6::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 6
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine7::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 7
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine8::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 8
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine9::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 9
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine10::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 10
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine11::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 11
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_Text_GameCornerClerk:
	.string "Welcome to VEILSTONE's GAME CORNER!\n"
	.string "The slots are hot today!$"
Sinnoh_Text_GameCornerCoins:
	.string "Can I get you some COINS?$"

@ VEILSTONE's PRIZE EXCHANGE: COINS for Platinum's prizes (src/sinnoh/s3d_places.c Sinnoh_PrizeMenu)
Sinnoh_EventScript_PrizeExchange::
	lock
	faceplayer
	msgbox Sinnoh_Text_PrizeWelcome, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_PrizeNoCase
	showcoinsbox 20, 1
Sinnoh_EventScript_PrizePick::
	message Sinnoh_Text_PrizeWhich
	waitmessage
	special Sinnoh_PrizeMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Sinnoh_EventScript_PrizeBye
	msgbox Sinnoh_Text_PrizeConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_PrizePick
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, VAR_0x8006, Sinnoh_EventScript_PrizeNoCoins
	checkitemspace VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_PrizeNoRoom
	removecoins VAR_0x8006
	additem VAR_0x8005
	updatecoinsbox 20, 1
	playse SE_SHOP
	msgbox Sinnoh_Text_PrizeHere, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_PrizePick

Sinnoh_EventScript_PrizeNoCoins::
	msgbox Sinnoh_Text_PrizeNoCoins, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_PrizePick

Sinnoh_EventScript_PrizeNoRoom::
	msgbox Sinnoh_Text_PrizeNoRoom, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_PrizePick

Sinnoh_EventScript_PrizeBye::
	hidecoinsbox 20, 1
	msgbox Sinnoh_Text_PrizeBye, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_PrizeNoCase::
	msgbox Sinnoh_Text_PrizeNoCase, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_PrizeWelcome:
	.string "Welcome to the PRIZE EXCHANGE!\n"
	.string "Trade your COINS for fine prizes.$"

Sinnoh_Text_PrizeWhich:
	.string "Which prize would you like?$"

Sinnoh_Text_PrizeConfirm:
	.string "{STR_VAR_1} for {STR_VAR_2} COINS.\n"
	.string "Is that right?$"

Sinnoh_Text_PrizeHere:
	.string "Here you are! Enjoy it!$"

Sinnoh_Text_PrizeNoCoins:
	.string "Oh dear. You don't have enough\n"
	.string "COINS for that one.$"

Sinnoh_Text_PrizeNoRoom:
	.string "Your BAG has no room for it.$"

Sinnoh_Text_PrizeBye:
	.string "Come again with more COINS!$"

Sinnoh_Text_PrizeNoCase:
	.string "You'll need a COIN CASE to hold\n"
	.string "COINS before you can trade them.$"

@ The BATTLE FRONTIER's exchange booths: BATTLE POINTS for items (left) and TMs (right): src/sinnoh/s3d_places.c
Sinnoh_EventScript_BpItems::
	lock
	faceplayer
	setvar VAR_TEMP_3, 0
	goto Sinnoh_EventScript_BpBooth

Sinnoh_EventScript_BpTms::
	lock
	faceplayer
	setvar VAR_TEMP_3, 1
Sinnoh_EventScript_BpBooth::
	msgbox Sinnoh_Text_BpWelcome, MSGBOX_DEFAULT
Sinnoh_EventScript_BpPick::
	specialvar VAR_RESULT, GetFrontierBattlePoints
	buffernumberstring STR_VAR_3, VAR_RESULT
	message Sinnoh_Text_BpWhich
	waitmessage
	copyvar VAR_0x8004, VAR_TEMP_3
	special Sinnoh_BpMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Sinnoh_EventScript_BpBye
	msgbox Sinnoh_Text_BpConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_BpPick
	special Sinnoh_BpBuy
	goto_if_eq VAR_RESULT, 1, Sinnoh_EventScript_BpShort
	goto_if_eq VAR_RESULT, 2, Sinnoh_EventScript_BpNoRoom
	playse SE_SHOP
	msgbox Sinnoh_Text_BpHere, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_BpPick

Sinnoh_EventScript_BpShort::
	msgbox Sinnoh_Text_BpShort, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_BpPick

Sinnoh_EventScript_BpNoRoom::
	msgbox Sinnoh_Text_PrizeNoRoom, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_BpPick

Sinnoh_EventScript_BpBye::
	msgbox Sinnoh_Text_BpBye, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_BpWelcome:
	.string "Welcome! Here you can trade BATTLE\n"
	.string "POINTS for prizes.$"

Sinnoh_Text_BpWhich:
	.string "You have {STR_VAR_3} BP.\n"
	.string "What would you like?$"

Sinnoh_Text_BpConfirm:
	.string "{STR_VAR_1} for {STR_VAR_2} BP.\n"
	.string "Is that right?$"

Sinnoh_Text_BpHere:
	.string "Here you are! Battle well!$"

Sinnoh_Text_BpShort:
	.string "You don't have enough BATTLE POINTS\n"
	.string "for that one yet.$"

Sinnoh_Text_BpBye:
	.string "Win big at the FRONTIER and come\n"
	.string "back for more!$"

@ PASTORIA's gym: a button stepped on moves the water (src/sinnoh/s3d_gyms.c Sinnoh_PastoriaWater; VAR_0x8004 the level)
Sinnoh_EventScript_PastoriaButton::
	lockall
	special Sinnoh_PastoriaWater
	waitstate
	releaseall
	end

@ SUNYSHORE's gym: a button turns the room's gears (src/sinnoh/s3d_gyms.c Sinnoh_SunyshoreTurn; VAR_0x8004 quarter turns)
Sinnoh_EventScript_SunyshoreButton::
	lockall
	special Sinnoh_SunyshoreTurn
	waitstate
	releaseall
	end

@ CANALAVE's gym: the platform stepped on carries the player (src/sinnoh/s3d_gyms.c Sinnoh_CanalaveRide)
Sinnoh_EventScript_CanalaveRide::
	lockall
	special Sinnoh_CanalaveRide
	waitstate
	releaseall
	end
