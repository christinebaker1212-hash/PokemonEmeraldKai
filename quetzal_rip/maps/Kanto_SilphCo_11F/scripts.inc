Kanto_SilphCo_11F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SilphCo_11F_EventScript_8411B33
	.byte 0

Kanto_SilphCo_11F_EventScript_8411B33::
	call_if_unset FLAG_QZ_0x21F5, Kanto_SilphCo_11F_EventScript_84589C7
	end

@ Kanto_SilphCo_11F object 1 (local id 1, gfx 0x3F2D) at 9,9
Kanto_SilphCo_11F_EventScript_8411B3D::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2156, Kanto_SilphCo_11F_EventScript_8411BB7
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_SilphCo_11F_EventScript_8411B9B
	call_if_eq VAR_RESULT, 1, Kanto_SilphCo_11F_EventScript_8411BA4
	checkitemspace ITEM_MASTER_BALL, 1
	goto_if_eq VAR_RESULT, 0, Kanto_SilphCo_11F_EventScript_8411BAD
	additem ITEM_MASTER_BALL, 1
	loadword 0, Kanto_SilphCo_11F_Text_84276A4
	setorcopyvar VAR_0x8000, 4
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	msgbox Kanto_SilphCo_11F_Text_84276D2, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2156
	release
	end

Kanto_SilphCo_11F_EventScript_8411B9B::
	msgbox Kanto_SilphCo_11F_Text_84274F9, MSGBOX_DEFAULT
	return

Kanto_SilphCo_11F_EventScript_8411BA4::
	msgbox Kanto_SilphCo_11F_Text_84275CE, MSGBOX_DEFAULT
	return

Kanto_SilphCo_11F_EventScript_8411BAD::
	msgbox Kanto_SilphCo_11F_Text_8427775, MSGBOX_DEFAULT
	release
	end

Kanto_SilphCo_11F_EventScript_8411BB7::
	msgbox Kanto_SilphCo_11F_Text_84276D2, MSGBOX_DEFAULT
	release
	end

@ Kanto_SilphCo_11F object 2 (local id 2, gfx 0x3F5A) at 11,9
Kanto_SilphCo_11F_EventScript_8411BC1::
	msgbox Kanto_SilphCo_11F_Text_8427790, MSGBOX_NPC
	end

@ Kanto_SilphCo_11F trigger at 5,15 (VAR_QZ_0x512D == 0)
Kanto_SilphCo_11F_EventScript_8411BCA::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_SilphCo_11F_EventScript_8411BE2

@ Kanto_SilphCo_11F trigger at 6,15 (VAR_QZ_0x512D == 0)
Kanto_SilphCo_11F_EventScript_8411BD6::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_SilphCo_11F_EventScript_8411BE2

Kanto_SilphCo_11F_EventScript_8411BE2::
	textcolor 0
	applymovement 3, Quetzal_Movement_8457AB0
	waitmovement 0
	delay 0x19
	msgbox Kanto_SilphCo_11F_Text_84277EF, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 0, Kanto_SilphCo_11F_EventScript_8411C42
	call_if_eq VAR_TEMP_1, 1, Kanto_SilphCo_11F_EventScript_8411C54
	setvar VAR_LAST_TALKED, 3
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8541, 0, Kanto_SilphCo_11F_Text_8427892
	msgbox Kanto_SilphCo_11F_Text_84278A9, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	removeobject 3
	removeobject 4
	removeobject 6
	fadescreen 0
	setvar VAR_QZ_0x512D, 1
	setflag FLAG_QZ_0x21BA
	clearflag FLAG_QZ_0x21B5
	releaseall
	end

Kanto_SilphCo_11F_EventScript_8411C42::
	applymovement 3, Kanto_SilphCo_11F_Movement_8411C5F
	applymovement LOCALID_PLAYER, Kanto_SilphCo_11F_Movement_8411C69
	waitmovement 0
	return

Kanto_SilphCo_11F_EventScript_8411C54::
	applymovement 3, Kanto_SilphCo_11F_Movement_8411C65
	waitmovement 0
	return

@ Kanto_SilphCo_11F sign / bg event at 8,1
Kanto_SilphCo_11F_EventScript_8411C79::
	msgbox Kanto_SilphCo_11F_Text_8427A31, MSGBOX_SIGN
	end

@ Kanto_SilphCo_11F object 4 (local id 4, gfx 0x3F3D) at 16,12
Kanto_SilphCo_11F_EventScript_8411C82::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8582, 0, Kanto_SilphCo_11F_Text_842799F, Kanto_SilphCo_11F_Text_84279CE
	msgbox Kanto_SilphCo_11F_Text_84279E0, MSGBOX_AUTOCLOSE
	end

@ Kanto_SilphCo_11F object 6 (local id 6, gfx 0x3F3D) at 2,19
Kanto_SilphCo_11F_EventScript_8411C99::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8583, 0, Kanto_SilphCo_11F_Text_8427951, Kanto_SilphCo_11F_Text_8427973
	msgbox Kanto_SilphCo_11F_Text_8427982, MSGBOX_AUTOCLOSE
	end

