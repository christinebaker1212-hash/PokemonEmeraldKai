Johto_Route34_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route34_EventScript_83EFC2C
	.byte 0

@ Johto_Route34 trigger at 25,5 (VAR_QZ_0x500A == 0)
@ Johto_Route34 trigger at 26,5 (VAR_QZ_0x500A == 0)
@ Johto_Route34 trigger at 27,5 (VAR_QZ_0x500A == 0)
@ Johto_Route34 trigger at 28,5 (VAR_QZ_0x500A == 0)
Johto_Route34_EventScript_83D23DB::
	setflag FLAG_QZ_0x11BE
	setvar VAR_QZ_0x500A, 1
	release
	end

Johto_Route34_EventScript_83EFC2C::
	special MP_JohtoDaycare_Flags   @ (Crystal's Route34EggCheckCallback: the DAY-CARE MAN out here with an EGG)
	end

@ Johto_Route34 object 1 (local id 1, gfx 0x23) at 34,59
Johto_Route34_EventScript_83EFC35::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4177, 0, Johto_Route34_Text_83EFD4F, Johto_Route34_Text_83EFD6C
	msgbox Johto_Route34_Text_83EFD8A, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 4 (local id 4, gfx 0x23) at 37,38
Johto_Route34_EventScript_83EFC4C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4925, 0, Johto_Route34_Text_83EFDD8, Johto_Route34_Text_83EFDFD
	msgbox Johto_Route34_Text_83EFE1C, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 3 (local id 3, gfx 0x1F) at 30,48
Johto_Route34_EventScript_83EFC63::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4116, 0, Johto_Route34_Text_83EFE4E, Johto_Route34_Text_83EFE89
	msgbox Johto_Route34_Text_83EFEA5, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 11 (local id 11, gfx 0x20) at 32,15
Johto_Route34_EventScript_83EFC7A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4335, 0, Johto_Route34_Text_83EFEE3, Johto_Route34_Text_83EFF14
	msgbox Johto_Route34_Text_83EFF2E, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 10 (local id 10, gfx 0x3EA9) at 20,26
Johto_Route34_EventScript_83EFC91::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4573, 0, Johto_Route34_Text_83EFF5A, Johto_Route34_Text_83EFF7E
	msgbox Johto_Route34_Text_83EFF99, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 2 (local id 2, gfx 0x3E84) at 41,53
Johto_Route34_EventScript_83EFCA8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4907, 0, Johto_Route34_Text_83EFFFE, Johto_Route34_Text_83F004C
	msgbox Johto_Route34_Text_83F0066, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 6 (local id 6, gfx 0x3E8F) at 25,84
Johto_Route34_EventScript_83EFCBF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4572, 0, Johto_Route34_Text_83F016E, Johto_Route34_Text_83F0183
	msgbox Johto_Route34_Text_83F01D0, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 5 (local id 5, gfx 0x3E8F) at 20,84
Johto_Route34_EventScript_83EFCD6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4292, 0, Johto_Route34_Text_83F020F, Johto_Route34_Text_83F0236
	msgbox Johto_Route34_Text_83F0287, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 object 7 (local id 7, gfx 0x3E8F) at 22,87
Johto_Route34_EventScript_83EFCED::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4849, 0, Johto_Route34_Text_83F02B0, Johto_Route34_Text_83F02E9
	@ Moemon Kai: Crystal's KATE gives the SOFT SAND after her battle (it was an item ball: docs/REGIONS_AUDIT.md 99)
	goto_if_set FLAG_JOHTO_GOT_SOFT_SAND, Johto_Route34_EventScript_KateAfter
	msgbox Johto_Route34_Text_KateOffer, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_SOFT_SAND
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_JOHTO_GOT_SOFT_SAND
Johto_Route34_EventScript_KateAfter::
	msgbox Johto_Route34_Text_83F0350, MSGBOX_AUTOCLOSE
	end

@ Johto_Route34 sign / bg event at 33,65
Johto_Route34_EventScript_83EFD04::
	lock
	msgbox Johto_Route34_Text_83F00C6, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route34 sign / bg event at 27,28
Johto_Route34_EventScript_83EFD10::
	lock
	msgbox Johto_Route34_Text_83F03AF, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route34 sign / bg event at 29,8
Johto_Route34_EventScript_83EFD1C::
	lock
	msgbox Johto_Route34_Text_83F010A, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route34 object 8 (local id 8, gfx 0x3A9E) at 29,84
Johto_Route34_EventScript_83EFD28::
	setorcopyvar VAR_0x8000, ITEM_SOFT_SAND
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route34 object 9 (local id 9, gfx 0x3A9E) at 26,53
Johto_Route34_EventScript_83EFD35::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route34 object 13 (local id 13, gfx 0x3A9E) at 33,21
Johto_Route34_EventScript_83EFD42::
	setorcopyvar VAR_0x8000, ITEM_SUPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route34_Text_83EFD4F::
	.string "Hey! I’m looking for MANKEY!$"

Johto_Route34_Text_83EFD6C::
	.string "Beaten by a passing stranger!$"

Johto_Route34_Text_83EFD8A::
	.string "I’m going to train even harder.\p"
	.string "After all, I’m trying to become\n"
	.string "a GYM LEADER.$"

Johto_Route34_Text_83EFDD8::
	.string "I’m the best in my class at\n"
	.string "MOéMON.$"

Johto_Route34_Text_83EFDFD::
	.string "No! There are better trainers…$"

Johto_Route34_Text_83EFE1C::
	.string "I’m trying hard so I can be the\n"
	.string "star in my class.$"

Johto_Route34_Text_83EFE4E::
	.string "I’m confident in my ability to\n"
	.string "raise MOéMON.\p"
	.string "Want to see?$"

Johto_Route34_Text_83EFE89::
	.string "Did I screw up my training?$"

Johto_Route34_Text_83EFEA5::
	.string "Maybe I should take one to DAY-CARE,\n"
	.string "or maybe use some items…$"

Johto_Route34_Text_83EFEE3::
	.string "Are you a TRAINER?\p"
	.string "Let’s have a practice battle.$"

Johto_Route34_Text_83EFF14::
	.string "Oh, no! I just can’t win…$"

Johto_Route34_Text_83EFF2E::
	.string "You’re too strong to be a\n"
	.string "practice partner.$"

Johto_Route34_Text_83EFF5A::
	.string "Who goes there?\n"
	.string "What are you up to?$"

Johto_Route34_Text_83EFF7E::
	.string "You’re a tough little kid.$"

Johto_Route34_Text_83EFF99::
	.string "Yep, nothing wrong today.\p"
	.string "Be good and stay out of trouble.$"

Johto_Route34_Text_83EFFFE::
	.string "I just got my MOéMON back from\n"
	.string "DAY-CARE.\p"
	.string "Let’s see how much stronger it got!$"

Johto_Route34_Text_83F004C::
	.string "Why does it end this way?$"

Johto_Route34_Text_83F0066::
	.string "My MOéMON came back with new moves.\p"
	.string "That confounded me to no end!$"

Johto_Route34_Text_83F00C6::
	.string "ROUTE 34\n"
	.string "GOLDENROD CITY - AZALEA TOWN\p"
	.string "ILEX FOREST SOMEWHERE BETWEEN$"

Johto_Route34_Text_83F010A::
	.string "TRAINER TIPS\p"
	.string "BERRY trees grow new BERRIES every\n"
	.string "day.\p"
	.string "Make a note of which trees bear\n"
	.string "which BERRIES.$"

Johto_Route34_Text_83F016E::
	.string "IRENE: Who are you?!$"

Johto_Route34_Text_83F0183::
	.string "IRENE: Ohhh! You’re too strong!$"

Johto_Route34_Text_83F01D0::
	.string "IRENE: Isn’t this beach great?\p"
	.string "It’s our secret little getaway!$"

Johto_Route34_Text_83F020F::
	.string "JENN: What the...?\n"
	.string "What are you doing?$"

Johto_Route34_Text_83F0236::
	.string "JENN: So sorry, IRENE! Dear Sis!$"

Johto_Route34_Text_83F0287::
	.string "JENN: Sunlight makes your body\n"
	.string "stronger.$"

Johto_Route34_Text_83F02B0::
	.string "KATE: You sure were mean to my\n"
	.string "little sisters!$"

Johto_Route34_Text_83F02E9::
	.string "KATE: No! I can’t believe I lost.$"

Johto_Route34_Text_83F0350::
	.string "KATE: I’m sorry we jumped you.\p"
	.string "We never expected anyone to find\n"
	.string "us here. You sure startled us.$"

Johto_Route34_Text_83F03AF::
	.string "DAY-CARE\p"
	.string "LET US RAISE YOUR MOéMON FOR YOU!$"

Johto_Route34_Text_KateOffer::
	.string "KATE: You’re too strong. I didn’t\n"
	.string "stand a chance.\p"
	.string "Here. You deserve this.$"

@ Moemon Kai: Crystal's DAY-CARE MAN outside (pokecrystal maps/Route34.asm DayCareManScript_Outside, engine/events/
@ daycare.asm DayCareManOutside; audit case 47), here when the game's DAY CARE has an EGG
Johto_Route34_EventScript_DCManOutside::
	lock
	faceplayer
	specialvar VAR_RESULT, GetDaycareState
	goto_if_ne VAR_RESULT, DAYCARE_EGG_WAITING, Johto_Route34_EventScript_DCNotYet
	msgbox Johto_Route34_Text_DCFoundAnEggText, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_Route34_EventScript_DCDeclined
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, Johto_Route34_EventScript_DCNoRoom
	special GiveEggFromDaycare
	clearflag FLAG_PENDING_DAYCARE_EGG
	textcolor 3
	message Johto_Route34_Text_DCReceivedEggText
	waitmessage
	playfanfare MUS_LEVEL_UP
	waitfanfare
	waitbuttonpress
	textcolor 0
	msgbox Johto_Route34_Text_DCTakeGoodCareOfEggText, MSGBOX_DEFAULT
	goto Johto_Route34_EventScript_DCWalkInside

