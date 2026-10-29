@ Moemon Co-op: the BATTLE SIMULATOR, a counter in each BATTLE TENT's lobby. Every opponent is a digital copy of another
@ player from the last server this game played on: their look and their top 6 Pokemon, everyone at level 50
@ (src/mp/mp_sim.c). A streak until a loss; prize money for each win, an item for a new best at 3 / 5 / 7 / 10 / 15 / 20.
EventScript_BattleSim::
	lock
	faceplayer
	special MP_Sim_Intro
	goto_if_eq VAR_RESULT, 0, EventScript_BattleSim_NoData
	msgbox BattleSim_Text_Welcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_BattleSim_Bye
	special MP_Sim_CanEnter
	goto_if_eq VAR_RESULT, FALSE, EventScript_BattleSim_NoMons
	msgbox BattleSim_Text_Rules, MSGBOX_DEFAULT
EventScript_BattleSim_Next::
	special MP_Sim_Next
	goto_if_eq VAR_RESULT, FALSE, EventScript_BattleSim_NoData
	msgbox BattleSim_Text_Loading, MSGBOX_DEFAULT
	closemessage
	special MP_Sim_Battle
	waitstate
	goto_if_eq VAR_RESULT, FALSE, EventScript_BattleSim_Lost
	special MP_Sim_Won
	goto_if_ne VAR_RESULT, ITEM_NONE, EventScript_BattleSim_WonItem
	msgbox BattleSim_Text_Won, MSGBOX_DEFAULT
	goto EventScript_BattleSim_AskMore
EventScript_BattleSim_WonItem::
	msgbox BattleSim_Text_WonItem, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
EventScript_BattleSim_AskMore::
	msgbox BattleSim_Text_More, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_BattleSim_Next
	special MP_Sim_End
	msgbox BattleSim_Text_Stopped, MSGBOX_DEFAULT
	release
	end

EventScript_BattleSim_Lost::
	special MP_Sim_End
	msgbox BattleSim_Text_Lost, MSGBOX_DEFAULT
	release
	end

EventScript_BattleSim_NoData::
	msgbox BattleSim_Text_NoData, MSGBOX_DEFAULT
	release
	end

EventScript_BattleSim_NoMons::
	msgbox BattleSim_Text_NoMons, MSGBOX_DEFAULT
	release
	end

EventScript_BattleSim_Bye::
	msgbox BattleSim_Text_Bye, MSGBOX_DEFAULT
	release
	end

BattleSim_Text_Welcome:
	.string "Welcome to the BATTLE SIMULATOR!\p"
	.string "I have battle data for {STR_VAR_1}\n"
	.string "trainer(s) you met online.\p"
	.string "Their best MOéMON wait for you\n"
	.string "as data, all at level 50.\p"
	.string "Your best streak: {STR_VAR_2}.\n"
	.string "Will you take the challenge?$"

BattleSim_Text_Rules:
	.string "Your MOéMON battle at level 50\n"
	.string "and come back just as they were.\p"
	.string "Win, and the next one loads.\n"
	.string "One loss ends your streak!$"

BattleSim_Text_Loading:
	.string "Battle {STR_VAR_2}!\n"
	.string "Loading {STR_VAR_1}'s battle data…$"

BattleSim_Text_Won:
	.string "Simulation cleared! Streak: {STR_VAR_1}.\n"
	.string "Here's ¥{STR_VAR_2} in prize money.$"

BattleSim_Text_WonItem:
	.string "Simulation cleared! Streak: {STR_VAR_1}.\n"
	.string "Here's ¥{STR_VAR_2} in prize money.\p"
	.string "And for a new best streak,\n"
	.string "please take this {STR_VAR_3}!$"

BattleSim_Text_More:
	.string "Shall I load the next simulation?$"

BattleSim_Text_Stopped:
	.string "You stopped at a streak of {STR_VAR_1}.\n"
	.string "Your best is {STR_VAR_2}. Come again!$"

BattleSim_Text_Lost:
	.string "The simulation is over.\n"
	.string "Your streak ended at {STR_VAR_1}.\p"
	.string "Your best is {STR_VAR_2}.\n"
	.string "Please challenge us again!$"

BattleSim_Text_NoData:
	.string "Welcome to the BATTLE SIMULATOR!\p"
	.string "I'm sorry, but I don't have any\n"
	.string "battle data to load yet.\p"
	.string "The data comes from the trainers\n"
	.string "you play with online.\p"
	.string "Please play with others, then\n"
	.string "come back and see me!$"

BattleSim_Text_NoMons:
	.string "Oh! You'll need a MOéMON that\n"
	.string "can battle to take the challenge.$"

BattleSim_Text_Bye:
	.string "Please come again any time!$"
