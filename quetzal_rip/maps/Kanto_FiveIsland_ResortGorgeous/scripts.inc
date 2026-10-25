Kanto_FiveIsland_ResortGorgeous_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_FiveIsland_ResortGorgeous_OnWarp_84184EE
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_FiveIsland_ResortGorgeous_OnFrame_84184FD
	.byte 0

Kanto_FiveIsland_ResortGorgeous_OnWarp_84184EE:
	map_script_2 VAR_QZ_0x510F, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_84184F8
	.2byte 0

Kanto_FiveIsland_ResortGorgeous_OnFrame_84184FD:
	map_script_2 VAR_QZ_0x510F, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_8418507
	.2byte 0

Kanto_FiveIsland_ResortGorgeous_EventScript_84184F8::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_8418507::
	lockall
	textcolor 1
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843AAC4, MSGBOX_DEFAULT
	closemessage
	applymovement 8, Quetzal_Movement_8457AAC
	waitmovement 0
	opendoor 0x27, 8
	waitdooranim
	applymovement 8, Kanto_FiveIsland_ResortGorgeous_Movement_8418540
	waitmovement 0
	closedoor 0x27, 8
	waitdooranim
	removeobject 8
	clearflag FLAG_QZ_0x21AD
	setvar VAR_QZ_0x510F, 2
	releaseall
	end

@ Kanto_FiveIsland_ResortGorgeous sign / bg event at 42,9
Kanto_FiveIsland_ResortGorgeous_EventScript_8418543::
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843AAE7, MSGBOX_SIGN
	end

@ Kanto_FiveIsland_ResortGorgeous object 1 (local id 1, gfx 0x3F16) at 44,10
Kanto_FiveIsland_ResortGorgeous_EventScript_845C4F8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8718, 0, Kanto_FiveIsland_ResortGorgeous_Text_843A6CB, Kanto_FiveIsland_ResortGorgeous_Text_843A6F7
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_845C51F
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A719, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_845C51F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8718, 0, Kanto_FiveIsland_ResortGorgeous_Text_847109C, Kanto_FiveIsland_ResortGorgeous_Text_843A6F7
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A719, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_ResortGorgeous object 2 (local id 2, gfx 0x3F16) at 33,12
Kanto_FiveIsland_ResortGorgeous_EventScript_845C536::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8754, 0, Kanto_FiveIsland_ResortGorgeous_Text_843A74A, Kanto_FiveIsland_ResortGorgeous_Text_843A787
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_845C55D
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A7AF, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_845C55D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8754, 0, Kanto_FiveIsland_ResortGorgeous_Text_84710CE, Kanto_FiveIsland_ResortGorgeous_Text_843A787
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A7AF, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_ResortGorgeous object 3 (local id 3, gfx 0x3F16) at 12,10
Kanto_FiveIsland_ResortGorgeous_EventScript_845C574::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8755, 0, Kanto_FiveIsland_ResortGorgeous_Text_843A7F7, Kanto_FiveIsland_ResortGorgeous_Text_843A81E
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_845C59B
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A83E, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_845C59B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8755, 0, Kanto_FiveIsland_ResortGorgeous_Text_8471116, Kanto_FiveIsland_ResortGorgeous_Text_843A81E
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A83E, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_ResortGorgeous object 4 (local id 4, gfx 0x3F58) at 23,9
Kanto_FiveIsland_ResortGorgeous_EventScript_845C5B2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8717, 0, Kanto_FiveIsland_ResortGorgeous_Text_843A8A2, Kanto_FiveIsland_ResortGorgeous_Text_843A8D7
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_845C5D9
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A8EA, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_845C5D9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8717, 0, Kanto_FiveIsland_ResortGorgeous_Text_8471156, Kanto_FiveIsland_ResortGorgeous_Text_843A8D7
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A8EA, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_ResortGorgeous object 5 (local id 5, gfx 0x3F58) at 33,8
Kanto_FiveIsland_ResortGorgeous_EventScript_845C5F0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8756, 0, Kanto_FiveIsland_ResortGorgeous_Text_843A92E, Kanto_FiveIsland_ResortGorgeous_Text_843A968
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_845C617
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A982, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_845C617::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8756, 0, Kanto_FiveIsland_ResortGorgeous_Text_8471197, Kanto_FiveIsland_ResortGorgeous_Text_843A968
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A982, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_ResortGorgeous object 6 (local id 6, gfx 0x3F5C) at 33,3
Kanto_FiveIsland_ResortGorgeous_EventScript_845C62E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8757, 0, Kanto_FiveIsland_ResortGorgeous_Text_843A9C3, Kanto_FiveIsland_ResortGorgeous_Text_843A9E7
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_845C655
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A9FE, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_845C655::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8757, 0, Kanto_FiveIsland_ResortGorgeous_Text_84711ED, Kanto_FiveIsland_ResortGorgeous_Text_843A9E7
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843A9FE, MSGBOX_AUTOCLOSE
	end

@ Kanto_FiveIsland_ResortGorgeous object 7 (local id 7, gfx 0x3F4F) at 56,7
Kanto_FiveIsland_ResortGorgeous_EventScript_845C66C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8758, 0, Kanto_FiveIsland_ResortGorgeous_Text_843AA33, Kanto_FiveIsland_ResortGorgeous_Text_843AA6F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_FiveIsland_ResortGorgeous_EventScript_845C693
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843AA95, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_EventScript_845C693::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8758, 0, Kanto_FiveIsland_ResortGorgeous_Text_8471219, Kanto_FiveIsland_ResortGorgeous_Text_843AA6F
	msgbox Kanto_FiveIsland_ResortGorgeous_Text_843AA95, MSGBOX_AUTOCLOSE
	end

