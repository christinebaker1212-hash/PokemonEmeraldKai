VoyageShip_Deck_MapScripts::
	.byte 0

@ The lookout: his spyglass shows the ship under way, and the coast once it's in sight
VoyageShip_EventScript_Lookout::
	lock
	faceplayer
	setvar VAR_0x8004, 8
	specialvar VAR_0x8006, MP_ShipTalk
	call VoyageShip_EventScript_Say
	specialvar VAR_RESULT, MP_ShipSpyglassAsk
	goto_if_eq VAR_RESULT, 0, VoyageShip_EventScript_LookoutEnd
	special ShowFieldMessageStringVar4
	waitmessage
	yesnobox 20, 8
	goto_if_eq VAR_RESULT, NO, VoyageShip_EventScript_LookoutEnd
	closemessage
	special MP_ShipSpyglass
	waitstate
VoyageShip_EventScript_LookoutEnd::
	closemessage
	call_if_eq VAR_0x8006, 1, VoyageShip_EventScript_Announce
	release
	end

VoyageShip_EventScript_Researcher::
	setvar VAR_0x8004, 9
	goto VoyageShip_EventScript_Talk
