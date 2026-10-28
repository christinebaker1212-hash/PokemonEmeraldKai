@ Moemon Kai: the CARD CLUB's Yu-Gi-Oh! duelist (OLDALE and MAUVILLE POKéMON MARTS, LILYCOVE DEPARTMENT STORE),
@ beside the CARD CLUB member. Hands over the CARD CASE if there isn't one yet and a Starter Deck (src/ygo/ygo_ui.c),
@ offers a teaching duel, then runs the counter: TRADE IN and the CARD SHOP (src/ygo/ygo_field.c).
EventScript_YgoClub::
	lock
	faceplayer
	special Ygo_ClubState
	goto_if_eq VAR_RESULT, TRUE, EventScript_YgoClub_Counter
	msgbox YgoClub_Text_Intro, MSGBOX_DEFAULT
	special Ygo_ClubHasCase
	goto_if_eq VAR_RESULT, TRUE, EventScript_YgoClub_HasCase
	msgbox YgoClub_Text_Case, MSGBOX_DEFAULT
	giveitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, EventScript_YgoClub_NoRoom
EventScript_YgoClub_HasCase::
	msgbox YgoClub_Text_ChooseDeck, MSGBOX_DEFAULT
	closemessage
	special Ygo_ClubChooseDeck
	msgbox YgoClub_Text_Lesson, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_YgoClub_NoLesson
	msgbox YgoClub_Text_LessonGo, MSGBOX_DEFAULT
	closemessage
	special Ygo_ClubTeachingDuel
	msgbox YgoClub_Text_AfterLesson, MSGBOX_DEFAULT
	msgbox YgoClub_Text_HowTo, MSGBOX_DEFAULT
	release
	end

EventScript_YgoClub_NoLesson::
	msgbox YgoClub_Text_HowTo, MSGBOX_DEFAULT
	release
	end

EventScript_YgoClub_NoRoom::
	msgbox YgoClub_Text_NoRoom, MSGBOX_DEFAULT
	release
	end

EventScript_YgoClub_Counter::
	msgbox YgoClub_Text_Welcome, MSGBOX_DEFAULT
	closemessage
	special Ygo_ClubCounter
	msgbox YgoClub_Text_Bye, MSGBOX_DEFAULT
	release
	end

YgoClub_Text_Intro:
	.string "Hey! I run the CARD CLUB's\n"
	.string "Yu-Gi-Oh! table.\p"
	.string "Lots of duelists around HOENN\n"
	.string "would rather play Yu-Gi-Oh!\l"
	.string "than MOéMON cards.$"

YgoClub_Text_Case:
	.string "You'll need somewhere to keep\n"
	.string "your cards. Here, take this!$"

YgoClub_Text_ChooseDeck:
	.string "Every duelist needs a deck.\n"
	.string "I've got the four Starter Decks.\p"
	.string "Pick the one that speaks to you!$"

YgoClub_Text_Lesson:
	.string "Great choice!\p"
	.string "Want me to walk you through a duel?\n"
	.string "It won't take long.$"

YgoClub_Text_LessonGo:
	.string "All right! I'll lend you YUGI's deck.\n"
	.string "It's time to duel!$"

YgoClub_Text_AfterLesson:
	.string "Not bad at all! You know the basics now.$"

YgoClub_Text_HowTo:
	.string "Your CARD CASE holds both games.\n"
	.string "Switch sides with L and R.\p"
	.string "REGISTER the CARD CASE to a button\n"
	.string "and press it in front of someone to\l"
	.string "ask for a duel.\p"
	.string "PSYCHICS, HEX MANIACS, BLACK BELTS,\n"
	.string "COOLTRAINERS, TEAM grunts and lots of\l"
	.string "young folks play Yu-Gi-Oh!\p"
	.string "TRAINERS duel after a MOéMON battle.\n"
	.string "The ELITE FOUR duel once you've\l"
	.string "beaten them.\p"
	.string "Win, and you get a booster pack and\n"
	.string "the pick of one card from their deck.\p"
	.string "Bring your spare cards to me for\n"
	.string "CARD POINTS. Good luck, duelist!$"

YgoClub_Text_NoRoom:
	.string "Oh, your BAG is full...\n"
	.string "Come back when you have room!$"

YgoClub_Text_Welcome:
	.string "Welcome to the Yu-Gi-Oh! table!$"

YgoClub_Text_Bye:
	.string "Duel again soon!$"
