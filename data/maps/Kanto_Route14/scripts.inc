Kanto_Route14_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route14 sign / bg event at 17,12
Kanto_Route14_EventScript_8417DB3::
	msgbox Kanto_Route14_Text_8436665, MSGBOX_SIGN
	end

@ Kanto_Route14 object 6 (local id 6, gfx 0x3EE9) at 7,34
Kanto_Route14_EventScript_845ACCC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8388, 0, Kanto_Route14_Text_8436610, Kanto_Route14_Text_843662E
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845ACF3
	msgbox Kanto_Route14_Text_843663A, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845ACF3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8388, 0, Kanto_Route14_Text_846FA7C, Kanto_Route14_Text_843662E
	msgbox Kanto_Route14_Text_843663A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 9 (local id 9, gfx 0x3EE9) at 10,37
Kanto_Route14_EventScript_845AD0A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8399, 0, Kanto_Route14_Text_84364C3, Kanto_Route14_Text_84364EF
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AD31
	msgbox Kanto_Route14_Text_8436505, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AD31::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8399, 0, Kanto_Route14_Text_846F9D7, Kanto_Route14_Text_84364EF
	msgbox Kanto_Route14_Text_8436505, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 5 (local id 5, gfx 0x3EE9) at 7,31
Kanto_Route14_EventScript_845AD48::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8400, 0, Kanto_Route14_Text_843652F, Kanto_Route14_Text_843655D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AD6F
	msgbox Kanto_Route14_Text_8436569, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AD6F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8400, 0, Kanto_Route14_Text_846FA18, Kanto_Route14_Text_843655D
	msgbox Kanto_Route14_Text_8436569, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 1 (local id 1, gfx 0x3EE9) at 7,37
Kanto_Route14_EventScript_845AD86::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8401, 0, Kanto_Route14_Text_8436589, Kanto_Route14_Text_84365BB
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845ADAD
	msgbox Kanto_Route14_Text_84365C4, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845ADAD::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8401, 0, Kanto_Route14_Text_846FA44, Kanto_Route14_Text_84365BB
	msgbox Kanto_Route14_Text_84365C4, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 2 (local id 2, gfx 0x3F3B) at 18,35
Kanto_Route14_EventScript_845ADC4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8495, 0, Kanto_Route14_Text_84363A5, Kanto_Route14_Text_84363D6
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845ADEB
	msgbox Kanto_Route14_Text_84363E9, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845ADEB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8495, 0, Kanto_Route14_Text_846F971, Kanto_Route14_Text_84363D6
	msgbox Kanto_Route14_Text_84363E9, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 10 (local id 10, gfx 0x3F3B) at 18,47
Kanto_Route14_EventScript_845AE02::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8496, 0, Kanto_Route14_Text_843645A, Kanto_Route14_Text_843647F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AE29
	msgbox Kanto_Route14_Text_843648A, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AE29::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8496, 0, Kanto_Route14_Text_846F9AB, Kanto_Route14_Text_843647F
	msgbox Kanto_Route14_Text_843648A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 8 (local id 8, gfx 0x3F3B) at 7,6
Kanto_Route14_EventScript_845AE40::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8505, 0, Kanto_Route14_Text_843610C, Kanto_Route14_Text_8436147
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AE67
	msgbox Kanto_Route14_Text_8436162, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AE67::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8505, 0, Kanto_Route14_Text_846F867, Kanto_Route14_Text_8436147
	msgbox Kanto_Route14_Text_8436162, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 7 (local id 7, gfx 0x3F3B) at 8,9
Kanto_Route14_EventScript_845AE7E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8506, 0, Kanto_Route14_Text_8436198, Kanto_Route14_Text_84361C4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AEA5
	msgbox Kanto_Route14_Text_84361D3, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AEA5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8506, 0, Kanto_Route14_Text_846F895, Kanto_Route14_Text_84361C4
	msgbox Kanto_Route14_Text_84361D3, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 3 (local id 3, gfx 0x3F3B) at 10,14
Kanto_Route14_EventScript_845AEBC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8507, 0, Kanto_Route14_Text_84361FF, Kanto_Route14_Text_843627A
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AEE3
	msgbox Kanto_Route14_Text_8436287, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AEE3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8507, 0, Kanto_Route14_Text_846F8CB, Kanto_Route14_Text_843627A
	msgbox Kanto_Route14_Text_8436287, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 4 (local id 4, gfx 0x3F3B) at 16,16
