@ Moemon Kai: SINNOH places of this game's own (words written for this game)

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
	msgbox Sinnoh_Text_BpNoRoom, MSGBOX_DEFAULT
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

Sinnoh_Text_BpNoRoom:
	.string "Your BAG has no room for it.$"

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

@ ETERNA's gym: the flower clock moves after a trainer or the leader is beaten (src/sinnoh/s3d_gyms.c Sinnoh_EternaClock)
Sinnoh_EventScript_EternaClock::
	lockall
	special Sinnoh_EternaClock
	waitstate
	releaseall
	end

@ VEILSTONE's gym: a punching bag hit (src/sinnoh/s3d_gyms.c Sinnoh_VeilstoneBag)
Sinnoh_EventScript_VeilstoneBag::
	lockall
	special Sinnoh_VeilstoneBag
	waitstate
	releaseall
	end

@ The DISTORTION WORLD (tools/sinnoh/story_people.py, gen_maps.py EXTRA_PEOPLE): after MARS and JUPITER fall at SPEAR
@ PILLAR, CYRUS is pulled through a tear in the sky; the player follows into GIRATINA's arena (Platinum's own 3D room),
@ battles CYRUS, then meets GIRATINA, and CYNTHIA leads them home. Flags: 0x3D34 CYRUS beaten there, 0x3D35 the
@ DISTORTION WORLD done, 0x3D36 its GIRATINA met (caught: 0x3D03, TURNBACK CAVE's GIRATINA too, as Platinum's).
Sinnoh_EventScript_DistortionEnter::
	lock
	faceplayer
	msgbox Sinnoh_Text_DistortionCyrus1, MSGBOX_DEFAULT
	closemessage
	waitse
	playmoncry SPECIES_GIRATINA, CRY_MODE_ENCOUNTER
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 16
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	waitmoncry
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox Sinnoh_Text_DistortionTear, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_DistortionNotYet
	closemessage
	playse SE_WARP_IN
	warp MAP_SINNOH_DISTORTION_WORLD_GIRATINA_ROOM, 15, 28
	waitstate
	release
	end

Sinnoh_EventScript_DistortionNotYet:
	msgbox Sinnoh_Text_DistortionNotYet, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_DistortionCyrus::
	lock
	faceplayer
	msgbox Sinnoh_Text_DistortionCyrus2, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_SINNOH_GALACTIC_BOSS_CYRUS_DISTORTION_WORLD, Sinnoh_Text_DistortionCyrusDefeat
	msgbox Sinnoh_Text_DistortionCyrus3, MSGBOX_DEFAULT
	closemessage
	setflag 0x3D34
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	release
	end

Sinnoh_EventScript_DistortionGiratina::
	lock
	goto_if_unset 0x3D34, Sinnoh_EventScript_DistortionGiratinaWaits
	faceplayer
	waitse
	playmoncry SPECIES_GIRATINA, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_GIRATINA, 47
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	setflag 0x3D36
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Sinnoh_EventScript_DistortionGiratinaCaught
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox Sinnoh_Text_DistortionGiratinaGone, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_DistortionGiratinaCaught:
	setflag 0x3D03
	removeobject VAR_LAST_TALKED
	release
	end

Sinnoh_EventScript_DistortionGiratinaWaits:
	msgbox Sinnoh_Text_DistortionGiratinaWaits, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_DistortionCynthia::
	lock
	faceplayer
	goto_if_set 0x3D36, Sinnoh_EventScript_DistortionCynthiaHome
	msgbox Sinnoh_Text_DistortionCynthia1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_DistortionCynthiaStay
	closemessage
	goto Sinnoh_EventScript_DistortionLeave

Sinnoh_EventScript_DistortionCynthiaStay:
	msgbox Sinnoh_Text_DistortionCynthiaStay, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_DistortionCynthiaHome:
	msgbox Sinnoh_Text_DistortionCynthia2, MSGBOX_DEFAULT
	closemessage
	setflag 0x3D35

Sinnoh_EventScript_DistortionLeave:
	playse SE_WARP_IN
	warp MAP_SINNOH_SPEAR_PILLAR, 31, 50
	waitstate
	release
	end

