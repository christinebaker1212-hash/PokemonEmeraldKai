@ Moemon Kai: SINNOH places of this game's own (words written for this game)

@ VEILSTONE's GAME CORNER: the coins counter and 12 slot machines (Emerald's, as MAUVILLE's)
Sinnoh_EventScript_GameCornerClerk::
	lock
	faceplayer
	msgbox Sinnoh_Text_GameCornerClerk, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NeedCoinCase
	message Sinnoh_Text_GameCornerCoins
	waitmessage
	showmoneybox 0, 0
	showcoinsbox 1, 6
	goto MauvilleCity_GameCorner_EventScript_ChooseCoinsDefault50
	end

Sinnoh_EventScript_SlotMachine0::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine1::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine2::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 2
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine3::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 3
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine4::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 4
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine5::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 5
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine6::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 6
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine7::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 7
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine8::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 8
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine9::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 9
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine10::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 10
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_EventScript_SlotMachine11::
	lockall
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, MauvilleCity_GameCorner_EventScript_NoCoinCase
	setvar VAR_0x8004, 11
	specialvar VAR_RESULT, GetSlotMachineId
	playslotmachine VAR_RESULT
	releaseall
	end

Sinnoh_Text_GameCornerClerk:
	.string "Welcome to VEILSTONE's GAME CORNER!\n"
	.string "The slots are hot today!$"
Sinnoh_Text_GameCornerCoins:
	.string "Can I get you some COINS?$"

@ VEILSTONE's PRIZE EXCHANGE: COINS for Platinum's prizes (src/sinnoh/s3d_places.c Sinnoh_PrizeMenu)
Sinnoh_EventScript_PrizeExchange::
	lock
	faceplayer
	msgbox Sinnoh_Text_PrizeWelcome, MSGBOX_DEFAULT
	checkitem ITEM_COIN_CASE
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_PrizeNoCase
	showcoinsbox 20, 1
Sinnoh_EventScript_PrizePick::
	message Sinnoh_Text_PrizeWhich
	waitmessage
	special Sinnoh_PrizeMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Sinnoh_EventScript_PrizeBye
	msgbox Sinnoh_Text_PrizeConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_PrizePick
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, VAR_0x8006, Sinnoh_EventScript_PrizeNoCoins
	checkitemspace VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_PrizeNoRoom
	removecoins VAR_0x8006
	additem VAR_0x8005
	updatecoinsbox 20, 1
	playse SE_SHOP
	msgbox Sinnoh_Text_PrizeHere, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_PrizePick

Sinnoh_EventScript_PrizeNoCoins::
	msgbox Sinnoh_Text_PrizeNoCoins, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_PrizePick

Sinnoh_EventScript_PrizeNoRoom::
	msgbox Sinnoh_Text_PrizeNoRoom, MSGBOX_DEFAULT
	goto Sinnoh_EventScript_PrizePick

Sinnoh_EventScript_PrizeBye::
	hidecoinsbox 20, 1
	msgbox Sinnoh_Text_PrizeBye, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_PrizeNoCase::
	msgbox Sinnoh_Text_PrizeNoCase, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_PrizeWelcome:
	.string "Welcome to the PRIZE EXCHANGE!\n"
	.string "Trade your COINS for fine prizes.$"

Sinnoh_Text_PrizeWhich:
	.string "Which prize would you like?$"

Sinnoh_Text_PrizeConfirm:
	.string "{STR_VAR_1} for {STR_VAR_2} COINS.\n"
	.string "Is that right?$"

Sinnoh_Text_PrizeHere:
	.string "Here you are! Enjoy it!$"

Sinnoh_Text_PrizeNoCoins:
	.string "Oh dear. You don't have enough\n"
	.string "COINS for that one.$"

Sinnoh_Text_PrizeNoRoom:
	.string "Your BAG has no room for it.$"

Sinnoh_Text_PrizeBye:
	.string "Come again with more COINS!$"

