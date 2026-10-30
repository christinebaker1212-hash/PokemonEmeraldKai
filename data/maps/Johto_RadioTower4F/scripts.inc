Johto_RadioTower4F_MapScripts::
	.byte 0

@ Johto_RadioTower4F sign / bg event at 28,4
Johto_RadioTower4F_EventScript_83D86E5::
	lock
	msgbox Johto_RadioTower4F_Text_83D8D3F, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower4F sign / bg event at 11,4
Johto_RadioTower4F_EventScript_83D86F1::
	lock
	msgbox Johto_RadioTower4F_Text_83D8D31, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RadioTower4F object 5 (local id 5, gfx 0x3EAF) at 15,5
Johto_RadioTower4F_EventScript_83D86FD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4524, 0, Johto_RadioTower4F_Text_83D8BF1, Johto_RadioTower4F_Text_83D8C49
	msgbox Johto_RadioTower4F_Text_83D8C65, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower4F object 2 (local id 2, gfx 0x3EB0) at 15,11
Johto_RadioTower4F_EventScript_83D8714::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4839, 0, Johto_RadioTower4F_Text_83D8AA4, Johto_RadioTower4F_Text_83D8AFB
	msgbox Johto_RadioTower4F_Text_83D8B0D, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower4F object 6 (local id 6, gfx 0x3EAC) at 28,9
Johto_RadioTower4F_EventScript_83D872B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4375, 0, Johto_RadioTower4F_Text_83D87E2, Johto_RadioTower4F_Text_83D88EB
	msgbox Johto_RadioTower4F_Text_83D88F3, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower4F object 3 (local id 3, gfx 0x11) at 17,10
Johto_RadioTower4F_EventScript_83D8742::
	lock
	faceplayer
	msgbox Johto_RadioTower4F_Text_83D894C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RadioTower4F object 1 (local id 1, gfx 0x3EB4) at 12,13
Johto_RadioTower4F_EventScript_83D874F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4149, 0, Johto_RadioTower4F_Text_83D8C98, Johto_RadioTower4F_Text_83D8CD8
	msgbox Johto_RadioTower4F_Text_83D8CFF, MSGBOX_AUTOCLOSE
	end

@ Johto_RadioTower4F object 4 (local id 4, gfx 0x8) at 28,12
Johto_RadioTower4F_EventScript_83D8766::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500A, 6, Johto_RadioTower4F_EventScript_83D87A8
	goto_if_eq VAR_QZ_0x500A, 7, Johto_RadioTower4F_EventScript_83D87A8
	goto_if_eq VAR_QZ_0x500A, 8, Johto_RadioTower4F_EventScript_83D87A8
	goto_if_eq VAR_QZ_0x500A, 9, Johto_RadioTower4F_EventScript_83D87A8
	goto_if_unset FLAG_QZ_0x1041, Johto_RadioTower4F_EventScript_83D87B3
	msgbox Johto_RadioTower4F_Text_83D8A58, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower4F_EventScript_83D87A8::
	msgbox Johto_RadioTower4F_Text_83D89A1, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RadioTower4F_EventScript_83D87B3::
	msgbox Johto_RadioTower4F_Text_83D89E3, MSGBOX_DEFAULT
	closemessage
	setorcopyvar VAR_0x8000, ITEM_BRIGHT_POWDER
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_RadioTower4F_Text_83D8A27, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1041
	closemessage
	release
	end

@ Johto_RadioTower4F trigger at 28,5 (VAR_QZ_0x500A == 10)
@ Johto_RadioTower4F trigger at 29,6 (VAR_QZ_0x500A == 10)
Johto_RadioTower4F_EventScript_83D9272::
	clearflag FLAG_QZ_0x1098
	setvar VAR_QZ_0x500A, 0xB
	end

Johto_RadioTower4F_Text_83D87E2::
	.string "PROTON: Now wait just one second!\p"
	.string "Aren’t you the one who got in our\n"
	.string "way at SLOWPOKE WELL?\p"
	.string "I see... You’re doing all this\n"
	.string "because you want to make me angry.\l"
	.string "Be careful!\p"
	.string "But since you seem to wish it, I’ll\n"
	.string "show you the full extent of a TEAM\l"
	.string "ROCKET EXECUTIVE’s wrath!$"

Johto_RadioTower4F_Text_83D88EB::
	.string "Phew...$"

Johto_RadioTower4F_Text_83D88F3::
	.string "PROTON: You may have won this\n"
	.string "time... But all you did was make\p"
	.string "TEAM ROCKET’s wrath grow.$"

Johto_RadioTower4F_Text_83D894C::
	.string "I listened to the radio while I was\n"
	.string "at the RUINS.\p"
	.string "I heard a strange broadcast there.$"

Johto_RadioTower4F_Text_83D89A1::
	.string "MARY: Why? Why do I have to suffer\n"
	.string "through this?\p"
	.string "MEOWTH, help me!$"

Johto_RadioTower4F_Text_83D89E3::
	.string "MARY: Oh! You’re my little savior!\p"
	.string "Will you take this as my thanks?$"

Johto_RadioTower4F_Text_83D8A27::
	.string "MARY: It’s good for dodging the\n"
	.string "enemy’s attacks.$"

Johto_RadioTower4F_Text_83D8A58::
	.string "MARY: Please tune into me on PROF.\n"
	.string "OAK’S POKéMON TALK show!$"

Johto_RadioTower4F_Text_83D8AA4::
	.string "You plan to rescue the DIRECTOR?\p"
	.string "That won’t be possible because I’m\n"
	.string "going to beat you!$"

Johto_RadioTower4F_Text_83D8AFB::
	.string "No! Unbelievable!$"

Johto_RadioTower4F_Text_83D8B0D::
	.string "I don’t believe it! I was beaten!$"

Johto_RadioTower4F_Text_83D8BF1::
	.string "Don’t I think POKéMON are cute?\p"
	.string "I’ll think my POKéMON are cute--\p"
	.string "after they beat yours!$"

Johto_RadioTower4F_Text_83D8C49::
	.string "Oh, no! They’re so useless!$"

Johto_RadioTower4F_Text_83D8C65::
	.string "I love my beautiful self!\p"
	.string "Who cares about POKéMON?$"

Johto_RadioTower4F_Text_83D8C98::
	.string "Most excellent.\p"
	.string "This RADIO TOWER will fulfill our\n"
	.string "grand design.$"

Johto_RadioTower4F_Text_83D8CD8::
	.string "Hmmm…\p"
	.string "All grand plans come with snags.$"

Johto_RadioTower4F_Text_83D8CFF::
	.string "Do you honestly believe you can stop\n"
	.string "TEAM ROCKET?$"

Johto_RadioTower4F_Text_83D8D31::
	.string "4F PRODUCTION$"

Johto_RadioTower4F_Text_83D8D3F::
	.string "4F STUDIO 2$"
