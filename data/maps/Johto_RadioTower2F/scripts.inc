Johto_RadioTower2F_MapScripts::
	.byte 0

@ Johto_RadioTower2F object 3 (local id 3, gfx 0x3EB0) at 19,11
Johto_RadioTower2F_EventScript_83D7A56::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4655, 0, Johto_RadioTower2F_Text_83D7D34, Johto_RadioTower2F_Text_83D7D8A
	msgbox Johto_RadioTower2F_Text_83D7DA0, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F object 6 (local id 6, gfx 0x3EB0) at 13,7
Johto_RadioTower2F_EventScript_83D7A6D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4658, 0, Johto_RadioTower2F_Text_83D7DD2, Johto_RadioTower2F_Text_83D7E1C
	msgbox Johto_RadioTower2F_Text_83D7E35, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F object 4 (local id 4, gfx 0x3EB0) at 19,5
Johto_RadioTower2F_EventScript_83D7A84::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4659, 0, Johto_RadioTower2F_Text_83D7E69, Johto_RadioTower2F_Text_83D7E88
	msgbox Johto_RadioTower2F_Text_83D7E9A, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F object 5 (local id 5, gfx 0x3EAF) at 7,7
Johto_RadioTower2F_EventScript_83D7A9B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4390, 0, Johto_RadioTower2F_Text_83D7EFD, Johto_RadioTower2F_Text_83D7F57
	msgbox Johto_RadioTower2F_Text_83D7F67, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower2F sign / bg event at 30,4
Johto_RadioTower2F_EventScript_83D7AB2::
	lock
	msgbox Johto_RadioTower2F_Text_83D7FAF, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower2F sign / bg event at 15,4
Johto_RadioTower2F_EventScript_83D7ABE::
	lock
	msgbox Johto_RadioTower2F_Text_83D7FB8, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower2F object 2 (local id 2, gfx 0xB) at 14,8
Johto_RadioTower2F_EventScript_83D7ACA::
	lock
	faceplayer
	msgbox Johto_RadioTower2F_Text_83D7B9B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower2F object 8 (local id 8, gfx 0x3EA9) at 10,5
Johto_RadioTower2F_EventScript_83D7AD7::
	lock
	faceplayer
	msgbox Johto_RadioTower2F_Text_83D7BCA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower2F object 7 (local id 7, gfx 0x2D) at 28,14
Johto_RadioTower2F_EventScript_83D7AE4::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500A, 6, Johto_RadioTower2F_EventScript_83D7B1D
	goto_if_eq VAR_QZ_0x500A, 7, Johto_RadioTower2F_EventScript_83D7B1D
	goto_if_eq VAR_QZ_0x500A, 8, Johto_RadioTower2F_EventScript_83D7B1D
	goto_if_eq VAR_QZ_0x500A, 9, Johto_RadioTower2F_EventScript_83D7B1D
	msgbox Johto_RadioTower2F_Text_83D7F8B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower2F_EventScript_83D7B1D::
	msgbox Johto_RadioTower2F_Text_83D7B6C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower2F object 1 (local id 1, gfx 0x2F) at 29,9
Johto_RadioTower2F_EventScript_83D7B28::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500A, 6, Johto_RadioTower2F_EventScript_83D7B61
	goto_if_eq VAR_QZ_0x500A, 7, Johto_RadioTower2F_EventScript_83D7B61
	goto_if_eq VAR_QZ_0x500A, 8, Johto_RadioTower2F_EventScript_83D7B61
	goto_if_eq VAR_QZ_0x500A, 9, Johto_RadioTower2F_EventScript_83D7B61
	msgbox Johto_RadioTower2F_Text_83D7BEF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower2F_EventScript_83D7B61::
	msgbox Johto_RadioTower2F_Text_83D7C1E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower2F_Text_83D7B6C::
	.string "BUENA: Huh? Today’s password?\n"
	.string "HELP, of course!$"

Johto_RadioTower2F_Text_83D7B9B::
	.string "You can listen to the radio\n"
	.string "anywhere. Tune in!$"

Johto_RadioTower2F_Text_83D7BCA::
	.string "Restricted access beyond this\n"
	.string "point.$"

Johto_RadioTower2F_Text_83D7BEF::
	.string "Lullabies on the radio may make\n"
	.string "POKéMON sleep.$"

Johto_RadioTower2F_Text_83D7C1E::
	.string "Why would they want to take over\n"
	.string "the RADIO TOWER?$"

Johto_RadioTower2F_Text_83D7D34::
	.string "Three years ago, TEAM ROCKET was\n"
	.string "forced to disband.\p"
	.string "But we’re making a comeback here!$"

Johto_RadioTower2F_Text_83D7D8A::
	.string "Gwah! Don’t get cute!$"

Johto_RadioTower2F_Text_83D7DA0::
	.string "We won’t let you ruin our plans\n"
	.string "for our comeback!$"

Johto_RadioTower2F_Text_83D7DD2::
	.string "We’re TEAM ROCKET, the exploiters\n"
	.string "of POKéMON!\p"
	.string "We love being evil! Scared?$"

Johto_RadioTower2F_Text_83D7E1C::
	.string "You think you’re a hero?$"

Johto_RadioTower2F_Text_83D7E35::
	.string "We’re not always evil. We just do\n"
	.string "whatever we like.$"

Johto_RadioTower2F_Text_83D7E69::
	.string "Hey, hey! Keep out of our way!$"

Johto_RadioTower2F_Text_83D7E88::
	.string "Arggh. I give up.$"

Johto_RadioTower2F_Text_83D7E9A::
	.string "Our EXECUTIVES are trying to take\n"
	.string "this place over.\p"
	.string "They have some big plan. I wonder\n"
	.string "what that is?$"

Johto_RadioTower2F_Text_83D7EFD::
	.string "Hahaha!\p"
	.string "How boring. It was far too easy\n"
	.string "to take over this place!\p"
	.string "Come on, keep me amused!$"

Johto_RadioTower2F_Text_83D7F57::
	.string "Wh-who are you?$"

Johto_RadioTower2F_Text_83D7F67::
	.string "You beat me, and I won’t forget\n"
	.string "it!$"

Johto_RadioTower2F_Text_83D7F8B::
	.string "BUENA: Tune in to my PASSWORD\n"
	.string "SHOW!$"

Johto_RadioTower2F_Text_83D7FAF::
	.string "2F SALES$"

Johto_RadioTower2F_Text_83D7FB8::
	.string "PROF.OAK’S POKéMON TALK\p"
	.string "The Hottest Show on the Air!$"
