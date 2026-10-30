Kanto_SaffronCity_Dojo_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SaffronCity_Dojo trigger at 5,5 (VAR_QZ_0x5129 == 0)
Kanto_SaffronCity_Dojo_EventScript_841EC65::
	lockall
	setvar VAR_QZ_0x5129, 1
	applymovement 5, Quetzal_Movement_8457AAA
	waitmovement 0
	releaseall
	end

@ Kanto_SaffronCity_Dojo trigger at 7,5 (VAR_QZ_0x5129 == 0)
Kanto_SaffronCity_Dojo_EventScript_841EC77::
	lockall
	setvar VAR_QZ_0x5129, 1
	applymovement 5, Quetzal_Movement_8457AAE
	waitmovement 0
	releaseall
	end

@ Kanto_SaffronCity_Dojo object 6 (local id 6, gfx 0x3F0E) at 5,3
Kanto_SaffronCity_Dojo_EventScript_841EC89::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x214D, Kanto_SaffronCity_Dojo_EventScript_841ECCA
	call Quetzal_EventScript_8361A4D
	showmonpic SPECIES_HITMONLEE, 0xA, 3
	setvar VAR_TEMP_1, 0x6A
	applymovement 5, Quetzal_Movement_8457AAC
	waitmovement 0
	textcolor 0
	msgbox Kanto_SaffronCity_Dojo_Text_844B3A9, MSGBOX_YESNO
	call Quetzal_EventScript_8456AB7
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_Dojo_EventScript_841ED15
	hidemonpic
	release
	end

Kanto_SaffronCity_Dojo_EventScript_841ECCA::
	msgbox Kanto_SaffronCity_Dojo_Text_844B43C, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_Dojo object 7 (local id 7, gfx 0x3F0E) at 7,3
Kanto_SaffronCity_Dojo_EventScript_841ECD4::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x214D, Kanto_SaffronCity_Dojo_EventScript_841ECCA
	call Quetzal_EventScript_8361A4D
	showmonpic SPECIES_HITMONCHAN, 0xA, 3
	setvar VAR_TEMP_1, 0x6B
	applymovement 5, Quetzal_Movement_8457AAC
	waitmovement 0
	textcolor 0
	msgbox Kanto_SaffronCity_Dojo_Text_844B3F9, MSGBOX_YESNO
	call Quetzal_EventScript_8456AB7
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_Dojo_EventScript_841ED15
	hidemonpic
	release
	end

Kanto_SaffronCity_Dojo_EventScript_841ED15::
	hidemonpic
	qz_givemon_ex VAR_TEMP_1, 0x19, ITEM_NONE, 1, 0, 2
	goto_if_eq VAR_RESULT, 0, Kanto_SaffronCity_Dojo_EventScript_841ED48
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_Dojo_EventScript_841ED7F
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8459563
	release
	end

Kanto_SaffronCity_Dojo_EventScript_841ED48::
	removeobject VAR_LAST_TALKED
	bufferspeciesname 0, VAR_TEMP_1
	playfanfare MUS_LEVEL_UP
	message Kanto_SaffronCity_Dojo_Text_844B3CE
	waitmessage
	waitfanfare
	setflag FLAG_QZ_0x214D
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SaffronCity_Dojo_EventScript_841EDBC
	call Quetzal_EventScript_84590EA
	call Quetzal_EventScript_84579AE
	goto Kanto_SaffronCity_Dojo_EventScript_841EDBC

Kanto_SaffronCity_Dojo_EventScript_841ED7F::
	removeobject VAR_LAST_TALKED
	bufferspeciesname 0, VAR_TEMP_1
	playfanfare MUS_LEVEL_UP
	message Kanto_SaffronCity_Dojo_Text_844B3CE
	waitmessage
	waitfanfare
	setflag FLAG_QZ_0x214D
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SaffronCity_Dojo_EventScript_841EDB1
	call Quetzal_EventScript_84590F6
	goto Kanto_SaffronCity_Dojo_EventScript_841EDB1

Kanto_SaffronCity_Dojo_EventScript_841EDB1::
	call Quetzal_EventScript_84590FF
	goto Kanto_SaffronCity_Dojo_EventScript_841EDBC

Kanto_SaffronCity_Dojo_EventScript_841EDBC::
	release
	end

@ Kanto_SaffronCity_Dojo sign / bg event at 3,13
@ Kanto_SaffronCity_Dojo sign / bg event at 9,13
Kanto_SaffronCity_Dojo_EventScript_841EDBE::
	msgbox Kanto_SaffronCity_Dojo_Text_844B489, MSGBOX_SIGN
	end

@ Kanto_SaffronCity_Dojo sign / bg event at 5,2
Kanto_SaffronCity_Dojo_EventScript_841EDC7::
	msgbox Kanto_SaffronCity_Dojo_Text_844B453, MSGBOX_SIGN
	end

@ Kanto_SaffronCity_Dojo sign / bg event at 7,2
Kanto_SaffronCity_Dojo_EventScript_841EDD0::
	msgbox Kanto_SaffronCity_Dojo_Text_844B46A, MSGBOX_SIGN
	end

@ Kanto_SaffronCity_Dojo object 1 (local id 1, gfx 0x3EEB) at 11,10
Kanto_SaffronCity_Dojo_EventScript_841EDD9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8513, 0, Kanto_SaffronCity_Dojo_Text_844B33A, Kanto_SaffronCity_Dojo_Text_844B36A
	msgbox Kanto_SaffronCity_Dojo_Text_844B37A, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Dojo object 2 (local id 2, gfx 0x3EEB) at 1,10
