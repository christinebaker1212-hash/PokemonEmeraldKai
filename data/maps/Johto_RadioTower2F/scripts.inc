Johto_RadioTower2F_MapScripts::
	.byte 0

@ Johto_RadioTower2F object 3 (local id 3, gfx 0x3EB0) at 19,11
Johto_RadioTower2F_EventScript_83D7A56::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4655, 0, Johto_RadioTower2F_Text_83D7D34, Johto_RadioTower2F_Text_83D7D8A
	msgbox Johto_RadioTower2F_Text_83D7DA0, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F object 6 (local id 6, gfx 0x3EB0) at 13,7
Johto_RadioTower2F_EventScript_83D7A6D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4658, 0, Johto_RadioTower2F_Text_83D7DD2, Johto_RadioTower2F_Text_83D7E1C
	msgbox Johto_RadioTower2F_Text_83D7E35, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F object 4 (local id 4, gfx 0x3EB0) at 19,5
Johto_RadioTower2F_EventScript_83D7A84::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4659, 0, Johto_RadioTower2F_Text_83D7E69, Johto_RadioTower2F_Text_83D7E88
	msgbox Johto_RadioTower2F_Text_83D7E9A, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F object 5 (local id 5, gfx 0x3EAF) at 7,7
Johto_RadioTower2F_EventScript_83D7A9B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4390, 0, Johto_RadioTower2F_Text_83D7EFD, Johto_RadioTower2F_Text_83D7F57
	msgbox Johto_RadioTower2F_Text_83D7F67, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F sign / bg event at 30,4
Johto_RadioTower2F_EventScript_83D7AB2::
	lock
	msgbox Johto_RadioTower2F_Text_83D7FAF, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower2F sign / bg event at 15,4
Johto_RadioTower2F_EventScript_83D7ABE::
	lock
	msgbox Johto_RadioTower2F_Text_83D7FB8, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower2F object 2 (local id 2, gfx 0xB) at 14,8
Johto_RadioTower2F_EventScript_83D7ACA::
	lock
	faceplayer
	msgbox Johto_RadioTower2F_Text_83D7B9B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower2F object 8 (local id 8, gfx 0x3EA9) at 10,5
Johto_RadioTower2F_EventScript_83D7AD7::
	lock
	faceplayer
	msgbox Johto_RadioTower2F_Text_83D7BCA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower2F object 7 (local id 7, gfx 0x2D) at 28,14: BUENA
