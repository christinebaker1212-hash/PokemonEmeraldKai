Johto_SproutTower2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SproutTower2F_EventScript_83FA5DB
	.byte 0

Johto_SproutTower2F_EventScript_83FA5DB::
	call Quetzal_EventScript_840EA62
	end

@ Johto_SproutTower2F object 2 (local id 2, gfx 0x3EB2) at 14,2
Johto_SproutTower2F_EventScript_83FA5E1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4318, 0, Johto_SproutTower2F_Text_83FA627, Johto_SproutTower2F_Text_83FA65C
	msgbox Johto_SproutTower2F_Text_83FA67C, MSGBOX_AUTOCLOSE
	end

@ Johto_SproutTower2F object 1 (local id 1, gfx 0x3EB2) at 3,14
Johto_SproutTower2F_EventScript_83FA5F8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4587, 0, Johto_SproutTower2F_Text_83FA6BB, Johto_SproutTower2F_Text_83FA6DA
	msgbox Johto_SproutTower2F_Text_83FA6E8, MSGBOX_AUTOCLOSE
	end

@ Johto_SproutTower2F object 3 (local id 3, gfx 0x3A9E) at 1,2
Johto_SproutTower2F_EventScript_83FA61A::
	setorcopyvar VAR_0x8000, ITEM_X_DEFEND
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_SproutTower2F_Text_83FA627::
	.string "However hard we battle,\n"
	.string "the TOWER will stand strong.$"

Johto_SproutTower2F_Text_83FA65C::
	.string "I fought hard but I’m too weak.$"

Johto_SproutTower2F_Text_83FA67C::
	.string "The flexible pillar protects the\n"
	.string "TOWER, even from earthquakes.$"

Johto_SproutTower2F_Text_83FA6BB::
	.string "…Sway like leaves in the wind…$"

Johto_SproutTower2F_Text_83FA6DA::
	.string "Oh, I’m weak!$"

Johto_SproutTower2F_Text_83FA6E8::
	.string "I tried to copy BELLSPROUT’s\n"
	.string "gentle movements for battle…\p"
	.string "But I didn’t train well enough.$"
