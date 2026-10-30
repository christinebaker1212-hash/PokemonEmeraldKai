Johto_BrunosRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_BrunosRoom_OnFrame_83E89AE
	map_script MAP_SCRIPT_ON_RESUME, Johto_BrunosRoom_EventScript_83E7058
	map_script MAP_SCRIPT_ON_LOAD, Johto_BrunosRoom_EventScript_83E7048
	.byte 0

Johto_BrunosRoom_OnFrame_83E89AE:
	map_script_2 VAR_TEMP_0, 0, Quetzal_EventScript_83E89B8
	.2byte 0

Johto_BrunosRoom_EventScript_83E7048::
	call_if_ge VAR_QZ_0x502A, 4, Quetzal_EventScript_83E8958
	goto Quetzal_EventScript_83E894C

Johto_BrunosRoom_EventScript_83E7058::
	setvar VAR_0x8004, 2
	special QzDoPokemonLeagueLightingEffect
	end

@ Johto_BrunosRoom object 1 (local id 1, gfx 0x3E88) at 6,6
Johto_BrunosRoom_EventScript_83E7061::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x502A, 4, Johto_BrunosRoom_EventScript_83E7135
	goto_if_set FLAG_QZ_0x11CA, Johto_BrunosRoom_EventScript_83E70B3
	msgbox Johto_BrunosRoom_Text_83E722E, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_BrunosRoom_EventScript_83E70A4
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4475, 0, Johto_BrunosRoom_Text_83E734F
	goto Johto_BrunosRoom_EventScript_83E70EF

Johto_BrunosRoom_EventScript_83E70A4::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4981, 0, Johto_BrunosRoom_Text_83E734F
	goto Johto_BrunosRoom_EventScript_83E70EF

Johto_BrunosRoom_EventScript_83E70B3::
	msgbox Johto_BrunosRoom_Text_83E7140, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_BrunosRoom_EventScript_83E70E0
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4475, 0, Johto_BrunosRoom_Text_83E734F
	goto Johto_BrunosRoom_EventScript_83E70EF

Johto_BrunosRoom_EventScript_83E70E0::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4981, 0, Johto_BrunosRoom_Text_83E734F
	goto Johto_BrunosRoom_EventScript_83E70EF

Johto_BrunosRoom_EventScript_83E70EF::
	clearflag FLAG_TEMP_5
	msgbox Johto_BrunosRoom_Text_83E7367, MSGBOX_DEFAULT
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
	setvar VAR_QZ_0x502A, 4
	setflag FLAG_TEMP_4
	release
	end

Johto_BrunosRoom_EventScript_83E7135::
	msgbox Johto_BrunosRoom_Text_83E7367, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_BrunosRoom_Text_83E7140::
	.string "BRUNO: Welcome back, young\n"
	.string "warrior.\p"
	.string "Through rigorous training, we have\n"
	.string "grown stronger.\p"
	.string "My rock-hard will is even more firm\n"
	.string "and unyielding.\p"
	.string "Are you prepared to face an even\n"
	.string "greater challenge?\p"
	.string "Show me the results of your\n"
	.string "training.\p"
	.string "Hoo hah!$"

Johto_BrunosRoom_Text_83E722E::
	.string "I am BRUNO of the ELITE FOUR.\p"
	.string "I always train to the extreme be-\n"
	.string "cause I believe in our potential.\p"
	.string "That is how we became strong.\p"
	.string "Can you withstand our power?\p"
	.string "Hm? I see no fear in you. You look\n"
	.string "determined. Perfect for battle!\p"
	.string "Ready, {PLAYER}? You will bow down\n"
	.string "to our overwhelming power!\p"
	.string "Hoo hah!$"

Johto_BrunosRoom_Text_83E734F::
	.string "Why? How could we lose?$"

Johto_BrunosRoom_Text_83E7367::
	.string "Having lost, I have no right to\n"
	.string "say anything…\p"
	.string "Go face your next challenge!$"
