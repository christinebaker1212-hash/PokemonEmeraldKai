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

@ ---- Colosseum's scenes (src/orre/orre_vm.c): a room's script took the player (heroMoveInitEvent); the field stays
@ locked until it lets go (its battles start while this waits; it goes on holding after them)
Orre_EventScript_VmHold::
	lockall
Orre_EventScript_VmHoldLoop:
	special Orre_VmHold
	waitstate
	specialvar VAR_RESULT, Orre_VmHolding   @ (still held: back from a battle the scene started)
	goto_if_eq VAR_RESULT, TRUE, Orre_EventScript_VmHoldLoop
	releaseall
	end

@ A person talked to: their talk script (Colosseum's) runs, holding the field as a scene does
Orre_EventScript_Talk::
	lockall
	special Orre_VmTalk
	goto Orre_EventScript_VmHoldLoop

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

@ HOENN's ferry clerks list ORRE among their destinations (LILYCOVE's SSTIDAL_SELECTION_ORRE, SLATEPORT's list). ORRE's
@ packs come first (src/orre/orre_ferry.c Orre_CheckBoat): here, downloaded now, or the boat doesn't sail yet
EventScript_OrreSail::
	special Orre_CheckBoat
	waitstate
	goto_if_eq VAR_RESULT, 2, EventScript_OrreDownload
	goto_if_eq VAR_RESULT, 5, EventScript_OrreDownloading
	goto_if_eq VAR_RESULT, 3, EventScript_OrreNotYet
	goto_if_eq VAR_RESULT, 4, EventScript_OrreNoConnection
EventScript_OrreBoard::
	msgbox gText_OrreAboard, MSGBOX_DEFAULT
	closemessage
	special Orre_Sail
	waitstate
	end
EventScript_OrreDownload::
	msgbox gText_OrreDownload, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_OrreStay
	special Orre_DownloadPacks
	waitstate
	goto_if_eq VAR_RESULT, TRUE, EventScript_OrreBoard
	msgbox gText_OrreDownloadFailed, MSGBOX_DEFAULT
	release
	end
EventScript_OrreDownloading::
	msgbox gText_OrreDownloading, MSGBOX_DEFAULT
	special Orre_DownloadPacks
	waitstate
	goto_if_eq VAR_RESULT, TRUE, EventScript_OrreBoard
	msgbox gText_OrreDownloadFailed, MSGBOX_DEFAULT
	release
	end
EventScript_OrreNotYet::
	msgbox gText_OrreNotYet, MSGBOX_DEFAULT
	release
	end
EventScript_OrreNoConnection::
	msgbox gText_OrreNoConnection, MSGBOX_DEFAULT
	release
	end
EventScript_OrreStay::
	release
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

gText_OrreDownload::
	.string "Before the boat sails, ORRE has to\n"
	.string "be downloaded ({STR_VAR_1} MB).\p"
	.string "Download it now?$"

gText_OrreDownloadFailed::
	.string "The download didn't finish.\n"
	.string "({STR_VAR_1})\p"
	.string "Try again whenever you like.$"

gText_OrreDownloading::
	.string "ORRE is still being downloaded.\n"
	.string "The boat sails as soon as it's done.$"

gText_OrreNotYet::
	.string "Sorry, the boat to ORRE isn't\n"
	.string "sailing just yet.$"

gText_OrreNoConnection::
	.string "ORRE has to be downloaded first,\n"
	.string "but it can't be reached right now.\p"
	.string "({STR_VAR_1})$"

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
