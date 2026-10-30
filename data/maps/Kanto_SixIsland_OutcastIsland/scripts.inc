Kanto_SixIsland_OutcastIsland_MapScripts::
	.byte 0

@ Kanto_SixIsland_OutcastIsland object 1 (local id 1, gfx 0x3F3D) at 9,24
Kanto_SixIsland_OutcastIsland_EventScript_84188C8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8732, 0, Kanto_SixIsland_OutcastIsland_Text_843B643, Kanto_SixIsland_OutcastIsland_Text_843B6AB
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B6B1, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_OutcastIsland object 2 (local id 2, gfx 0x3F06) at 12,15
Kanto_SixIsland_OutcastIsland_EventScript_845C7A2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8765, 0, Kanto_SixIsland_OutcastIsland_Text_843B6E9, Kanto_SixIsland_OutcastIsland_Text_843B724
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_OutcastIsland_EventScript_845C7C9
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B73A, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_OutcastIsland_EventScript_845C7C9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8765, 0, Kanto_SixIsland_OutcastIsland_Text_8471376, Kanto_SixIsland_OutcastIsland_Text_843B724
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B73A, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_OutcastIsland object 3 (local id 3, gfx 0x3F4F) at 13,34
Kanto_SixIsland_OutcastIsland_EventScript_845C7E0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8766, 0, Kanto_SixIsland_OutcastIsland_Text_843B778, Kanto_SixIsland_OutcastIsland_Text_843B7AE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_OutcastIsland_EventScript_845C807
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B7BA, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_OutcastIsland_EventScript_845C807::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8766, 0, Kanto_SixIsland_OutcastIsland_Text_84713B7, Kanto_SixIsland_OutcastIsland_Text_843B7AE
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B7BA, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_OutcastIsland object 4 (local id 4, gfx 0x3F4D) at 14,61
Kanto_SixIsland_OutcastIsland_EventScript_845C81E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8767, 0, Kanto_SixIsland_OutcastIsland_Text_843B7E3, Kanto_SixIsland_OutcastIsland_Text_843B822
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_OutcastIsland_EventScript_845C845
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B85C, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_OutcastIsland_EventScript_845C845::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8767, 0, Kanto_SixIsland_OutcastIsland_Text_84713F7, Kanto_SixIsland_OutcastIsland_Text_843B822
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B85C, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_OutcastIsland object 5 (local id 5, gfx 0x3F4D) at 10,44
Kanto_SixIsland_OutcastIsland_EventScript_845C85C::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8768, 0, Kanto_SixIsland_OutcastIsland_Text_843B8B1, Kanto_SixIsland_OutcastIsland_Text_843B8DD, Kanto_SixIsland_OutcastIsland_Text_843B949
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_OutcastIsland_EventScript_845C887
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B917, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_OutcastIsland_EventScript_845C887::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8768, 0, Kanto_SixIsland_OutcastIsland_Text_8471421, Kanto_SixIsland_OutcastIsland_Text_843B8DD, Kanto_SixIsland_OutcastIsland_Text_843B949
	msgbox Kanto_SixIsland_OutcastIsland_Text_843B917, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_OutcastIsland object 6 (local id 6, gfx 0x3F53) at 11,44
Kanto_SixIsland_OutcastIsland_EventScript_845C8A2::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8768, 0, Kanto_SixIsland_OutcastIsland_Text_843B9AA, Kanto_SixIsland_OutcastIsland_Text_843B9D8, Kanto_SixIsland_OutcastIsland_Text_843BA4B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_OutcastIsland_EventScript_845C8CD
	msgbox Kanto_SixIsland_OutcastIsland_Text_843BA09, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_OutcastIsland_EventScript_845C8CD::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8768, 0, Kanto_SixIsland_OutcastIsland_Text_8471459, Kanto_SixIsland_OutcastIsland_Text_843B9D8, Kanto_SixIsland_OutcastIsland_Text_843BA4B
	msgbox Kanto_SixIsland_OutcastIsland_Text_843BA09, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_OutcastIsland object 7 (local id 7, gfx 0x3F0E) at 11,16
