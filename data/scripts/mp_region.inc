@ Moemon Kai: going to another region (docs/REGIONS_PLAN.md "Customs"): only one Pokémon boards with the player,
@ the others wait in the GLOBAL PC. Called by the ports right before QzTravelToRegion (VAR_0x8004 = the region):
@ VAR_0x8005 = the party slot of the one who comes. B / cancel ends the script.
EventScript_RegionCustoms::
	call EventScript_RegionHostOnly
	specialvar VAR_RESULT, MP_RegionFirstVisit
	call_if_eq VAR_RESULT, TRUE, EventScript_RegionFirstVisit
EventScript_RegionCustomsPick::   @ (the gates between JOHTO and KANTO: customs without the boats' first-visit greeting)
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, MP_RegionPartyCount
	goto_if_le VAR_RESULT, 1, EventScript_RegionCustoms_Done
	copyvar VAR_0x8006, VAR_0x8004
	msgbox gText_RegionCustoms, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate   @ (1.101.0: without it the region's number was taken as the party slot)
	copyvar VAR_0x8005, VAR_0x8004
	copyvar VAR_0x8004, VAR_0x8006
	goto_if_eq VAR_0x8005, PARTY_NOTHING_CHOSEN, EventScript_RegionCustoms_Cancel
	msgbox gText_RegionCustomsSent, MSGBOX_DEFAULT
EventScript_RegionCustoms_Done::
	call EventScript_RegionLuggage
	return

@ Moemon Kai (1.101.0): only the KEY ITEMS board; customs sends the rest of the BAG to the GLOBAL PC (qz_specials.c
@ TravelNow, mp_region.c MP_Region_SendLuggage)
EventScript_RegionLuggage::
	specialvar VAR_RESULT, MP_RegionHasLuggage
	goto_if_eq VAR_RESULT, FALSE, EventScript_RegionLuggage_None
	msgbox gText_RegionLuggage, MSGBOX_DEFAULT
EventScript_RegionLuggage_None::
	return

gText_RegionLuggage::
	.string "CUSTOMS: Oof! Your luggage is far\n"
	.string "too heavy to take aboard!\p"
	.string "Everything but your KEY ITEMS will\n"
	.string "wait for you in the GLOBAL PC.\p"
	.string "It reaches any PC in a region whose\n"
	.string "POKéMON LEAGUE you've beaten.$"

@ Moemon Kai: online, only the room's host leads the way to another region (the user, 2026-11-05); everyone follows
EventScript_RegionHostOnly::
	specialvar VAR_RESULT, MP_RegionTripAllowed
	goto_if_eq VAR_RESULT, FALSE, EventScript_RegionHostOnly_No
	return

EventScript_RegionHostOnly_No::
	msgbox gText_RegionHostOnly, MSGBOX_DEFAULT
	releaseall
	end

gText_RegionHostOnly::
	.string "You're in a friend's room: only its\n"
	.string "host can lead the way to another\l"
	.string "region. Everyone sails together!$"

@ ...and when the host sets sail, each friend in the room (mp_main.c): VAR_0x8004 = the region, STR_VAR_1 its name.
@ The one POKéMON that comes along is picked as at customs (B: the first), then the same voyage to the same port.
EventScript_RegionFollowHost::
	lockall
	msgbox gText_RegionFollowHost, MSGBOX_DEFAULT
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, MP_RegionPartyCount
	goto_if_le VAR_RESULT, 1, EventScript_RegionFollowHost_Go
	copyvar VAR_0x8006, VAR_0x8004
	msgbox gText_RegionCustoms, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x8005, VAR_0x8004
	copyvar VAR_0x8004, VAR_0x8006
	call_if_eq VAR_0x8005, PARTY_NOTHING_CHOSEN, EventScript_RegionFollowHost_First
	msgbox gText_RegionCustomsSent, MSGBOX_DEFAULT
EventScript_RegionFollowHost_Go::
	call EventScript_RegionLuggage
	closemessage
	specialvar VAR_RESULT, MP_RegionFollowNow
	goto_if_eq VAR_RESULT, 0, EventScript_RegionFollowHost_NoRoom
	waitstate
	releaseall
	end

EventScript_RegionFollowHost_First::
	setvar VAR_0x8005, 0
	return

EventScript_RegionFollowHost_NoRoom::
	releaseall
	end

gText_RegionFollowHost::
	.string "Your host is setting sail for\n"
	.string "{STR_VAR_1}! Everyone in the room\l"
	.string "boards the same boat.$"

EventScript_RegionFirstVisit::
	msgbox gStringVar4, MSGBOX_DEFAULT
	return

EventScript_RegionCustoms_Cancel::
	msgbox gText_RegionCustomsCancel, MSGBOX_DEFAULT
	releaseall
	end

gText_RegionCustoms::
	.string "CUSTOMS: only one POKéMON may board\n"
	.string "with you.\p"
	.string "The others will wait for you in the\n"
	.string "GLOBAL PC. Which one comes along?$"

gText_RegionCustomsSent::
	.string "Your other POKéMON were sent to the\n"
	.string "GLOBAL PC. Have a pleasant voyage!$"

gText_RegionCustomsCancel::
	.string "We hope to see you again!$"

@ HOENN's ferry clerks (SLATEPORT, LILYCOVE): the boat to the other regions beside the S.S. TIDAL's own voyages.
@ Returns to go on with the usual voyages; a trip or EXIT ends the script.
EventScript_HoennRegionFerry::
	call EventScript_BattleIslandOffer
	specialvar VAR_RESULT, QzGetReachableRegionsMask
	goto_if_eq VAR_RESULT, 0, EventScript_HoennRegionFerry_Usual
	message gText_HoennRegionFerry
	waitmessage
	multichoice 17, 6, MULTI_QZ_0x8D, FALSE
	goto_if_eq VAR_RESULT, 0, EventScript_HoennRegionFerry_Usual
	goto_if_eq VAR_RESULT, 1, EventScript_HoennRegionFerry_Other
	goto EventScript_RegionCustoms_Cancel

EventScript_HoennRegionFerry_Usual::
	return

EventScript_HoennRegionFerry_Other::
	call Quetzal_EventScript_840F954
	goto_if_eq VAR_RESULT, 127, EventScript_RegionCustoms_Cancel
	goto Quetzal_EventScript_840FA9D

gText_HoennRegionFerry::
	.string "Where would you like to sail?$"

@ GOLDENROD's MAGNET TRAIN (Quetzal's is broken down): it runs to KANTO's SAFFRON once JOHTO's League is beaten.
@ The way back is the boat (OLIVINE / VERMILION). Customs as at the ports.
EventScript_MagnetTrain::
	specialvar VAR_RESULT, MP_MagnetTrainOpen   @ (either LEAGUE beaten, the S.S. TICKET: mp_region.c)
	goto_if_eq VAR_RESULT, 0, EventScript_MagnetTrain_Usual
	msgbox gText_MagnetTrainRide, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_RegionCustoms_Cancel
	setvar VAR_0x8004, 2
	call EventScript_RegionCustoms
	msgbox gText_MagnetTrainDeparts, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8007, 0x7A
	specialvar VAR_RESULT, QzTravelToRegion
	goto_if_eq VAR_RESULT, 0, EventScript_RegionCustoms_Cancel
	waitstate
	end

EventScript_MagnetTrain_Usual::
	return

@ ...and SAFFRON's way back to GOLDENROD (an attendant by the POKeMON CENTER: FireRed's SAFFRON has no station),
@ once KANTO's League is beaten
EventScript_MagnetTrainSaffron::
	lock
	faceplayer
	specialvar VAR_RESULT, MP_MagnetTrainOpen   @ (either LEAGUE beaten, the S.S. TICKET: mp_region.c)
	goto_if_eq VAR_RESULT, 0, EventScript_MagnetTrainSaffron_Closed
	msgbox gText_MagnetTrainRideJohto, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_RegionCustoms_Cancel
	setvar VAR_0x8004, 1
	call EventScript_RegionCustoms
	msgbox gText_MagnetTrainDepartsJohto, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8007, 0x7A
	specialvar VAR_RESULT, QzTravelToRegion
	goto_if_eq VAR_RESULT, 0, EventScript_RegionCustoms_Cancel
	waitstate
	end

