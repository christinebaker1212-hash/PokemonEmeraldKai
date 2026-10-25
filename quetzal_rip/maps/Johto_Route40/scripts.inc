Johto_Route40_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route40_EventScript_83F3D29
	.byte 0

Johto_Route40_EventScript_83F3D29::
	call Quetzal_EventScript_840EA62
	goto_if_ge VAR_QZ_0x500D, 5, Johto_Route40_EventScript_83F3D3A
	end

Johto_Route40_EventScript_83F3D3A::
	setweather 0
	end

@ Johto_Route40 object 8 (local id 8, gfx 0x3EB9) at 13,26
Johto_Route40_EventScript_83F3D3E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4161, 0, Johto_Route40_Text_83F3D55, Johto_Route40_Text_83F3D94
	msgbox Johto_Route40_Text_83F3DA7, MSGBOX_AUTOCLOSE
	end

@ Johto_Route40 object 10 (local id 10, gfx 0x3EB9) at 22,50
Johto_Route40_EventScript_83F3DD8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4167, 0, Johto_Route40_Text_83F3DEF, Johto_Route40_Text_83F3E29
	msgbox Johto_Route40_Text_83F3E38, MSGBOX_AUTOCLOSE
	end

@ Johto_Route40 object 5 (local id 5, gfx 0x3EB8) at 11,41
Johto_Route40_EventScript_83F3E6B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4171, 0, Johto_Route40_Text_83F3E82, Johto_Route40_Text_83F3EBD
	msgbox Johto_Route40_Text_83F3ECE, MSGBOX_AUTOCLOSE
	end

@ Johto_Route40 object 9 (local id 9, gfx 0x3EB8) at 26,28
Johto_Route40_EventScript_83F3EFB::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4525, 0, Johto_Route40_Text_83F3F12, Johto_Route40_Text_83F3F49
	msgbox Johto_Route40_Text_83F3F61, MSGBOX_AUTOCLOSE
	end

@ Johto_Route40 object 7 (local id 7, gfx 0x14) at 21,18
Johto_Route40_EventScript_83F3F94::
	lock
	faceplayer
	msgbox Johto_Route40_Text_83F3FA1, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route40 object 6 (local id 6, gfx 0x3E87) at 25,21
Johto_Route40_EventScript_83F406C::
	lock
	faceplayer
	msgbox Johto_Route40_Text_83F4079, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route40 object 11 (local id 11, gfx 0x2F) at 19,13
Johto_Route40_EventScript_83F42D3::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1047, Johto_Route40_EventScript_83F4393
	msgbox Johto_Route40_Text_83F411A, MSGBOX_DEFAULT
	closemessage
	multichoice 0x14, 0, 0x4A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_Route40_EventScript_83F4385
	goto_if_eq VAR_0x8000, 1, Johto_Route40_EventScript_83F4358
	goto_if_eq VAR_0x8000, 2, Johto_Route40_EventScript_83F4385
	goto_if_eq VAR_0x8000, 3, Johto_Route40_EventScript_83F4385
	goto_if_eq VAR_0x8000, 4, Johto_Route40_EventScript_83F4385
	goto_if_eq VAR_0x8000, 5, Johto_Route40_EventScript_83F4385
	goto_if_eq VAR_0x8000, 6, Johto_Route40_EventScript_83F4385
	goto_if_eq VAR_0x8000, 7, Johto_Route40_EventScript_83F4390
	goto_if_eq VAR_0x8000, 127, Johto_Route40_EventScript_83F4390
	closemessage
	release
	end

Johto_Route40_EventScript_83F4358::
	msgbox Johto_Route40_Text_83F4189, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1B2
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1047
	msgbox Johto_Route40_Text_83F41CC, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route40_EventScript_83F4385::
	msgbox Johto_Route40_Text_83F42C5, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route40_EventScript_83F4390::
	closemessage
	release
	end

Johto_Route40_EventScript_83F4393::
	msgbox Johto_Route40_Text_83F423D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route40_Text_83F3D55::
	.string "You have to warm up before\n"
	.string "going into the water.\p"
	.string "That’s basic.$"

Johto_Route40_Text_83F3D94::
	.string "OK! Uncle! I give!$"

Johto_Route40_Text_83F3DA7::
	.string "CIANWOOD CITY is a good\n"
	.string "distance away from here.$"

Johto_Route40_Text_83F3DEF::
	.string "Hey, you’re young and fit!\p"
	.string "Don’t ride your POKéMON!\n"
	.string "Swim!$"

Johto_Route40_Text_83F3E29::
	.string "Uh-oh. I lost…$"

Johto_Route40_Text_83F3E38::
	.string "Swimming exercises your\n"
	.string "entire body. It’s healthy.$"

Johto_Route40_Text_83F3E82::
	.string "Are you going to CIANWOOD?\p"
	.string "How about a quick battle\n"
	.string "first?$"

Johto_Route40_Text_83F3EBD::
	.string "I lost that one!$"

Johto_Route40_Text_83F3ECE::
	.string "I’d say I’m a better swimmer\n"
	.string "than you. Yeah!$"

Johto_Route40_Text_83F3F12::
	.string "No inner tube for me.\p"
	.string "I’m hanging on to a sea\n"
	.string "POKéMON!$"

Johto_Route40_Text_83F3F49::
	.string "Ooh, I’m feeling dizzy!$"

Johto_Route40_Text_83F3F61::
	.string "While I float like this,\n"
	.string "the waves carry me along.$"

Johto_Route40_Text_83F3FA1::
	.string "Although you can’t see it from here,\n"
	.string "CIANWOOD is across the sea.$"

Johto_Route40_Text_83F4079::
	.string "Have you gone to the TRAINER\n"
	.string "HILL?\p"
	.string "I think a lot of tough trainers\n"
	.string "have gathered there already.\p"
	.string "But since you have so many BADGES\n"
	.string "you shouldn’t do badly at all.$"

Johto_Route40_Text_83F411A::
	.string "MONICA: Glad to meet you.\n"
	.string "I’m MONICA of the WEEKDAY FAMILY.\p"
	.string "I bet you can’t guess my favorite\n"
	.string "day of the week!$"

Johto_Route40_Text_83F4189::
	.string "That’s right!\p"
	.string "As a token of our friendship,\n"
	.string "I have a gift for you!$"

Johto_Route40_Text_83F41CC::
	.string "MONICA: It’s an item that raises\n"
	.string "the power of FLYING-type moves.\p"
	.string "You should equip a bird\n"
	.string "POKéMON with that item.$"

Johto_Route40_Text_83F423D::
	.string "MONICA: My brothers and\n"
	.string "sisters are all over the place.\p"
	.string "See if you could find\n"
	.string "them all!$"

Johto_Route40_Text_83F42C5::
	.string "Not that one.$"