Kanto_SaffronCity_Dojo_EventScript_841EDF0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8511, 0, Kanto_SaffronCity_Dojo_Text_844B23B, Kanto_SaffronCity_Dojo_Text_844B258
	msgbox Kanto_SaffronCity_Dojo_Text_844B26A, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Dojo object 3 (local id 3, gfx 0x3EEB) at 11,8
Kanto_SaffronCity_Dojo_EventScript_841EE07::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8512, 0, Kanto_SaffronCity_Dojo_Text_844B29C, Kanto_SaffronCity_Dojo_Text_844B2F1
	msgbox Kanto_SaffronCity_Dojo_Text_844B307, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Dojo object 4 (local id 4, gfx 0x3EEB) at 1,8
Kanto_SaffronCity_Dojo_EventScript_841EE1E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8510, 0, Kanto_SaffronCity_Dojo_Text_844B1D0, Kanto_SaffronCity_Dojo_Text_844B1ED
	msgbox Kanto_SaffronCity_Dojo_Text_844B1F8, MSGBOX_AUTOCLOSE
	end

@ Kanto_SaffronCity_Dojo object 5 (local id 5, gfx 0x3EEB) at 6,5
Kanto_SaffronCity_Dojo_EventScript_841EE35::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8509, 0, Kanto_SaffronCity_Dojo_Text_844B087, Kanto_SaffronCity_Dojo_Text_844B0EE, Kanto_SaffronCity_Dojo_EventScript_841EE63
	goto_if_set FLAG_QZ_0x214D, Kanto_SaffronCity_Dojo_EventScript_841EE59
	msgbox Kanto_SaffronCity_Dojo_Text_844B102, MSGBOX_AUTOCLOSE
	end

Kanto_SaffronCity_Dojo_EventScript_841EE59::
	msgbox Kanto_SaffronCity_Dojo_Text_844B1A7, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_Dojo_EventScript_841EE63::
	setvar VAR_QZ_0x5129, 1
	release
	end

Kanto_SaffronCity_Dojo_Text_844B087::
	.string "Grunt!\p"
	.string "I am the KARATE MASTER!\n"
	.string "I am the LEADER here!\p"
	.string "You wish to challenge us?\n"
	.string "Expect no mercy!\p"
	.string "Fwaaa!$"

Kanto_SaffronCity_Dojo_Text_844B0EE::
	.string "Hwa!\n"
	.string "Arrgh! Beaten!$"

Kanto_SaffronCity_Dojo_Text_844B102::
	.string "Indeed, I have lost!\p"
	.string "But, I beseech you, do not take\n"
	.string "our emblem as your trophy!\p"
	.string "In return, I will give you a prized\n"
	.string "FIGHTING POKéMON!\p"
	.string "Choose whichever one you like!$"

Kanto_SaffronCity_Dojo_Text_844B1A7::
	.string "Grunt!\n"
	.string "Stay and train at karate with us!$"

Kanto_SaffronCity_Dojo_Text_844B1D0::
	.string "Hoargh!\n"
	.string "Take your shoes off!$"

Kanto_SaffronCity_Dojo_Text_844B1ED::
	.string "I give up!$"

Kanto_SaffronCity_Dojo_Text_844B1F8::
	.string "You wait ’til you see our MASTER!\n"
	.string "I’m a small fry compared to him!$"

Kanto_SaffronCity_Dojo_Text_844B23B::
	.string "I hear you’re good!\n"
	.string "Show me!$"

Kanto_SaffronCity_Dojo_Text_844B258::
	.string "Judge!\n"
	.string "One point!$"

Kanto_SaffronCity_Dojo_Text_844B26A::
	.string "Our MASTER is a pro fighter.\n"
	.string "Be prepared to lose!$"

Kanto_SaffronCity_Dojo_Text_844B29C::
	.string "Hiyah!\n"
	.string "I have no fear for anything tough!\p"
	.string "I shatter boulders for training\n"
	.string "every day!$"

Kanto_SaffronCity_Dojo_Text_844B2F1::
	.string "Yow!\n"
	.string "Stubbed fingers!$"

Kanto_SaffronCity_Dojo_Text_844B307::
	.string "The only thing that frightens us is\n"
	.string "psychic power!$"

Kanto_SaffronCity_Dojo_Text_844B33A::
	.string "Hwaah!\p"
	.string "You’re trespassing in our\n"
	.string "FIGHTING DOJO!$"

Kanto_SaffronCity_Dojo_Text_844B36A::
	.string "Oof!\n"
	.string "I give up!$"

Kanto_SaffronCity_Dojo_Text_844B37A::
	.string "The prime fighters across the land\n"
	.string "train here.$"

Kanto_SaffronCity_Dojo_Text_844B3A9::
	.string "You want the hard-kicking\n"
	.string "HITMONLEE?$"

Kanto_SaffronCity_Dojo_Text_844B3CE::
	.string "{PLAYER} received the {STR_VAR_1}\n"
	.string "from the KARATE MASTER.$"

Kanto_SaffronCity_Dojo_Text_844B3F9::
	.string "You want the piston-punching\n"
	.string "HITMONCHAN?$"

Kanto_SaffronCity_Dojo_Text_844B43C::
	.string "Better not get greedy…$"

Kanto_SaffronCity_Dojo_Text_844B453::
	.string "Enemies on every side!$"

Kanto_SaffronCity_Dojo_Text_844B46A::
	.string "What goes around comes around.$"

Kanto_SaffronCity_Dojo_Text_844B489::
	.string "FIGHTING DOJO$"