Kanto_FiveIsland_ResortGorgeous_Movement_8418540:
	walk_up
	set_invisible
	step_end

Kanto_FiveIsland_ResortGorgeous_Text_843A6CB::
	.string "With these hands, I will create\n"
	.string "my victory!$"

Kanto_FiveIsland_ResortGorgeous_Text_843A6F7::
	.string "The shame of this…\n"
	.string "I’m red-faced…$"

Kanto_FiveIsland_ResortGorgeous_Text_843A719::
	.string "Uh-oh, my paint tubes have\n"
	.string "scattered everywhere!$"

Kanto_FiveIsland_ResortGorgeous_Text_843A74A::
	.string "Excuse me!\n"
	.string "I’m trying to paint.\l"
	.string "Please keep out of my sight!$"

Kanto_FiveIsland_ResortGorgeous_Text_843A787::
	.string "This loss…\n"
	.string "I feel like I’m whiting out…$"

Kanto_FiveIsland_ResortGorgeous_Text_843A7AF::
	.string "GILLIAN?\n"
	.string "Please tilt your head slightly…\l"
	.string "There! That’s the look I want!$"

Kanto_FiveIsland_ResortGorgeous_Text_843A7F7::
	.string "Hmm…\n"
	.string "The angle, it is not quite there…$"

Kanto_FiveIsland_ResortGorgeous_Text_843A81E::
	.string "Oh…\n"
	.string "My feelings, they are blue…$"

Kanto_FiveIsland_ResortGorgeous_Text_843A83E::
	.string "The sky and the sea…\n"
	.string "They are so fickle.\p"
	.string "Their changing expressions make\n"
	.string "them difficult to capture.$"

Kanto_FiveIsland_ResortGorgeous_Text_843A8A2::
	.string "I am visiting my friend’s retreat,\n"
	.string "if you must know.$"

Kanto_FiveIsland_ResortGorgeous_Text_843A8D7::
	.string "Oh, my, so strong.$"

Kanto_FiveIsland_ResortGorgeous_Text_843A8EA::
	.string "I find it pleasurable to leisurely\n"
	.string "battle in the gentle sea breeze.$"

Kanto_FiveIsland_ResortGorgeous_Text_843A92E::
	.string "I’m thinking of having a pool\n"
	.string "made specially for POKéMON.$"

Kanto_FiveIsland_ResortGorgeous_Text_843A968::
	.string "Oh, my goodness gracious!$"

Kanto_FiveIsland_ResortGorgeous_Text_843A982::
	.string "A pool would be nice, but I must\n"
	.string "say, a sun-room may be sublime.$"

Kanto_FiveIsland_ResortGorgeous_Text_843A9C3::
	.string "I’m a good runner.\n"
	.string "I’m really fast!$"

Kanto_FiveIsland_ResortGorgeous_Text_843A9E7::
	.string "Oops!\n"
	.string "I rushed things!$"

Kanto_FiveIsland_ResortGorgeous_Text_843A9FE::
	.string "When I run with the wind, it feels\n"
	.string "like I could fly!$"

Kanto_FiveIsland_ResortGorgeous_Text_843AA33::
	.string "I say, this marine life here at\n"
	.string "this resort agrees with me.$"

Kanto_FiveIsland_ResortGorgeous_Text_843AA6F::
	.string "That’s a shocker!\n"
	.string "You must be joking.$"

Kanto_FiveIsland_ResortGorgeous_Text_843AA95::
	.string "Getting so serious over this…\n"
	.string "How nonsensical.$"

Kanto_FiveIsland_ResortGorgeous_Text_843AAC4::
	.string "SELPHY: Thank you.\p"
	.string "You may go now.$"

Kanto_FiveIsland_ResortGorgeous_Text_843AAE7::
	.string "SELPHY’S HOUSE$"

Kanto_FiveIsland_ResortGorgeous_Text_847109C::
	.string "With these hands, I will create\n"
	.string "my victory today.$"

Kanto_FiveIsland_ResortGorgeous_Text_84710CE::
	.string "Must I repeat myself?\n"
	.string "I’m trying to paint.\l"
	.string "Please keep out of my sight!$"

Kanto_FiveIsland_ResortGorgeous_Text_8471116::
	.string "I haven’t made any progress…\n"
	.string "I still can’t get the right angle…$"

Kanto_FiveIsland_ResortGorgeous_Text_8471156::
	.string "Oh, you will give me another\n"
	.string "opportunity to match wits with you?$"

Kanto_FiveIsland_ResortGorgeous_Text_8471197::
	.string "They’re almost finished making the\n"
	.string "pool for my POKéMON.\p"
	.string "You must drop in for a visit.$"

Kanto_FiveIsland_ResortGorgeous_Text_84711ED::
	.string "I’m a good runner.\n"
	.string "I’ve gotten even faster!$"

Kanto_FiveIsland_ResortGorgeous_Text_8471219::
	.string "I say, friend!\n"
	.string "Let us enjoy our time together!$"
