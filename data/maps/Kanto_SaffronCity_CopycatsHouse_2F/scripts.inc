Kanto_SaffronCity_CopycatsHouse_2F_MapScripts::
	.byte 0

@ Kanto_SaffronCity_CopycatsHouse_2F object 1 (local id 1, gfx 0x3F02) at 6,8
Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EBDF::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_DODUO, 0
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_844AF8B, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_SaffronCity_CopycatsHouse_2F object 2 (local id 2, gfx 0x3F34) at 4,6
@ Kanto_SaffronCity_CopycatsHouse_2F object 3 (local id 3, gfx 0x3F0F) at 7,2
Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EBF2::
	lock
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_844AFCC, MSGBOX_DEFAULT
	release
	end

@ Kanto_SaffronCity_CopycatsHouse_2F object 4 (local id 4, gfx 0x3F18) at 9,6
Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EBFD::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	lock
	faceplayer
	checkitem ITEM_POKE_DOLL, 1
	goto_if_eq VAR_RESULT, 1, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_8471CC9
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EC3B
	call_if_eq VAR_RESULT, 1, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EC44
	release
	end

Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EC3B::
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_844ABBB, MSGBOX_DEFAULT
	return

Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EC44::
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_844AC4C, MSGBOX_DEFAULT
	return

@ Kanto_SaffronCity_CopycatsHouse_2F sign / bg event at 1,1
Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EC4D::
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_844B02B, MSGBOX_SIGN
	end

@ Kanto_SaffronCity_CopycatsHouse_2F sign / bg event at 3,5
Kanto_SaffronCity_CopycatsHouse_2F_EventScript_841EC56::
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_844AFFB, MSGBOX_SIGN
	end

@ Moemon Kai: FireRed's MIMIC tutor (pokefirered data/scripts/move_tutors.inc EventScript_MimicTutor): a POKé DOLL for
@ her, MIMIC once, on Kai's move tutors (Quetzal's "move training from the team menu" stand-in is gone)
Kanto_SaffronCity_CopycatsHouse_2F_EventScript_8471CC9::
	goto_if_set 0x2221, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicTaught
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicTeach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicDeclined
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_MIMIC
	call MoveTutor_EventScript_OpenPartyMenu
	lock
	faceplayer
	goto_if_eq VAR_RESULT, 0, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicDeclined
	removeitem ITEM_POKE_DOLL
	setflag 0x2221
	goto Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicTaught
	end

Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicDeclined::
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicDeclined, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicTaught::
	checkplayergender
	call_if_eq VAR_RESULT, MALE, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicTaughtMale
	call_if_eq VAR_RESULT, FEMALE, Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicTaughtFemale
	release
	end

Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicTaughtMale::
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicTaughtMale, MSGBOX_DEFAULT
	return

Kanto_SaffronCity_CopycatsHouse_2F_EventScript_MimicTaughtFemale::
	msgbox Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicTaughtFemale, MSGBOX_DEFAULT
	return

Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicTeach::
	.string "Oh wow!\n"
	.string "A POKé DOLL!\p"
	.string "For me?\n"
	.string "Thank you!\p"
	.string "You know what?\n"
	.string "I can teach the move MIMIC.$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicWhichMon::
	.string "I really love MIMIC!\n"
	.string "Who’s going to learn it?$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicDeclined::
	.string "Don’t you like MIMIC?$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicTaughtMale::
	.string "{PLAYER}: Hi!\n"
	.string "Thanks for earlier!\p"
	.string "{PLAYER}: Pardon?\p"
	.string "{PLAYER}: Is it that fun to mimic my\n"
	.string "every move?\p"
	.string "COPYCAT: You bet!\n"
	.string "It’s a scream!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_MimicTaughtFemale::
	.string "{PLAYER}: Hi!\n"
	.string "Thanks for what you did for us.\p"
	.string "{PLAYER}: Pardon?\p"
	.string "{PLAYER}: Is it really that fun to\n"
	.string "copy what I say and do?\p"
	.string "COPYCAT: You bet!\n"
	.string "It’s a scream!$"


Kanto_SaffronCity_CopycatsHouse_2F_Text_844ABBB::
	.string "{PLAYER}: Hi!\n"
	.string "Do you like MOéMON?\p"
	.string "{PLAYER}: Uh, no, I just asked you.\p"
	.string "{PLAYER}: Huh?\n"
	.string "You’re strange!\p"
	.string "COPYCAT: Hmm? Quit mimicking?\n"
	.string "But, that’s my favorite hobby!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844AC4C::
	.string "{PLAYER}: Hi.\n"
	.string "You must like MOéMON.\p"
	.string "{PLAYER}: No, not me.\n"
	.string "I asked you.\p"
	.string "{PLAYER}: Pardon?\n"
	.string "You’re weird!\l"
	.string " \l"
	.string "COPYCAT: Hmm? Quit mimicking?\l"
	.string "But that’s my favorite hobby!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844AF8B::
	.string "DODUO: Giiih!\p"
	.string "PRETTY MIRROR LOOK AT ME,\n"
	.string "SEE HOW PRETTY I CAN BE?$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844AFCC::
	.string "This is a rare MOéMON!\n"
	.string "Huh? It’s only a doll!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844AFFB::
	.string "A game with MARIO wearing a bucket\n"
	.string "on his head!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844B02B::
	.string "…\p"
	.string "My Secrets!\p"
	.string "Skill: Mimicry!\n"
	.string "Hobby: Collecting dolls!\l"
	.string "Favorite MOéMON: CLEFAIRY!$"
