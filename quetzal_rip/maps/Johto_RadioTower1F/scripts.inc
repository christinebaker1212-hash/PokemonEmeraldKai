Johto_RadioTower1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_RadioTower1F_EventScript_83D6C21
	.byte 0

Johto_RadioTower1F_EventScript_83D6C21::
	special QzClearRadioTowerFlag14A3_793
	end

@ Johto_RadioTower1F object 3 (local id 3, gfx 0x3EB0) at 22,5
Johto_RadioTower1F_EventScript_83D6C25::
	lock
	faceplayer
	msgbox Johto_RadioTower1F_Text_83D7327, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4837, 0, Johto_RadioTower1F_Text_83D73B6
	msgbox Johto_RadioTower1F_Text_83D73D2, MSGBOX_AUTOCLOSE
	turnobject 3, 4
	applymovement 3, Johto_RadioTower1F_Movement_83D6C5A
	waitmovement 0
	playse SE_EXIT
	removeobject 3
	setflag FLAG_QZ_0x10A5
	release
	end

@ Johto_RadioTower1F object 2 (local id 2, gfx 0x10) at 23,8
Johto_RadioTower1F_EventScript_83D6C5C::
	lock
	faceplayer
	msgbox Johto_RadioTower1F_Text_83D72A8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower1F object 1 (local id 1, gfx 0x3E87) at 22,8
Johto_RadioTower1F_EventScript_83D6C69::
	lock
	faceplayer
	msgbox Johto_RadioTower1F_Text_83D72DD, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower1F object 6 (local id 6, gfx 0x3EBC) at 13,8
Johto_RadioTower1F_EventScript_83D6C76::
	lock
	faceplayer
	msgbox Johto_RadioTower1F_Text_83D6C87, MSGBOX_DEFAULT
	closemessage
	turnobject 6, 1
	release
	end

@ Johto_RadioTower1F object 5 (local id 5, gfx 0x3E82) at 14,10
Johto_RadioTower1F_EventScript_83D6D6E::
	lock
	faceplayer
	msgbox Johto_RadioTower1F_Text_83D6EE1, MSGBOX_DEFAULT
	goto_if_set FLAG_QZ_0x149F, Johto_RadioTower1F_EventScript_83D6E4C
	goto_if_unset FLAG_QZ_0x10A3, Johto_RadioTower1F_EventScript_83D6D95
	msgbox Johto_RadioTower1F_Text_83D6EEA, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower1F_EventScript_83D6D95::
	msgbox Johto_RadioTower1F_Text_83D6F24, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_RadioTower1F_EventScript_83D6DB3
	msgbox Johto_RadioTower1F_Text_83D7276, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower1F_EventScript_83D6DB3::
	msgbox Johto_RadioTower1F_Text_83D6F8F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_RadioTower1F_EventScript_83D6DD1
	msgbox Johto_RadioTower1F_Text_83D723D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower1F_EventScript_83D6DD1::
	msgbox Johto_RadioTower1F_Text_83D6FE1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_RadioTower1F_EventScript_83D6DEF
	msgbox Johto_RadioTower1F_Text_83D723D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower1F_EventScript_83D6DEF::
	msgbox Johto_RadioTower1F_Text_83D7028, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower1F_EventScript_83D6E0D
	msgbox Johto_RadioTower1F_Text_83D723D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower1F_EventScript_83D6E0D::
	msgbox Johto_RadioTower1F_Text_83D7093, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_RadioTower1F_EventScript_83D6E2B
	msgbox Johto_RadioTower1F_Text_83D723D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower1F_EventScript_83D6E2B::
	msgbox Johto_RadioTower1F_Text_83D70E8, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower1F_EventScript_83D6E49
	msgbox Johto_RadioTower1F_Text_83D723D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower1F_EventScript_83D6E49::
	setflag FLAG_QZ_0x149F

Johto_RadioTower1F_EventScript_83D6E4C::
	msgbox Johto_RadioTower1F_Text_83D714F, MSGBOX_DEFAULT
	closemessage
	setorcopyvar VAR_0x8000, 0x1E3
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower1F_EventScript_83D6EC1
	clearflag FLAG_QZ_0x149F
	msgbox Johto_RadioTower1F_Text_83D71E0, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 6, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject 6, 4
	delay 0x14
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_RadioTower1F_Text_83D6D02, MSGBOX_DEFAULT
	closemessage
	applymovement 6, Johto_RadioTower1F_Movement_83D6ECC
	waitmovement 0
	playse SE_EXIT
	removeobject 6
	setflag FLAG_QZ_0x10A3
	setflag FLAG_QZ_0x10A2
	goto_if_ge VAR_QZ_0x500A, 2, Johto_RadioTower1F_EventScript_83D6EBF
	setvar VAR_QZ_0x500A, 2

