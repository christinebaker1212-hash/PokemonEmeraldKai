Johto_AzaleaGym_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_AzaleaGym_EventScript_83C442A
	.byte 0

Johto_AzaleaGym_EventScript_83C442A::
	setflag FLAG_QZ_0x1035
	setflag FLAG_QZ_0x1036
	setflag FLAG_QZ_0x1037
	end

@ Johto_AzaleaGym trigger at 6,39 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C4434::
	lock
	turnobject LOCALID_PLAYER, 2
	clearflag FLAG_QZ_0x1035
	addobject 8
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	turnobject 8, 2
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	waitmovement 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CE3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D09
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	applymovement 8, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	waitmovement 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	setflag FLAG_QZ_0x1035
	removeobject 8
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 11,27 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C44C4::
	lock
	turnobject LOCALID_PLAYER, 1
	clearflag FLAG_QZ_0x1036
	addobject 9
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	turnobject 9, 1
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	waitmovement 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CEC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D14
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	applymovement 9, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	setflag FLAG_QZ_0x1036
	removeobject 9
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 11,39 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C455D::
	lock
	turnobject LOCALID_PLAYER, 2
	clearflag FLAG_QZ_0x1036
	addobject 9
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	turnobject 9, 2
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	waitmovement 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CE3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D09
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	applymovement 9, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	applymovement 9, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	setflag FLAG_QZ_0x1036
	removeobject 9
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 16,27 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C45F0::
	lock
	turnobject LOCALID_PLAYER, 1
	clearflag FLAG_QZ_0x1037
	addobject 10
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	turnobject 10, 1
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	waitmovement 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CEC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D14
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	applymovement 10, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	setflag FLAG_QZ_0x1037
	removeobject 10
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 16,39 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C4689::
	lock
	turnobject LOCALID_PLAYER, 2
	clearflag FLAG_QZ_0x1037
	addobject 10
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	turnobject 10, 2
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	waitmovement 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CEC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D14
	waitmovement 0
	waitmovement 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CEC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D14
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	applymovement 10, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	applymovement 10, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	setflag FLAG_QZ_0x1037
	removeobject 10
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 6,27 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C4730::
	lock
	turnobject LOCALID_PLAYER, 1
	clearflag FLAG_QZ_0x1035
	addobject 8
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	turnobject 8, 1
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	waitmovement 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CE3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D09
	waitmovement 0
	waitmovement 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CE3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D09
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	applymovement 8, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	applymovement 8, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	setflag FLAG_QZ_0x1035
	removeobject 8
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 6,23 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C47DD::
	lock
	turnobject LOCALID_PLAYER, 2
	clearflag FLAG_QZ_0x1035
	addobject 11
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	turnobject 11, 2
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	waitmovement 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	setflag FLAG_QZ_0x1035
	removeobject 11
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 6,11 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C4858::
	lock
	turnobject LOCALID_PLAYER, 1
	clearflag FLAG_QZ_0x1035
	addobject 11
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	turnobject 11, 1
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	waitmovement 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	applymovement 11, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	setflag FLAG_QZ_0x1035
	removeobject 11
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 11,23 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C48D9::
	lock
	turnobject LOCALID_PLAYER, 2
	clearflag FLAG_QZ_0x1036
	addobject 12
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	turnobject 12, 2
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	waitmovement 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CE3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D09
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	applymovement 12, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	setflag FLAG_QZ_0x1036
	removeobject 12
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 16,11 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C496C::
	lock
	turnobject LOCALID_PLAYER, 1
	clearflag FLAG_QZ_0x1037
	addobject 13
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	turnobject 13, 1
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	waitmovement 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CEC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D14
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	applymovement 13, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	setflag FLAG_QZ_0x1037
	removeobject 13
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 16,23 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C4A05::
	lock
	turnobject LOCALID_PLAYER, 2
	clearflag FLAG_QZ_0x1037
	addobject 13
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	turnobject 13, 2
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	waitmovement 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CEC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D14
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	applymovement 13, Johto_AzaleaGym_Movement_83C4CDC
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CFF
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_836083F
	playse SE_LEDGE
	waitmovement 0
	applymovement 13, Johto_AzaleaGym_Movement_83C4CD5
	waitmovement 0
	setflag FLAG_QZ_0x1037
	removeobject 13
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym trigger at 11,11 (VAR_TEMP_0 == 0)
Johto_AzaleaGym_EventScript_83C4A98::
	lock
	turnobject LOCALID_PLAYER, 1
	clearflag FLAG_QZ_0x1036
	addobject 12
	setobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM, 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	turnobject 12, 1
	delay 0x1E
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	waitmovement 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	waitmovement 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CE3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4D09
	waitmovement 0
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	applymovement 12, Johto_AzaleaGym_Movement_83C4CD5
	delay 3
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_83C4CF5
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_AzaleaGym_Movement_8360841
	playse SE_LEDGE
	waitmovement 0
	applymovement 12, Johto_AzaleaGym_Movement_83C4CDC
	waitmovement 0
	setflag FLAG_QZ_0x1036
	removeobject 12
	resetobjectsubpriority LOCALID_PLAYER, MAP_JOHTO_AZALEA_GYM
	release
	end

