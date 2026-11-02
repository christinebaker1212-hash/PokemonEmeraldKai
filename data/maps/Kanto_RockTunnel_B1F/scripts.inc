Kanto_RockTunnel_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_RockTunnel_B1F object 3 (local id 3, gfx 0x3F31) at 11,15
Kanto_RockTunnel_B1F_EventScript_8412B48::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8350, 0, Kanto_RockTunnel_B1F_Text_8429D1A, Kanto_RockTunnel_B1F_Text_8429D7C
	msgbox Kanto_RockTunnel_B1F_Text_8429D91, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 7 (local id 7, gfx 0x3F31) at 15,33
Kanto_RockTunnel_B1F_EventScript_8412B5F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8351, 0, Kanto_RockTunnel_B1F_Text_8429F70, Kanto_RockTunnel_B1F_Text_8429FA0
	msgbox Kanto_RockTunnel_B1F_Text_8429FAC, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 2 (local id 2, gfx 0x3F0C) at 5,11
Kanto_RockTunnel_B1F_EventScript_8412B76::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8381, 0, Kanto_RockTunnel_B1F_Text_8429DA4, Kanto_RockTunnel_B1F_Text_8429DC3
	msgbox Kanto_RockTunnel_B1F_Text_8429DD5, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 4 (local id 4, gfx 0x3F0C) at 36,11
Kanto_RockTunnel_B1F_EventScript_8412B8D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8382, 0, Kanto_RockTunnel_B1F_Text_8429EEC, Kanto_RockTunnel_B1F_Text_8429F19
	msgbox Kanto_RockTunnel_B1F_Text_8429F3D, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 5 (local id 5, gfx 0x3F0C) at 41,5
Kanto_RockTunnel_B1F_EventScript_8412BA4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8383, 0, Kanto_RockTunnel_B1F_Text_8429FD9, Kanto_RockTunnel_B1F_Text_8429FF5
	msgbox Kanto_RockTunnel_B1F_Text_842A001, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 1 (local id 1, gfx 0x3F4B) at 2,6
Kanto_RockTunnel_B1F_EventScript_8412BBB::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8356, 0, Kanto_RockTunnel_B1F_Text_8429DFD, Kanto_RockTunnel_B1F_Text_8429E22
	msgbox Kanto_RockTunnel_B1F_Text_8429E3A, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 6 (local id 6, gfx 0x3F4B) at 23,23
Kanto_RockTunnel_B1F_EventScript_8412BD2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8357, 0, Kanto_RockTunnel_B1F_Text_8429E67, Kanto_RockTunnel_B1F_Text_8429E8E
	msgbox Kanto_RockTunnel_B1F_Text_8429EA1, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 8 (local id 8, gfx 0x3F4B) at 29,35
Kanto_RockTunnel_B1F_EventScript_8412BE9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8358, 0, Kanto_RockTunnel_B1F_Text_842A02B, Kanto_RockTunnel_B1F_Text_842A05A
	msgbox Kanto_RockTunnel_B1F_Text_842A06F, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_B1F object 10 (local id 10, gfx 0x3F0E) at 2,37
Kanto_RockTunnel_B1F_EventScript_846C017::
	setorcopyvar VAR_0x8000, ITEM_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RockTunnel_B1F object 11 (local id 11, gfx 0x3F0E) at 9,2
Kanto_RockTunnel_B1F_EventScript_846C024::
	setorcopyvar VAR_0x8000, ITEM_MAX_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RockTunnel_B1F object 9 (local id 9, gfx 0x3F5C) at 2,29
Kanto_RockTunnel_B1F_EventScript_8471C66::
	@ Moemon Kai: FireRed's ROCK SLIDE tutor (tools/quetzal_port/fix_move_tutors.py), on Kai's move tutors
	lock
	faceplayer
	goto_if_set 0x21EE, Kanto_RockTunnel_B1F_EventScript_8471C66_Taught
	msgbox Kanto_RockTunnel_B1F_EventScript_8471C66_Teach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_RockTunnel_B1F_EventScript_8471C66_Declined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_RockTunnel_B1F_EventScript_8471C66_Declined
	msgbox Kanto_RockTunnel_B1F_EventScript_8471C66_WhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_ROCK_SLIDE
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Kanto_RockTunnel_B1F_EventScript_8471C66_Declined
	setflag 0x21EE
	goto Kanto_RockTunnel_B1F_EventScript_8471C66_Taught
	end

Kanto_RockTunnel_B1F_EventScript_8471C66_Declined:
	msgbox Kanto_RockTunnel_B1F_EventScript_8471C66_DeclinedText, MSGBOX_DEFAULT
	release
	end

