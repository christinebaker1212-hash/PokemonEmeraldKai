Johto_Route35GoldenrodGate_MapScripts::
	.byte 0

@ Johto_Route35GoldenrodGate object 1 (local id 1, gfx 0x3EA9) at 3,5
@ Johto_Route35GoldenrodGate object 2 (local id 2, gfx 0x3E91) at 18,15
Johto_Route35GoldenrodGate_EventScript_83D0E0D::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5018, 3, Johto_Route35GoldenrodGate_EventScript_83D0EAF
	goto_if_eq VAR_QZ_0x5018, 2, Johto_Route35GoldenrodGate_EventScript_83D0E7F
	goto_if_eq VAR_QZ_0x5018, 1, Johto_Route35GoldenrodGate_EventScript_83D0E74
	call Quetzal_EventScript_8361A4D
	msgbox Johto_Route35GoldenrodGate_Text_83D0EE8, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_Route35GoldenrodGate_EventScript_83D0E69
	closemessage
	msgbox Johto_Route35GoldenrodGate_Text_83D0F56, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, 0x15
	bufferspeciesname 0, SPECIES_SPEAROW
	qz_givestorymon 1
	call Quetzal_EventScript_840181E
	setvar VAR_QZ_0x5018, 1
	release
	end

Johto_Route35GoldenrodGate_EventScript_83D0E69::
	msgbox Johto_Route35GoldenrodGate_Text_83D10AD, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route35GoldenrodGate_EventScript_83D0E74::
	msgbox Johto_Route35GoldenrodGate_Text_83D0FE8, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route35GoldenrodGate_EventScript_83D0E7F::
	msgbox Johto_Route35GoldenrodGate_Text_83D10C3, MSGBOX_DEFAULT
	closemessage
	setorcopyvar VAR_0x8000, ITEM_HP_UP
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_Route35GoldenrodGate_Text_83D1111, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5018, 3
	release
	end

Johto_Route35GoldenrodGate_EventScript_83D0EAF::
	msgbox Johto_Route35GoldenrodGate_Text_83D1111, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route35GoldenrodGate object 3 (local id 3, gfx 0x18) at 5,3
Johto_Route35GoldenrodGate_EventScript_83D0EBA::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102E, Johto_Route35GoldenrodGate_EventScript_83D0ED0
	msgbox Johto_Route35GoldenrodGate_Text_83D1140, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route35GoldenrodGate_EventScript_83D0ED0::
	msgbox Johto_Route35GoldenrodGate_Text_83D11CF, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route35GoldenrodGate object 4 (local id 4, gfx 0x3E95) at 9,4
Johto_Route35GoldenrodGate_EventScript_83D0EDB::
	lock
	faceplayer
	msgbox Johto_Route35GoldenrodGate_Text_83D1202, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route35GoldenrodGate_Text_83D0EE8::
	.string "Excuse me, kid! Can you do a guy\n"
	.string "a favor?\p"
	.string "Can you take this POKéMON with MAIL\n"
	.string "to my friend?\p"
	.string "He’s on ROUTE 31.$"

Johto_Route35GoldenrodGate_Text_83D0F56::
	.string "You will? Perfect! Thanks, kid!\p"
	.string "My pal’s a chubby guy who snoozes\n"
	.string "all the time.\p"
	.string "You’ll recognize him right away!$"

Johto_Route35GoldenrodGate_Text_83D0FE8::
	.string "Make sure to put it at the front of\n"
	.string "your team when you find my friend!\p"
	.string "ROUTE 31!\p"
	.string "Oh, yeah. There was a weird tree\n"
	.string "blocking the road.\p"
	.string "I wonder if it’s been cleared?$"

Johto_Route35GoldenrodGate_Text_83D10AD::
	.string "Oh… Never mind, then…$"

Johto_Route35GoldenrodGate_Text_83D10C3::
	.string "Thanks, kid! You made the delivery\n"
	.string "for me!\p"
	.string "Here’s something for your trouble!$"

Johto_Route35GoldenrodGate_Text_83D1111::
	.string "My pal was snoozing, right? Heh,\n"
	.string "what’d I say?$"

Johto_Route35GoldenrodGate_Text_83D1140::
	.string "A strange tree is blocking the road.\p"
	.string "It wriggles around if you talk to it.\p"
	.string "I heard it became wild when someone\n"
	.string "watered it with a SQUIRTBOTTLE.$"

Johto_Route35GoldenrodGate_Text_83D11CF::
	.string "I like the POKéMON Lullaby they play\n"
	.string "on the radio.$"

Johto_Route35GoldenrodGate_Text_83D1202::
	.string "I wonder how many kinds of POKéMON\n"
	.string "there are in the world.\p"
	.string "Three years ago, PROF.OAK said that\n"
	.string "there were 150 different kinds.\p"
	.string "But recently I heard there’s\n"
	.string "over 400!$"
