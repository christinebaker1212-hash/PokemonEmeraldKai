Kanto_SevenIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_EventScript_8417B51
	.byte 0

Kanto_SevenIsland_EventScript_8417B51::
	special QzKantoTravelingEventCheck
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
	@ Moemon Kai: FireRed's SWORDS DANCE tutor (tools/quetzal_port/fix_move_tutors.py),
	@ now gives its TM (tools/quetzal_port/tutors_to_tms.py)
	lock
	faceplayer
	goto_if_set FLAG_KANTO_GOT_TM_SWORDS_DANCE, Kanto_SevenIsland_EventScript_8471CA5_Taught
	msgbox Kanto_SevenIsland_EventScript_8471CA5_Teach, MSGBOX_DEFAULT
	giveitem ITEM_TM75
	goto_if_eq VAR_RESULT, FALSE, Kanto_SevenIsland_EventScript_8471CA5_Declined
	setflag FLAG_KANTO_GOT_TM_SWORDS_DANCE
	goto Kanto_SevenIsland_EventScript_8471CA5_Taught
	end

Kanto_SevenIsland_EventScript_8471CA5_Declined::
	release
	end

Kanto_SevenIsland_EventScript_8471CA5_Taught:
	msgbox Kanto_SevenIsland_EventScript_8471CA5_TaughtText, MSGBOX_DEFAULT
	release
	end

Kanto_SevenIsland_EventScript_8471CA5_Teach:
	.string "Not many people come out here.\p"
	.string "If I train here, I'm convinced that\n"
	.string "I'll get stronger and stronger.\p"
	.string "Yep, stronger and stronger…\p"
	.string "How would you like to learn a move\n"
	.string "like that? Take this TM:\l"
	.string "SWORDS DANCE!$"

Kanto_SevenIsland_EventScript_8471CA5_TaughtText:
	.string "If only we could keep getting\n"
	.string "stronger and stronger, eh?$"

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
