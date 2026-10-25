Kanto_SevenIsland_TrainerTower_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_TrainerTower_EventScript_84189DA
	.byte 0

@ Kanto_SevenIsland_TrainerTower object 3 (local id 3, gfx 0x144) at 58,8
Kanto_SevenIsland_TrainerTower_EventScript_8366160::
	lockall
	msgbox Kanto_SevenIsland_TrainerTower_Text_83684BF, MSGBOX_DEFAULT
	releaseall
	end

Kanto_SevenIsland_TrainerTower_EventScript_84189DA::
	setvar VAR_QZ_0x5132, 0
	end

@ Kanto_SevenIsland_TrainerTower sign / bg event at 56,8
Kanto_SevenIsland_TrainerTower_EventScript_84189E0::
	msgbox Kanto_SevenIsland_TrainerTower_Text_843C650, MSGBOX_SIGN
	end

@ Kanto_SevenIsland_TrainerTower sign / bg event at 50,30
Kanto_SevenIsland_TrainerTower_EventScript_84189E9::
	msgbox Kanto_SevenIsland_TrainerTower_Text_843C688, MSGBOX_SIGN
	end

@ Kanto_SevenIsland_TrainerTower object 1 (local id 1, gfx 0x3EEE) at 56,26
Kanto_SevenIsland_TrainerTower_EventScript_845CC1E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8778, 0, Kanto_SevenIsland_TrainerTower_Text_843C505, Kanto_SevenIsland_TrainerTower_Text_843C51D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_TrainerTower_EventScript_845CC45
	msgbox Kanto_SevenIsland_TrainerTower_Text_843C526, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TrainerTower_EventScript_845CC45::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8778, 0, Kanto_SevenIsland_TrainerTower_Text_847177C, Kanto_SevenIsland_TrainerTower_Text_843C51D
	msgbox Kanto_SevenIsland_TrainerTower_Text_843C526, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_TrainerTower object 2 (local id 2, gfx 0x3F57) at 56,29
Kanto_SevenIsland_TrainerTower_EventScript_845CC5C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8779, 0, Kanto_SevenIsland_TrainerTower_Text_843C59B, Kanto_SevenIsland_TrainerTower_Text_843C5D4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_TrainerTower_EventScript_845CC83
	msgbox Kanto_SevenIsland_TrainerTower_Text_843C5ED, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TrainerTower_EventScript_845CC83::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8779, 0, Kanto_SevenIsland_TrainerTower_Text_8471794, Kanto_SevenIsland_TrainerTower_Text_843C5D4
	msgbox Kanto_SevenIsland_TrainerTower_Text_843C5ED, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TrainerTower_Text_83684BF::
	.string "Under construction...$"

Kanto_SevenIsland_TrainerTower_Text_843C505::
	.string "I sensed your approach.$"

Kanto_SevenIsland_TrainerTower_Text_843C51D::
	.string "Give up…$"

Kanto_SevenIsland_TrainerTower_Text_843C526::
	.string "In your future, I sense that you\n"
	.string "will meet many others.\p"
	.string "I think you will be linked to each\n"
	.string "other by a strange power.$"

Kanto_SevenIsland_TrainerTower_Text_843C59B::
	.string "Somewhere on this island, peculiar\n"
	.string "POKéMON are sleeping.$"

Kanto_SevenIsland_TrainerTower_Text_843C5D4::
	.string "What an amazing TRAINER!$"

Kanto_SevenIsland_TrainerTower_Text_843C5ED::
	.string "Somewhere on this island, peculiar\n"
	.string "POKéMON are sleeping.\p"
	.string "Could it be that they’re waiting\n"
	.string "for you?$"

Kanto_SevenIsland_TrainerTower_Text_843C650::
	.string "TRAINER TOWER ENTRANCE\n"
	.string "Rise to the Challenge, TRAINERS!$"

Kanto_SevenIsland_TrainerTower_Text_843C688::
	.string "TRAINER TOWER AHEAD$"

Kanto_SevenIsland_TrainerTower_Text_847177C::
	.string "I sensed your approach.$"

Kanto_SevenIsland_TrainerTower_Text_8471794::
	.string "Somewhere on this island, peculiar\n"
	.string "POKéMON are sleeping.$"
