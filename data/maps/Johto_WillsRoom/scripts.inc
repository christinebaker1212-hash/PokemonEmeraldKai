Johto_WillsRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_WillsRoom_OnFrame_83E89AE
	map_script MAP_SCRIPT_ON_RESUME, Johto_WillsRoom_EventScript_83E89A5
	map_script MAP_SCRIPT_ON_LOAD, Johto_WillsRoom_EventScript_83E893C
	.byte 0

Johto_WillsRoom_OnFrame_83E89AE:
	map_script_2 VAR_TEMP_0, 0, Quetzal_EventScript_83E89B8
	.2byte 0

Johto_WillsRoom_EventScript_83E893C::
	call_if_ge VAR_QZ_0x502A, 2, Quetzal_EventScript_83E8958
	goto Quetzal_EventScript_83E894C

Johto_WillsRoom_EventScript_83E89A5::
	setvar VAR_0x8004, 0
	special QzDoPokemonLeagueLightingEffect
	end

@ Johto_WillsRoom object 1 (local id 1, gfx 0x3EBD) at 6,6
Johto_WillsRoom_EventScript_83E8A2B::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x502A, 2, Johto_WillsRoom_EventScript_83E8AFF
	goto_if_set FLAG_QZ_0x11CA, Johto_WillsRoom_EventScript_83E8A7D
	msgbox Johto_WillsRoom_Text_83E8B0A, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_WillsRoom_EventScript_83E8A6E
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4832, 0, Johto_WillsRoom_Text_83E8C0A
	goto Johto_WillsRoom_EventScript_83E8AB9

Johto_WillsRoom_EventScript_83E8A6E::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4977, 0, Johto_WillsRoom_Text_83E8C0A
	goto Johto_WillsRoom_EventScript_83E8AB9

Johto_WillsRoom_EventScript_83E8A7D::
	msgbox Johto_WillsRoom_Text_83E8CD8, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_WillsRoom_EventScript_83E8AAA
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4832, 0, Johto_WillsRoom_Text_83E8C0A
	goto Johto_WillsRoom_EventScript_83E8AB9

Johto_WillsRoom_EventScript_83E8AAA::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4977, 0, Johto_WillsRoom_Text_83E8C0A
	goto Johto_WillsRoom_EventScript_83E8AB9

Johto_WillsRoom_EventScript_83E8AB9::
	clearflag FLAG_TEMP_5
	msgbox Johto_WillsRoom_Text_83E8C22, MSGBOX_DEFAULT
	closemessage
	setmetatile 6, 1, 0x28E, 0
	setmetatile 6, 2, 0x296, 0
	playse SE_BANG
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 4
	setvar VAR_0x8007, 4
	special ShakeCamera
	waitstate
	special DrawWholeMapView
	setvar VAR_QZ_0x502A, 2
	setflag FLAG_TEMP_4
	release
	end

Johto_WillsRoom_EventScript_83E8AFF::
	msgbox Johto_WillsRoom_Text_83E8C22, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_WillsRoom_Text_83E8B0A::
	.string "Welcome to POKéMON LEAGUE,\n"
	.string "{PLAYER}.\p"
	.string "Allow me to introduce myself.\n"
	.string "I am WILL.\p"
	.string "I have trained all around the\n"
	.string "world, making my psychic\l"
	.string "POKéMON powerful.\p"
	.string "And, at last, I’ve been accepted\n"
	.string "into the ELITE FOUR.\p"
	.string "I can only keep getting better!\p"
	.string "Losing is not an option!$"

Johto_WillsRoom_Text_83E8C0A::
	.string "I… I can’t… believe it…$"

Johto_WillsRoom_Text_83E8C22::
	.string "Even though I was defeated, I\n"
	.string "won’t change my course.\p"
	.string "I will continue battling until I\n"
	.string "stand above all trainers!\p"
	.string "Now, {PLAYER}, move on and\n"
	.string "experience the true ferocity of\l"
	.string "the ELITE FOUR.$"

Johto_WillsRoom_Text_83E8CD8::
	.string "WILL: Ah, welcome back.\p"
	.string "I have foreseen this encounter.\p"
	.string "My psychic POKéMON and\n"
	.string "I have evolved, developing\l"
	.string "new tactics.\p"
	.string "Prepare yourself for a battle of\n"
	.string "enhanced strategy and might.\p"
	.string "Let us see if your foresight\n"
	.string "matches mine.$"
