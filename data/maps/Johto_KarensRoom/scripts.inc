Johto_KarensRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_KarensRoom_OnFrame_83E89AE
	map_script MAP_SCRIPT_ON_RESUME, Johto_KarensRoom_EventScript_83E81A4
	map_script MAP_SCRIPT_ON_LOAD, Johto_KarensRoom_EventScript_83E8194
	.byte 0

Johto_KarensRoom_OnFrame_83E89AE:
	map_script_2 VAR_TEMP_0, 0, Quetzal_EventScript_83E89B8
	.2byte 0

Johto_KarensRoom_EventScript_83E8194::
	call_if_ge VAR_QZ_0x502A, 5, Quetzal_EventScript_83E8958
	goto Quetzal_EventScript_83E894C

Johto_KarensRoom_EventScript_83E81A4::
	setvar VAR_0x8004, 3
	special QzDoPokemonLeagueLightingEffect
	end

@ Johto_KarensRoom object 1 (local id 1, gfx 0x3E9C) at 6,6
Johto_KarensRoom_EventScript_83E81AD::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x502A, 5, Johto_KarensRoom_EventScript_83E8281
	goto_if_set FLAG_QZ_0x11CA, Johto_KarensRoom_EventScript_83E81FF
	msgbox Johto_KarensRoom_Text_83E83B7, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_KarensRoom_EventScript_83E81F0
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4477, 0, Johto_KarensRoom_Text_83E848E
	goto Johto_KarensRoom_EventScript_83E823B

Johto_KarensRoom_EventScript_83E81F0::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4983, 0, Johto_KarensRoom_Text_83E848E
	goto Johto_KarensRoom_EventScript_83E823B

Johto_KarensRoom_EventScript_83E81FF::
	msgbox Johto_KarensRoom_Text_83E828C, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_KarensRoom_EventScript_83E822C
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4477, 0, Johto_KarensRoom_Text_83E848E
	goto Johto_KarensRoom_EventScript_83E823B

Johto_KarensRoom_EventScript_83E822C::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4983, 0, Johto_KarensRoom_Text_83E848E
	goto Johto_KarensRoom_EventScript_83E823B

Johto_KarensRoom_EventScript_83E823B::
	clearflag FLAG_TEMP_5
	msgbox Johto_KarensRoom_Text_83E84BF, MSGBOX_DEFAULT
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
	setvar VAR_QZ_0x502A, 5
	setflag FLAG_TEMP_4
	release
	end

Johto_KarensRoom_EventScript_83E8281::
	msgbox Johto_KarensRoom_Text_83E84BF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_KarensRoom_Text_83E828C::
	.string "KAREN: Ah, it’s you again.\p"
	.string "I trust you haven’t forgotten\n"
	.string "my belief:\p"
	.string "True strength arises from trust\n"
	.string "in one’s POKéMON.\p"
	.string "Since our last encounter, my\n"
	.string "bond with my POKéMON has\l"
	.string "deepened further.\p"
	.string "Are you ready to face a team\n"
	.string "united by an unbreakable bond?\p"
	.string "Show me the power of the bond\n"
	.string "with your POKéMON.$"

Johto_KarensRoom_Text_83E83B7::
	.string "I am KAREN of the ELITE FOUR.\p"
	.string "You’re {PLAYER}? How amusing.\p"
	.string "I love DARK-type POKéMON.\p"
	.string "I find their wild, tough image\n"
	.string "to be so appealing. And they’re\l"
	.string "so strong.\p"
	.string "Think you can take them?\n"
	.string "Just try to entertain me.\p"
	.string "Let’s go.$"

Johto_KarensRoom_Text_83E848E::
	.string "Well, aren’t you good. I like that\n"
	.string "in a trainer.$"

Johto_KarensRoom_Text_83E84BF::
	.string "Strong POKéMON.\p"
	.string "Weak POKéMON.\p"
	.string "That is only the selfish\n"
	.string "perception of people.\p"
	.string "Truly skilled trainers should\n"
	.string "try to win with their favorites.\p"
	.string "I like your style. You understand\n"
	.string "what’s important.\p"
	.string "Go on--the CHAMPION is waiting.$"