EventScript_MagnetTrainSaffron_Closed::
	msgbox gText_MagnetTrainSaffronClosed, MSGBOX_DEFAULT
	release
	end

gText_MagnetTrainRideJohto::
	.string "This is the MAGNET TRAIN's SAFFRON\n"
	.string "counter. It goes to GOLDENROD CITY\p"
	.string "in JOHTO. Would you like to ride it?$"

gText_MagnetTrainDepartsJohto::
	.string "The MAGNET TRAIN for GOLDENROD CITY\n"
	.string "is now departing!$"

gText_MagnetTrainSaffronClosed::
	.string "This is the MAGNET TRAIN's SAFFRON\n"
	.string "counter.\p"
	.string "The line to JOHTO opens for a\n"
	.string "LEAGUE CHAMPION with an S.S. TICKET.$"

gText_MagnetTrainRide::
	.string "The MAGNET TRAIN is running again!\p"
	.string "It goes to SAFFRON CITY in KANTO.\n"
	.string "Would you like to ride it?$"

gText_MagnetTrainDeparts::
	.string "The MAGNET TRAIN for SAFFRON CITY\n"
	.string "is now departing!$"

@ Moemon Kai: the Bug-Catching Contest's end by time or the last SAFARI BALL (src/qz_bugcontest.c; Crystal's
@ BugCatchingContestOverScript / OutOfBallsScript), on to the judging
EventScript_QzBugContestOver::
	lockall
	playse SE_DING_DONG
	msgbox gText_QzBugContestTimeUp, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C33FA

