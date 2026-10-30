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
