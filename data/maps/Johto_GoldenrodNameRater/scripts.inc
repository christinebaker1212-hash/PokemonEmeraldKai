Johto_GoldenrodNameRater_MapScripts::
	.byte 0

@ Johto_GoldenrodNameRater sign / bg event at 0,1
@ Johto_GoldenrodNameRater sign / bg event at 1,1
Johto_GoldenrodNameRater_EventScript_83D616F::
	lock
	msgbox Johto_GoldenrodNameRater_Text_83D617A, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodNameRater sign / bg event at 2,1
Johto_GoldenrodNameRater_EventScript_83D62A0::
	msgbox Johto_GoldenrodNameRater_Text_83D62A9, MSGBOX_DEFAULT
	end

@ Johto_GoldenrodNameRater object 1 (local id 1, gfx 0x3EA6) at 4,4
Johto_GoldenrodNameRater_EventScript_83D62CD::
	@ Moemon Kai: Crystal's NAME RATER (pokecrystal engine/events/name_rater.asm, docs/REGIONS_AUDIT.md 118) on the
	@ flow of KANTO's (Kanto_LavenderTown_House2): like SLATEPORT's he renames traded POKéMON too
	lock
	faceplayer
	msgbox Johto_GoldenrodNameRater_Text_Hello, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodNameRater_EventScript_ComeAgain
	msgbox Johto_GoldenrodNameRater_Text_WhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Johto_GoldenrodNameRater_EventScript_ComeAgain
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Johto_GoldenrodNameRater_EventScript_Egg
	special BufferMonNickname
	msgbox Johto_GoldenrodNameRater_Text_BetterName, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodNameRater_EventScript_ComeAgain
	msgbox Johto_GoldenrodNameRater_Text_WhatName, MSGBOX_DEFAULT
	call Common_EventScript_NameReceivedPartyMon
	specialvar VAR_RESULT, TryPutNameRaterShowOnTheAir
	special BufferMonNickname
	goto_if_eq VAR_RESULT, TRUE, Johto_GoldenrodNameRater_EventScript_Named
	msgbox Johto_GoldenrodNameRater_Text_SameName, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodNameRater_EventScript_Named::
	msgbox Johto_GoldenrodNameRater_Text_Finished, MSGBOX_DEFAULT
	msgbox Johto_GoldenrodNameRater_Text_NamedIt, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodNameRater_EventScript_Egg::
	msgbox Johto_GoldenrodNameRater_Text_Egg, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodNameRater_EventScript_ComeAgain::
	msgbox Johto_GoldenrodNameRater_Text_ComeAgain, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodNameRater_Text_83D617A::
	.string "It’s full of difficult books.$"

Johto_GoldenrodNameRater_Text_83D62A9::
	.string "The redheaded boy’s name\n"
	.string "is {RIVAL}.$"

Johto_GoldenrodNameRater_Text_83D62D9::
	.string "You can change any nickname\n"
	.string "from the party menu.$"

Johto_GoldenrodNameRater_Text_Hello::
	.string "Hello, hello! I’m the NAME RATER.\p"
	.string "I rate the names of MOéMON.\p"
	.string "Would you like me to rate names?$"

Johto_GoldenrodNameRater_Text_WhichMon::
	.string "Which MOéMON’s nickname should I\n"
	.string "rate for you?$"

Johto_GoldenrodNameRater_Text_BetterName::
	.string "Hm… {STR_VAR_1}… That’s a fairly\n"
	.string "decent name.\p"
	.string "But, how about a slightly better\n"
	.string "nickname?\p"
	.string "Want me to give it a better name?$"

Johto_GoldenrodNameRater_Text_WhatName::
	.string "All right. What name should we\n"
	.string "give it, then?$"

Johto_GoldenrodNameRater_Text_Finished::
	.string "That’s a better name than before!\p"
	.string "Well done!$"

Johto_GoldenrodNameRater_Text_ComeAgain::
	.string "OK, then. Come again sometime.$"

Johto_GoldenrodNameRater_Text_Egg::
	.string "Whoa… That’s just an EGG.$"

Johto_GoldenrodNameRater_Text_SameName::
	.string "It might look the same as before,\n"
	.string "but this new name is much better!\p"
	.string "Well done!$"

Johto_GoldenrodNameRater_Text_NamedIt::
	.string "All right. This MOéMON is now\n"
	.string "named {STR_VAR_1}.$"
