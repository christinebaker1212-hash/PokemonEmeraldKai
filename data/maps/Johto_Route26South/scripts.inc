Johto_Route26South_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route26South_EventScript_83EBC5E
	.byte 0

Johto_Route26South_EventScript_83EBC5E::
	call Quetzal_EventScript_840EA62
	end

@ Johto_Route26South object 4 (local id 4, gfx 0x3E90) at 19,13
Johto_Route26South_EventScript_83EBC7B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4334, 0, Johto_Route26South_Text_83EBDAB, Johto_Route26South_Text_83EBDDA
	msgbox Johto_Route26South_Text_83EBDF6, MSGBOX_AUTOCLOSE
	end

@ Johto_Route26South object 1 (local id 1, gfx 0x25) at 19,40
Johto_Route26South_EventScript_83EBCC0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4262, 0, Johto_Route26South_Text_83EC022, Johto_Route26South_Text_83EC089
	msgbox Johto_Route26South_Text_83EC096, MSGBOX_AUTOCLOSE
	end

Johto_Route26South_Text_83EBDAB::
	.string "By experiencing tough battles, you\n"
	.string "gain power.$"

Johto_Route26South_Text_83EBDDA::
	.string "Gaah! Life is even tougher!$"

Johto_Route26South_Text_83EBDF6::
	.string "To get to POKéMON LEAGUE, you have\n"
	.string "to get through VICTORY ROAD.\p"
	.string "But VICTORY ROAD is tough.\p"
	.string "Practically nobody goes there!$"

Johto_Route26South_Text_83EC022::
	.string "Wow, look at all those BADGES!\n"
	.string "I’m impressed.\p"
	.string "But you’re not satisfied by just\n"
	.string "collecting them, right?$"

Johto_Route26South_Text_83EC089::
	.string "Good battle!$"

Johto_Route26South_Text_83EC096::
	.string "People and POKéMON grow from their\n"
	.string "experiences.\p"
	.string "Don’t get lazy and complacent.$"
