Johto_EmbeddedTower_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_EmbeddedTower object 4 (local id 4, gfx 0x3A9F) at 14,9
Johto_EmbeddedTower_EventScript_840EABD::
	lockall
	specialvar VAR_RESULT, QzIsSaveFlag2F4Bit4Set
	goto_if_eq VAR_RESULT, 1, Johto_EmbeddedTower_EventScript_840EAD8
	msgbox Quetzal_Text_840F0C0, MSGBOX_DEFAULT
	releaseall
	end

Johto_EmbeddedTower_EventScript_840EAD8::
	lockall
	special QzJohtoStaticEncounterSetupTower
	goto_if_eq VAR_RESULT, 0, Johto_EmbeddedTower_EventScript_840EB0A
	waitmoncry
	special CreateEnemyEventMon
	setflag FLAG_QZ_0xAD9
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0xAD9
	special QzJohtoStaticEncounterAfterBattle
	goto_if_eq VAR_RESULT, 1, Johto_EmbeddedTower_EventScript_840EB05
	releaseall
	end

Johto_EmbeddedTower_EventScript_840EB05::
	removeobject 4
	releaseall
	end

Johto_EmbeddedTower_EventScript_840EB0A::
	msgbox Quetzal_Text_840F0C0, MSGBOX_DEFAULT
	releaseall
	end
