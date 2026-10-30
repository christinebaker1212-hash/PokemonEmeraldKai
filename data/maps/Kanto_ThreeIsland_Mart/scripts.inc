Kanto_ThreeIsland_Mart_MapScripts::
	.byte 0

@ Kanto_ThreeIsland_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_ThreeIsland_Mart_EventScript_84215E7::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

@ Kanto_ThreeIsland_Mart object 2 (local id 2, gfx 0x3F31) at 2,7
Kanto_ThreeIsland_Mart_EventScript_8421620::
	msgbox Kanto_ThreeIsland_Mart_Text_84536AC, MSGBOX_NPC
	end

@ Kanto_ThreeIsland_Mart object 3 (local id 3, gfx 0x3EF1) at 7,2
Kanto_ThreeIsland_Mart_EventScript_8421629::
	msgbox Kanto_ThreeIsland_Mart_Text_84536F4, MSGBOX_NPC
	end

@ Kanto_ThreeIsland_Mart object 4 (local id 4, gfx 0x3F5C) at 6,5
Kanto_ThreeIsland_Mart_EventScript_8421632::
	msgbox Kanto_ThreeIsland_Mart_Text_8453788, MSGBOX_NPC
	end

Kanto_ThreeIsland_Mart_Text_84536AC::
	.string "Is it true?\p"
	.string "CELADON DEPT. STORE is several\n"
	.string "times bigger than this shop?$"

Kanto_ThreeIsland_Mart_Text_84536F4::
	.string "I sometimes buy medicine here.\p"
	.string "But a lot of people heal POKéMON\n"
	.string "with BERRIES from BERRY FOREST.\p"
	.string "After all, BERRIES are free and\n"
	.string "they never run out.$"

Kanto_ThreeIsland_Mart_Text_8453788::
	.string "Those BIKERS were about to trash\n"
	.string "this POKéMON MART.\p"
	.string "Am I glad they decided to leave!$"