Johto_RadioTower1F_EventScript_83D6EBF::
	release
	end

Johto_RadioTower1F_EventScript_83D6EC1::
	msgbox Johto_RadioTower1F_Text_83D717F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower1F sign / bg event at 20,4
@ Johto_RadioTower1F sign / bg event at 10,4
Johto_RadioTower1F_EventScript_83D6ED5::
	lock
	msgbox Johto_RadioTower1F_Text_83D7417, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower1F object 4 (local id 4, gfx 0x3E83) at 19,10
Johto_RadioTower1F_EventScript_83D745D::
	lock
	faceplayer
	dotimebasedevents
	goto_if_ne VAR_QZ_0x5001, 0, Johto_RadioTower1F_EventScript_83D75B0
	goto_if_set FLAG_QZ_0x149D, Johto_RadioTower1F_EventScript_83D7546
	msgbox Johto_RadioTower1F_Text_83D75E8, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower1F_EventScript_83D7550
	setflag FLAG_QZ_0x149D
	message Johto_RadioTower1F_Text_83D76E5
	waitmessage
	special RetrieveLotteryNumber
	copyvar VAR_0x8008, VAR_RESULT
	special BufferLottoTicketNumber
	msgbox Johto_RadioTower1F_Text_83D7728, MSGBOX_DEFAULT
	applymovement 4, Johto_RadioTower1F_Movement_8360833
	waitmovement 0
	playse SE_PC_ON
	special PickLotteryCornerTicket
	delay 0xDC
	delay 0xA
	applymovement 4, Johto_RadioTower1F_Movement_83607FF
	waitmovement 0
	goto_if_eq VAR_0x8004, 0, Johto_RadioTower1F_EventScript_83D755A
	call_if_eq VAR_0x8006, 0, Johto_RadioTower1F_EventScript_83D7534
	call_if_eq VAR_0x8006, 1, Johto_RadioTower1F_EventScript_83D753D
	bufferitemname 0, VAR_0x8005
	call_if_eq VAR_0x8004, 1, Johto_RadioTower1F_EventScript_83D7572
	call_if_eq VAR_0x8004, 2, Johto_RadioTower1F_EventScript_83D757B
	call_if_eq VAR_0x8004, 3, Johto_RadioTower1F_EventScript_83D7584
	call_if_eq VAR_0x8004, 4, Johto_RadioTower1F_EventScript_83D758D
	setorcopyvar VAR_0x8000, VAR_0x8005
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower1F_EventScript_83D7596
	special TryPutLotteryWinnerReportOnAir
	goto Johto_RadioTower1F_EventScript_83D7568

Johto_RadioTower1F_EventScript_83D7534::
	msgbox Johto_RadioTower1F_Text_83D77AF, MSGBOX_DEFAULT
	return

Johto_RadioTower1F_EventScript_83D753D::
	msgbox Johto_RadioTower1F_Text_83D77FC, MSGBOX_DEFAULT
	return

Johto_RadioTower1F_EventScript_83D7546::
	msgbox Johto_RadioTower1F_Text_83D76B3, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower1F_EventScript_83D7550::
	msgbox Johto_RadioTower1F_Text_83D76CE, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower1F_EventScript_83D755A::
	msgbox Johto_RadioTower1F_Text_83D7849, MSGBOX_DEFAULT
	goto Johto_RadioTower1F_EventScript_83D7568

Johto_RadioTower1F_EventScript_83D7568::
	msgbox Johto_RadioTower1F_Text_83D7A3E, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower1F_EventScript_83D7572::
	msgbox Johto_RadioTower1F_Text_83D7871, MSGBOX_DEFAULT
	return

Johto_RadioTower1F_EventScript_83D757B::
	msgbox Johto_RadioTower1F_Text_83D78B4, MSGBOX_DEFAULT
	return

Johto_RadioTower1F_EventScript_83D7584::
	msgbox Johto_RadioTower1F_Text_83D78FA, MSGBOX_DEFAULT
	return

Johto_RadioTower1F_EventScript_83D758D::
	msgbox Johto_RadioTower1F_Text_83D793E, MSGBOX_DEFAULT
	return

