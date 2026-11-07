@ Moemon Kai: ORRE's shared scripts (docs/ORRE_DECISIONS.md)

@ A place's exits (tools/orre/maps.py "exits"): Colosseum's world map (src/orre/worldmap.c), then off to the place
@ chosen there, or back to the field
Orre_EventScript_WorldMap::
	lockall
	special Orre_WorldMap
	special Orre_WorldMapTravel
	goto_if_eq VAR_RESULT, TRUE, Orre_EventScript_WorldMapGone
	releaseall
	end
Orre_EventScript_WorldMapGone:
	end

@ ---- The boat to ORRE (src/orre/orre_ferry.c; docs/ORRE_DECISIONS.md "A post-game place"). Kai's own words: the boat
@ and its sailor aren't Colosseum's (the user's choice, 2026-11-07).

@ JOHTO's, KANTO's and SINNOH's ports offer it before their usual trips, as BATTLE ISLAND's boat (with the ORRE TICKET);
@ returns when the player sails elsewhere or has no ticket; a trip ends the script
EventScript_OrreOffer::
	checkitem ITEM_ORRE_TICKET, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_OrreOffer_Return
	msgbox gText_OrreOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_OrreOffer_Return
	goto EventScript_OrreSail
EventScript_OrreOffer_Return::
	return

@ HOENN's ferry clerks list ORRE among their destinations (LILYCOVE's SSTIDAL_SELECTION_ORRE, SLATEPORT's list)
EventScript_OrreSail::
	msgbox gText_OrreAboard, MSGBOX_DEFAULT
	closemessage
	special Orre_Sail
	waitstate
	end

@ The OUTSKIRT STAND's sailor (tools/orre/gen_maps.py KAI_OBJECTS): back to the port the player came from, never with a
@ Shadow Pokémon (docs/SHADOW_POKEMON.md decision 2)
Orre_EventScript_Sailor::
	lock
	faceplayer
	msgbox gText_OrreSailBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Orre_EventScript_SailorStay
	special Orre_ShadowAboard
	goto_if_eq VAR_RESULT, TRUE, Orre_EventScript_SailorShadow
	msgbox gText_OrreAllAboard, MSGBOX_DEFAULT
	closemessage
	special Orre_SailBack
	waitstate
	end
Orre_EventScript_SailorStay::
	msgbox gText_OrreTakeYourTime, MSGBOX_DEFAULT
	release
	end
Orre_EventScript_SailorShadow::
	msgbox gText_OrreShadowStays, MSGBOX_DEFAULT
	release
	end

gText_OrreOffer::
	.string "That's an ORRE TICKET! Would you\n"
	.string "like to sail to ORRE?$"

gText_OrreAboard::
	.string "Off to ORRE, then.\n"
	.string "All aboard!$"

gText_OrreSailBack::
	.string "The boat goes back to the port you\n"
	.string "came from whenever you like.\p"
	.string "Shall we set sail?$"

gText_OrreAllAboard::
	.string "All aboard!$"

gText_OrreTakeYourTime::
	.string "Take your time. I'll be right here\n"
	.string "with the boat.$"

gText_OrreShadowStays::
	.string "Hold on! There's a SHADOW POKéMON\n"
	.string "with you.\p"
	.string "One like that can't leave ORRE until\n"
	.string "its heart is open and it's purified.$"
