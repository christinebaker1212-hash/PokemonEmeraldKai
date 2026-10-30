Kanto_Route13_MapScripts::
	.byte 0

@ Kanto_Route13 sign / bg event at 21,14
Kanto_Route13_EventScript_8417D92::
	msgbox Kanto_Route13_Text_8436076, MSGBOX_SIGN
	end

@ Kanto_Route13 sign / bg event at 37,6
Kanto_Route13_EventScript_8417D9B::
	msgbox Kanto_Route13_Text_84360AE, MSGBOX_SIGN
	end

@ Kanto_Route13 sign / bg event at 41,12
Kanto_Route13_EventScript_8417DA4::
	msgbox Kanto_Route13_Text_84360EB, MSGBOX_SIGN
	end

@ Kanto_Route13 object 10 (local id 10, gfx 0x3EE9) at 14,8
Kanto_Route13_EventScript_845AA60::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8387, 0, Kanto_Route13_Text_8435FC4, Kanto_Route13_Text_8435FDC
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845AA87
	msgbox Kanto_Route13_Text_8435FF2, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845AA87::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8387, 0, Kanto_Route13_Text_846F811, Kanto_Route13_Text_8435FDC
	msgbox Kanto_Route13_Text_8435FF2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 5 (local id 5, gfx 0x3EE8) at 43,7
Kanto_Route13_EventScript_845AA9E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8460, 0, Kanto_Route13_Text_8435EBB, Kanto_Route13_Text_8435EDE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845AAC5
	msgbox Kanto_Route13_Text_8435EF4, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845AAC5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8460, 0, Kanto_Route13_Text_846F7AC, Kanto_Route13_Text_8435EDE
	msgbox Kanto_Route13_Text_8435EF4, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 4 (local id 4, gfx 0x3EE8) at 42,7
Kanto_Route13_EventScript_845AADC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8461, 0, Kanto_Route13_Text_8435F28, Kanto_Route13_Text_8435F54
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845AB03
	msgbox Kanto_Route13_Text_8435F67, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845AB03::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8461, 0, Kanto_Route13_Text_846F7E6, Kanto_Route13_Text_8435F54
	msgbox Kanto_Route13_Text_8435F67, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 2 (local id 2, gfx 0x3F3B) at 63,12
Kanto_Route13_EventScript_845AB1A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8492, 0, Kanto_Route13_Text_8435C1C, Kanto_Route13_Text_8435C45
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845AB41
	msgbox Kanto_Route13_Text_8435C61, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845AB41::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8492, 0, Kanto_Route13_Text_846F694, Kanto_Route13_Text_8435C45
	msgbox Kanto_Route13_Text_8435C61, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 9 (local id 9, gfx 0x3F3B) at 16,5
Kanto_Route13_EventScript_845AB58::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8493, 0, Kanto_Route13_Text_8435E4E, Kanto_Route13_Text_8435E89
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845AB7F
	msgbox Kanto_Route13_Text_8435E9A, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845AB7F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8493, 0, Kanto_Route13_Text_846F770, Kanto_Route13_Text_8435E89
	msgbox Kanto_Route13_Text_8435E9A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 8 (local id 8, gfx 0x3F3B) at 9,14
Kanto_Route13_EventScript_845AB96::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8494, 0, Kanto_Route13_Text_8435FFC, Kanto_Route13_Text_843603A
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845ABBD
	msgbox Kanto_Route13_Text_8436048, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845ABBD::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8494, 0, Kanto_Route13_Text_846F829, Kanto_Route13_Text_843603A
	msgbox Kanto_Route13_Text_8436048, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 1 (local id 1, gfx 0x3F31) at 62,6
Kanto_Route13_EventScript_845ABD4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8658, 0, Kanto_Route13_Text_8435DD1, Kanto_Route13_Text_8435E07
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845ABFB
	msgbox Kanto_Route13_Text_8435E26, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845ABFB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8658, 0, Kanto_Route13_Text_846F73F, Kanto_Route13_Text_8435E07
	msgbox Kanto_Route13_Text_8435E26, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 3 (local id 3, gfx 0x3F31) at 54,11
Kanto_Route13_EventScript_845AC12::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8659, 0, Kanto_Route13_Text_8435C8E, Kanto_Route13_Text_8435CAB
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845AC39
	msgbox Kanto_Route13_Text_8435CB8, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845AC39::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8659, 0, Kanto_Route13_Text_846F6B2, Kanto_Route13_Text_8435CAB
	msgbox Kanto_Route13_Text_8435CB8, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 6 (local id 6, gfx 0x3F31) at 35,10
Kanto_Route13_EventScript_845AC50::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8660, 0, Kanto_Route13_Text_8435CF6, Kanto_Route13_Text_8435D15
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845AC77
	msgbox Kanto_Route13_Text_8435D21, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845AC77::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8660, 0, Kanto_Route13_Text_846F6E2, Kanto_Route13_Text_8435D15
	msgbox Kanto_Route13_Text_8435D21, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route13 object 7 (local id 7, gfx 0x3F31) at 29,11
Kanto_Route13_EventScript_845AC8E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8661, 0, Kanto_Route13_Text_8435D50, Kanto_Route13_Text_8435D80
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route13_EventScript_845ACB5
	msgbox Kanto_Route13_Text_8435D9D, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_EventScript_845ACB5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8661, 0, Kanto_Route13_Text_846F701, Kanto_Route13_Text_8435D80
	msgbox Kanto_Route13_Text_8435D9D, MSGBOX_AUTOCLOSE
	end

