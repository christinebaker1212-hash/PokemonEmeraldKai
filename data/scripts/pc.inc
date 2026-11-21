EventScript_PC::
	lockall
	setvar VAR_0x8004, PC_LOCATION_OTHER
	special DoPCTurnOnEffect
	playse SE_PC_ON
	msgbox Text_BootUpPC, MSGBOX_DEFAULT
	goto EventScript_PCMainMenu
	end

EventScript_PCMainMenu::
	message gText_WhichPCShouldBeAccessed
	waitmessage
	special ScriptMenu_CreatePCMultichoice
	goto EventScript_AccessPC
	end

EventScript_AccessPC::
	specialvar VAR_RESULT, MP_PcMenuChoice   @ Moemon Kai: the row picked as a choice (the GLOBAL PC adds one)
	switch VAR_RESULT
	case 0, EventScript_AccessPokemonStorage
	case 1, EventScript_AccessPlayersPC
	case 2, EventScript_AccessHallOfFame
	case 3, EventScript_TurnOffPC
	case 4, EventScript_AccessGlobalPC
	case 5, EventScript_AccessVm2
	case MULTI_B_PRESSED, EventScript_TurnOffPC
	end

EventScript_AccessPlayersPC::
	playse SE_PC_LOGIN
	msgbox gText_AccessedPlayersPC, MSGBOX_DEFAULT
	special PlayerPC
	goto EventScript_PCMainMenu
	end

EventScript_AccessPokemonStorage::
	playse SE_PC_LOGIN
	call_if_unset FLAG_SYS_PC_LANETTE, EventScript_AccessSomeonesPC
	call_if_set FLAG_SYS_PC_LANETTE, EventScript_AccessLanettesPC
	msgbox gText_StorageSystemOpened, MSGBOX_DEFAULT
	special ShowPokemonStorageSystemPC
	goto EventScript_PCMainMenu
	end

EventScript_AccessSomeonesPC::
	msgbox gText_AccessedSomeonesPC, MSGBOX_DEFAULT
	return

EventScript_AccessLanettesPC::
	msgbox gText_AccessedLanettesPC, MSGBOX_DEFAULT
	return

EventScript_TurnOffPC::
	setvar VAR_0x8004, PC_LOCATION_OTHER
	playse SE_PC_OFF
	special DoPCTurnOffEffect
	specialvar VAR_RESULT, Orre_VmHolding   @ Moemon Kai: ORRE's PC, opened by Colosseum's script (orre.inc)
	goto_if_eq VAR_RESULT, TRUE, Orre_EventScript_VmHoldLoop
	releaseall
	end

EventScript_AccessHallOfFame::
	goto_if_unset FLAG_SYS_GAME_CLEAR, EventScript_TurnOffPC
	playse SE_PC_LOGIN
	special AccessHallOfFamePC
	goto EventScript_AccessPC
	end

@ Moemon Kai: the GLOBAL PC (docs/REGIONS_PLAN.md): one set of boxes for every region, open in a region whose Elite
@ Four the player has beaten
EventScript_AccessGlobalPC::
	playse SE_PC_LOGIN
	specialvar VAR_RESULT, MP_GlobalPcLinked
	goto_if_eq VAR_RESULT, 0, EventScript_GlobalPcNoLink
	msgbox gText_GlobalPcOpened, MSGBOX_DEFAULT
	special MP_ShowGlobalPC
	goto EventScript_PCMainMenu
	end

EventScript_GlobalPcNoLink::
	msgbox gText_GlobalPcNoLink, MSGBOX_DEFAULT
	goto EventScript_PCMainMenu
	end

@ Moemon Kai: the VMU pet's VM2 (src/mp/mp_vm2.c; docs/vmu_pet/kai_vm2_mockups.png)
EventScript_AccessVm2::
	playse SE_PC_LOGIN
	message Text_Vm2Connecting
	waitmessage
	special MP_Vm2_StartRead
	call EventScript_Vm2Wait
	specialvar VAR_RESULT, MP_Vm2_Result
	switch VAR_RESULT
	case 0, EventScript_Vm2NoAnswer
	case 1, EventScript_Vm2NoPet
	case 2, EventScript_Vm2Empty
	case 3, EventScript_Vm2Egg
	case 4, EventScript_Vm2Mon
	end

EventScript_Vm2Wait::
	delay 4
	specialvar VAR_RESULT, MP_Vm2_Busy
	goto_if_eq VAR_RESULT, TRUE, EventScript_Vm2Wait
	return

EventScript_Vm2NoAnswer::
	msgbox Text_Vm2NoAnswer, MSGBOX_DEFAULT
	special MP_Vm2_Close
	goto EventScript_PCMainMenu
	end

EventScript_Vm2NoPet::
	msgbox Text_Vm2NoPet, MSGBOX_DEFAULT
	special MP_Vm2_Close
	goto EventScript_PCMainMenu
	end

EventScript_Vm2Empty::
	msgbox Text_Vm2Empty, MSGBOX_DEFAULT
	goto EventScript_Vm2Box
	end

EventScript_Vm2Egg::
	msgbox Text_Vm2Egg, MSGBOX_DEFAULT
	goto EventScript_Vm2Box
	end

EventScript_Vm2Mon::
	msgbox Text_Vm2Mon, MSGBOX_DEFAULT
	goto EventScript_Vm2Box
	end

EventScript_Vm2Box::
	special MP_ShowVm2Box
	call EventScript_Vm2Wait
	specialvar VAR_RESULT, MP_Vm2_Returned
	call_if_ne VAR_RESULT, 0, EventScript_Vm2Returned
	specialvar VAR_RESULT, MP_Vm2_Written
	goto_if_eq VAR_RESULT, FALSE, EventScript_Vm2NotWritten
	msgbox Text_Vm2Disconnected, MSGBOX_DEFAULT
	goto EventScript_PCMainMenu
	end

EventScript_Vm2Returned::
	msgbox Text_Vm2Returned, MSGBOX_DEFAULT
	return

EventScript_Vm2NotWritten::
	msgbox Text_Vm2NotWritten, MSGBOX_DEFAULT
	goto EventScript_PCMainMenu
	end

Text_Vm2Connecting:
	.string "Connecting to the VM2…$"

Text_Vm2NoAnswer:
	.string "The VM2 didn't answer.\n"
	.string "Is it in GUI-mode?$"

Text_Vm2NoPet:
	.string "The VM2 has no MOéMON VMU PET\n"
	.string "on it.$"

Text_Vm2Empty:
	.string "Connected to the VM2.\n"
	.string "It holds no MOéMON.$"

Text_Vm2Egg:
	.string "Connected to the VM2.\n"
	.string "It holds a MOéMON EGG.$"

Text_Vm2Mon:
	.string "Connected to the VM2.\n"
	.string "It holds {STR_VAR_1}.$"

Text_Vm2Returned:
	.string "{STR_VAR_1} POKéMON went back to your PC:\n"
	.string "only an EGG goes on the VMU's screen.$"

Text_Vm2Disconnected:
	.string "The VM2 was disconnected.\n"
	.string "It’s safe to unplug it.$"

Text_Vm2NotWritten:
	.string "The VM2 couldn't be written to.\n"
	.string "Check its cable and GUI-mode.$"
