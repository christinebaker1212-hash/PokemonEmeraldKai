BattleIsland_MapScripts::
	.byte 0

@ The landing's sailor: the boat back to the port the player came from (mp_battleisland.c MP_BattleIsland_Return)
BattleIsland_EventScript_Sailor::
	lock
	faceplayer
	msgbox BattleIsland_Text_SailBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, BattleIsland_EventScript_SailorStay
	msgbox BattleIsland_Text_AllAboard, MSGBOX_DEFAULT
	closemessage
	special MP_BattleIsland_Return
	waitstate
	end

BattleIsland_EventScript_SailorStay::
	msgbox BattleIsland_Text_TakeYourTime, MSGBOX_DEFAULT
	release
	end

BattleIsland_EventScript_Sign::
	msgbox BattleIsland_Text_Sign, MSGBOX_SIGN
	end

BattleIsland_Text_SailBack:
	.string "The boat goes back to the port you\n"
	.string "came from whenever you like.\p"
	.string "Shall we set sail?$"

BattleIsland_Text_AllAboard:
	.string "All aboard!$"

BattleIsland_Text_TakeYourTime:
	.string "Take your time. The STADIUM\n"
	.string "isn't going anywhere!$"

BattleIsland_Text_Sign:
	.string "BATTLE ISLAND\n"
	.string "POKéMON STADIUM straight ahead$"
