@ Moemon Kai: TRIPLE TRIAD. A seller in each region's POKéMON CENTER (RUSTBORO's at the GAME CENTER's CARD SHOP): a
@ CARD CASE and a first hand for anyone without, then the counter (src/tt/tt_ui.c: SINGLES, PACKS, SELL). The CARD
@ QUEEN, who travels between the regions (she shows only in hers: src/tt/tt_field.c Tt_MapSetup) and stirs up their
@ rules. Pressing the CARD CASE's button in front of either starts a game.
EventScript_TtMapSetup::
	special Tt_MapSetup
	return

EventScript_TtSeller::
	lock
	faceplayer
	special Tt_SellerState
	goto_if_eq VAR_RESULT, TRUE, EventScript_TtSeller_Shop
	msgbox TtSeller_Text_Intro, MSGBOX_DEFAULT
	special Tt_SellerHasCase
	goto_if_eq VAR_RESULT, TRUE, EventScript_TtSeller_HasCase
	msgbox TtSeller_Text_Case, MSGBOX_DEFAULT
	giveitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, EventScript_TtSeller_NoRoom
EventScript_TtSeller_HasCase::
	msgbox TtSeller_Text_Starter, MSGBOX_DEFAULT
	closemessage
	special Tt_SellerStarter
	special Tt_SellerState
	msgbox TtSeller_Text_HowTo, MSGBOX_DEFAULT
EventScript_TtSeller_Shop::
	msgbox TtSeller_Text_Welcome, MSGBOX_DEFAULT
	closemessage
	special Tt_SellerShop
	msgbox TtSeller_Text_Bye, MSGBOX_DEFAULT
	release
	end

EventScript_TtSeller_NoRoom::
	msgbox TtSeller_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

EventScript_TtQueen::
	lock
	faceplayer
	special Tt_QueenFirstMeeting
	goto_if_eq VAR_RESULT, FALSE, EventScript_TtQueen_Menu
	msgbox TtQueen_Text_Intro, MSGBOX_DEFAULT
EventScript_TtQueen_Menu::
	msgbox TtQueen_Text_Hello, MSGBOX_DEFAULT
	closemessage
	special Tt_QueenTalk
	special Tt_QueenHere
	goto_if_eq VAR_RESULT, TRUE, EventScript_TtQueen_Stay
	msgbox TtQueen_Text_Leaving, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
EventScript_TtQueen_Stay::
	release
	end

@ The card players near the GAME CENTER's TRIPLE TRIAD seller: talking to them asks for a game
EventScript_TtPlayer::
	special Tt_SellerHasCase
	goto_if_eq VAR_RESULT, FALSE, EventScript_TtPlayer_NoCase
	special Tcg_TalkChallenge
	end

EventScript_TtPlayer_NoCase::
	msgbox TtPlayer_Text_NoCase, MSGBOX_NPC
	end

TtSeller_Text_Intro:
	.string "Welcome, welcome! TRIPLE TRIAD\n"
	.string "cards, from lands far beyond HOENN.\p"
	.string "Five cards, a three-by-three board,\n"
	.string "and the higher number takes the card\l"
	.string "it faces. Simple to learn!$"

TtSeller_Text_Case:
	.string "You'll need somewhere to keep your\n"
	.string "cards. Here, take this!$"

TtSeller_Text_Starter:
	.string "And every player needs a hand.\n"
	.string "These five are on the house!$"

TtSeller_Text_HowTo:
	.string "Your CARD CASE holds every card game.\n"
	.string "Switch sides with L and R.\p"
	.string "REGISTER it, press its button in front\n"
	.string "of someone who plays, and a game\l"
	.string "begins. Win, and you take their cards!\p"
	.string "Lose, and you keep yours. Nobody here\n"
	.string "plays for your cards, only for theirs.\p"
	.string "Mind the rules, though: each region\n"
	.string "plays its own, and the rules you\l"
	.string "carry from one region can spread to\l"
	.string "the next. Or wear its rules away!$"

TtSeller_Text_Welcome:
	.string "Welcome to {STR_VAR_1}'s TRIPLE TRIAD\n"
	.string "counter!\p"
	.string "The rules around here:\n"
	.string "{STR_VAR_2}.\p"
	.string "TRADE RULE: {STR_VAR_3}.$"

TtSeller_Text_Bye:
	.string "May the numbers favor you!$"

TtSeller_Text_NoRoom:
	.string "Oh, your BAG is full...\n"
	.string "Come back when you have room!$"

TtQueen_Text_Intro:
	.string "My, my. A new face at the table.\p"
	.string "They call me the CARD QUEEN. I travel\n"
	.string "HOENN, and wherever I go, the rules\l"
	.string "go a little wild.\p"
	.string "Rules spread more easily where I am,\n"
	.string "and old ones fall away faster.\p"
	.string "Beat me, and I'll grant you a favor:\n"
	.string "a new TRADE RULE here, or a rule\l"
	.string "abolished. Pay my way, and I'll go\l"
	.string "wherever you like.$"

TtQueen_Text_Hello:
	.string "Well? What will it be, darling?$"

TtQueen_Text_Leaving:
	.string "Off I go! Look for me in my new\n"
	.string "region. The rules won't wait!$"

TtPlayer_Text_NoCase:
	.string "You want to play? You'll need a\n"
	.string "CARD CASE. Ask at the counter!$"