Kanto_Route13_Text_8435C1C::
	.string "My bird POKéMON want to battle\n"
	.string "with you!$"

Kanto_Route13_Text_8435C45::
	.string "My bird POKéMON combo lost?$"

Kanto_Route13_Text_8435C61::
	.string "My POKéMON look happy even though\n"
	.string "they lost.$"

Kanto_Route13_Text_8435C8E::
	.string "I’m told I’m good for a kid.$"

Kanto_Route13_Text_8435CAB::
	.string "Ohh!\n"
	.string "I lost!$"

Kanto_Route13_Text_8435CB8::
	.string "I want to become a good TRAINER.\n"
	.string "I’ll train hard, just watch.$"

Kanto_Route13_Text_8435CF6::
	.string "Wow!\n"
	.string "Your BADGES are too cool!$"

Kanto_Route13_Text_8435D15::
	.string "Not enough!$"

Kanto_Route13_Text_8435D21::
	.string "You got those BADGES from GYM\n"
	.string "LEADERS. I know!$"

Kanto_Route13_Text_8435D50::
	.string "My cute POKéMON wish to make your\n"
	.string "acquaintance.$"

Kanto_Route13_Text_8435D80::
	.string "Good going!\n"
	.string "You totally won!$"

Kanto_Route13_Text_8435D9D::
	.string "You have to make POKéMON battle\n"
	.string "to toughen them up.$"

Kanto_Route13_Text_8435DD1::
	.string "I found CARBOS in a cave once\n"
	.string "while I was spelunking.$"

Kanto_Route13_Text_8435E07::
	.string "Oh, too bad!\n"
	.string "I just messed up!$"

Kanto_Route13_Text_8435E26::
	.string "CARBOS boosted the SPEED of my\n"
	.string "POKéMON.$"

Kanto_Route13_Text_8435E4E::
	.string "I’m not going to lose.\n"
	.string "Not when the wind’s blowing my way!$"

Kanto_Route13_Text_8435E89::
	.string "The wind turned!$"

Kanto_Route13_Text_8435E9A::
	.string "I’m beat.\n"
	.string "I guess I’ll FLY home.$"

Kanto_Route13_Text_8435EBB::
	.string "Sure, I’ll play with you, sweetie.$"

Kanto_Route13_Text_8435EDE::
	.string "Oh!\n"
	.string "You little beast!$"

Kanto_Route13_Text_8435EF4::
	.string "I wonder which is stronger, male or\n"
	.string "female POKéMON?$"

Kanto_Route13_Text_8435F28::
	.string "Do you want to battle some\n"
	.string "POKéMON with me?$"

Kanto_Route13_Text_8435F54::
	.string "It’s over already?$"

Kanto_Route13_Text_8435F67::
	.string "I don’t know anything about\n"
	.string "POKéMON actually.\p"
	.string "The ones I use… I picked them\n"
	.string "for their looks!$"

Kanto_Route13_Text_8435FC4::
	.string "What’re you lookin’ at?$"

Kanto_Route13_Text_8435FDC::
	.string "Dang!\n"
	.string "Stripped gears!$"

Kanto_Route13_Text_8435FF2::
	.string "Get lost!$"

Kanto_Route13_Text_8435FFC::
	.string "I always go with bird POKéMON.\n"
	.string "I’ve dedicated myself to them.$"

Kanto_Route13_Text_843603A::
	.string "Out of power!$"

Kanto_Route13_Text_8436048::
	.string "I wish I could fly like PIDGEY and\n"
	.string "PIDGEOTTO…$"

Kanto_Route13_Text_8436076::
	.string "TRAINER TIPS\p"
	.string "Look, look!\n"
	.string "Look to the left of that post!$"

Kanto_Route13_Text_84360AE::
	.string "TRAINER TIPS\p"
	.string "Use SELECT to switch items in the\n"
	.string "ITEMS window.$"

Kanto_Route13_Text_84360EB::
	.string "ROUTE 13\n"
	.string "North to SILENCE BRIDGE$"

Kanto_Route13_Text_846F694::
	.string "My bird POKéMON remember you!$"

Kanto_Route13_Text_846F6B2::
	.string "I want to be the best TRAINER \n"
	.string "while I’m a kid!$"

Kanto_Route13_Text_846F6E2::
	.string "Wow!\n"
	.string "You got more cool BADGES!$"

Kanto_Route13_Text_846F701::
	.string "My cutely grown POKéMON wish to\n"
	.string "become reacquainted with you.$"

Kanto_Route13_Text_846F73F::
	.string "I cleaned out my savings and\n"
	.string "bought more CARBOS.$"

Kanto_Route13_Text_846F770::
	.string "I’m not going to lose this time.\n"
	.string "The wind’s blowing my way!$"

Kanto_Route13_Text_846F7AC::
	.string "Oh, you’re back?\p"
	.string "Sure, I’ll play again with you,\n"
	.string "sweetie.$"

Kanto_Route13_Text_846F7E6::
	.string "Can’t you forget that you battled\n"
	.string "with me?$"

Kanto_Route13_Text_846F811::
	.string "What’re you starin’ at?$"

Kanto_Route13_Text_846F829::
	.string "I always go with bird POKéMON.\n"
	.string "I’ve dedicated myself to them.$"