Sinnoh_Text_PrizeNoCase:
	.string "You'll need a COIN CASE to hold\n"
	.string "COINS before you can trade them.$"

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
	msgbox Sinnoh_Text_PrizeNoRoom, MSGBOX_DEFAULT
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

@ OREBURGH's MINING MUSEUM: its fossil scientist brings fossils back (JOHTO's / KANTO's specials, src/qz_specials.c:
@ Lv 20, as Platinum), and gives the player one SKULL or ARMOR FOSSIL first (flag 0x3D37; Platinum's come from the
@ Underground, which this game hasn't)
Sinnoh_EventScript_FossilScientist::
	lock
	faceplayer
	goto_if_set 0x3D37, Sinnoh_EventScript_FossilAsk
	msgbox Sinnoh_Text_FossilGift, MSGBOX_DEFAULT
	random 2
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_FossilGiftArmor
	giveitem ITEM_SKULL_FOSSIL
	goto Sinnoh_EventScript_FossilGiven

Sinnoh_EventScript_FossilGiftArmor:
	giveitem ITEM_ARMOR_FOSSIL

Sinnoh_EventScript_FossilGiven:
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_FossilEnd
	setflag 0x3D37

Sinnoh_EventScript_FossilAsk:
	special QzBagHasFossil
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_FossilNone
	msgbox Sinnoh_Text_FossilAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Sinnoh_EventScript_FossilNo
	special QzChooseItemFromBagForFossil
	waitstate
	goto_if_eq VAR_ITEM_ID, 0, Sinnoh_EventScript_FossilNo
	special QzIsSelectedItemFossil
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_EventScript_FossilNotOne
	special QzGiveRevivedFossilMon
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, Sinnoh_EventScript_FossilFull
	bufferitemname STR_VAR_2, VAR_ITEM_ID
	removeitem VAR_ITEM_ID, 1
	bufferspeciesname STR_VAR_1, VAR_0x800A
	msgbox Sinnoh_Text_FossilWorking, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	fadescreen FADE_FROM_BLACK
	msgbox Sinnoh_Text_FossilDone, MSGBOX_DEFAULT

Sinnoh_EventScript_FossilEnd:
	release
	end

Sinnoh_EventScript_FossilNone:
	msgbox Sinnoh_Text_FossilNone, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_FossilNo:
	msgbox Sinnoh_Text_FossilNo, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_FossilNotOne:
	msgbox Sinnoh_Text_FossilNotOne, MSGBOX_DEFAULT
	release
	end

Sinnoh_EventScript_FossilFull:
	msgbox Sinnoh_Text_FossilFull, MSGBOX_DEFAULT
	release
	end

Sinnoh_Text_FossilGift:
	.string "I study fossils from the mine. Here,\n"
	.string "start your own collection with this!$"

Sinnoh_Text_FossilAsk:
	.string "Fossils hold the shapes of ancient\n"
	.string "POKéMON. My machine can wake them up.\p"
	.string "Do you have a fossil for me?$"

Sinnoh_Text_FossilWorking:
	.string "The {STR_VAR_2}! Wonderful.\n"
	.string "Into the machine it goes...$"

Sinnoh_Text_FossilDone:
	.string "There! The {STR_VAR_1} is awake, and it's\n"
	.string "yours. Take good care of it!$"

Sinnoh_Text_FossilNone:
	.string "Fossils hold the shapes of ancient\n"
	.string "POKéMON. Bring me one, and I'll wake it.$"

Sinnoh_Text_FossilNo:
	.string "Come back whenever you like. The past\n"
	.string "isn't going anywhere.$"

Sinnoh_Text_FossilNotOne:
	.string "Hm, that's not a fossil. Not even a\n"
	.string "very old one.$"

Sinnoh_Text_FossilFull:
	.string "You've no room for another POKéMON,\n"
	.string "not even in your PC. Make some space!$"
