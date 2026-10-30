Kanto_OneIsland_Harbor_MapScripts::
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_OneIsland_Harbor_OnWarp_8421285
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_OneIsland_Harbor_OnFrame_8421294
	.byte 0

Kanto_OneIsland_Harbor_OnWarp_8421285:
	map_script_2 VAR_QZ_0x5116, 1, Kanto_OneIsland_Harbor_EventScript_842128F
	.2byte 0

Kanto_OneIsland_Harbor_OnFrame_8421294:
	map_script_2 VAR_QZ_0x5116, 1, Kanto_OneIsland_Harbor_EventScript_842129E
	.2byte 0

Kanto_OneIsland_Harbor_EventScript_842128F::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_OneIsland_Harbor_EventScript_842129E::
	lockall
	textcolor 0
	applymovement LOCALID_PLAYER, Kanto_OneIsland_Harbor_Movement_84212BB
	waitmovement 0
	setvar VAR_QZ_0x5116, 2
	warp MAP_KANTO_ONE_ISLAND, 255, 12, 18
	waitstate
	releaseall
	end

@ Kanto_OneIsland_Harbor object 2 (local id 2, gfx 0x3F41) at 8,6
Kanto_OneIsland_Harbor_EventScript_84212BF::
	lock
	faceplayer
	message Quetzal_Text_8431905
	waitmessage
	setvar VAR_0x8004, 1
	goto Quetzal_EventScript_84593E7

Kanto_OneIsland_Harbor_Movement_84212BB:
	walk_up
	walk_up
	walk_up
	step_end
