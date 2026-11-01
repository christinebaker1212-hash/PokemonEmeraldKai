Sinnoh_IronRuins_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_IronRuins_OnTransition
	.byte 0

@ REGISTEEL's ruins (Platinum's way: after SINNOH's LEAGUE, with REGIGIGAS along, the seven dots, then the statue)
Sinnoh_IronRuins_OnTransition:
	setvar 0x51E3, 0
	setvar 0x51E4, 0
	setvar 0x51E5, 0
	setvar 0x51E6, 0
	setvar 0x51E7, 0
	setvar 0x51E8, 0
	setvar 0x51E9, 0
	end

Sinnoh_IronRuins_EventScript_Trigger1::
	setvar 0x51E6, 1
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_DotEnd
	playse SE_PIN
	end

Sinnoh_IronRuins_EventScript_Trigger2::
	setvar 0x51E3, 1
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_DotEnd
	playse SE_PIN
	end

Sinnoh_IronRuins_EventScript_Trigger3::
	setvar 0x51E4, 1
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_DotEnd
	playse SE_PIN
	end

Sinnoh_IronRuins_EventScript_Trigger4::
	setvar 0x51E5, 1
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_DotEnd
	playse SE_PIN
	end

Sinnoh_IronRuins_EventScript_Trigger5::
	setvar 0x51E7, 1
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_DotEnd
	playse SE_PIN
	end

Sinnoh_IronRuins_EventScript_Trigger6::
	setvar 0x51E8, 1
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_DotEnd
	playse SE_PIN
	end

Sinnoh_IronRuins_EventScript_Trigger7::
	setvar 0x51E9, 1
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_DotEnd
	playse SE_PIN
	end

Sinnoh_IronRuins_EventScript_DotEnd::
	end
Sinnoh_IronRuins_EventScript_Bg1::
Sinnoh_IronRuins_EventScript_Bg2::
Sinnoh_IronRuins_EventScript_Bg3::
	lockall
	goto_if_set 0x3D0E, Sinnoh_IronRuins_EventScript_Silent
	specialvar VAR_RESULT, Sinnoh_LeagueDone
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_IronRuins_EventScript_NotYet
	specialvar VAR_RESULT, Sinnoh_PartyHasRegigigas
	goto_if_eq VAR_RESULT, FALSE, Sinnoh_IronRuins_EventScript_Waiting
	goto_if_ne 0x51E3, 1, Sinnoh_IronRuins_EventScript_Humming
	goto_if_ne 0x51E4, 1, Sinnoh_IronRuins_EventScript_Humming
	goto_if_ne 0x51E5, 1, Sinnoh_IronRuins_EventScript_Humming
	goto_if_ne 0x51E6, 1, Sinnoh_IronRuins_EventScript_Humming
	goto_if_ne 0x51E7, 1, Sinnoh_IronRuins_EventScript_Humming
	goto_if_ne 0x51E8, 1, Sinnoh_IronRuins_EventScript_Humming
	goto_if_ne 0x51E9, 1, Sinnoh_IronRuins_EventScript_Humming
	msgbox Sinnoh_IronRuins_Text_Wakes, MSGBOX_DEFAULT
	closemessage
	playmoncry SPECIES_REGISTEEL, CRY_MODE_ENCOUNTER
	waitmoncry
	setwildbattle SPECIES_REGISTEEL, 30
	special BattleSetup_StartLegendaryBattle
	waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, Sinnoh_IronRuins_EventScript_End
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, Sinnoh_IronRuins_EventScript_End
	setflag 0x3D0E
Sinnoh_IronRuins_EventScript_End::
	releaseall
	end

Sinnoh_IronRuins_EventScript_Silent::
	msgbox Sinnoh_IronRuins_Text_Silent, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_IronRuins_EventScript_NotYet::
	msgbox Sinnoh_IronRuins_Text_NotYet, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_IronRuins_EventScript_Waiting::
	msgbox Sinnoh_IronRuins_Text_Waiting, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_IronRuins_EventScript_Humming::
	msgbox Sinnoh_IronRuins_Text_Humming, MSGBOX_DEFAULT
	releaseall
	end

Sinnoh_IronRuins_Text_NotYet:
	.string "An ancient statue of a POKéMON.\n"
	.string "It seems to be sizing you up.$"

Sinnoh_IronRuins_Text_Waiting:
	.string "An ancient statue of a POKéMON.\p"
	.string "It looks as if it's waiting for its\n"
	.string "colossal master to come home.$"

Sinnoh_IronRuins_Text_Humming:
	.string "The statue hums faintly.\p"
	.string "The dots on the floor around it\n"
	.string "seem to be part of something…$"

Sinnoh_IronRuins_Text_Wakes:
	.string "REGIGIGAS stirs! The statue's eyes\n"
	.string "light up in answer!$"

Sinnoh_IronRuins_Text_Silent:
	.string "The statue has gone silent.$"

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
@ --- end of SINNOH words ---