Sinnoh_Text_DistortionCyrus1:
	.string "CYRUS: The red chains are broken, and still\n"
	.string "the sky answers me. Do you hear it?\p"
	.string "Something has been watching from the other\n"
	.string "side of this world all along.$"

Sinnoh_Text_DistortionTear:
	.string "A shadow tore the sky open and dragged\n"
	.string "CYRUS through!\p"
	.string "The tear hangs in the air, twisting.\n"
	.string "Will you follow him?$"

Sinnoh_Text_DistortionNotYet:
	.string "The tear in the sky hums softly.\n"
	.string "It doesn't look like it will close.$"

Sinnoh_Text_DistortionCyrus2:
	.string "CYRUS: So this is the world on the other\n"
	.string "side. Up is down. Time does not move.\p"
	.string "There is no spirit here. Only stillness.\n"
	.string "It is everything I wanted. Stand aside!$"

Sinnoh_Text_DistortionCyrusDefeat:
	.string "Even here, your heart wins out...$"

Sinnoh_Text_DistortionCyrus3:
	.string "CYRUS: This world has a master, and it\n"
	.string "isn't me. I will find another way.\p"
	.string "Go. GIRATINA is waiting for you.$"

Sinnoh_Text_DistortionGiratinaWaits:
	.string "GIRATINA watches from the shadows,\n"
	.string "unmoving. CYRUS stands in the way.$"

Sinnoh_Text_DistortionGiratinaGone:
	.string "GIRATINA slipped back into the shadows\n"
	.string "of its world.$"

Sinnoh_Text_DistortionCynthia1:
	.string "CYNTHIA: I followed the tear after you.\n"
	.string "This is the world behind our world.\p"
	.string "We can't stay long. Shall we go back now?$"

Sinnoh_Text_DistortionCynthiaStay:
	.string "CYNTHIA: All right. I'll wait right here.\n"
	.string "Be careful.$"

Sinnoh_Text_DistortionCynthia2:
	.string "CYNTHIA: The world feels calmer. GIRATINA\n"
	.string "was keeping the balance, I think.\p"
	.string "Thank you. Let's go home together.$"

Sinnoh_EventScript_FactoryGuide::
	lock
	faceplayer
	msgbox Sinnoh_Text_FactoryGuide, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_FactoryNo
	closemessage
	playse SE_EXIT
	warp MAP_BATTLE_FRONTIER_BATTLE_FACTORY_LOBBY, 9, 10
	waitstate
	release
	end

Sinnoh_EventScript_FactoryNo:
	msgbox Sinnoh_Text_FactoryNo, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_FactoryGuide:
	.string "Welcome to the BATTLE FACTORY!\n"
	.string "Here you battle with rental MOéMON.\p"
	.string "Shall I show you to the rental halls?$"

Sinnoh_Text_FactoryNo:
	.string "Come back whenever you're ready to\n"
	.string "borrow a new team!$"

@ Platinum's Frontier Brains at the HALL, CASTLE and ARCADE (s3d_frontier.c Sinnoh_BrainText; VAR_0x8008 the facility:
@ 0 CASTLE, 1 ARCADE, 2 HALL): before the battle, its words when it's the brain's turn; after a win, its words beaten
@ and the print added. Platinum's own words.
Sinnoh_EventScript_BrainComes::
	setvar VAR_0x8004, 0
	special Sinnoh_BrainText
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_BrainNone
	message gStringVar4
	waitmessage
	setvar VAR_0x8004, 1
	special Sinnoh_BrainText
	message gStringVar4
	waitmessage
Sinnoh_EventScript_BrainNone:
	return

Sinnoh_EventScript_BrainBeaten::
	setvar VAR_0x8004, 2
	special Sinnoh_BrainText
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_BrainNone
	message gStringVar4
	waitmessage
	setvar VAR_0x8004, 3
	special Sinnoh_BrainText
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_BrainNone
	playfanfare MUS_OBTAIN_SYMBOL
	message gStringVar4
	waitmessage
	waitfanfare
	closemessage
	return

@ SINNOH's BATTLE HALL (src/sinnoh/s3d_hall.c): one POKéMON against the type the player picks, ten ranks a type
Sinnoh_EventScript_HallCounter::
	lock
	faceplayer
	special Sinnoh_HallInfo
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_HallWelcome
	msgbox Sinnoh_Text_HallRecord, MSGBOX_DEFAULT

