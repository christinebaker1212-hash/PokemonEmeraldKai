Kanto_Route15_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route15 sign / bg event at 41,11
Kanto_Route15_EventScript_8417DC2::
	msgbox Kanto_Route15_Text_8436D43, MSGBOX_SIGN
	end

@ Kanto_Route15 object 7 (local id 7, gfx 0x3EE9) at 63,13
Kanto_Route15_EventScript_845AFC4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8389, 0, Kanto_Route15_Text_8436B82, Kanto_Route15_Text_8436BB4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845AFEB
	msgbox Kanto_Route15_Text_8436BBD, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845AFEB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8389, 0, Kanto_Route15_Text_846FC8D, Kanto_Route15_Text_8436BB4
	msgbox Kanto_Route15_Text_8436BBD, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 6 (local id 6, gfx 0x3EE9) at 59,12
Kanto_Route15_EventScript_845B002::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8390, 0, Kanto_Route15_Text_8436C02, Kanto_Route15_Text_8436C34
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B029
	msgbox Kanto_Route15_Text_8436C48, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B029::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8390, 0, Kanto_Route15_Text_846FCBA, Kanto_Route15_Text_8436C34
	msgbox Kanto_Route15_Text_8436C48, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 9 (local id 9, gfx 0x3EE8) at 54,12
Kanto_Route15_EventScript_845B040::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8465, 0, Kanto_Route15_Text_8436A57, Kanto_Route15_Text_8436A8D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B067
	msgbox Kanto_Route15_Text_8436AA6, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B067::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8465, 0, Kanto_Route15_Text_846FBF9, Kanto_Route15_Text_8436A8D
	msgbox Kanto_Route15_Text_8436AA6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 5 (local id 5, gfx 0x3EE8) at 50,12
Kanto_Route15_EventScript_845B07E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8466, 0, Kanto_Route15_Text_8436AD8, Kanto_Route15_Text_8436B0D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B0A5
	msgbox Kanto_Route15_Text_8436B3B, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B0A5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8466, 0, Kanto_Route15_Text_846FC43, Kanto_Route15_Text_8436B0D
	msgbox Kanto_Route15_Text_8436B3B, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 2 (local id 2, gfx 0x3F3B) at 31,12
Kanto_Route15_EventScript_845B0BC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8497, 0, Kanto_Route15_Text_843691B, Kanto_Route15_Text_8436946
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B0E3
	msgbox Kanto_Route15_Text_8436959, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B0E3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8497, 0, Kanto_Route15_Text_846FB79, Kanto_Route15_Text_8436946
	msgbox Kanto_Route15_Text_8436959, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 3 (local id 3, gfx 0x3F3B) at 37,14
Kanto_Route15_EventScript_845B0FA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8498, 0, Kanto_Route15_Text_843699D, Kanto_Route15_Text_84369D3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B121
	msgbox Kanto_Route15_Text_84369E6, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B121::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8498, 0, Kanto_Route15_Text_846FBBD, Kanto_Route15_Text_84369D3
	msgbox Kanto_Route15_Text_84369E6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 1 (local id 1, gfx 0x3F31) at 22,13
Kanto_Route15_EventScript_845B138::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8670, 0, Kanto_Route15_Text_8436CEE, Kanto_Route15_Text_8436D0C
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B15F
	msgbox Kanto_Route15_Text_8436D21, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B15F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8670, 0, Kanto_Route15_Text_846FD23, Kanto_Route15_Text_8436D0C
	msgbox Kanto_Route15_Text_8436D21, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 4 (local id 4, gfx 0x3F31) at 44,13
Kanto_Route15_EventScript_845B176::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8671, 0, Kanto_Route15_Text_84367D7, Kanto_Route15_Text_8436815
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B19D
	msgbox Kanto_Route15_Text_8436826, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B19D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8671, 0, Kanto_Route15_Text_846FAF6, Kanto_Route15_Text_8436815
	msgbox Kanto_Route15_Text_8436826, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 8 (local id 8, gfx 0x3F31) at 52,8
Kanto_Route15_EventScript_845B1B4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8672, 0, Kanto_Route15_Text_843689F, Kanto_Route15_Text_84368DE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B1DB
	msgbox Kanto_Route15_Text_84368E9, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B1DB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8672, 0, Kanto_Route15_Text_846FB33, Kanto_Route15_Text_84368DE
	msgbox Kanto_Route15_Text_84368E9, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 10 (local id 10, gfx 0x3F31) at 28,6
