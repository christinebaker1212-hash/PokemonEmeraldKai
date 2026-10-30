Johto_RadioTower3F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Johto_RadioTower3F_EventScript_83D8013
	.byte 0

Johto_RadioTower3F_EventScript_83D8013::
	call_if_set FLAG_QZ_0x10A4, Quetzal_EventScript_83D801D
	end

@ Johto_RadioTower3F object 4 (local id 4, gfx 0x3EB0) at 16,5
Johto_RadioTower3F_EventScript_83D809C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4660, 0, Johto_RadioTower3F_Text_83D84BF, Johto_RadioTower3F_Text_83D850E
	msgbox Johto_RadioTower3F_Text_83D8515, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower3F object 5 (local id 5, gfx 0x3EB0) at 23,10
Johto_RadioTower3F_EventScript_83D80B3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4661, 0, Johto_RadioTower3F_Text_83D8547, Johto_RadioTower3F_Text_83D8579
	msgbox Johto_RadioTower3F_Text_83D8589, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower3F object 6 (local id 6, gfx 0x3EB0) at 25,5
Johto_RadioTower3F_EventScript_83D80CA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4571, 0, Johto_RadioTower3F_Text_83D85C4, Johto_RadioTower3F_Text_83D8606
	msgbox Johto_RadioTower3F_Text_83D8614, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower3F object 1 (local id 1, gfx 0x3EB4) at 17,10
Johto_RadioTower3F_EventScript_83D80E1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4667, 0, Johto_RadioTower3F_Text_83D8657, Johto_RadioTower3F_Text_83D8685
	msgbox Johto_RadioTower3F_Text_83D86A2, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower3F object 2 (local id 2, gfx 0x3E8F) at 14,13
Johto_RadioTower3F_EventScript_83D80F8::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1040, Johto_RadioTower3F_EventScript_83D8149
	goto_if_ge VAR_QZ_0x500A, 10, Johto_RadioTower3F_EventScript_83D8119
	msgbox Johto_RadioTower3F_Text_83D8371, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower3F_EventScript_83D8119::
	lock
	faceplayer
	msgbox Johto_RadioTower3F_Text_83D83FE, MSGBOX_DEFAULT
	closemessage
	setorcopyvar VAR_0x8000, ITEM_LIFE_ORB
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1040
	msgbox Johto_RadioTower3F_Text_83D843D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower3F_EventScript_83D8149::
	msgbox Johto_RadioTower3F_Text_83D84A4, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower3F object 3 (local id 3, gfx 0x3EB7) at 12,10
Johto_RadioTower3F_EventScript_83D8154::
	lock
	faceplayer
	msgbox Johto_RadioTower3F_Text_83D823A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower3F sign / bg event at 25,9
@ Johto_RadioTower3F sign / bg event at 26,9
@ Johto_RadioTower3F sign / bg event at 27,9
@ Johto_RadioTower3F sign / bg event at 28,9
@ Johto_RadioTower3F sign / bg event at 29,9
@ Johto_RadioTower3F sign / bg event at 30,9
@ Johto_RadioTower3F sign / bg event at 31,9
Johto_RadioTower3F_EventScript_83D8161::
	lock
	msgbox Johto_RadioTower3F_Text_83D81CA, MSGBOX_SIGN
	goto_if_set FLAG_QZ_0x10A4, Johto_RadioTower3F_EventScript_83D8186
	checkitem ITEM_CARD_KEY, 1
	goto_if_eq VAR_RESULT, 1, Johto_RadioTower3F_EventScript_83D8191
	closemessage
	release
	end

Johto_RadioTower3F_EventScript_83D8186::
	msgbox Johto_RadioTower3F_Text_83D81FC, MSGBOX_SIGN
	closemessage
	release
	end

Johto_RadioTower3F_EventScript_83D8191::
	msgbox Johto_RadioTower3F_Text_83D81E2, MSGBOX_SIGN
	call Quetzal_EventScript_83D801D
	playse SE_CLICK
	special DrawWholeMapView
	msgbox Johto_RadioTower3F_Text_83D81FC, MSGBOX_SIGN
	setflag FLAG_QZ_0x10A4
	closemessage
	release
	end

@ Johto_RadioTower3F sign / bg event at 11,4
Johto_RadioTower3F_EventScript_83D81B2::
	lock
	msgbox Johto_RadioTower3F_Text_83D820E, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower3F sign / bg event at 15,4
Johto_RadioTower3F_EventScript_83D81BE::
	lock
	msgbox Johto_RadioTower3F_Text_83D821B, MSGBOX_SIGN
	closemessage
	release
	end

Johto_RadioTower3F_Text_83D81CA::
	.string "It’s the CARD KEY slot.$"

Johto_RadioTower3F_Text_83D81E2::
	.string "{PLAYER} inserted the CARD KEY.$"

Johto_RadioTower3F_Text_83D81FC::
	.string "The gate is open.$"

Johto_RadioTower3F_Text_83D820E::
	.string "3F PERSONNEL$"

Johto_RadioTower3F_Text_83D821B::
	.string "POKéMON MUSIC with Host DJ BEN$"

Johto_RadioTower3F_Text_83D823A::
	.string "We have recordings of the cries\n"
	.string "of all POKéMON that have been\l"
	.string "found.\p"
	.string "We must have about 400 kinds.$"

Johto_RadioTower3F_Text_83D8371::
	.string "The TEAM ROCKET boss has locked\n"
	.string "himself in.\p"
	.string "But the DIRECTOR can open it.\p"
	.string "He’s up on the fifth floor.\p"
	.string "Please save him!$"

Johto_RadioTower3F_Text_83D83FE::
	.string "Thank you! You’re my hero!\p"
	.string "This is a token of my appreciation.$"

Johto_RadioTower3F_Text_83D843D::
	.string "THROAT SPRAY is used up after\n"
	.string "its holder uses a sound move.\p"
	.string "In return, it boosts\n"
	.string "the holder’s Sp. Atk!$"

Johto_RadioTower3F_Text_83D84A4::
	.string "You were simply marvelous!$"

Johto_RadioTower3F_Text_83D84BF::
	.string "I’ve been given strict orders.\p"
	.string "I’m to crush anyone who challenges\n"
	.string "TEAM ROCKET!$"

Johto_RadioTower3F_Text_83D850E::
	.string "What?!$"

Johto_RadioTower3F_Text_83D8515::
	.string "I failed in my duties…\p"
	.string "ARCHER will be so upset...$"

Johto_RadioTower3F_Text_83D8547::
	.string "It feels great ordering POKéMON\n"
	.string "to commit crimes.$"

Johto_RadioTower3F_Text_83D8579::
	.string "You’re kidding!$"

Johto_RadioTower3F_Text_83D8589::
	.string "I feel lousy over losing!\p"
	.string "Darn it! I hate useless POKéMON!$"

Johto_RadioTower3F_Text_83D85C4::
	.string "Why did the shutter open? Did you\n"
	.string "have something to do with this?$"

Johto_RadioTower3F_Text_83D8606::
	.string "I’m done for!$"

Johto_RadioTower3F_Text_83D8614::
	.string "What?! You made it past our men in\n"
	.string "the UNDERGROUND?\p"
	.string "How could you?$"

Johto_RadioTower3F_Text_83D8657::
	.string "An unknown child wandering here?\p"
	.string "Who are you?$"

Johto_RadioTower3F_Text_83D8685::
	.string "Tch! I took you too lightly!$"

Johto_RadioTower3F_Text_83D86A2::
	.string "Bwahahaha…\p"
	.string "I can transmit as strong a signal\n"
	.string "as I need from here.$"
