Kanto_FiveIsland_Meadow_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_FiveIsland_Meadow_EventScript_84186E3
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

Kanto_FiveIsland_Meadow_EventScript_84186E3::
	call_if_set FLAG_QZ_0x222E, Kanto_FiveIsland_Meadow_EventScript_841872E
	end

@ Kanto_FiveIsland_Meadow sign / bg event at 12,21
Kanto_FiveIsland_Meadow_EventScript_84186ED::
	lockall
	goto_if_set FLAG_QZ_0x222E, Kanto_FiveIsland_Meadow_EventScript_8418724
	goto_if_set FLAG_QZ_0x21DD, Kanto_FiveIsland_Meadow_EventScript_841870A
	msgbox Kanto_FiveIsland_Meadow_Text_843AE77, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FiveIsland_Meadow_EventScript_841870A::
	setflag FLAG_QZ_0x222E
	message Kanto_FiveIsland_Meadow_Text_843AEDD
	waitmessage
	delay 0x3C
	call Kanto_FiveIsland_Meadow_EventScript_841872E
	special DrawWholeMapView
	playse SE_PIN
	waitse
	releaseall
	end

Kanto_FiveIsland_Meadow_EventScript_8418724::
	msgbox Kanto_FiveIsland_Meadow_Text_843AF4B, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FiveIsland_Meadow_EventScript_841872E::
	setmetatile 0xC, 0x15, 0x2AF, 1
	return

@ Kanto_FiveIsland_Meadow object 2 (local id 2, gfx 0x3F3D) at 17,5
Kanto_FiveIsland_Meadow_EventScript_8418738::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8759, 0, Kanto_FiveIsland_Meadow_Text_843AF71, Kanto_FiveIsland_Meadow_Text_843AF9C
	msgbox Kanto_FiveIsland_Meadow_Text_843AFAE, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_Meadow object 3 (local id 3, gfx 0x3F3D) at 11,27
Kanto_FiveIsland_Meadow_EventScript_841874F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8760, 0, Kanto_FiveIsland_Meadow_Text_843AFD9, Kanto_FiveIsland_Meadow_Text_843B022
	msgbox Kanto_FiveIsland_Meadow_Text_843B042, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_Meadow object 1 (local id 1, gfx 0x3F3C) at 18,18
Kanto_FiveIsland_Meadow_EventScript_8418766::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8761, 0, Kanto_FiveIsland_Meadow_Text_843B0AF, Kanto_FiveIsland_Meadow_Text_843B0E4
	msgbox Kanto_FiveIsland_Meadow_Text_843B0FF, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_Meadow object 6 (local id 6, gfx 0x3F0E) at 12,11
Kanto_FiveIsland_Meadow_EventScript_846C14F::
	setorcopyvar VAR_0x8000, 0x1F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_FiveIsland_Meadow object 7 (local id 7, gfx 0x3F0E) at 3,22
Kanto_FiveIsland_Meadow_EventScript_846C15C::
	setorcopyvar VAR_0x8000, 0x47
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_FiveIsland_Meadow_Text_843AE77::
	.string "{PLAYER} entered a password.\p"
	.string "“GOLDEEN need log.”\p"
	.string "… …  … …  … …\p"
	.string "Another password is needed to\n"
	.string "open the door…$"

Kanto_FiveIsland_Meadow_Text_843AEDD::
	.string "{PLAYER} entered two passwords.\p"
	.string "“GOLDEEN need log.”\n"
	.string "“Yes, nah, CHANSEY.”\p"
	.string "… …  … …  … …\p"
	.string "The WAREHOUSE’s door opened!$"

Kanto_FiveIsland_Meadow_Text_843AF4B::
	.string "The WAREHOUSE’s door is already\n"
	.string "open.$"

Kanto_FiveIsland_Meadow_Text_843AF71::
	.string "This area’s strictly off-limits!\n"
	.string "Out, out!$"

Kanto_FiveIsland_Meadow_Text_843AF9C::
	.string "Is this for real?$"

Kanto_FiveIsland_Meadow_Text_843AFAE::
	.string "You don’t have any business here!\n"
	.string "Go home!$"

Kanto_FiveIsland_Meadow_Text_843AFD9::
	.string "There’s nothing past here except\n"
	.string "our WAREHOUSE.\p"
	.string "So what brings you here?$"

Kanto_FiveIsland_Meadow_Text_843B022::
	.string "That wasn’t supposed to happen…$"

Kanto_FiveIsland_Meadow_Text_843B042::
	.string "You’re wanting to know what’s inside\n"
	.string "this WAREHOUSE?\p"
	.string "It’s better you didn’t know.\n"
	.string "For your own sake, I mean.$"

Kanto_FiveIsland_Meadow_Text_843B0AF::
	.string "You’d be wasting time even if you\n"
	.string "were to get by me.$"

Kanto_FiveIsland_Meadow_Text_843B0E4::
	.string "Oh, that makes me furious!$"

Kanto_FiveIsland_Meadow_Text_843B0FF::
	.string "You won’t be able to get in without\n"
	.string "the passwords anyway.$"