@ Moemon Kai: Crystal's (maps/RadioTower2F.asm Buena; audit case 141): the BLUE CARD, the day's password after 6 PM
@ once you've heard the show, one point a day (src/mp/mp_buena.c). Her phone number isn't offered (BUENA isn't on
@ this game's POKeGEAR phone).
Johto_RadioTower2F_EventScript_83D7AE4::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500A, 6, Johto_RadioTower2F_EventScript_83D7B1D
	goto_if_eq VAR_QZ_0x500A, 7, Johto_RadioTower2F_EventScript_83D7B1D
	goto_if_eq VAR_QZ_0x500A, 8, Johto_RadioTower2F_EventScript_83D7B1D
	goto_if_eq VAR_QZ_0x500A, 9, Johto_RadioTower2F_EventScript_83D7B1D
	goto_if_unset FLAG_JOHTO_MET_BUENA, Johto_RadioTower2F_EventScript_BuenaIntroduction
	goto_if_set FLAG_DAILY_JOHTO_BUENA_PLAYED, Johto_RadioTower2F_EventScript_BuenaPlayedAlready
	gettime
	goto_if_lt VAR_0x8000, 18, Johto_RadioTower2F_EventScript_BuenaTooEarly
	goto_if_unset FLAG_DAILY_JOHTO_BUENA_PASSWORD, Johto_RadioTower2F_EventScript_BuenaTuneIn
	checkitem ITEM_BLUE_CARD
	goto_if_eq VAR_RESULT, FALSE, Johto_RadioTower2F_EventScript_BuenaNoBlueCard
	specialvar VAR_RESULT, MP_Buena_CardFull
	goto_if_eq VAR_RESULT, TRUE, Johto_RadioTower2F_EventScript_BuenaCardIsFull
	playbgm MUS_JOHTO_RADIO_BUENAS_PASSWORD, FALSE
	msgbox Johto_RadioTower2F_Text_BuenaDoYouKnowPassword, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_RadioTower2F_EventScript_BuenaForgotPassword
	msgbox Johto_RadioTower2F_Text_BuenaJoinTheShow, MSGBOX_DEFAULT
	closemessage
	turnobject 7, DIR_EAST
	call_if_eq VAR_FACING, DIR_EAST, Johto_RadioTower2F_EventScript_BuenaWalkToMicrophone
	turnobject LOCALID_PLAYER, DIR_EAST
	msgbox Johto_RadioTower2F_Text_BuenaEveryoneSayPassword, MSGBOX_DEFAULT
	turnobject 7, DIR_SOUTH
	special MP_Buena_BufferChoices
	multichoice 0, 0, MULTI_BUENA_PASSWORD, TRUE
	special MP_Buena_CheckAnswer
	closemessage
	setflag FLAG_DAILY_JOHTO_BUENA_PLAYED
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower2F_EventScript_BuenaWrongAnswer
	msgbox Johto_RadioTower2F_Text_BuenaCorrectAnswer, MSGBOX_DEFAULT
	closemessage
	playse SE_SHOP
	waitse
	delay 20
	turnobject 7, DIR_EAST
	msgbox Johto_RadioTower2F_Text_BuenaThanksForComing, MSGBOX_DEFAULT
	closemessage
	fadedefaultbgm
	release
	end

Johto_RadioTower2F_EventScript_BuenaWalkToMicrophone::
	applymovement LOCALID_PLAYER, Johto_RadioTower2F_Movement_WalkToMicrophone
	waitmovement 0
	return

Johto_RadioTower2F_EventScript_BuenaIntroduction::
	msgbox Johto_RadioTower2F_Text_BuenaShowIntroduction, MSGBOX_DEFAULT
	setflag FLAG_JOHTO_MET_BUENA
	giveitem ITEM_BLUE_CARD
Johto_RadioTower2F_EventScript_BuenaTuneIn::
	msgbox Johto_RadioTower2F_Text_BuenaTuneInToMyShow, MSGBOX_DEFAULT
	closemessage
	turnobject 7, DIR_EAST
	release
	end

Johto_RadioTower2F_EventScript_BuenaForgotPassword::
	msgbox Johto_RadioTower2F_Text_BuenaComeBackAfterListening, MSGBOX_DEFAULT
	closemessage
	turnobject 7, DIR_EAST
	fadedefaultbgm
	release
	end

Johto_RadioTower2F_EventScript_BuenaPlayedAlready::
	msgbox Johto_RadioTower2F_Text_BuenaAlreadyPlayed, MSGBOX_DEFAULT
	closemessage
	turnobject 7, DIR_EAST
	release
	end

Johto_RadioTower2F_EventScript_BuenaWrongAnswer::
	msgbox Johto_RadioTower2F_Text_BuenaDidYouForget, MSGBOX_DEFAULT
	closemessage
	turnobject 7, DIR_EAST
	delay 20
	msgbox Johto_RadioTower2F_Text_BuenaThanksForComing, MSGBOX_DEFAULT
	closemessage
	fadedefaultbgm
	release
	end

Johto_RadioTower2F_EventScript_BuenaNoBlueCard::
	msgbox Johto_RadioTower2F_Text_BuenaNoBlueCard, MSGBOX_DEFAULT
	closemessage
	turnobject 7, DIR_EAST
	release
	end

Johto_RadioTower2F_EventScript_BuenaCardIsFull::
	msgbox Johto_RadioTower2F_Text_BuenaCardIsFull, MSGBOX_DEFAULT
	closemessage
	turnobject 7, DIR_EAST
	release
	end

Johto_RadioTower2F_EventScript_BuenaTooEarly::
	msgbox Johto_RadioTower2F_Text_BuenaTuneInAfterSix, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower2F_EventScript_83D7B1D::
	msgbox Johto_RadioTower2F_Text_BuenaPasswordIsHelp, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower2F object 9 (local id 9) at 26,14: the prize desk (Crystal's RadioTowerBuenaPrizeReceptionist,
@ engine/events/buena.asm BuenaPrize)
Johto_RadioTower2F_EventScript_BuenaReceptionist::
	lock
	faceplayer
	checkitem ITEM_BLUE_CARD
	goto_if_eq VAR_RESULT, FALSE, Johto_RadioTower2F_EventScript_ReceptionistNoCard
	msgbox Johto_RadioTower2F_Text_ReceptionistPointsForPrizes, MSGBOX_DEFAULT
	special MP_Buena_ShowPoints
Johto_RadioTower2F_EventScript_PrizeLoop::
	message Johto_RadioTower2F_Text_BuenaAskWhichPrize
	waitmessage
	special MP_Buena_PrizeMenu
	waitstate
	special MP_Buena_BufferPrize
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower2F_EventScript_PrizeDone
	msgbox Johto_RadioTower2F_Text_BuenaIsThatRight, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_RadioTower2F_EventScript_PrizeLoop
	specialvar VAR_RESULT, MP_Buena_CanAfford
	goto_if_eq VAR_RESULT, FALSE, Johto_RadioTower2F_EventScript_PrizeNotEnough
	checkitemspace VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Johto_RadioTower2F_EventScript_PrizeNoRoom
	additem VAR_0x8005
	special MP_Buena_Pay
	playse SE_SHOP
	msgbox Johto_RadioTower2F_Text_BuenaHereYouGo, MSGBOX_DEFAULT
	goto Johto_RadioTower2F_EventScript_PrizeLoop

Johto_RadioTower2F_EventScript_PrizeNotEnough::
	msgbox Johto_RadioTower2F_Text_BuenaNotEnoughPoints, MSGBOX_DEFAULT
	goto Johto_RadioTower2F_EventScript_PrizeLoop

Johto_RadioTower2F_EventScript_PrizeNoRoom::
	msgbox Johto_RadioTower2F_Text_BuenaNoRoom, MSGBOX_DEFAULT
	goto Johto_RadioTower2F_EventScript_PrizeLoop

Johto_RadioTower2F_EventScript_PrizeDone::
	special MP_Buena_HidePoints
	msgbox Johto_RadioTower2F_Text_BuenaComeAgain, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower2F_EventScript_ReceptionistNoCard::
	msgbox Johto_RadioTower2F_Text_ReceptionistNoCard, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower2F_Movement_WalkToMicrophone:
	walk_slow_down
	walk_slow_right
	step_end

@ Johto_RadioTower2F object 1 (local id 1, gfx 0x2F) at 29,9
Johto_RadioTower2F_EventScript_83D7B28::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500A, 6, Johto_RadioTower2F_EventScript_83D7B61
	goto_if_eq VAR_QZ_0x500A, 7, Johto_RadioTower2F_EventScript_83D7B61
	goto_if_eq VAR_QZ_0x500A, 8, Johto_RadioTower2F_EventScript_83D7B61
	goto_if_eq VAR_QZ_0x500A, 9, Johto_RadioTower2F_EventScript_83D7B61
	msgbox Johto_RadioTower2F_Text_83D7BEF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower2F_EventScript_83D7B61::
	msgbox Johto_RadioTower2F_Text_83D7C1E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower2F_Text_83D7B9B::
	.string "You can listen to the radio\n"
	.string "anywhere. Tune in!$"

Johto_RadioTower2F_Text_83D7BCA::
	.string "Restricted access beyond this\n"
	.string "point.$"

Johto_RadioTower2F_Text_83D7BEF::
	.string "Lullabies on the radio may make\n"
	.string "MOéMON sleep.$"

Johto_RadioTower2F_Text_83D7C1E::
	.string "Why would they want to take over\n"
	.string "the RADIO TOWER?$"

Johto_RadioTower2F_Text_83D7D34::
	.string "Three years ago, TEAM ROCKET was\n"
	.string "forced to disband.\p"
	.string "But we’re making a comeback here!$"

Johto_RadioTower2F_Text_83D7D8A::
	.string "Gwah! Don’t get cute!$"

Johto_RadioTower2F_Text_83D7DA0::
	.string "We won’t let you ruin our plans\n"
	.string "for our comeback!$"

Johto_RadioTower2F_Text_83D7DD2::
	.string "We’re TEAM ROCKET, the exploiters\n"
	.string "of MOéMON!\p"
	.string "We love being evil! Scared?$"

Johto_RadioTower2F_Text_83D7E1C::
	.string "You think you’re a hero?$"

Johto_RadioTower2F_Text_83D7E35::
	.string "We’re not always evil. We just do\n"
	.string "whatever we like.$"

Johto_RadioTower2F_Text_83D7E69::
	.string "Hey, hey! Keep out of our way!$"

Johto_RadioTower2F_Text_83D7E88::
	.string "Arggh. I give up.$"

Johto_RadioTower2F_Text_83D7E9A::
	.string "Our EXECUTIVES are trying to take\n"
	.string "this place over.\p"
	.string "They have some big plan. I wonder\n"
	.string "what that is?$"

Johto_RadioTower2F_Text_83D7EFD::
	.string "Hahaha!\p"
	.string "How boring. It was far too easy\n"
	.string "to take over this place!\p"
	.string "Come on, keep me amused!$"

Johto_RadioTower2F_Text_83D7F57::
	.string "Wh-who are you?$"

Johto_RadioTower2F_Text_83D7F67::
	.string "You beat me, and I won’t forget\n"
	.string "it!$"

Johto_RadioTower2F_Text_83D7FAF::
	.string "2F SALES$"

Johto_RadioTower2F_Text_83D7FB8::
	.string "PROF.OAK’S MOéMON TALK\p"
	.string "The Hottest Show on the Air!$"

Johto_RadioTower2F_Text_BuenaShowIntroduction::
	.string "BUENA: Hi! I’m BUENA!\p"
	.string "Do you know about a radio program\n"
	.string "called PASSWORD?\p"
	.string "If you can tell me the password\n"
	.string "from the program, you will earn\l"
	.string "points.\p"
	.string "Save up those points and trade\n"
	.string "them to that sweet young lady over\l"
	.string "there for some choice prizes!\p"
	.string "Here you go!\p"
	.string "It’s your very own point card!$"

Johto_RadioTower2F_Text_BuenaTuneInToMyShow::
	.string "BUENA: Tune in to my PASSWORD\n"
	.string "SHOW!$"

Johto_RadioTower2F_Text_BuenaDoYouKnowPassword::
	.string "BUENA: Hi! Did you tune in to my\n"
	.string "radio show?\p"
	.string "Do you remember today’s password?$"

Johto_RadioTower2F_Text_BuenaJoinTheShow::
	.string "BUENA: Oh, wow! Thank you!\p"
	.string "What was your name again?\p"
	.string "…{PLAYER}, OK!\p"
	.string "Come on, {PLAYER}. Join the show.$"

Johto_RadioTower2F_Text_BuenaEveryoneSayPassword::
	.string "BUENA: Everyone ready?\p"
	.string "I want to hear you shout out\n"
	.string "today’s password for {PLAYER}!$"

Johto_RadioTower2F_Text_BuenaComeBackAfterListening::
	.string "BUENA: Come back after you listen\n"
	.string "to my show, OK? Catch ya later!$"

Johto_RadioTower2F_Text_BuenaAlreadyPlayed::
	.string "BUENA: Sorry…\p"
	.string "You get just one chance each day.\p"
	.string "Come back tomorrow for another\n"
	.string "try!$"

Johto_RadioTower2F_Text_BuenaCorrectAnswer::
	.string "BUENA: YIPPEE! That’s right!\p"
	.string "You did tune in! I’m so happy!\p"
	.string "You earned one point! Congrats!$"

Johto_RadioTower2F_Text_BuenaDidYouForget::
	.string "BUENA: Aww… That’s not it…\p"
	.string "Did you forget the password?$"

Johto_RadioTower2F_Text_BuenaThanksForComing::
	.string "BUENA: Yup! Our contestant was\n"
	.string "{PLAYER}. Thanks for coming!\p"
	.string "I hope all you listeners will come\n"
	.string "too! I’ll be waiting!$"

Johto_RadioTower2F_Text_BuenaPasswordIsHelp::
	.string "BUENA: Huh? Today’s password?\p"
	.string "HELP, of course!$"

Johto_RadioTower2F_Text_BuenaCardIsFull::
	.string "BUENA: Your BLUE CARD’s full.\p"
	.string "Trade it in for a fabulous prize!$"

Johto_RadioTower2F_Text_BuenaTuneInAfterSix::
	.string "BUENA: Tune in to PASSWORD every\n"
	.string "night from six to midnight!\p"
	.string "Tune in, then drop in for a visit!$"

Johto_RadioTower2F_Text_BuenaNoBlueCard::
	.string "BUENA: Oh? You forgot to bring\n"
	.string "your BLUE CARD?\p"
	.string "I can’t give you points if you\n"
	.string "don’t have it.$"

Johto_RadioTower2F_Text_ReceptionistPointsForPrizes::
	.string "You can cash in your saved points\n"
	.string "for a lovely prize of your choice!$"

Johto_RadioTower2F_Text_ReceptionistNoCard::
	.string "You can’t trade in points without\n"
	.string "your BLUE CARD.\p"
	.string "Don’t forget your BLUE CARD!$"

Johto_RadioTower2F_Text_BuenaAskWhichPrize::
	.string "Which prize would you like?$"

Johto_RadioTower2F_Text_BuenaHereYouGo::
	.string "Here you go!$"

Johto_RadioTower2F_Text_BuenaNotEnoughPoints::
	.string "You don’t have enough points.$"

Johto_RadioTower2F_Text_BuenaNoRoom::
	.string "You have no room for it.$"

Johto_RadioTower2F_Text_BuenaComeAgain::
	.string "Oh. Please come back again!$"

@ (Crystal's _BuenaIsThatRightText starts with the prize's name: typed from it, the buffer as STR_VAR_1)
Johto_RadioTower2F_Text_BuenaIsThatRight::
	.string "{STR_VAR_1}?\n"
	.string "Is that right?$"
