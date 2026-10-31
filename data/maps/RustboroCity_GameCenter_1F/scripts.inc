@ Moemon Co-op: the GAME CENTER (RUSTBORO CITY, north-east): 1F reception, where you pay to get in. Its floors are
@ full-color pictures (src/mp/mp_mappic.c); the maps only carry collision (tools/moemon_gamecenter).

RustboroCity_GameCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, RustboroCity_GameCenter_1F_OnTransition
	.byte 0

@ In from the street (the doormats are on the bottom row): admission again
RustboroCity_GameCenter_1F_OnTransition:
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	goto_if_ne VAR_TEMP_1, 14, RustboroCity_GameCenter_1F_OnTransition_End
	setvar VAR_GAME_CENTER_PAID, 0
RustboroCity_GameCenter_1F_OnTransition_End:
	end

RustboroCity_GameCenter_1F_EventScript_Receptionist::
	lock
	faceplayer
	goto_if_eq VAR_GAME_CENTER_PAID, 1, RustboroCity_GameCenter_1F_EventScript_AlreadyPaid
	showmoneybox 0, 0
	msgbox RustboroCity_GameCenter_1F_Text_Welcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_GameCenter_1F_EventScript_Declined
	checkmoney 500
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_GameCenter_1F_EventScript_NoMoney
	removemoney 500
	updatemoneybox
	playse SE_SHOP
	setvar VAR_GAME_CENTER_PAID, 1
	msgbox RustboroCity_GameCenter_1F_Text_Enjoy, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

RustboroCity_GameCenter_1F_EventScript_AlreadyPaid::
	msgbox RustboroCity_GameCenter_1F_Text_AlreadyPaid, MSGBOX_DEFAULT
	release
	end

RustboroCity_GameCenter_1F_EventScript_Declined::
	msgbox RustboroCity_GameCenter_1F_Text_Declined, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

RustboroCity_GameCenter_1F_EventScript_NoMoney::
	msgbox RustboroCity_GameCenter_1F_Text_NoMoney, MSGBOX_DEFAULT
	hidemoneybox
	release
	end

@ The way to the stairs and the elevator, not paid yet
RustboroCity_GameCenter_1F_EventScript_PayFirst::
	lockall
	msgbox RustboroCity_GameCenter_1F_Text_PayFirst, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, RustboroCity_GameCenter_1F_Movement_StepBack
	waitmovement 0
	releaseall
	end

RustboroCity_GameCenter_1F_Movement_StepBack:
	walk_down
	step_end

@ Every floor's elevator (not to the ROOFTOP: the stairs from 3F go up there)
RustboroCity_GameCenter_EventScript_Elevator::
	lockall
	specialvar VAR_RESULT, MP_Region_GameCenterOtherFloor   @ Moemon Kai: JOHTO's / KANTO's GAME CENTER: 2F and 3F
	goto_if_eq VAR_RESULT, 2, RustboroCity_GameCenter_EventScript_ElevatorDownTo2F
	goto_if_eq VAR_RESULT, 3, RustboroCity_GameCenter_EventScript_ElevatorUpTo3F
	goto_if_eq VAR_GAME_CENTER_PAID, 0, RustboroCity_GameCenter_EventScript_ElevatorUnpaid
	msgbox RustboroCity_GameCenter_Text_WhichFloor, MSGBOX_DEFAULT
	multichoice 0, 0, MULTI_GAME_CENTER_FLOORS, FALSE
	switch VAR_RESULT
	case 0, RustboroCity_GameCenter_EventScript_Elevator1F
	case 1, RustboroCity_GameCenter_EventScript_Elevator2F
	case 2, RustboroCity_GameCenter_EventScript_Elevator3F
	closemessage
	releaseall
	end

RustboroCity_GameCenter_EventScript_ElevatorDownTo2F::
	msgbox RustboroCity_GameCenter_Text_DownTo2F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, RustboroCity_GameCenter_EventScript_Elevator2F
	releaseall
	end

