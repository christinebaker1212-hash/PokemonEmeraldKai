Kanto_Route9_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route9 sign / bg event at 29,7
Kanto_Route9_EventScript_8417C9B::
	msgbox Kanto_Route9_Text_8434E2D, MSGBOX_SIGN
	end

@ Kanto_Route9 object 5 (local id 5, gfx 0x3EF1) at 26,1
Kanto_Route9_EventScript_845A14A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8306, 0, Kanto_Route9_Text_8434CF6, Kanto_Route9_Text_8434D31
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A171
	msgbox Kanto_Route9_Text_8434D53, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A171::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8306, 0, Kanto_Route9_Text_846F029, Kanto_Route9_Text_8434D31
	msgbox Kanto_Route9_Text_8434D53, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 6 (local id 6, gfx 0x3EF1) at 49,7
Kanto_Route9_EventScript_845A188::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8307, 0, Kanto_Route9_Text_8434DDF, Kanto_Route9_Text_8434DF9
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A1AF
	msgbox Kanto_Route9_Text_8434E02, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A1AF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8307, 0, Kanto_Route9_Text_846F082, Kanto_Route9_Text_8434DF9
	msgbox Kanto_Route9_Text_8434E02, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 4 (local id 4, gfx 0x3EF3) at 26,7
Kanto_Route9_EventScript_845A1C6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8340, 0, Kanto_Route9_Text_8434B0B, Kanto_Route9_Text_8434B45
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A1ED
	msgbox Kanto_Route9_Text_8434B57, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A1ED::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8340, 0, Kanto_Route9_Text_846EF03, Kanto_Route9_Text_8434B45
	msgbox Kanto_Route9_Text_8434B57, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 9 (local id 9, gfx 0x3EF3) at 36,6
Kanto_Route9_EventScript_845A204::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8341, 0, Kanto_Route9_Text_8434B65, Kanto_Route9_Text_8434B93
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A22B
	msgbox Kanto_Route9_Text_8434BA5, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A22B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8341, 0, Kanto_Route9_Text_846EF41, Kanto_Route9_Text_8434B93
	msgbox Kanto_Route9_Text_8434BA5, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 1 (local id 1, gfx 0x3F31) at 14,10
Kanto_Route9_EventScript_845A242::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8346, 0, Kanto_Route9_Text_8434A7E, Kanto_Route9_Text_8434AA6
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A269
	msgbox Kanto_Route9_Text_8434AB7, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A269::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8346, 0, Kanto_Route9_Text_846EED4, Kanto_Route9_Text_8434AA6
	msgbox Kanto_Route9_Text_8434AB7, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 8 (local id 8, gfx 0x3F31) at 60,9
Kanto_Route9_EventScript_845A280::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8347, 0, Kanto_Route9_Text_8434BC6, Kanto_Route9_Text_8434BE3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A2A7
	msgbox Kanto_Route9_Text_8434BF8, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A2A7::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8347, 0, Kanto_Route9_Text_846EFA1, Kanto_Route9_Text_8434BE3
	msgbox Kanto_Route9_Text_8434BF8, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 3 (local id 3, gfx 0x3F0C) at 61,16
Kanto_Route9_EventScript_845A2BE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8377, 0, Kanto_Route9_Text_8434D85, Kanto_Route9_Text_8434D9C
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A2E5
	msgbox Kanto_Route9_Text_8434DB8, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A2E5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8377, 0, Kanto_Route9_Text_846F064, Kanto_Route9_Text_8434D9C
	msgbox Kanto_Route9_Text_8434DB8, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 7 (local id 7, gfx 0x3F0C) at 53,2
Kanto_Route9_EventScript_845A2FC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8378, 0, Kanto_Route9_Text_8434CA6, Kanto_Route9_Text_8434CCB
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A323
	msgbox Kanto_Route9_Text_8434CD8, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A323::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8378, 0, Kanto_Route9_Text_846F004, Kanto_Route9_Text_8434CCB
	msgbox Kanto_Route9_Text_8434CD8, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 2 (local id 2, gfx 0x3F0C) at 20,17
Kanto_Route9_EventScript_845A33A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8657, 0, Kanto_Route9_Text_8434C25, Kanto_Route9_Text_8434C46
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route9_EventScript_845A361
	msgbox Kanto_Route9_Text_8434C77, MSGBOX_AUTOCLOSE
	end

Kanto_Route9_EventScript_845A361::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8657, 0, Kanto_Route9_Text_846EFDD, Kanto_Route9_Text_8434C46
	msgbox Kanto_Route9_Text_8434C77, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route9 object 11 (local id 11, gfx 0x3F0E) at 12,17
Kanto_Route9_EventScript_846BB6F::
	setorcopyvar VAR_0x8000, ITEM_OVAL_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_Route9 object 12 (local id 12, gfx 0x3F0E) at 65,17
