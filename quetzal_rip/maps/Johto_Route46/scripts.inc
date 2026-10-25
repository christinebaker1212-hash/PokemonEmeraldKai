Johto_Route46_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83ECB9A
	.byte 0

@ Johto_Route46 object 4 (local id 4, gfx 0x37) at 19,25
Johto_Route46_EventScript_83F63B9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4766, 0, Johto_Route46_Text_83F6417, Johto_Route46_Text_83F644D
	msgbox Johto_Route46_Text_83F646E, MSGBOX_AUTOCLOSE
	end

@ Johto_Route46 object 2 (local id 2, gfx 0x1F) at 6,18
Johto_Route46_EventScript_83F63D0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4370, 0, Johto_Route46_Text_83F64A0, Johto_Route46_Text_83F64D2
	msgbox Johto_Route46_Text_83F64D8, MSGBOX_AUTOCLOSE
	end

@ Johto_Route46 object 3 (local id 3, gfx 0x20) at 4,18
Johto_Route46_EventScript_83F63E7::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4199, 0, Johto_Route46_Text_83F6515, Johto_Route46_Text_83F6543
	msgbox Johto_Route46_Text_83F654D, MSGBOX_AUTOCLOSE
	end

@ Johto_Route46 sign / bg event at 11,35
Johto_Route46_EventScript_83F63FE::
	lock
	msgbox Johto_Route46_Text_83F6647, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route46 object 1 (local id 1, gfx 0x3A9E) at 2,21
Johto_Route46_EventScript_83F640A::
	setorcopyvar VAR_0x8000, 0x21
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route46_Text_83F6417::
	.string "Awright! I’ll show you the\n"
	.string "power of mountain POKéMON!$"

Johto_Route46_Text_83F644D::
	.string "Mercy! You showed me your\n"
	.string "power!$"

Johto_Route46_Text_83F646E::
	.string "It’s over. I don’t mind.\n"
	.string "We HIKERS are like that.$"

Johto_Route46_Text_83F64A0::
	.string "I’m raising POKéMON too!\p"
	.string "Will you battle with me?$"

Johto_Route46_Text_83F64D2::
	.string "Wha…?$"

Johto_Route46_Text_83F64D8::
	.string "I did my best but came up\n"
	.string "short.\p"
	.string "No excuses--I admit I lost.$"

Johto_Route46_Text_83F6515::
	.string "I raise POKéMON too!\p"
	.string "Will you battle with me?$"

Johto_Route46_Text_83F6543::
	.string "Oh, rats!$"

Johto_Route46_Text_83F654D::
	.string "I’ve been to many GYMS, but the\n"
	.string "GYM in GOLDENROD is my favorite.\p"
	.string "It’s filled with pretty flowers!$"

Johto_Route46_Text_83F6647::
	.string "ROUTE 46\n"
	.string "MOUNTAIN RD. AHEAD$"
