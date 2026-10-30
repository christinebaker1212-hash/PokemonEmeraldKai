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
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_8449FC0::
	.string "EEVEE can evolve into one of three\n"
	.string "kinds of POKéMON.$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_8449FF5::
	.string "There’s an e-mail message.\p"
	.string "… … …\p"
	.string "There are three legendary bird\n"
	.string "POKéMON.\p"
	.string "They are ARTICUNO, ZAPDOS, and\n"
	.string "MOLTRES.\p"
	.string "Their whereabouts are unknown.\p"
	.string "We plan to explore the cavern\n"
	.string "close to CERULEAN.\p"
	.string "From: POKéMON RESEARCH TEAM…$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_Text_844A0D3::
	.string "An amber pipe!$"
