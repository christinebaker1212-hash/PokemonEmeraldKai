@ Moemon Co-op: the GAME CENTER's games. Played with COINS from the COIN CASE; for now the machines are only
@ looked at (the coin counter sells COINS).
RustboroCity_GameCenter_3F_MapScripts::
	.byte 0

RustboroCity_GameCenter_3F_EventScript_CoinCounter::
	lock
	faceplayer
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_GameCenter_3F_EventScript_NoCoinCase
	showmoneybox 0, 0
	showcoinsbox 1, 6
	msgbox RustboroCity_GameCenter_3F_Text_BuyCoins, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_GameCenter_3F_EventScript_CoinsBye
	checkcoins VAR_TEMP_1
	goto_if_ge VAR_TEMP_1, (MAX_COINS + 1 - 50), RustboroCity_GameCenter_3F_EventScript_CoinCaseFull
	checkmoney 1000
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_GameCenter_3F_EventScript_NoMoneyForCoins
	removemoney 1000
	addcoins 50
	updatemoneybox
	updatecoinsbox 1, 6
	playse SE_SHOP
	msgbox RustboroCity_GameCenter_3F_Text_HereYouGo, MSGBOX_DEFAULT
	goto RustboroCity_GameCenter_3F_EventScript_CoinsDone

RustboroCity_GameCenter_3F_EventScript_CoinsBye::
	msgbox RustboroCity_GameCenter_3F_Text_CoinsBye, MSGBOX_DEFAULT
RustboroCity_GameCenter_3F_EventScript_CoinsDone::
	hidemoneybox
	hidecoinsbox 0, 5
	release
	end

RustboroCity_GameCenter_3F_EventScript_CoinCaseFull::
	msgbox RustboroCity_GameCenter_3F_Text_CoinCaseFull, MSGBOX_DEFAULT
	goto RustboroCity_GameCenter_3F_EventScript_CoinsDone

RustboroCity_GameCenter_3F_EventScript_NoMoneyForCoins::
	msgbox RustboroCity_GameCenter_3F_Text_NoMoneyForCoins, MSGBOX_DEFAULT
	goto RustboroCity_GameCenter_3F_EventScript_CoinsDone

RustboroCity_GameCenter_3F_EventScript_NoCoinCase::
	msgbox RustboroCity_GameCenter_3F_Text_NoCoinCase, MSGBOX_DEFAULT
	release
	end

RustboroCity_GameCenter_3F_EventScript_Console::
	msgbox RustboroCity_GameCenter_3F_Text_Console, MSGBOX_SIGN
	end

RustboroCity_GameCenter_3F_EventScript_Tv::
	msgbox RustboroCity_GameCenter_3F_Text_Tv, MSGBOX_SIGN
	end

RustboroCity_GameCenter_3F_EventScript_Arcade::
	msgbox RustboroCity_GameCenter_3F_Text_Arcade, MSGBOX_SIGN
	end

RustboroCity_GameCenter_3F_EventScript_Poster::
	msgbox RustboroCity_GameCenter_3F_Text_Poster, MSGBOX_SIGN
	end

RustboroCity_GameCenter_3F_EventScript_FloorGuide::
	msgbox RustboroCity_GameCenter_3F_Text_FloorGuide, MSGBOX_SIGN
	end

RustboroCity_GameCenter_3F_EventScript_Gamer1::
	msgbox RustboroCity_GameCenter_3F_Text_Gamer1, MSGBOX_NPC
	end

RustboroCity_GameCenter_3F_EventScript_Gamer2::
	msgbox RustboroCity_GameCenter_3F_Text_Gamer2, MSGBOX_NPC
	end

RustboroCity_GameCenter_3F_EventScript_ArcadeFan::
	msgbox RustboroCity_GameCenter_3F_Text_ArcadeFan, MSGBOX_NPC
	end

RustboroCity_GameCenter_3F_EventScript_Girl::
	msgbox RustboroCity_GameCenter_3F_Text_Girl, MSGBOX_NPC
	end

RustboroCity_GameCenter_3F_Text_BuyCoins:
	.string "Welcome to the GAME CENTER's\n"
	.string "COIN counter!\p"
	.string "It's ¥1000 for 50 COINS.\n"
	.string "Would you like some?$"

RustboroCity_GameCenter_3F_Text_HereYouGo:
	.string "Here are your COINS!\n"
	.string "Have fun!$"

RustboroCity_GameCenter_3F_Text_CoinsBye:
	.string "Come back any time!$"

RustboroCity_GameCenter_3F_Text_CoinCaseFull:
	.string "Your COIN CASE is full!$"

RustboroCity_GameCenter_3F_Text_NoMoneyForCoins:
	.string "I'm sorry, it's ¥1000 for 50 COINS.$"

RustboroCity_GameCenter_3F_Text_NoCoinCase:
	.string "You'll need a COIN CASE to keep\n"
	.string "your COINS in, I'm afraid.\p"
	.string "MAUVILLE CITY's GAME CORNER might\n"
	.string "know where to find one.$"

RustboroCity_GameCenter_3F_Text_Console:
	.string "A game console hooked up to a TV.\n"
	.string "A racing game is on the screen.\p"
	.string "INSERT COINS... It isn't taking\n"
	.string "COINS yet.$"

RustboroCity_GameCenter_3F_Text_Tv:
	.string "The TVs show a demo: MOéMON racing\n"
	.string "around a rainbow track!$"

RustboroCity_GameCenter_3F_Text_Arcade:
	.string "An arcade cabinet. Bright lights,\n"
	.string "loud music... COMING SOON!$"

RustboroCity_GameCenter_3F_Text_Poster:
	.string "A poster: THE GAME CENTER'S FIRST\n"
	.string "HIGH SCORE TOURNAMENT. Date to be\l"
	.string "announced!$"

RustboroCity_GameCenter_3F_Text_FloorGuide:
	.string "3F: GAMES\n"
	.string "Played with COINS from your COIN CASE.$"

RustboroCity_GameCenter_3F_Text_Gamer1:
	.string "I've been on this level for an hour.\n"
	.string "Don't break my concentration!$"

RustboroCity_GameCenter_3F_Text_Gamer2:
	.string "These consoles are way better than\n"
	.string "the one I have at home.$"

RustboroCity_GameCenter_3F_Text_ArcadeFan:
	.string "They say new games are coming to\n"
	.string "these cabinets soon. I can't wait!$"

RustboroCity_GameCenter_3F_Text_Girl:
	.string "The music up here makes me want\n"
	.string "to dance!$"