Johto_RadioTower1F_EventScript_83D7596::
	copyvar VAR_QZ_0x5002, VAR_0x8004
	copyvar VAR_QZ_0x5001, VAR_0x8005
	goto Johto_RadioTower1F_EventScript_83D75A6

Johto_RadioTower1F_EventScript_83D75A6::
	msgbox Johto_RadioTower1F_Text_83D7999, MSGBOX_DEFAULT
	release
	end

Johto_RadioTower1F_EventScript_83D75B0::
	msgbox Johto_RadioTower1F_Text_83D79EE, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, VAR_QZ_0x5001
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_RadioTower1F_EventScript_83D75A6
	copyvar VAR_0x8004, VAR_QZ_0x5002
	copyvar VAR_0x8005, VAR_QZ_0x5001
	special TryPutLotteryWinnerReportOnAir
	setvar VAR_QZ_0x5001, 0
	setvar VAR_QZ_0x5002, 0
	release
	end

@ Johto_RadioTower1F object 7 (local id 7, gfx 0x3EB0) at 22,5
Johto_RadioTower1F_EventScript_8401BD2::
	end

Johto_RadioTower1F_Movement_83607FF:
	face_player
	step_end

Johto_RadioTower1F_Movement_8360833:
	walk_in_place_faster_right
	step_end

Johto_RadioTower1F_Movement_83D6C5A:
	walk_in_place_right
	step_end

Johto_RadioTower1F_Movement_83D6ECC:
	face_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_in_place_down
	step_end

Johto_RadioTower1F_Text_83D6C87::
	.string "WHITNEY: Hi!\p"
	.string "My name is WHITNEY!\p"
	.string "I heard you can win a METRONOME\n"
	.string "in this QUIZ, so I came to try…\p"
	.string "But this QUIZ is so hard!$"

Johto_RadioTower1F_Text_83D6D02::
	.string "WHITNEY: Wow, you did it!\p"
	.string "I thought catches counted in\n"
	.string "every region! Oops!\p"
	.string "It’s time to go back to the GYM!$"

Johto_RadioTower1F_Text_83D6EE1::
	.string "Welcome!$"

Johto_RadioTower1F_Text_83D6EEA::
	.string "Hello. I’m sorry, but we’re not\n"
	.string "offering any tours today.$"

Johto_RadioTower1F_Text_83D6F24::
	.string "Try our special QUIZ!\p"
	.string "Answer five questions correctly to\n"
	.string "win a METRONOME.\p"
	.string "Would you like to take the QUIZ?$"

Johto_RadioTower1F_Text_83D6F8F::
	.string "Question 1:\p"
	.string "Does FALKNER, VIOLET’s GYM LEADER,\n"
	.string "specialize in FLYING-type POKéMON?$"

Johto_RadioTower1F_Text_83D6FE1::
	.string "Correct!\n"
	.string "Question 2:\p"
	.string "Can you buy BERRIES at the\n"
	.string "GOLDENROD flower shop?$"

Johto_RadioTower1F_Text_83D7028::
	.string "Bull’s-eye!\n"
	.string "Question 3:\p"
	.string "Does catching a POKéMON in HOENN\n"
	.string "automatically register it as\l"
	.string "caught in JOHTO too?$"

Johto_RadioTower1F_Text_83D7093::
	.string "So far so good!\n"
	.string "Question 4:\p"
	.string "Can you change a POKéMON’s nickname\n"
	.string "from the party menu?$"

Johto_RadioTower1F_Text_83D70E8::
	.string "Wow! Right again!\n"
	.string "Here’s the final question:\p"
	.string "Do you need to give KURT apricorns\n"
	.string "to buy his POKé BALLS?$"

Johto_RadioTower1F_Text_83D714F::
	.string "You passed the QUIZ!\p"
	.string "Your prize is a METRONOME!$"

Johto_RadioTower1F_Text_83D717F::
	.string "There is no room in your BAG.\p"
	.string "Make room and talk to me again.\n"
	.string "You won’t have to repeat the QUIZ.$"

Johto_RadioTower1F_Text_83D71E0::
	.string "A POKéMON holding a METRONOME hits\n"
	.string "harder when it uses the same move\l"
	.string "several times in a row.$"

Johto_RadioTower1F_Text_83D723D::
	.string "Oh, dear.\n"
	.string "Sorry, but you got it wrong.\p"
	.string "Please try again!$"