@ Johto_AzaleaGym object 1 (local id 1, gfx 0x3E98) at 12,42
Johto_AzaleaGym_EventScript_83C4B31::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11B3, Johto_AzaleaGym_EventScript_83C4B47
	msgbox Johto_AzaleaGym_Text_83C5518, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_AzaleaGym_EventScript_83C4B47::
	msgbox Johto_AzaleaGym_Text_83C565F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaGym object 7 (local id 7, gfx 0x24) at 6,6
Johto_AzaleaGym_EventScript_83C4B52::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4456, 0, Johto_AzaleaGym_Text_83C522A, Johto_AzaleaGym_Text_83C5260
	msgbox Johto_AzaleaGym_Text_83C527E, MSGBOX_AUTOCLOSE
	end

@ Johto_AzaleaGym object 6 (local id 6, gfx 0x24) at 17,5
Johto_AzaleaGym_EventScript_83C4B69::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4416, 0, Johto_AzaleaGym_Text_83C52C8, Johto_AzaleaGym_Text_83C5327
	msgbox Johto_AzaleaGym_Text_83C5330, MSGBOX_AUTOCLOSE
	end

@ Johto_AzaleaGym object 5 (local id 5, gfx 0x24) at 17,26
Johto_AzaleaGym_EventScript_83C4B80::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4503, 0, Johto_AzaleaGym_Text_83C519D, Johto_AzaleaGym_Text_83C51DE
	msgbox Johto_AzaleaGym_Text_83C51FA, MSGBOX_AUTOCLOSE
	end

@ Johto_AzaleaGym object 3 (local id 3, gfx 0x6) at 5,26
Johto_AzaleaGym_EventScript_83C4B97::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4577, 0, Johto_AzaleaGym_Text_83C535A, Johto_AzaleaGym_Text_83C538B, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_AzaleaGym_Text_83C53AB, MSGBOX_DEFAULT
	release
	end

@ Johto_AzaleaGym object 4 (local id 4, gfx 0x6) at 6,26
Johto_AzaleaGym_EventScript_83C4BB6::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4577, 0, Johto_AzaleaGym_Text_83C5416, Johto_AzaleaGym_Text_83C547E, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_AzaleaGym_Text_83C549E, MSGBOX_DEFAULT
	release
	end

@ Johto_AzaleaGym object 2 (local id 2, gfx 0x3E89) at 11,6
Johto_AzaleaGym_EventScript_83C4BD5::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x14BA, Johto_AzaleaGym_EventScript_83C4C67
	goto_if_set FLAG_QZ_0x11A9, Johto_AzaleaGym_EventScript_83C4C91
	msgbox Johto_AzaleaGym_Text_83C4D89, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_AzaleaGym_EventScript_83C4C10
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4692, 0, Johto_AzaleaGym_Text_83C4E7B
	goto Johto_AzaleaGym_EventScript_83C4C1A

Johto_AzaleaGym_EventScript_83C4C10::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4964, 0, Johto_AzaleaGym_Text_83C4E7B

Johto_AzaleaGym_EventScript_83C4C1A::
	message Johto_AzaleaGym_Text_83C4EE5
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_AzaleaGym_Text_83C4EFC, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B3, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B3
	setflag FLAG_QZ_0x11A9
	addvar VAR_QZ_0x502E, 1
	setvar VAR_0x8008, 2
	call Quetzal_EventScript_83601F0
	setflag FLAG_QZ_0x14BA
	call Johto_AzaleaGym_EventScript_83C4C6E
	setvar VAR_QZ_0x5009, 5
	closemessage
	setflag FLAG_QZ_0x1087
	setflag FLAG_QZ_0x107A
	clearflag FLAG_QZ_0x1093
	clearflag FLAG_QZ_0x108F
	release
	end

