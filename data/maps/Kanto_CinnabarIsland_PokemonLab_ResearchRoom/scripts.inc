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
	@ Moemon Kai: FireRed's METRONOME tutor (tools/quetzal_port/fix_move_tutors.py), on Kai's move tutors
	lock
	faceplayer
	goto_if_set 0x2220, Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Taught
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Teach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Declined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Declined
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_WhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_METRONOME
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Declined
	setflag 0x2220
	goto Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Taught
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Declined:
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_DeclinedText, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Taught:
	msgbox Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_TaughtText, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_Teach:
	.string "Tch-tch-tch!\n"
	.string "I'll teach you a nifty move.\p"
	.string "Teach it to a MOéMON, and watch\n"
	.string "the fun unfold!\p"
	.string "It's a move called METRONOME.\n"
	.string "Does it strike your fancy?$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_DeclinedText:
	.string "I tell you, it's a lot of fun!$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_WhichMon:
	.string "Okay!\n"
	.string "Which MOéMON should I teach?$"

Kanto_CinnabarIsland_PokemonLab_ResearchRoom_EventScript_8471CC0_TaughtText:
	.string "Tch-tch-tch!\n"
	.string "That's the sound of a METRONOME.\p"
	.string "It tweaks your MOéMON's brain into\n"
	.string "using moves it doesn't even know.$"

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
