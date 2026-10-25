Johto_KogasRoom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_KogasRoom_OnFrame_83E89AE
	map_script MAP_SCRIPT_ON_RESUME, Johto_KogasRoom_EventScript_83E85BF
	map_script MAP_SCRIPT_ON_LOAD, Johto_KogasRoom_EventScript_83E85AF
	.byte 0

Johto_KogasRoom_OnFrame_83E89AE:
	map_script_2 VAR_TEMP_0, 0, Quetzal_EventScript_83E89B8
	.2byte 0

Johto_KogasRoom_EventScript_83E85AF::
	call_if_ge VAR_QZ_0x502A, 3, Quetzal_EventScript_83E8958
	goto Quetzal_EventScript_83E894C

Johto_KogasRoom_EventScript_83E85BF::
	setvar VAR_0x8004, 1
	special QzDoPokemonLeagueLightingEffect_672
	end

@ Johto_KogasRoom object 1 (local id 1, gfx 0x3E9E) at 6,6
Johto_KogasRoom_EventScript_83E85C8::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x502A, 3, Johto_KogasRoom_EventScript_83E869C
	goto_if_set FLAG_QZ_0x11CA, Johto_KogasRoom_EventScript_83E861A
	msgbox Johto_KogasRoom_Text_83E8762, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_KogasRoom_EventScript_83E860B
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4479, 0, Johto_KogasRoom_Text_83E8878
	goto Johto_KogasRoom_EventScript_83E8656

Johto_KogasRoom_EventScript_83E860B::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4979, 0, Johto_KogasRoom_Text_83E8878
	goto Johto_KogasRoom_EventScript_83E8656

Johto_KogasRoom_EventScript_83E861A::
	msgbox Johto_KogasRoom_Text_83E86A7, MSGBOX_DEFAULT
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_KogasRoom_EventScript_83E8647
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4479, 0, Johto_KogasRoom_Text_83E8878
	goto Johto_KogasRoom_EventScript_83E8656

Johto_KogasRoom_EventScript_83E8647::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4979, 0, Johto_KogasRoom_Text_83E8878
	goto Johto_KogasRoom_EventScript_83E8656

Johto_KogasRoom_EventScript_83E8656::
	clearflag FLAG_TEMP_5
	msgbox Johto_KogasRoom_Text_83E8898, MSGBOX_DEFAULT
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
	setvar VAR_QZ_0x502A, 3
	setflag FLAG_TEMP_4
	release
	end

Johto_KogasRoom_EventScript_83E869C::
	msgbox Johto_KogasRoom_Text_83E8898, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_KogasRoom_Text_83E86A7::
	.string "KOGA: Fwahahahaha!\p"
	.string "You return, brave challenger.\p"
	.string "My ninja skills have sharpened.\p"
	.string "New sinister traps await you.\p"
	.string "Poison, sleep, and more...\n"
	.string "Be ready!\p"
	.string "Fwahahahaha!\p"
	.string "Show me how you’ve grown!$"

Johto_KogasRoom_Text_83E8762::
	.string "Fwahahahaha!\p"
	.string "I am KOGA of the ELITE FOUR.\p"
	.string "I live in shadows, a ninja!\p"
	.string "My intricate style will confound\n"
	.string "and destroy you!\p"
	.string "Confusion, sleep, poison…\p"
	.string "Prepare to be the victim of my\n"
	.string "sinister technique!\p"
	.string "Fwahahahaha!\p"
	.string "POKéMON is not merely about brute\n"
	.string "force--you shall see soon enough!$"

Johto_KogasRoom_Text_83E8878::
	.string "Ah!\n"
	.string "You have proven your worth!$"

Johto_KogasRoom_Text_83E8898::
	.string "I subjected you to everything I\n"
	.string "could muster.\p"
	.string "But my efforts failed. I must hone\n"
	.string "my skills.\p"
	.string "Go on to the next room, and put\n"
	.string "your abilities to test!$"
