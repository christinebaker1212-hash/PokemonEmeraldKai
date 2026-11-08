Orre_OutskirtStand_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Orre_OutskirtStand_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Orre_OutskirtStand_OnFrame
	.byte 0

@ The first landing without the SNAG MACHINE plays Colosseum's opening (src/orre/orre_opening.c): Kai's glue, no words
Orre_OutskirtStand_OnTransition:
	special Orre_OpeningNeeded
	copyvar VAR_TEMP_1, VAR_RESULT
	end

Orre_OutskirtStand_OnFrame:
	map_script_2 VAR_TEMP_1, 1, Orre_EventScript_Opening
	.2byte 0

Orre_EventScript_Opening::
	lockall
	special Orre_Opening
	waitstate
	setvar VAR_TEMP_1, 0
	releaseall
	end