Sinnoh_EventScript_HallWelcome:
	msgbox Sinnoh_Text_HallWelcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_HallBye
	msgbox Sinnoh_Text_HallChoose, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Sinnoh_EventScript_HallBye
	special Sinnoh_HallSetMon
	goto_if_eq VAR_RESULT, 1, Sinnoh_EventScript_HallTooLow
	goto_if_eq VAR_RESULT, 2, Sinnoh_EventScript_HallNotThat
	msgbox Sinnoh_Text_HallReady, MSGBOX_DEFAULT

Sinnoh_EventScript_HallPick:
	msgbox Sinnoh_Text_HallWhichType, MSGBOX_DEFAULT
	special Sinnoh_HallTypeMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Sinnoh_EventScript_HallStop
	msgbox Sinnoh_Text_HallGo, MSGBOX_DEFAULT
	setvar VAR_0x8008, 2
	call Sinnoh_EventScript_BrainComes
	closemessage
	special Sinnoh_HallBattle
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_HallLost
	call Sinnoh_EventScript_BrainBeaten
	special Sinnoh_HallWon
	msgbox Sinnoh_Text_HallWon, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_HallPick
	playfanfare MUS_OBTAIN_B_POINTS
	msgbox Sinnoh_Text_HallBp, MSGBOX_DEFAULT
	waitfanfare
	goto Sinnoh_EventScript_HallPick

Sinnoh_EventScript_HallStop:
	msgbox Sinnoh_Text_HallStopAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_HallPick
	special Sinnoh_HallEnd
	msgbox Sinnoh_Text_HallStopped, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_HallLost:
	special Sinnoh_HallEnd
	msgbox Sinnoh_Text_HallLost, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_HallTooLow:
	msgbox Sinnoh_Text_HallTooLow, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_HallNotThat:
	msgbox Sinnoh_Text_HallNotThat, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_HallBye:
	msgbox Sinnoh_Text_HallBye, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_HallRecord:
	.string "Your {STR_VAR_3} is on a streak of\n"
	.string "{STR_VAR_1}. Its best is {STR_VAR_2}.$"

Sinnoh_Text_HallWelcome:
	.string "Welcome to the BATTLE HALL! One\n"
	.string "MOéMON battles every type in turn.\p"
	.string "Each type has ten ranks. Win, and the\n"
	.string "next one of that type is tougher.\p"
	.string "Will you take the challenge?$"

Sinnoh_Text_HallChoose:
	.string "Which MOéMON will it be? It must be\n"
	.string "Lv. 30 or above.$"

Sinnoh_Text_HallReady:
	.string "{STR_VAR_1} it is! It will be healed\n"
	.string "after every battle.$"

Sinnoh_Text_HallWhichType:
	.string "Which type will you face next?$"

Sinnoh_Text_HallGo:
	.string "A {STR_VAR_1}-type at rank {STR_VAR_2}.\n"
	.string "Your opponent is ready!$"

Sinnoh_Text_HallWon:
	.string "A win! That's {STR_VAR_1} in a row.\n"
	.string "That type is now at rank {STR_VAR_2}.$"

Sinnoh_Text_HallBp:
	.string "Ten more wins! Here are {STR_VAR_3}\n"
	.string "BATTLE POINTS.$"

Sinnoh_Text_HallStopAsk:
	.string "End your challenge? Your streak and\n"
	.string "ranks will start over.$"

Sinnoh_Text_HallStopped:
	.string "Your challenge ends at {STR_VAR_1} wins.\n"
	.string "Well battled!$"

Sinnoh_Text_HallLost:
	.string "So close! Your streak ends at\n"
	.string "{STR_VAR_1} wins. Come back anytime.$"

Sinnoh_Text_HallTooLow:
	.string "That MOéMON needs to be Lv. 30 or\n"
	.string "above to enter.$"

Sinnoh_Text_HallNotThat:
	.string "I'm sorry, that one can't take part\n"
	.string "in the BATTLE HALL.$"

Sinnoh_Text_HallBye:
	.string "Come back whenever you're ready!$"

