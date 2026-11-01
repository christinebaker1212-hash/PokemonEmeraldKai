Sinnoh_SandgemTownPokemonResearchLab_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SandgemTownPokemonResearchLab_EventScript_Bg1::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Bg2::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Bg3::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Bg4::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Bg4, MSGBOX_SIGN
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Bg5::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Bg5, MSGBOX_SIGN
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Bg6::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Bg6, MSGBOX_SIGN
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DD8, Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person1_Given
	checkitem ITEM_POKE_RADAR
	goto_if_eq VAR_RESULT, TRUE, Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person1_Given
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_POKE_RADAR, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DD8
	release
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person1_Given::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person2::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person3::
	goto Sinnoh_EventScript_RadarAide
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person4::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SandgemTownPokemonResearchLab_EventScript_Person5::
	msgbox Sinnoh_SandgemTownPokemonResearchLab_Text_Person5, MSGBOX_NPC
	end

Sinnoh_SandgemTownPokemonResearchLab_Text_Bg1:
	.string "A thick book: “A TRAINER'S FIRST RULE:\n"
	.string "Take care of your MOéMON, and they'll\l"
	.string "take care of you.”$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Bg2:
	.string "A thick book: “A TRAINER'S SECOND\n"
	.string "RULE: Never stop being curious.”$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Bg3:
	.string "The shelves are packed with books\n"
	.string "about MOéMON from every region.$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Bg4:
	.string "A binder full of sketches of SINNOH's\n"
	.string "MOéMON, with notes in tiny\l"
	.string "handwriting.$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Bg5:
	.string "The PC shows a chart of how fast\n"
	.string "different MOéMON grow.\p"
	.string "The lines look like mountains.$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Bg6:
	.string "A fridge for samples. One shelf is just\n"
	.string "labeled “LUNCH. DO NOT STUDY.”$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Person1:
	.string "PROF. ROWAN: Welcome to my lab. Sit, if\n"
	.string "you can find a chair under the papers.\p"
	.string "SINNOH's POKéMON evolve in ways the\n"
	.string "world is only starting to understand.\l"
	.string "Travel far, and tell me what you see.$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Person1Gift:
	.string "Here, a POKé RADAR. It finds POKéMON\n"
	.string "hiding in the grass. Use it well.$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Person2:
	.string "We study how MOéMON evolve.\p"
	.string "Some evolve with a stone, some with a\n"
	.string "friend, and some just when they're\l"
	.string "good and ready.$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Person4:
	.string "We've been following TEAM GALACTIC's\n"
	.string "movements for the PROFESSOR.\p"
	.string "Whatever they want with the lakes, it\n"
	.string "can't be good.$"

Sinnoh_SandgemTownPokemonResearchLab_Text_Person5:
	.string "PROF. OAK: Oh ho! You must be the\n"
	.string "TRAINER who cleared SINNOH's LEAGUE.\p"
	.string "ROWAN and I go way back. He was always\n"
	.string "the serious one. I was always the one\l"
	.string "with the snacks.$"

@ --- end of SINNOH words ---