Kanto_Route14_EventScript_845AEFA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8508, 0, Kanto_Route14_Text_84362ED, Kanto_Route14_Text_8436348
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AF21
	msgbox Kanto_Route14_Text_843635D, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AF21::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8508, 0, Kanto_Route14_Text_846F907, Kanto_Route14_Text_8436348
	msgbox Kanto_Route14_Text_843635D, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 15 (local id 15, gfx 0x3F18) at 12,51
Kanto_Route14_EventScript_845AF38::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8679, 0, Kanto_Route14_Text_8436683, Kanto_Route14_Text_84366B6, Kanto_Route14_Text_84366FA
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AF63
	msgbox Kanto_Route14_Text_84366D9, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AF63::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8679, 0, Kanto_Route14_Text_846FAA1, Kanto_Route14_Text_84366B6, Kanto_Route14_Text_84366FA
	msgbox Kanto_Route14_Text_84366D9, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route14 object 14 (local id 14, gfx 0x3F18) at 13,51
Kanto_Route14_EventScript_845AF7E::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8679, 0, Kanto_Route14_Text_8436727, Kanto_Route14_Text_8436753, Kanto_Route14_Text_843679F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route14_EventScript_845AFA9
	msgbox Kanto_Route14_Text_8436768, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_EventScript_845AFA9::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8679, 0, Kanto_Route14_Text_846FAC1, Kanto_Route14_Text_8436753, Kanto_Route14_Text_843679F
	msgbox Kanto_Route14_Text_8436768, MSGBOX_AUTOCLOSE
	end

Kanto_Route14_Text_843610C::
	.string "You should review your POKéMON’s\n"
	.string "moves from the team menu.$"

Kanto_Route14_Text_8436147::
	.string "Not good enough quite yet.$"

Kanto_Route14_Text_8436162::
	.string "Field techniques become available\n"
	.string "as you earn BADGES.$"

Kanto_Route14_Text_8436198::
	.string "My bird POKéMON should be ready\n"
	.string "for battle.$"

Kanto_Route14_Text_84361C4::
	.string "Not ready yet!$"

Kanto_Route14_Text_84361D3::
	.string "My bird POKéMON need to learn\n"
	.string "better moves.$"

Kanto_Route14_Text_84361FF::
	.string "They have useful items on sale at\n"
	.string "the CELADON DEPT. STORE.\p"
	.string "Evolution items are rare, so I\n"
	.string "always keep an eye out for them.$"

Kanto_Route14_Text_843627A::
	.string "Aww, bummer!$"

Kanto_Route14_Text_8436287::
	.string "Try teaching POKéMON a move that’s\n"
	.string "the same type as it.\p"
	.string "That apparently boosts the power\n"
	.string "of the move.$"

Kanto_Route14_Text_84362ED::
	.string "Have you taught your bird POKéMON\n"
	.string "how to FLY?\p"
	.string "You’ll be able to soar with it into\n"
	.string "the sky!$"

Kanto_Route14_Text_8436348::
	.string "Shot down in flames!$"

Kanto_Route14_Text_843635D::
	.string "Bird POKéMON are my one true love.\n"
	.string "I don’t want to raise anything else.$"

Kanto_Route14_Text_84363A5::
	.string "Have you heard the legend of the\n"
	.string "winged mirages?$"

Kanto_Route14_Text_84363D6::
	.string "Why?\n"
	.string "Why’d I lose?$"

Kanto_Route14_Text_84363E9::
	.string "Well, the winged mirages are the\n"
	.string "legendary bird POKéMON.\p"
	.string "There are three of them: ARTICUNO,\n"
	.string "ZAPDOS, and MOLTRES.$"

Kanto_Route14_Text_843645A::
	.string "I’m not into it, but okay.\n"
	.string "Let’s go!$"

Kanto_Route14_Text_843647F::
	.string "I knew it!$"

Kanto_Route14_Text_843648A::
	.string "Winning, losing… It’s insignificant\n"
	.string "under this huge sky.$"

