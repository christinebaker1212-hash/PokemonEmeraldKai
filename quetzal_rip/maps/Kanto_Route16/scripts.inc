Kanto_Route16_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_Route16_EventScript_8417DDB
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route16_EventScript_8417DE9
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_Route16_OnWarp_8417DFE
	.byte 0

Kanto_Route16_OnWarp_8417DFE:
	map_script_2 VAR_QZ_0x5124, 1, Kanto_Route16_EventScript_8417E08
	.2byte 0

Kanto_Route16_EventScript_8417DDB::
	call_if_set FLAG_QZ_0x2215, Kanto_Route16_EventScript_8417DE5
	end

Kanto_Route16_EventScript_8417DE5::
	removeobject VAR_LAST_TALKED
	return

Kanto_Route16_EventScript_8417DE9::
	call Quetzal_EventScript_840FBB5
	call_if_eq VAR_QZ_0x5124, 1, Kanto_Route16_EventScript_8417DFA
	end

Kanto_Route16_EventScript_8417DFA::
	setflag FLAG_QZ_0x2210
	return

Kanto_Route16_EventScript_8417E08::
	special QzForcePlayerOntoBike_728
	end

@ Kanto_Route16 object 10 (local id 10, gfx 0x3F48) at 31,13
Kanto_Route16_EventScript_8417E0C::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x215D, Kanto_Route16_EventScript_8417E95
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop_737
	msgbox Quetzal_Text_8435693, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_Route16_EventScript_8417E89
	call Quetzal_EventScript_8459531
	setwildbattle SPECIES_SNORLAX, 0x1E, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	waitse
	playmoncry SPECIES_SNORLAX, 2
	delay 0x28
	waitmoncry
	setflag FLAG_QZ_0x21B0
	setflag FLAG_QZ_0x2215
	dowildbattle
	clearflag FLAG_QZ_0x2215
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_8417E8B
	goto_if_eq VAR_RESULT, 4, Kanto_Route16_EventScript_8417E8B
	goto_if_eq VAR_RESULT, 5, Kanto_Route16_EventScript_8417E8B
	release
	end

Kanto_Route16_EventScript_8417E89::
	release
	end

Kanto_Route16_EventScript_8417E8B::
	msgbox Quetzal_Text_843564A, MSGBOX_DEFAULT
	release
	end

Kanto_Route16_EventScript_8417E95::
	msgbox Kanto_Route16_Text_8437209, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route16 sign / bg event at 33,11
Kanto_Route16_EventScript_8417E9F::
	msgbox Kanto_Route16_Text_8437246, MSGBOX_SIGN
	end

@ Kanto_Route16 sign / bg event at 6,17
Kanto_Route16_EventScript_8417EA8::
	msgbox Kanto_Route16_Text_8437263, MSGBOX_SIGN
	end

@ Kanto_Route16 object 1 (local id 1, gfx 0x3EE9) at 16,12
Kanto_Route16_EventScript_845B2BC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8391, 0, Kanto_Route16_Text_8436F19, Kanto_Route16_Text_8436F2B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B2E3
	msgbox Kanto_Route16_Text_8436F41, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B2E3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8391, 0, Kanto_Route16_Text_846FDB9, Kanto_Route16_Text_8436F2B
	msgbox Kanto_Route16_Text_8436F41, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route16 object 5 (local id 5, gfx 0x3EE9) at 10,14
Kanto_Route16_EventScript_845B2FA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8393, 0, Kanto_Route16_Text_8437005, Kanto_Route16_Text_843701E
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B321
	msgbox Kanto_Route16_Text_8437026, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B321::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8393, 0, Kanto_Route16_Text_846FE21, Kanto_Route16_Text_843701E
	msgbox Kanto_Route16_Text_8437026, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route16 object 4 (local id 4, gfx 0x3EE9) at 8,12
Kanto_Route16_EventScript_845B338::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8394, 0, Kanto_Route16_Text_8437152, Kanto_Route16_Text_8437180
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B35F
	msgbox Kanto_Route16_Text_8437193, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B35F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8394, 0, Kanto_Route16_Text_846FE91, Kanto_Route16_Text_8437180
	msgbox Kanto_Route16_Text_8437193, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route16 object 2 (local id 2, gfx 0x3EE9) at 14,14
Kanto_Route16_EventScript_845B376::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8441, 0, Kanto_Route16_Text_8436F6E, Kanto_Route16_Text_8436F87
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B39D
	msgbox Kanto_Route16_Text_8436F91, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B39D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8441, 0, Kanto_Route16_Text_846FDCB, Kanto_Route16_Text_8436F87
	msgbox Kanto_Route16_Text_8436F91, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route16 object 3 (local id 3, gfx 0x3EE9) at 12,12
Kanto_Route16_EventScript_845B3B4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8442, 0, Kanto_Route16_Text_8436FB1, Kanto_Route16_Text_8436FD2
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B3DB
	msgbox Kanto_Route16_Text_8436FE2, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B3DB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8442, 0, Kanto_Route16_Text_846FDEA, Kanto_Route16_Text_8436FD2
	msgbox Kanto_Route16_Text_8436FE2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route16 object 6 (local id 6, gfx 0x3EE9) at 6,14
Kanto_Route16_EventScript_845B3F2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8443, 0, Kanto_Route16_Text_84370A5, Kanto_Route16_Text_84370D9
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B419
	msgbox Kanto_Route16_Text_84370E8, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B419::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8443, 0, Kanto_Route16_Text_846FE56, Kanto_Route16_Text_84370D9
	msgbox Kanto_Route16_Text_84370E8, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route16 object 9 (local id 9, gfx 0x3F1E) at 31,5
