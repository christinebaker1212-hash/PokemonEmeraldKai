Kanto_SevenIsland_House_Room1_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_House_Room1_EventScript_84209D8
	.byte 0

Kanto_SevenIsland_House_Room1_EventScript_84209D8::
	setmaplayoutindex LAYOUT_KANTO_SEVEN_ISLAND_HOUSE_ROOM1   @ (Quetzal's layout 0x3DB)
	setobjectxyperm 1, 4, 4
	setobjectmovementtype 1, 0xA
	end

@ Kanto_SevenIsland_House_Room1 object 1 (local id 1, gfx 0x3F2F) at 4,4
Kanto_SevenIsland_House_Room1_EventScript_84209E7::
	@ Moemon Kai: FireRed's old woman (pokefirered SevenIsland_House_Room1, without the e-Reader TRAINER: no e-Reader
	@ cards in this game); Quetzal's LANSAT / STARF BERRY gift is gone
	msgbox Kanto_SevenIsland_House_Room1_Text_OnlyEnjoymentWatchingBattles, MSGBOX_NPC
	end

@ Kanto_SevenIsland_House_Room1 sign / bg event at 4,1
Kanto_SevenIsland_House_Room1_EventScript_8420A7D::
	msgbox Kanto_SevenIsland_House_Room1_Text_84509FB, MSGBOX_SIGN
	end

Kanto_SevenIsland_House_Room1_Text_845094B::
	.string "When I was young, I traveled the\n"
	.string "islands looking for new rivals.\p"
	.string "These Berries were with me\n"
	.string "through many battles.\p"
	.string "Now I’d like them to travel with\n"
	.string "you. Take good care of them!$"

Kanto_SevenIsland_House_Room1_Text_84509FB::
	.string "There is a slight breeze blowing\n"
	.string "around the box.$"

Kanto_SevenIsland_House_Room1_Text_8450A32::
	.string "I still have a gift for you.\n"
	.string "Let’s see if you can carry it now.$"

Kanto_SevenIsland_House_Room1_Text_8450A72::
	.string "May they serve you well on\n"
	.string "your next adventures!$"

Kanto_SevenIsland_House_Room1_Text_8450AA3::
	.string "I’ll keep the rest for you.\n"
	.string "Come back when you have room.$"

Kanto_SevenIsland_House_Room1_Text_8450ADD::
	.string "A held Berry can change\n"
	.string "the course of a battle.\p"
	.string "Choose one that suits each\n"
	.string "of your MOéMON.$"

Kanto_SevenIsland_House_Room1_Text_OnlyEnjoymentWatchingBattles::
	.string "I may not look like much now,\n"
	.string "but when I was younger…\p"
	.string "I flew everywhere, challenging\n"
	.string "TRAINERS wherever I found them.\p"
	.string "Now, my only source of enjoyment\n"
	.string "is reminiscing about the past while\l"
	.string "watching battles.$"
