@ Moemon Kai: MAGIC at MOSSDEEP's CARD SHOP (the old GAME CORNER: docs/MTG_DECISIONS.md). The clerk at the counter
@ hands out a CARD CASE if needed and a starter deck (src/mtg/mtg_ui.c), then shows the way to the play room below.
@ The regulars upstairs and down (gMtgDuelists in src/mtg/mtg_decks.c, by VAR_0x8004) don't let on that they play
@ MAGIC until the player has a deck; then talking to them asks for a duel.
EventScript_MtgClerk::
	lock
	faceplayer
	special Mtg_ShopState
	goto_if_eq VAR_RESULT, TRUE, EventScript_MtgClerk_Regular
	msgbox MtgClerk_Text_Intro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_MtgClerk_No
	checkitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, TRUE, EventScript_MtgClerk_HasCase
	msgbox MtgClerk_Text_Case, MSGBOX_DEFAULT
	giveitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, EventScript_MtgClerk_NoRoom
EventScript_MtgClerk_HasCase::
	msgbox MtgClerk_Text_Pick, MSGBOX_DEFAULT
	closemessage
	special Mtg_ShopStarter
	special Mtg_ShopState
	goto_if_eq VAR_RESULT, FALSE, EventScript_MtgClerk_No
	msgbox MtgClerk_Text_Welcome, MSGBOX_DEFAULT
	release
	end

EventScript_MtgClerk_Regular::
	msgbox MtgClerk_Text_Boosters, MSGBOX_DEFAULT
	closemessage
	special Mtg_ShopBoosters
	msgbox MtgClerk_Text_Downstairs, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_MtgClerk_Bye
	msgbox MtgClerk_Text_ThisWay, MSGBOX_DEFAULT
	closemessage
	warp MAP_MOSSDEEP_CITY_GAME_CORNER_B1F, 3, 2
	waitstate
	release
	end

EventScript_MtgClerk_Bye::
	msgbox MtgClerk_Text_Bye, MSGBOX_DEFAULT
	release
	end

EventScript_MtgClerk_No::
	msgbox MtgClerk_Text_No, MSGBOX_DEFAULT
	release
	end

EventScript_MtgClerk_NoRoom::
	msgbox MtgClerk_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

EventScript_MtgShopSign::
	msgbox MtgShop_Text_Sign, MSGBOX_SIGN
	end

@ the regulars: VAR_0x8004 = their place in gMtgDuelists
EventScript_MtgDuelist0::
	setvar VAR_0x8004, 0
	goto EventScript_MtgDuelist
EventScript_MtgDuelist1::
	setvar VAR_0x8004, 1
	goto EventScript_MtgDuelist
EventScript_MtgDuelist2::
	setvar VAR_0x8004, 2
	goto EventScript_MtgDuelist
EventScript_MtgDuelist3::
	setvar VAR_0x8004, 3
	goto EventScript_MtgDuelist
EventScript_MtgDuelist4::
	setvar VAR_0x8004, 4
	goto EventScript_MtgDuelist
EventScript_MtgDuelist5::
	setvar VAR_0x8004, 5
	goto EventScript_MtgDuelist

EventScript_MtgDuelist::
	lock
	faceplayer
	special Mtg_DuelistState
	goto_if_eq VAR_RESULT, FALSE, EventScript_MtgDuelist_NoDeck
	special Mtg_DuelistHello
	msgbox gStringVar4, MSGBOX_DEFAULT
	special Mtg_DuelistChallenge
	release
	end

EventScript_MtgDuelist_NoDeck::
	msgbox MtgDuelist_Text_NoDeck, MSGBOX_DEFAULT
	release
	end

MtgClerk_Text_Intro:
	.string "Welcome to the CARD SHOP!\p"
	.string "We play a card game here called\n"
	.string "MAGIC. Not many people know it…\p"
	.string "Would you like to learn? I can give\n"
	.string "you a deck to start with.$"

MtgClerk_Text_Case:
	.string "First, something to keep your cards\n"
	.string "in. Here, take this!$"

MtgClerk_Text_Pick:
	.string "There are five starter decks, one for\n"
	.string "each color of MAGIC. Pick the one\l"
	.string "that suits you!$"

MtgClerk_Text_Welcome:
	.string "Now you're one of us! The regulars\n"
	.string "will be glad of a new face.\p"
	.string "Talk to anyone here to play. There's\n"
	.string "a play room downstairs, too.$"

MtgClerk_Text_Boosters:
	.string "Welcome back! Boosters of every set\n"
	.string "are on the counter, for CARD POINTS.$"

MtgClerk_Text_Downstairs:
	.string "Would you like to go down to the\n"
	.string "play room?$"

MtgClerk_Text_ThisWay:
	.string "Right this way. Have fun!$"

MtgClerk_Text_Bye:
	.string "The regulars up here are always\n"
	.string "ready for a game, too.$"

MtgClerk_Text_No:
	.string "That's all right. Come back if you\n"
	.string "ever change your mind!$"

MtgClerk_Text_NoRoom:
	.string "Oh, your BAG is full. Make some room\n"
	.string "and come back!$"

MtgShop_Text_Sign:
	.string "MOSSDEEP CARD SHOP\n"
	.string "New players welcome at the counter.$"

MtgDuelist_Text_NoDeck:
	.string "Hm? Oh, nothing. We're just talking.\n"
	.string "Don't mind us.$"