Kanto_SixIsland_OutcastIsland_EventScript_846C176::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SixIsland_OutcastIsland_Text_843B643::
	.string "There’re no rare POKéMON around\n"
	.string "these parts! Not a one!\p"
	.string "That burns me up, man.\n"
	.string "I’ll take it out on you!$"

Kanto_SixIsland_OutcastIsland_Text_843B6AB::
	.string "…Huh?$"

Kanto_SixIsland_OutcastIsland_Text_843B6B1::
	.string "So listen, you haven’t seen any\n"
	.string "rare POKéMON, have you?$"

Kanto_SixIsland_OutcastIsland_Text_843B6E9::
	.string "I’m having no luck at all.\n"
	.string "A battle’d be a change of pace!$"

Kanto_SixIsland_OutcastIsland_Text_843B724::
	.string "Nope, no luck at all…$"

Kanto_SixIsland_OutcastIsland_Text_843B73A::
	.string "I can’t very well go home without\n"
	.string "catching something, though.$"

Kanto_SixIsland_OutcastIsland_Text_843B778::
	.string "Gasp… Gasp…\p"
	.string "I swam here from SIX ISLE PORT\n"
	.string "in one go.$"

Kanto_SixIsland_OutcastIsland_Text_843B7AE::
	.string "Gasp…\n"
	.string "Gasp…$"

Kanto_SixIsland_OutcastIsland_Text_843B7BA::
	.string "I’m only at the halfway point…\n"
	.string "I’m beat…$"

Kanto_SixIsland_OutcastIsland_Text_843B7E3::
	.string "It’s not so easy sending POKéMON\n"
	.string "out while swimming, you know?$"

Kanto_SixIsland_OutcastIsland_Text_843B822::
	.string "I didn’t lose to you at swimming.\n"
	.string "This doesn’t bother me.$"

Kanto_SixIsland_OutcastIsland_Text_843B85C::
	.string "Are you headed for the island up\n"
	.string "past here?\p"
	.string "I didn’t see anything interesting\n"
	.string "there.$"

Kanto_SixIsland_OutcastIsland_Text_843B8B1::
	.string "AVA: Let’s have a two-on-two\n"
	.string "marine battle!$"

Kanto_SixIsland_OutcastIsland_Text_843B8DD::
	.string "AVA: Oh, you’re amazing!\n"
	.string "Even better, you’re on your own!$"

Kanto_SixIsland_OutcastIsland_Text_843B917::
	.string "AVA: You know, I do prefer the\n"
	.string "sea over any pool.$"

Kanto_SixIsland_OutcastIsland_Text_843B949::
	.string "AVA: You’re challenging us to\n"
	.string "a battle?\p"
	.string "You’ll need at least two POKéMON\n"
	.string "if you want to do that.$"

Kanto_SixIsland_OutcastIsland_Text_843B9AA::
	.string "GEB: Big Sister, help!\n"
	.string "Please battle with me!$"

Kanto_SixIsland_OutcastIsland_Text_843B9D8::
	.string "GEB: Wow, Big Sister, this person’s\n"
	.string "really good!$"

Kanto_SixIsland_OutcastIsland_Text_843BA09::
	.string "GEB: I’m hanging onto my sister\n"
	.string "because I can’t touch the bottom.$"

Kanto_SixIsland_OutcastIsland_Text_843BA4B::
	.string "GEB: Please battle against me and\n"
	.string "my sister!\p"
	.string "…Oh, you don’t have two POKéMON?$"

Kanto_SixIsland_OutcastIsland_Text_8471376::
	.string "I’m still having no luck at all.\n"
	.string "A battle’d be a change of pace!$"

Kanto_SixIsland_OutcastIsland_Text_84713B7::
	.string "Gasp… Gasp…\p"
	.string "I swam here from SIX ISLAND’s port\n"
	.string "in one go again.$"

Kanto_SixIsland_OutcastIsland_Text_84713F7::
	.string "You always come along while I’m\n"
	.string "swimming.$"

Kanto_SixIsland_OutcastIsland_Text_8471421::
	.string "AVA: Let’s have another\n"
	.string "two-on-two marine battle today!$"

Kanto_SixIsland_OutcastIsland_Text_8471459::
	.string "GEB: My big sister and I are way\n"
	.string "tougher than before!$"
