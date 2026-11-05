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
@ Moemon Kai: FireRed's fossil scientist (pokefirered CinnabarIsland_PokemonLab_ExperimentRoom; audit case 61): HELIX
@ FOSSIL, DOME FOSSIL and OLD AMBER only, the POKéMON back after a walk out of the building (the lab's entrance sets
@ VAR_QZ_0x510C 1 -> 2), at Lv 5. VAR_QZ_0x510D is the fossil left with him (1 HELIX, 2 DOME, 3 AMBER).
Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_841E77E::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x510C, 2, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveRevivedMon
	goto_if_eq VAR_QZ_0x510C, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilStillReviving
	checkitem ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilHelix
	checkitem ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilDome
	checkitem ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, TRUE, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilAmber
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A22D, MSGBOX_DEFAULT
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DontShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DontShowFossil::
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2BF, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilHelix::
	message Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A22D
	waitmessage
	checkitem ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, TRUE, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilHelixAmber
	multichoice 0, 0, MULTI_FR_HELIX, FALSE
	switch VAR_RESULT
	case 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowHelixFossil
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DontShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilDome::
	message Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A22D
	waitmessage
	checkitem ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, TRUE, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilDomeAmber
	multichoice 0, 0, MULTI_FR_DOME, FALSE
	switch VAR_RESULT
	case 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowDomeFossil
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DontShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilAmber::
	message Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A22D
	waitmessage
	multichoice 0, 0, MULTI_FR_AMBER, FALSE
	switch VAR_RESULT
	case 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowOldAmber
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DontShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilHelixAmber::
	multichoice 0, 0, MULTI_FR_HELIX_AMBER, FALSE
	switch VAR_RESULT
	case 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowHelixFossil
	case 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowOldAmber
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DontShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ChooseFossilDomeAmber::
	multichoice 0, 0, MULTI_FR_DOME_AMBER, FALSE
	switch VAR_RESULT
	case 0, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowDomeFossil
	case 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowOldAmber
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DontShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowHelixFossil::
	bufferspeciesname STR_VAR_1, SPECIES_OMANYTE
	bufferitemname STR_VAR_2, ITEM_HELIX_FOSSIL
	setvar VAR_0x8004, ITEM_HELIX_FOSSIL
	setvar VAR_0x8005, 1
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowDomeFossil::
	bufferspeciesname STR_VAR_1, SPECIES_KABUTO
	bufferitemname STR_VAR_2, ITEM_DOME_FOSSIL
	setvar VAR_0x8004, ITEM_DOME_FOSSIL
	setvar VAR_0x8005, 2
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowFossil

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowOldAmber::
	bufferspeciesname STR_VAR_1, SPECIES_AERODACTYL
	bufferitemname STR_VAR_2, ITEM_OLD_AMBER
	setvar VAR_0x8004, ITEM_OLD_AMBER
	setvar VAR_0x8005, 3
Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_ShowFossil::
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_ThatFossilIsOfMon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DeclineReviveFossil
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A3E1, MSGBOX_DEFAULT
	removeitem VAR_0x8004
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_TakesTimeGoForWalk, MSGBOX_DEFAULT
	setvar VAR_QZ_0x510C, 1
	copyvar VAR_QZ_0x510D, VAR_0x8005
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_DeclineReviveFossil::
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_YouComeAgain, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilStillReviving::
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_TakesTimeGoForWalk, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_GiveRevivedMon::
	setvar VAR_TEMP_1, SPECIES_OMANYTE
	call_if_eq VAR_QZ_0x510D, 2, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_SetKabuto
	call_if_eq VAR_QZ_0x510D, 3, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_SetAerodactyl
	bufferspeciesname STR_VAR_1, VAR_TEMP_1
	msgbox Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2FA, MSGBOX_DEFAULT
	givemon VAR_TEMP_1, 5, ITEM_NONE
	goto_if_eq VAR_RESULT, 2, Common_EventScript_NoMoreRoomForPokemon
	setvar VAR_QZ_0x510C, 0
	setvar VAR_QZ_0x510D, 0
	playfanfare MUS_LEVEL_UP
	message Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A322
	waitmessage
	waitfanfare
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_NicknameMonPC
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_EndGiveMon
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_EndGiveMon

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_NicknameMonPC::
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_MonSentToPC
	call Common_EventScript_NameReceivedBoxMon
Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_MonSentToPC::
	call Common_EventScript_TransferredToPC
Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_EndGiveMon::
	release
	end

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_SetKabuto::
	setvar VAR_TEMP_1, SPECIES_KABUTO
	return

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_SetAerodactyl::
	setvar VAR_TEMP_1, SPECIES_AERODACTYL
	return

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A22D::
	.string "Hiya!\p"
	.string "I am important doctor.\n"
	.string "Yes, very much so, indeed.\p"
	.string "Here, I study rare MOéMON fossils.\n"
	.string "All the time, I study.\p"
	.string "You!\n"
	.string "Have you a fossil for me?$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2BF::
	.string "No!\n"
	.string "Is too bad!$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A2FA::
	.string "You late.\n"
	.string "Where were you?\p"
	.string "Your fossil is back to life!\n"
	.string "It was {STR_VAR_1} like I think!$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A322::
	.string "{PLAYER} received the {STR_VAR_1}\n"
	.string "from the doctor.$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_844A3E1::
	.string "So!\n"
	.string "You hurry and give me that!\p"
	.string "{FONT_NORMAL}{PLAYER} handed over the\n"
	.string "{STR_VAR_2} to the weird doctor.$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_ThatFossilIsOfMon::
	.string "Oh!\n"
	.string "That is {STR_VAR_2}, it is!\p"
	.string "It is fossil of {STR_VAR_1},\n"
	.string "a MOéMON that is already extinct!\p"
	.string "My Resurrection Machine will make\n"
	.string "that MOéMON live again!$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_TakesTimeGoForWalk::
	.string "I take a little time!\n"
	.string "You go for walk a little while!$"

Kanto_CinnabarIsland_PokemonLab_ExperimentRoom_Text_YouComeAgain::
	.string "Aiyah!\n"
	.string "You come again!$"
