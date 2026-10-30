Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_MapScripts::
	.byte 0

@ Kanto_CinnabarIsland_PokemonLab_ExperimentRoom object 1 (local id 1, gfx 0x3EEE) at 11,8
Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E6F7::
	lock
	faceplayer
	setvar VAR_0x8008, 0xF
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x212D, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E774
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E75C
	msgbox Quetzal_Text_8455C79, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E75C
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E75C
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E766
	call Quetzal_EventScript_845919C
	msgbox Quetzal_Text_8455CD8, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x212D
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E75C::
	msgbox Quetzal_Text_8455CB4, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E766::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455CC4, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E774::
	msgbox Quetzal_Text_8455CE5, MSGBOX_DEFAULT
	release
	end

@ Kanto_CinnabarIsland_PokemonLab_ExperimentRoom object 2 (local id 2, gfx 0x3F43) at 12,3
Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E77E::
	lock
	faceplayer
	call Quetzal_EventScript_8361A4D
	goto_if_ne VAR_QZ_0x510C, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E8CD

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E790::
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A22D, MSGBOX_DEFAULT
	special QzBagHasFossil
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E7A8
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E7A8::
	closemessage
	playse SE_PIN
	applymovement VAR_LAST_TALKED, Quetzal_Movement_83607F9
	waitmovement 0
	applymovement VAR_LAST_TALKED, Quetzal_Movement_83607FB
	waitmovement 0
	msgbox Quetzal_Text_83F760D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E960
	special QzChooseItemFromBagForFossil
	waitstate
	goto_if_eq VAR_ITEM_ID, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E960
	special QzIsSelectedItemFossil
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E960
	special QzIsSelectedItemFossilHalf
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E804
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E850

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E804::
	msgbox Quetzal_Text_82F8280, MSGBOX_DEFAULT
	special QzChooseItemFromBagForFossil
	waitstate
	goto_if_eq VAR_ITEM_ID, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E960
	special QzIsSelectedItemFossil
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E842
	special QzAreFossilHalvesCompatible
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E83D
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E86F

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E83D::
	copyvar VAR_0x800B, VAR_ITEM_ID

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E842::
	msgbox Quetzal_Text_83F721C, MSGBOX_DEFAULT
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E804

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E850::
	special QzGiveRevivedFossilMon
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E88E
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E88E
	goto Quetzal_EventScript_8361BCF

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E86F::
	special QzGiveRevivedFossilMon
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E8A5
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E8A5
	goto Quetzal_EventScript_8361BCF

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E88E::
	bufferitemname 1, VAR_ITEM_ID
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A3E1, MSGBOX_DEFAULT
	removeitem VAR_ITEM_ID, 1
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E93E

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E8A5::
	bufferitemname 1, VAR_ITEM_ID
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A3E1, MSGBOX_DEFAULT
	removeitem VAR_ITEM_ID, 1
	bufferitemname 1, VAR_0x800B
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A3E1, MSGBOX_DEFAULT
	removeitem VAR_0x800B, 1
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E93E

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E8CD::
	goto_if_eq VAR_QZ_0x510D, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E8FE
	goto_if_eq VAR_QZ_0x510D, 2, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E909
	goto_if_eq VAR_QZ_0x510D, 3, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E914
	setvar VAR_QZ_0x510C, 0
	setvar VAR_QZ_0x510D, 0
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E790

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E8FE::
	setvar VAR_ITEM_ID, 0xA5
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E91F

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E909::
	setvar VAR_ITEM_ID, 0xA6
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E91F

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E914::
	setvar VAR_ITEM_ID, 0xA7
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E91F

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E91F::
	special QzGiveRevivedFossilMon
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E93E
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E93E
	goto Quetzal_EventScript_8361BCF

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E93E::
	setvar VAR_QZ_0x510C, 0
	setvar VAR_QZ_0x510D, 0
	bufferspeciesname 0, VAR_0x800A
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2FA, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A322
	waitmessage
	waitfanfare
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E960::
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2BF, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A22D::
	.string "Hiya!\p"
	.string "I am important doctor.\n"
	.string "Yes, very much so, indeed.\p"
	.string "Here, I study rare POKéMON fossils.\n"
	.string "All the time, I study.\p"
	.string "You!\n"
	.string "Have you a fossil for me?$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2BF::
	.string "No!\n"
	.string "Is too bad!$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2FA::
	.string "Your fossil is back to life!\p"
	.string "It was {STR_VAR_1}!$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A322::
	.string "{PLAYER} received the {STR_VAR_1}\n"
	.string "from the doctor.$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A3E1::
	.string "{PLAYER} handed over the\n"
	.string "{STR_VAR_2} to the doctor.$"