RustboroCity_GameCenter_EventScript_ElevatorUpTo3F::
	msgbox RustboroCity_GameCenter_Text_UpTo3F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, RustboroCity_GameCenter_EventScript_Elevator3F
	releaseall
	end

RustboroCity_GameCenter_Text_DownTo2F:
	.string "Go down to 2F, the CARD SHOP?$"

RustboroCity_GameCenter_Text_UpTo3F:
	.string "Go up to 3F, the games floor?$"

RustboroCity_GameCenter_EventScript_ElevatorUnpaid::
	msgbox RustboroCity_GameCenter_1F_Text_PayFirst, MSGBOX_DEFAULT
	releaseall
	end

RustboroCity_GameCenter_EventScript_Elevator1F::
	closemessage
	playse SE_ELEVATOR
	delay 40
	warp MAP_RUSTBORO_CITY_GAME_CENTER_1F, 6, 2
	waitstate
	releaseall
	end

RustboroCity_GameCenter_EventScript_Elevator2F::
	closemessage
	playse SE_ELEVATOR
	delay 40
	warp MAP_RUSTBORO_CITY_GAME_CENTER_2F, 6, 2
	waitstate
	releaseall
	end

RustboroCity_GameCenter_EventScript_Elevator3F::
	closemessage
	playse SE_ELEVATOR
	delay 40
	warp MAP_RUSTBORO_CITY_GAME_CENTER_3F, 6, 2
	waitstate
	releaseall
	end

RustboroCity_GameCenter_1F_EventScript_FloorGuide::
	msgbox RustboroCity_GameCenter_1F_Text_FloorGuide, MSGBOX_SIGN
	end

RustboroCity_GameCenter_1F_EventScript_TicketMachine::
	msgbox RustboroCity_GameCenter_1F_Text_TicketMachine, MSGBOX_SIGN
	end

RustboroCity_GameCenter_1F_EventScript_Man::
	msgbox RustboroCity_GameCenter_1F_Text_Man, MSGBOX_NPC
	end

RustboroCity_GameCenter_1F_EventScript_Twin::
	msgbox RustboroCity_GameCenter_1F_Text_Twin, MSGBOX_NPC
	end

RustboroCity_GameCenter_1F_Text_Welcome:
	.string "Welcome to the GAME CENTER!\p"
	.string "Admission is ¥500, and it's good for\n"
	.string "every floor until you leave.\p"
	.string "Would you like to come in?$"

RustboroCity_GameCenter_1F_Text_Enjoy:
	.string "Thank you! The CARD SHOP is on 2F,\n"
	.string "the games are on 3F.\p"
	.string "The stairs and the elevator are\n"
	.string "right behind me. Have fun!$"

RustboroCity_GameCenter_1F_Text_AlreadyPaid:
	.string "You're all set! Enjoy the GAME CENTER!$"

RustboroCity_GameCenter_1F_Text_Declined:
	.string "Come back whenever you like!$"

RustboroCity_GameCenter_1F_Text_NoMoney:
	.string "Oh, I'm sorry. Admission is ¥500...$"

RustboroCity_GameCenter_1F_Text_PayFirst:
	.string "Excuse me! Please pay the admission\n"
	.string "at the reception first.$"

RustboroCity_GameCenter_Text_WhichFloor:
	.string "Which floor would you like?$"

RustboroCity_GameCenter_1F_Text_FloorGuide:
	.string "GAME CENTER FLOOR GUIDE\p"
	.string "1F: RECEPTION\n"
	.string "2F: CARD SHOP\p"
	.string "3F: GAMES\n"
	.string "ROOFTOP: SKY LOUNGE$"

RustboroCity_GameCenter_1F_Text_TicketMachine:
	.string "It's a ticket machine.\n"
	.string "OUT OF ORDER. Please pay at the\l"
	.string "reception.$"

RustboroCity_GameCenter_1F_Text_Man:
	.string "DEVON built the tallest building in\n"
	.string "RUSTBORO... until this one went up!$"

RustboroCity_GameCenter_1F_Text_Twin:
	.string "Mom said we can play upstairs after\n"
	.string "I finish my homework!$"
