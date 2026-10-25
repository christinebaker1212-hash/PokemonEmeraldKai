Kanto_LavenderTown_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_LavenderTown_EventScript_84164BF
	.byte 0

Kanto_LavenderTown_EventScript_84164BF::
	special QzKantoTravelingEventCheck_773
	setflag FLAG_QZ_0x2240
	setvar VAR_QZ_0x5130, 1
	setflag FLAG_QZ_0x21CD
	end

@ Kanto_LavenderTown object 1 (local id 1, gfx 0x3F18) at 19,10
Kanto_LavenderTown_EventScript_84164CE::
	lock
	faceplayer
	msgbox Kanto_LavenderTown_Text_842F602, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_LavenderTown_EventScript_84164ED
	msgbox Kanto_LavenderTown_Text_842F63C, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_EventScript_84164ED::
	msgbox Kanto_LavenderTown_Text_842F61C, MSGBOX_DEFAULT
	release
	end

@ Kanto_LavenderTown object 2 (local id 2, gfx 0x3F5B) at 12,12
Kanto_LavenderTown_EventScript_84164F7::
	msgbox Kanto_LavenderTown_Text_842F68A, MSGBOX_NPC
	end

@ Kanto_LavenderTown object 3 (local id 3, gfx 0x3EEE) at 10,7
Kanto_LavenderTown_EventScript_8416500::
	msgbox Kanto_LavenderTown_Text_842F6E8, MSGBOX_NPC
	end

@ Kanto_LavenderTown sign / bg event at 15,11
Kanto_LavenderTown_EventScript_8416509::
	msgbox Kanto_LavenderTown_Text_842F74A, MSGBOX_SIGN
	end

@ Kanto_LavenderTown sign / bg event at 12,4
Kanto_LavenderTown_EventScript_8416512::
	msgbox Kanto_LavenderTown_Text_842F76E, MSGBOX_SIGN
	end

@ Kanto_LavenderTown sign / bg event at 7,11
Kanto_LavenderTown_EventScript_841651B::
	msgbox Kanto_LavenderTown_Text_842F7AA, MSGBOX_SIGN
	end

@ Kanto_LavenderTown sign / bg event at 21,7
Kanto_LavenderTown_EventScript_8416524::
	msgbox Kanto_LavenderTown_Text_842F7CB, MSGBOX_SIGN
	end

Kanto_LavenderTown_Text_842F602::
	.string "Do you believe in ghosts?$"

Kanto_LavenderTown_Text_842F61C::
	.string "Really?\n"
	.string "So there are believers…$"

Kanto_LavenderTown_Text_842F63C::
	.string "Hahaha, I guess not.\p"
	.string "That white hand on your shoulder…\n"
	.string "I’m just imagining it.$"

Kanto_LavenderTown_Text_842F68A::
	.string "This town is known as the grave\n"
	.string "site of POKéMON.\p"
	.string "Memorial services are held in\n"
	.string "POKéMON TOWER.$"

Kanto_LavenderTown_Text_842F6E8::
	.string "Ghosts appeared in POKéMON TOWER.\p"
	.string "I think they’re the spirits of\n"
	.string "POKéMON that the ROCKETS killed.$"

Kanto_LavenderTown_Text_842F74A::
	.string "LAVENDER TOWN\n"
	.string "The Noble Purple Town$"

Kanto_LavenderTown_Text_842F76E::
	.string "New SILPH SCOPE!\n"
	.string "Make the Invisible Plain to See!\p"
	.string "SILPH CO.$"

Kanto_LavenderTown_Text_842F7AA::
	.string "LAVENDER VOLUNTEER\n"
	.string "POKéMON HOUSE$"

Kanto_LavenderTown_Text_842F7CB::
	.string "POKéMON TOWER\n"
	.string "Becalm the Spirits of POKéMON$"
