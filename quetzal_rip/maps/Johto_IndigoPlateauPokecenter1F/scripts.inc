Johto_IndigoPlateauPokecenter1F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_IndigoPlateauPokecenter1F_OnFrame_83DCE6E
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA9
	.byte 0

Johto_IndigoPlateauPokecenter1F_OnFrame_83DCE6E:
	map_script_2 VAR_QZ_0x502A, 0, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA0
	map_script_2 VAR_QZ_0x502A, 2, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA0
	map_script_2 VAR_QZ_0x502A, 3, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA0
	map_script_2 VAR_QZ_0x502A, 4, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA0
	map_script_2 VAR_QZ_0x502A, 5, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA0
	map_script_2 VAR_QZ_0x502A, 6, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA0
	.2byte 0

@ Johto_IndigoPlateauPokecenter1F object 3 (local id 3, gfx 0x3E8E) at 10,10
Johto_IndigoPlateauPokecenter1F_EventScript_830FFFB::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA0::
	setflag FLAG_QZ_0x11C6
	setvar VAR_QZ_0x502A, 1
	end

Johto_IndigoPlateauPokecenter1F_EventScript_83DCEA9::
	setrespawn 0x23
	goto_if_unset FLAG_QZ_0x11B1, Johto_IndigoPlateauPokecenter1F_EventScript_83DCEB9
	clearflag FLAG_QZ_0x10DA
	end

Johto_IndigoPlateauPokecenter1F_EventScript_83DCEB9::
	setflag FLAG_QZ_0x10DA
	end

@ Johto_IndigoPlateauPokecenter1F object 4 (local id 4, gfx 0x3E90) at 29,10
Johto_IndigoPlateauPokecenter1F_EventScript_83DCEBD::
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DCFF9, MSGBOX_NPC
	end

@ Johto_IndigoPlateauPokecenter1F object 8 (local id 8, gfx 0x2C) at 32,3
Johto_IndigoPlateauPokecenter1F_EventScript_83DCEC6::
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DCFA0, MSGBOX_NPC
	end

@ Johto_IndigoPlateauPokecenter1F object 9 (local id 9, gfx 0x3E90) at 7,15
Johto_IndigoPlateauPokecenter1F_EventScript_83DCECF::
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DCED8, MSGBOX_NPC
	end

@ Johto_IndigoPlateauPokecenter1F object 7 (local id 7, gfx 0x3EB5) at 32,8
Johto_IndigoPlateauPokecenter1F_EventScript_83DD079::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4325, 0, Johto_IndigoPlateauPokecenter1F_Text_83DD097, Johto_IndigoPlateauPokecenter1F_Text_83DD100
	goto Johto_IndigoPlateauPokecenter1F_EventScript_83DD08C

Johto_IndigoPlateauPokecenter1F_EventScript_83DD08C::
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DD10E, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_IndigoPlateauPokecenter1F object 5 (local id 5, gfx 0x25) at 30,14
Johto_IndigoPlateauPokecenter1F_EventScript_83DD156::
	lock
	faceplayer
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DD1BB, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_IndigoPlateauPokecenter1F_EventScript_83DD176
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DD295, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_IndigoPlateauPokecenter1F_EventScript_83DD176::
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DD26C, MSGBOX_DEFAULT
	closemessage
	turnobject LOCALID_PLAYER, 1
	playbgm MUS_QZ_611, 1
	delay 0x3C
	playmoncry SPECIES_ABRA, 0
	applymovement 6, Johto_IndigoPlateauPokecenter1F_Movement_83DD1A5
	waitmovement 0
	fadescreen 3
	warpteleport MAP_JOHTO_NEW_BARK_TOWN, 255, 20, 12
	waitstate
	end

@ Johto_IndigoPlateauPokecenter1F object 6 (local id 6, gfx 0x13F) at 28,15
Johto_IndigoPlateauPokecenter1F_EventScript_83DD1A8::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_ABRA, 0
	msgbox Johto_IndigoPlateauPokecenter1F_Text_83DD2B6, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Johto_IndigoPlateauPokecenter1F object 10 (local id 10, gfx 0x3EB0) at 32,3
Johto_IndigoPlateauPokecenter1F_EventScript_8401C96::
	lock
	special Script_ClearHeldMovement
	faceplayer
	goto_if_set FLAG_QZ_0x1319, Johto_IndigoPlateauPokecenter1F_EventScript_8401CAE
	msgbox Johto_IndigoPlateauPokecenter1F_Text_8401DBF, MSGBOX_DEFAULT
	release
	end

Johto_IndigoPlateauPokecenter1F_EventScript_8401CAE::
	msgbox Johto_IndigoPlateauPokecenter1F_Text_8403484, MSGBOX_DEFAULT
	goto Quetzal_EventScript_8401C92

Johto_IndigoPlateauPokecenter1F_Movement_83DD1A5:
	jump_in_place_right
	jump_in_place_right
	step_end

Johto_IndigoPlateauPokecenter1F_Text_83DCED8::
	.string "Congrats on making it here!\p"
	.string "I’ve seen tons of trainers come\n"
	.string "through here over the years, and\l"
	.string "all of them are wonderfully unique.\p"
	.string "You should be proud of yourself\n"
	.string "for getting this far!\p"
	.string "Now, go get ’em!$"

Johto_IndigoPlateauPokecenter1F_Text_83DCFA0::
	.string "The POKéMON LEAGUE is not taking\n"
	.string "new challengers at the moment.\p"
	.string "Please check back later.$"

Johto_IndigoPlateauPokecenter1F_Text_83DCFF9::
	.string "At the POKéMON LEAGUE, you’ll get\n"
	.string "tested by the ELITE FOUR.\p"
	.string "You have to beat them all. If you\n"
	.string "lose, you have to start all over!$"

Johto_IndigoPlateauPokecenter1F_Text_83DD097::
	.string "You won the JOHTO LEAGUE, but this\n"
	.string "isn’t over.\p"
	.string "My POKéMON and I are still training.\p"
	.string "{PLAYER}! I challenge you!$"

Johto_IndigoPlateauPokecenter1F_Text_83DD100::
	.string "…\p"
	.string "OK--I lost…$"

Johto_IndigoPlateauPokecenter1F_Text_83DD10E::
	.string "I’ll keep training with my POKéMON.\p"
	.string "I’ll be here. Challenge me anytime.$"

Johto_IndigoPlateauPokecenter1F_Text_83DD1BB::
	.string "Ah! You’re challenging the ELITE\n"
	.string "FOUR? Are you sure you’re ready?\p"
	.string "If you need to train some more,\n"
	.string "my ABRA can help you.\p"
	.string "It can TELEPORT you home.\p"
	.string "Would you like to go home now?$"

Johto_IndigoPlateauPokecenter1F_Text_83DD26C::
	.string "OK, OK. Picture your house in your\n"
	.string "mind…$"

Johto_IndigoPlateauPokecenter1F_Text_83DD295::
	.string "OK, OK. The best of luck to you!$"

Johto_IndigoPlateauPokecenter1F_Text_83DD2B6::
	.string "ABRA: Aabra…$"

Johto_IndigoPlateauPokecenter1F_Text_8401DBF::
	.string "GIOVANNI ordered the LEAGUE closed.\n"
	.string "No one enters without his permission.$"

Johto_IndigoPlateauPokecenter1F_Text_8403484::
	.string "This Johto is recovering. The LEAGUE\n"
	.string "remains closed to the public.$"