EventScript_QzBugContestNoBalls::
	lockall
	playse SE_DING_DONG
	msgbox gText_QzBugContestIsOver, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C33FA

@ ...a second catch: Crystal's BugContest_SetCaughtContestMon ("You already caught a ... Switch POKeMON?")
EventScript_QzBugContestSwitch::
	lockall
	msgbox gText_QzBugContestAlreadyCaught, MSGBOX_DEFAULT
	msgbox gText_QzBugContestAskSwitch, MSGBOX_YESNO
	special QzBugContestSwitch
	releaseall
	end

@ ...and leaving the park during it (the catch to the party or the PC, the party back)
EventScript_QzBugContestLeft::
	lockall
	msgbox gText_QzBugContestLeft, MSGBOX_DEFAULT
	releaseall
	end

gText_QzBugContestTimeUp::
	.string "ANNOUNCER: BEEEP!\p"
	.string "Time’s up!$"

gText_QzBugContestIsOver::
	.string "ANNOUNCER: The Contest is over!$"

gText_QzBugContestAlreadyCaught::
	.string "You already caught a {STR_VAR_2}.$"

gText_QzBugContestAskSwitch::
	.string "Switch MOéMON?$"

gText_QzBugContestLeft::
	.string "You left the NATIONAL PARK, so the\n"
	.string "Bug-Catching Contest is over.\p"
	.string "Your party is back with you.$"

