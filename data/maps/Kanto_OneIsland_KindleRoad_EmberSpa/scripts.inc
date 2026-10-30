Kanto_OneIsland_KindleRoad_EmberSpa_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_OneIsland_KindleRoad_EmberSpa object 1 (local id 1, gfx 0x3F2C) at 11,13
Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_84151B6::
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DA53, MSGBOX_NPC
	end

@ Kanto_OneIsland_KindleRoad_EmberSpa object 2 (local id 2, gfx 0x3EE6) at 15,11
Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_84151BF::
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DAE7, MSGBOX_NPC
	end

@ Kanto_OneIsland_KindleRoad_EmberSpa object 6 (local id 6, gfx 0x3EE6) at 11,20
Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_84151C8::
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DBB3, MSGBOX_NPC
	end

@ Kanto_OneIsland_KindleRoad_EmberSpa object 4 (local id 4, gfx 0x3F2F) at 20,7
Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_84151D1::
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DBEC, MSGBOX_NPC
	end

@ Kanto_OneIsland_KindleRoad_EmberSpa object 3 (local id 3, gfx 0x3EEB) at 6,8
Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_84151DA::
	lock
	faceplayer
	setvar VAR_0x8004, 0xA
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DC59, MSGBOX_DEFAULT
	release
	end

@ Kanto_OneIsland_KindleRoad_EmberSpa object 5 (local id 5, gfx 0x3F2C) at 10,5
Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_84151F3::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2153, Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_841521F
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DD2B, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_PROTECTOR
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setflag FLAG_QZ_0x2153
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DD9B, MSGBOX_DEFAULT
	release
	end

Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_841521F::
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DD9B, MSGBOX_DEFAULT
	release
	end

@ Kanto_OneIsland_KindleRoad_EmberSpa trigger at 13,13 (VAR_TEMP_1 == 0)
Kanto_OneIsland_KindleRoad_EmberSpa_EventScript_8415229::
	lockall
	fadescreen 3
	special QzHealPlayerPartyEx
	fadescreen 2
	msgbox Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DB59, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, 1
	releaseall
	end

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DA53::
	.string "Hm!\n"
	.string "Haaaaaaaaaaah!\p"
	.string "Aaaah! This is heavenly!\n"
	.string "The water warms me to my core!\p"
	.string "You should jump in, too.\p"
	.string "Go to the middle, relax, and get\n"
	.string "good and warm!$"

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DAE7::
	.string "My favorite part of bathing comes\n"
	.string "up right after this.\p"
	.string "I enjoy a bowl of chowder to\n"
	.string "warm me from the inside, too!$"

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DB59::
	.string "The water is perfectly warm.\n"
	.string "It feels exquisite…\p"
	.string "{PLAYER} and the POKéMON were\n"
	.string "fully refreshed!$"

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DBB3::
	.string "Soaking in a hot spring pool is\n"
	.string "therapeutic, people say.$"

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DBEC::
	.string "Have a look at my face.\n"
	.string "See how smooth my skin is?\p"
	.string "The hot spring’s water keeps my\n"
	.string "skin younger than my age.$"

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DC59::
	.string "After an honest day of training,\n"
	.string "nothing beats a relaxing soak in\l"
	.string "the hot spring pool.\p"
	.string "BRUNO, who’s a senior ahead of me,\n"
	.string "visits the SPA on occasion.\p"
	.string "He comes to rehab injuries, both\n"
	.string "his own and his POKéMON’s.$"

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DD2B::
	.string "Hot springs go hand-in-hand with\n"
	.string "volcanoes.\p"
	.string "I found this PROTECTOR while I\n"
	.string "carved the pool. You should have it.$"

Kanto_OneIsland_KindleRoad_EmberSpa_Text_842DD9B::
	.string "A PROTECTOR can help a certain\n"
	.string "POKéMON evolve.$"
