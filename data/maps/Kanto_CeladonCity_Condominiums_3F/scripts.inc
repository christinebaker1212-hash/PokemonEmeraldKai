Kanto_CeladonCity_Condominiums_3F_MapScripts::
	.byte 0

@ Kanto_CeladonCity_Condominiums_3F object 1 (local id 1, gfx 0x3F3B) at 0,10
Kanto_CeladonCity_Condominiums_3F_EventScript_841C6AF::
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_844638B, MSGBOX_NPC
	end

@ Kanto_CeladonCity_Condominiums_3F object 3 (local id 3, gfx 0x3F4B) at 4,10
Kanto_CeladonCity_Condominiums_3F_EventScript_841C6B8::
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_84463A3, MSGBOX_NPC
	end

@ Kanto_CeladonCity_Condominiums_3F object 4 (local id 4, gfx 0x3F04) at 0,13
Kanto_CeladonCity_Condominiums_3F_EventScript_841C6C1::
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_84463C7, MSGBOX_NPC
	end

@ Kanto_CeladonCity_Condominiums_3F object 2 (local id 2, gfx 0x3F1E) at 3,8
Kanto_CeladonCity_Condominiums_3F_EventScript_841C6CA::
	lock
	faceplayer
	specialvar VAR_RESULT, QzIsRegionalDexComplete
	goto_if_eq VAR_RESULT, 1, Kanto_CeladonCity_Condominiums_3F_EventScript_841C6E6
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_8446422, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_Condominiums_3F_EventScript_841C6E6::
	goto Kanto_CeladonCity_Condominiums_3F_EventScript_841C6EC

Kanto_CeladonCity_Condominiums_3F_EventScript_841C6EC::
	message Kanto_CeladonCity_Condominiums_3F_Text_84464CF
	waitmessage
	delay 0x3C
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special Special_ShowDiploma
	waitstate
	release
	end

@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 8,12
@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 7,12
Kanto_CeladonCity_Condominiums_3F_EventScript_841C70B::
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_84465D4, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 1,9
@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 0,9
Kanto_CeladonCity_Condominiums_3F_EventScript_841C714::
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_8446538, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 5,9
@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 4,9
Kanto_CeladonCity_Condominiums_3F_EventScript_841C71D::
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_8446577, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 1,12
@ Kanto_CeladonCity_Condominiums_3F sign / bg event at 0,12
Kanto_CeladonCity_Condominiums_3F_EventScript_841C726::
	msgbox Kanto_CeladonCity_Condominiums_3F_Text_84465A4, MSGBOX_SIGN
	end

Kanto_CeladonCity_Condominiums_3F_Text_844638B::
	.string "Me?\n"
	.string "I’m the programmer!$"

Kanto_CeladonCity_Condominiums_3F_Text_84463A3::
	.string "I’m the graphic artist!\n"
	.string "I drew you!$"

Kanto_CeladonCity_Condominiums_3F_Text_84463C7::
	.string "I wrote the story!\n"
	.string "Isn’t ERIKA cute?\p"
	.string "I like MISTY a lot, too!\n"
	.string "Oh, and SABRINA, I like her!$"

Kanto_CeladonCity_Condominiums_3F_Text_8446422::
	.string "I’m the game designer!\p"
	.string "Register every NATIONAL POKéDEX\n"
	.string "species as caught in KANTO.\p"
	.string "Catches from other regions don’t\n"
	.string "count toward this diploma.\p"
	.string "Come see me when you’re done!$"

Kanto_CeladonCity_Condominiums_3F_Text_84464CF::
	.string "You’ve completed the NATIONAL\n"
	.string "POKéDEX with your KANTO catches!\p"
	.string "Congratulations!\n"
	.string "This diploma is for you!$"

Kanto_CeladonCity_Condominiums_3F_Text_8446538::
	.string "It’s the game program! Messing with\n"
	.string "it could bug out the game!$"

Kanto_CeladonCity_Condominiums_3F_Text_8446577::
	.string "Someone’s playing a game instead of\n"
	.string "working!$"

Kanto_CeladonCity_Condominiums_3F_Text_84465A4::
	.string "It’s the script!\n"
	.string "Better not look at the ending!$"

Kanto_CeladonCity_Condominiums_3F_Text_84465D4::
	.string "GAME FREAK Development Room$"
