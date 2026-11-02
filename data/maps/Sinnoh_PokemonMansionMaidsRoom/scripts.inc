Sinnoh_PokemonMansionMaidsRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonMansionMaidsRoom_EventScript_Bg2::
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Bg3::
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Bg4::
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Bg4, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Bg5::
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Bg5, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Bg6::
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Bg6, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Bg7::
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Bg7, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DC5, Sinnoh_PokemonMansionMaidsRoom_EventScript_Person2_Given
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_SOOTHE_BELL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC5
	release
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Person2_Given::
	msgbox Sinnoh_PokemonMansionMaidsRoom_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_PokemonMansionMaidsRoom_EventScript_Person3::
	goto SinnohP_pokemon_mansion_maids_room_2
	end

Sinnoh_PokemonMansionMaidsRoom_Text_Bg2:
	.string "A soft, tidy bed. It looks very\n"
	.string "comfortable.$"

Sinnoh_PokemonMansionMaidsRoom_Text_Bg3:
	.string "A soft, tidy bed. It looks very\n"
	.string "comfortable.$"

Sinnoh_PokemonMansionMaidsRoom_Text_Bg4:
	.string "A soft, tidy bed. It looks very\n"
	.string "comfortable.$"

Sinnoh_PokemonMansionMaidsRoom_Text_Bg5:
	.string "A soft, tidy bed. It looks very\n"
	.string "comfortable.$"

Sinnoh_PokemonMansionMaidsRoom_Text_Bg6:
	.string "A soft, tidy bed. It looks very\n"
	.string "comfortable.$"

Sinnoh_PokemonMansionMaidsRoom_Text_Bg7:
	.string "A soft, tidy bed. It looks very\n"
	.string "comfortable.$"

Sinnoh_PokemonMansionMaidsRoom_Text_Person2:
	.string "This is the maids' room. Shh, someone's\n"
	.string "napping!$"

Sinnoh_PokemonMansionMaidsRoom_Text_Person2Gift:
	.string "Here, this bell is too loud for the\n"
	.string "maids' room. Your MOéMON will love it.$"

@ --- end of SINNOH words ---
