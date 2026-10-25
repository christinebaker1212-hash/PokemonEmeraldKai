Kanto_SevenIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_EventScript_8417B51
	.byte 0

Kanto_SevenIsland_EventScript_8417B51::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2252
	end

@ Kanto_SevenIsland sign / bg event at 17,9
Kanto_SevenIsland_EventScript_8417B58::
	msgbox Kanto_SevenIsland_Text_843330D, MSGBOX_SIGN
	end

@ Kanto_SevenIsland object 2 (local id 2, gfx 0x3F2F) at 9,7
Kanto_SevenIsland_EventScript_8417B61::
	msgbox Kanto_SevenIsland_Text_8433333, MSGBOX_NPC
	end

@ Kanto_SevenIsland object 3 (local id 3, gfx 0x3F43) at 15,5
Kanto_SevenIsland_EventScript_8417B6A::
	msgbox Kanto_SevenIsland_Text_843340A, MSGBOX_NPC
	end

@ Kanto_SevenIsland object 1 (local id 1, gfx 0x3EFD) at 7,18
Kanto_SevenIsland_EventScript_8471CA5::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_SevenIsland_Text_843330D::
	.string "SEVEN ISLAND\n"
	.string "Quest Island of Infinity$"

Kanto_SevenIsland_Text_8433333::
	.string "These islands are called the SEVII\n"
	.string "ISLANDS because there are seven.\p"
	.string "…Or at least that’s what the young\n"
	.string "people believe.\p"
	.string "The truth is, these islands are so\n"
	.string "named because they are said to\l"
	.string "have been made in seven days.$"

Kanto_SevenIsland_Text_843340A::
	.string "This island has been left virtually\n"
	.string "untouched in its natural state.\p"
	.string "It could be a little difficult to\n"
	.string "hike around because of that.$"
