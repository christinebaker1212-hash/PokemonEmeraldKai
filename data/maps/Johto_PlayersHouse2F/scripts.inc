Johto_PlayersHouse2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_PlayersHouse2F_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Johto_PlayersHouse2F_OnWarp
	.byte 0

@ Moemon Kai: MY ROOM as Crystal's, decorated on the bedroom's logic (decoration.c / secret_base.c; its own
@ decorations in JOHTO's vars: mp_region.c MP_RoomDecorations): the dolls and cushions shown on entering, the PC's
@ DECORATION to place them
Johto_PlayersHouse2F_OnTransition:
	call SecretBase_EventScript_SetDecorationFlags
	setvar VAR_SECRET_BASE_INITIALIZED, 0
	end

Johto_PlayersHouse2F_OnWarp:
	map_script_2 VAR_SECRET_BASE_INITIALIZED, 0, SecretBase_EventScript_InitDecorations
	.2byte 0

Johto_PlayersHouse2F_EventScript_PC::
	lockall
	playse SE_PC_ON
	msgbox gText_PlayerHouseBootPC, MSGBOX_DEFAULT
	special BedroomPC
	releaseall
	end

Johto_PlayersHouse2F_EventScript_TurnOffPC::
	playse SE_PC_OFF
	releaseall
	end

@ Johto_PlayersHouse2F sign / bg event at 1,1
Johto_PlayersHouse2F_EventScript_82DE10F::
	end

Johto_PlayersHouse2F_EventScript_83811BB::
	fadescreen 1
	special StartWallClock
	waitstate
	return

@ Johto_PlayersHouse2F trigger at 9,3 (VAR_QZ_0x5003 == 0)
Johto_PlayersHouse2F_EventScript_83E3EAE::
	@ Moemon Kai: no "The clock is stopped" scene: the game clock is the PC's and Crystal sets it before the
	@ game starts (docs/REGIONS_AUDIT.md 134); the story still moves on here
	setvar VAR_QZ_0x5003, 1
	setflag FLAG_QZ_0x51
	end

@ Johto_PlayersHouse2F trigger at 10,3 (VAR_QZ_0x5003 == 0)
Johto_PlayersHouse2F_EventScript_83E3EE8::
	@ Moemon Kai: no "The clock is stopped" scene: the game clock is the PC's and Crystal sets it before the
	@ game starts (docs/REGIONS_AUDIT.md 134); the story still moves on here
	setvar VAR_QZ_0x5003, 1
	setflag FLAG_QZ_0x51
	end

@ Johto_PlayersHouse2F sign / bg event at 4,2
Johto_PlayersHouse2F_EventScript_83E3F29::
	msgbox Johto_PlayersHouse2F_Text_83E3F32, MSGBOX_SIGN
	end

@ Johto_PlayersHouse2F sign / bg event at 1,6
Johto_PlayersHouse2F_EventScript_840F0C4::
	lockall
	special QzDreamEventEligibility
	goto_if_eq VAR_RESULT, 1, Johto_PlayersHouse2F_EventScript_840F103
	goto_if_eq VAR_RESULT, 2, Johto_PlayersHouse2F_EventScript_840F11B
	msgbox Johto_PlayersHouse2F_Text_840F1E7, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_PlayersHouse2F_EventScript_840F162
	closemessage
	fadescreen 1
	special QzHealPlayerPartyEx
	fadescreen 0
	msgbox Johto_PlayersHouse2F_Text_840F218, MSGBOX_DEFAULT
	releaseall
	end

Johto_PlayersHouse2F_EventScript_840F103::
	msgbox Johto_PlayersHouse2F_Text_840F22B, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	special QzStartDreamEvent
	warp MAP_JOHTO_PLAYERS_HOUSE2F, 255, 2, 6
	waitstate
	end

Johto_PlayersHouse2F_EventScript_840F11B::
	msgbox Johto_PlayersHouse2F_Text_840F27C, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_PlayersHouse2F_EventScript_840F162
	closemessage
	fadescreen 1
	special QzStartDreamEvent
	warp MAP_JOHTO_PLAYERS_HOUSE2F, 255, 2, 6
	waitstate
	end

Johto_PlayersHouse2F_EventScript_840F162::
	closemessage
	releaseall
	end

Johto_PlayersHouse2F_Movement_83E3F22:
	walk_right
	walk_up
	delay_8
	step_end

Johto_PlayersHouse2F_Movement_83E3F26:
	walk_up
	delay_8
	step_end

Johto_PlayersHouse2F_Text_82DE143::
	.string "The clock is stopped…\p"
	.string "Better set it and start it!$"

Johto_PlayersHouse2F_Text_83E3F32::
	.string "{PLAYER} played with the NES.\p"
	.string "…Okay!\n"
	.string "It’s time to go!$"

Johto_PlayersHouse2F_Text_840F1E7::
	.string "Your bed is neatly made.\p"
	.string "Would you like to rest?$"

Johto_PlayersHouse2F_Text_840F218::
	.string "You rested deeply.$"

Johto_PlayersHouse2F_Text_840F22B::
	.string "A shadow smothers your dreams...\p"
	.string "When you open your eyes,\n"
	.string "something feels wrong.$"

Johto_PlayersHouse2F_Text_840F27C::
	.string "The memory of that nightmare\n"
	.string "still lingers.\p"
	.string "Would you like to sleep again?$"
