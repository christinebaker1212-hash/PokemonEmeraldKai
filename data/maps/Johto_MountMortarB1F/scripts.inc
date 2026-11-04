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
	@ Moemon Kai: Crystal's gift (tools/quetzal_port/fix_johto_gifts.py) (Quetzal: two scrolls, seven badges first)
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10BA, Johto_MountMortarB1F_EventScript_83E00A0
	goto_if_set FLAG_QZ_0x10BB, Johto_MountMortarB1F_EventScript_83E003D
	msgbox Johto_MountMortarB1F_Text_83E0106, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4277, 0, Johto_MountMortarB1F_Text_83E0161
	setflag FLAG_QZ_0x10BB

Johto_MountMortarB1F_EventScript_83E003D::
	msgbox Johto_MountMortarB1F_Text_83E0178, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, SPECIES_TYROGUE
	bufferspeciesname 0, SPECIES_TYROGUE
	qz_givemon_ex SPECIES_TYROGUE, 10, ITEM_NONE, 1, 2, 3
	call Quetzal_EventScript_840181E
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

Johto_MountMortarB1F_Text_83E0106::
	.string "Hey!\p"
	.string "I am the KARATE KING!\p"
	.string "I train alone here in the dark!\p"
	.string "You! Battle with me!\p"
	.string "Hwaaarggh!$"

Johto_MountMortarB1F_Text_83E0161::
	.string "Waaaarggh! I’m beaten!$"

Johto_MountMortarB1F_Text_83E0178::
	.string "I… I’m crushed…\p"
	.string "My training is still not enough…\p"
	.string "But a loss is a loss. I admit it.\p"
	.string "As proof that you defeated me,\n"
	.string "I’ll give you a rare fighting\l"
	.string "MOéMON.$"

Johto_MountMortarB1F_Text_83E0221::
	.string "TYROGUE is a fighting-type.\p"
	.string "It evolves into a tougher MOéMON.\p"
	.string "Keep up the hard work. I’ll keep\n"
	.string "training too.\p"
	.string "Farewell!$"
