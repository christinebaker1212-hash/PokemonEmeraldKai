Johto_EcruteakCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_EcruteakCity_EventScript_83CD1BE
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_EcruteakCity_EventScript_83CD1BE::
	special QzJohtoTravelingEventCheck
	call Quetzal_EventScript_840EA62
	goto_if_unset FLAG_QZ_0x10A9, Johto_EcruteakCity_EventScript_83CD1DC
	goto_if_unset FLAG_QZ_0x10AC, Johto_EcruteakCity_EventScript_83CD1DC
	setflag FLAG_QZ_0x10B1
	end

Johto_EcruteakCity_EventScript_83CD1DC::
	clearflag FLAG_QZ_0x10B1
	end

@ Johto_EcruteakCity trigger at 39,41 (VAR_QZ_0x501A == 4)
Johto_EcruteakCity_EventScript_83CD1E9::
	lockall
	opendoor 0x27, 0x28
	waitdooranim
	delay 0xA
	clearflag FLAG_QZ_0x10D5
	addobject 10
	playbgm MUS_QZ_648, 1
	playse SE_BANG
	applymovement 10, Johto_EcruteakCity_Movement_83CD25E
	applymovement LOCALID_PLAYER, Johto_EcruteakCity_Movement_83CD25E
	message Johto_EcruteakCity_Text_83CD26D
	waitmovement LOCALID_PLAYER
	waitmovement 10
	turnobject LOCALID_PLAYER, 2
	closedoor 0x27, 0x28
	waitdooranim
	msgbox Johto_EcruteakCity_Text_83CD274, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 10, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject 10, 1
	msgbox Johto_EcruteakCity_Text_83CD2C2, MSGBOX_DEFAULT
	closemessage
	applymovement 10, Johto_EcruteakCity_Movement_83CD263
	waitmovement 10
	setflag FLAG_QZ_0x10D5
	removeobject 10
	fadedefaultbgm
	setvar VAR_QZ_0x501A, 5
	releaseall
	end

@ Johto_EcruteakCity object 4 (local id 4, gfx 0x3EA6) at 36,32
Johto_EcruteakCity_EventScript_83CD43C::
	lock
	faceplayer
	msgbox Johto_EcruteakCity_Text_83CD5F9, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCity object 1 (local id 1, gfx 0x2F) at 35,43
Johto_EcruteakCity_EventScript_83CD449::
	lock
	faceplayer
	msgbox Johto_EcruteakCity_Text_83CD6CD, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCity object 7 (local id 7, gfx 0x10) at 19,22
Johto_EcruteakCity_EventScript_83CD456::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10A9, Johto_EcruteakCity_EventScript_83CD46C
	msgbox Johto_EcruteakCity_Text_83CD70A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakCity_EventScript_83CD46C::
	msgbox Johto_EcruteakCity_Text_83CD77B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCity object 6 (local id 6, gfx 0x11) at 22,42
Johto_EcruteakCity_EventScript_83CD477::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10AA, Johto_EcruteakCity_EventScript_83CD48D
	msgbox Johto_EcruteakCity_Text_83CD7BE, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakCity_EventScript_83CD48D::
	msgbox Johto_EcruteakCity_Text_83CD839, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCity object 9 (local id 9, gfx 0x3E87) at 52,49
Johto_EcruteakCity_EventScript_83CD498::
	lock
	faceplayer
	msgbox Johto_EcruteakCity_Text_83CD89C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCity object 3 (local id 3, gfx 0x3EB2) at 51,34
Johto_EcruteakCity_EventScript_83CD4A5::
	lock
	faceplayer
	msgbox Johto_EcruteakCity_Text_83CD63A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCity object 5 (local id 5, gfx 0x3EB2) at 27,18
Johto_EcruteakCity_EventScript_83CD4B2::
	lock
	faceplayer
	msgbox Johto_EcruteakCity_Text_83CD8E4, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakCity object 2 (local id 2, gfx 0x3EB2) at 20,49
Johto_EcruteakCity_EventScript_83CD4BF::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x10A9, Johto_EcruteakCity_EventScript_83CD4D7
	msgbox Johto_EcruteakCity_Text_83CD9F0, MSGBOX_DEFAULT
	goto Johto_EcruteakCity_EventScript_83CD4DF

Johto_EcruteakCity_EventScript_83CD4D7::
	msgbox Johto_EcruteakCity_Text_83CDA3E, MSGBOX_DEFAULT

Johto_EcruteakCity_EventScript_83CD4DF::
	closemessage
	release
	end

@ Johto_EcruteakCity sign / bg event at 36,40
Johto_EcruteakCity_EventScript_83CD4E2::
	lock
	msgbox Johto_EcruteakCity_Text_83CDBA4, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_EcruteakCity sign / bg event at 41,25
Johto_EcruteakCity_EventScript_83CD4EE::
	lock
	msgbox Johto_EcruteakCity_Text_83CDB21, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_EcruteakCity sign / bg event at 24,15
Johto_EcruteakCity_EventScript_83CD4FA::
	lock
	msgbox Johto_EcruteakCity_Text_83CDBBB, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_EcruteakCity sign / bg event at 27,40
