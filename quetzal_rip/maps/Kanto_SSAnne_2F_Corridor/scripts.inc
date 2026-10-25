Kanto_SSAnne_2F_Corridor_MapScripts::
	.byte 0

@ Kanto_SSAnne_2F_Corridor trigger at 30,6 (VAR_QZ_0x5130 == 0)
Kanto_SSAnne_2F_Corridor_EventScript_8410471::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_SSAnne_2F_Corridor_EventScript_8410495

@ Kanto_SSAnne_2F_Corridor trigger at 31,6 (VAR_QZ_0x5130 == 0)
Kanto_SSAnne_2F_Corridor_EventScript_841047D::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_SSAnne_2F_Corridor_EventScript_8410495

@ Kanto_SSAnne_2F_Corridor trigger at 32,6 (VAR_QZ_0x5130 == 0)
Kanto_SSAnne_2F_Corridor_EventScript_8410489::
	lockall
	setvar VAR_TEMP_1, 2
	goto Kanto_SSAnne_2F_Corridor_EventScript_8410495

Kanto_SSAnne_2F_Corridor_EventScript_8410495::
	textcolor 0
	playse SE_EXIT
	delay 5
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	addobject 1
	delay 0xA
	applymovement 1, Quetzal_Movement_8457AB0
	waitmovement 0
	delay 0x14
	call_if_eq VAR_TEMP_1, 0, Kanto_SSAnne_2F_Corridor_EventScript_8410523
	call_if_eq VAR_TEMP_1, 1, Kanto_SSAnne_2F_Corridor_EventScript_841052E
	call_if_eq VAR_TEMP_1, 2, Kanto_SSAnne_2F_Corridor_EventScript_8410540
	msgbox Kanto_SSAnne_2F_Corridor_Text_842361C, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 1
	call Kanto_SSAnne_2F_Corridor_EventScript_8410552
	msgbox Kanto_SSAnne_2F_Corridor_Text_842374D, MSGBOX_DEFAULT
	closemessage
	delay 0xA
	playbgm MUS_RG_RIVAL_EXIT, 0
	call_if_eq VAR_TEMP_1, 0, Kanto_SSAnne_2F_Corridor_EventScript_8410573
	call_if_eq VAR_TEMP_1, 1, Kanto_SSAnne_2F_Corridor_EventScript_841057E
	call_if_eq VAR_TEMP_1, 2, Kanto_SSAnne_2F_Corridor_EventScript_8410589
	fadedefaultbgm
	removeobject 1
	setvar VAR_QZ_0x5130, 1
	releaseall
	end

Kanto_SSAnne_2F_Corridor_EventScript_8410523::
	applymovement 1, Kanto_SSAnne_2F_Corridor_Movement_84105A0
	waitmovement 0
	return

Kanto_SSAnne_2F_Corridor_EventScript_841052E::
	applymovement 1, Kanto_SSAnne_2F_Corridor_Movement_84105A4
	applymovement LOCALID_PLAYER, Kanto_SSAnne_2F_Corridor_Movement_841059A
	waitmovement 0
	return

Kanto_SSAnne_2F_Corridor_EventScript_8410540::
	applymovement LOCALID_PLAYER, Kanto_SSAnne_2F_Corridor_Movement_8410594
	applymovement 1, Kanto_SSAnne_2F_Corridor_Movement_84105AA
	waitmovement 0
	return

Kanto_SSAnne_2F_Corridor_EventScript_8410552::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8618, 0, Kanto_SSAnne_2F_Corridor_Text_84236F9
	return

Kanto_SSAnne_2F_Corridor_EventScript_8410573::
	applymovement 1, Kanto_SSAnne_2F_Corridor_Movement_84105B1
	waitmovement 0
	return

Kanto_SSAnne_2F_Corridor_EventScript_841057E::
	applymovement 1, Kanto_SSAnne_2F_Corridor_Movement_84105BA
	waitmovement 0
	return

Kanto_SSAnne_2F_Corridor_EventScript_8410589::
	applymovement 1, Kanto_SSAnne_2F_Corridor_Movement_84105C2
	waitmovement 0
	return

@ Kanto_SSAnne_2F_Corridor object 2 (local id 2, gfx 0x3F41) at 2,7
Kanto_SSAnne_2F_Corridor_EventScript_84105CA::
	msgbox Kanto_SSAnne_2F_Corridor_Text_84235AA, MSGBOX_NPC
	end

Kanto_SSAnne_2F_Corridor_Movement_8410594:
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

Kanto_SSAnne_2F_Corridor_Movement_841059A:
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

Kanto_SSAnne_2F_Corridor_Movement_84105A0:
	walk_down
	walk_down
	walk_down
	step_end

Kanto_SSAnne_2F_Corridor_Movement_84105A4:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_right
	step_end

Kanto_SSAnne_2F_Corridor_Movement_84105AA:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	delay_4
	step_end

Kanto_SSAnne_2F_Corridor_Movement_84105B1:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_SSAnne_2F_Corridor_Movement_84105BA:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_SSAnne_2F_Corridor_Movement_84105C2:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_SSAnne_2F_Corridor_Text_84235AA::
	.string "This ship, she is a luxury liner for\n"
	.string "TRAINERS of the world!\p"
	.string "At every port, we hold parties with\n"
	.string "invited TRAINERS.$"

Kanto_SSAnne_2F_Corridor_Text_842361C::
	.string "BLUE: Bonjour!\n"
	.string "{PLAYER}!\p"
	.string "Imagine seeing you here!\n"
	.string "{PLAYER}, were you really invited?\p"
	.string "So how’s your POKéDEX coming?\p"
	.string "I already caught 40 kinds, pal.\n"
	.string "Different kinds are everywhere.\p"
	.string "Crawl around in grassy areas, and\n"
	.string "look hard for them.$"

Kanto_SSAnne_2F_Corridor_Text_84236F9::
	.string "Humph!\p"
	.string "At least you’re raising your\n"
	.string "POKéMON!$"

Kanto_SSAnne_2F_Corridor_Text_842374D::
	.string "BLUE: I heard there was a CUT\n"
	.string "master on board.\p"
	.string "But he was just a seasick old man!\p"
	.string "CUT itself is really useful.\n"
	.string "Yup, it’ll be handy.\p"
	.string "You should go see him, too.\n"
	.string "Smell ya!$"
