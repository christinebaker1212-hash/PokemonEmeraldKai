Johto_Route37_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route37_EventScript_83F2254
	.byte 0

@ Johto_Route37 trigger at 17,11 (VAR_QZ_0x500B == 0)
@ Johto_Route37 trigger at 18,11 (VAR_QZ_0x500B == 0)
Johto_Route37_EventScript_83CD1E0::
	setflag FLAG_QZ_0x11BF
	setvar VAR_QZ_0x500B, 1
	end

Johto_Route37_EventScript_83F2254::
	end

@ Johto_Route37 object 1 (local id 1, gfx 0x6) at 13,29
Johto_Route37_EventScript_83F2255::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4578, 0, Johto_Route37_Text_83F2381, Johto_Route37_Text_83F23A7, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_Route37_Text_83F23CD, MSGBOX_DEFAULT
	release
	end

@ Johto_Route37 object 2 (local id 2, gfx 0x6) at 14,29
Johto_Route37_EventScript_83F2274::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_4578, 0, Johto_Route37_Text_83F2409, Johto_Route37_Text_83F242F, Quetzal_Text_8383BB5
	special GetPlayerBigGuyGirlString
	msgbox Johto_Route37_Text_83F2455, MSGBOX_DEFAULT
	release
	end

@ Johto_Route37 object 3 (local id 3, gfx 0x25) at 28,27
Johto_Route37_EventScript_83F2293::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4715, 0, Johto_Route37_Text_83F2486, Johto_Route37_Text_83F24D4
	msgbox Johto_Route37_Text_83F24EF, MSGBOX_AUTOCLOSE
	end

@ Johto_Route37 object 4 (local id 4, gfx 0xB) at 20,26
Johto_Route37_EventScript_83F22AA::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1044, Johto_Route37_EventScript_83F236A
	msgbox Johto_Route37_Text_83F2539, MSGBOX_DEFAULT
	closemessage
	multichoice 0x14, 0, 0x4A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_Route37_EventScript_83F232F
	goto_if_eq VAR_0x8000, 1, Johto_Route37_EventScript_83F235C
	goto_if_eq VAR_0x8000, 2, Johto_Route37_EventScript_83F235C
	goto_if_eq VAR_0x8000, 3, Johto_Route37_EventScript_83F235C
	goto_if_eq VAR_0x8000, 4, Johto_Route37_EventScript_83F235C
	goto_if_eq VAR_0x8000, 5, Johto_Route37_EventScript_83F235C
	goto_if_eq VAR_0x8000, 6, Johto_Route37_EventScript_83F235C
	goto_if_eq VAR_0x8000, 7, Johto_Route37_EventScript_83F2367
	goto_if_eq VAR_0x8000, 127, Johto_Route37_EventScript_83F2367
	closemessage
	release
	end

Johto_Route37_EventScript_83F232F::
	msgbox Johto_Route37_Text_83F25C9, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_MAGNET
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1044
	msgbox Johto_Route37_Text_83F25F3, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route37_EventScript_83F235C::
	msgbox Quetzal_Text_83ED204, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route37_EventScript_83F2367::
	closemessage
	release
	end

Johto_Route37_EventScript_83F236A::
	msgbox Johto_Route37_Text_83F2697, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route37 sign / bg event at 20,13
Johto_Route37_EventScript_83F2375::
	lock
	msgbox Johto_Route37_Text_83F2732, MSGBOX_SIGN
	closemessage
	release
	end

Johto_Route37_Text_83F2381::
	.string "ANN: ANNE and I are in this\n"
	.string "together!$"

Johto_Route37_Text_83F23A7::
	.string "ANN & ANNE: Nnn… A little too\n"
	.string "strong.$"

Johto_Route37_Text_83F23CD::
	.string "ANN: I can tell what my sister and\n"
	.string "my POKéMON are thinking.$"

Johto_Route37_Text_83F2409::
	.string "ANNE: ANN and I are in this\n"
	.string "together!$"

Johto_Route37_Text_83F242F::
	.string "ANN & ANNE: Nnn… A little too\n"
	.string "strong.$"

Johto_Route37_Text_83F2455::
	.string "ANNE: We share the same feelings as\n"
	.string "our POKéMON.$"

Johto_Route37_Text_83F2486::
	.string "POKéMON can’t do a thing if they are\n"
	.string "asleep.\p"
	.string "I’ll show you how scary that is!$"

Johto_Route37_Text_83F24D4::
	.string "I lost. That’s pretty sad…$"

Johto_Route37_Text_83F24EF::
	.string "Putting POKéMON to sleep or paralyz-\n"
	.string "ing them are good battle techniques.$"

Johto_Route37_Text_83F2539::
	.string "SUNNY: Hi!\p"
	.string "I’m SUNNY of the WEEKDAY family!\p"
	.string "If you can guess my favorite day,\n"
	.string "I’ll give you a reward.$"

Johto_Route37_Text_83F25C9::
	.string "I was told to give you this if I saw\n"
	.string "you!$"

Johto_Route37_Text_83F25F3::
	.string "SUNNY: That thing…\p"
	.string "Um…\p"
	.string "… What was it now…\p"
	.string "…\p"
	.string "Oh! I remember now!\p"
	.string "A POKéMON that knows electric\n"
	.string "moves should hold it.\p"
	.string "My sis MONICA said it powers up\n"
	.string "electric moves!$"

Johto_Route37_Text_83F2697::
	.string "SUNNY: My sisters and brothers are\n"
	.string "MONICA, TUSCANY, WESLEY, ARTHUR,\l"
	.string "FRIEDA and SANTOS.\p"
	.string "They’re all older than me!$"

Johto_Route37_Text_83F2732::
	.string "ROUTE 37\n"
	.string "ECRUTEAK CITY AHEAD$"
