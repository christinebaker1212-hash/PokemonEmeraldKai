@ Moemon Kai: the boat to BATTLE ISLAND, offered by JOHTO's, KANTO's and SINNOH's ports before their usual trips (the BATTLE TICKET,
@ given after any region's ELITE FOUR: mp_battleisland.c). Returns when the player sails elsewhere or has no ticket;
@ a trip to the island ends the script.
EventScript_BattleIslandOffer::
	checkitem ITEM_BATTLE_TICKET, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_BattleIslandOffer_Return
	msgbox gText_BattleIslandOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_BattleIslandOffer_Return
	msgbox gText_BattleIslandAboard, MSGBOX_DEFAULT
	closemessage
	special MP_BattleIsland_Sail
	waitstate
	end

EventScript_BattleIslandOffer_Return::
	return

@ HOENN's ferry clerks list BATTLE ISLAND among their destinations (LILYCOVE's SSTIDAL_SELECTION_BATTLE_ISLAND,
@ SLATEPORT's MULTI_SSTIDAL_SLATEPORT_*_BI) instead of asking first: the trip itself.
EventScript_BattleIslandSail::
	msgbox gText_BattleIslandAboard, MSGBOX_DEFAULT
	closemessage
	special MP_BattleIsland_Sail
	waitstate
	end

gText_BattleIslandOffer::
	.string "That's a BATTLE TICKET! Would you\n"
	.string "like to sail to BATTLE ISLAND?$"

gText_BattleIslandAboard::
	.string "Off to BATTLE ISLAND, then.\n"
	.string "All aboard!$"
