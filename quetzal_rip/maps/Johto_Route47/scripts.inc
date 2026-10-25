Johto_Route47_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_Route47 object 1 (local id 1, gfx 0x37) at 89,39
Johto_Route47_EventScript_83F6669::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4128, 0, Johto_Route47_Text_83F6680, Johto_Route47_Text_83F66CC
	msgbox Johto_Route47_Text_83F66D4, MSGBOX_AUTOCLOSE
	end

@ Johto_Route47 object 2 (local id 2, gfx 0x1F) at 71,37
Johto_Route47_EventScript_83F6711::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4129, 0, Johto_Route47_Text_83F6728, Johto_Route47_Text_83F6771
	msgbox Johto_Route47_Text_83F67A8, MSGBOX_AUTOCLOSE
	end

@ Johto_Route47 object 5 (local id 5, gfx 0x3E90) at 86,30
Johto_Route47_EventScript_83F67B0::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4616, 0, Johto_Route47_Text_83F67EC, Johto_Route47_Text_83F6865, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_Route47_Text_83F68A1, MSGBOX_AUTOCLOSE
	end

@ Johto_Route47 object 4 (local id 4, gfx 0x3E8F) at 86,29
Johto_Route47_EventScript_83F67CE::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4616, 0, Johto_Route47_Text_83F67EC, Johto_Route47_Text_83F6865, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_Route47_Text_83F68D5, MSGBOX_AUTOCLOSE
	end

@ Johto_Route47 object 7 (local id 7, gfx 0x3EBF) at 57,15
Johto_Route47_EventScript_83F6918::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4617, 0, Johto_Route47_Text_83F6951, Johto_Route47_Text_83F69D2, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_Route47_Text_83F6A13, MSGBOX_AUTOCLOSE
	end

@ Johto_Route47 object 8 (local id 8, gfx 0x3EBE) at 58,15
Johto_Route47_EventScript_83F6936::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4617, 0, Johto_Route47_Text_83F6951, Johto_Route47_Text_83F69D2, Quetzal_Text_8383BB5
	msgbox Johto_Route47_Text_83F6A47, MSGBOX_AUTOCLOSE
	end

@ Johto_Route47 object 9 (local id 9, gfx 0x3B) at 83,14
Johto_Route47_EventScript_83F6CB2::
	setorcopyvar VAR_0x8000, 0x8C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route47 object 3 (local id 3, gfx 0x3B) at 72,43
Johto_Route47_EventScript_83F6CBF::
	setorcopyvar VAR_0x8000, 0x21
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route47 object 6 (local id 6, gfx 0x3B) at 59,33
Johto_Route47_EventScript_83F6CCC::
	setorcopyvar VAR_0x8000, 0x71
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route47 object 10 (local id 10, gfx 0x3B) at 31,13
Johto_Route47_EventScript_83F6CD9::
	setorcopyvar VAR_0x8000, 0x8F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route47 object 12 (local id 12, gfx 0x3B) at 32,10
Johto_Route47_EventScript_83F6CE6::
	setorcopyvar VAR_0x8000, 0x18B
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route47_Text_83F6680::
	.string "Longing for others is...\n"
	.string "what it is. \p"
	.string "But battles are battles,\n"
	.string "so let’s go!$"

Johto_Route47_Text_83F66CC::
	.string "Wahaha!$"

Johto_Route47_Text_83F66D4::
	.string "After a battle you should always leave\n"
	.string "with a parting laugh!$"

Johto_Route47_Text_83F6728::
	.string "I’m not good with large crowds, so\n"
	.string "I’m camping out here with my\l"
	.string "POKéMON!$"

Johto_Route47_Text_83F6771::
	.string "I should be headed deeper into the\n"
	.string "mountain I guess...$"

Johto_Route47_Text_83F67A8::
	.string "See ya!$"

Johto_Route47_Text_83F67EC::
	.string "THOM: One plus one is two. But the\n"
	.string "two of us together is much\l"
	.string "stronger!\p"
	.string "KAE: The two of us aren’t just some\n"
	.string "boring pair!$"

Johto_Route47_Text_83F6865::
	.string "THOM: That wasn’t what I expected.\l"
	.string "KAE: You’re unusual too!$"

Johto_Route47_Text_83F68A1::
	.string "THOM: It seems your power is\n"
	.string "impossible to predict!$"

Johto_Route47_Text_83F68D5::
	.string "KAE: I figured you were an ordinary\n"
	.string "lone trainer. Boy was I wrong!$"

Johto_Route47_Text_83F6951::
	.string "DUFF: Watching over my girlfriend\n"
	.string "makes my heart soar!\p"
	.string "EDA: My heart’s feelings for my\n"
	.string "boyfriend are as wide and deep\l"
	.string "the ocean!$"

Johto_Route47_Text_83F69D2::
	.string "DUFF: I was no match for you...\l"
	.string "EDA: You are really something...$"

Johto_Route47_Text_83F6A13::
	.string "DUFF: Thank you for the awe-\n"
	.string "inspiring battle! Bye!$"

Johto_Route47_Text_83F6A47::
	.string "EDA: You should really find a\n"
	.string "wonderful partner, too!$"
