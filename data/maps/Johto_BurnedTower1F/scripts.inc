Johto_BurnedTower1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Johto_BurnedTower1F_EventScript_83C8397
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_BurnedTower1F_OnFrame_83C83AD
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_BurnedTower1F_EventScript_83C8391
	.byte 0

Johto_BurnedTower1F_OnFrame_83C83AD:
	map_script_2 VAR_QZ_0x500B, 1, Johto_BurnedTower1F_EventScript_83C83B7
	.2byte 0

Johto_BurnedTower1F_EventScript_83C8391::
	call Quetzal_EventScript_840EA62
	end

Johto_BurnedTower1F_EventScript_83C8397::
	goto_if_ge VAR_QZ_0x500B, 4, Quetzal_EventScript_83C83A3
	end

Johto_BurnedTower1F_EventScript_83C83B7::
	lock
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 1, Johto_BurnedTower1F_Movement_83C83E9
	waitmovement 0
	msgbox Johto_BurnedTower1F_Text_83C83EF, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_BurnedTower1F_Movement_83C83EC
	waitmovement 0
	setvar VAR_QZ_0x500B, 2
	release
	end

@ Johto_BurnedTower1F object 6 (local id 6, gfx 0x3A9E) at 23,18
Johto_BurnedTower1F_EventScript_83C84C6::
	setorcopyvar VAR_0x8000, ITEM_HP_UP
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_BurnedTower1F object 7 (local id 7, gfx 0x3A9E) at 3,7
Johto_BurnedTower1F_EventScript_83C84D3::
	setorcopyvar VAR_0x8000, ITEM_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_BurnedTower1F object 1 (local id 1, gfx 0x3E92) at 15,19
Johto_BurnedTower1F_EventScript_83C84E0::
	lock
	faceplayer
	msgbox Johto_BurnedTower1F_Text_83C84FA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BurnedTower1F object 2 (local id 2, gfx 0x3EA3) at 9,20
Johto_BurnedTower1F_EventScript_83C84ED::
	lock
	faceplayer
	msgbox Johto_BurnedTower1F_Text_83C856B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_BurnedTower1F trigger at 15,12 (VAR_QZ_0x500B == 2)
Johto_BurnedTower1F_EventScript_83C861F::
	lock
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	applymovement 3, Quetzal_Movement_836083D
	waitmovement 0
	waitmovement 0
	msgbox Johto_BurnedTower1F_Text_83C86B6, MSGBOX_DEFAULT
	call Johto_BurnedTower1F_EventScript_83C86AB
	playbgm MUS_QZ_649, 1
	msgbox Johto_BurnedTower1F_Text_83C87DE, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	playse SE_CLICK
	setmetatile 0x10, 0xC, 0x37E, 1
	special DrawWholeMapView
	waitse
	delay 0x3C
	applymovement LOCALID_PLAYER, Johto_BurnedTower1F_Movement_83C86A9
	waitmovement 0
	playse SE_FALL
	delay 0x3C
	msgbox Johto_BurnedTower1F_Text_83C886B, MSGBOX_AUTOCLOSE
	waitmessage
	setvar VAR_QZ_0x500B, 3
	warphole MAP_JOHTO_BURNED_TOWER_B1F
	waitstate
	end

Johto_BurnedTower1F_EventScript_83C86AB::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4422, 0, Johto_BurnedTower1F_Text_83C8792
	return

@ Johto_BurnedTower1F object 4 (local id 4, gfx 0x3E94) at 14,4
Johto_BurnedTower1F_EventScript_83C88B9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4814, 0, Johto_BurnedTower1F_Text_83C88E7, Johto_BurnedTower1F_Text_83C8936
	msgbox Johto_BurnedTower1F_Text_83C8957, MSGBOX_AUTOCLOSE
	end

@ Johto_BurnedTower1F object 5 (local id 5, gfx 0x3E8A) at 3,15
Johto_BurnedTower1F_EventScript_83C88D0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4813, 0, Johto_BurnedTower1F_Text_83C89C7, Johto_BurnedTower1F_Text_83C8A0F
	msgbox Johto_BurnedTower1F_Text_83C8A21, MSGBOX_AUTOCLOSE
	end

Johto_BurnedTower1F_Movement_83C83E9:
	walk_down
	walk_down
	step_end

Johto_BurnedTower1F_Movement_83C83EC:
	walk_up
	walk_up
	step_end

Johto_BurnedTower1F_Movement_83C86A9:
	set_invisible
	step_end

Johto_BurnedTower1F_Text_83C83EF::
	.string "EUSINE: My name’s EUSINE.\p"
	.string "I’m on the trail of a POKéMON\n"
	.string "called SUICUNE.\p"
	.string "And you are...?\p"
	.string "{PLAYER}? Glad to meet you!\p"
	.string "I heard SUICUNE might be in\n"
	.string "this BURNED TOWER, so I came\l"
	.string "to check it out.\p"
	.string "But where exactly could it be?$"

Johto_BurnedTower1F_Text_83C84FA::
	.string "EUSINE: I heard SUICUNE might\n"
	.string "be in this BURNED TOWER, so I\l"
	.string "came to check it out.\p"
	.string "But where exactly could it be?$"

Johto_BurnedTower1F_Text_83C856B::
	.string "MORTY: ECRUTEAK’s GYM LEADER\n"
	.string "has to study what are said to\l"
	.string "be the legendary POKéMON--\l"
	.string "SUICUNE, ENTEI and RAIKOU.\p"
	.string "EUSINE is here, so I’ve\n"
	.string "decided to investigate the\l"
	.string "TOWER with him.$"

Johto_BurnedTower1F_Text_83C86B6::
	.string "..............................\p"
	.string "...Oh, it’s you.\p"
	.string "I came looking for some\n"
	.string "legendary POKéMON they say\l"
	.string "roost here.\p"
	.string "But there’s nothing here!\p"
	.string "Nothing, after all the effort\n"
	.string "of coming to this dump?\l"
	.string "No way!\p"
	.string "It’s all your fault!$"

Johto_BurnedTower1F_Text_83C8792::
	.string "...Humph!\p"
	.string "I only lost because my\n"
	.string "POKéMON are incredibly weak.\p"
	.string "How pathetic.$"

Johto_BurnedTower1F_Text_83C87DE::
	.string "...Aw, whatever.\p"
	.string "You’d never be able to catch\n"
	.string "a legendary POKéMON anyway.$"

Johto_BurnedTower1F_Text_83C886B::
	.string "Humph!\p"
	.string "What are you doing falling\n"
	.string "into a hole? Some genius!\p"
	.string "Serves you right!$"

Johto_BurnedTower1F_Text_83C88E7::
	.string "Don’t worry!\p"
	.string "We fire-breathers know the true\n"
	.string "power of fire better than anyone!$"

Johto_BurnedTower1F_Text_83C8936::
	.string "Thanks for such a heated battle!$"

Johto_BurnedTower1F_Text_83C8957::
	.string "The reason this tower’s burned is\n"
	.string "that it was hit by lightning.\p"
	.string "The fire I’m breathing is tiny\n"
	.string "in comparison...$"

Johto_BurnedTower1F_Text_83C89C7::
	.string "I’m practicing my stealing in the\n"
	.string "BURNED TOWER! Don’t try\l"
	.string "this at home!$"

Johto_BurnedTower1F_Text_83C8A0F::
	.string "Pfffffffffffffff.$"

Johto_BurnedTower1F_Text_83C8A21::
	.string "Maybe a life of crime isn’t for\n"
	.string "me...$"
