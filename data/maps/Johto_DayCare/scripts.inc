Johto_DayCare_MapScripts::
	.byte 0

@ Johto_DayCare sign / bg event at 2,1
Johto_DayCare_EventScript_83F03DC::
	msgbox Johto_DayCare_Text_83F0400, MSGBOX_SIGN
	end

@ Johto_DayCare sign / bg event at 3,1
Johto_DayCare_EventScript_83F03E5::
	msgbox Johto_DayCare_Text_83F047C, MSGBOX_SIGN
	end

@ Johto_DayCare sign / bg event at 4,1
Johto_DayCare_EventScript_83F03EE::
	msgbox Johto_DayCare_Text_83F04FA, MSGBOX_SIGN
	end

@ Johto_DayCare sign / bg event at 5,1
Johto_DayCare_EventScript_83F03F7::
	msgbox Johto_DayCare_Text_83F05BD, MSGBOX_SIGN
	end

@ Johto_DayCare object 2 (local id 2, gfx 0x3E8E) at 10,3
Johto_DayCare_EventScript_83F0695::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1069, Johto_DayCare_EventScript_83F06FC
	call Quetzal_EventScript_8361A4D
	msgbox Johto_DayCare_Text_83F0707, MSGBOX_DEFAULT
	special QzIsCurrentPcBoxFull
	goto_if_eq VAR_RESULT, 1, Johto_DayCare_EventScript_83F06F1
	random 7
	addvar VAR_RESULT, 1
	qz_giveoddegg VAR_RESULT
	goto_if_eq VAR_RESULT, 2, Johto_DayCare_EventScript_83F06F1
	goto Johto_DayCare_EventScript_83F06D6

Johto_DayCare_EventScript_83F06D6::
	msgbox Johto_DayCare_Text_83F07E5, MSGBOX_DEFAULT
	closemessage
	delay 0x1E
	setflag FLAG_QZ_0x1069
	playfanfare MUS_OBTAIN_ITEM
	message Johto_DayCare_Text_83F07CC
	waitfanfare
	closemessage
	release
	end

Johto_DayCare_EventScript_83F06F1::
	msgbox Johto_DayCare_Text_83F0796, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DayCare_EventScript_83F06FC::
	msgbox Johto_DayCare_Text_83F082A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_DayCare_Text_83F0400::
	.string "A dictionary!\p"
	.string "{PLAYER} opened the dictionary to\n"
	.string "a random page.\p"
	.string "’BABE’\n"
	.string "A noun, meaning infant or baby.\p"
	.string "{PLAYER} put away the dictionary.$"

Johto_DayCare_Text_83F047C::
	.string "A book about POKéMON!\p"
	.string "{PLAYER} flipped through\n"
	.string "the book.\p"
	.string "It must have the word\n"
	.string "POKéMON written on every page.\p"
	.string "{PLAYER} put away the book.$"

Johto_DayCare_Text_83F04FA::
	.string "A book of word assocations!\p"
	.string "{PLAYER} flipped through\n"
	.string "the book.\p"
	.string "PICHU -> CUTE\l"
	.string "CLEFFA -> CUTE\l"
	.string "IGGLYBUFF -> SONG\l"
	.string "TYROGUE -> FIGHTING\l"
	.string "SMOOCHUM -> ICE\l"
	.string "ELEKID -> ELECTRIC\l"
	.string "MAGBY -> FIRE\p"
	.string "{PLAYER} put away the book.$"

Johto_DayCare_Text_83F05BD::
	.string "A book of POKéMON abilities!\p"
	.string "{PLAYER} flipped through\n"
	.string "the book.\p"
	.string "PICHU -> STATIC\l"
	.string "CLEFFA -> CUTE CHARM\l"
	.string "IGGLYBUFF -> CUTE CHARM\l"
	.string "TYROGUE -> GUTS\l"
	.string "SMOOCHUM -> OBLIVIOUS\l"
	.string "ELEKID -> STATIC\l"
	.string "MAGBY -> FLAME BODY\p"
	.string "{PLAYER} put away the book.$"

Johto_DayCare_Text_83F0707::
	.string "I’m the DAY-CARE couple’s son.\p"
	.string "An ODD EGG was found with a TRAINER’s\n"
	.string "POKéMON, but they didn’t want it...\p"
	.string "I’d like you to take good care of it.$"

Johto_DayCare_Text_83F0796::
	.string "It looks like you can’t carry another\n"
	.string "POKéMON anyway.$"

Johto_DayCare_Text_83F07CC::
	.string "{PLAYER} Received the ODD EGG.$"

Johto_DayCare_Text_83F07E5::
	.string "I knew you were the right trainer\n"
	.string "for the job. Take good care of it!$"

Johto_DayCare_Text_83F082A::
	.string "We see a lot of those eggs around\n"
	.string "here.\p"
	.string "Something was different about that\n"
	.string "one, though.$"
