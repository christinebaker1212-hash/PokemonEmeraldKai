Johto_CianwoodOldWomansHouse_MapScripts::
	.byte 0

@ Johto_CianwoodOldWomansHouse object 1 (local id 1, gfx 0x1E) at 4,4
Johto_CianwoodOldWomansHouse_EventScript_83CAC29::
	@ Moemon Kai: Crystal's POKé SEER (pokecrystal maps/PokeSeersHouse.asm, engine/events/poke_seer.asm;
	@ docs/REGIONS_AUDIT.md 146) in the place of Quetzal's bells lady; this game keeps no time of day for a meeting
	lock
	faceplayer
	msgbox Johto_CianwoodOldWomansHouse_Text_SeeAll, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	specialvar VAR_RESULT, MP_PokeSeer
	switch VAR_RESULT
	case 0, Johto_CianwoodOldWomansHouse_EventScript_SeerNothing
	case 1, Johto_CianwoodOldWomansHouse_EventScript_SeerEgg
	case 3, Johto_CianwoodOldWomansHouse_EventScript_SeerTraded
	case 4, Johto_CianwoodOldWomansHouse_EventScript_SeerCantTell
	case 5, Johto_CianwoodOldWomansHouse_EventScript_SeerLevelOnly
	msgbox Johto_CianwoodOldWomansHouse_Text_MetAt, MSGBOX_DEFAULT
	goto Johto_CianwoodOldWomansHouse_EventScript_SeerLevel
Johto_CianwoodOldWomansHouse_EventScript_SeerTraded::
	special MP_PokeSeerOT
	msgbox Johto_CianwoodOldWomansHouse_Text_Trade, MSGBOX_DEFAULT
	special MP_PokeSeerLevel
Johto_CianwoodOldWomansHouse_EventScript_SeerLevel::
	msgbox Johto_CianwoodOldWomansHouse_Text_Level, MSGBOX_DEFAULT
	goto Johto_CianwoodOldWomansHouse_EventScript_SeerAdvice
Johto_CianwoodOldWomansHouse_EventScript_SeerLevelOnly::
	msgbox Johto_CianwoodOldWomansHouse_Text_NoLocation, MSGBOX_DEFAULT
Johto_CianwoodOldWomansHouse_EventScript_SeerAdvice::
	specialvar VAR_RESULT, MP_PokeSeerAdvice
	switch VAR_RESULT
	case 1, Johto_CianwoodOldWomansHouse_EventScript_SeerConfident
	case 2, Johto_CianwoodOldWomansHouse_EventScript_SeerStrength
	case 3, Johto_CianwoodOldWomansHouse_EventScript_SeerMighty
	case 4, Johto_CianwoodOldWomansHouse_EventScript_SeerImpressed
	msgbox Johto_CianwoodOldWomansHouse_Text_MoreCare, MSGBOX_DEFAULT
	release
	end
Johto_CianwoodOldWomansHouse_EventScript_SeerConfident::
	msgbox Johto_CianwoodOldWomansHouse_Text_MoreConfident, MSGBOX_DEFAULT
	release
	end
Johto_CianwoodOldWomansHouse_EventScript_SeerStrength::
	msgbox Johto_CianwoodOldWomansHouse_Text_MuchStrength, MSGBOX_DEFAULT
	release
	end
Johto_CianwoodOldWomansHouse_EventScript_SeerMighty::
	msgbox Johto_CianwoodOldWomansHouse_Text_Mighty, MSGBOX_DEFAULT
	release
	end
Johto_CianwoodOldWomansHouse_EventScript_SeerImpressed::
	msgbox Johto_CianwoodOldWomansHouse_Text_Impressed, MSGBOX_DEFAULT
	release
	end
Johto_CianwoodOldWomansHouse_EventScript_SeerNothing::
	msgbox Johto_CianwoodOldWomansHouse_Text_DoNothing, MSGBOX_DEFAULT
	release
	end
Johto_CianwoodOldWomansHouse_EventScript_SeerEgg::
	msgbox Johto_CianwoodOldWomansHouse_Text_Egg, MSGBOX_DEFAULT
	release
	end
Johto_CianwoodOldWomansHouse_EventScript_SeerCantTell::
	msgbox Johto_CianwoodOldWomansHouse_Text_CantTell, MSGBOX_DEFAULT
	release
	end

Johto_CianwoodOldWomansHouse_Text_83CAC79::
	.string "I don’t remember the last time\n"
	.string "someone came to visit me.\p"
	.string "When I was young, I was a\n"
	.string "MOéMON Trainer, too.\l"
	.string "I tried to reach the tower and\l"
	.string "dive into the ocean.\p"
	.string "I put a lot of effort into many\n"
	.string "things.\p"
	.string "Well, I take things easy now,\n"
	.string "listening to the waves and winds\l"
	.string "on this beach.$"

