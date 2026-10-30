Johto_DarkCaveBlackthornEntrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_DarkCaveBlackthornEntrance_EventScript_83CBD37
	.byte 0

Johto_DarkCaveBlackthornEntrance_EventScript_83CBD37::
	call Quetzal_EventScript_840EA62
	end

@ Johto_DarkCaveBlackthornEntrance object 1 (local id 1, gfx 0x3A9E) at 13,33
Johto_DarkCaveBlackthornEntrance_EventScript_83CBD3D::
	setorcopyvar VAR_0x8000, ITEM_BLACK_FLUTE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_DarkCaveBlackthornEntrance object 2 (local id 2, gfx 0x3A9E) at 31,31
Johto_DarkCaveBlackthornEntrance_EventScript_83CBD4A::
	setorcopyvar VAR_0x8000, ITEM_STAR_PIECE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_DarkCaveBlackthornEntrance object 3 (local id 3, gfx 0x3EB7) at 8,5
Johto_DarkCaveBlackthornEntrance_EventScript_83CBD57::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x105B, Johto_DarkCaveBlackthornEntrance_EventScript_83CBD7C
	msgbox Johto_DarkCaveBlackthornEntrance_Text_83CBD87, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_BLACK_GLASSES
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setflag FLAG_QZ_0x105B
	closemessage
	release
	end

Johto_DarkCaveBlackthornEntrance_EventScript_83CBD7C::
	msgbox Johto_DarkCaveBlackthornEntrance_Text_83CBE55, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_DarkCaveBlackthornEntrance object 6 (local id 6, gfx 0x2EB) at 32,32
Johto_DarkCaveBlackthornEntrance_EventScript_840F19E::
	lock
	faceplayer
	special QzJohtoStaticEncounterSetup
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_840EBAE
	waitmoncry
	special CreateEnemyEventMon
	setflag FLAG_QZ_0xAD9
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0xAD9
	special QzJohtoStaticEncounterAfterBattle
	goto_if_eq VAR_RESULT, 0, Johto_DarkCaveBlackthornEntrance_EventScript_840F1E5
	removeobject VAR_LAST_TALKED
	msgbox Johto_DarkCaveBlackthornEntrance_Text_840F336, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	special QzEndDreamEvent
	warp MAP_JOHTO_PLAYERS_HOUSE2F, 255, 2, 6
	waitstate
	end

Johto_DarkCaveBlackthornEntrance_EventScript_840F1E5::
	release
	end

Johto_DarkCaveBlackthornEntrance_Text_83CBD87::
	.string "Whoa! You startled me there!\p"
	.string "I had my BLACK-GLASSES on, so I\n"
	.string "didn’t notice you at all.\p"
	.string "What am I doing here?\p"
	.string "Hey, don’t you worry about it.\p"
	.string "I’ll give you a pair of BLACK-\n"
	.string "GLASSES, so forget you saw me,\n"
	.string "OK?$"

Johto_DarkCaveBlackthornEntrance_Text_83CBE55::
	.string "BLACK-GLASSES ups the power of\n"
	.string "DARK-type moves.$"

Johto_DarkCaveBlackthornEntrance_Text_840F336::
	.string "The nightmare begins to crumble...$"