Johto_Route34_EventScript_DCDeclined::
	msgbox Johto_Route34_Text_DCIllKeepItThanksText, MSGBOX_DEFAULT
	clearflag FLAG_PENDING_DAYCARE_EGG
	special RejectEggFromDayCare
Johto_Route34_EventScript_DCWalkInside::
	closemessage
	setflag FLAG_JOHTO_HIDE_DAYCARE_MAN_OUT
	goto_if_eq VAR_FACING, DIR_WEST, Johto_Route34_EventScript_DCWalkAroundPlayer
	applymovement 12, Johto_Route34_EventScript_DCMovement_WalkInside
	waitmovement 0
	goto Johto_Route34_EventScript_DCGoneInside

Johto_Route34_EventScript_DCWalkAroundPlayer::
	applymovement 12, Johto_Route34_EventScript_DCMovement_WalkInsideAround
	waitmovement 0
Johto_Route34_EventScript_DCGoneInside::
	playse SE_DOOR
	removeobject 12
	release
	end

Johto_Route34_EventScript_DCNotYet::
	msgbox Johto_Route34_Text_DCNotYetText, MSGBOX_DEFAULT
	release
	end

Johto_Route34_EventScript_DCNoRoom::
	msgbox Johto_Route34_Text_DCNoRoomForEggText, MSGBOX_DEFAULT
	release
	end

