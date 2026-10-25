Johto_MountMortar1FInside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_MountMortar1FInside_EventScript_83DFC5D
	.byte 0

Johto_MountMortar1FInside_EventScript_83DFC5D::
	call Quetzal_EventScript_840EA62
	end

@ Johto_MountMortar1FInside object 1 (local id 1, gfx 0x3A9E) at 16,10
Johto_MountMortar1FInside_EventScript_83DFC63::
	setorcopyvar VAR_0x8000, 0x74
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FInside object 2 (local id 2, gfx 0x3A9E) at 35,19
Johto_MountMortar1FInside_EventScript_83DFC70::
	setorcopyvar VAR_0x8000, 0x78
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FInside object 5 (local id 5, gfx 0x3A9E) at 8,16
Johto_MountMortar1FInside_EventScript_83DFC7D::
	setorcopyvar VAR_0x8000, 0x1F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FInside object 4 (local id 4, gfx 0x3A9E) at 16,17
Johto_MountMortar1FInside_EventScript_83DFC8A::
	setorcopyvar VAR_0x8000, 0x87
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FInside object 3 (local id 3, gfx 0x3A9E) at 22,20
Johto_MountMortar1FInside_EventScript_83DFC97::
	setorcopyvar VAR_0x8000, 3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FInside object 6 (local id 6, gfx 0x3A9E) at 11,27
Johto_MountMortar1FInside_EventScript_83DFCA4::
	setorcopyvar VAR_0x8000, 0x21
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FInside object 7 (local id 7, gfx 0x3A9E) at 35,38
Johto_MountMortar1FInside_EventScript_83DFCB1::
	setorcopyvar VAR_0x8000, 0x1D1
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FInside object 9 (local id 9, gfx 0x11) at 34,44
Johto_MountMortar1FInside_EventScript_83DFCBE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4595, 0, Johto_MountMortar1FInside_Text_83DFCEC, Johto_MountMortar1FInside_Text_83DFD06
	msgbox Johto_MountMortar1FInside_Text_83DFD1D, MSGBOX_AUTOCLOSE
	end

@ Johto_MountMortar1FInside object 8 (local id 8, gfx 0x3EB7) at 23,27
Johto_MountMortar1FInside_EventScript_83DFCD5::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4613, 0, Johto_MountMortar1FInside_Text_83DFDC9, Johto_MountMortar1FInside_Text_83DFDD4
	msgbox Johto_MountMortar1FInside_Text_83DFDF4, MSGBOX_AUTOCLOSE
	end

Johto_MountMortar1FInside_Text_83DFCEC::
	.string "I’m not losing this time!$"

Johto_MountMortar1FInside_Text_83DFD06::
	.string "I lost to some kid...?$"

Johto_MountMortar1FInside_Text_83DFD1D::
	.string "A while back, this karate dude\n"
	.string "wanted to battle.\p"
	.string "He was ridiculously good.\n"
	.string "He just thrashed us silly.\p"
	.string "He went in deeper saying it\n"
	.string "was for his training.\p"
	.string "I wonder how he is?$"

Johto_MountMortar1FInside_Text_83DFDC9::
	.string "Hey! HUGH!$"

Johto_MountMortar1FInside_Text_83DFDD4::
	.string "I mistook you for someone\n"
	.string "else…$"

Johto_MountMortar1FInside_Text_83DFDF4::
	.string "I came to explore MT.MORTAR,\n"
	.string "but I got separated from my\l"
	.string "partner…\p"
	.string "Did you run into a trainer\n"
	.string "who uses a SEADRA that knows\l"
	.string "WATERFALL?$"
