Johto_Route43_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route43_EventScript_83F506D
	.byte 0

Johto_Route43_EventScript_83F506D::
	call_if_set FLAG_QZ_0x10B9, Quetzal_EventScript_83F4AD7
	end

@ Johto_Route43 object 3 (local id 3, gfx 0x11) at 16,16
Johto_Route43_EventScript_83F5077::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4419, 0, Johto_Route43_Text_83F5120, Johto_Route43_Text_83F516F
	msgbox Johto_Route43_Text_83F518C, MSGBOX_AUTOCLOSE
	end

@ Johto_Route43 object 9 (local id 9, gfx 0x3EB7) at 23,20
Johto_Route43_EventScript_83F508E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4319, 0, Johto_Route43_Text_83F51D6, Johto_Route43_Text_83F51F9
	msgbox Johto_Route43_Text_83F5218, MSGBOX_AUTOCLOSE
	end

@ Johto_Route43 object 8 (local id 8, gfx 0x3EB7) at 25,24
Johto_Route43_EventScript_83F50A5::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4936, 0, Johto_Route43_Text_83F5248, Johto_Route43_Text_83F52A1
	msgbox Johto_Route43_Text_83F52C2, MSGBOX_AUTOCLOSE
	end

@ Johto_Route43 object 4 (local id 4, gfx 0x3E95) at 7,15
Johto_Route43_EventScript_83F50BC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4838, 0, Johto_Route43_Text_83F5332, Johto_Route43_Text_83F5384
	msgbox Johto_Route43_Text_83F53A6, MSGBOX_AUTOCLOSE
	end

@ Johto_Route43 object 1 (local id 1, gfx 0x1F) at 14,40
Johto_Route43_EventScript_83F50D3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4255, 0, Johto_Route43_Text_83F540C, Johto_Route43_Text_83F543E
	msgbox Johto_Route43_Text_83F5457, MSGBOX_AUTOCLOSE
	end

@ Johto_Route43 object 2 (local id 2, gfx 0x20) at 10,24
Johto_Route43_EventScript_83F50EA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4227, 0, Johto_Route43_Text_83F5499, Johto_Route43_Text_83F54DB
	msgbox Johto_Route43_Text_83F54EE, MSGBOX_AUTOCLOSE
	end

@ Johto_Route43 sign / bg event at 17,3
@ Johto_Route43 sign / bg event at 14,47
Johto_Route43_EventScript_83F5101::
	msgbox Johto_Route43_Text_83F5549, MSGBOX_SIGN
	end

@ Johto_Route43 sign / bg event at 21,39
Johto_Route43_EventScript_83F510A::
	msgbox Johto_Route43_Text_83F556F, MSGBOX_SIGN
	end

@ Johto_Route43 object 5 (local id 5, gfx 0x3A9E) at 4,23
Johto_Route43_EventScript_83F5113::
	setorcopyvar VAR_0x8000, 0x32
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route43_Text_83F5120::
	.string "I love POKéMON!\p"
	.string "That’s why I started--and why I’ll\n"
	.string "keep on collecting POKéMON!$"

Johto_Route43_Text_83F516F::
	.string "How could you do this to me?$"

Johto_Route43_Text_83F518C::
	.string "What else do I like besides\n"
	.string "POKéMON?\p"
	.string "MARY on the radio. I bet she’s\n"
	.string "cute!$"

Johto_Route43_Text_83F51D6::
	.string "Hey! Do you have any rare\n"
	.string "POKéMON?$"

Johto_Route43_Text_83F51F9::
	.string "Oh, my poor POKéMON! Darlings!$"

Johto_Route43_Text_83F5218::
	.string "I’d be happy just to own a single\n"
	.string "rare POKéMON.$"

Johto_Route43_Text_83F5248::
	.string "Would you get this?\p"
	.string "Some SILVER guy made fun of my\n"
	.string "POKéMON!\p"
	.string "Darn it! My POKéMON’s great!$"

Johto_Route43_Text_83F52A1::
	.string "My NIDOKING did pretty right on!$"

Johto_Route43_Text_83F52C2::
	.string "It’s okay for people to like\n"
	.string "different types of POKéMON.\p"
	.string "POKéMON isn’t just about having the\n"
	.string "most powerful one.$"

Johto_Route43_Text_83F5332::
	.string "I’m in a slump.\p"
	.string "Maybe it’s the gear I’m using.\p"
	.string "Let’s battle for a change of pace!$"

Johto_Route43_Text_83F5384::
	.string "I lost, but I feel better anyway.$"

Johto_Route43_Text_83F53A6::
	.string "KURT’s LURE BALL is the best for\n"
	.string "catching hooked POKéMON.\p"
	.string "It’s much more effective than a\n"
	.string "ULTRA BALL.$"

Johto_Route43_Text_83F540C::
	.string "I can do so much with my\n"
	.string "POKéMON--it’s super-fun!$"

Johto_Route43_Text_83F543E::
	.string "Losing isn’t fun at all…$"

Johto_Route43_Text_83F5457::
	.string "What is going on at LAKE OF RAGE?\p"
	.string "We were planning to camp there.$"

Johto_Route43_Text_83F5499::
	.string "Are you going to LAKE OF RAGE too?\p"
	.string "Let’s play for a little while!$"

Johto_Route43_Text_83F54DB::
	.string "I played too much!$"

Johto_Route43_Text_83F54EE::
	.string "I’m having a picnic with POKéMON.\p"
	.string "Won’t you join us?$"

Johto_Route43_Text_83F5549::
	.string "ROUTE 43\p"
	.string "LAKE OF RAGE - MAHOGANY TOWN$"

Johto_Route43_Text_83F556F::
	.string "TRAINER TIPS\p"
	.string "All POKéMON have pros and cons\n"
	.string "depending on their types.\p"
	.string "If their types differ, a higher-\n"
	.string "level POKéMON may lose in battle.\p"
	.string "Learn which types are strong and\n"
	.string "weak against your POKéMON’s type.$"
