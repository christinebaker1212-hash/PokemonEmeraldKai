Sinnoh_CanalaveCity_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Sinnoh_CanalaveCity_OnFrame
	.byte 0

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
	msgbox Sinnoh_CanalaveCity_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_CanalaveCity_EventScript_Person1::
	msgbox Sinnoh_CanalaveCity_Text_Person1, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person2::
	msgbox Sinnoh_CanalaveCity_Text_Person2, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person3::
	msgbox Sinnoh_CanalaveCity_Text_Person3, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person4::
	msgbox Sinnoh_CanalaveCity_Text_Person4, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person10::
	msgbox Sinnoh_CanalaveCity_Text_Person10, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person11::
	msgbox Sinnoh_CanalaveCity_Text_Person11, MSGBOX_NPC
	end

Sinnoh_CanalaveCity_EventScript_Person15::
	goto EventScript_SinnohRegionBoat
	end

Sinnoh_CanalaveCity_EventScript_Person17::
	lock
	faceplayer
	msgbox Sinnoh_CanalaveCity_Text_Person17Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_CanalaveCity_EventScript_Person17_Sail0
	msgbox Sinnoh_CanalaveCity_Text_Person17Ask1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_CanalaveCity_EventScript_Person17_Sail1
	msgbox Sinnoh_CanalaveCity_Text_Person17Ask2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_CanalaveCity_EventScript_Person17_Sail2
	msgbox Sinnoh_CanalaveCity_Text_Person17Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCity_EventScript_Person17_Sail0::
	msgbox Sinnoh_CanalaveCity_Text_Person17Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_IRON_ISLAND, 4, 22
	waitstate
	releaseall
	end

Sinnoh_CanalaveCity_EventScript_Person17_Sail1::
	msgbox Sinnoh_CanalaveCity_Text_Person17Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_FULLMOON_ISLAND, 9, 21
	waitstate
	releaseall
	end

Sinnoh_CanalaveCity_EventScript_Person17_Sail2::
	msgbox Sinnoh_CanalaveCity_Text_Person17Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_NEWMOON_ISLAND, 24, 21
	waitstate
	releaseall
	end

Sinnoh_CanalaveCity_EventScript_Sign5::
	msgbox Sinnoh_CanalaveCity_Text_Sign5, MSGBOX_SIGN
	end

Sinnoh_CanalaveCity_EventScript_Sign6::
	msgbox Sinnoh_CanalaveCity_Text_Sign6, MSGBOX_SIGN
	end

Sinnoh_CanalaveCity_EventScript_Sign7::
	msgbox Sinnoh_CanalaveCity_Text_Sign7, MSGBOX_SIGN
	end

Sinnoh_CanalaveCity_EventScript_Sign8::
	msgbox Sinnoh_CanalaveCity_Text_Sign8, MSGBOX_SIGN
	end

Sinnoh_CanalaveCity_EventScript_Sign9::
	msgbox Sinnoh_CanalaveCity_Text_Sign9, MSGBOX_SIGN
	end

Sinnoh_CanalaveCity_EventScript_Sign18::
	msgbox Sinnoh_CanalaveCity_Text_Sign18, MSGBOX_SIGN
	end

Sinnoh_CanalaveCity_Text_Bg1:
	.string "The door is locked. A sign says “Back\n"
	.string "soon.”$"

Sinnoh_CanalaveCity_Text_Person1:
	.string "Boats come and go from CANALAVE all\n"
	.string "day. I like to guess where each one is\l"
	.string "headed.$"

Sinnoh_CanalaveCity_Text_Person2:
	.string "The library here has books older than\n"
	.string "my great-grandma! The third floor is\l"
	.string "the best.$"

Sinnoh_CanalaveCity_Text_Person3:
	.string "The drawbridge goes up when big ships\n"
	.string "pass. You get used to waiting.$"

Sinnoh_CanalaveCity_Text_Person4:
	.string "BYRON at the GYM is tough. His MOéMON\n"
	.string "are as hard as the iron from the\l"
	.string "island.$"

Sinnoh_CanalaveCity_Text_Person10:
	.string "PSYDUCK: Psy...? Psy-yi-yi...\p"
	.string "It's holding its head. It looks like\n"
	.string "it's thinking very hard.$"

Sinnoh_CanalaveCity_Text_Person11:
	.string "I've fished from this canal for fifty\n"
	.string "years. The fish here are smarter than\l"
	.string "they look.$"

Sinnoh_CanalaveCity_Text_Person17Ask0:
	.string "Shall I take you to IRON ISLAND?$"

Sinnoh_CanalaveCity_Text_Person17Ask1:
	.string "Shall I take you to FULLMOON ISLAND?$"

Sinnoh_CanalaveCity_Text_Person17Ask2:
	.string "Shall I take you to NEWMOON ISLAND?$"

Sinnoh_CanalaveCity_Text_Person17Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_CanalaveCity_Text_Person17Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_CanalaveCity_Text_Sign5:
	.string "CANALAVE LIBRARY\p"
	.string "Stories of SINNOH's past, open to\n"
	.string "everyone.$"

Sinnoh_CanalaveCity_Text_Sign6:
	.string "CANALAVE CITY MOéMON GYM\p"
	.string "LEADER: BYRON\p"
	.string "A wall of solid steel!$"

Sinnoh_CanalaveCity_Text_Sign7:
	.string "CANALAVE DOCK\p"
	.string "Boats to IRON ISLAND and beyond.$"

Sinnoh_CanalaveCity_Text_Sign8:
	.string "CANALAVE CITY\p"
	.string "The port town where the sea and the\n"
	.string "land shake hands.$"

Sinnoh_CanalaveCity_Text_Sign9:
	.string "SAILOR ELDRITCH'S HOUSE$"

Sinnoh_CanalaveCity_Text_Sign18:
	.string "HARBOR INN\p"
	.string "A warm bed for tired travelers.$"

@ --- end of SINNOH words ---
