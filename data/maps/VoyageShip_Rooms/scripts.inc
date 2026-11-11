VoyageShip_Rooms_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VoyageShip_Rooms_OnTransition
	.byte 0

@ Who is in here this voyage, where and looking how: mp_ship.c sCast, by where the ship is bound
VoyageShip_Rooms_OnTransition:
	setvar VAR_0x8004, 1
	special MP_ShipArrange
	end

VoyageShip_EventScript_Homecomer::
	setvar VAR_0x8004, 2
	goto VoyageShip_EventScript_Talk

VoyageShip_EventScript_Tourist::
	setvar VAR_0x8004, 3
	goto VoyageShip_EventScript_Talk

@ The trainer shows a POKéMON that lives where the ship is bound (VAR_0x8005: mp_ship.c sShowSpecies)
VoyageShip_EventScript_Trainer::
	lock
	faceplayer
	setvar VAR_0x8004, 4
	specialvar VAR_0x8006, MP_ShipTalk
	special ShowFieldMessageStringVar4
	waitmessage
	special MP_ShipShowMon
	waitmoncry
	waitbuttonpress
	hidemonpic
	closemessage
	call_if_eq VAR_0x8006, 1, VoyageShip_EventScript_Announce
	release
	end

@ The keepsake: a treat from where the ship is bound, once a region (VAR_0x8005: the item, 0 once given)
VoyageShip_EventScript_Granny::
	lock
	faceplayer
	setvar VAR_0x8004, 5
	specialvar VAR_0x8006, MP_ShipTalk
	call VoyageShip_EventScript_Say
	goto_if_eq VAR_0x8005, 0, VoyageShip_EventScript_GrannyEnd
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, VoyageShip_EventScript_GrannyFull
	special MP_ShipGiftDone
VoyageShip_EventScript_GrannyEnd::
	closemessage
	call_if_eq VAR_0x8006, 1, VoyageShip_EventScript_Announce
	release
	end

VoyageShip_EventScript_GrannyFull::
	call Common_EventScript_ShowBagIsFull
	goto VoyageShip_EventScript_GrannyEnd

VoyageShip_EventScript_CoupleHim::
	setvar VAR_0x8004, 6
	goto VoyageShip_EventScript_Talk

VoyageShip_EventScript_CoupleHer::
	setvar VAR_0x8004, 7
	goto VoyageShip_EventScript_Talk

@ The cabin's bed: once land is in sight, the rest that brings the ship in (as the S.S. TIDAL's)
VoyageShip_EventScript_Bed::
	lockall
	specialvar VAR_RESULT, MP_ShipBed
	goto_if_eq VAR_RESULT, 1, VoyageShip_EventScript_BedAsk
	call VoyageShip_EventScript_Say
	closemessage
	specialvar VAR_0x8006, MP_ShipCheckNear
	call_if_eq VAR_0x8006, 1, VoyageShip_EventScript_Announce
	releaseall
	end

VoyageShip_EventScript_BedAsk::
	special ShowFieldMessageStringVar4
	waitmessage
	yesnobox 20, 8
	closemessage
	goto_if_eq VAR_RESULT, NO, VoyageShip_EventScript_BedNo
	call Common_EventScript_OutOfCenterPartyHeal
	special MP_ShipDock
	call VoyageShip_EventScript_Announce
VoyageShip_EventScript_BedNo::
	releaseall
	end
