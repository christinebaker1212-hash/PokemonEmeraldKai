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
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop_737
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

Kanto_SaffronCity_CopycatsHouse_2F_EventScript_8471CC9::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_SaffronCity_CopycatsHouse_2F_Text_844ABBB::
	.string "{PLAYER}: Hi!\n"
	.string "Do you like POKéMON?\p"
	.string "{PLAYER}: Uh, no, I just asked you.\p"
	.string "{PLAYER}: Huh?\n"
	.string "You’re strange!\p"
	.string "COPYCAT: Hmm? Quit mimicking?\n"
	.string "But, that’s my favorite hobby!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844AC4C::
	.string "{PLAYER}: Hi.\n"
	.string "You must like POKéMON.\p"
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
	.string "This is a rare POKéMON!\n"
	.string "Huh? It’s only a doll!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844AFFB::
	.string "A game with MARIO wearing a bucket\n"
	.string "on his head!$"

Kanto_SaffronCity_CopycatsHouse_2F_Text_844B02B::
	.string "…\p"
	.string "My Secrets!\p"
	.string "Skill: Mimicry!\n"
	.string "Hobby: Collecting dolls!\l"
	.string "Favorite POKéMON: CLEFAIRY!$"
