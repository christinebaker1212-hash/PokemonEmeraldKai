Johto_UnionCaveB1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_UnionCaveB1F object 1 (local id 1, gfx 0x37) at 9,2
Johto_UnionCaveB1F_EventScript_83FDF6E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4590, 0, Johto_UnionCaveB1F_Text_83FDFE4, Johto_UnionCaveB1F_Text_83FE033
	msgbox Johto_UnionCaveB1F_Text_83FE03B, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCaveB1F object 2 (local id 2, gfx 0x37) at 13,12
Johto_UnionCaveB1F_EventScript_83FDF85::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4591, 0, Johto_UnionCaveB1F_Text_83FE088, Johto_UnionCaveB1F_Text_83FE0A5
	msgbox Johto_UnionCaveB1F_Text_83FE0C3, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCaveB1F object 7 (local id 7, gfx 0x3EB7) at 3,44
Johto_UnionCaveB1F_EventScript_83FDF9C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4432, 0, Johto_UnionCaveB1F_Text_83FE113, Johto_UnionCaveB1F_Text_83FE13F
	msgbox Johto_UnionCaveB1F_Text_83FE14C, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCaveB1F object 6 (local id 6, gfx 0x3EB7) at 20,39
Johto_UnionCaveB1F_EventScript_83FDFB3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4910, 0, Johto_UnionCaveB1F_Text_83FE17E, Johto_UnionCaveB1F_Text_83FE1E7
	msgbox Johto_UnionCaveB1F_Text_83FE1FF, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCaveB1F object 3 (local id 3, gfx 0x3A9E) at 14,3
Johto_UnionCaveB1F_EventScript_83FDFCA::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_UnionCaveB1F object 5 (local id 5, gfx 0x3A9E) at 2,18
Johto_UnionCaveB1F_EventScript_83FDFD7::
	setorcopyvar VAR_0x8000, ITEM_X_DEFEND
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_UnionCaveB1F_Text_83FDFE4::
	.string "It’s been a while since I last saw\n"
	.string "another person.\p"
	.string "Don’t be shy. Let’s battle!$"

Johto_UnionCaveB1F_Text_83FE033::
	.string "Uurggh…$"

Johto_UnionCaveB1F_Text_83FE03B::
	.string "I’ve been lost for a long time…\p"
	.string "I don’t mind it here, but I am\n"
	.string "soooo hungry!$"

Johto_UnionCaveB1F_Text_83FE088::
	.string "What do you know! A visitor!$"

Johto_UnionCaveB1F_Text_83FE0A5::
	.string "Wahahah! You’re a feisty one!$"

Johto_UnionCaveB1F_Text_83FE0C3::
	.string "I live down here.\p"
	.string "You can, too, if you’d like.\p"
	.string "There’s plenty of room, you see.$"

Johto_UnionCaveB1F_Text_83FE113::
	.string "Who’s there?\p"
	.string "Leave me and my POKéMON alone!$"

Johto_UnionCaveB1F_Text_83FE13F::
	.string "Go… Go away!$"

Johto_UnionCaveB1F_Text_83FE14C::
	.string "Just me and my POKéMON. I’m de-\n"
	.string "lirious with joy.$"

Johto_UnionCaveB1F_Text_83FE17E::
	.string "I came all the way here to conduct\n"
	.string "my POKéMON research.\p"
	.string "Let me demonstrate my findings in a\n"
	.string "real battle!$"

Johto_UnionCaveB1F_Text_83FE1E7::
	.string "You demonstrated on me!$"

Johto_UnionCaveB1F_Text_83FE1FF::
	.string "I should compile and announce my\n"
	.string "study findings.\p"
	.string "I might even become famous like\n"
	.string "PROF.ELM.$"
