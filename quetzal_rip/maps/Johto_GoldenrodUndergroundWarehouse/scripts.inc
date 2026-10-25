Johto_GoldenrodUndergroundWarehouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_GoldenrodUndergroundWarehouse object 7 (local id 7, gfx 0x30) at 13,8
Johto_GoldenrodUndergroundWarehouse_EventScript_83D9C92::
	lock
	faceplayer
	checkitem ITEM_CARD_KEY, 1
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodUndergroundWarehouse_EventScript_83D9CC4
	msgbox Johto_GoldenrodUndergroundWarehouse_Text_83D9F90, MSGBOX_DEFAULT
	closemessage
	setorcopyvar VAR_0x8000, 0x2EE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	msgbox Johto_GoldenrodUndergroundWarehouse_Text_83DA025, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodUndergroundWarehouse_EventScript_83D9CC4::
	msgbox Johto_GoldenrodUndergroundWarehouse_Text_83DA054, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodUndergroundWarehouse object 5 (local id 5, gfx 0x3EB0) at 11,13
Johto_GoldenrodUndergroundWarehouse_EventScript_83D9CCF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4213, 0, Johto_GoldenrodUndergroundWarehouse_Text_83D9D3B, Johto_GoldenrodUndergroundWarehouse_Text_83D9D86
	msgbox Johto_GoldenrodUndergroundWarehouse_Text_83D9D99, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundWarehouse object 4 (local id 4, gfx 0x3EB0) at 11,16
Johto_GoldenrodUndergroundWarehouse_EventScript_83D9CE6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4951, 0, Johto_GoldenrodUndergroundWarehouse_Text_83D9DF6, Johto_GoldenrodUndergroundWarehouse_Text_83D9E46
	msgbox Johto_GoldenrodUndergroundWarehouse_Text_83D9E50, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundWarehouse object 6 (local id 6, gfx 0x3EB0) at 11,7
Johto_GoldenrodUndergroundWarehouse_EventScript_83D9CFD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4875, 0, Johto_GoldenrodUndergroundWarehouse_Text_83D9E75, Johto_GoldenrodUndergroundWarehouse_Text_83D9EB5
	msgbox Johto_GoldenrodUndergroundWarehouse_Text_83D9F58, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundWarehouse object 3 (local id 3, gfx 0x3A9E) at 14,9
Johto_GoldenrodUndergroundWarehouse_EventScript_83D9D14::
	setorcopyvar VAR_0x8000, 0xDC
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUndergroundWarehouse object 1 (local id 1, gfx 0x3A9E) at 1,2
Johto_GoldenrodUndergroundWarehouse_EventScript_83D9D21::
	setorcopyvar VAR_0x8000, 3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUndergroundWarehouse object 2 (local id 2, gfx 0x3A9E) at 21,16
Johto_GoldenrodUndergroundWarehouse_EventScript_83D9D2E::
	setorcopyvar VAR_0x8000, 0x32
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_GoldenrodUndergroundWarehouse_Text_83D9D3B::
	.string "How did you get this far?\p"
	.string "I guess it can’t be helped. I’ll\n"
	.string "dispose of you.$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9D86::
	.string "I got disposed of…$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9D99::
	.string "TEAM ROCKET will keep going, waiting\n"
	.string "for the return of GIOVANNI.\p"
	.string "We’ll do whatever it takes.$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9DF6::
	.string "You’re not going any farther!\p"
	.string "I don’t show mercy to my enemies,\n"
	.string "not even brats!$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9E46::
	.string "Blast it!$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9E50::
	.string "I lost…\p"
	.string "Please forgive me, GIOVANNI!$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9E75::
	.string "Hyuck-hyuck-hyuck! Look who it is!\p"
	.string "Ready? I’m really tough now!$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9EB5::
	.string "Hyuck-hyuck-hyuck! You’re so good!\p"
	.string "This TEAM ROCKET thing is just\n"
	.string "a job, but I can see it hurts others.\p"
	.string "That’s it, I’m leaving.\p"
	.string "Thank you so much. Sorry GIOVANNI!$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9F58::
	.string "Hyuck-hyuck-hyuck! That was a blast!\n"
	.string "I’ll remember you!$"

Johto_GoldenrodUndergroundWarehouse_Text_83D9F90::
	.string "DIRECTOR: Who? What? You came to\n"
	.string "rescue me?\p"
	.string "Thank you!\p"
	.string "The RADIO TOWER!\p"
	.string "What’s happening there?\p"
	.string "Taken over by TEAM ROCKET?\p"
	.string "Here. Take this CARD KEY.$"

Johto_GoldenrodUndergroundWarehouse_Text_83DA025::
	.string "DIRECTOR: Use that to open the\n"
	.string "shutters on 3F.$"

Johto_GoldenrodUndergroundWarehouse_Text_83DA054::
	.string "I’m begging you to help.\p"
	.string "There’s no telling what they’ll do if\n"
	.string "they control the transmitter.\p"
	.string "They may even be able to control\n"
	.string "POKéMON using a special signal!\p"
	.string "You’re the only one I can call on.\p"
	.string "Please save the RADIO TOWER…\p"
	.string "And all the POKéMON nationwide!$"