@ Moemon Kai: HEADBUTT on JOHTO's / KANTO's trees (field_control_avatar.c, wild_encounter.c MP_Headbutt_IsTree): a
@ POKeMON may fall out (the map's tree table)
EventScript_QzHeadbuttTree::
	lockall
	checkpartymove MOVE_HEADBUTT
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_QzHeadbuttEnd
	bufferpartymonnick STR_VAR_1, VAR_RESULT
	msgbox gText_QzHeadbuttAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_QzHeadbuttEnd
	msgbox gText_QzHeadbuttUsed, MSGBOX_DEFAULT
	closemessage
	playse SE_M_HEADBUTT
	setvar VAR_0x8004, 0
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 6
	setvar VAR_0x8007, 2
	special ShakeCamera
	waitstate
	special MP_HeadbuttWildEncounter
	goto_if_eq VAR_RESULT, FALSE, EventScript_QzHeadbuttNothing
	waitstate
	releaseall
	end

EventScript_QzHeadbuttNothing::
	msgbox gText_QzHeadbuttNothing, MSGBOX_DEFAULT
EventScript_QzHeadbuttEnd::
	closemessage
	releaseall
	end

gText_QzHeadbuttAsk::
	.string "A POKéMON could be up in this tree.\n"
	.string "Want to use HEADBUTT?$"

gText_QzHeadbuttUsed::
	.string "{STR_VAR_1} did a HEADBUTT!$"

gText_QzHeadbuttNothing::
	.string "Nope.\nNothing happened…$"

@ GOLDENROD's and CELADON's GAME CORNERS: the way up to the GAME CENTER. JOHTO's / KANTO's is HOENN's 2F and 3F
@ (shared by every region: mp_region.c MP_Region_IsSharedMap; no 1F, no ROOFTOP), its stairs down lead back here
EventScript_GameCenterGuide::
	lock
	faceplayer
	showmoneybox 0, 0
	msgbox gText_GameCenterGuide, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_GameCenterGuide_NoBox
	checkmoney 500   @ (admission as HOENN's GAME CENTER charges: RustboroCity_GameCenter_1F)
	goto_if_eq VAR_RESULT, FALSE, EventScript_GameCenterGuide_NoMoney
	removemoney 500
	updatemoneybox
	playse SE_SHOP
	msgbox gText_GameCenterGuideGo, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	setvar VAR_GAME_CENTER_PAID, 1
	warp MAP_RUSTBORO_CITY_GAME_CENTER_2F, 0
	waitstate
	end

EventScript_GameCenterGuide_NoMoney::
	msgbox gText_GameCenterGuideNoMoney, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

EventScript_GameCenterGuide_NoBox::
	hidemoneybox
	goto EventScript_GameCenterGuide_No

EventScript_GameCenterRoofClosed::
	lockall
	msgbox gText_GameCenterRoofClosed, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, EventScript_GameCenterRoofClosed_StepBack
	waitmovement 0
	releaseall
	end

EventScript_GameCenterRoofClosed_StepBack:
	walk_down
	step_end

gText_GameCenterRoofClosed::
	.string "The way up to the ROOFTOP is closed.$"


EventScript_GameCenterGuide_No::
	msgbox gText_GameCenterGuideNo, MSGBOX_DEFAULT
	release
	end

gText_GameCenterGuide::
	.string "Upstairs is the GAME CENTER! The\n"
	.string "CARD SHOP on 2F, games and the DANCE\p"
	.string "machine on 3F.\p"
	.string "Admission is ¥500, and it's good for\n"
	.string "every floor until you leave.\p"
	.string "Shall I take you up?$"

gText_GameCenterGuideNoMoney::
	.string "Oh, I'm sorry. Admission is ¥500...$"

gText_GameCenterGuideGo::
	.string "Right this way! Have fun!$"

gText_GameCenterGuideNo::
	.string "Any time! The GAME CENTER is always\n"
	.string "open.$"

@ GOLDENROD's department store elevator (Quetzal's ran in its own code): the panel lists B1F-6F
@ (qz_specials.c QzGoldenrodElevatorMenu), the doors then open on the floor picked (the dynamic warp)
EventScript_GoldenrodElevator::
	lockall
	message gText_GoldenrodElevatorWhich
	waitmessage
	special QzGoldenrodElevatorMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, EventScript_GoldenrodElevator_Done
	specialvar VAR_RESULT, QzGoldenrodElevatorGo
	goto_if_eq VAR_RESULT, 0, EventScript_GoldenrodElevator_Done
	closemessage
	waitse
	special MoveElevator
	waitstate
EventScript_GoldenrodElevator_Done::
	closemessage
	releaseall
	end

gText_GoldenrodElevatorWhich::
	.string "Which floor would you like?$"

@ The boat's list with SINNOH on it (Quetzal_EventScript_840F954 comes here once SINNOH can be sailed to): the other
@ three regions' ports. RESULT = the region (0 HOENN .. 3 SINNOH), 0x7F: EXIT (as that script's own menus)
EventScript_RegionPickAll::
	setvar VAR_0x8001, 0xFF
	specialvar VAR_0x8002, MP_RegionBoatAsk   @ (0xFF: the region the player is in)
	message Quetzal_Text_840FAE5
	waitmessage
	goto_if_eq VAR_0x8002, 1, EventScript_RegionPickAll_Johto
	goto_if_eq VAR_0x8002, 2, EventScript_RegionPickAll_Kanto
	goto_if_eq VAR_0x8002, 3, EventScript_RegionPickAll_Sinnoh
	multichoice 12, 2, MULTI_REGIONS_FROM_HOENN, FALSE
	goto EventScript_RegionPickAll_Pick

EventScript_RegionPickAll_Johto::
	multichoice 12, 2, MULTI_REGIONS_FROM_JOHTO, FALSE
	goto EventScript_RegionPickAll_Pick

EventScript_RegionPickAll_Kanto::
	multichoice 12, 2, MULTI_REGIONS_FROM_KANTO, FALSE
	goto EventScript_RegionPickAll_Pick