@ SINNOH's BATTLE CASTLE and BATTLE ARCADE (src/sinnoh/s3d_frontier.c): three POKéMON at Lv 50, sets of seven
@ battles. VAR_0x8008 the facility (0 CASTLE, 1 ARCADE) for the whole challenge.
Sinnoh_EventScript_CastleCounter::
	lock
	faceplayer
	setvar VAR_0x8008, 0
	msgbox Sinnoh_Text_CastleWelcome, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_FrontierCounter

Sinnoh_EventScript_ArcadeCounter::
	lock
	faceplayer
	setvar VAR_0x8008, 1
	msgbox Sinnoh_Text_ArcadeWelcome, MSGBOX_DEFAULT

Sinnoh_EventScript_FrontierCounter:
	copyvar VAR_0x8004, VAR_0x8008
	special Sinnoh_FrontierInfo
	msgbox Sinnoh_Text_FrontierRecord, MSGBOX_DEFAULT
	msgbox Sinnoh_Text_FrontierAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_FrontierBye

Sinnoh_EventScript_FrontierChoose:
	msgbox Sinnoh_Text_FrontierChoose, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Sinnoh_EventScript_FrontierBye
	special Sinnoh_FrontierAddMon
	goto_if_eq VAR_RESULT, 10, Sinnoh_EventScript_FrontierNotThat
	goto_if_eq VAR_RESULT, 11, Sinnoh_EventScript_FrontierAlready
	goto_if_lt VAR_RESULT, 3, Sinnoh_EventScript_FrontierChoose
	msgbox Sinnoh_Text_FrontierReady, MSGBOX_DEFAULT

Sinnoh_EventScript_FrontierNext:
	goto_if_eq VAR_0x8008, 1, Sinnoh_EventScript_ArcadeBoard
	msgbox Sinnoh_Text_CastleWhat, MSGBOX_DEFAULT
	special Sinnoh_CastleMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Sinnoh_EventScript_FrontierStop
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_FrontierFight
	copyvar VAR_0x8004, VAR_RESULT
	special Sinnoh_CastleBuy
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_CastleTooFew
	msgbox Sinnoh_Text_CastleBought, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_FrontierNext

Sinnoh_EventScript_CastleTooFew:
	msgbox Sinnoh_Text_CastleTooFew, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_FrontierNext

Sinnoh_EventScript_ArcadeBoard:
	msgbox Sinnoh_Text_ArcadeSpin, MSGBOX_DEFAULT
	playse SE_ROULETTE_BALL
	special Sinnoh_ArcadeSpin
	waitse
	msgbox Sinnoh_Text_ArcadeLanded, MSGBOX_DEFAULT

Sinnoh_EventScript_FrontierFight:
	msgbox Sinnoh_Text_FrontierGo, MSGBOX_DEFAULT
	call Sinnoh_EventScript_BrainComes
	closemessage
	special Sinnoh_FrontierBattle
	waitstate
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_FrontierLost
	call Sinnoh_EventScript_BrainBeaten
	special Sinnoh_FrontierWon
	goto_if_ne VAR_RESULT, 0, Sinnoh_EventScript_FrontierSetWon
	goto_if_eq VAR_0x8008, 1, Sinnoh_EventScript_FrontierWonPlain
	msgbox Sinnoh_Text_CastleWon, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_FrontierNext

Sinnoh_EventScript_FrontierWonPlain:
	msgbox Sinnoh_Text_FrontierWon, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_FrontierNext

Sinnoh_EventScript_FrontierSetWon:
	playfanfare MUS_OBTAIN_B_POINTS
	msgbox Sinnoh_Text_FrontierSetWon, MSGBOX_DEFAULT
	waitfanfare
	msgbox Sinnoh_Text_FrontierGoOn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_EventScript_FrontierNext

Sinnoh_EventScript_FrontierStop:
	msgbox Sinnoh_Text_FrontierStopAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_FrontierNext
	special Sinnoh_FrontierEnd
	msgbox Sinnoh_Text_FrontierStopped, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_FrontierLost:
	special Sinnoh_FrontierEnd
	msgbox Sinnoh_Text_FrontierLost, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_FrontierNotThat:
	msgbox Sinnoh_Text_FrontierNotThat, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_FrontierChoose

Sinnoh_EventScript_FrontierAlready:
	msgbox Sinnoh_Text_FrontierAlready, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_FrontierChoose

