Johto_UnionCaveB2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_UnionCaveB2F object 2 (local id 2, gfx 0x3E90) at 6,12
Johto_UnionCaveB2F_EventScript_83FE260::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4317, 0, Johto_UnionCaveB2F_Text_83FE2CC, Johto_UnionCaveB2F_Text_83FE333
	msgbox Johto_UnionCaveB2F_Text_83FE34E, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCaveB2F object 4 (local id 4, gfx 0x3E8F) at 17,20
Johto_UnionCaveB2F_EventScript_83FE277::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4155, 0, Johto_UnionCaveB2F_Text_83FE3A5, Johto_UnionCaveB2F_Text_83FE3C8
	msgbox Johto_UnionCaveB2F_Text_83FE3E9, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCaveB2F object 5 (local id 5, gfx 0x3E8F) at 2,27
Johto_UnionCaveB2F_EventScript_83FE28E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4190, 0, Johto_UnionCaveB2F_Text_83FE417, Johto_UnionCaveB2F_Text_83FE46C
	msgbox Johto_UnionCaveB2F_Text_83FE490, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCaveB2F object 1 (local id 1, gfx 0x3A9E) at 23,2
Johto_UnionCaveB2F_EventScript_83FE2A5::
	setorcopyvar VAR_0x8000, ITEM_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_UnionCaveB2F object 6 (local id 6, gfx 0x3A9E) at 3,12
Johto_UnionCaveB2F_EventScript_83FE2B2::
	setorcopyvar VAR_0x8000, ITEM_HYPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_UnionCaveB2F object 3 (local id 3, gfx 0x3A9E) at 16,21
Johto_UnionCaveB2F_EventScript_83FE2BF::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_UnionCaveB2F_Text_83FE2CC::
	.string "There are two kinds of people.\p"
	.string "Those who have style, and those\n"
	.string "who don’t.\p"
	.string "What kind of person are you?$"

Johto_UnionCaveB2F_Text_83FE333::
	.string "You’ve got dazzling style!$"

Johto_UnionCaveB2F_Text_83FE34E::
	.string "Your POKéMON style is stunning and\n"
	.string "colorful, I admit.\p"
	.string "You’ll just keep getting better!$"

Johto_UnionCaveB2F_Text_83FE3A5::
	.string "I’m in training. Care for a round?$"

Johto_UnionCaveB2F_Text_83FE3C8::
	.string "Aww, no! You’re too good for me.$"

Johto_UnionCaveB2F_Text_83FE3E9::
	.string "I’m going to train by myself until I\n"
	.string "improve.$"

Johto_UnionCaveB2F_Text_83FE417::
	.string "If the POKéMON I liked were there,\n"
	.string "I’d go anywhere.\p"
	.string "That’s what a real trainer does.$"

Johto_UnionCaveB2F_Text_83FE46C::
	.string "I’d rather pet my babies than this!$"

Johto_UnionCaveB2F_Text_83FE490::
	.string "Just once a week, a POKéMON comes to\n"
	.string "the water’s edge.\p"
	.string "I wanted to see that POKéMON…$"
