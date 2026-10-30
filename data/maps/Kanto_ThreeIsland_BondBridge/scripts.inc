Kanto_ThreeIsland_BondBridge_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_ThreeIsland_BondBridge sign / bg event at 14,7
Kanto_ThreeIsland_BondBridge_EventScript_8418470::
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A4F9, MSGBOX_SIGN
	end

@ Kanto_ThreeIsland_BondBridge sign / bg event at 91,9
Kanto_ThreeIsland_BondBridge_EventScript_8418479::
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A50C, MSGBOX_SIGN
	end

@ Kanto_ThreeIsland_BondBridge object 1 (local id 1, gfx 0x3F58) at 27,10
Kanto_ThreeIsland_BondBridge_EventScript_845C336::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8715, 0, Kanto_ThreeIsland_BondBridge_Text_843A12B, Kanto_ThreeIsland_BondBridge_Text_843A15B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_ThreeIsland_BondBridge_EventScript_845C35D
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A187, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_EventScript_845C35D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8715, 0, Kanto_ThreeIsland_BondBridge_Text_8470F4D, Kanto_ThreeIsland_BondBridge_Text_843A15B
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A187, MSGBOX_AUTOCLOSE
	end

@ Kanto_ThreeIsland_BondBridge object 2 (local id 2, gfx 0x3F58) at 68,10
Kanto_ThreeIsland_BondBridge_EventScript_845C374::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8750, 0, Kanto_ThreeIsland_BondBridge_Text_843A1BD, Kanto_ThreeIsland_BondBridge_Text_843A1E3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_ThreeIsland_BondBridge_EventScript_845C39B
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A20A, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_EventScript_845C39B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8750, 0, Kanto_ThreeIsland_BondBridge_Text_8470F65, Kanto_ThreeIsland_BondBridge_Text_843A1E3
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A20A, MSGBOX_AUTOCLOSE
	end

@ Kanto_ThreeIsland_BondBridge object 3 (local id 3, gfx 0x3F52) at 33,13
Kanto_ThreeIsland_BondBridge_EventScript_845C3B2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8711, 0, Kanto_ThreeIsland_BondBridge_Text_843A24C, Kanto_ThreeIsland_BondBridge_Text_843A283
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_ThreeIsland_BondBridge_EventScript_845C3D9
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A291, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_EventScript_845C3D9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8711, 0, Kanto_ThreeIsland_BondBridge_Text_8470F98, Kanto_ThreeIsland_BondBridge_Text_843A283
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A291, MSGBOX_AUTOCLOSE
	end

@ Kanto_ThreeIsland_BondBridge object 4 (local id 4, gfx 0x3F52) at 50,11
Kanto_ThreeIsland_BondBridge_EventScript_845C3F0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8751, 0, Kanto_ThreeIsland_BondBridge_Text_843A2C4, Kanto_ThreeIsland_BondBridge_Text_843A2D7
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_ThreeIsland_BondBridge_EventScript_845C417
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A2E9, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_EventScript_845C417::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8751, 0, Kanto_ThreeIsland_BondBridge_Text_8470FC3, Kanto_ThreeIsland_BondBridge_Text_843A2D7
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A2E9, MSGBOX_AUTOCLOSE
	end

@ Kanto_ThreeIsland_BondBridge object 5 (local id 5, gfx 0x3F4D) at 36,4
Kanto_ThreeIsland_BondBridge_EventScript_845C42E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8753, 0, Kanto_ThreeIsland_BondBridge_Text_843A308, Kanto_ThreeIsland_BondBridge_Text_843A33C
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_ThreeIsland_BondBridge_EventScript_845C455
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A36B, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_EventScript_845C455::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8753, 0, Kanto_ThreeIsland_BondBridge_Text_8470FD6, Kanto_ThreeIsland_BondBridge_Text_843A33C
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A36B, MSGBOX_AUTOCLOSE
	end

@ Kanto_ThreeIsland_BondBridge object 6 (local id 6, gfx 0x3F18) at 77,5
Kanto_ThreeIsland_BondBridge_EventScript_845C46C::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8752, 0, Kanto_ThreeIsland_BondBridge_Text_843A3AD, Kanto_ThreeIsland_BondBridge_Text_843A3E0, Kanto_ThreeIsland_BondBridge_Text_843A421
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_ThreeIsland_BondBridge_EventScript_845C497
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A3EF, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_EventScript_845C497::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8752, 0, Kanto_ThreeIsland_BondBridge_Text_8471014, Kanto_ThreeIsland_BondBridge_Text_843A3E0, Kanto_ThreeIsland_BondBridge_Text_843A421
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A3EF, MSGBOX_AUTOCLOSE
	end

@ Kanto_ThreeIsland_BondBridge object 7 (local id 7, gfx 0x3F18) at 78,5
Kanto_ThreeIsland_BondBridge_EventScript_845C4B2::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8752, 0, Kanto_ThreeIsland_BondBridge_Text_843A462, Kanto_ThreeIsland_BondBridge_Text_843A48C, Kanto_ThreeIsland_BondBridge_Text_843A4B6
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_ThreeIsland_BondBridge_EventScript_845C4DD
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A49B, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_EventScript_845C4DD::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8752, 0, Kanto_ThreeIsland_BondBridge_Text_847103F, Kanto_ThreeIsland_BondBridge_Text_843A48C, Kanto_ThreeIsland_BondBridge_Text_843A4B6
	msgbox Kanto_ThreeIsland_BondBridge_Text_843A49B, MSGBOX_AUTOCLOSE
	end