Kanto_SilphCo_11F_EventScript_84589C7::
	setmetatile 5, 0x10, 0x3B0, 1
	setmetatile 6, 0x10, 0x3B1, 1
	setmetatile 5, 0x11, 0x3B8, 1
	setmetatile 6, 0x11, 0x3B9, 1
	setmetatile 6, 0x12, 0x335, 0
	return

@ Kanto_SilphCo_11F sign / bg event at 5,16
@ Kanto_SilphCo_11F sign / bg event at 5,17
@ Kanto_SilphCo_11F sign / bg event at 6,16
@ Kanto_SilphCo_11F sign / bg event at 6,17
Kanto_SilphCo_11F_EventScript_8458FCC::
	lockall
	setvar VAR_TEMP_1, 0x14
	setvar VAR_0x8004, 0x21F5
	goto_if_set FLAG_QZ_0x21F5, Quetzal_EventScript_8458602
	goto Quetzal_EventScript_8458FE6

@ Kanto_SilphCo_11F object 5 (local id 5, gfx 0x3F0E) at 16,18
Kanto_SilphCo_11F_EventScript_846C03E::
	setorcopyvar VAR_0x8000, 0x45
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SilphCo_11F_Movement_8411C5F:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_left
	step_end

Kanto_SilphCo_11F_Movement_8411C65:
	walk_down
	walk_down
	walk_down
	step_end

Kanto_SilphCo_11F_Movement_8411C69:
	delay_16
	delay_16
	delay_16
	delay_8
	delay_4
	walk_in_place_faster_right
	step_end

Kanto_SilphCo_11F_Text_84274F9::
	.string "PRESIDENT: Oh, dear boy!\n"
	.string "Thank you for saving SILPH.\p"
	.string "I will never forget you saved us in\n"
	.string "our moment of peril.\p"
	.string "I have to thank you in some way.\p"
	.string "Because I am rich, I can give you\n"
	.string "anything.\p"
	.string "Here, maybe this will do.$"

Kanto_SilphCo_11F_Text_84275CE::
	.string "PRESIDENT: Oh, dear girl!\n"
	.string "Thank you for saving SILPH.\p"
	.string "I will never forget you saved us in\n"
	.string "our moment of peril.\p"
	.string "I have to thank you in some way.\p"
	.string "Because I am rich, I can give you\n"
	.string "anything.\p"
	.string "Here, maybe this will do.$"

Kanto_SilphCo_11F_Text_84276A4::
	.string "{PLAYER} obtained a MASTER BALL\n"
	.string "from the PRESIDENT!$"

Kanto_SilphCo_11F_Text_84276D2::
	.string "PRESIDENT: You can’t buy that\n"
	.string "anywhere.\p"
	.string "It’s our secret prototype MASTER\n"
	.string "BALL.\p"
	.string "It will catch any POKéMON without\n"
	.string "fail!\p"
	.string "You should be quiet about using it,\n"
	.string "though.$"

Kanto_SilphCo_11F_Text_8427775::
	.string "You have no room for this.$"

Kanto_SilphCo_11F_Text_8427790::
	.string "SECRETARY: Thank you for rescuing\n"
	.string "all of us.\p"
	.string "We, from the PRESIDENT down,\n"
	.string "are indebted to you.$"

Kanto_SilphCo_11F_Text_84277EF::
	.string "Ah, {PLAYER}!\n"
	.string "So we meet again!\p"
	.string "The PRESIDENT and I are discussing\n"
	.string "a vital business proposition.\p"
	.string "Keep your nose out of grown-up\n"
	.string "matters…\p"
	.string "Or, experience a world of pain!$"

Kanto_SilphCo_11F_Text_8427892::
	.string "Arrgh!!\n"
	.string "I lost again!?$"

Kanto_SilphCo_11F_Text_84278A9::
	.string "Blast it all!\n"
	.string "You ruined our plans for SILPH!\p"
	.string "But, TEAM ROCKET will never fall!\p"
	.string "{PLAYER}! Never forget that all\n"
	.string "POKéMON exist for TEAM ROCKET!\p"
	.string "I must go, but I shall return!$"

Kanto_SilphCo_11F_Text_8427951::
	.string "Stop right there!\n"
	.string "Don’t you move!$"

Kanto_SilphCo_11F_Text_8427973::
	.string "Don’t…\n"
	.string "Please!$"

Kanto_SilphCo_11F_Text_8427982::
	.string "So, you want to see my BOSS?$"

Kanto_SilphCo_11F_Text_842799F::
	.string "Halt! Do you have an appointment\n"
	.string "with my BOSS?$"

Kanto_SilphCo_11F_Text_84279CE::
	.string "Gaah!\n"
	.string "Demolished!$"

Kanto_SilphCo_11F_Text_84279E0::
	.string "Watch your step…\n"
	.string "My BOSS likes his POKéMON tough!$"

Kanto_SilphCo_11F_Text_8427A31::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "11F$"
