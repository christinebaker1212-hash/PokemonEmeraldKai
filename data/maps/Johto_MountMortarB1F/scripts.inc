Johto_MountMortarB1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_MountMortarB1F_EventScript_83DFFBD
	.byte 0

Johto_MountMortarB1F_EventScript_83DFFBD::
	call Quetzal_EventScript_840EA62
	end

@ Johto_MountMortarB1F object 7 (local id 7, gfx 0x3B) at 33,4
Johto_MountMortarB1F_EventScript_83DFFC3::
	setorcopyvar VAR_0x8000, ITEM_HYPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortarB1F object 5 (local id 5, gfx 0x3B) at 29,12
Johto_MountMortarB1F_EventScript_83DFFD0::
	setorcopyvar VAR_0x8000, ITEM_CARBOS
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortarB1F object 3 (local id 3, gfx 0x3B) at 4,16
Johto_MountMortarB1F_EventScript_83DFFDD::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortarB1F object 6 (local id 6, gfx 0x3B) at 34,24
Johto_MountMortarB1F_EventScript_83DFFEA::
	setorcopyvar VAR_0x8000, ITEM_MAX_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortarB1F object 4 (local id 4, gfx 0x3B) at 21,26
Johto_MountMortarB1F_EventScript_83DFFF7::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortarB1F object 2 (local id 2, gfx 0x2C) at 16,4
Johto_MountMortarB1F_EventScript_83E0004::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10BA, Johto_MountMortarB1F_EventScript_83E00A0
	goto_if_set FLAG_QZ_0x10BB, Johto_MountMortarB1F_EventScript_83E003D
	specialvar VAR_RESULT, QzGetRegionBadgeCount
	goto_if_lt VAR_RESULT, 7, Johto_MountMortarB1F_EventScript_83E00AE
	msgbox Johto_MountMortarB1F_Text_83E0106, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4277, 0, Johto_MountMortarB1F_Text_83E0161
	setflag FLAG_QZ_0x10BB

Johto_MountMortarB1F_EventScript_83E003D::
	msgbox Johto_MountMortarB1F_Text_83E0178, MSGBOX_DEFAULT
	checkitem ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, 1, Johto_MountMortarB1F_EventScript_83E0071
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	checkitem ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, 0, Johto_MountMortarB1F_EventScript_83E00AB

Johto_MountMortarB1F_EventScript_83E0071::
	checkitem ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, 1, Johto_MountMortarB1F_EventScript_83E009D
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	checkitem ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, 0, Johto_MountMortarB1F_EventScript_83E00AB

Johto_MountMortarB1F_EventScript_83E009D::
	setflag FLAG_QZ_0x10BA

Johto_MountMortarB1F_EventScript_83E00A0::
	msgbox Johto_MountMortarB1F_Text_83E0221, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MountMortarB1F_EventScript_83E00AB::
	closemessage
	release
	end

Johto_MountMortarB1F_EventScript_83E00AE::
	msgbox Johto_MountMortarB1F_Text_83E00B9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MountMortarB1F_Text_83E00B9::
	.string "Keep training!\p"
	.string "Return with seven JOHTO Badges,\n"
	.string "and we’ll test your strength!$"

Johto_MountMortarB1F_Text_83E0106::
	.string "Hey!\p"
	.string "I am the KARATE KING!\n"
	.string "I train alone here in the dark!\p"
	.string "You! Battle with me!\n"
	.string "Hwaaarggh!$"

Johto_MountMortarB1F_Text_83E0161::
	.string "Waaaarggh!\n"
	.string "I’m beaten!$"

Johto_MountMortarB1F_Text_83E0178::
	.string "I... I’m crushed...\p"
	.string "My training is still not\n"
	.string "enough...\p"
	.string "But a loss is a loss.\n"
	.string "I admit it.\p"
	.string "You mastered both of my\n"
	.string "fighting styles.\p"
	.string "I’ll entrust these two scrolls\n"
	.string "to you.$"

Johto_MountMortarB1F_Text_83E0221::
	.string "The two scrolls represent\n"
	.string "opposing paths.\p"
	.string "When you find a KUBFU, choose\n"
	.string "carefully which one to show it.\p"
	.string "Keep training!$"
