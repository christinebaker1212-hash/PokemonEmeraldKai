Kanto_ThreeIsland_Port_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ThreeIsland_Port_EventScript_8418488
	.byte 0

Kanto_ThreeIsland_Port_EventScript_8418488::
	clearflag FLAG_QZ_0x2211
	setflag FLAG_QZ_0x21F1
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_ThreeIsland_Port object 1 (local id 1, gfx 0x3F59) at 14,9
Kanto_ThreeIsland_Port_EventScript_8418494::
	lock
	faceplayer
	goto_if_ge VAR_QZ_0x5134, 2, Kanto_ThreeIsland_Port_EventScript_84184B6
	goto_if_ge VAR_QZ_0x5131, 4, Kanto_ThreeIsland_Port_EventScript_84184C0
	msgbox Kanto_ThreeIsland_Port_Text_843A5F0, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_Port_EventScript_84184B6::
	msgbox Kanto_ThreeIsland_Port_Text_843A674, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_Port_EventScript_84184C0::
	msgbox Kanto_ThreeIsland_Port_Text_843A622, MSGBOX_DEFAULT
	release
	end

@ Kanto_ThreeIsland_Port object 2 (local id 2, gfx 0x3EE9) at 8,8
Kanto_ThreeIsland_Port_EventScript_84184CA::
	msgbox Kanto_ThreeIsland_Port_Text_843A54A, MSGBOX_NPC
	end

@ Kanto_ThreeIsland_Port object 3 (local id 3, gfx 0x3EE9) at 13,10
Kanto_ThreeIsland_Port_EventScript_84184D3::
	lock
	msgbox Kanto_ThreeIsland_Port_Text_843A5C1, MSGBOX_DEFAULT
	release
	end

Kanto_ThreeIsland_Port_Text_843A54A::
	.string "We’re the KANTO RIDER FEDERATION!\n"
	.string "Vroom! Vavavavooom! Vroom!\p"
	.string "My bike’s wimpy, so I have to\n"
	.string "make exhaust noises myself!$"

Kanto_ThreeIsland_Port_Text_843A5C1::
	.string "Hey, hey, lady!\n"
	.string "Fork over some spending money!$"

Kanto_ThreeIsland_Port_Text_843A5F0::
	.string "Aiyeeh!\p"
	.string "Wh-who are you?!\n"
	.string "I… I’ll call the police!$"

Kanto_ThreeIsland_Port_Text_843A622::
	.string "Those BIKERS…\n"
	.string "Are they gone?\p"
	.string "Oh, thank goodness!\n"
	.string "They certainly had me terrified!$"

Kanto_ThreeIsland_Port_Text_843A674::
	.string "Those BIKERS ran off, and\n"
	.string "LOSTELLE was found safe.\p"
	.string "Everything turned out for the best!$"