Johto_CianwoodOldWomansHouse_Text_83CAD8C::
	.string "Have you heard of the TIN TOWER\n"
	.string "in ECRUTEAK?\p"
	.string "There are what they call Clear Bells\n"
	.string "on top of the tower.\p"
	.string "They don’t make a single noise\n"
	.string "in strong winds.\p"
	.string "There is only one way to make them\n"
	.string "ring.\p"
	.string "It is to obtain another CLEAR BELL\n"
	.string "just like the ones at the top of the\l"
	.string "tower.\p"
	.string "I went after it when I was young.\n"
	.string "But I never got one...$"

Johto_CianwoodOldWomansHouse_Text_83CAED4::
	.string "Oh! You’ve managed to get a\n"
	.string "CLEAR BELL?\p"
	.string "The old-time dream of mine...\n"
	.string "I see...\p"
	.string "You’ve done it...\n"
	.string "...\l"
	.string "...I can hear it!\p"
	.string "The chime of the bells on the wind\n"
	.string "coming from the direction of the\l"
	.string "TIN TOWER...$"

Johto_CianwoodOldWomansHouse_Text_83CAF9C::
	.string "Oh! You’ve managed to get a\n"
	.string "Tidal Bell?\p"
	.string "The old-time dream of mine...\n"
	.string "I see...\p"
	.string "You’ve done it...\n"
	.string "...\l"
	.string "...I can hear it!\p"
	.string "The chime of the bells on the sea\n"
	.string "breeze coming from the direction of\l"
	.string "the WHIRL ISLANDS...$"

Johto_CianwoodOldWomansHouse_Text_SeeAll::
	.string "I see all. I know all…\p"
	.string "Certainly, I know of your MOéMON!$"

Johto_CianwoodOldWomansHouse_Text_MetAt::
	.string "Hm… I see you met {STR_VAR_1}\n"
	.string "here: {STR_VAR_2}!$"

Johto_CianwoodOldWomansHouse_Text_Trade::
	.string "Hm… {STR_VAR_1} came from\n"
	.string "{STR_VAR_3} in a trade?\p"
	.string "{STR_VAR_2} was where {STR_VAR_3}\n"
	.string "met {STR_VAR_1}!$"

Johto_CianwoodOldWomansHouse_Text_Level::
	.string "Its level was {STR_VAR_3}!\p"
	.string "Am I good or what?$"

Johto_CianwoodOldWomansHouse_Text_NoLocation::
	.string "What!? Incredible!\p"
	.string "I don’t understand how, but it is\n"
	.string "incredible! You are special.\p"
	.string "I can’t tell where you met it, but\n"
	.string "it was at level {STR_VAR_3}.\p"
	.string "Am I good or what?$"

Johto_CianwoodOldWomansHouse_Text_MoreCare::
	.string "Incidentally…\p"
	.string "It would be wise to raise your\n"
	.string "MOéMON with a little more care.$"

Johto_CianwoodOldWomansHouse_Text_MoreConfident::
	.string "Incidentally…\p"
	.string "It seems to have grown a little.\p"
	.string "{STR_VAR_1} seems to be becoming\n"
	.string "more confident.$"

Johto_CianwoodOldWomansHouse_Text_MuchStrength::
	.string "Incidentally…\p"
	.string "{STR_VAR_1} has grown. It’s gained\n"
	.string "much strength.$"

Johto_CianwoodOldWomansHouse_Text_Mighty::
	.string "Incidentally…\p"
	.string "It certainly has grown mighty!\p"
	.string "This {STR_VAR_1} must have come\n"
	.string "through numerous MOéMON battles.\p"
	.string "It looks brimming with confidence.$"

Johto_CianwoodOldWomansHouse_Text_Impressed::
	.string "Incidentally…\p"
	.string "I’m impressed by your dedication.\p"
	.string "It’s been a long time since I’ve\n"
	.string "seen a MOéMON as mighty as this\l"
	.string "{STR_VAR_1}.\p"
	.string "I’m sure that seeing {STR_VAR_1}\n"
	.string "in battle would excite anyone.$"

Johto_CianwoodOldWomansHouse_Text_DoNothing::
	.string "Fufufu! I saw that you’d do\n"
	.string "nothing!$"

Johto_CianwoodOldWomansHouse_Text_Egg::
	.string "Hey!\p"
	.string "That’s an EGG!\p"
	.string "You can’t say that you’ve met it\n"
	.string "yet…$"

Johto_CianwoodOldWomansHouse_Text_CantTell::
	.string "Whaaaat? I can’t tell a thing!\p"
	.string "How could I not know of this?$"
