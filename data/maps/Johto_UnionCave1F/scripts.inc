Johto_UnionCave1F_MapScripts::
	.byte 0

@ Johto_UnionCave1F trigger at 33,32 (VAR_TEMP_0 == 0)
Johto_UnionCave1F_EventScript_83FDB22::
	special SpawnCameraObject
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083B
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083B
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083B
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083B
	waitmovement 0
	message Quetzal_Text_83ECC08
	waitbuttonpress
	closemessage
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083D
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083D
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083D
	waitmovement 0
	applymovement LOCALID_CAMERA, Quetzal_Movement_836083D
	waitmovement 0
	special RemoveCameraObject
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	end

@ Johto_UnionCave1F object 3 (local id 3, gfx 0x37) at 24,3
Johto_UnionCave1F_EventScript_83FDB8A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4744, 0, Johto_UnionCave1F_Text_83FDC31, Johto_UnionCave1F_Text_83FDC88
	msgbox Johto_UnionCave1F_Text_83FDC94, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCave1F object 7 (local id 7, gfx 0x3EB7) at 7,31
Johto_UnionCave1F_EventScript_83FDBA1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4309, 0, Johto_UnionCave1F_Text_83FDCE1, Johto_UnionCave1F_Text_83FDD4D
	msgbox Johto_UnionCave1F_Text_83FDD63, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCave1F object 2 (local id 2, gfx 0x37) at 16,13
Johto_UnionCave1F_EventScript_83FDBB8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4308, 0, Johto_UnionCave1F_Text_83FDDA7, Johto_UnionCave1F_Text_83FDDE2
	msgbox Johto_UnionCave1F_Text_83FDDFD, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCave1F object 5 (local id 5, gfx 0x3E94) at 27,30
Johto_UnionCave1F_EventScript_83FDBCF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4846, 0, Johto_UnionCave1F_Text_83FDE49, Johto_UnionCave1F_Text_83FDE98
	msgbox Johto_UnionCave1F_Text_83FDEA6, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCave1F object 8 (local id 8, gfx 0x3E94) at 25,37
Johto_UnionCave1F_EventScript_83FDBE6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4750, 0, Johto_UnionCave1F_Text_83FDEE5, Johto_UnionCave1F_Text_83FDF30
	msgbox Johto_UnionCave1F_Text_83FDF37, MSGBOX_AUTOCLOSE
	end

@ Johto_UnionCave1F object 1 (local id 1, gfx 0x3A9E) at 7,3
Johto_UnionCave1F_EventScript_83FDBFD::
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_UnionCave1F object 6 (local id 6, gfx 0x3A9E) at 32,32
Johto_UnionCave1F_EventScript_83FDC0A::
	setorcopyvar VAR_0x8000, ITEM_GREAT_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_UnionCave1F object 9 (local id 9, gfx 0x3A9E) at 22,55
Johto_UnionCave1F_EventScript_83FDC17::
	setorcopyvar VAR_0x8000, ITEM_X_ATTACK
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_UnionCave1F object 4 (local id 4, gfx 0x3A9E) at 11,18
Johto_UnionCave1F_EventScript_83FDC24::
	setorcopyvar VAR_0x8000, ITEM_AWAKENING
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_UnionCave1F_Text_83FDC31::
	.string "You’re headed to AZALEA, are you?\p"
	.string "Let my POKéMON see if you are good\n"
	.string "enough to battle.$"

Johto_UnionCave1F_Text_83FDC88::
	.string "Oh, oh, oh!$"

Johto_UnionCave1F_Text_83FDC94::
	.string "All right, then! I’ve decided.\p"
	.string "I’m not leaving until my POKéMON\n"
	.string "get tougher!$"

Johto_UnionCave1F_Text_83FDCE1::
	.string "I roam far and wide in search of\n"
	.string "POKéMON.\p"
	.string "Are you looking for POKéMON too?\p"
	.string "Then you’re my collecting rival!$"

Johto_UnionCave1F_Text_83FDD4D::
	.string "Ugh. My poor POKéMON…$"

Johto_UnionCave1F_Text_83FDD63::
	.string "Every Friday, you can hear POKéMON\n"
	.string "roars from deep inside the cave.$"

Johto_UnionCave1F_Text_83FDDA7::
	.string "Whoa! What a surprise!\p"
	.string "I didn’t expect to see anyone here!$"

Johto_UnionCave1F_Text_83FDDE2::
	.string "Whoa! I’m beaten big time!$"

Johto_UnionCave1F_Text_83FDDFD::
	.string "I was conned into buying a\n"
	.string "SLOWPOKETAIL.\p"
	.string "I feel sorry for the poor POKéMON.$"

Johto_UnionCave1F_Text_83FDE49::
	.string "ZUBAT’s SUPERSONIC keeps confusing\n"
	.string "my POKéMON.\p"
	.string "I’m seriously upset about that!$"

Johto_UnionCave1F_Text_83FDE98::
	.string "I flamed out!$"

Johto_UnionCave1F_Text_83FDEA6::
	.string "On weekends, you can hear strange\n"
	.string "roars from deep in the cave.$"

Johto_UnionCave1F_Text_83FDEE5::
	.string "If it’s light, a cave isn’t scary.\p"
	.string "If you’re strong, POKéMON aren’t\n"
	.string "scary.$"

Johto_UnionCave1F_Text_83FDF30::
	.string "FLASH!$"

Johto_UnionCave1F_Text_83FDF37::
	.string "It’s my POKéMON’s fire that lights\n"
	.string "up this cave.$"
