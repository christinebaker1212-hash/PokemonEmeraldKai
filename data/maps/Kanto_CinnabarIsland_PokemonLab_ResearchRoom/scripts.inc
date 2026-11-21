Kanto_CinnabarIsland_PokemonLab_ResearchRoom_MapScripts::
	.byte 0

@ Kanto_CinnabarIsland_PokemonLab_ResearchRoom object 1 (local id 1, gfx 0x3F4B) at 5,4
Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_841E6D5::
	goto Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0

@ Kanto_CinnabarIsland_PokemonLab_ResearchRoom object 2 (local id 2, gfx 0x3F43) at 10,3
Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_841E6DB::
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_8449FC0, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland_PokemonLab_ResearchRoom sign / bg event at 3,1
Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_841E6E4::
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_8449FF5, MSGBOX_SIGN
	end

@ Kanto_CinnabarIsland_PokemonLab_ResearchRoom sign / bg event at 6,1
Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_841E6ED::
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_844A0D3, MSGBOX_SIGN
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0::
	@ Moemon Kai: FireRed's METRONOME tutor (tools/quetzal_port/fix_move_tutors.py),
	@ now gives its TM (tools/quetzal_port/tutors_to_tms.py)
	lock
	faceplayer
	goto_if_set FLAG_KANTO_GOT_TM_METRONOME, Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Taught
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Teach, MSGBOX_DEFAULT
	giveitem ITEM_TM125
	goto_if_eq VAR_RESULT, FALSE, Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Declined
	setflag FLAG_KANTO_GOT_TM_METRONOME
	goto Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Taught
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Declined::
	release
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Taught:
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_TaughtText, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Teach:
	.string "Tch-tch-tch! I made a cool TM!\p"
	.string "It can cause all kinds of fun!$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_TaughtText:
	.string "Tch-tch-tch! That’s the sound\n"
	.string "of a METRONOME!\p"
	.string "It tweaks your MOéMON’s brain\n"
	.string "into using moves it doesn’t\l"
	.string "know!$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_8449FC0::
	.string "EEVEE can evolve into one of three\n"
	.string "kinds of MOéMON.$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_8449FF5::
	.string "There’s an e-mail message.\p"
	.string "… … …\p"
	.string "There are three legendary bird\n"
	.string "MOéMON.\p"
	.string "They are ARTICUNO, ZAPDOS, and\n"
	.string "MOLTRES.\p"
	.string "Their whereabouts are unknown.\p"
	.string "We plan to explore the cavern\n"
	.string "close to CERULEAN.\p"
	.string "From: MOéMON RESEARCH TEAM…$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_844A0D3::
	.string "An amber pipe!$"
