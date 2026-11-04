Kanto_LavenderTown_House2_MapScripts::
	.byte 0

@ Moemon Kai: FireRed's NAME RATER (pokefirered LavenderTown_House2) on Emerald's naming specials; like SLATEPORT's he
@ renames traded and gifted POKéMON too (no OT check)
@ Kanto_LavenderTown_House2 object 1 (local id 1, gfx 0x3F09) at 4,4
Kanto_LavenderTown_House2_EventScript_841B24D::
	lock
	faceplayer
	msgbox Kanto_LavenderTown_House2_Text_WantMeToRateNicknames, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_LavenderTown_House2_EventScript_ChooseMon
	goto_if_eq VAR_RESULT, NO, Kanto_LavenderTown_House2_EventScript_DontRateNickname
	end

Kanto_LavenderTown_House2_EventScript_ChooseMon::
	msgbox Kanto_LavenderTown_House2_Text_CritiqueWhichMonsNickname, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_lt VAR_0x8004, PARTY_SIZE, Kanto_LavenderTown_House2_EventScript_CheckCanRateMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kanto_LavenderTown_House2_EventScript_DontRateNickname
	end

Kanto_LavenderTown_House2_EventScript_DontRateNickname::
	msgbox Kanto_LavenderTown_House2_Text_ISeeComeVisitAgain, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_House2_EventScript_CheckCanRateMon::
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, Kanto_LavenderTown_House2_EventScript_CantNicknameEgg
	special BufferMonNickname
	msgbox Kanto_LavenderTown_House2_Text_GiveItANicerName, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kanto_LavenderTown_House2_EventScript_ChooseNewNickname
	goto_if_eq VAR_RESULT, NO, Kanto_LavenderTown_House2_EventScript_DontRateNickname
	end

Kanto_LavenderTown_House2_EventScript_CantNicknameEgg::
	msgbox Kanto_LavenderTown_House2_Text_ThatIsMerelyAnEgg, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_House2_EventScript_CantNicknameTradeMon::
	msgbox Kanto_LavenderTown_House2_Text_TrulyImpeccableName, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_House2_EventScript_ChooseNewNickname::
	msgbox Kanto_LavenderTown_House2_Text_WhatShallNewNicknameBe, MSGBOX_DEFAULT
	call Common_EventScript_NameReceivedPartyMon
	specialvar VAR_RESULT, TryPutNameRaterShowOnTheAir
	special BufferMonNickname
	goto_if_eq VAR_RESULT, TRUE, Kanto_LavenderTown_House2_EventScript_ChoseNewNickname
	msgbox Kanto_LavenderTown_House2_Text_FromNowOnShallBeKnownAsSameName, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_House2_EventScript_ChoseNewNickname::
	msgbox Kanto_LavenderTown_House2_Text_FromNowOnShallBeKnownAsName, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_House2_Text_CritiqueWhichMonsNickname::
	.string "Which MOéMON’s nickname should\n"
	.string "I critique?$"

Kanto_LavenderTown_House2_Text_FromNowOnShallBeKnownAsName::
	.string "Done! From now on, this MOéMON\n"
	.string "shall be known as {STR_VAR_1}!\p"
	.string "It is a better name than before!\n"
	.string "How fortunate for you!$"

Kanto_LavenderTown_House2_Text_FromNowOnShallBeKnownAsSameName::
	.string "Done! From now on, this MOéMON\n"
	.string "shall be known as {STR_VAR_1}!\p"
	.string "It looks no different from before,\n"
	.string "and yet, this is vastly superior!\p"
	.string "How fortunate for you!$"

Kanto_LavenderTown_House2_Text_GiveItANicerName::
	.string "{STR_VAR_1}, is it?\n"
	.string "That is a decent nickname!\p"
	.string "But, would you like me to give it\n"
	.string "a nicer name?\p"
	.string "How about it?$"

Kanto_LavenderTown_House2_Text_ISeeComeVisitAgain::
	.string "I see.\n"
	.string "Do come visit again.$"

Kanto_LavenderTown_House2_Text_ThatIsMerelyAnEgg::
	.string "Now, now.\n"
	.string "That is merely an EGG!$"

Kanto_LavenderTown_House2_Text_TrulyImpeccableName::
	.string "{STR_VAR_1}, is it?\n"
	.string "That is a truly impeccable name!\p"
	.string "Take good care of {STR_VAR_1}!$"

Kanto_LavenderTown_House2_Text_WantMeToRateNicknames::
	.string "Hello, hello!\n"
	.string "I am the official NAME RATER!\p"
	.string "Want me to rate the nicknames of\n"
	.string "your MOéMON?$"

Kanto_LavenderTown_House2_Text_WhatShallNewNicknameBe::
	.string "Ah, good. Then, what shall the new\n"
	.string "nickname be?$"