EventScript_RegionPickAll_Sinnoh::
	multichoice 12, 2, MULTI_REGIONS_FROM_SINNOH, FALSE
EventScript_RegionPickAll_Pick::
	specialvar VAR_RESULT, MP_RegionBoatPick   @ (the choice -> the region, 0x7F: EXIT)
	return

@ SINNOH's S.S. TICKET, once its CHAMPION is beaten (the CHAMPION's win; Platinum has no ticket of its own, and a game
@ begun in SINNOH has no other way to the boat): as HOENN's from Dad, JOHTO's from ELM, KANTO's from BILL
EventScript_SinnohTicket::
	checkitem ITEM_SS_TICKET, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_SinnohTicket_Had
	giveitem ITEM_SS_TICKET
EventScript_SinnohTicket_Had::
	return

@ CANALAVE's harbor sailor: the boat to the other regions (SINNOH's port). Shown only while the boat can sail
@ (FLAG_SINNOH_HIDE_REGION_BOAT: s3d_scrcmd.c, SINNOH's ELITE FOUR beaten and the S.S. TICKET, as JOHTO's and
@ KANTO's ports) and for the first boat in
EventScript_SinnohRegionBoat::
	lock
	faceplayer
	call EventScript_BattleIslandOffer
	specialvar VAR_RESULT, QzGetReachableRegionsMask
	goto_if_eq VAR_RESULT, 0, EventScript_SinnohRegionBoat_NotYet
	call Quetzal_EventScript_840F954
	goto_if_eq VAR_RESULT, 0x7F, EventScript_RegionCustoms_Cancel
	goto Quetzal_EventScript_840FA9D

EventScript_SinnohRegionBoat_NotYet::
	msgbox gText_SinnohRegionBoatNotYet, MSGBOX_DEFAULT
	release
	end

gText_SinnohRegionBoatNotYet::
	.string "This boat sails to faraway regions.\p"
	.string "Come back with an S.S. TICKET once\n"
	.string "you're the CHAMPION of SINNOH.$"

@ SINNOH's Seal shop (the SUNYSHORE MARKET, Platinum's PokeMartSeal), POKé BALL effects in the Seals' place
@ (src/sinnoh/s3d_seals.c): the day's pieces at Platinum's prices; VAR_0x8004 = the day's stock (0 Monday)
EventScript_SinnohSealShop::
	sinnoh 13, VAR_0x8005, 1
	sinnoh 49, 4, VAR_0x8004
	sinnoh 15
	goto_if_eq VAR_0x8005, 0xFFFE, EventScript_SinnohSealShop_End
	sinnoh 49, 5, VAR_0x8005, VAR_RESULT
	goto_if_eq VAR_RESULT, 0, EventScript_SinnohSealShop_NoMoney
	goto_if_eq VAR_RESULT, 2, EventScript_SinnohSealShop
	playse SE_SHOP
	msgbox gText_HereYouGoThankYou, MSGBOX_DEFAULT
	goto EventScript_SinnohSealShop

EventScript_SinnohSealShop_NoMoney::
	msgbox gText_YouDontHaveMoney, MSGBOX_DEFAULT
	goto EventScript_SinnohSealShop

EventScript_SinnohSealShop_End::
	closemessage
	return

@ Moemon Kai: an HM handed over unless the player has it already (from another region's giver: the BAG, the PC or a
@ held one: item.c MP_PlayerHasHm). VAR_0x8004 the HM; VAR_RESULT TRUE if it was given
EventScript_GiveHmOnce::
	specialvar VAR_RESULT, MP_PlayerHasHmVar
	goto_if_eq VAR_RESULT, TRUE, EventScript_GiveHmOnce_Had
	giveitem VAR_0x8004
	setvar VAR_RESULT, TRUE
	return

EventScript_GiveHmOnce_Had:
	bufferitemname STR_VAR_1, VAR_0x8004
	msgbox Text_GiveHmOnce_Had, MSGBOX_DEFAULT
	setvar VAR_RESULT, FALSE
	return

Text_GiveHmOnce_Had:
	.string "Oh! You're already carrying\n"
	.string "{STR_VAR_1}.\p"
	.string "Then there's nothing more I can give\n"
	.string "you. Your MOéMON know the way!$"

