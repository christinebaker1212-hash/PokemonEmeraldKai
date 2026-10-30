Kanto_CinnabarIsland_PokemonLab_Lounge_MapScripts::
	.byte 0

@ Kanto_CinnabarIsland_PokemonLab_Lounge object 1 (local id 1, gfx 0x3F43) at 5,3
Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E599::
	msgbox Kanto_CinnabarIsland_PokemonLab_Lounge_Text_8449F5E, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland_PokemonLab_Lounge object 2 (local id 2, gfx 0x3F2D) at 4,6
Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E5A2::
	lock
	faceplayer
	setvar VAR_0x8008, 0xD
	call Quetzal_EventScript_8459170
	goto_if_set FLAG_QZ_0x2128, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E61F
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E607
	msgbox Quetzal_Text_8455CFC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E607
	call Quetzal_EventScript_8459180
	goto_if_ge VAR_0x8004, 6, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E607
	call Quetzal_EventScript_845918C
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E611
	call Quetzal_EventScript_845919C
	msgbox Quetzal_Text_8455D9B, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2128
	release
	end

Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E607::
	msgbox Quetzal_Text_8455D4C, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E611::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455D68, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E61F::
	msgbox Quetzal_Text_8455DA3, MSGBOX_DEFAULT
	release
	end

@ Kanto_CinnabarIsland_PokemonLab_Lounge object 3 (local id 3, gfx 0x3F58) at 10,5
Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E629::
	lock
	faceplayer
	setvar VAR_0x8008, 0xE
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	copyvar VAR_0x8009, VAR_RESULT
	goto_if_set FLAG_QZ_0x212E, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6CA
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6B2
	msgbox Quetzal_Text_8455DD4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6B2
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_ge VAR_0x8004, 6, Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6B2
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_0x8009
	goto_if_ne Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6BC
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	msgbox Quetzal_Text_8455E48, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x212E
	release
	end

Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6B2::
	msgbox Quetzal_Text_8455E06, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6BC::
	bufferspeciesname 0, VAR_0x8009
	msgbox Quetzal_Text_8455E16, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_Lounge_EventScript_841E6CA::
	msgbox Quetzal_Text_8455E5E, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_Lounge_Text_8449F5E::
	.string "I found this most remarkable fossil\n"
	.string "inside MT. MOON.\p"
	.string "I think it’s of a rare, prehistoric\n"
	.string "POKéMON.$"