Kanto_Route14_Text_84364C3::
	.string "C’mon, c’mon.\n"
	.string "Let’s go, let’s go, let’s go!$"

Kanto_Route14_Text_84364EF::
	.string "Arrg!\n"
	.string "Lost! Get lost!$"

Kanto_Route14_Text_8436505::
	.string "What, what, what?\n"
	.string "What do you want still?$"

Kanto_Route14_Text_843652F::
	.string "I need to burn some time.\n"
	.string "Shut up and battle.$"

Kanto_Route14_Text_843655D::
	.string "What?\n"
	.string "You!?$"

Kanto_Route14_Text_8436569::
	.string "Raising POKéMON is a drag, man.$"

Kanto_Route14_Text_8436589::
	.string "We ride out here because of the\n"
	.string "wide-open spaces.$"

Kanto_Route14_Text_84365BB::
	.string "Wipeout!$"

Kanto_Route14_Text_84365C4::
	.string "It’s cool you made your POKéMON so\n"
	.string "strong.\p"
	.string "Might is right!\n"
	.string "And you know it!$"

Kanto_Route14_Text_8436610::
	.string "POKéMON battle?\n"
	.string "Cool! Rumble!$"

Kanto_Route14_Text_843662E::
	.string "Blown away!$"

Kanto_Route14_Text_843663A::
	.string "You know who’d win, you and me\n"
	.string "one-on-one!$"

Kanto_Route14_Text_8436665::
	.string "ROUTE 14\n"
	.string "West to FUCHSIA CITY$"

Kanto_Route14_Text_8436683::
	.string "KIRI: JAN, let’s try really,\n"
	.string "really hard together.$"

Kanto_Route14_Text_84366B6::
	.string "KIRI: Whimper…\n"
	.string "We lost, didn’t we?$"

Kanto_Route14_Text_84366D9::
	.string "KIRI: Did we lose because of me?$"

Kanto_Route14_Text_84366FA::
	.string "KIRI: We can battle if you have\n"
	.string "two POKéMON.$"

Kanto_Route14_Text_8436727::
	.string "JAN: KIRI, here we go!\n"
	.string "We have to try hard!$"

Kanto_Route14_Text_8436753::
	.string "JAN: Eeeeh!\n"
	.string "No fair!$"

Kanto_Route14_Text_8436768::
	.string "JAN: KIRI, don’t cry!\n"
	.string "We’ll just try harder next time.$"

Kanto_Route14_Text_843679F::
	.string "JAN: You want to battle?\n"
	.string "You don’t have enough POKéMON.$"

Kanto_Route14_Text_846F867::
	.string "I used TMs to teach good moves\n"
	.string "to my POKéMON.$"

Kanto_Route14_Text_846F895::
	.string "My bird POKéMON should be ready\n"
	.string "for battle this time.$"

Kanto_Route14_Text_846F8CB::
	.string "Are you using TMs on POKéMON?\n"
	.string "Just holding them is useless.$"

Kanto_Route14_Text_846F907::
	.string "Have you taught your bird POKéMON\n"
	.string "how to FLY?\p"
	.string "You’ll be able to soar with it into\n"
	.string "the sky! Give it a try.$"

Kanto_Route14_Text_846F971::
	.string "The legend of the winged mirages…\n"
	.string "You’ve heard it, right?$"

Kanto_Route14_Text_846F9AB::
	.string "I’m really not into it, but okay.\n"
	.string "Let’s go!$"

Kanto_Route14_Text_846F9D7::
	.string "Hey!\n"
	.string "I remember you!\p"
	.string "C’mon, c’mon.\n"
	.string "Let’s go, let’s go, let’s go!$"

Kanto_Route14_Text_846FA18::
	.string "You’re here again, huh?\n"
	.string "Shut up and battle.$"

Kanto_Route14_Text_846FA44::
	.string "We’ve been riding here because of\n"
	.string "the wide-open spaces.$"

Kanto_Route14_Text_846FA7C::
	.string "POKéMON battle, right?\n"
	.string "Cool! Rumble!$"

Kanto_Route14_Text_846FAA1::
	.string "KIRI: JAN, I hope we win today.$"

Kanto_Route14_Text_846FAC1::
	.string "JAN: KIRI, here we go!\n"
	.string "Let’s win for sure this time!$"
