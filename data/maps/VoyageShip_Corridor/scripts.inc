@ Moemon Kai: the ship of a first voyage to a region (src/mp/mp_ship.c). Every word comes from there (gStringVar4),
@ by where the ship is bound.
VoyageShip_Corridor_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VoyageShip_Corridor_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, VoyageShip_Corridor_OnFrame
	.byte 0

VoyageShip_Corridor_OnTransition:
	specialvar VAR_TEMP_1, MP_ShipGreetDue
	end

VoyageShip_Corridor_OnFrame:
	map_script_2 VAR_TEMP_1, 1, VoyageShip_EventScript_Greeting
	.2byte 0

VoyageShip_EventScript_Greeting::
	lockall
	setvar VAR_TEMP_1, 0
	turnobject OBJ_EVENT_ID_PLAYER, DIR_SOUTH
	special MP_ShipGreet
	call VoyageShip_EventScript_Say
	closemessage
	releaseall
	end

VoyageShip_EventScript_Say::
	special ShowFieldMessageStringVar4
	waitmessage
	waitbuttonpress
	return

@ VAR_0x8004 = the passenger (mp_ship.c P_*)
VoyageShip_EventScript_Talk::
	lock
	faceplayer
	specialvar VAR_0x8006, MP_ShipTalk
	call VoyageShip_EventScript_Say
	closemessage
	call_if_eq VAR_0x8006, 1, VoyageShip_EventScript_Announce
	release
	end

VoyageShip_EventScript_Announce::
	playse SE_DING_DONG
	waitse
	special MP_ShipAnnounce
	call VoyageShip_EventScript_Say
	closemessage
	return

VoyageShip_EventScript_OldSalt::
	setvar VAR_0x8004, 0
	goto VoyageShip_EventScript_Talk

VoyageShip_EventScript_Kid::
	setvar VAR_0x8004, 1
	goto VoyageShip_EventScript_Talk

VoyageShip_EventScript_ExitSailor::
	lock
	faceplayer
	specialvar VAR_RESULT, MP_ShipExit
	call VoyageShip_EventScript_Say
	closemessage
	goto_if_eq VAR_RESULT, 1, VoyageShip_EventScript_Leave
	specialvar VAR_0x8006, MP_ShipCheckNear
	call_if_eq VAR_0x8006, 1, VoyageShip_EventScript_Announce
	release
	end

VoyageShip_EventScript_Leave::
	special MP_ShipLeave
	waitstate
	release
	end

VoyageShip_EventScript_Porthole::
	lockall
	special MP_ShipPorthole
	call VoyageShip_EventScript_Say
	closemessage
	releaseall
	end

VoyageShip_EventScript_CabinSign::
	msgbox VoyageShip_Text_CabinSign, MSGBOX_SIGN
	end

VoyageShip_EventScript_YourCabinSign::
	msgbox VoyageShip_Text_YourCabinSign, MSGBOX_SIGN
	end

VoyageShip_Text_CabinSign:
	.string "A passenger's cabin.$"

VoyageShip_Text_YourCabinSign:
	.string "CABIN 2\n"
	.string "{PLAYER}'s cabin.$"
