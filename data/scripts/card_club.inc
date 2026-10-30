@ Moemon Kai: the CARD CLUB member (OLDALE and MAUVILLE POKéMON MARTS, LILYCOVE DEPARTMENT STORE). Hands over
@ the CARD CASE and a first deck (src/tcg/tcg_ui.c: the three decks turned over on the table),
@ offers a teaching duel, then runs the counter: TRADE IN, CARD SHOP, PRIZES. The gift for
@ every card collected comes from here too (src/tcg/tcg_field.c).
EventScript_CardClub::
	lock
	faceplayer
	special Tcg_ClubState
	goto_if_eq VAR_RESULT, TRUE, EventScript_CardClub_Counter
	msgbox CardClub_Text_Intro, MSGBOX_DEFAULT
	giveitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, EventScript_CardClub_NoRoom
	msgbox CardClub_Text_ChooseDeck, MSGBOX_DEFAULT
	closemessage
	special Tcg_ClubChooseDeck
	msgbox CardClub_Text_Lesson, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_CardClub_NoLesson
	msgbox CardClub_Text_LessonGo, MSGBOX_DEFAULT
	closemessage
	special Tcg_ClubTeachingDuel
	msgbox CardClub_Text_AfterLesson, MSGBOX_DEFAULT
	msgbox CardClub_Text_HowTo, MSGBOX_DEFAULT
	release
	end

EventScript_CardClub_NoLesson::
	msgbox CardClub_Text_HowTo, MSGBOX_DEFAULT
	release
	end

EventScript_CardClub_NoRoom::
	msgbox CardClub_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

EventScript_CardClub_Counter::
	special Tcg_ClubGiftReady
	goto_if_eq VAR_RESULT, TRUE, EventScript_CardClub_Gift
	msgbox CardClub_Text_Welcome, MSGBOX_DEFAULT
	closemessage
	special Tcg_ClubCounter
	msgbox CardClub_Text_Bye, MSGBOX_DEFAULT
	release
	end

EventScript_CardClub_Gift::
	msgbox CardClub_Text_GiftIntro, MSGBOX_DEFAULT
	special Tcg_ClubGiveGift
	goto_if_eq VAR_RESULT, 0, EventScript_CardClub_GiftNoRoom
	playfanfare MUS_OBTAIN_ITEM
	message CardClub_Text_GiftGot
	waitmessage
	waitfanfare
	goto_if_eq VAR_RESULT, 1, EventScript_CardClub_GiftDone
	msgbox CardClub_Text_GiftPc, MSGBOX_DEFAULT
EventScript_CardClub_GiftDone::
	msgbox CardClub_Text_GiftAfter, MSGBOX_DEFAULT
	release
	end

EventScript_CardClub_GiftNoRoom::
	msgbox CardClub_Text_GiftNoRoom, MSGBOX_DEFAULT
	release
	end

CardClub_Text_Intro:
	.string "Hi there! I'm with the CARD CLUB.\p"
	.string "We play the MOéMON TRADING CARD GAME\n"
	.string "all over HOENN.\p"
	.string "You look like a TRAINER with an eye\n"
	.string "for strategy. Here, this is yours!$"

CardClub_Text_ChooseDeck:
	.string "Every player needs a deck.\n"
	.string "I've got three ready to go.\p"
	.string "Pick the one that calls to you!$"

CardClub_Text_Lesson:
	.string "Great choice!\p"
	.string "Want me to show you how a duel goes?\n"
	.string "It'll only take a few minutes.$"

CardClub_Text_LessonGo:
	.string "All right! I'll lend you a deck.\n"
	.string "Let's play!$"

CardClub_Text_AfterLesson:
	.string "Nicely done! Now you know the basics.$"

CardClub_Text_HowTo:
	.string "REGISTER the CARD CASE to a button\n"
	.string "in your BAG.\p"
	.string "Press that button in front of\n"
	.string "someone to ask them for a card duel.\p"
	.string "Most people in HOENN play cards!\n"
	.string "TRAINERS will play after a MOéMON\l"
	.string "battle, and GYM LEADERS once you\l"
	.string "hold their BADGE.\p"
	.string "Win, and you get a booster pack and\n"
	.string "the pick of one card from their deck.\p"
	.string "Bring your spare cards to any CARD\n"
	.string "CLUB counter. Good luck!$"

CardClub_Text_NoRoom:
	.string "Oh, your BAG is full...\n"
	.string "Come back when you have room!$"

CardClub_Text_Welcome:
	.string "Welcome to the CARD CLUB counter!$"

CardClub_Text_Bye:
	.string "Come back any time!$"

CardClub_Text_GiftIntro:
	.string "Wh-what?! You've collected every\n"
	.string "single card?!\p"
	.string "In all my years at the CARD CLUB...\n"
	.string "This calls for something special!\p"
	.string "This PIKACHU loves cards as much as\n"
	.string "you do. It can even FLY and SURF!$"

CardClub_Text_GiftGot:
	.string "{PLAYER} received PIKACHU!$"

CardClub_Text_GiftPc:
	.string "PIKACHU was sent to the PC.$"

CardClub_Text_GiftAfter:
	.string "Your CARD CASE shines gold now.\n"
	.string "You're a true CARD MASTER!$"

CardClub_Text_GiftNoRoom:
	.string "You don't have room for a MOéMON...\n"
	.string "Make some room and come back!$"