Kanto_Route15_EventScript_845B1F2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8673, 0, Kanto_Route15_Text_8436C86, Kanto_Route15_Text_8436CB2
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B219
	msgbox Kanto_Route15_Text_8436CC0, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B219::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8673, 0, Kanto_Route15_Text_846FCEC, Kanto_Route15_Text_8436CB2
	msgbox Kanto_Route15_Text_8436CC0, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 13 (local id 13, gfx 0x3EE7) at 40,7
Kanto_Route15_EventScript_845B230::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8680, 0, Kanto_Route15_Text_8436D61, Kanto_Route15_Text_8436D97, Kanto_Route15_Text_8436E12
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B25B
	msgbox Kanto_Route15_Text_8436DD6, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B25B::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8680, 0, Kanto_Route15_Text_846FD47, Kanto_Route15_Text_8436D97, Kanto_Route15_Text_8436E12
	msgbox Kanto_Route15_Text_8436DD6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 12 (local id 12, gfx 0x3EEB) at 39,7
Kanto_Route15_EventScript_845B276::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8680, 0, Kanto_Route15_Text_8436E55, Kanto_Route15_Text_8436E7D, Kanto_Route15_Text_8436ECA
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route15_EventScript_845B2A1
	msgbox Kanto_Route15_Text_8436E9E, MSGBOX_AUTOCLOSE
	end

Kanto_Route15_EventScript_845B2A1::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8680, 0, Kanto_Route15_Text_846FD92, Kanto_Route15_Text_8436E7D, Kanto_Route15_Text_8436ECA
	msgbox Kanto_Route15_Text_8436E9E, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route15 object 11 (local id 11, gfx 0x3F0E) at 20,6
Kanto_Route15_EventScript_846BC80::
	setorcopyvar VAR_0x8000, 0x1BF
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_Route15_Text_84367D7::
	.string "I just got some POKéMON in trades.\n"
	.string "Can I try them out on you?$"

Kanto_Route15_Text_8436815::
	.string "Not good enough!$"

Kanto_Route15_Text_8436826::
	.string "You can’t change the nickname of\n"
	.string "any POKéMON you get in a trade.\p"
	.string "Only the Original TRAINER can get\n"
	.string "the nickname changed.$"

Kanto_Route15_Text_843689F::
	.string "You look gentle, so I think I can\n"
	.string "beat you.\p"
	.string "I’ll give it a go!$"

Kanto_Route15_Text_84368DE::
	.string "No, wrong!$"

Kanto_Route15_Text_84368E9::
	.string "I’m afraid of BIKERS. They look so\n"
	.string "ugly and mean!$"

Kanto_Route15_Text_843691B::
	.string "When I whistle, I can summon bird\n"
	.string "POKéMON.$"

Kanto_Route15_Text_8436946::
	.string "Ow!\n"
	.string "That’s tragic!$"

Kanto_Route15_Text_8436959::
	.string "Maybe I’m not cut out for battles.\n"
	.string "I’m not the right type, I guess.$"

Kanto_Route15_Text_843699D::
	.string "Hmm? My birds are shivering!\n"
	.string "You’re good, aren’t you?$"

Kanto_Route15_Text_84369D3::
	.string "Just as I thought!$"

Kanto_Route15_Text_84369E6::
	.string "This is so obvious, you should know\n"
	.string "this, but…\p"
	.string "Moves like EARTHQUAKE and FISSURE\n"
	.string "have no effect on bird POKéMON.$"

Kanto_Route15_Text_8436A57::
	.string "Oh, you’re a little cutie!\n"
	.string "So like a darling POKéMON!$"

Kanto_Route15_Text_8436A8D::
	.string "You looked so cute, too!$"

Kanto_Route15_Text_8436AA6::
	.string "I forgive you.\n"
	.string "I can take it.\l"
	.string "I’m a big girl now.$"

Kanto_Route15_Text_8436AD8::
	.string "I raise POKéMON for protection\n"
	.string "because I live alone.$"

Kanto_Route15_Text_8436B0D::
	.string "POKéMON isn’t about winning or\n"
	.string "losing for me.$"

Kanto_Route15_Text_8436B3B::
	.string "I love having my POKéMON greet me\n"
	.string "when I get home.\p"
	.string "It’s so reassuring.$"

Kanto_Route15_Text_8436B82::
	.string "Hey, kid! C’mon!\n"
	.string "I just got these off some loser!$"