Johto_Route34_EventScript_DCMovement_WalkInside:
	walk_right
	walk_up
	step_end

Johto_Route34_EventScript_DCMovement_WalkInsideAround:
	walk_up
	walk_right
	step_end

Johto_Route34_Text_DCNotYetText::
	.string "Not yet…$"

Johto_Route34_Text_DCFoundAnEggText::
	.string "Ah, it’s you!\p"
	.string "We were raising your MOéMON, and\n"
	.string "my goodness, were we surprised!\p"
	.string "Your MOéMON had an EGG!\p"
	.string "We don’t know how it got there,\n"
	.string "but your MOéMON had it. You want\l"
	.string "it?$"

Johto_Route34_Text_DCReceivedEggText::
	.string "{PLAYER} received the EGG!$"

Johto_Route34_Text_DCTakeGoodCareOfEggText::
	.string "Take good care of it.$"

Johto_Route34_Text_DCIllKeepItThanksText::
	.string "Well then, I’ll keep it. Thanks!$"

Johto_Route34_Text_DCNoRoomForEggText::
	.string "You have no room in your party.\n"
	.string "Come back later.$"

@ Crystal's DayCareMon1Script / DayCareMon2Script: the POKéMON in the yard (mp_daycareyard.c draws them)
Johto_Route34_EventScript_DCMon1::
	lock
	setvar VAR_0x8009, 1
	specialvar VAR_0x8005, MP_JohtoDaycare_YardMon
	message Johto_Route34_Text_DCLeftWithDayCareManText
	goto Johto_Route34_EventScript_DCMonCry