Johto_EcruteakCity_EventScript_83CD506::
	lock
	msgbox Johto_EcruteakCity_Text_83CDAAB, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_EcruteakCity sign / bg event at 22,48
Johto_EcruteakCity_EventScript_83CD512::
	lock
	msgbox Johto_EcruteakCity_Text_83CDB5E, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_EcruteakCity object 8 (local id 8, gfx 0x3A9E) at 19,15
Johto_EcruteakCity_EventScript_83CD51E::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_EcruteakCity_Movement_83CD25E:
	delay_16
	lock_facing_direction
	walk_down
	unlock_facing_direction
	step_end

Johto_EcruteakCity_Movement_83CD263:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_EcruteakCity_Text_83CD26D::
	.string "THUD!!$"

Johto_EcruteakCity_Text_83CD274::
	.string "SILVER: How...? How is this\n"
	.string "possible? How can I lose to mere\l"
	.string "KIMONO GIRLS...?$"

Johto_EcruteakCity_Text_83CD2C2::
	.string "SILVER: You were watching me,\n"
	.string "{PLAYER}...\p"
	.string "Yeah, that’s right!\p"
	.string "I never thought defeating five\n"
	.string "KIMONO GIRLS would be so hard...\p"
	.string "I got beaten to a pulp...\p"
	.string "...\p"
	.string "You must have come here for the\n"
	.string "same reason, right?\p"
	.string "Don’t get cocky just because you\n"
	.string "defeated TEAM ROCKET!\p"
	.string "These KIMONO GIRLS are insanely\n"
	.string "strong.\p"
	.string "See if you can defeat them.\p"
	.string "But I’m not going back, I don’t\n"
	.string "want to see you win.$"

Johto_EcruteakCity_Text_83CD5F9::
	.string "ECRUTEAK used to have two towers:\n"
	.string "one each in the east and west.$"

Johto_EcruteakCity_Text_83CD63A::
	.string "Ah, child. Have you learned to\n"
	.string "dance like the KIMONO GIRLS?\p"
	.string "If you go to their DANCE THEATER,\n"
	.string "an odd old man will give you\l"
	.string "something nice, I hear.$"

Johto_EcruteakCity_Text_83CD6CD::
	.string "I’m going to practice at the DANCE\n"
	.string "THEATER. Care to join me?$"

Johto_EcruteakCity_Text_83CD70A::
	.string "The tower that used to be here…\p"
	.string "My grandma told me it used to be\n"
	.string "much taller.\p"
	.string "I hear there are ghosts in it now.$"

Johto_EcruteakCity_Text_83CD77B::
	.string "Three big POKéMON ran off in different\n"
	.string "directions. What were they?$"

Johto_EcruteakCity_Text_83CD7BE::
	.string "I heard a rumor about OLIVINE\n"
	.string "LIGHTHOUSE.\p"
	.string "The POKéMON that serves as the\n"
	.string "beacon fell ill. Sounds like they\l"
	.string "are in trouble.$"

Johto_EcruteakCity_Text_83CD839::
	.string "The POKéMON at OLIVINE LIGHTHOUSE\n"
	.string "has been cured.\p"
	.string "Boats can safely sail out to sea at\n"
	.string "night again.$"

Johto_EcruteakCity_Text_83CD89C::
	.string "I hear POKéMON are rampaging at the\n"
	.string "LAKE OF RAGE. I’d like to see that.$"

Johto_EcruteakCity_Text_83CD8E4::
	.string "In the distant past…\p"
	.string "This tower burned in a fire. Three\n"
	.string "nameless POKéMON perished in it.\p"
	.string "A rainbow-colored POKéMON descended\n"
	.string "from the sky and resurrected them…\p"
	.string "It’s a legend that has been passed\n"
	.string "down by ECRUTEAK GYM LEADERS.\p"
	.string "Me?\p"
	.string "I was a trainer way back when.\n"
	.string "Hohoho!$"

Johto_EcruteakCity_Text_83CD9F0::
	.string "Gym Leader MORTY is not here right\n"
	.string "now. You may find him at the\l"
	.string "BURNED TOWER.$"

Johto_EcruteakCity_Text_83CDA3E::
	.string "A TEAM ROCKET member is bothering\n"
	.string "the KIMONO GIRLS.\p"
	.string "Please help them at the THEATER\n"
	.string "before entering the GYM.$"

Johto_EcruteakCity_Text_83CDAAB::
	.string "ECRUTEAK CITY\n"
	.string "A Historical City\p"
	.string "Where the Past Meets the Present$"

Johto_EcruteakCity_Text_83CDB21::
	.string "Barrier Station to TIN TOWER\p"
	.string "No Trespassing by the Unworthy.$"

Johto_EcruteakCity_Text_83CDB5E::
	.string "ECRUTEAK CITY POKéMON GYM\n"
	.string "LEADER: MORTY\p"
	.string "The Mystic Seer of the Future$"

Johto_EcruteakCity_Text_83CDBA4::
	.string "ECRUTEAK DANCE THEATER$"

Johto_EcruteakCity_Text_83CDBBB::
	.string "BURNED TOWER\p"
	.string "It was destroyed by a mysterious\n"
	.string "fire.\p"
	.string "Please stay away, as it is unsafe.$"
