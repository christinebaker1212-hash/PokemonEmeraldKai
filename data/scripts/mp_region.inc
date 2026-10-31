@ Moemon Kai: going to another region (docs/REGIONS_PLAN.md "Customs"): only one Pokémon boards with the player,
@ the others wait in the GLOBAL PC. Called by the ports right before QzTravelToRegion (VAR_0x8004 = the region):
@ VAR_0x8005 = the party slot of the one who comes. B / cancel ends the script.
EventScript_RegionCustoms::
	specialvar VAR_RESULT, MP_RegionFirstVisit
	call_if_eq VAR_RESULT, TRUE, EventScript_RegionFirstVisit
	setvar VAR_0x8005, 0
	specialvar VAR_RESULT, MP_RegionPartyCount
	goto_if_le VAR_RESULT, 1, EventScript_RegionCustoms_Done
	copyvar VAR_0x8006, VAR_0x8004
	msgbox gText_RegionCustoms, MSGBOX_DEFAULT
	special ChoosePartyMon
	copyvar VAR_0x8005, VAR_0x8004
	copyvar VAR_0x8004, VAR_0x8006
	goto_if_eq VAR_0x8005, PARTY_NOTHING_CHOSEN, EventScript_RegionCustoms_Cancel
	msgbox gText_RegionCustomsSent, MSGBOX_DEFAULT
EventScript_RegionCustoms_Done::
	return

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
	specialvar VAR_RESULT, MP_GlobalPcLinked
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
	specialvar VAR_RESULT, MP_GlobalPcLinked
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
	.string "The line to JOHTO opens for the\n"
	.string "CHAMPION of KANTO's POKéMON LEAGUE.$"

gText_MagnetTrainRide::
	.string "The MAGNET TRAIN is running again!\p"
	.string "It goes to SAFFRON CITY in KANTO.\n"
	.string "Would you like to ride it?$"

gText_MagnetTrainDeparts::
	.string "The MAGNET TRAIN for SAFFRON CITY\n"
	.string "is now departing!$"

@ Moemon Kai: the Bug-Catching Contest's end by time, the fifth catch or the last SAFARI BALL (src/qz_bugcontest.c):
@ on to the judging (Quetzal's own script from here)
EventScript_QzBugContestOver::
	lockall
	playse SE_DING_DONG
	msgbox gText_QzBugContestOver, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C33FA

@ ...and leaving the park during it (the catches went to the PC, the party is back)
EventScript_QzBugContestLeft::
	lockall
	msgbox gText_QzBugContestLeft, MSGBOX_DEFAULT
	releaseall
	end

gText_QzBugContestOver::
	.string "ANNOUNCER: BEEEP!\p"
	.string "The Bug-Catching Contest is over!\n"
	.string "Please return to the judges.$"

gText_QzBugContestLeft::
	.string "You left the NATIONAL PARK, so the\n"
	.string "Bug-Catching Contest is over.\p"
	.string "Your catches are in the PC and your\n"
	.string "party is back with you.$"

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
	msgbox gText_GameCenterGuide, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_GameCenterGuide_No
	msgbox gText_GameCenterGuideGo, MSGBOX_DEFAULT
	closemessage
	setvar VAR_GAME_CENTER_PAID, 1
	warp MAP_RUSTBORO_CITY_GAME_CENTER_2F, 0
	waitstate
	end

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
	.string "machine on 3F. Shall I take you up?$"

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
