Kanto_LavenderTown_VolunteerPokemonHouse_MapScripts::
	.byte 0

@ Kanto_LavenderTown_VolunteerPokemonHouse object 1 (local id 1, gfx 0x3F26) at 3,3
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B123::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x215D, Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B17D
	checkitem ITEM_POKE_FLUTE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B170
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443861, MSGBOX_DEFAULT
	additem ITEM_POKE_FLUTE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B187
	loadword 0, Kanto_LavenderTown_VolunteerPokemonHouse_Text_844391F
	setorcopyvar VAR_0x8000, 0x2D4
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E

Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B170::
	setflag FLAG_QZ_0x215D
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443947, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B17D::
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_84439DA, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B187::
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_84439BD, MSGBOX_DEFAULT
	release
	end

@ Kanto_LavenderTown_VolunteerPokemonHouse object 6 (local id 6, gfx 0x3F18) at 7,2
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B191::
	lock
	faceplayer
	setvar VAR_0x8004, 0xE
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_84437B4, MSGBOX_DEFAULT
	release
	end

@ Kanto_LavenderTown_VolunteerPokemonHouse object 4 (local id 4, gfx 0x3F17) at 9,5
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B1AA::
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443816, MSGBOX_NPC
	end

@ Kanto_LavenderTown_VolunteerPokemonHouse object 5 (local id 5, gfx 0x3F5C) at 3,5
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B1B3::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x21ED, Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B1C8
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_844374F, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B1C8::
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_844377F, MSGBOX_DEFAULT
	release
	end

@ Kanto_LavenderTown_VolunteerPokemonHouse object 2 (local id 2, gfx 0x3F29) at 1,4
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B1D2::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_NIDORINO, 0
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443840, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_LavenderTown_VolunteerPokemonHouse object 3 (local id 3, gfx 0x3F39) at 9,6
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B1E5::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PSYDUCK, 0
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443850, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_LavenderTown_VolunteerPokemonHouse sign / bg event at 5,4
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B1F8::
	lockall
	setvar VAR_0x8004, 0xE
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443A02, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_LavenderTown_VolunteerPokemonHouse sign / bg event at 3,1
@ Kanto_LavenderTown_VolunteerPokemonHouse sign / bg event at 4,1
Kanto_LavenderTown_VolunteerPokemonHouse_EventScript_841B210::
	msgbox Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443A8D, MSGBOX_SIGN
	end

Kanto_LavenderTown_VolunteerPokemonHouse_Text_844374F::
	.string "That’s odd, MR. FUJI isn’t here.\n"
	.string "Where’d he go?$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_844377F::
	.string "MR. FUJI had been praying alone\n"
	.string "for CUBONE’s mother.$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_84437B4::
	.string "This is really MR. FUJI’s house.\p"
	.string "He’s really kind.\p"
	.string "He looks after abandoned and\n"
	.string "orphaned POKéMON.$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443816::
	.string "It’s so warm!\n"
	.string "POKéMON are so nice to hug.$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443840::
	.string "NIDORINO: Gaoo!$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443850::
	.string "PSYDUCK: Gwappa!$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443861::
	.string "MR. FUJI: {PLAYER}…\p"
	.string "Your POKéDEX quest is one that\n"
	.string "requires strong dedication.\p"
	.string "Without deep love for POKéMON,\n"
	.string "your quest may fail.\p"
	.string "I’m not sure if this will help you,\n"
	.string "but I’d like you to have it.$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_844391F::
	.string "{PLAYER} received a POKé FLUTE\n"
	.string "from MR. FUJI.$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443947::
	.string "Upon hearing the POKé FLUTE,\n"
	.string "sleeping POKéMON will spring awake.\p"
	.string "Try using it on POKéMON that are\n"
	.string "sleeping obstacles.$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_84439BD::
	.string "You must make room for this!$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_84439DA::
	.string "MR. FUJI: Has my POKé FLUTE\n"
	.string "helped you?$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443A02::
	.string "POKéMON FAN MAGAZINE\n"
	.string "Monthly Grand Prize Drawing!\p"
	.string "The application form is…\p"
	.string "Gone! It’s been clipped out.\n"
	.string "Someone must have applied already.$"

Kanto_LavenderTown_VolunteerPokemonHouse_Text_8443A8D::
	.string "POKéMON magazines line the shelf.\p"
	.string "POKéMON INSIDER…\p"
	.string "POKéMON FAN…$"
