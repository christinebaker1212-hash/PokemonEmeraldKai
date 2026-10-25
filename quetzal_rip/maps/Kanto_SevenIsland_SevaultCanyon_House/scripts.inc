Kanto_SevenIsland_SevaultCanyon_House_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_SevaultCanyon_House_EventScript_8421FF4
	.byte 0

Kanto_SevenIsland_SevaultCanyon_House_EventScript_8421FF4::
	setvar VAR_QZ_0x4010, 0xC01
	special QzKantoStaticEncountersRefresh_757
	end

@ Kanto_SevenIsland_SevaultCanyon_House object 1 (local id 1, gfx 0x3EE6) at 2,4
Kanto_SevenIsland_SevaultCanyon_House_EventScript_8421FFD::
	lock
	goto_if_set FLAG_TEMP_2, Kanto_SevenIsland_SevaultCanyon_House_EventScript_8422117
	playbgm MUS_SCHOOL, 0
	applymovement 1, Kanto_SevenIsland_SevaultCanyon_House_Movement_8422137
	waitmovement 0
	fadedefaultbgm
	delay 0x1E
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_8455629, MSGBOX_DEFAULT
	textcolor 3
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_8455666, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SevenIsland_SevaultCanyon_House_EventScript_842212B
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_845568F, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_8456AB7
	call_if_ne VAR_FACING, 1, Kanto_SevenIsland_SevaultCanyon_House_EventScript_842210C
	delay 0x1E
	special QzSevaultHouseEventEligibility_759
	goto_if_ne VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_House_EventScript_8422074
	call Kanto_SevenIsland_SevaultCanyon_House_EventScript_84220C7
	goto Kanto_SevenIsland_SevaultCanyon_House_EventScript_8422078

Kanto_SevenIsland_SevaultCanyon_House_EventScript_8422074::
	playbgm MUS_SCHOOL, 0

Kanto_SevenIsland_SevaultCanyon_House_EventScript_8422078::
	applymovement LOCALID_PLAYER, Kanto_SevenIsland_SevaultCanyon_House_Movement_8422137
	goto_if_unset FLAG_QZ_0x2924, Kanto_SevenIsland_SevaultCanyon_House_EventScript_84220AE
	goto_if_set FLAG_QZ_0x2922, Kanto_SevenIsland_SevaultCanyon_House_EventScript_84220AE
	goto_if_set FLAG_QZ_0x2923, Kanto_SevenIsland_SevaultCanyon_House_EventScript_84220AE
	applymovement 1, Kanto_SevenIsland_SevaultCanyon_House_Movement_8422137
	applymovement 4, Kanto_SevenIsland_SevaultCanyon_House_Movement_8422137
	waitmovement 1
	waitmovement 4

Kanto_SevenIsland_SevaultCanyon_House_EventScript_84220AE::
	waitmovement LOCALID_PLAYER
	fadedefaultbgm
	delay 0x1E
	call Quetzal_EventScript_84570E0
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_84556AC, MSGBOX_DEFAULT
	setflag FLAG_TEMP_2
	release
	end

Kanto_SevenIsland_SevaultCanyon_House_EventScript_84220C7::
	removeobject 4
	setvar VAR_QZ_0x4010, 0x388
	clearflag FLAG_QZ_0x2921
	addobject 4
	delay 0x10
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_84555C6, MSGBOX_DEFAULT
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_845560B, MSGBOX_DEFAULT
	closemessage
	playmoncry SPECIES_MELOETTA, 0
	waitmoncry
	playse SE_M_SING
	waitse
	playbgm MUS_SCHOOL, 0
	removeobject 4
	setvar VAR_QZ_0x4010, 0xC01
	clearflag FLAG_QZ_0x2921
	addobject 4
	setflag FLAG_QZ_0x2924
	delay 0x10
	return

Kanto_SevenIsland_SevaultCanyon_House_EventScript_842210C::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AB0
	waitmovement 0
	return

Kanto_SevenIsland_SevaultCanyon_House_EventScript_8422117::
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_8455629, MSGBOX_DEFAULT
	release
	end

Kanto_SevenIsland_SevaultCanyon_House_EventScript_842212B::
	textcolor 0
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_845567F, MSGBOX_DEFAULT
	release
	end

@ Kanto_SevenIsland_SevaultCanyon_House object 2 (local id 2, gfx 0x3EF7) at 4,2
Kanto_SevenIsland_SevaultCanyon_House_EventScript_8422148::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_CHANSEY, 0
	msgbox Kanto_SevenIsland_SevaultCanyon_House_Text_84556DD, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_SevenIsland_SevaultCanyon_House object 3 (local id 3, gfx 0x3F0E) at 6,4
Kanto_SevenIsland_SevaultCanyon_House_EventScript_846C2E2::
	setorcopyvar VAR_0x8000, 0x18B
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SevenIsland_SevaultCanyon_House_Movement_8422137:
	walk_in_place_down
	delay_16
	walk_in_place_down
	delay_16
	walk_in_place_down
	delay_16
	walk_in_place_down
	delay_16
	walk_in_place_faster_right
	delay_4
	walk_in_place_faster_up
	delay_4
	walk_in_place_faster_left
	delay_4
	walk_in_place_faster_down
	delay_4
	step_end

Kanto_SevenIsland_SevaultCanyon_House_Text_84555C6::
	.string "Oh, it’s MELOETTA!\p"
	.string "It looks like it wants to join\n"
	.string "our CHANSEY Dance!$"

Kanto_SevenIsland_SevaultCanyon_House_Text_845560B::
	.string "MELOETTA performs Relic Song!$"

Kanto_SevenIsland_SevaultCanyon_House_Text_8455629::
	.string "Dance, dance!\n"
	.string "CHANSEY Dance!\p"
	.string "You join in, too!\n"
	.string "Dance, dance!$"

Kanto_SevenIsland_SevaultCanyon_House_Text_8455666::
	.string "Would you like to dance?$"

Kanto_SevenIsland_SevaultCanyon_House_Text_845567F::
	.string "Come on, dance!$"

Kanto_SevenIsland_SevaultCanyon_House_Text_845568F::
	.string "{PLAYER} danced the CHANSEY Dance!$"

Kanto_SevenIsland_SevaultCanyon_House_Text_84556AC::
	.string "Ahahahaha!\p"
	.string "There, you’re all chipper now!\n"
	.string "Lucky!$"

Kanto_SevenIsland_SevaultCanyon_House_Text_84556DD::
	.string "CHANSEY: Chansi! Chansi!$"
