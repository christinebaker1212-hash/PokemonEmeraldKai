Kanto_SaffronCity_PokemonTrainerFanClub_MapScripts::
	.byte 0

@ Kanto_SaffronCity_PokemonTrainerFanClub object 1 (local id 1, gfx 0x3EE7) at 7,13
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F31C::
	lock
	faceplayer
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D53E, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_PokemonTrainerFanClub object 2 (local id 2, gfx 0x3F18) at 6,11
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F360::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2108, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F375
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D933, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F375::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D985, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_PokemonTrainerFanClub object 3 (local id 3, gfx 0x3F5C) at 3,13
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F3B7::
	lock
	faceplayer
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D5D2, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_PokemonTrainerFanClub object 4 (local id 4, gfx 0x3F09) at 3,12
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F3FB::
	lock
	faceplayer
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D649, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_PokemonTrainerFanClub object 5 (local id 5, gfx 0x3F59) at 9,11
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F43F::
	lock
	faceplayer
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D471, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_PokemonTrainerFanClub object 6 (local id 6, gfx 0x3F3B) at 7,12
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F479::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2108, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F48E
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D82F, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F48E::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D891, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_PokemonTrainerFanClub object 7 (local id 7, gfx 0x3EE8) at 9,12
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F4C6::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2108, Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F4DB
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D72E, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F4DB::
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D78D, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_PokemonTrainerFanClub object 8 (local id 8, gfx 0x3EEB) at 5,11
Kanto_SaffronCity_PokemonTrainerFanClub_EventScript_841F513::
	lock
	faceplayer
	buffermovename 0, MOVE_DYNAMIC_PUNCH
	msgbox Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D6C4, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D471::
	.string "We’re trying to complete\n"
	.string "Kanto’s Pokédex.\p"
	.string "Even if you own a Pokémon from\n"
	.string "another region, you must obtain or\p"
	.string "evolve it here to register it\n"
	.string "in our regional collection.\p"
	.string "Taking it out of the PC\n"
	.string "doesn’t count!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D53E::
	.string "To search for a species with DexNav,\n"
	.string "you must register it as caught\p"
	.string "in the region you’re exploring.\p"
	.string "Your DexNav bonus carries over\n"
	.string "between regions!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D5D2::
	.string "Once you can use Surf, return to\n"
	.string "Route 10 and explore the river.\p"
	.string "There’s a Power Plant that\n"
	.string "many Trainers never visit.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D649::
	.string "The lab on Cinnabar Island can\n"
	.string "revive many different fossils.\p"
	.string "Some Pokémon even need two\n"
	.string "compatible pieces to be restored!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D6C4::
	.string "Primeape’s strength can go\n"
	.string "much further!\p"
	.string "If it levels up while knowing\n"
	.string "{STR_VAR_1}, it can evolve\p"
	.string "into Annihilape.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D72E::
	.string "I love visiting ancient ruins.\p"
	.string "Sometimes a stone or an inscription\n"
	.string "holds the key to a mystery.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D78D::
	.string "On Seven Island, there’s a cave\n"
	.string "with seven boulders and switches.\p"
	.string "Bring a Pokémon that can use\n"
	.string "Strength. That puzzle is linked\p"
	.string "to the secret of the Tanoby Ruins.$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D82F::
	.string "I love music that brings\n"
	.string "people and Pokémon together.\p"
	.string "I’m always looking for\n"
	.string "new places to dance!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D891::
	.string "In a house on Seven Island,\n"
	.string "they hold a Chansey dance.\p"
	.string "They say that when the music\n"
	.string "and steps are in perfect harmony,\p"
	.string "someone unexpected joins\n"
	.string "the celebration...$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D933::
	.string "I love Pokémon dolls!\p"
	.string "Someday, I want a collection\n"
	.string "big enough to fill this house!$"

Kanto_SaffronCity_PokemonTrainerFanClub_Text_844D985::
	.string "I love Lorelei’s Pokémon dolls!\p"
	.string "Her collection starts growing after\n"
	.string "you beat the Kanto League twice.\p"
	.string "The last one appears after twenty\n"
	.string "wins. I want to see them all!$"