Johto_AzaleaGym_EventScript_83C4C67::
	call Johto_AzaleaGym_EventScript_83C4C6E
	release
	end

Johto_AzaleaGym_EventScript_83C4C6E::
	setorcopyvar VAR_0x8000, 0x31B
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602B4
	clearflag FLAG_QZ_0x14BA
	msgbox Johto_AzaleaGym_Text_83C4FCE, MSGBOX_DEFAULT
	return

Johto_AzaleaGym_EventScript_83C4C91::
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_AzaleaGym_EventScript_83C4CB8
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4692, 0, Johto_AzaleaGym_Text_83C4D89, Johto_AzaleaGym_Text_83C4E7B, Johto_AzaleaGym_EventScript_83C4CCA
	goto Johto_AzaleaGym_EventScript_83C4CCA

Johto_AzaleaGym_EventScript_83C4CB8::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4964, 0, Johto_AzaleaGym_Text_83C4D89, Johto_AzaleaGym_Text_83C4E7B, Johto_AzaleaGym_EventScript_83C4CCA

Johto_AzaleaGym_EventScript_83C4CCA::
	msgbox Johto_AzaleaGym_Text_83C5047, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaGym sign / bg event at 9,42
@ Johto_AzaleaGym sign / bg event at 13,42
Johto_AzaleaGym_EventScript_83C4D1F::
	lock
	goto_if_set FLAG_QZ_0x11A9, Johto_AzaleaGym_EventScript_83C4D34
	msgbox Johto_AzaleaGym_Text_83C4D3F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_AzaleaGym_EventScript_83C4D34::
	msgbox Johto_AzaleaGym_Text_83C4D52, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_AzaleaGym_Movement_836083F:
	jump_up
	step_end

Johto_AzaleaGym_Movement_8360841:
	jump_down
	step_end

Johto_AzaleaGym_Movement_83C4CD5:
	delay_8
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_AzaleaGym_Movement_83C4CDC:
	delay_8
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Johto_AzaleaGym_Movement_83C4CE3:
	lock_facing_direction
	delay_8
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	unlock_facing_direction
	step_end

Johto_AzaleaGym_Movement_83C4CEC:
	lock_facing_direction
	delay_8
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	unlock_facing_direction
	step_end

Johto_AzaleaGym_Movement_83C4CF5:
	lock_anim
	delay_8
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	unlock_anim
	unlock_facing_direction
	step_end

Johto_AzaleaGym_Movement_83C4CFF:
	lock_anim
	delay_8
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	unlock_anim
	unlock_facing_direction
	step_end

Johto_AzaleaGym_Movement_83C4D09:
	lock_facing_direction
	lock_anim
	delay_8
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	unlock_anim
	unlock_facing_direction
	step_end

Johto_AzaleaGym_Movement_83C4D14:
	lock_facing_direction
	lock_anim
	delay_8
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	unlock_anim
	unlock_facing_direction
	step_end

Johto_AzaleaGym_Text_83C4D3F::
	.string "AZALEA POKéMON GYM$"

Johto_AzaleaGym_Text_83C4D52::
	.string "AZALEA TOWN POKéMON GYM\p"
	.string "BUGSY’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

Johto_AzaleaGym_Text_83C4D89::
	.string "I’m BUGSY!\p"
	.string "I never lose when it comes\n"
	.string "to bug POKéMON.\p"
	.string "My research is going to make me the\n"
	.string "authority on bug POKéMON!\p"
	.string "For example, did you know\n"
	.string "that the move CUT is\l"
	.string "actually a BUG-type move?\p"
	.string "Let me demonstrate all I’ve\n"
	.string "learned from my studies.$"

Johto_AzaleaGym_Text_83C4E7B::
	.string "Whoa, amazing! You’re an\n"
	.string "expert on POKéMON!\p"
	.string "My research isn’t complete\n"
	.string "yet.\p"
	.string "OK, you win. Take this\n"
	.string "BADGE.$"

Johto_AzaleaGym_Text_83C4EE5::
	.string "{PLAYER} received HIVEBADGE.$"

