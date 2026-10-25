Kanto_IndigoPlateau_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EAF1
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EAF1::
	setrespawn 0x2C
	call Quetzal_EventScript_84569D3
	setobjectxyperm 4, 5, 3
	call_if_set FLAG_QZ_0x2108, Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB0A
	end

Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB0A::
	goto_if_set FLAG_QZ_0x220A, Quetzal_EventScript_8457C6C
	setobjectxyperm 4, 4, 2
	return

@ Kanto_IndigoPlateau_PokemonCenter_1F object 4 (local id 4, gfx 0x3EFC) at 5,3
Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB1B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2108, Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB30
	msgbox Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A833, MSGBOX_DEFAULT
	release
	end

Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB30::
	goto_if_set FLAG_QZ_0x220A, Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB43
	msgbox Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A89E, MSGBOX_DEFAULT
	release
	end

Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB43::
	msgbox Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A833, MSGBOX_DEFAULT
	release
	end

@ Kanto_IndigoPlateau_PokemonCenter_1F object 3 (local id 3, gfx 0x3F0B) at 7,14
Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB4D::
	msgbox Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A792, MSGBOX_NPC
	end

@ Kanto_IndigoPlateau_PokemonCenter_1F object 1 (local id 1, gfx 0x3EFA) at 0,7
Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB56::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

@ Kanto_IndigoPlateau_PokemonCenter_1F object 2 (local id 2, gfx 0x3F2A) at 13,10
Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_841EB92::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_IndigoPlateau_PokemonCenter_1F object 5 (local id 5, gfx 0x3EEB) at 23,13
Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_845D37F::
	lock
	faceplayer
	setvar VAR_0x8004, 0xB
	setvar VAR_0x8005, 1
	special QzQuestLogNop_737
	setvar VAR_0x8004, 0xB
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	msgbox Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A942, MSGBOX_DEFAULT
	release
	end

@ Kanto_IndigoPlateau_PokemonCenter_1F object 6 (local id 6, gfx 0x3EFD) at 16,15
Kanto_IndigoPlateau_PokemonCenter_1F_EventScript_845D458::
	lock
	faceplayer
	setvar VAR_0x8004, 0xC
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	msgbox Kanto_IndigoPlateau_PokemonCenter_1F_Text_844AA71, MSGBOX_DEFAULT
	release
	end

Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A792::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "At the POKéMON LEAGUE, you will\n"
	.string "face the ELITE FOUR all in a row.\p"
	.string "If you lose, you have to start all\n"
	.string "over again!\p"
	.string "This is it!\n"
	.string "Go for it!$"

Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A833::
	.string "From here on, you face the\n"
	.string "ELITE FOUR one by one.\p"
	.string "If you win, a door opens to the\n"
	.string "next TRAINER. Good luck!$"

Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A89E::
	.string "I am so sorry, but you’ve wasted\n"
	.string "your time coming here.\p"
	.string "LORELEI of the ELITE FOUR is\n"
	.string "absent, you see.\p"
	.string "As a result, the POKéMON LEAGUE is\n"
	.string "closed for the time being.$"

Kanto_IndigoPlateau_PokemonCenter_1F_Text_844A942::
	.string "AGATHA’s GHOST-type POKéMON are\n"
	.string "horrifically terrifying in toughness.\p"
	.string "I took my FIGHTING-type POKéMON\n"
	.string "and raised them to the max.\p"
	.string "I went at AGATHA feeling pretty\n"
	.string "confident, but she whupped us.\p"
	.string "That old lady’s also got a really\n"
	.string "short fuse, too.\p"
	.string "It doesn’t take anything to get\n"
	.string "that scary lady hollering.$"

Kanto_IndigoPlateau_PokemonCenter_1F_Text_844AA71::
	.string "Maybe becoming an ELITE FOUR\n"
	.string "member is in the blood.\p"
	.string "From what I’ve heard, LANCE has\n"
	.string "a cousin who’s a GYM LEADER\l"
	.string "somewhere far away.$"