Kanto_Route15_Text_8436BB4::
	.string "Why not?$"

Kanto_Route15_Text_8436BBD::
	.string "Life’s too short.\n"
	.string "It’s cool to live as an outlaw.\l"
	.string "TEAM ROCKET RULES!$"

Kanto_Route15_Text_8436C02::
	.string "Fork over all your cash when you\n"
	.string "lose to me, kid!$"

Kanto_Route15_Text_8436C34::
	.string "That can’t be true!$"

Kanto_Route15_Text_8436C48::
	.string "I was just joking about the money.\n"
	.string "Don’t take me all serious.$"

Kanto_Route15_Text_8436C86::
	.string "What’s cool and happening?\n"
	.string "Trading POKéMON!$"

Kanto_Route15_Text_8436CB2::
	.string "I said trade!$"

Kanto_Route15_Text_8436CC0::
	.string "I trade POKéMON with my friends\n"
	.string "all the time.$"

Kanto_Route15_Text_8436CEE::
	.string "Want to play with my POKéMON?$"

Kanto_Route15_Text_8436D0C::
	.string "I was too impatient!$"

Kanto_Route15_Text_8436D21::
	.string "I’ll go train with weaker people.$"

Kanto_Route15_Text_8436D43::
	.string "ROUTE 15\n"
	.string "West to FUCHSIA CITY$"

Kanto_Route15_Text_8436D61::
	.string "MYA: You’re perfect.\n"
	.string "Help me train my little brother?$"

Kanto_Route15_Text_8436D97::
	.string "MYA: RON, you have to focus!\n"
	.string "Concentrate on what you’re doing!$"

Kanto_Route15_Text_8436DD6::
	.string "MYA: Okay, we’ll turn it up.\n"
	.string "I’ll add to our training menu!$"

Kanto_Route15_Text_8436E12::
	.string "MYA: Do you want to challenge us?\n"
	.string "You’ll need two POKéMON, though.$"

Kanto_Route15_Text_8436E55::
	.string "RON: My sister gets scary when we\n"
	.string "lose.$"

Kanto_Route15_Text_8436E7D::
	.string "RON: Oh, no, no…\n"
	.string "Sis, I’m sorry!$"

Kanto_Route15_Text_8436E9E::
	.string "RON: Oh, bleah…\n"
	.string "I wish I had a nice sister…$"

Kanto_Route15_Text_8436ECA::
	.string "RON: Did you want to battle with\n"
	.string "my sister and me?\p"
	.string "You need two POKéMON, then.$"

Kanto_Route15_Text_846FAF6::
	.string "I’m going to keep battling with the\n"
	.string "POKéMON I got in trades.$"

Kanto_Route15_Text_846FB33::
	.string "You look gentle, so I think I can\n"
	.string "beat you.\p"
	.string "I’ll give it one more go!$"

Kanto_Route15_Text_846FB79::
	.string "When I whistle, bird POKéMON\n"
	.string "gather around.\p"
	.string "They’re amazingly cute!$"

Kanto_Route15_Text_846FBBD::
	.string "Hmm? My birds are shivering!\n"
	.string "Wait, aren’t you that TRAINER…$"

Kanto_Route15_Text_846FBF9::
	.string "Oh, you’re a little cutie!\n"
	.string "So like a darling POKéMON!\l"
	.string "I remember you now!$"

Kanto_Route15_Text_846FC43::
	.string "I raise POKéMON for protection\n"
	.string "because I live alone.\p"
	.string "That hasn’t changed.$"

Kanto_Route15_Text_846FC8D::
	.string "Hey, kid! C’mon!\n"
	.string "I got these off some loser!$"

Kanto_Route15_Text_846FCBA::
	.string "Fork over all your cash when you\n"
	.string "lose to me, kid!$"

Kanto_Route15_Text_846FCEC::
	.string "What’s cool and happening?\n"
	.string "Trading POKéMON, of course.$"

Kanto_Route15_Text_846FD23::
	.string "Want to play with my POKéMON\n"
	.string "again?$"

Kanto_Route15_Text_846FD47::
	.string "MYA: Hi, we keep meeting,\n"
	.string "don’t we?\p"
	.string "Help me train my little brother\n"
	.string "again?$"

Kanto_Route15_Text_846FD92::
	.string "RON: My sister’s gotten more\n"
	.string "powerful…$"
