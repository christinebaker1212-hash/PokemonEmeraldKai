Johto_RuinsOfAlphOutside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_RuinsOfAlphOutside_EventScript_83F7ABC
	.byte 0

Johto_RuinsOfAlphOutside_EventScript_83F7ABC::
	call Quetzal_EventScript_840EA62
	end

@ Johto_RuinsOfAlphOutside object 2 (local id 2, gfx 0x3EB7) at 9,23
Johto_RuinsOfAlphOutside_EventScript_83F7AD6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4258, 0, Johto_RuinsOfAlphOutside_Text_83F7C83, Johto_RuinsOfAlphOutside_Text_83F7CB6
	msgbox Johto_RuinsOfAlphOutside_Text_83F7CEF, MSGBOX_AUTOCLOSE
	end

@ Johto_RuinsOfAlphOutside object 1 (local id 1, gfx 0x25) at 12,37
Johto_RuinsOfAlphOutside_EventScript_83F7AED::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4315, 0, Johto_RuinsOfAlphOutside_Text_83F7D8C, Johto_RuinsOfAlphOutside_Text_83F7DAB
	msgbox Johto_RuinsOfAlphOutside_Text_83F7DAD, MSGBOX_AUTOCLOSE
	end

@ Johto_RuinsOfAlphOutside object 3 (local id 3, gfx 0x23) at 25,21
Johto_RuinsOfAlphOutside_EventScript_83F7B04::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphOutside_Text_83F7EE2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RuinsOfAlphOutside object 6 (local id 6, gfx 0x3E96) at 25,20
Johto_RuinsOfAlphOutside_EventScript_83F7B11::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphOutside_Text_83F7F26, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RuinsOfAlphOutside object 4 (local id 4, gfx 0x3E95) at 28,32
Johto_RuinsOfAlphOutside_EventScript_83F7B1E::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphOutside_Text_83F7E3F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RuinsOfAlphOutside object 5 (local id 5, gfx 0x3E95) at 29,32
Johto_RuinsOfAlphOutside_EventScript_83F7B2B::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphOutside_Text_83F7EBA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RuinsOfAlphOutside sign / bg event at 31,24
Johto_RuinsOfAlphOutside_EventScript_83F7B38::
	lock
	msgbox Johto_RuinsOfAlphOutside_Text_83F7DDF, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RuinsOfAlphOutside sign / bg event at 38,16
@ Johto_RuinsOfAlphOutside sign / bg event at 37,16
Johto_RuinsOfAlphOutside_EventScript_83F7B44::
	lock
	msgbox Johto_RuinsOfAlphOutside_Text_83F7DFE, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RuinsOfAlphOutside sign / bg event at 30,11
Johto_RuinsOfAlphOutside_EventScript_83F7B50::
	lock
	msgbox Johto_RuinsOfAlphOutside_Text_83F7DC3, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_RuinsOfAlphOutside object 18 (local id 18, gfx 0x510) at 34,11
Johto_RuinsOfAlphOutside_EventScript_840EB76::
	lock
	goto_if_unset FLAG_QZ_0x11CA, Johto_RuinsOfAlphOutside_EventScript_840EB9F
	faceplayer
	special QzSetupUnownStaticEncounter
	waitmoncry
	special CreateEnemyEventMon
	setflag FLAG_QZ_0xAD9
	dowildbattle
	clearflag FLAG_QZ_0xAD9
	special QzClearStaticFlagAfterWildBattle
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_840EBA9
	release
	end

Johto_RuinsOfAlphOutside_EventScript_840EB9F::
	msgbox Johto_RuinsOfAlphOutside_Text_840ED22, MSGBOX_DEFAULT
	release
	end

Johto_RuinsOfAlphOutside_Text_83F7C83::
	.string "What do you want? I’m studying--\n"
	.string "don’t disturb me!$"

Johto_RuinsOfAlphOutside_Text_83F7CB6::
	.string "Sorry…\n"
	.string "I’m frustrated by our lack of\p"
	.string "real understanding…$"

Johto_RuinsOfAlphOutside_Text_83F7CEF::
	.string "The RUINS are from about 1500\n"
	.string "years ago.\p"
	.string "Nobody knows who built them.\p"
	.string "It’s also not known if the POKéMON\n"
	.string "statues have any meaning.\p"
	.string "It’s all one big mystery…$"

Johto_RuinsOfAlphOutside_Text_83F7D8C::
	.string "Hmmm… This is a strange place.$"

Johto_RuinsOfAlphOutside_Text_83F7DAB::
	.string "…$"

Johto_RuinsOfAlphOutside_Text_83F7DAD::
	.string "I like thinking here.$"

Johto_RuinsOfAlphOutside_Text_83F7DC3::
	.string "MYSTERY STONE PANEL CHAMBER$"

Johto_RuinsOfAlphOutside_Text_83F7DDF::
	.string "RUINS OF ALPH\n"
	.string "VISITORS WELCOME$"

Johto_RuinsOfAlphOutside_Text_83F7DFE::
	.string "RUINS OF ALPH RESEARCH CENTER\p"
	.string "THE AUTHORITY ON THE\n"
	.string "RUINS OF ALPH$"

Johto_RuinsOfAlphOutside_Text_83F7E3F::
	.string "While exploring the RUINS, we\n"
	.string "suddenly noticed an odd presence.\p"
	.string "We all got scared and ran away.\n"
	.string "You should be careful too.$"

Johto_RuinsOfAlphOutside_Text_83F7EBA::
	.string "The RUINS hide a huge secret!\p"
	.string "…I think…$"

Johto_RuinsOfAlphOutside_Text_83F7EE2::
	.string "There are many kinds of UNOWN, so\n"
	.string "we use them for our secret codes.$"

Johto_RuinsOfAlphOutside_Text_83F7F26::
	.string "A… H… E… A… D… Hmm…\p"
	.string "What?\p"
	.string "I’m decoding this message!$"

Johto_RuinsOfAlphOutside_Text_840ED22::
	.string "DUDUNSPARCE is drilling through the wall.\p"
	.string "It seems determined to finish its tunnel.$"