Sinnoh_EventScript_FrontierBye:
	msgbox Sinnoh_Text_FrontierBye, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_CastleWelcome:
	.string "Welcome to the BATTLE CASTLE.\p"
	.string "Here, HP isn't restored between battles.\n"
	.string "Win CASTLE POINTS, and spend them on\l"
	.string "rest or berries for your team.$"

Sinnoh_Text_ArcadeWelcome:
	.string "Welcome to the BATTLE ARCADE!\p"
	.string "Before every battle, our board spins and\n"
	.string "lands on an event. Good or bad, it's\l"
	.string "all part of the game!$"

Sinnoh_Text_FrontierRecord:
	.string "Your streak here is {STR_VAR_1}.\n"
	.string "Your best is {STR_VAR_2}.$"

Sinnoh_Text_FrontierAsk:
	.string "Bring three MOéMON. They battle at\n"
	.string "Lv. 50, in sets of seven. Ready?$"

Sinnoh_Text_FrontierChoose:
	.string "Choose a MOéMON for your team.$"

Sinnoh_Text_FrontierReady:
	.string "Your team is set. Good luck!$"

Sinnoh_Text_CastleWhat:
	.string "You have {STR_VAR_3} CP. What will you do?$"

Sinnoh_Text_CastleBought:
	.string "Done! You have {STR_VAR_3} CP left.$"

Sinnoh_Text_CastleTooFew:
	.string "You don't have enough CP for that.$"

Sinnoh_Text_ArcadeSpin:
	.string "Round and round the board goes...$"

Sinnoh_Text_ArcadeLanded:
	.string "It landed on {STR_VAR_1}!$"

Sinnoh_Text_FrontierGo:
	.string "Your opponent is waiting. Go!$"

Sinnoh_Text_FrontierWon:
	.string "That's battle {STR_VAR_1} won!$"

Sinnoh_Text_CastleWon:
	.string "That's battle {STR_VAR_1} won!\n"
	.string "You earned {STR_VAR_2} CP.$"

Sinnoh_Text_FrontierSetWon:
	.string "Seven wins in a row! Here are\n"
	.string "{STR_VAR_3} BATTLE POINTS. Your team is healed.$"

Sinnoh_Text_FrontierGoOn:
	.string "Will you keep going?$"

Sinnoh_Text_FrontierStopAsk:
	.string "End your challenge? Your streak will\n"
	.string "start over.$"

Sinnoh_Text_FrontierStopped:
	.string "Your challenge ends at {STR_VAR_1} wins.\n"
	.string "Thanks for playing!$"

Sinnoh_Text_FrontierLost:
	.string "Your streak ends at {STR_VAR_1} wins.\n"
	.string "Come back and try again!$"

Sinnoh_Text_FrontierNotThat:
	.string "That one can't enter, sorry.$"

Sinnoh_Text_FrontierAlready:
	.string "That one is already on your team.$"

Sinnoh_Text_FrontierBye:
	.string "Come back anytime!$"

