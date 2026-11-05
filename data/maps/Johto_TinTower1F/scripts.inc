@ Moemon Kai: Crystal's TIN TOWER 1F (pokecrystal maps/TinTower1F.asm; audit case 139): the first time in, SUICUNE (Lv 40)
@ waits with RAIKOU and ENTEI (the ones the player hasn't caught), EUSINE and the WISE TRIO come in after; after the
@ ELITE FOUR, with all three beasts, the sage at the stairs gives the RAINBOW WING and the stairs appear (HO-OH on the
@ roof). SUICUNE beaten, not caught, stands here again a day later (mp_legends.c, FLAG_JOHTO_TIN_SUICUNE_GONE).
Johto_TinTower1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Johto_TinTower1F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_TinTower1F_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_TinTower1F_OnFrame
	.byte 0

Johto_TinTower1F_OnFrame:
	map_script_2 VAR_TEMP_1, 1, Johto_TinTower1F_EventScript_SuicuneBattle
	.2byte 0

@ (Crystal's TinTower1FStairsCallback: no stairs before the RAINBOW WING)
Johto_TinTower1F_OnLoad::
	goto_if_set FLAG_JOHTO_GOT_RAINBOW_WING, Johto_TinTower1F_EventScript_Stairs
	setmetatile 9, 4, 0x238, FALSE
Johto_TinTower1F_EventScript_Stairs::
	end

Johto_TinTower1F_OnTransition::
	special QzJohtoStaticEncountersRefresh
	setflag FLAG_TEMP_14
	setvar VAR_TEMP_1, 0
	goto_if_set FLAG_JOHTO_GOT_RAINBOW_WING, Johto_TinTower1F_EventScript_GotRainbowWing
	goto_if_unset FLAG_JOHTO_FOUGHT_SUICUNE, Johto_TinTower1F_EventScript_FaceBeasts
	specialvar VAR_RESULT, MP_JohtoLeagueBeaten
	goto_if_eq VAR_RESULT, FALSE, Johto_TinTower1F_EventScript_FoughtSuicune
	specialvar VAR_RESULT, MP_JohtoBeastsCheck
	goto_if_eq VAR_RESULT, FALSE, Johto_TinTower1F_EventScript_FoughtSuicune
Johto_TinTower1F_EventScript_GotRainbowWing::
	setflag FLAG_TEMP_15
	call Johto_TinTower1F_EventScript_SuicuneReturned
	goto_if_unset FLAG_JOHTO_HO_OH_DONE, Johto_TinTower1F_EventScript_HideRaikouEntei
	clearflag FLAG_TEMP_14
Johto_TinTower1F_EventScript_HideRaikouEntei::
	setflag FLAG_TEMP_12
	setflag FLAG_TEMP_13
	end

Johto_TinTower1F_EventScript_FaceBeasts::
	setflag FLAG_TEMP_15
	setflag FLAG_TEMP_16
	setvar VAR_TEMP_1, 1
	setvar VAR_0x8004, SPECIES_RAIKOU
	specialvar VAR_RESULT, MP_JohtoMonCheck
	call_if_eq VAR_RESULT, TRUE, Johto_TinTower1F_EventScript_NoRaikou
	setvar VAR_0x8004, SPECIES_ENTEI
	specialvar VAR_RESULT, MP_JohtoMonCheck
	call_if_eq VAR_RESULT, TRUE, Johto_TinTower1F_EventScript_NoEntei
	end

Johto_TinTower1F_EventScript_NoRaikou::
	setflag FLAG_TEMP_12
	return

Johto_TinTower1F_EventScript_NoEntei::
	setflag FLAG_TEMP_13
	return

Johto_TinTower1F_EventScript_FoughtSuicune::
	setflag FLAG_TEMP_12
	setflag FLAG_TEMP_13
	setflag FLAG_TEMP_16
	call Johto_TinTower1F_EventScript_SuicuneReturned
	end

@ (SUICUNE stands here again once its day is up, if it wasn't caught)
Johto_TinTower1F_EventScript_SuicuneReturned::
	goto_if_unset FLAG_JOHTO_TIN_SUICUNE_GONE, Johto_TinTower1F_EventScript_SuicuneStays
	setflag FLAG_TEMP_11
	return

Johto_TinTower1F_EventScript_SuicuneStays::
	setobjectxyperm 1, 9, 13
	return

@ (Crystal's TinTower1FSuicuneBattleScript)
Johto_TinTower1F_EventScript_SuicuneBattle::
	lockall
	setvar VAR_TEMP_1, 0
	applymovement LOCALID_PLAYER, Johto_TinTower1F_Movement_PlayerEnters
	waitmovement 0
	delay 15
	goto_if_set FLAG_TEMP_12, Johto_TinTower1F_EventScript_Next1
	applymovement 2, Johto_TinTower1F_Movement_BeastApproaches
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_WEST
	playmoncry SPECIES_RAIKOU, CRY_MODE_NORMAL
	waitmoncry
	delay 10
	playse SE_WARP_OUT
	applymovement 2, Johto_TinTower1F_Movement_RaikouLeaves
	waitmovement 0
	removeobject 2
	playse SE_EXIT
	waitse
Johto_TinTower1F_EventScript_Next1::
	goto_if_set FLAG_TEMP_13, Johto_TinTower1F_EventScript_Next2
	applymovement 3, Johto_TinTower1F_Movement_BeastApproaches
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_EAST
	playmoncry SPECIES_ENTEI, CRY_MODE_NORMAL
	waitmoncry
	delay 10
	playse SE_WARP_OUT
	applymovement 3, Johto_TinTower1F_Movement_EnteiLeaves
	waitmovement 0
	removeobject 3
	playse SE_EXIT
	waitse
Johto_TinTower1F_EventScript_Next2::
	turnobject LOCALID_PLAYER, DIR_NORTH
	delay 10
	playmoncry SPECIES_SUICUNE, CRY_MODE_NORMAL
	waitmoncry
	delay 20
	setwildbattle SPECIES_SUICUNE, 40, ITEM_NONE
	special BattleSetup_StartLegendaryBattle
	waitstate
	setflag FLAG_JOHTO_FOUGHT_SUICUNE
	setvar VAR_QZ_0x502D, 9   @ (no more sightings at CIANWOOD, ROUTE 42 and ROUTE 36)
	setvar VAR_0x8004, FLAG_JOHTO_TIN_SUICUNE_GONE
	special MP_JohtoLegendAfterBattle
	goto_if_eq VAR_RESULT, FALSE, Johto_TinTower1F_EventScript_SuicuneStillHere
	removeobject 1
Johto_TinTower1F_EventScript_SuicuneStillHere::
	delay 20
	turnobject LOCALID_PLAYER, DIR_SOUTH
	playse SE_DOOR
	setobjectxyperm 4, 9, 16
	addobject 4
	applymovement 4, Johto_TinTower1F_Movement_EusineEnters
	waitmovement 0
	playse SE_DOOR
	setobjectxyperm 5, 8, 16
	addobject 5
	applymovement 5, Johto_TinTower1F_Movement_Sage1Enters
	waitmovement 0
	playse SE_DOOR
	setobjectxyperm 6, 10, 16
	addobject 6
	applymovement 6, Johto_TinTower1F_Movement_Sage2Enters
	waitmovement 0
	playse SE_DOOR
	setobjectxyperm 7, 8, 16
	addobject 7
	applymovement 7, Johto_TinTower1F_Movement_Sage3Enters
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_SOUTH
	msgbox Johto_TinTower1F_Text_EusineSuicune, MSGBOX_DEFAULT
	closemessage
	applymovement 4, Johto_TinTower1F_Movement_EusineLeaves
	waitmovement 0
	playse SE_EXIT
	removeobject 4
	setobjectxyperm 5, 4, 10
	setobjectxyperm 6, 14, 12
	setobjectxyperm 7, 14, 7
	releaseall
	end

@ (a SUICUNE back after its day: Crystal's battle, standing)
Johto_TinTower1F_EventScript_Suicune::
	lock
	faceplayer
	playmoncry SPECIES_SUICUNE, CRY_MODE_NORMAL
	waitmoncry
	setwildbattle SPECIES_SUICUNE, 40, ITEM_NONE
	special BattleSetup_StartLegendaryBattle
	waitstate
	setvar VAR_0x8004, FLAG_JOHTO_TIN_SUICUNE_GONE
	special MP_JohtoLegendAfterBattle
	goto_if_eq VAR_RESULT, FALSE, Johto_TinTower1F_EventScript_SuicuneRelease
	removeobject 1
Johto_TinTower1F_EventScript_SuicuneRelease::
	release
	end

Johto_TinTower1F_EventScript_Sage1::
	msgbox Johto_TinTower1F_Text_Sage1, MSGBOX_NPC
	end

Johto_TinTower1F_EventScript_Sage2::
	msgbox Johto_TinTower1F_Text_Sage2, MSGBOX_NPC
	end

Johto_TinTower1F_EventScript_Sage3::
	msgbox Johto_TinTower1F_Text_Sage3, MSGBOX_NPC
	end

Johto_TinTower1F_EventScript_Sage4::
	goto_if_set FLAG_JOHTO_HO_OH_DONE, Johto_TinTower1F_EventScript_Sage4FoughtHoOh
	msgbox Johto_TinTower1F_Text_Sage4_1, MSGBOX_NPC
	end

Johto_TinTower1F_EventScript_Sage4FoughtHoOh::
	msgbox Johto_TinTower1F_Text_Sage4_2, MSGBOX_NPC
	end

@ (Crystal's TinTower1FSage5Script: the RAINBOW WING; the stairs appear)
Johto_TinTower1F_EventScript_Sage5::
	lock
	faceplayer
	goto_if_set FLAG_JOHTO_HO_OH_DONE, Johto_TinTower1F_EventScript_Sage5FoughtHoOh
	goto_if_set FLAG_JOHTO_GOT_RAINBOW_WING, Johto_TinTower1F_EventScript_Sage5GotRainbowWing
	msgbox Johto_TinTower1F_Text_Sage5_1, MSGBOX_DEFAULT
	giveitem ITEM_RAINBOW_WING
	closemessage
	setflag FLAG_JOHTO_GOT_RAINBOW_WING
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 12
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_M_STRENGTH
	setmetatile 9, 4, 0x320, FALSE
	special DrawWholeMapView
	waitse
Johto_TinTower1F_EventScript_Sage5GotRainbowWing::
	msgbox Johto_TinTower1F_Text_Sage5_2, MSGBOX_DEFAULT
	release
	end

Johto_TinTower1F_EventScript_Sage5FoughtHoOh::
	msgbox Johto_TinTower1F_Text_Sage5_3, MSGBOX_DEFAULT
	release
	end

Johto_TinTower1F_EventScript_Sage6::
	goto_if_set FLAG_JOHTO_HO_OH_DONE, Johto_TinTower1F_EventScript_Sage6FoughtHoOh
	msgbox Johto_TinTower1F_Text_Sage6_1, MSGBOX_NPC
	end

Johto_TinTower1F_EventScript_Sage6FoughtHoOh::
	msgbox Johto_TinTower1F_Text_Sage6_2, MSGBOX_NPC
	end

Johto_TinTower1F_EventScript_Eusine::
	msgbox Johto_TinTower1F_Text_EusineHoOh, MSGBOX_NPC
	end

Johto_TinTower1F_Movement_PlayerEnters:
	walk_slow_up
	walk_slow_up
	walk_slow_up
	step_end

Johto_TinTower1F_Movement_BeastApproaches:
	jump_down
	step_end

Johto_TinTower1F_Movement_RaikouLeaves:
	jump_left
	step_end

Johto_TinTower1F_Movement_EnteiLeaves:
	jump_right
	step_end

Johto_TinTower1F_Movement_EusineEnters:
	walk_up
	face_up
	step_end

Johto_TinTower1F_Movement_EusineLeaves:
	walk_down
	step_end

Johto_TinTower1F_Movement_Sage1Enters:
	walk_up
	walk_up
	walk_left
	face_up
	step_end

Johto_TinTower1F_Movement_Sage2Enters:
	walk_up
	walk_up
	walk_right
	face_up
	step_end

Johto_TinTower1F_Movement_Sage3Enters:
	walk_up
	face_up
	step_end

Johto_TinTower1F_Text_EusineSuicune::
	.string "EUSINE: Awesome! Too awesome,\n"
	.string "even!\p"
	.string "I’ve never seen a battle that\n"
	.string "great.\p"
	.string "That was truly inspiring to see.\p"
	.string "SUICUNE was tough, but you were\n"
	.string "even more incredible, {PLAYER}.\p"
	.string "I heard SUICUNE’s mystic power\n"
	.string "summons a rainbowcolored MOéMON.\p"
	.string "Maybe, just maybe, what went on\n"
	.string "today will cause that MOéMON to\l"
	.string "appear.\p"
	.string "I’m going to study the legends\n"
	.string "more.\p"
	.string "Thanks for showing me that\n"
	.string "fantastic battle.\p"
	.string "Later, {PLAYER}!$"

Johto_TinTower1F_Text_Sage1::
	.string "According to legend…\p"
	.string "When the souls of MOéMON and\n"
	.string "humans commune, from the heavens\l"
	.string "descends a MOéMON of rainbow\l"
	.string "colors…\p"
	.string "Could it mean the legendary MOéMON\n"
	.string "are testing us humans?$"

Johto_TinTower1F_Text_Sage2::
	.string "When the BRASS TOWER burned down,\n"
	.string "three nameless MOéMON were said to\l"
	.string "have perished. It was tragic.\p"
	.string "However…\p"
	.string "A rainbow-colored MOéMON…\p"
	.string "In other words…\p"
	.string "HO-OH descended from the sky and\n"
	.string "gave new life to the three MOéMON.\p"
	.string "They are…\p"
	.string "SUICUNE, ENTEI and RAIKOU.\p"
	.string "That is what they say.$"

Johto_TinTower1F_Text_Sage3::
	.string "The two TOWERS are said to have\n"
	.string "been built to foster friendship\l"
	.string "and hope between MOéMON and\l"
	.string "people.\p"
	.string "That was 700 years ago, but the\n"
	.string "ideal still remains important\l"
	.string "today.$"

Johto_TinTower1F_Text_Sage4_1::
	.string "HO-OH appears to have descended\n"
	.string "upon this, the TIN TOWER!$"

Johto_TinTower1F_Text_Sage4_2::
	.string "The legendary MOéMON are said to\n"
	.string "embody three powers…\p"
	.string "The lightning that struck the\n"
	.string "TOWER.\p"
	.string "The fire that burned the TOWER.\p"
	.string "And the rain that put out the\n"
	.string "fire…$"

Johto_TinTower1F_Text_Sage5_1::
	.string "This will protect you. Take it.$"

Johto_TinTower1F_Text_Sage5_2::
	.string "Now, go.$"

Johto_TinTower1F_Text_Sage5_3::
	.string "When the legendary MOéMON\n"
	.string "appeared…\p"
	.string "They struck terror in those who\n"
	.string "saw their rise.\p"
	.string "And…\p"
	.string "Some even took to futile attacks.\p"
	.string "The legendary MOéMON, knowing\n"
	.string "their own power, fled, ignoring\l"
	.string "the frightened people.$"

Johto_TinTower1F_Text_Sage6_1::
	.string "I believe you are being tested.\p"
	.string "Free your mind from uncertainty,\n"
	.string "and advance.$"

Johto_TinTower1F_Text_Sage6_2::
	.string "Of the legendary MOéMON, SUICUNE\n"
	.string "is said to be the closest to\l"
	.string "HO-OH.\p"
	.string "I hear there may also be a link to\n"
	.string "MOéMON known as UNOWN.\p"
	.string "The MOéMON UNOWN must be sharing a\n"
	.string "cooperative bond with SUICUNE.$"

Johto_TinTower1F_Text_EusineHoOh::
	.string "I knew it.\p"
	.string "I knew you’d get to see the MOéMON\n"
	.string "of rainbow colors, {PLAYER}.\p"
	.string "It happened just as I envisioned.\p"
	.string "My research isn’t bad, I might\n"
	.string "say.\p"
	.string "I’m going to keep studying MOéMON\n"
	.string "to become a famous POKéMANIAC!$"