@ Moemon Kai: an HM lying on the ground (KANTO's ICEFALL CAVE HM07), unless the player has it already (then the ball
@ is just gone). VAR_0x8004 the HM
EventScript_FindHmOnce::
	specialvar VAR_RESULT, MP_PlayerHasHmVar
	goto_if_eq VAR_RESULT, TRUE, EventScript_FindHmOnce_Had
	setorcopyvar VAR_0x8000, VAR_0x8004
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

EventScript_FindHmOnce_Had:
	lock
	bufferitemname STR_VAR_1, VAR_0x8004
	msgbox Text_FindHmOnce_Had, MSGBOX_DEFAULT
	removeobject VAR_LAST_TALKED
	release
	end

Text_FindHmOnce_Had:
	.string "It's {STR_VAR_1}! But you're already\n"
	.string "carrying one, so you leave it be.$"

@ Moemon Kai: OPTIONS > GAME > RETURN TO MAIN MENU (src/option_menu.c): the game's save prompt; saved, or once the
@ player is sure about leaving without saving, back to the main menu (a soft reset)
EventScript_MpReturnToTitle::
	lockall
	call Common_EventScript_SaveGame
	goto_if_ne VAR_RESULT, 0, EventScript_MpReturnToTitle_Go
	msgbox Text_MpReturnToTitle_Sure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_MpReturnToTitle_Go
	releaseall
	end

EventScript_MpReturnToTitle_Go:
	special MP_ReturnToTitle
	end

Text_MpReturnToTitle_Sure:
	.string "Return to the main menu without\n"
	.string "saving? Anything since your last\l"
	.string "save will be lost.$"

@ Moemon Kai (tests, mp rivalname): JOHTO's rival named as at ELM's (QzNameJohtoRival), then the officer's line
EventScript_MpNameJohtoRival::
	lockall
	specialvar VAR_RESULT, QzNameJohtoRival
	call_if_eq VAR_RESULT, TRUE, EventScript_MpNameJohtoRivalWait
	msgbox Johto_ElmsLab_Text_83E3A0D, MSGBOX_DEFAULT
	releaseall
	end

EventScript_MpNameJohtoRivalWait:
	waitstate
	return

@ Moemon Kai: JOHTO's APRICORN trees as Crystal's fruit trees, one APRICORN a day each, with Crystal's words
@ (pokecrystal engine/events/fruit_trees.asm; its PACK is the BAG here): src/mp/mp_apricorn.c
Johto_EventScript_FruitTree::
	lockall
	special QzFruitTreeCheck
	msgbox Johto_Text_FruitBearingTree, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, FALSE, Johto_EventScript_FruitTreeBare
	msgbox Johto_Text_HeyItsFruit, MSGBOX_DEFAULT
	checkitemspace VAR_0x8005, 1
	goto_if_eq VAR_RESULT, FALSE, Johto_EventScript_FruitTreeFull
	additem VAR_0x8005, 1
	special QzFruitTreePick
	playfanfare MUS_OBTAIN_ITEM
	message Johto_Text_ObtainedFruit
	waitmessage
	waitfanfare
	waitbuttonpress
	closemessage
	releaseall
	end

Johto_EventScript_FruitTreeBare::
	msgbox Johto_Text_NothingHere, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Johto_EventScript_FruitTreeFull::
	msgbox Johto_Text_FruitBagFull, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Johto_Text_FruitBearingTree::
	.string "It’s a fruit-bearing tree.$"

Johto_Text_HeyItsFruit:
	.string "Hey! It’s\n{STR_VAR_1}!$"

Johto_Text_ObtainedFruit:
	.string "Obtained\n{STR_VAR_1}!$"

Johto_Text_FruitBagFull:
	.string "But the BAG is full…$"

Johto_Text_NothingHere::
	.string "There’s nothing here…$"

@ The first boat to KANTO / JOHTO (MP_Region_FirstBoatIn, VAR_REGION_ARRIVAL 0x52FD = the region): off the boat at
@ VERMILION / OLIVINE, the port's sailor meets the player and takes them to the region's first town, where a room
@ waits (PALLET TOWN, NEW BARK TOWN). The ports' frame scripts run these.
EventScript_RegionArrivalKanto::
	lockall
	turnobject OBJ_EVENT_ID_PLAYER, DIR_SOUTH
	turnobject 4, DIR_NORTH
	msgbox gText_RegionArrivalKanto, MSGBOX_DEFAULT
	closemessage
	setvar 0x52FD, 0
	warp MAP_KANTO_PALLET_TOWN_PLAYERS_HOUSE_2F, 6, 6
	waitstate
	releaseall
	end

