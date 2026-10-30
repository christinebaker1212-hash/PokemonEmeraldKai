Kanto_FuchsiaCity_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FuchsiaCity_EventScript_8416A37
	.byte 0

Kanto_FuchsiaCity_EventScript_8416A37::
	special QzKantoTravelingEventCheck
	setflag FLAG_QZ_0x223E
	call_if_set FLAG_QZ_0x2144, Kanto_FuchsiaCity_EventScript_8416A50
	call_if_unset FLAG_QZ_0x2144, Kanto_FuchsiaCity_EventScript_8416A56
	end

Kanto_FuchsiaCity_EventScript_8416A50::
	setvar VAR_QZ_0x513E, 0x3F30
	return

Kanto_FuchsiaCity_EventScript_8416A56::
	setvar VAR_QZ_0x513E, 0x3F10
	return

@ Kanto_FuchsiaCity object 6 (local id 6, gfx 0x3F17) at 14,15
Kanto_FuchsiaCity_EventScript_8416A5C::
	msgbox Kanto_FuchsiaCity_Text_843074A, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity object 8 (local id 8, gfx 0x3F2C) at 36,20
Kanto_FuchsiaCity_EventScript_8416A65::
	msgbox Kanto_FuchsiaCity_Text_84307A2, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity object 1 (local id 1, gfx 0x3F04) at 37,17
Kanto_FuchsiaCity_EventScript_8416A6E::
	msgbox Kanto_FuchsiaCity_Text_8430808, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity object 4 (local id 4, gfx 0x3F5C) at 33,11
Kanto_FuchsiaCity_EventScript_8416A77::
	msgbox Kanto_FuchsiaCity_Text_8430836, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity object 16 (local id 16, gfx 0x3F16) at 40,6
Kanto_FuchsiaCity_EventScript_8416A80::
	lock
	faceplayer
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	msgbox Kanto_FuchsiaCity_Text_8430C99, MSGBOX_DEFAULT
	release
	end

@ Kanto_FuchsiaCity sign / bg event at 31,18
Kanto_FuchsiaCity_EventScript_8416A99::
	msgbox Kanto_FuchsiaCity_Text_8430887, MSGBOX_SIGN
	end

@ Kanto_FuchsiaCity sign / bg event at 26,18
Kanto_FuchsiaCity_EventScript_8416AA2::
	msgbox Kanto_FuchsiaCity_Text_84308AF, MSGBOX_SIGN
	end

@ Kanto_FuchsiaCity sign / bg event at 22,7
Kanto_FuchsiaCity_EventScript_8416AAB::
	msgbox Kanto_FuchsiaCity_Text_84308CC, MSGBOX_SIGN
	end

@ Kanto_FuchsiaCity sign / bg event at 31,31
Kanto_FuchsiaCity_EventScript_8416AB4::
	msgbox Kanto_FuchsiaCity_Text_84308E9, MSGBOX_SIGN
	end

@ Kanto_FuchsiaCity sign / bg event at 5,32
Kanto_FuchsiaCity_EventScript_8416AC6::
	lockall
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	msgbox Kanto_FuchsiaCity_Text_8430944, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_FuchsiaCity sign / bg event at 41,10
Kanto_FuchsiaCity_EventScript_8416ADE::
	lockall
	setvar VAR_0x8004, SPECIES_CHANSEY
	special QzSetSeenMon
	showmonpic SPECIES_CHANSEY, 0xA, 3
	msgbox Kanto_FuchsiaCity_Text_8430985, MSGBOX_DEFAULT
	hidemonpic
	releaseall
	end

@ Kanto_FuchsiaCity sign / bg event at 35,10
Kanto_FuchsiaCity_EventScript_8416AF7::
	lockall
	setvar VAR_0x8004, SPECIES_VOLTORB
	special QzSetSeenMon
	showmonpic SPECIES_VOLTORB, 0xA, 3
	msgbox Kanto_FuchsiaCity_Text_84309B5, MSGBOX_DEFAULT
	hidemonpic
	releaseall
	end

@ Kanto_FuchsiaCity sign / bg event at 17,10
Kanto_FuchsiaCity_EventScript_8416B10::
	lockall
	setvar VAR_0x8004, SPECIES_KANGASKHAN
	special QzSetSeenMon
	showmonpic SPECIES_KANGASKHAN, 0xA, 3
	msgbox Kanto_FuchsiaCity_Text_84309E2, MSGBOX_DEFAULT
	hidemonpic
	releaseall
	end

@ Kanto_FuchsiaCity sign / bg event at 38,16
Kanto_FuchsiaCity_EventScript_8416B29::
	lockall
	setvar VAR_0x8004, SPECIES_SLOWPOKE
	special QzSetSeenMon
	showmonpic SPECIES_SLOWPOKE, 0xA, 3
	msgbox Kanto_FuchsiaCity_Text_8430A35, MSGBOX_DEFAULT
	hidemonpic
	releaseall
	end