@ A new game begun in SINNOH (mp_region.c MP_Region_StartNewGame): PROF. ROWAN's briefcase on ROUTE 201 holds
@ Platinum's three (Sinnoh_StarterMenu); one picked sets FLAG_SINNOH_STARTER (0x3D39). The briefcase is shown only for
@ that new game, until then (tools/sinnoh/story_people.py ROUTE_201_BRIEFCASE).
Sinnoh_EventScript_Briefcase::
	lock
	msgbox Sinnoh_Text_Briefcase, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_BriefcaseLeave
	msgbox Sinnoh_Text_BriefcaseWhich, MSGBOX_DEFAULT
	special Sinnoh_StarterMenu
	waitstate
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_BriefcaseLeave
	copyvar VAR_TEMP_2, VAR_RESULT
	givemon VAR_TEMP_2, 5
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, Sinnoh_EventScript_BriefcaseLeave
	setflag 0x3D39
	setflag FLAG_SYS_POKEMON_GET   @ (1.63.8: the START menu's MOéMON)
	bufferspeciesname STR_VAR_1, VAR_TEMP_2
	playfanfare MUS_OBTAIN_ITEM
	message Sinnoh_Text_BriefcaseGot
	waitmessage
	waitfanfare
	msgbox Sinnoh_Text_BriefcaseNick, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_BriefcaseDone
	setvar VAR_0x8004, 0
	call Common_EventScript_NameReceivedPartyMon

Sinnoh_EventScript_BriefcaseDone:
	msgbox Sinnoh_Text_BriefcaseAfter, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	release
	end

Sinnoh_EventScript_BriefcaseLeave:
	msgbox Sinnoh_Text_BriefcaseLeave, MSGBOX_DEFAULT
	release
	end

@ PROF. ROWAN in SANDGEM's lab: the POKéDEX and five POKé BALLS for a TRAINER who has none yet; else his words
Sinnoh_EventScript_RowanLab::
	lock
	faceplayer
	goto_if_set FLAG_SYS_POKEDEX_GET, Sinnoh_EventScript_RowanTalk
	getpartysize
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_RowanNoTeam
	msgbox Sinnoh_Text_RowanDex, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Sinnoh_Text_RowanGotDex
	waitmessage
	waitfanfare
	setflag FLAG_SYS_POKEDEX_GET
	special SetUnlockedPokedexFlags
	msgbox Sinnoh_Text_RowanBalls, MSGBOX_DEFAULT
	giveitem ITEM_POKE_BALL, 5
	msgbox Sinnoh_Text_RowanGo, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_RowanNoTeam:
	msgbox Sinnoh_Text_RowanNoTeam, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_RowanTalk:
	msgbox Sinnoh_Text_RowanTalk, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_Briefcase:
	.string "Someone left a briefcase here. A tag\n"
	.string "on it reads “PROF. ROWAN.”\p"
	.string "Three POKé BALLS are tucked inside.\n"
	.string "Will you take a look?$"

Sinnoh_Text_BriefcaseWhich:
	.string "Which MOéMON will you choose?$"

Sinnoh_Text_BriefcaseGot:
	.string "{PLAYER} chose {STR_VAR_1}!$"

Sinnoh_Text_BriefcaseNick:
	.string "Give a nickname to {STR_VAR_1}?$"

Sinnoh_Text_BriefcaseAfter:
	.string "Better bring the briefcase back to\n"
	.string "PROF. ROWAN's lab in SANDGEM TOWN.$"

Sinnoh_Text_BriefcaseLeave:
	.string "It's not yours... but the grass ahead\n"
	.string "rustles with wild MOéMON.$"

Sinnoh_Text_RowanDex:
	.string "PROF. ROWAN: So you found my briefcase,\n"
	.string "and a partner in it. Hm.\p"
	.string "I can see it trusts you already.\n"
	.string "Keep it. And take this as well.$"

Sinnoh_Text_RowanGotDex:
	.string "{PLAYER} received the MOéDEX!$"

Sinnoh_Text_RowanBalls:
	.string "The MOéDEX records every MOéMON you\n"
	.string "meet. These will help you catch them.$"

Sinnoh_Text_RowanGo:
	.string "SINNOH is wide, and its MOéMON are\n"
	.string "many. Go and see them all.$"

Sinnoh_Text_RowanNoTeam:
	.string "PROF. ROWAN: A TRAINER without a\n"
	.string "MOéMON? I left my briefcase out on\l"
	.string "ROUTE 201. See if it's still there.$"

Sinnoh_Text_RowanTalk:
	.string "PROF. ROWAN: SINNOH's MOéMON evolve in\n"
	.string "ways the world is only starting to\l"
	.string "understand. Travel far, and tell me\l"
	.string "what you see.$"

@ A story battle with no POKéMON in the party (tools/sinnoh/legends.py): not yet
@ BARRY's mom's visit (once, a new game's or the boat's first time leaving the house): stepping next to the door mat,
@ she comes in, the player is pushed back a step the way they came, she talks, walks out and is gone (object 2 of
@ the house's 1F, on the mat; tools/sinnoh/gen_maps.py EXTRA_TRIGGERS)
Sinnoh_EventScript_RivalMomVisit::
	lockall
	setvar 0x52F0, 3   @ (VAR_SINNOH_ARRIVAL: her visit played; the trigger needs 2)
	setflag 0x3D3A
	playse SE_EXIT
	clearflag 0x34F6
	addobject 2
	specialvar VAR_RESULT, GetPlayerFacingDirection
	call_if_eq VAR_RESULT, DIR_SOUTH, Sinnoh_EventScript_RivalMomPushUp
	call_if_eq VAR_RESULT, DIR_WEST, Sinnoh_EventScript_RivalMomPushRight
	call_if_eq VAR_RESULT, DIR_EAST, Sinnoh_EventScript_RivalMomPushLeft
	call_if_eq VAR_RESULT, DIR_NORTH, Sinnoh_EventScript_RivalMomPushDown
	applymovement 2, Sinnoh_Movement_RivalMomIn
	waitmovement 0
	msgbox Sinnoh_TwinleafTownPlayerHouse1f_Text_Person2, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Sinnoh_Movement_RivalMomOut
	waitmovement 0
	playse SE_EXIT
	removeobject 2
	setflag 0x34F6
	releaseall
	end

Sinnoh_EventScript_RivalMomPushUp:
	applymovement LOCALID_PLAYER, Sinnoh_Movement_PushedUp
	waitmovement 0
	return
Sinnoh_EventScript_RivalMomPushRight:
	applymovement LOCALID_PLAYER, Sinnoh_Movement_PushedRight
	waitmovement 0
	return
Sinnoh_EventScript_RivalMomPushLeft:
	applymovement LOCALID_PLAYER, Sinnoh_Movement_PushedLeft
	waitmovement 0
	return
Sinnoh_EventScript_RivalMomPushDown:
	applymovement LOCALID_PLAYER, Sinnoh_Movement_PushedDown
	waitmovement 0
	return

Sinnoh_Movement_PushedUp:
	lock_facing_direction
	walk_fast_up
	unlock_facing_direction
	step_end
Sinnoh_Movement_PushedRight:
	lock_facing_direction
	walk_fast_right
	unlock_facing_direction
	step_end
Sinnoh_Movement_PushedLeft:
	lock_facing_direction
	walk_fast_left
	unlock_facing_direction
	step_end
Sinnoh_Movement_PushedDown:
	lock_facing_direction
	walk_fast_down
	unlock_facing_direction
	step_end
Sinnoh_Movement_RivalMomIn:
	walk_up
	face_player
	step_end
Sinnoh_Movement_RivalMomOut:
	walk_down
	step_end

Sinnoh_EventScript_StoryNoTeam::
	msgbox Sinnoh_Text_StoryNoTeam, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_StoryNoTeam:
	.string "You don't have a MOéMON yet!\n"
	.string "Come back once you do.$"

@ Moemon Kai: a dig spot in the Underground: Platinum's "The wall is bulging slightly. Try digging into it?", then the
@ digging screen (src/sinnoh/s3d_mining.c; field_control_avatar.c finds the spot)
Sinnoh_EventScript_UndergroundDig::
	lockall
	special Sinnoh_UndergroundBulging
	message gSinnohMessage
	waitmessage
	yesnobox 20, 8
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_UndergroundDigEnd
	closemessage
	special Sinnoh_UndergroundDig
Sinnoh_EventScript_UndergroundDigEnd:
	closemessage
	releaseall
	end

@ Moemon Kai: the EXPLORER KIT used (item_use.c): down into the Underground, or back up (src/sinnoh/s3d_underground.c)
Sinnoh_EventScript_ExplorerKit::
	lockall
	playse SE_M_DIG
	special Sinnoh_ExplorerKitUse
	end

@ SURF on a DISTORTION WORLD floating platform's water (src/sinnoh/s3d_distortion.c S3D_Dw_FacingWater): the game's own
@ question, then the hop onto the water there (Platinum's FieldMoves_CanUseSurfDistortionWorld)
Sinnoh_EventScript_DwSurf::
	checkpartymove MOVE_SURF
	goto_if_eq VAR_RESULT, PARTY_SIZE, Sinnoh_EventScript_DwSurfEnd
	bufferpartymonnick STR_VAR_1, VAR_RESULT
	lockall
	msgbox gText_WantToUseSurf, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_DwSurfRelease
	msgbox gText_PlayerUsedSurf, MSGBOX_DEFAULT
	special S3D_Dw_Surf
Sinnoh_EventScript_DwSurfRelease:
	releaseall
Sinnoh_EventScript_DwSurfEnd:
	end