Kanto_Route16_EventScript_845B430::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8681, 0, Kanto_Route16_Text_8437288, Kanto_Route16_Text_84372C5, Kanto_Route16_Text_8437322
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B45B
	msgbox Kanto_Route16_Text_84372F2, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B45B::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8681, 0, Kanto_Route16_Text_846FEBF, Kanto_Route16_Text_84372C5, Kanto_Route16_Text_8437322
	msgbox Kanto_Route16_Text_84372F2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route16 object 8 (local id 8, gfx 0x3EE8) at 30,5
Kanto_Route16_EventScript_845B476::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8681, 0, Kanto_Route16_Text_8437362, Kanto_Route16_Text_8437397, Kanto_Route16_Text_84373EB
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route16_EventScript_845B4A1
	msgbox Kanto_Route16_Text_84373C4, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_EventScript_845B4A1::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8681, 0, Kanto_Route16_Text_846FEF9, Kanto_Route16_Text_8437397, Kanto_Route16_Text_84373EB
	msgbox Kanto_Route16_Text_84373C4, MSGBOX_AUTOCLOSE
	end

Kanto_Route16_Text_8436F19::
	.string "What do you want?$"

Kanto_Route16_Text_8436F2B::
	.string "Don’t you dare laugh!$"

Kanto_Route16_Text_8436F41::
	.string "We like just hanging here.\n"
	.string "What’s it to you?$"

Kanto_Route16_Text_8436F6E::
	.string "Nice BIKE!\n"
	.string "Hand it over!$"

Kanto_Route16_Text_8436F87::
	.string "Knockout!$"

Kanto_Route16_Text_8436F91::
	.string "Forget it, who needs your BIKE!$"

Kanto_Route16_Text_8436FB1::
	.string "Come out and play, little mouse!$"

Kanto_Route16_Text_8436FD2::
	.string "You little rat!$"

Kanto_Route16_Text_8436FE2::
	.string "I hate losing!\n"
	.string "Get out of my face!$"

Kanto_Route16_Text_8437005::
	.string "Hey, you just bumped me!$"

Kanto_Route16_Text_843701E::
	.string "Kaboom!$"

Kanto_Route16_Text_8437026::
	.string "We’ll always be hanging around here\n"
	.string "even if you don’t like it.\p"
	.string "You can detour to VERMILION from\n"
	.string "FUCHSIA going along the coast.$"

Kanto_Route16_Text_84370A5::
	.string "I’m feeling hungry and mean!\n"
	.string "I need a punching bag!$"

Kanto_Route16_Text_84370D9::
	.string "Bad, bad, bad!$"

Kanto_Route16_Text_84370E8::
	.string "If I’m going to have POKéMON, they\n"
	.string "may as well be ferocious.\p"
	.string "I’d use them to leave my enemies\n"
	.string "in tatters.$"

Kanto_Route16_Text_8437152::
	.string "Hey, there!\n"
	.string "Let’s have ourselves a good time!$"

Kanto_Route16_Text_8437180::
	.string "Don’t make me mad!$"

Kanto_Route16_Text_8437193::
	.string "I get my kicks by harassing people\n"
	.string "with my stinking POKéMON.\p"
	.string "They’re great for startling people.\n"
	.string "And, they bite, too.$"

Kanto_Route16_Text_8437209::
	.string "A POKéMON is sprawled out in\n"
	.string "a deep and comfortable slumber.$"

Kanto_Route16_Text_8437246::
	.string "CYCLING ROAD\n"
	.string "Enjoy the ride!$"

Kanto_Route16_Text_8437263::
	.string "ROUTE 16\n"
	.string "CELADON CITY - FUCHSIA CITY$"

Kanto_Route16_Text_8437288::
	.string "JED: Our love knows no bounds.\n"
	.string "We’re in love and we show it!$"

Kanto_Route16_Text_84372C5::
	.string "JED: Oh, no!\n"
	.string "My love has seen me as a loser!$"

Kanto_Route16_Text_84372F2::
	.string "JED: Listen, LEA.\n"
	.string "You need to focus less on me.$"

Kanto_Route16_Text_8437322::
	.string "JED: You have just one POKéMON?\n"
	.string "Is there no love in your heart?$"

Kanto_Route16_Text_8437362::
	.string "LEA: Sometimes, the intensity of\n"
	.string "our love scares me.$"

Kanto_Route16_Text_8437397::
	.string "LEA: Ohh! But JED looks cool\n"
	.string "even in a loss!$"

Kanto_Route16_Text_84373C4::
	.string "LEA: Ehehe, I’m sorry.\n"
	.string "JED is so cool.$"

Kanto_Route16_Text_84373EB::
	.string "LEA: Oh, you don’t have two\n"
	.string "POKéMON with you?\p"
	.string "Doesn’t it feel lonely for you or\n"
	.string "your POKéMON?$"

Kanto_Route16_Text_846FDB9::
	.string "What do you want?$"

Kanto_Route16_Text_846FDCB::
	.string "Nice BIKE!\n"
	.string "Hand it over quick!$"

Kanto_Route16_Text_846FDEA::
	.string "Come out and play, little mouse.\n"
	.string "I’ll treat you right!$"

Kanto_Route16_Text_846FE21::
	.string "Hey, wait a second!\n"
	.string "Don’t call me and then run away!$"

Kanto_Route16_Text_846FE56::
	.string "I’m feeling hungry and mean!\n"
	.string "I really need a punching bag!$"

Kanto_Route16_Text_846FE91::
	.string "Hey, there!\n"
	.string "We’ll have ourselves a good time!$"

Kanto_Route16_Text_846FEBF::
	.string "JED: Are you here to see our\n"
	.string "love, which knows no bounds?$"

Kanto_Route16_Text_846FEF9::
	.string "LEA: Sometimes, the intensity of\n"
	.string "our love scares me.$"
