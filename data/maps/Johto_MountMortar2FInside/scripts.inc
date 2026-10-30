Johto_MountMortar2FInside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_MountMortar2FInside_EventScript_83DFEA5
	.byte 0

Johto_MountMortar2FInside_EventScript_83DFEA5::
	call Quetzal_EventScript_840EA62
	end

@ Johto_MountMortar2FInside object 1 (local id 1, gfx 0x3A9E) at 16,9
Johto_MountMortar2FInside_EventScript_83DFEAB::
	setorcopyvar VAR_0x8000, ITEM_IRON
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar2FInside object 2 (local id 2, gfx 0x3A9E) at 29,9
Johto_MountMortar2FInside_EventScript_83DFEB8::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar2FInside object 3 (local id 3, gfx 0x3A9E) at 9,15
Johto_MountMortar2FInside_EventScript_83DFEC5::
	setorcopyvar VAR_0x8000, ITEM_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar2FInside object 4 (local id 4, gfx 0x3A9E) at 19,21
Johto_MountMortar2FInside_EventScript_83DFED2::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar2FInside object 5 (local id 5, gfx 0x3A9E) at 28,26
Johto_MountMortar2FInside_EventScript_83DFEDF::
	setorcopyvar VAR_0x8000, ITEM_DRAGON_SCALE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar2FInside object 6 (local id 6, gfx 0x3A9E) at 2,37
Johto_MountMortar2FInside_EventScript_83DFEEC::
	setorcopyvar VAR_0x8000, ITEM_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar2FInside object 7 (local id 7, gfx 0x3EB7) at 11,29
Johto_MountMortar2FInside_EventScript_83DFEF9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4495, 0, Johto_MountMortar2FInside_Text_83DFF10, Johto_MountMortar2FInside_Text_83DFF1C
	msgbox Johto_MountMortar2FInside_Text_83DFF40, MSGBOX_AUTOCLOSE
	end

Johto_MountMortar2FInside_Text_83DFF10::
	.string "Yo! MARKUS!$"

Johto_MountMortar2FInside_Text_83DFF1C::
	.string "Sorry, my mistake.\n"
	.string "I misjudged you.$"

Johto_MountMortar2FInside_Text_83DFF40::
	.string "I’m investigating MT.MORTAR,\n"
	.string "but my pal’s gone AWOL.\p"
	.string "Have you seen a trainer who\n"
	.string "has a SLOWPOKE that can use\l"
	.string "STRENGTH?$"