Johto_RadioTower1F_Text_83D7276::
	.string "Oh. I see. Please see me if you\n"
	.string "change your mind.$"

Johto_RadioTower1F_Text_83D72A8::
	.string "BEN is a fabulous DJ.\p"
	.string "His sweet voice makes me melt!$"

Johto_RadioTower1F_Text_83D72DD::
	.string "I love MARY, from POKéMON TALK.\p"
	.string "I only know what she sounds like,\n"
	.string "though.$"

Johto_RadioTower1F_Text_83D7327::
	.string "We’ve finally taken over the\n"
	.string "RADIO TOWER!\p"
	.string "Now everyone will get to experience\n"
	.string "the true terror of TEAM ROCKET!\p"
	.string "We’ll show you how scary we are!$"

Johto_RadioTower1F_Text_83D73B6::
	.string "Too strong! I’m sorry boss…$"

Johto_RadioTower1F_Text_83D73D2::
	.string "You’re too strong.\p"
	.string "Our plan could be ruined. I must\n"
	.string "warn the others…$"

Johto_RadioTower1F_Text_83D7417::
	.string "1F RECEPTION\n"
	.string "2F SALES\p"
	.string "3F PERSONNEL\n"
	.string "4F PRODUCTION\p"
	.string "5F DIRECTOR’S OFFICE$"

Johto_RadioTower1F_Text_83D75E8::
	.string "Welcome to the POKéMON LOTTERY!\p"
	.string "We compare the last digits of your\n"
	.string "TRAINER ID with the ticket number.\p"
	.string "Match at least two digits to win.\n"
	.string "More matches mean a better prize.\p"
	.string "Would you like to draw a ticket?$"

Johto_RadioTower1F_Text_83D76B3::
	.string "Please come back tomorrow.$"

Johto_RadioTower1F_Text_83D76CE::
	.string "Please do visit again.$"

Johto_RadioTower1F_Text_83D76E5::
	.string "Please pick a LOTO TICKET.\n"
	.string "…{PAUSE 0x0F}{PAUSE 0x0F}{PAUSE 0x0F}{PAUSE 0x0F}…{PAUSE 0x0F}{PAUSE 0x0F}{PAUSE 0x0F}{PAUSE 0x0F}…{PAUSE 0x0F}{PAUSE 0x0F}{PAUSE 0x0F}{PAUSE 0x0F}$"

Johto_RadioTower1F_Text_83D7728::
	.string "The LOTO TICKET number is {STR_VAR_1}.\p"
	.string "I need to run a check on this number\n"
	.string "to see if it matches any of your\l"
	.string "POKéMON’s ID numbers. Please wait.$"

Johto_RadioTower1F_Text_83D77AF::
	.string "Congratulations!\p"
	.string "The last digits of your TRAINER ID\n"
	.string "match the ticket number!$"

Johto_RadioTower1F_Text_83D77FC::
	.string "Congratulations!\p"
	.string "The last digits of your TRAINER ID\n"
	.string "match the ticket number!$"

Johto_RadioTower1F_Text_83D7849::
	.string "I’m sorry.\n"
	.string "None of the numbers matched.$"

Johto_RadioTower1F_Text_83D7871::
	.string "Two digits matched, so you win the\n"
	.string "third prize!\l"
	.string "You’ve won the {STR_VAR_1}!$"

Johto_RadioTower1F_Text_83D78B4::
	.string "Three digits matched, so you win the\n"
	.string "second prize!\l"
	.string "You’ve won the {STR_VAR_1}!$"

Johto_RadioTower1F_Text_83D78FA::
	.string "Four digits matched, so you win the\n"
	.string "first prize!\l"
	.string "You’ve won the {STR_VAR_1}!$"

Johto_RadioTower1F_Text_83D793E::
	.string "Oh, my goodness, all five digits\n"
	.string "matched!\p"
	.string "You’ve won the jackpot prize!\n"
	.string "You’ve won the {STR_VAR_1}!$"

Johto_RadioTower1F_Text_83D7999::
	.string "Oh?\n"
	.string "You seem to have no room for this.\p"
	.string "Please make room in your BAG and\n"
	.string "let me know.$"

Johto_RadioTower1F_Text_83D79EE::
	.string "{PLAYER}?\n"
	.string "Yes, I’ve been expecting you.\p"
	.string "This is the prize we’ve been holding\n"
	.string "for you.$"

Johto_RadioTower1F_Text_83D7A3E::
	.string "Please do visit again.$"