Johto_Route34_EventScript_DCMon2::
	lock
	setvar VAR_0x8009, 0
	specialvar VAR_0x8005, MP_JohtoDaycare_YardMon
	message Johto_Route34_Text_DCLeftWithDayCareLadyText
Johto_Route34_EventScript_DCMonCry::
	waitmessage
	playmoncry VAR_0x8005, CRY_MODE_NORMAL
	waitbuttonpress
	waitmoncry
	specialvar VAR_RESULT, MP_JohtoDaycare_YardOther
	goto_if_eq VAR_RESULT, 1, Johto_Route34_EventScript_DCNoInterest
	goto_if_eq VAR_RESULT, 2, Johto_Route34_EventScript_DCShowsInterest
	goto_if_eq VAR_RESULT, 3, Johto_Route34_EventScript_DCFriendly
	goto_if_eq VAR_RESULT, 4, Johto_Route34_EventScript_DCCaresFor
	closemessage
	release
	end

Johto_Route34_EventScript_DCNoInterest::
	msgbox Johto_Route34_Text_DCBreedNoInterestText, MSGBOX_DEFAULT
	release
	end

Johto_Route34_EventScript_DCShowsInterest::
	msgbox Johto_Route34_Text_DCBreedShowsInterestText, MSGBOX_DEFAULT
	release
	end

Johto_Route34_EventScript_DCFriendly::
	msgbox Johto_Route34_Text_DCBreedFriendlyText, MSGBOX_DEFAULT
	release
	end

Johto_Route34_EventScript_DCCaresFor::
	msgbox Johto_Route34_Text_DCBreedAppearsToCareForText, MSGBOX_DEFAULT
	release
	end

Johto_Route34_Text_DCLeftWithDayCareManText::
	.string "It’s {STR_VAR_1} that was left\n"
	.string "with the DAY-CARE MAN.$"

Johto_Route34_Text_DCLeftWithDayCareLadyText::
	.string "It’s {STR_VAR_1} that was left\n"
	.string "with the DAY-CARE LADY.$"

Johto_Route34_Text_DCBreedNoInterestText::
	.string "It has no interest in {STR_VAR_1}.$"

Johto_Route34_Text_DCBreedShowsInterestText::
	.string "It shows interest in {STR_VAR_1}.$"

Johto_Route34_Text_DCBreedFriendlyText::
	.string "It’s friendly with {STR_VAR_1}.$"

Johto_Route34_Text_DCBreedAppearsToCareForText::
	.string "It appears to care for\n"
	.string "{STR_VAR_1}.$"