Kanto_RockTunnel_B1F_EventScript_8471C66_Taught:
	msgbox Kanto_RockTunnel_B1F_EventScript_8471C66_TaughtText, MSGBOX_DEFAULT
	release
	end

Kanto_RockTunnel_B1F_EventScript_8471C66_Teach:
	.string "When you're up on a rocky mountain\n"
	.string "like this, rockslides are a threat.\p"
	.string "Can you imagine?\n"
	.string "Boulders tumbling down on you?\p"
	.string "That'd be, like, waaaaaaaaaaah!\n"
	.string "Total terror!\p"
	.string "You don't seem to be scared.\n"
	.string "Want to try using ROCK SLIDE?$"

Kanto_RockTunnel_B1F_EventScript_8471C66_DeclinedText:
	.string "Oh, so you are scared after all.$"

Kanto_RockTunnel_B1F_EventScript_8471C66_WhichMon:
	.string "Which MOéMON should I teach\n"
	.string "ROCK SLIDE?$"

Kanto_RockTunnel_B1F_EventScript_8471C66_TaughtText:
	.string "It might be scary to use it in this\n"
	.string "tunnel…$"

Kanto_RockTunnel_B1F_Text_8429D1A::
	.string "Do you know how you can avoid\n"
	.string "getting lost in the mountains?\p"
	.string "You can bend twigs as trail\n"
	.string "markers.$"

Kanto_RockTunnel_B1F_Text_8429D7C::
	.string "Ohhh!\n"
	.string "I did my best!$"

Kanto_RockTunnel_B1F_Text_8429D91::
	.string "I want to go home!$"

Kanto_RockTunnel_B1F_Text_8429DA4::
	.string "Hahaha!\n"
	.string "Can you beat my power?$"

Kanto_RockTunnel_B1F_Text_8429DC3::
	.string "Oops!\n"
	.string "Outmuscled!$"

Kanto_RockTunnel_B1F_Text_8429DD5::
	.string "I go for power because I hate\n"
	.string "thinking.$"

Kanto_RockTunnel_B1F_Text_8429DFD::
	.string "You have a MOéDEX?\n"
	.string "I want one, too.$"

Kanto_RockTunnel_B1F_Text_8429E22::
	.string "Shoot!\n"
	.string "I am so jealous!$"

Kanto_RockTunnel_B1F_Text_8429E3A::
	.string "When you finish your MOéDEX,\n"
	.string "can I have it?$"

Kanto_RockTunnel_B1F_Text_8429E67::
	.string "Um… Do you know about MOéMON\n"
	.string "cosplay?$"

Kanto_RockTunnel_B1F_Text_8429E8E::
	.string "Well, that’s that.$"

Kanto_RockTunnel_B1F_Text_8429EA1::
	.string "MOéMON cosplay is dressing up as\n"
	.string "MOéMON for fun.\p"
	.string "CLEFAIRY is a favorite.$"

Kanto_RockTunnel_B1F_Text_8429EEC::
	.string "My MOéMON techniques will leave\n"
	.string "you crying!$"

Kanto_RockTunnel_B1F_Text_8429F19::
	.string "I give!\n"
	.string "You’re a better technician!$"

Kanto_RockTunnel_B1F_Text_8429F3D::
	.string "In mountains, you’ll often find\n"
	.string "ROCK-type MOéMON.$"

Kanto_RockTunnel_B1F_Text_8429F70::
	.string "I don’t often come here, but I will\n"
	.string "battle you.$"

Kanto_RockTunnel_B1F_Text_8429FA0::
	.string "Oh!\n"
	.string "I lost!$"

Kanto_RockTunnel_B1F_Text_8429FAC::
	.string "I like tiny MOéMON.\n"
	.string "Big ones are too scary!$"

Kanto_RockTunnel_B1F_Text_8429FD9::
	.string "Hit me with your best shot!$"

Kanto_RockTunnel_B1F_Text_8429FF5::
	.string "Fired away!$"

Kanto_RockTunnel_B1F_Text_842A001::
	.string "I’ll raise my MOéMON to beat\n"
	.string "yours, kid.$"

Kanto_RockTunnel_B1F_Text_842A02B::
	.string "I draw illustrations of MOéMON\n"
	.string "when I’m home.$"

Kanto_RockTunnel_B1F_Text_842A05A::
	.string "Whew…\n"
	.string "I’m exhausted…$"

Kanto_RockTunnel_B1F_Text_842A06F::
	.string "I’m an artist, not a fighter.\n"
	.string "I’ll go home to draw.$"