Johto_AzaleaGym_Text_83C4EFC::
	.string "Do you know the benefits\n"
	.string "of HIVEBADGE?\p"
	.string "If you have it, POKéMON up\n"
	.string "to L30 will obey you,\l"
	.string "even traded ones.\p"
	.string "POKéMON that know CUT will be able\n"
	.string "to use it outside of battle too.\p"
	.string "Here, I also want you to\n"
	.string "have this.$"

Johto_AzaleaGym_Text_83C4FCE::
	.string "That’s BLACK AUGURITE.\p"
	.string "It can awaken a rugged new form\n"
	.string "in a certain Bug-type POKéMON.\p"
	.string "Isn’t that great?\n"
	.string "I discovered it!$"

Johto_AzaleaGym_Text_83C5047::
	.string "Bug POKéMON are deep. There are\n"
	.string "many mysteries to be explored.\p"
	.string "Study your favorites thoroughly.$"

Johto_AzaleaGym_Text_83C519D::
	.string "Bug POKéMON evolve young. So they\n"
	.string "get stronger that much faster.$"

Johto_AzaleaGym_Text_83C51DE::
	.string "Just evolving isn’t enough!$"

Johto_AzaleaGym_Text_83C51FA::
	.string "POKéMON become stronger\n"
	.string "if they evolve. Really!$"

Johto_AzaleaGym_Text_83C522A::
	.string "Bug POKéMON are cool and tough!\n"
	.string "I’ll prove it to you!$"

Johto_AzaleaGym_Text_83C5260::
	.string "You proved how tough you are…$"

Johto_AzaleaGym_Text_83C527E::
	.string "They’re so cool, but most girls\n"
	.string "don’t like bug POKéMON.\p"
	.string "I don’t know why…$"

Johto_AzaleaGym_Text_83C52C8::
	.string "You saved all the SLOWPOKE?\n"
	.string "Whew, you’re mighty!\p"
	.string "But my grown-up POKéMON are\n"
	.string "pretty tough too!$"

Johto_AzaleaGym_Text_83C5327::
	.string "Urrgggh!$"

Johto_AzaleaGym_Text_83C5330::
	.string "I guess I should teach them\n"
	.string "better moves…$"

Johto_AzaleaGym_Text_83C535A::
	.string "AMY: Hi! Are you challenging\n"
	.string "the LEADER? No way!$"

Johto_AzaleaGym_Text_83C538B::
	.string "AMY & MAY: Oh, double\n"
	.string "goodness!$"

Johto_AzaleaGym_Text_83C53AB::
	.string "AMY: You’re really strong!\p"
	.string "But BUGSY’s even stronger!\n"
	.string "His SCYTHER’s CUT almost\l"
	.string "always makes critical hits!$"

Johto_AzaleaGym_Text_83C5416::
	.string "MAY: You want to see the\n"
	.string "LEADER? Good luck!\p"
	.string "BUGSY is so strong!\n"
	.string "His SCYTHER’s CUT always\l"
	.string "seems to crit!$"

Johto_AzaleaGym_Text_83C547E::
	.string "AMY & MAY: Oh, double\n"
	.string "goodness!$"

Johto_AzaleaGym_Text_83C549E::
	.string "MAY: Our bug POKéMON lost!\n"
	.string "Oh, what a shame.\p"
	.string "BUGSY’s won’t lose though!\p"
	.string "His PINECO makes short work\n"
	.string "of ROCK-type POKéMON!$"

Johto_AzaleaGym_Text_83C5518::
	.string "Yo, challenger!\p"
	.string "BUGSY’s young, but his knowledge\n"
	.string "of bug POKéMON is for real.\p"
	.string "It’s going to be tough\n"
	.string "without my advice.\p"
	.string "Let’s see… BUGSY’s bagworm POKéMON\n"
	.string "knows a move that covers its\l"
	.string "ROCK-type weakness.\p"
	.string "He’s also developed a new version\n"
	.string "of CUT that is BUG-type.\p"
	.string "Oh, and don’t forget about those\n"
	.string "nasty critical hits! Good luck!$"

Johto_AzaleaGym_Text_83C565F::
	.string "Well done! That was a great\n"
	.string "clash of talented young trainers.\p"
	.string "With people like you, the\n"
	.string "future of POKéMON is bright!$"
