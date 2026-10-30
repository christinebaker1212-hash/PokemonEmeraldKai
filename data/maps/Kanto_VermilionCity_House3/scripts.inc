Kanto_VermilionCity_House3_MapScripts::
	.byte 0

@ Kanto_VermilionCity_House3 object 1 (local id 1, gfx 0x3EEE) at 7,4
Kanto_VermilionCity_House3_EventScript_841BAB9::
	msgbox Kanto_VermilionCity_House3_Text_8445411, MSGBOX_NPC
	end

@ Kanto_VermilionCity_House3 object 4 (local id 4, gfx 0x3F16) at 2,4
Kanto_VermilionCity_House3_EventScript_841BAC2::
	msgbox Kanto_VermilionCity_House3_Text_84454D3, MSGBOX_NPC
	end

@ Kanto_VermilionCity_House3 object 2 (local id 2, gfx 0x3F33) at 2,5
Kanto_VermilionCity_House3_EventScript_841BACB::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PIDGEY, 0
	msgbox Kanto_VermilionCity_House3_Text_8445450, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_VermilionCity_House3 sign / bg event at 6,4
Kanto_VermilionCity_House3_EventScript_841BADE::
	msgbox Kanto_VermilionCity_House3_Text_8445462, MSGBOX_SIGN
	end

Kanto_VermilionCity_House3_Text_8445411::
	.string "I’m getting my PIDGEY to fly a\n"
	.string "letter to SAFFRON in the north.$"

Kanto_VermilionCity_House3_Text_8445450::
	.string "PIDGEY: Kurukkoo!$"

Kanto_VermilionCity_House3_Text_8445462::
	.string "Dear PIPPI,\n"
	.string "I hope to see you soon.\p"
	.string "I heard SAFFRON has problems with\n"
	.string "TEAM ROCKET.\p"
	.string "VERMILION appears to be safe.$"

Kanto_VermilionCity_House3_Text_84454D3::
	.string "I want to exchange MAIL with all\n"
	.string "sorts of people.\p"
	.string "I send my PIDGEY to a UNION ROOM\n"
	.string "to exchange MAIL for me.$"
