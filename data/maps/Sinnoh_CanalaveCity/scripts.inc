Sinnoh_CanalaveCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_CanalaveCity_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Sinnoh_CanalaveCity_OnFrame
	.byte 0

Sinnoh_CanalaveCity_OnTransition:
	special Sinnoh_LegendsRefresh
	end

Sinnoh_CanalaveCity_OnFrame:
	map_script_2 0x52F0, 1, Sinnoh_CanalaveCity_EventScript_Arrival   @ (VAR_SINNOH_ARRIVAL: the first boat in)
	.2byte 0

@ The first time in SINNOH: the harbor sailor (Person15) meets the player at the dock and sails them to TWINLEAF TOWN,
@ where the journey starts (CANALAVE's GYM is the sixth). MP_Region_GoToStartHouse sets VAR_SINNOH_ARRIVAL 1.
Sinnoh_CanalaveCity_EventScript_Arrival::
	lockall
	turnobject 15, DIR_SOUTH
	msgbox Sinnoh_CanalaveCity_Text_Arrival, MSGBOX_DEFAULT
	closemessage
	setvar 0x52F0, 2
	warp MAP_SINNOH_TWINLEAF_TOWN_PLAYER_HOUSE2F, 8, 5
	waitstate
	releaseall
	end

Sinnoh_CanalaveCity_Text_Arrival:
	.string "Ahoy! You must be the TRAINER from\n"
	.string "overseas. Welcome to SINNOH!\p"
	.string "PROF. ROWAN asked me to watch for\n"
	.string "you. Every journey in SINNOH starts\l"
	.string "in TWINLEAF TOWN, down south.\p"
	.string "A friend of the professor's has a\n"
	.string "room ready for you there.\p"
	.string "Hop in my little boat! I'll have you\n"
	.string "there before the tide turns.$"

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCity_EventScript_Bg1::
	goto SinnohP_canalave_city_21
	end

Sinnoh_CanalaveCity_EventScript_Person1::
	goto SinnohP_canalave_city_8
	end

Sinnoh_CanalaveCity_EventScript_Person2::
	goto SinnohP_canalave_city_11
	end

Sinnoh_CanalaveCity_EventScript_Person3::
	goto SinnohP_canalave_city_9
	end

Sinnoh_CanalaveCity_EventScript_Person4::
	goto SinnohP_canalave_city_10
	end

Sinnoh_CanalaveCity_EventScript_Person10::
	goto SinnohP_canalave_city_12
	end

Sinnoh_CanalaveCity_EventScript_Person11::
	goto SinnohP_canalave_city_13
	end

Sinnoh_CanalaveCity_EventScript_Person13::
	msgbox Sinnoh_CanalaveCity_Text_Person13, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person14::
	msgbox Sinnoh_CanalaveCity_Text_Person14, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person15::
	goto EventScript_SinnohRegionBoat
	end

Sinnoh_CanalaveCity_EventScript_Person17::
	goto SinnohP_canalave_city_20
	end

Sinnoh_CanalaveCity_EventScript_Person19::
	msgbox Sinnoh_CanalaveCity_Text_Person19, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Sign5::
	goto SinnohP_canalave_city_16
	end

Sinnoh_CanalaveCity_EventScript_Sign6::
	goto SinnohP_canalave_city_15
	end

Sinnoh_CanalaveCity_EventScript_Sign7::
	goto SinnohP_canalave_city_18
	end

Sinnoh_CanalaveCity_EventScript_Sign8::
	goto SinnohP_canalave_city_14
	end

Sinnoh_CanalaveCity_EventScript_Sign9::
	goto SinnohP_canalave_city_17
	end

Sinnoh_CanalaveCity_EventScript_Sign18::
	goto SinnohP_canalave_city_19
	end

Sinnoh_CanalaveCity_Text_Person13:
	.string "The PROFESSOR wants us all at the\n"
	.string "library. Something about the lakes!$"

Sinnoh_CanalaveCity_Text_Person14:
	.string "PROF. ROWAN: The library holds a book\n"
	.string "about SINNOH's three lakes. I must\l"
	.string "read it at once.$"

Sinnoh_CanalaveCity_Text_Person19:
	.string "BARRY: The library! The PROFESSOR's\n"
	.string "calling a meeting. Hurry up, I'll fine\l"
	.string "you if you're late!$"

@ --- end of SINNOH words ---