@ Kanto_FuchsiaCity sign / bg event at 17,18
Kanto_FuchsiaCity_EventScript_8416B42::
	lockall
	setvar VAR_0x8004, SPECIES_LAPRAS
	special QzSetSeenMon
	showmonpic SPECIES_LAPRAS, 0xA, 3
	msgbox Kanto_FuchsiaCity_Text_8430A63, MSGBOX_DEFAULT
	hidemonpic
	releaseall
	end

@ Kanto_FuchsiaCity sign / bg event at 11,10
Kanto_FuchsiaCity_EventScript_8416B5B::
	lockall
	goto_if_set FLAG_QZ_0x2144, Kanto_FuchsiaCity_EventScript_8416B7D
	setvar VAR_0x8004, SPECIES_KABUTO
	special QzSetSeenMon
	showmonpic SPECIES_KABUTO, 0xA, 3
	msgbox Kanto_FuchsiaCity_Text_8430AD6, MSGBOX_DEFAULT
	hidemonpic
	releaseall
	end

Kanto_FuchsiaCity_EventScript_8416B7D::
	setvar VAR_0x8004, SPECIES_OMANYTE
	special QzSetSeenMon
	showmonpic SPECIES_OMANYTE, 0xA, 3
	msgbox Kanto_FuchsiaCity_Text_8430A8A, MSGBOX_DEFAULT
	hidemonpic
	releaseall
	end

@ Kanto_FuchsiaCity object 15 (local id 15, gfx 0x3EF1) at 15,11
Kanto_FuchsiaCity_EventScript_8471C9C::
	msgbox Quetzal_Text_8471CE4, MSGBOX_NPC
	end

Kanto_FuchsiaCity_Text_843074A::
	.string "Did you try the SAFARI GAME?\p"
	.string "There are some rare POKéMON that\n"
	.string "can only be caught there.$"

Kanto_FuchsiaCity_Text_84307A2::
	.string "The SAFARI ZONE has a zoo in front\n"
	.string "of the entrance.\p"
	.string "Out back is the SAFARI GAME for\n"
	.string "catching POKéMON.$"

Kanto_FuchsiaCity_Text_8430808::
	.string "ERIK: Where’s SARA?\n"
	.string "I said I’d meet her here.$"

Kanto_FuchsiaCity_Text_8430836::
	.string "That item ball in there…\n"
	.string "Were you wanting that?\p"
	.string "Me, too!\n"
	.string "…Huh? That’s a POKéMON?$"

Kanto_FuchsiaCity_Text_8430887::
	.string "FUCHSIA CITY\n"
	.string "Behold! It’s Passion Pink!$"

Kanto_FuchsiaCity_Text_84308AF::
	.string "POKéMON PARADISE\n"
	.string "SAFARI ZONE$"

Kanto_FuchsiaCity_Text_84308CC::
	.string "SAFARI GAME\n"
	.string "POKéMON-U-CATCH!$"

Kanto_FuchsiaCity_Text_84308E9::
	.string "SAFARI ZONE\n"
	.string "WARDEN’S HOME$"

Kanto_FuchsiaCity_Text_8430944::
	.string "FUCHSIA CITY POKéMON GYM\n"
	.string "LEADER: KOGA\l"
	.string "The Poisonous Ninja Master$"

Kanto_FuchsiaCity_Text_8430985::
	.string "Name: CHANSEY\n"
	.string "Catching one is all up to chance.$"

Kanto_FuchsiaCity_Text_84309B5::
	.string "Name: VOLTORB\n"
	.string "The very image of a POKé BALL.$"

Kanto_FuchsiaCity_Text_84309E2::
	.string "Name: KANGASKHAN\p"
	.string "A maternal POKéMON that raises its\n"
	.string "young in a pouch on its belly.$"

Kanto_FuchsiaCity_Text_8430A35::
	.string "Name: SLOWPOKE\n"
	.string "Friendly and very slow moving.$"

Kanto_FuchsiaCity_Text_8430A63::
	.string "Name: LAPRAS\n"
	.string "AKA the king of the seas.$"

Kanto_FuchsiaCity_Text_8430A8A::
	.string "Name: OMANYTE\n"
	.string "An extremely rare POKéMON that was\l"
	.string "regenerated from a fossil.$"

Kanto_FuchsiaCity_Text_8430AD6::
	.string "Name: KABUTO\n"
	.string "An extremely rare POKéMON that was\l"
	.string "regenerated from a fossil.$"

Kanto_FuchsiaCity_Text_8430C99::
	.string "My father is the GYM LEADER of\n"
	.string "this town.\p"
	.string "I’m training to use POISON POKéMON\n"
	.string "as well as my father.$"