Kanto_Route9_EventScript_846BFE3::
	setorcopyvar VAR_0x8000, ITEM_BURN_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_Route9_Text_8434A7E::
	.string "You have POKéMON with you!\n"
	.string "You’re mine!$"

Kanto_Route9_Text_8434AA6::
	.string "You deceived me…$"

Kanto_Route9_Text_8434AB7::
	.string "The tunnel coming up is pitch-black\n"
	.string "inside.\p"
	.string "You’ll need FLASH to get through\n"
	.string "there.$"

Kanto_Route9_Text_8434B0B::
	.string "Who’s that walking there with those\n"
	.string "good-looking POKéMON?$"

Kanto_Route9_Text_8434B45::
	.string "Out like a light!$"

Kanto_Route9_Text_8434B57::
	.string "Keep walking!$"

Kanto_Route9_Text_8434B65::
	.string "I’m taking the ROCK TUNNEL to\n"
	.string "go to LAVENDER…$"

Kanto_Route9_Text_8434B93::
	.string "Can’t measure up…$"

Kanto_Route9_Text_8434BA5::
	.string "Are you off to ROCK TUNNEL, too?$"

Kanto_Route9_Text_8434BC6::
	.string "Don’t you dare patronize me!$"

Kanto_Route9_Text_8434BE3::
	.string "No!\n"
	.string "You’re too much.$"

Kanto_Route9_Text_8434BF8::
	.string "You’re obviously talented.\n"
	.string "Good luck to you!$"

Kanto_Route9_Text_8434C25::
	.string "Bwahaha!\n"
	.string "Great! I was bored, eh!$"

Kanto_Route9_Text_8434C46::
	.string "Keep it coming, eh!\p"
	.string "Oh wait.\n"
	.string "I’m out of POKéMON!$"

Kanto_Route9_Text_8434C77::
	.string "You sure had guts standing up to\n"
	.string "me there, eh?$"

Kanto_Route9_Text_8434CA6::
	.string "Hahaha!\n"
	.string "Aren’t you a little toughie!$"

Kanto_Route9_Text_8434CCB::
	.string "What’s that?$"

Kanto_Route9_Text_8434CD8::
	.string "Hahaha!\n"
	.string "Kids should be tough!$"

Kanto_Route9_Text_8434CF6::
	.string "I got up early every day to raise\n"
	.string "my POKéMON from cocoons!$"

Kanto_Route9_Text_8434D31::
	.string "WHAT?\p"
	.string "What a total waste of time!$"

Kanto_Route9_Text_8434D53::
	.string "I have to collect more than bugs\n"
	.string "to get stronger…$"

Kanto_Route9_Text_8434D85::
	.string "Hahahaha!\n"
	.string "Bring it on!$"

Kanto_Route9_Text_8434D9C::
	.string "Hahahaha!\n"
	.string "You beat me fair!$"

Kanto_Route9_Text_8434DB8::
	.string "Hahahaha!\n"
	.string "Us hearty guys always laugh!$"

Kanto_Route9_Text_8434DDF::
	.string "Go, my super BUG POKéMON!$"

Kanto_Route9_Text_8434DF9::
	.string "My bugs…$"

Kanto_Route9_Text_8434E02::
	.string "If you don’t like BUG POKéMON,\n"
	.string "you bug me!$"

Kanto_Route9_Text_8434E2D::
	.string "ROUTE 9\n"
	.string "CERULEAN CITY - ROCK TUNNEL$"

Kanto_Route9_Text_846EED4::
	.string "We’re to battle again?\n"
	.string "This time, you’re mine!$"

Kanto_Route9_Text_846EF03::
	.string "I haven’t forgotten you were with\n"
	.string "those good-looking POKéMON.$"

Kanto_Route9_Text_846EF41::
	.string "I’m taking ROCK TUNNEL to go to\n"
	.string "LAVENDER…\p"
	.string "But I keep getting stopped by\n"
	.string "everyone along the way…$"

Kanto_Route9_Text_846EFA1::
	.string "Don’t you dare patronize me today!\n"
	.string "We’re playing for keeps!$"

Kanto_Route9_Text_846EFDD::
	.string "Bwahaha!\n"
	.string "Great! I was bored again, eh!$"

Kanto_Route9_Text_846F004::
	.string "Hahaha!\n"
	.string "A little toughie, as always!$"

Kanto_Route9_Text_846F029::
	.string "I got up early every day to train\n"
	.string "my POKéMON from cocoons!$"

Kanto_Route9_Text_846F064::
	.string "Hahahaha!\n"
	.string "I’ll win this time!$"

Kanto_Route9_Text_846F082::
	.string "Go win, my super BUG POKéMON!$"
