Kanto_CinnabarIsland_PokemonLab_Entrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E498
	.byte 0

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E498::
	call_if_eq VAR_QZ_0x510C, 1, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4A4
	end

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4A4::
	setvar VAR_QZ_0x510C, 2
	return

@ Kanto_CinnabarIsland_PokemonLab_Entrance object 1 (local id 1, gfx 0x3F43) at 2,4
Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4AA::
	lock
	faceplayer
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449E2F, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzIsSaveFlag2F4Bit4Set_829
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4FD
	goto_if_unset FLAG_QZ_0x2E13, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4FD
	goto_if_set FLAG_QZ_0x2E16, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4FD

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4D6::
	message Quetzal_Text_8360D02
	waitmessage
	multichoice 0x14, 0, 0x89, 1
	nop
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4FD
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E50C
	goto Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E502

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4FD::
	pokemart Kanto_CinnabarIsland_PokemonLab_Entrance_Mart_841E574

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E502::
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E50C::
	setvar VAR_0x8004, 0
	special QzBufferLabQuestChecklist_781
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EDF, MSGBOX_DEFAULT

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E51C::
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449F58, MSGBOX_DEFAULT
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E537
	special QzBufferLabQuestChecklist_781
	goto Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E51C

Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E537::
	goto_if_eq VAR_0x8006, 0, Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4D6
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EFF, MSGBOX_DEFAULT
	goto Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E4D6

@ Kanto_CinnabarIsland_PokemonLab_Entrance sign / bg event at 4,1
Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E54F::
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449E6D, MSGBOX_SIGN
	end

@ Kanto_CinnabarIsland_PokemonLab_Entrance sign / bg event at 12,5
Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E558::
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449E96, MSGBOX_SIGN
	end

@ Kanto_CinnabarIsland_PokemonLab_Entrance sign / bg event at 18,5
Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E561::
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EAF, MSGBOX_SIGN
	end

@ Kanto_CinnabarIsland_PokemonLab_Entrance sign / bg event at 24,5
Kanto_CinnabarIsland_PokemonLab_Entrance_EventScript_841E56A::
	msgbox Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EC6, MSGBOX_SIGN
	end

Kanto_CinnabarIsland_PokemonLab_Entrance_Mart_841E574:
	.2byte ITEM_FLAME_PLATE
	.2byte ITEM_SPLASH_PLATE
	.2byte ITEM_ZAP_PLATE
	.2byte ITEM_MEADOW_PLATE
	.2byte ITEM_ICICLE_PLATE
	.2byte ITEM_FIST_PLATE
	.2byte ITEM_TOXIC_PLATE
	.2byte ITEM_EARTH_PLATE
	.2byte ITEM_SKY_PLATE
	.2byte ITEM_MIND_PLATE
	.2byte ITEM_INSECT_PLATE
	.2byte ITEM_STONE_PLATE
	.2byte ITEM_SPOOKY_PLATE
	.2byte ITEM_DRACO_PLATE
	.2byte ITEM_DREAD_PLATE
	.2byte ITEM_IRON_PLATE
	.2byte ITEM_NONE

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449E2F::
	.string "We study POKéMON here.\p"
	.string "We also sell plates\n"
	.string "for your research.$"

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449E6D::
	.string "A photo of the LAB’s founder…\n"
	.string "DR. FUJI?!$"

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449E96::
	.string "POKéMON LAB\n"
	.string "Meeting Room$"

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EAF::
	.string "POKéMON LAB\n"
	.string "R & D Room$"

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EC6::
	.string "POKéMON LAB\n"
	.string "Testing Room$"

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EDF::
	.string "ARCEUS trials\n"
	.string "Completed: {STR_VAR_3}/17.$"

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449EFF::
	.string "You have overcome all seventeen types!\p"
	.string "Only the challenge of\n"
	.string "Normal-type ARCEUS remains.$"

Kanto_CinnabarIsland_PokemonLab_Entrance_Text_8449F58::
	.string "{STR_VAR_1}\n"
	.string "{STR_VAR_2}$"
