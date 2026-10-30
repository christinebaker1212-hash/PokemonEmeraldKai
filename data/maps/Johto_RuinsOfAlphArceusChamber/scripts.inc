Johto_RuinsOfAlphArceusChamber_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_RuinsOfAlphArceusChamber object 2 (local id 2, gfx 0x3EC1) at 15,12
Johto_RuinsOfAlphArceusChamber_EventScript_8401CF3::
	lock
	special QzGiovanniStoryItemAvailable
	goto_if_eq VAR_RESULT, 0, Johto_RuinsOfAlphArceusChamber_EventScript_8401D16
	msgbox Johto_RuinsOfAlphArceusChamber_Text_8401E09, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	special QzGiovanniStoryItemTaken
	removeobject 2
	release
	end

Johto_RuinsOfAlphArceusChamber_EventScript_8401D16::
	msgbox Quetzal_Text_8401EDB, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphArceusChamber object 1 (local id 1, gfx 0x2ED) at 15,12
Johto_RuinsOfAlphArceusChamber_EventScript_840EB14::
	lock
	faceplayer
	special QzJohtoStaticEncounterSetup2
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840EBAE
	waitmoncry
	special CreateEnemyEventMon
	special QzQzEnemyMonEventFixup
	setvar VAR_0x8000, 1
	setvar VAR_0x8001, 6
	setvar VAR_0x8002, 6
	setvar VAR_0x8003, 6
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 6
	setvar VAR_0x8006, 6
	setvar VAR_0x8007, 6
	special QzSetEnemyMonOverrides
	setflag FLAG_QZ_0x28
	setflag FLAG_QZ_0xAD9
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0xAD9
	clearflag FLAG_QZ_0x28
	special QzJohtoStaticEncounterAfterBattle
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_840EBA9
	release
	end

Johto_RuinsOfAlphArceusChamber_Text_8401E09::
	.string "The ORIGIN EGG answers\n"
	.string "your six Mythical Pokémon.$"
