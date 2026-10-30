Johto_Route30_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route30_EventScript_83ED370
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_Route30_OnFrame_83ED371
	.byte 0

Johto_Route30_OnFrame_83ED371:
	map_script_2 VAR_QZ_0x5005, 2, Johto_Route30_EventScript_83ED417
	.2byte 0

Johto_Route30_EventScript_83ED370::
	end

@ Johto_Route30 object 1 (local id 1, gfx 0xB) at 26,31
Johto_Route30_EventScript_83ED37B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x14A0, Johto_Route30_EventScript_83ED391
	msgbox Johto_Route30_Text_83ED6CC, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route30_EventScript_83ED391::
	lock
	faceplayer
	msgbox Johto_Route30_Text_83ED6FB, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route30 object 3 (local id 3, gfx 0x10) at 20,13
Johto_Route30_EventScript_83ED39E::
	lock
	faceplayer
	msgbox Johto_Route30_Text_83ED72B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route30 object 6 (local id 6, gfx 0x23) at 23,25
Johto_Route30_EventScript_83ED3AB::
	lock
	msgbox Johto_Route30_Text_83ED45B, MSGBOX_AUTOCLOSE
	waitmessage
	playse SE_PIN
	applymovement 6, Quetzal_Movement_83607F7
	delay 0x1E
	faceplayer
	msgbox Johto_Route30_Text_83ED470, MSGBOX_AUTOCLOSE
	waitmessage
	turnobject 6, 2
	release
	end

@ Johto_Route30 object 2 (local id 2, gfx 0x23) at 20,28
Johto_Route30_EventScript_83ED3D2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4418, 0, Johto_Route30_Text_83ED49C, Johto_Route30_Text_83ED4F9
	msgbox Johto_Route30_Text_83ED518, MSGBOX_AUTOCLOSE
	end

@ Johto_Route30 object 7 (local id 7, gfx 0x23) at 23,22
Johto_Route30_EventScript_83ED3E9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4593, 0, Johto_Route30_Text_83ED581, Johto_Route30_Text_83ED5BB
	msgbox Johto_Route30_Text_83ED5D9, MSGBOX_AUTOCLOSE
	end

@ Johto_Route30 object 4 (local id 4, gfx 0x24) at 20,8
Johto_Route30_EventScript_83ED400::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4871, 0, Johto_Route30_Text_83ED632, Johto_Route30_Text_83ED65F
	msgbox Johto_Route30_Text_83ED678, MSGBOX_AUTOCLOSE
	end

@ Johto_Route30 trigger at 35,6 (VAR_QZ_0x5005 == 2)
@ Johto_Route30 trigger at 34,5 (VAR_QZ_0x5005 == 2)
@ Johto_Route30 trigger at 36,5 (VAR_QZ_0x5005 == 2)
Johto_Route30_EventScript_83ED417::
	lock
	msgbox Johto_Route30_Text_83ED8CA, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5005, 3
	release
	end

@ Johto_Route30 sign / bg event at 31,29
Johto_Route30_EventScript_83ED427::
	lock
	msgbox Johto_Route30_Text_83ED772, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route30 sign / bg event at 20,19
Johto_Route30_EventScript_83ED433::
	lock
	msgbox Johto_Route30_Text_83ED7D2, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route30 sign / bg event at 32,4
Johto_Route30_EventScript_83ED43F::
	msgbox Johto_Route30_Text_83ED448, MSGBOX_SIGN
	end

Johto_Route30_Text_83ED448::
	.string "MR.POKéMON’s House$"

Johto_Route30_Text_83ED45B::
	.string "Go, RATTATA!\p"
	.string "TACKLE!$"

Johto_Route30_Text_83ED470::
	.string "What? This is a big battle!\n"
	.string "Leave me alone!$"

Johto_Route30_Text_83ED49C::
	.string "I just lost, so I’m trying to\n"
	.string "find more POKéMON.\p"
	.string "Wait! You look weak! Come on,\n"
	.string "let’s battle!$"

Johto_Route30_Text_83ED4F9::
	.string "Ack! I lost again!\n"
	.string "Doggone it!$"

Johto_Route30_Text_83ED518::
	.string "Do I have to have more POKéMON\n"
	.string "in order to battle better?\p"
	.string "No! I’m sticking with this one\n"
	.string "no matter what!$"

Johto_Route30_Text_83ED581::
	.string "You’re a POKéMON trainer,\n"
	.string "right?\p"
	.string "Then you have to battle!$"

Johto_Route30_Text_83ED5BB::
	.string "That’s strange.\n"
	.string "I won before.$"

Johto_Route30_Text_83ED5D9::
	.string "Becoming a good trainer is\n"
	.string "really tough.\p"
	.string "I’m going to battle other\n"
	.string "people to get better.$"

Johto_Route30_Text_83ED632::
	.string "Instead of a bug POKéMON,\n"
	.string "I found a trainer!$"

Johto_Route30_Text_83ED65F::
	.string "Argh! You’re too strong!$"

Johto_Route30_Text_83ED678::
	.string "I ran out of POKé BALLS while\n"
	.string "I was catching POKéMON.\p"
	.string "I should’ve bought some more…$"

Johto_Route30_Text_83ED6CC::
	.string "MR. POKéMON’s house?\n"
	.string "It’s a bit farther ahead.$"

Johto_Route30_Text_83ED6FB::
	.string "Everyone’s having fun\n"
	.string "battling!\p"
	.string "You should too!$"

Johto_Route30_Text_83ED72B::
	.string "I’m not a trainer.\p"
	.string "But if you look one in the\n"
	.string "eyes, prepare to battle.$"

Johto_Route30_Text_83ED772::
	.string "ROUTE 30\p"
	.string "VIOLET CITY - CHERRYGROVE CITY$"

Johto_Route30_Text_83ED7D2::
	.string "TRAINER TIPS\p"
	.string "No stealing other people’s\n"
	.string "POKéMON!\p"
	.string "POKé BALLS are to be thrown\n"
	.string "only at wild POKéMON!$"

Johto_Route30_Text_83ED8CA::
	.string "A message from PROF. ELM arrived:\p"
	.string "{PLAYER}, it’s a disaster!\p"
	.string "Something terrible happened at\n"
	.string "the LAB. Please come back now!$"
