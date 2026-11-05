Johto_Route31_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route31_EventScript_83EE4B0
	.byte 0

Johto_Route31_EventScript_83EE4B0::
	end

@ Johto_Route31 object 1 (local id 1, gfx 0x24) at 27,10
Johto_Route31_EventScript_83EE4B1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4440, 0, Johto_Route31_Text_83EE5F1, Johto_Route31_Text_83EE626
	msgbox Johto_Route31_Text_83EE62E, MSGBOX_AUTOCLOSE
	end

@ Johto_Route31 object 2 (local id 2, gfx 0x23) at 50,13
Johto_Route31_EventScript_83EE4C8::
	lock
	faceplayer
	msgbox Johto_Route31_Text_83EE5B8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route31 object 3 (local id 3, gfx 0x11) at 18,11
Johto_Route31_EventScript_83EE4D5::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5018, 1, Johto_Route31_EventScript_83EE4F8
	goto_if_ge VAR_QZ_0x5018, 2, Johto_Route31_EventScript_83EE561

Johto_Route31_EventScript_83EE4ED::
	msgbox Johto_Route31_Text_83EE699, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route31_EventScript_83EE4F8::
	msgbox Johto_Route31_Text_83EE728, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route31_EventScript_83EE516
	@ Moemon Kai: Crystal's sleepy man's other lines (docs/REGIONS_AUDIT.md 138)
	msgbox Johto_Route31_Text_Declined, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route31_EventScript_83EE516::
	getpartysize
	goto_if_lt VAR_RESULT, 2, Johto_Route31_EventScript_CantTakeLastMon
	qz_takebackstorymon 1
	goto_if_eq VAR_RESULT, 2, Johto_Route31_EventScript_83EE56F
	setvar VAR_QZ_0x5018, 2
	msgbox Johto_Route31_Text_83EE75E, MSGBOX_DEFAULT
	delay 0x37
	msgbox Johto_Route31_Text_83EE78B, MSGBOX_DEFAULT

Johto_Route31_EventScript_83EE53C::
	setorcopyvar VAR_0x8000, ITEM_TM139   @ Moemon Kai: Crystal's NIGHTMARE (T11b)
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x149A
	msgbox Johto_Route31_Text_83EE832, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route31_EventScript_83EE561::
	goto_if_unset FLAG_QZ_0x149A, Johto_Route31_EventScript_83EE53C
	goto Johto_Route31_EventScript_83EE4ED

Johto_Route31_EventScript_83EE56F::
	msgbox Johto_Route31_Text_MissingMail, MSGBOX_DEFAULT
	release
	end

Johto_Route31_EventScript_CantTakeLastMon::
	msgbox Johto_Route31_Text_CantTakeLastMon, MSGBOX_DEFAULT
	release
	end

@ Johto_Route31 object 4 (local id 4, gfx 0x3E87) at 9,9
Johto_Route31_EventScript_83EE579::
	lock
	faceplayer
	msgbox Johto_Route31_Text_83EE927, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route31 sign / bg event at 50,10
Johto_Route31_EventScript_83EE586::
	lock
	msgbox Johto_Route31_Text_83EE9C4, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route31 sign / bg event at 8,8
Johto_Route31_EventScript_83EE592::
	lock
	msgbox Johto_Route31_Text_83EE99C, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route31 object 5 (local id 5, gfx 0x3A9E) at 44,8
Johto_Route31_EventScript_83EE59E::
	setorcopyvar VAR_0x8000, ITEM_POTION   @ Moemon Kai: Crystal\'s (fix_item_balls.py)
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route31 object 6 (local id 6, gfx 0x3A9E) at 13,17
Johto_Route31_EventScript_83EE5AB::
	setorcopyvar VAR_0x8000, ITEM_POKE_BALL   @ Moemon Kai: Crystal\'s (fix_item_balls.py)
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route31_Text_83EE5B8::
	.string "DARK CAVE…\p"
	.string "If MOéMON could light it up,\n"
	.string "I’d explore it.$"

Johto_Route31_Text_83EE5F1::
	.string "I caught a bunch of MOéMON.\n"
	.string "Let me battle with you!$"

Johto_Route31_Text_83EE626::
	.string "Awwwww…$"

Johto_Route31_Text_83EE62E::
	.string "You can catch MOéMON even if\n"
	.string "you have six with you.\p"
	.string "If you catch one, it’ll go to\n"
	.string "your BOX automatically.$"

Johto_Route31_Text_83EE699::
	.string "… Hnuurg… Huh?\p"
	.string "I walked too far today looking\n"
	.string "for MOéMON.\p"
	.string "My feet hurt and I’m sleepy…\p"
	.string "If I were a wild MOéMON, I’d\n"
	.string "be easy to catch…\p"
	.string "…Zzzz…$"

Johto_Route31_Text_83EE728::
	.string "…Zzzz… Huh?\p"
	.string "What’s that? You have MAIL\n"
	.string "for me?$"

Johto_Route31_Text_83EE757::
	.string "…Zzzz…$"

Johto_Route31_Text_83EE75E::
	.string "{PLAYER} handed over the\n"
	.string "MOéMON holding the MAIL.$"

Johto_Route31_Text_83EE78B::
	.string "Let’s see…\p"
	.string "…DARK CAVE leads to another road…\p"
	.string "That’s good to know.\p"
	.string "Thanks for bringing this to me.\p"
	.string "My friend’s a good guy, and you’re\n"
	.string "swell too!\p"
	.string "I’d like to do something good in\n"
	.string "return too!\p"
	.string "I know! I want you to have this!$"

Johto_Route31_Text_83EE832::
	.string "TM139 is NIGHTMARE.\p"
	.string "It’s a wicked move that steadily\n"
	.string "cuts the HP of a sleeping enemy.\p"
	.string "Ooooh… That’s scary…\p"
	.string "I don’t want to have bad dreams.$"

Johto_Route31_Text_83EE927::
	.string "I found a good MOéMON in DARK\n"
	.string "CAVE.\p"
	.string "I’m going to raise it to take\n"
	.string "on FALKNER.\p"
	.string "He’s the leader of VIOLET\n"
	.string "CITY’s GYM.$"

Johto_Route31_Text_83EE99C::
	.string "ROUTE 31\p"
	.string "VIOLET CITY - CHERRYGROVE CITY$"

Johto_Route31_Text_83EE9C4::
	.string "DARK CAVE$"

Johto_Route31_Text_Declined::
	.string "What? You don’t want anything?$"

Johto_Route31_Text_MissingMail::
	.string "Why is this MOéMON so special?\p"
	.string "It doesn’t have any MAIL.$"

Johto_Route31_Text_CantTakeLastMon::
	.string "If I take that MOéMON from you,\n"
	.string "what are you going to use in\l"
	.string "battle?$"