Kanto_ThreeIsland_BondBridge_Text_843A12B::
	.string "The salty tang of the sea…\n"
	.string "It tickles my heart.$"

Kanto_ThreeIsland_BondBridge_Text_843A15B::
	.string "…Sniff, sniff…\n"
	.string "Something smells unpleasant…$"

Kanto_ThreeIsland_BondBridge_Text_843A187::
	.string "Perhaps that reek is your sweaty\n"
	.string "POKéMON’s body odor…$"

Kanto_ThreeIsland_BondBridge_Text_843A1BD::
	.string "Where are you off to in such\n"
	.string "a hurry?$"

Kanto_ThreeIsland_BondBridge_Text_843A1E3::
	.string "You’re raising some wonderful\n"
	.string "POKéMON.$"

Kanto_ThreeIsland_BondBridge_Text_843A20A::
	.string "If you keep going this way, you’ll\n"
	.string "eventually reach BERRY FOREST.$"

Kanto_ThreeIsland_BondBridge_Text_843A24C::
	.string "My mommy said that I can’t swim\n"
	.string "without my float ring.$"

Kanto_ThreeIsland_BondBridge_Text_843A283::
	.string "Waaah!\n"
	.string "Waaah!$"

Kanto_ThreeIsland_BondBridge_Text_843A291::
	.string "This year, I’m going to finally\n"
	.string "learn how to swim!$"

Kanto_ThreeIsland_BondBridge_Text_843A2C4::
	.string "Yay, yay!\n"
	.string "POKéMON!$"

Kanto_ThreeIsland_BondBridge_Text_843A2D7::
	.string "What happens now?$"

Kanto_ThreeIsland_BondBridge_Text_843A2E9::
	.string "Did I just win?\n"
	.string "Or did I lose?$"

Kanto_ThreeIsland_BondBridge_Text_843A308::
	.string "Oh, no, don’t come here!\n"
	.string "Please, stay away from me!$"

Kanto_ThreeIsland_BondBridge_Text_843A33C::
	.string "Okay, you’ve won!\n"
	.string "Now will you please go away?$"

Kanto_ThreeIsland_BondBridge_Text_843A36B::
	.string "A POKéMON bit through my \n"
	.string "swimsuit.\p"
	.string "I can’t get out of the water!$"

Kanto_ThreeIsland_BondBridge_Text_843A3AD::
	.string "JOY: We’ll teach you what our\n"
	.string "favorite POKéMON is!$"

Kanto_ThreeIsland_BondBridge_Text_843A3E0::
	.string "JOY: Ohh…\n"
	.string "MEG!$"

Kanto_ThreeIsland_BondBridge_Text_843A3EF::
	.string "JOY: Wasn’t that fun?\n"
	.string "I hope we can battle again!$"

Kanto_ThreeIsland_BondBridge_Text_843A421::
	.string "JOY: I really want to battle with\n"
	.string "MEG.\p"
	.string "One POKéMON isn’t enough.$"

Kanto_ThreeIsland_BondBridge_Text_843A462::
	.string "MEG: We’ll show you our favorite\n"
	.string "POKéMON.$"

Kanto_ThreeIsland_BondBridge_Text_843A48C::
	.string "MEG: Ohh…\n"
	.string "JOY!$"

Kanto_ThreeIsland_BondBridge_Text_843A49B::
	.string "MEG: JOY, wasn’t that fun?$"

Kanto_ThreeIsland_BondBridge_Text_843A4B6::
	.string "MEG: I want to battle together\n"
	.string "with JOY.\p"
	.string "One POKéMON isn’t enough.$"

Kanto_ThreeIsland_BondBridge_Text_843A4F9::
	.string "BERRY FOREST AHEAD$"

Kanto_ThreeIsland_BondBridge_Text_843A50C::
	.string "BOND BRIDGE\n"
	.string "Please be quiet when crossing.\p"
	.string "BERRY FOREST AHEAD$"

Kanto_ThreeIsland_BondBridge_Text_8470F4D::
	.string "Are we to battle again?$"

Kanto_ThreeIsland_BondBridge_Text_8470F65::
	.string "From where have you come, and\n"
	.string "where are you bound?$"

Kanto_ThreeIsland_BondBridge_Text_8470F98::
	.string "I want to swim without my float\n"
	.string "ring soon.$"

Kanto_ThreeIsland_BondBridge_Text_8470FC3::
	.string "Yay, yay!\n"
	.string "POKéMON!$"

Kanto_ThreeIsland_BondBridge_Text_8470FD6::
	.string "Oh, no, didn’t I tell you already?\n"
	.string "Please, stay away from me!$"

Kanto_ThreeIsland_BondBridge_Text_8471014::
	.string "JOY: We’ve gotten stronger!\n"
	.string "Lots and lots!$"

Kanto_ThreeIsland_BondBridge_Text_847103F::
	.string "MEG: You can’t beat us today!$"