EventScript_RegionArrivalJohto::
	lockall
	turnobject OBJ_EVENT_ID_PLAYER, DIR_SOUTH
	turnobject 1, DIR_NORTH
	msgbox gText_RegionArrivalJohto, MSGBOX_DEFAULT
	closemessage
	setvar 0x52FD, 0
	warp MAP_JOHTO_PLAYERS_HOUSE2F, 4, 4
	waitstate
	releaseall
	end

gText_RegionArrivalKanto::
	.string "Ahoy! You're the TRAINER from\n"
	.string "overseas, aren't you?\p"
	.string "Welcome to KANTO! PROF. OAK asked\n"
	.string "me to watch for your boat.\p"
	.string "A room's ready for you in PALLET\n"
	.string "TOWN, where his POKéMON LAB is.\p"
	.string "Follow me, I'll take you there!$"

gText_RegionArrivalJohto::
	.string "Ahoy! You're the TRAINER from\n"
	.string "overseas, aren't you?\p"
	.string "Welcome to JOHTO! PROF. ELM asked\n"
	.string "me to watch for your boat.\p"
	.string "A room's ready for you in NEW BARK\n"
	.string "TOWN, where his POKéMON LAB is.\p"
	.string "Follow me, I'll take you there!$"

@ The walk between JOHTO's VICTORY ROAD gate and KANTO's ROUTE 22 gate (docs/REGIONS_AUDIT.md cases 43, 70): Crystal's
@ way through, for champions of both LEAGUES, with Kai's customs as on the boats (qz_specials.c MP_RegionGateCross)
EventScript_RegionGateToKanto::
	lockall
	setvar VAR_0x8004, 2   @ REGION_KANTO
	specialvar VAR_RESULT, QzVictoryRoadGateCheck
	goto_if_eq VAR_RESULT, 0, EventScript_RegionGateClosedJohto
	specialvar VAR_RESULT, MP_RegionTripAllowed
	goto_if_eq VAR_RESULT, FALSE, EventScript_RegionGateHostOnlyJohto
	call EventScript_RegionCustomsPick
	specialvar VAR_RESULT, MP_RegionGateCross
	goto_if_eq VAR_RESULT, 0, EventScript_RegionGateRefusedJohto
	warp MAP_KANTO_ROUTE22_NORTH_ENTRANCE, 3, 2
	waitstate
	releaseall
	end

EventScript_RegionGateToJohto::
	lockall
	setvar VAR_0x8004, 1   @ REGION_JOHTO
	specialvar VAR_RESULT, QzVictoryRoadGateCheck
	goto_if_eq VAR_RESULT, 0, EventScript_RegionGateClosedKanto
	specialvar VAR_RESULT, MP_RegionTripAllowed
	goto_if_eq VAR_RESULT, FALSE, EventScript_RegionGateHostOnlyKanto
	call EventScript_RegionCustomsPick
	specialvar VAR_RESULT, MP_RegionGateCross
	goto_if_eq VAR_RESULT, 0, EventScript_RegionGateRefusedKanto
	warp MAP_JOHTO_VICTORY_ROAD_GATE, 19, 9
	waitstate
	releaseall
	end

EventScript_RegionGateRefusedJohto::
	msgbox Quetzal_Text_840FB2D, MSGBOX_DEFAULT
EventScript_RegionGateClosedJohto::
	msgbox Quetzal_Text_83E90EF, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkLeft
	waitmovement 0
	releaseall
	end

EventScript_RegionGateRefusedKanto::
	msgbox Quetzal_Text_840FB2D, MSGBOX_DEFAULT
EventScript_RegionGateClosedKanto::
	msgbox Quetzal_Text_83E90EF, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkDown
	waitmovement 0
	releaseall
	end

EventScript_RegionGateHostOnlyJohto::
	msgbox gText_RegionHostOnly, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkLeft
	waitmovement 0
	releaseall
	end

EventScript_RegionGateHostOnlyKanto::
	msgbox gText_RegionHostOnly, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkDown
	waitmovement 0
	releaseall
	end
