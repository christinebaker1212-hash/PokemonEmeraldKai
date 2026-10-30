@ Moemon Co-op: the GAME CENTER's CARD SHOP: the best selection in HOENN (singles the CARD CLUB counters don't
@ have, and three structure decks), a CARD CASE for anyone without one, and card players at the table.
RustboroCity_GameCenter_2F_MapScripts::
	.byte 0

RustboroCity_GameCenter_2F_EventScript_CardShop::
	lock
	faceplayer
	special Tcg_ClubState
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_GameCenter_2F_EventScript_Shop
	msgbox RustboroCity_GameCenter_2F_Text_NoCase, MSGBOX_DEFAULT
	giveitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_GameCenter_2F_EventScript_NoRoom
	msgbox RustboroCity_GameCenter_2F_Text_ChooseDeck, MSGBOX_DEFAULT
	closemessage
	special Tcg_ClubChooseDeck
	msgbox RustboroCity_GameCenter_2F_Text_HowTo, MSGBOX_DEFAULT
RustboroCity_GameCenter_2F_EventScript_Shop::
	special Tcg_ClubGiftReady
	goto_if_eq VAR_RESULT, TRUE, EventScript_CardClub_Gift
	msgbox RustboroCity_GameCenter_2F_Text_Welcome, MSGBOX_DEFAULT
	closemessage
	special Tcg_GameCenterShop
	msgbox RustboroCity_GameCenter_2F_Text_Bye, MSGBOX_DEFAULT
	release
	end

RustboroCity_GameCenter_2F_EventScript_NoRoom::
	msgbox RustboroCity_GameCenter_2F_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

@ The Yu-Gi-Oh! clerk: a CARD CASE and a Starter Deck for anyone without, then singles and Starter Decks for money
@ and the CARD CLUB duelist's counter (src/ygo/ygo_ui.c, the GAME CENTER counter)
RustboroCity_GameCenter_2F_EventScript_SecondSeller::
	lock
	faceplayer
	special Ygo_ClubState
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_GameCenter_2F_EventScript_YgoShop
	msgbox RustboroCity_GameCenter_2F_Text_YgoIntro, MSGBOX_DEFAULT
	special Ygo_ClubHasCase
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_GameCenter_2F_EventScript_YgoHasCase
	msgbox RustboroCity_GameCenter_2F_Text_YgoCase, MSGBOX_DEFAULT
	giveitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_GameCenter_2F_EventScript_NoRoom
RustboroCity_GameCenter_2F_EventScript_YgoHasCase::
	msgbox RustboroCity_GameCenter_2F_Text_YgoChooseDeck, MSGBOX_DEFAULT
	closemessage
	special Ygo_ClubChooseDeck
	msgbox RustboroCity_GameCenter_2F_Text_YgoHowTo, MSGBOX_DEFAULT
RustboroCity_GameCenter_2F_EventScript_YgoShop::
	msgbox RustboroCity_GameCenter_2F_Text_YgoWelcome, MSGBOX_DEFAULT
	closemessage
	special Ygo_GameCenterShop
	msgbox RustboroCity_GameCenter_2F_Text_Bye, MSGBOX_DEFAULT
	release
	end

@ The card players at the table: talking to them asks for a duel (they always play)
RustboroCity_GameCenter_2F_EventScript_CardPlayer1::
RustboroCity_GameCenter_2F_EventScript_CardPlayer2::
RustboroCity_GameCenter_2F_EventScript_CardPlayer3::
	special Tcg_ClubState
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_GameCenter_2F_EventScript_PlayerNoCase
	special Tcg_TalkChallenge
	end

RustboroCity_GameCenter_2F_EventScript_PlayerNoCase::
	msgbox RustboroCity_GameCenter_2F_Text_PlayerNoCase, MSGBOX_NPC
	end

RustboroCity_GameCenter_2F_EventScript_FloorGuide::
	msgbox RustboroCity_GameCenter_2F_Text_FloorGuide, MSGBOX_SIGN
	end

RustboroCity_GameCenter_2F_EventScript_Shelf::
	msgbox RustboroCity_GameCenter_2F_Text_Shelf, MSGBOX_SIGN
	end

RustboroCity_GameCenter_2F_EventScript_Case::
	msgbox RustboroCity_GameCenter_2F_Text_Case, MSGBOX_SIGN
	end

RustboroCity_GameCenter_2F_EventScript_Rack::
	msgbox RustboroCity_GameCenter_2F_Text_Rack, MSGBOX_SIGN
	end

RustboroCity_GameCenter_2F_Text_NoCase:
	.string "Welcome to the GAME CENTER's\n"
	.string "CARD SHOP!\p"
	.string "Hm? No CARD CASE? Then you can't\n"
	.string "leave empty-handed. Here!$"

RustboroCity_GameCenter_2F_Text_ChooseDeck:
	.string "And a case needs a deck.\n"
	.string "Pick the one you like!$"

RustboroCity_GameCenter_2F_Text_HowTo:
	.string "REGISTER the CARD CASE to a button\n"
	.string "in your BAG, and press it in front\l"
	.string "of someone to ask them for a duel.\p"
	.string "The players at our table will duel\n"
	.string "you any time. Just talk to them!$"

RustboroCity_GameCenter_2F_Text_Welcome:
	.string "Welcome to the best CARD SHOP in\n"
	.string "HOENN! Have a look around.$"

RustboroCity_GameCenter_2F_Text_Bye:
	.string "Thank you! Come back soon!$"

RustboroCity_GameCenter_2F_Text_NoRoom:
	.string "Oh, your BAG is full...\n"
	.string "Come back when you have room!$"

RustboroCity_GameCenter_2F_Text_YgoIntro:
	.string "All unpacked! Welcome to the\n"
	.string "Yu-Gi-Oh! counter!\p"
	.string "Hm? You don't duel yet?\n"
	.string "Let's fix that!$"

RustboroCity_GameCenter_2F_Text_YgoCase:
	.string "First, somewhere to keep your cards.\n"
	.string "Here, take this!$"

RustboroCity_GameCenter_2F_Text_YgoChooseDeck:
	.string "And every duelist needs a deck.\n"
	.string "Pick a Starter Deck!$"

RustboroCity_GameCenter_2F_Text_YgoHowTo:
	.string "Your CARD CASE holds both games.\n"
	.string "Switch sides with L and R.\p"
	.string "I sell singles and Starter Decks,\n"
	.string "one of each per customer!$"

RustboroCity_GameCenter_2F_Text_YgoWelcome:
	.string "Welcome to the Yu-Gi-Oh! counter!\n"
	.string "Singles, Starter Decks and more!$"

RustboroCity_GameCenter_2F_Text_PlayerNoCase:
	.string "You want to play? You'll need a\n"
	.string "CARD CASE. Ask at the counter!$"

RustboroCity_GameCenter_2F_Text_FloorGuide:
	.string "2F: CARD SHOP\n"
	.string "Singles, structure decks and duels!$"

RustboroCity_GameCenter_2F_Text_Shelf:
	.string "Sleeves, binders and deck boxes in\n"
	.string "every color.$"

RustboroCity_GameCenter_2F_Text_Case:
	.string "Rare cards shine behind the glass.\n"
	.string "Some of them are holos!$"

RustboroCity_GameCenter_2F_Text_Rack:
	.string "Racks of booster packs: COLOSSEUM,\n"
	.string "EVOLUTION, MYSTERY, LABORATORY...\p"
	.string "The sign says: Booster packs are\n"
	.string "prizes for card duels!$"
